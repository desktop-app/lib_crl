// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include <crl/crl_queue.h>

#ifdef CRL_USE_COMMON_QUEUE

#include <crl/crl_async.h>
#include <crl/crl_fp_exceptions.h>

#include <thread>

namespace crl {

queue::queue() = default;

queue::queue(main_queue_processor processor) : _main_processor(processor) {
}

queue::queue(self_destroying_t) : _self_destroying(true) {
}

queue::~queue() {
	// WHY: a self-destroying queue's last job can free it while a releasing
	// pusher or the previous batch's tail is still between its push or
	// _queued.clear() and its last access, so wait for those few steps.
	while (_touching.load(std::memory_order_acquire)) {
		std::this_thread::yield();
	}
}

void queue::wake_async() {
	if (!_queued.test_and_set()) {
		(_main_processor ? _main_processor : details::async_plain)(
			ProcessCallback,
			static_cast<void*>(this));
	}
}

void queue::process() {
	if (!_list.process()) {
		return;
	} else if (!_self_destroying) {
		finish_batch();
		return;
	}
	_touching.fetch_add(1);
	const auto guard = details::finally([&] {
		_touching.fetch_sub(1, std::memory_order_release);
	});
	finish_batch();
}

void queue::finish_batch() {
	_queued.clear();

	if (!_list.empty()) {
		wake_async();
	}
}

void queue::ProcessCallback(void *that) {
#ifdef CRL_THROW_FP_EXCEPTIONS
	static thread_local const bool kInited = [] {
		toggle_fp_exceptions(true);
		return true;
	}();
#endif // CRL_THROW_FP_EXCEPTIONS

	static_cast<queue*>(that)->process();
}

} // namespace crl

#endif // CRL_USE_COMMON_QUEUE
