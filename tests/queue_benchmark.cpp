#include <crl/crl_object_on_queue.h>
#include <crl/crl_object_on_thread.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <future>
#include <latch>
#include <semaphore>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

constexpr auto kPostsPerProducer = 100000;
constexpr auto kSamples = 7;

auto Filter = std::array<std::string_view, 4>();
auto Measured = 0;

struct Counter {
	explicit Counter(std::promise<void> *destroyed);
	~Counter();

	std::promise<void> *destroyed = nullptr;
	int value = 0;
};

struct LargeCounter : Counter {
	using Counter::Counter;
	std::array<char, 256> extra = {};
};

Counter::Counter(std::promise<void> *destroyed) : destroyed(destroyed) {
}

Counter::~Counter() {
	destroyed->set_value();
}

double NanosecondsPerPost(Clock::duration elapsed, int posts) {
	return std::chrono::duration<double, std::nano>(elapsed).count() / posts;
}

template <typename Object>
void Measure(const char *name, int producers, bool weak, bool backlogged) {
	const auto ownership = weak ? "weak" : "strong";
	const auto consumer = backlogged ? "backlogged" : "running";
	if (!Filter[0].empty()
		&& (Filter[0] != name
			|| Filter[1] != ownership
			|| Filter[2] != consumer
			|| Filter[3] != std::to_string(producers))) {
		return;
	}
	++Measured;
	auto enqueue = std::vector<double>();
	auto complete = std::vector<double>();
	auto destroyed = std::promise<void>();
	auto destruction = destroyed.get_future();
	{
		auto object = Object(&destroyed);
		const auto observer = object.weak();
		for (auto sample = 0; sample != kSamples + 1; ++sample) {
			object.with_sync([](auto &counter) { counter.value = 0; });
			auto gate = std::binary_semaphore(0);
			auto entered = std::promise<void>();
			if (backlogged) {
				const auto entering = entered.get_future();
				object.with([&](auto&) {
					entered.set_value();
					gate.acquire();
				});
				entering.wait();
			}
			auto ready = std::latch(producers);
			auto start = std::latch(1);
			auto threads = std::vector<std::thread>();
			for (auto producer = 0; producer != producers; ++producer) {
				threads.emplace_back([&] {
					ready.count_down();
					start.wait();
					const auto post = [&](auto &target) {
						for (auto i = 0; i != kPostsPerProducer; ++i) {
							target.with([](auto &counter) { ++counter.value; });
						}
					};
					if (weak) {
						post(observer);
					} else {
						post(object);
					}
				});
			}
			ready.wait();
			const auto began = Clock::now();
			start.count_down();
			for (auto &thread : threads) {
				thread.join();
			}
			const auto posted = Clock::now();
			if (backlogged) {
				gate.release();
			}
			object.with_sync([&](auto &counter) {
				if (counter.value != producers * kPostsPerProducer) {
					std::abort();
				}
			});
			const auto finished = Clock::now();
			if (sample) {
				enqueue.push_back(NanosecondsPerPost(
					posted - began,
					producers * kPostsPerProducer));
				complete.push_back(NanosecondsPerPost(
					finished - began,
					producers * kPostsPerProducer));
			}
		}
	}
	destruction.wait();
	std::sort(enqueue.begin(), enqueue.end());
	std::sort(complete.begin(), complete.end());
	std::printf(
		"%s,%s,%s,%d,%.2f,%.2f\n",
		name,
		ownership,
		consumer,
		producers,
		enqueue[kSamples / 2],
		complete[kSamples / 2]);
	std::fflush(stdout);
}

template <typename Object>
void MeasureObject(const char *name) {
	for (const auto producers : { 1, 4, 8 }) {
		for (const auto weak : { false, true }) {
			for (const auto backlogged : { false, true }) {
				Measure<Object>(name, producers, weak, backlogged);
			}
		}
	}
}

} // namespace

int main(int argc, char **argv) {
	if (argc != 1 && argc != 5) {
		std::fputs(
			"Usage: crl_queue_benchmark [object ownership consumer producers]\n",
			stderr);
		return 1;
	}
	if (argc == 5) {
		for (auto i = 0; i != 4; ++i) {
			Filter[i] = argv[i + 1];
		}
	}
	std::puts("object,ownership,consumer,producers,enqueue_ns,complete_ns");
	MeasureObject<crl::object_on_thread<Counter>>("thread-small");
	MeasureObject<crl::object_on_thread<LargeCounter>>("thread-large");
	MeasureObject<crl::object_on_queue<Counter>>("queue-small");
	MeasureObject<crl::object_on_queue<LargeCounter>>("queue-large");
	return Measured ? 0 : 1;
}
