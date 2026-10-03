#include <crl/common/crl_common_list.h>
#include <crl/crl_object_on_queue.h>
#include <crl/crl_object_on_thread.h>

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <source_location>
#include <thread>
#include <vector>

namespace {

using Callback = std::pair<void(*)(void*), void*>;

auto Submit = static_cast<void(*)(Callback)>(nullptr);
auto Pending = std::deque<Callback>();

void Check(
		bool value,
		std::source_location location = std::source_location::current()) {
	if (!value) {
		std::fprintf(
			stderr,
			"Check failed at %s:%u\n",
			location.file_name(),
			location.line());
		std::abort();
	}
}

void RunOne() {
	Check(!Pending.empty());
	const auto callback = Pending.front();
	Pending.pop_front();
	callback.first(callback.second);
}

class Pool {
public:
	Pool();
	~Pool();
	void submit(Callback callback);

private:
	void run();

	std::mutex _mutex;
	std::condition_variable _condition;
	std::deque<Callback> _pending;
	std::vector<std::thread> _threads;
	bool _stopping = false;

};

struct Probe {
	Probe(std::promise<void> *destroyed, bool threadAffinity);
	~Probe();

	std::promise<void> *destroyed = nullptr;
	bool threadAffinity = false;
	std::thread::id thread = std::this_thread::get_id();
	std::array<int, 4> last = { -1, -1, -1, -1 };
};

struct ThrowingMove {
	ThrowingMove() = default;
	ThrowingMove(ThrowingMove&&) noexcept(false);
	void operator()(int &count) &&;
};

Pool::Pool() {
	for (auto i = 0; i != 4; ++i) {
		_threads.emplace_back([this] { run(); });
	}
}

Pool::~Pool() {
	{
		const auto lock = std::lock_guard(_mutex);
		_stopping = true;
	}
	_condition.notify_all();
	for (auto &thread : _threads) {
		thread.join();
	}
}

void Pool::submit(Callback callback) {
	{
		const auto lock = std::lock_guard(_mutex);
		_pending.push_back(callback);
	}
	_condition.notify_one();
}

void Pool::run() {
	while (true) {
		auto callback = Callback();
		{
			auto lock = std::unique_lock(_mutex);
			_condition.wait(lock, [&] {
				return _stopping || !_pending.empty();
			});
			if (_pending.empty()) {
				return;
			}
			callback = _pending.front();
			_pending.pop_front();
		}
		callback.first(callback.second);
	}
}

Probe::Probe(std::promise<void> *destroyed, bool threadAffinity)
: destroyed(destroyed)
, threadAffinity(threadAffinity) {
}

Probe::~Probe() {
	Check(!threadAffinity || thread == std::this_thread::get_id());
	destroyed->set_value();
}

ThrowingMove::ThrowingMove(ThrowingMove&&) noexcept(false) {
}

void ThrowingMove::operator()(int &count) && {
	++count;
}

void TestListOwnership() {
	auto list = std::make_unique<crl::details::list>();
	auto count = 0;
	Check(list->push_is_first([&] { Check(++count == 1); }));
	Check(!list->push_is_first([&] { Check(++count == 2); }));
	Check(list->process());
	Check(!list->push_is_first([&] { Check(++count == 3); }));
	Check(!list->try_idle());
	Check(list->process());
	Check(list->try_idle());
	Check(list->push_is_first([&] { Check(++count == 4); }));
	Check(list->process());
	Check(!list->push_is_first([&] { list.reset(); }));
	const auto raw = list.get();
	Check(!raw->process());
	Check(!list && count == 4);
}

void TestQueueBatches() {
	Submit = [](Callback callback) { Pending.push_back(callback); };
	auto object = std::make_unique<crl::object_on_queue<int>>(0);
	Check(Pending.size() == 1);
	RunOne();
	Check(Pending.empty());
	object->with([&](int &count) {
		Check(++count == 1);
		object->with([](int &count) { Check(++count == 3); });
	});
	object->with([](int &count) { Check(++count == 2); });
	Check(Pending.size() == 1);
	RunOne();
	Check(Pending.size() == 1);
	RunOne();
	Check(Pending.empty());
	object->with([&](int&) { object.reset(); });
	RunOne();
	Check(!object && Pending.size() == 1);
	RunOne();
	Check(Pending.empty());
}

void TestInlineDestruction() {
	Submit = [](Callback callback) { callback.first(callback.second); };
	auto destroyed = std::promise<void>();
	auto done = destroyed.get_future();
	auto object = std::make_unique<crl::object_on_queue<Probe>>(
		&destroyed,
		false);
	const auto weak = object->weak();
	weak.with([&](Probe&) { object.reset(); });
	Check(done.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
	weak.with([](Probe&) { Check(false); });
}

void TestThreadWakes() {
	auto object = crl::object_on_thread<int>(0);
	const auto weak = object.weak();
	weak.with(ThrowingMove());
	weak.with([owned = std::make_unique<int>(1)](int &count) {
		count += *owned;
	});
	for (auto i = 0; i != 5000; ++i) {
		object.with_sync([&](int &count) { Check(count++ == i + 2); });
		if (!(i % 32)) {
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}
	}
	const auto &constant = object;
	constant.weak().with_sync([](const int &count) { Check(count == 5002); });
}

template <typename Object>
void TestDestructionRaces(bool threadAffinity) {
	auto executed = std::atomic<int>(0);
	for (auto round = 0; round != 1000; ++round) {
		auto destroyed = std::promise<void>();
		auto done = destroyed.get_future();
		auto object = std::make_unique<Object>(
			&destroyed,
			bool(threadAffinity));
		object->with_sync([](Probe&) {});
		const auto weak = object->weak();
		auto start = std::atomic<bool>(false);
		auto producers = std::vector<std::thread>();
		for (auto producer = 0; producer != 4; ++producer) {
			producers.emplace_back([&, weak, producer] {
				while (!start.load(std::memory_order_acquire)) {
					std::this_thread::yield();
				}
				for (auto job = 0; job != 250; ++job) {
					weak.with([&, producer, job](Probe &probe) {
						Check(probe.last[producer] < job);
						Check(!probe.threadAffinity
							|| probe.thread == std::this_thread::get_id());
						probe.last[producer] = job;
						++executed;
					});
					if (!(job % 13)) {
						std::this_thread::yield();
					}
				}
			});
		}
		start.store(true, std::memory_order_release);
		object.reset();
		for (auto &producer : producers) {
			producer.join();
		}
		Check(done.wait_for(std::chrono::seconds(5)) == std::future_status::ready);
	}
	std::printf(
		"%s: 1000 destructions, 1000000 weak posts, %d executed\n",
		threadAffinity ? "thread" : "queue",
		executed.load());
}

auto CurrentPool = static_cast<Pool*>(nullptr);

} // namespace

namespace crl::details {

void async_plain(void (*callable)(void*), void *argument) {
	Submit({ callable, argument });
}

} // namespace crl::details

int main() {
	TestListOwnership();
	TestQueueBatches();
	TestInlineDestruction();
	TestThreadWakes();
	{
		auto pool = Pool();
		CurrentPool = &pool;
		Submit = [](Callback callback) { CurrentPool->submit(callback); };
		TestDestructionRaces<crl::object_on_queue<Probe>>(false);
	}
	TestDestructionRaces<crl::object_on_thread<Probe>>(true);
	std::puts("All queue ownership and lifetime checks passed.");
}
