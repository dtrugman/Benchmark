/*
 *  Copyright 2016-present Daniel Trugman
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef BENCHMARK_THREAD_CLOCK_HPP
#define BENCHMARK_THREAD_CLOCK_HPP

#if defined __linux__
#   include <unistd.h>
#   ifdef _POSIX_THREAD_CPUTIME
#       define BENCHMARK_THREAD_CPUTIME
#   endif
#endif

#ifdef BENCHMARK_THREAD_CPUTIME

#include <time.h>

namespace bm {

// A defenition of a thread clock according to std::chrono
// based on the CLOCK_THREAD_CPUTIME_ID implementation
struct thread_clock
{
    using duration   = std::chrono::nanoseconds;
    using rep        = duration::rep;
    using period     = duration::period;
    using time_point = std::chrono::time_point<thread_clock>;

    static const bool is_steady = true;

    static bool supported() noexcept
    {
        clockid_t cid;
        return pthread_getcpuclockid(pthread_self(), &cid) == 0;
    }

    static time_point now() noexcept
    {
        struct timespec ts = {};
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
        return time_point(duration(((uint64_t)ts.tv_sec * 1000000000) + (uint64_t)ts.tv_nsec));
    }
};

} // namespace

#endif // BENCHMARK_THREAD_CPUTIME

#endif // BENCHMARK_HPP
