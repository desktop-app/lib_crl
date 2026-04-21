// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include <crl/winapi/crl_winapi_async.h>

#ifdef CRL_USE_WINAPI

#include <crl/winapi/crl_winapi_windows_h.h>
#include <concrt.h>
#include <VersionHelpers.h>

namespace crl::details {
namespace {

struct AsyncPlainContext {
	void (*callable)(void*);
	void *argument;
};

VOID CALLBACK AsyncPlainCallback(
		PTP_CALLBACK_INSTANCE /*instance*/,
		PVOID context) {
	const auto ctx = static_cast<AsyncPlainContext*>(context);
	ctx->callable(ctx->argument);
	delete ctx;
}

// On Windows 7, MSVC PPL scheduler bootstrap calls WinRT::Initialize which
// tries to LoadLibraryExW("combase.dll", ..., LOAD_LIBRARY_SEARCH_SYSTEM32).
// combase.dll does not exist on Win 7, so PPL throws
// scheduler_resource_allocation_error and crashes startup. Fall back to the
// native Win32 thread pool on Win 7 only — same fire-and-forget semantics,
// no PPL/WinRT dependency. Win 8+ keeps the original PPL path.
const bool UseThreadPoolFallback = !IsWindows8OrGreater();

} // namespace

void async_plain(void (*callable)(void*), void *argument) {
	if (UseThreadPoolFallback) {
		const auto ctx = new AsyncPlainContext{ callable, argument };
		if (TrySubmitThreadpoolCallback(AsyncPlainCallback, ctx, nullptr)) {
			return;
		}
		delete ctx;
		callable(argument);
		return;
	}
	Concurrency::CurrentScheduler::ScheduleTask(callable, argument);
}

} // namespace crl::details

#endif // CRL_USE_WINAPI
