// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include <crl/common/crl_common_object_async.h>
#include <crl/common/crl_common_list.h>
#include <crl/crl_semaphore.h>
#include <atomic>

namespace crl {
namespace details {

class thread_policy {
protected:
	thread_policy();
	~thread_policy();

	template <typename Callable>
	void async_plain(Callable &&callable) const;

	template <typename Callable>
	void async_releasing(Callable &&callable) const;

private:
	void run();
	void wake_async() const;

	[[maybe_unused]] char _refcountPadding[kQueueCacheSeparation] = {};
	mutable details::list _list;
	mutable semaphore _semaphore;
	mutable std::atomic<int> _waking = 0;

};

template <typename Callable>
void thread_policy::async_plain(Callable &&callable) const {
	if (_list.push_is_first(std::forward<Callable>(callable))) {
		wake_async();
	}
}

template <typename Callable>
void thread_policy::async_releasing(Callable &&callable) const {
	async_plain(std::forward<Callable>(callable));
}

} // namespace details

template <typename Type>
using weak_on_thread = details::weak_async<details::thread_policy, Type>;

template <typename Type>
using object_on_thread = details::object_async<details::thread_policy, Type>;

} // namespace
