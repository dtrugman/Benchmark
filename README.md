# C++ (11+) Benchmarking tool

A simple yet very powerful cross-platform benchmarking tool for C++.

## TL;DR

Benchmark methods easily.
*See a wide variety of code samples under the 'sample' directory.*

Quick snippet:
```cpp
auto ns = Bench::mark(method_to_measure, arg1, arg2);
std::cout << "Method took " << ns << "ns" << std::endl;
```

## What good here?

My life is much easier when I use it because:

- I don't have to mess with `std::chrono::duration`s and `std::chrono::duration_cast`s
- It very easy to continuously benchmark performance of certain classes in production
  (by using `Mark` members in my class that aggregate min, max & avg run times)
- I can use the posix-specific thread-specific clock, getting real runtime results
  (ignore sleep times, etc.)

## How to embed into your project

As simple as `cp include <your-project's-include-dir>/benchamrk`.
Then include `benchmark.hpp` and voilà...
