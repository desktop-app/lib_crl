add_library(lib_crl OBJECT)
init_target(lib_crl)

get_filename_component(src_loc . REALPATH)

set(lib_crl_sources
PRIVATE
    crl/common/crl_common_config.h
    crl/common/crl_common_guards.h
    crl/common/crl_common_list.cpp
    crl/common/crl_common_list.h
    crl/common/crl_common_on_main.cpp
    crl/common/crl_common_on_main.h
    crl/common/crl_common_on_main_guarded.h
    crl/common/crl_common_queue.cpp
    crl/common/crl_common_queue.h
    crl/common/crl_common_sync.h
    crl/common/crl_common_utils.h
    crl/dispatch/crl_dispatch_async.cpp
    crl/dispatch/crl_dispatch_async.h
    crl/dispatch/crl_dispatch_on_main.h
    crl/dispatch/crl_dispatch_queue.cpp
    crl/dispatch/crl_dispatch_queue.h
    crl/dispatch/crl_dispatch_semaphore.cpp
    crl/dispatch/crl_dispatch_semaphore.h
    crl/mac/crl_mac_time.cpp
    crl/linux/crl_linux_time.cpp
    crl/qt/crl_qt_async.cpp
    crl/qt/crl_qt_async.h
    crl/qt/crl_qt_guards.h
    crl/qt/crl_qt_semaphore.cpp
    crl/qt/crl_qt_semaphore.h
    crl/winapi/crl_winapi_async.cpp
    crl/winapi/crl_winapi_async.h
    crl/winapi/crl_winapi_dll.h
    crl/winapi/crl_winapi_list.cpp
    crl/winapi/crl_winapi_list.h
    crl/winapi/crl_winapi_semaphore.cpp
    crl/winapi/crl_winapi_semaphore.h
    crl/winapi/crl_winapi_time.cpp
    crl/winapi/crl_winapi_windows_h.h
    crl/crl.h
    crl/crl_async.h
    crl/crl_object_on_queue.h
    crl/crl_on_main.h
    crl/crl_queue.h
    crl/crl_semaphore.h
    crl/crl_time.cpp
    crl/crl_time.h
)
nice_target_sources(lib_crl ${src_loc} "${lib_crl_sources}")

target_include_directories(lib_crl
PUBLIC
    ${src_loc}
)

target_link_libraries(lib_crl
PUBLIC
    common
    external_qt
)
