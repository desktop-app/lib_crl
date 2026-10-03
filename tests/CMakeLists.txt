cmake_minimum_required(VERSION 3.16)
project(crl_queue_tests LANGUAGES CXX)

find_package(Threads REQUIRED)
enable_testing()

add_executable(crl_queue_tests
    queue_tests.cpp
    ../crl/common/crl_common_fp_exceptions.cpp
    ../crl/common/crl_common_list.cpp
    ../crl/common/crl_common_queue.cpp
    ../crl/crl_object_on_thread.cpp
    ../crl/dispatch/crl_dispatch_semaphore.cpp
    ../crl/winapi/crl_winapi_fp_exceptions.cpp
)
target_compile_features(crl_queue_tests PRIVATE cxx_std_20)
target_include_directories(crl_queue_tests PRIVATE support ..)
target_link_libraries(crl_queue_tests PRIVATE Threads::Threads)
target_compile_definitions(crl_queue_tests PRIVATE CRL_FORCE_COMMON_QUEUE)
option(CRL_TEST_DISPATCH_SEMAPHORE "Use the dispatch semaphore" ${APPLE})
if (CRL_TEST_DISPATCH_SEMAPHORE)
    target_compile_definitions(crl_queue_tests PRIVATE CRL_USE_DISPATCH)
endif()

set(CRL_TEST_SANITIZER "" CACHE STRING "Sanitizers for the queue tests")
if (CRL_TEST_SANITIZER AND NOT MSVC)
    target_compile_options(crl_queue_tests PRIVATE
        -fsanitize=${CRL_TEST_SANITIZER} -fno-omit-frame-pointer)
    target_link_options(crl_queue_tests PRIVATE
        -fsanitize=${CRL_TEST_SANITIZER})
endif()

add_test(NAME crl_queue_tests COMMAND crl_queue_tests)
set_tests_properties(crl_queue_tests PROPERTIES TIMEOUT 120)

if (APPLE)
    add_executable(crl_queue_benchmark EXCLUDE_FROM_ALL
        queue_benchmark.cpp
        ../crl/common/crl_common_fp_exceptions.cpp
        ../crl/common/crl_common_list.cpp
        ../crl/common/crl_common_queue.cpp
        ../crl/crl_object_on_thread.cpp
        ../crl/dispatch/crl_dispatch_async.cpp
        ../crl/dispatch/crl_dispatch_semaphore.cpp
    )
    target_compile_features(crl_queue_benchmark PRIVATE cxx_std_20)
    target_include_directories(crl_queue_benchmark PRIVATE ..)
    target_link_libraries(crl_queue_benchmark PRIVATE Threads::Threads)
    target_compile_definitions(crl_queue_benchmark PRIVATE
        CRL_USE_DISPATCH CRL_FORCE_COMMON_QUEUE)
    target_compile_options(crl_queue_benchmark PRIVATE -O2)
endif()
