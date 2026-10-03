# Queue lifetime tests and contention benchmark

The standalone tests need CMake and a C++20 compiler, without Qt or the app:
Run these commands from the library root.

```sh
cmake -S tests -B /tmp/crl-tests -DCMAKE_BUILD_TYPE=Debug \
    -DCRL_TEST_SANITIZER=address,undefined
cmake --build /tmp/crl-tests --parallel 4
ctest --test-dir /tmp/crl-tests --output-on-failure
```

Use a separate build directory with `CRL_TEST_SANITIZER=thread` for TSan.
On macOS, `CRL_TEST_DISPATCH_SEMAPHORE=OFF` also tests the common atomic
semaphore. The queue tests substitute a controllable scheduler to exercise
inline destruction, batch boundaries, FIFO ordering, and concurrent weak
posts during destruction. The thread tests use real worker threads.

On macOS the optional benchmark uses the real dispatch scheduler, the common
queue, and optimized compilation, independently of the sanitizer test target:

```sh
cmake --build /tmp/crl-tests --target crl_queue_benchmark --parallel 4
/tmp/crl-tests/crl_queue_benchmark > /tmp/crl-benchmark.csv
```

It measures strong and weak calls with 1, 4, and 8 producers, for both a live
consumer and a consumer blocked until all producers finish. Small and large
wrapped objects exercise different distances between the shared ownership
counters and the queue head. Each CSV row reports the median of seven samples
after a warmup, with enqueue time and total time through callback completion
in nanoseconds per callback. Every sample checks the executed callback count.

For comparisons, build this same benchmark source against both library
versions, run each executable separately on an otherwise idle host, and
alternate their order. These are measurements, not pass/fail thresholds:
allocation layout, hardware, scheduler load, and the standard library affect
the results. Queue object destruction is outside the timed region.

Individual cases can also run in a fresh process, avoiding allocator history
from earlier cases:

```sh
/tmp/crl-tests/crl_queue_benchmark thread-small strong backlogged 8
```

The four selectors are `thread-small`, `thread-large`, `queue-small`, or
`queue-large`; `strong` or `weak`; `running` or `backlogged`; and `1`, `4`, or `8`.
