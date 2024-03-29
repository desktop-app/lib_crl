// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include <crl/common/crl_common_config.h>

#if defined CRL_USE_WINAPI
#include <crl/winapi/crl_winapi_semaphore.h>
#elif defined CRL_USE_DISPATCH // CRL_USE_WINAPI
#include <crl/dispatch/crl_dispatch_semaphore.h>
#else // CRL_USE_DISPATCH
#include <crl/common/crl_common_semaphore.h>
#endif // !CRL_USE_WINAPI && !CRL_USE_DISPATCH
