// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include <crl/common/crl_common_config.h>

#ifdef CRL_FORCE_COMMON_QUEUE
#include <crl/common/crl_common_queue.h>
#elif defined CRL_USE_DISPATCH // CRL_FORCE_COMMON_QUEUE
#include <crl/dispatch/crl_dispatch_queue.h>
#else // CRL_USE_DISPATCH
#include <crl/common/crl_common_queue.h>
#endif // !CRL_FORCE_COMMON_QUEUE && !CRL_USE_DISPATCH
