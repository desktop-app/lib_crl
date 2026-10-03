// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include <crl/common/crl_common_object_async.h>
#include <crl/crl_queue.h>

namespace crl {
namespace details {

class queue_policy {
protected:
	template <typename Callable>
	void async_plain(Callable &&callable) const;

	template <typename Callable>
	void async_releasing(Callable &&callable) const;

private:
#ifdef CRL_USE_COMMON_QUEUE
	mutable crl::queue _queue{ crl::queue::self_destroying_t() };
#else // CRL_USE_COMMON_QUEUE
	mutable crl::queue _queue;
#endif // !CRL_USE_COMMON_QUEUE

};

template <typename Callable>
void queue_policy::async_plain(Callable &&callable) const {
	_queue.async(std::forward<Callable>(callable));
}

template <typename Callable>
void queue_policy::async_releasing(Callable &&callable) const {
#ifdef CRL_USE_COMMON_QUEUE
	_queue.async_releasing(std::forward<Callable>(callable));
#else // CRL_USE_COMMON_QUEUE
	async_plain(std::forward<Callable>(callable));
#endif // !CRL_USE_COMMON_QUEUE
}

} // namespace details

template <typename Type>
using weak_on_queue = details::weak_async<details::queue_policy, Type>;

template <typename Type>
using object_on_queue = details::object_async<details::queue_policy, Type>;

} // namespace
