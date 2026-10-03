// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include <crl/common/crl_common_list.h>

namespace crl::details {

list::BasicEntry list::_idle(nullptr);

list::list() : _alive(new bool(true)) {
}

list::BasicEntry *list::ReverseList(BasicEntry *entry, BasicEntry *next) {
	entry->next = nullptr;
	do {
		auto third = next->next;
		next->next = entry;
		entry = next;
		next = third;
	} while (next);
	return entry;
}

bool list::push_entry(BasicEntry *entry) {
	auto head = (BasicEntry*)nullptr;
	while (true) {
		if (_head.compare_exchange_weak(head, entry)) {
			return (head == &_idle);
		}
		entry->next = (head == &_idle) ? nullptr : head;
	}
}

bool list::try_idle() {
	auto head = (BasicEntry*)nullptr;
	// A retiring runner must not touch the list after handing it off.
	return !_head.load() && _head.compare_exchange_strong(head, &_idle);
}

bool list::process() {
	if (auto entry = _head.exchange(nullptr)) {
		const auto alive = _alive;
		if (const auto next = entry->next) {
			entry = ReverseList(entry, next);
		}
		do {
			const auto basic = entry;
			entry = entry->next;
			basic->process(basic);
			if (!*alive) {
				delete alive;
				return false;
			}
		} while (entry);
	}
	return true;
}

list::~list() {
	*_alive = false;
}

} // namespace crl::details
