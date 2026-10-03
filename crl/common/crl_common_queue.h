// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include <crl/common/crl_common_config.h>

#include <crl/common/crl_common_list.h>
#include <crl/common/crl_common_utils.h>
#include <atomic>

#ifndef CRL_USE_COMMON_QUEUE
#define CRL_USE_COMMON_QUEUE
#endif // !CRL_USE_COMMON_QUEUE

namespace crl {
namespace details {
class main_queue_pointer;
class queue_policy;
} // namespace details

class queue {
public:
	queue();
	queue(const queue &other) = delete;
	queue &operator=(const queue &other) = delete;
	~queue();

	template <typename Callable>
	void async(Callable &&callable) {
		if (_list.push_is_first(std::forward<Callable>(callable))) {
			wake_async();
		}
	}

	template <typename Callable>
	void sync(Callable &&callable) {
		semaphore waiter;
		async([&] {
			const auto guard = details::finally([&] { waiter.release(); });
			callable();
		});
		waiter.acquire();
	}

private:
	friend class details::main_queue_pointer;
	friend class details::queue_policy;

	struct self_destroying_t {
	};

	static void ProcessCallback(void *that);

	queue(main_queue_processor processor);
	explicit queue(self_destroying_t);

	// The pushed job may hold the last reference to this queue's owner.
	template <typename Callable>
	void async_releasing(Callable &&callable) {
		_touching.fetch_add(1);
		const auto guard = details::finally([&] {
			_touching.fetch_sub(1, std::memory_order_release);
		});
		async(std::forward<Callable>(callable));
	}

	void wake_async();
	void process();
	void finish_batch();

	main_queue_processor _main_processor = nullptr;
	details::list _list;
	std::atomic<int> _touching = 0;
	std::atomic_flag _queued = ATOMIC_FLAG_INIT;
	const bool _self_destroying = false;

};

} // namespace crl
