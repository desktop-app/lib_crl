# This file is part of Desktop App Toolkit,
# a set of libraries for developing nice desktop applications.
#
# For license and copyright information please follow this link:
# https://github.com/desktop-app/legal/blob/master/LEGAL

{
  'includes': [
    '../gyp_helpers/common/common.gypi',
  ],
  'targets': [{
    'target_name': 'lib_crl',
    'hard_dependency': 1,
    'dependencies': [
    ],
    'includes': [
      '../gyp_helpers/common/library.gypi',
      '../gyp_helpers/modules/qt.gypi',
    ],
    'defines': [
    ],
    'variables': {
      'src_loc': '.',
      'crl_src_loc': '<(src_loc)/crl',
    },
    'include_dirs': [
      '<(src_loc)',
    ],
    'direct_dependent_settings': {
      'include_dirs': [
        '<(src_loc)',
      ],
    },
    'sources': [
      '<(crl_src_loc)/common/crl_common_config.h',
      '<(crl_src_loc)/common/crl_common_guards.h',
      '<(crl_src_loc)/common/crl_common_list.cpp',
      '<(crl_src_loc)/common/crl_common_list.h',
      '<(crl_src_loc)/common/crl_common_on_main.cpp',
      '<(crl_src_loc)/common/crl_common_on_main.h',
      '<(crl_src_loc)/common/crl_common_on_main_guarded.h',
      '<(crl_src_loc)/common/crl_common_queue.cpp',
      '<(crl_src_loc)/common/crl_common_queue.h',
      '<(crl_src_loc)/common/crl_common_sync.h',
      '<(crl_src_loc)/common/crl_common_utils.h',
      '<(crl_src_loc)/dispatch/crl_dispatch_async.cpp',
      '<(crl_src_loc)/dispatch/crl_dispatch_async.h',
      '<(crl_src_loc)/dispatch/crl_dispatch_on_main.h',
      '<(crl_src_loc)/dispatch/crl_dispatch_queue.cpp',
      '<(crl_src_loc)/dispatch/crl_dispatch_queue.h',
      '<(crl_src_loc)/dispatch/crl_dispatch_semaphore.cpp',
      '<(crl_src_loc)/dispatch/crl_dispatch_semaphore.h',
      '<(crl_src_loc)/mac/crl_mac_time.cpp',
      '<(crl_src_loc)/linux/crl_linux_time.cpp',
      '<(crl_src_loc)/qt/crl_qt_async.cpp',
      '<(crl_src_loc)/qt/crl_qt_async.h',
      '<(crl_src_loc)/qt/crl_qt_guards.h',
      '<(crl_src_loc)/qt/crl_qt_semaphore.cpp',
      '<(crl_src_loc)/qt/crl_qt_semaphore.h',
      '<(crl_src_loc)/winapi/crl_winapi_async.cpp',
      '<(crl_src_loc)/winapi/crl_winapi_async.h',
      '<(crl_src_loc)/winapi/crl_winapi_dll.h',
      '<(crl_src_loc)/winapi/crl_winapi_list.cpp',
      '<(crl_src_loc)/winapi/crl_winapi_list.h',
      '<(crl_src_loc)/winapi/crl_winapi_semaphore.cpp',
      '<(crl_src_loc)/winapi/crl_winapi_semaphore.h',
      '<(crl_src_loc)/winapi/crl_winapi_time.cpp',
      '<(crl_src_loc)/winapi/crl_winapi_windows_h.h',
      '<(crl_src_loc)/crl.h',
      '<(crl_src_loc)/crl_async.h',
      '<(crl_src_loc)/crl_object_on_queue.h',
      '<(crl_src_loc)/crl_on_main.h',
      '<(crl_src_loc)/crl_queue.h',
      '<(crl_src_loc)/crl_semaphore.h',
      '<(crl_src_loc)/crl_time.cpp',
      '<(crl_src_loc)/crl_time.h',
    ],
  }],
}
