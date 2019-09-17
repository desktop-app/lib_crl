// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include <crl/winapi/crl_winapi_semaphore.h>

#ifdef CRL_USE_WINAPI

#include <crl/winapi/crl_winapi_windows_h.h>

namespace crl {

auto semaphore::implementation::create() -> pointer {
	auto result = CreateSemaphore(nullptr, 0, 1, nullptr);
	if (!result) {
		std::terminate();
	}
	return result;
}

void semaphore::implementation::operator()(pointer value) {
	if (value) {
		CloseHandle(value);
	}
};

void semaphore::acquire() {
	WaitForSingleObject(_handle.get(), INFINITE);
}

void semaphore::release() {
	ReleaseSemaphore(_handle.get(), 1, nullptr);
}

} // namespace crl

#endif // CRL_USE_WINAPI
