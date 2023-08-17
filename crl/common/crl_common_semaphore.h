// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include <crl/common/crl_common_config.h>

#include <semaphore>

namespace crl {

class semaphore {
public:
	semaphore() = default;
	semaphore(const semaphore &other) = delete;
	semaphore &operator=(const semaphore &other) = delete;
	semaphore(semaphore &&other) = delete;
	semaphore &operator=(semaphore &&other) = delete;

	void acquire() {
		_impl.acquire();
	}
	void release() {
		_impl.release();
	}

private:
	std::binary_semaphore _impl{0};

};

} // namespace crl
