// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include <crl/crl_object_on_thread.h>

#include <crl/crl_fp_exceptions.h>
#include <thread>

namespace crl::details {

thread_policy::thread_policy() {
	std::thread([this] { run(); }).detach();
}

thread_policy::~thread_policy() {
	// WHY: a wake can finish after its job destroys the owner, so keep
	// the semaphore alive until every actual wake has returned.
	while (_waking.load(std::memory_order_acquire)) {
		std::this_thread::yield();
	}
}

void thread_policy::run() {
	toggle_fp_exceptions(true);

	_semaphore.acquire();
	while (_list.process()) {
		if (_list.try_idle()) {
			_semaphore.acquire();
		}
	}
}

void thread_policy::wake_async() const {
	_waking.fetch_add(1);
	const auto guard = details::finally([&] {
		_waking.fetch_sub(1, std::memory_order_release);
	});
	_semaphore.release();
}

} // namespace crl::details
