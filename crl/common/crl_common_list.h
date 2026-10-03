// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include <crl/common/crl_common_config.h>

#include <crl/common/crl_common_utils.h>
#include <atomic>

namespace crl::details {

// Separate queue heads from shared_ptr counters on 128-byte cache lines.
inline constexpr auto kQueueCacheSeparation = 128;

class list {
public:
	list();

	template <typename Callable>
	bool push_is_first(Callable &&callable) {
		return push_entry(AllocateEntry(std::forward<Callable>(callable)));
	}
	bool process();
	[[nodiscard]] bool try_idle();

	~list();

private:
	struct BasicEntry;
	using ProcessEntryMethod = void(*)(BasicEntry *entry);

	struct BasicEntry {
		BasicEntry(ProcessEntryMethod method) : process(method) {
		}

		BasicEntry *next = nullptr;
		ProcessEntryMethod process = nullptr;
	};

	template <typename Function>
	struct Entry : BasicEntry {
		Entry(Function &&function)
		: BasicEntry(Entry::Process)
		, function(std::move(function)) {
		}
		Entry(const Function &function)
		: BasicEntry(Entry::Process)
		, function(function) {
		}
		Function function;

		static void Process(BasicEntry *entry) {
			auto full = static_cast<Entry*>(entry);
			auto guard = details::finally([=] { delete full; });
			full->function();
		}

	};

	template <typename Callable>
	static Entry<std::decay_t<Callable>> *AllocateEntry(
			Callable &&callable) {
		using Function = std::decay_t<Callable>;
		using Type = Entry<Function>;

		return new Type(std::forward<Callable>(callable));
	}

	static BasicEntry *ReverseList(BasicEntry *entry, BasicEntry *next);

	bool push_entry(BasicEntry *entry);

	static BasicEntry _idle;
	static_assert(std::atomic<BasicEntry*>::is_always_lock_free);

	// The idle marker makes publication the last access for active pushers.
	std::atomic<BasicEntry*> _head = &_idle;
	bool *_alive = nullptr;

};

} // namespace crl::details
