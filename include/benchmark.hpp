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

#ifndef BENCHMARK_BENCHMARK_HPP
#define BENCHMARK_BENCHMARK_HPP

#include <type_traits>
#include <iostream>
#include <chrono>

#include "mark.hpp"
#include "thread_clock.hpp"

namespace bm {

template < class Clock >
class GenericBench
{
public:
    template< class T >
    using result_type = typename std::result_of<T>::type;

    template< bool B, class T = void >
    using enable_if_type = typename std::enable_if<B, T>::type;

public:
    GenericBench() = delete;

public:
    class Probe
    {
    public:
        Probe(Mark & mark) :
            _done(false), _mark(mark), _start(Clock::now()) {}
        ~Probe() { done(); }

        void done()
        {
            if (_done) return;

            _done = true;
            _stop = Clock::now();
            _mark += (_stop - _start);
        }

    private:
        using timepoint = typename Clock::time_point;

    private:
        bool      _done;
        Mark &    _mark;
        timepoint _start;
        timepoint _stop;
    };

public:
    template < class Func, class... Args >
    static auto mark(Func&& func, Args&&... args)
        -> enable_if_type< std::is_void<result_type<Func&&(Args&&...)>>::value,
                           Mark >
    {
        auto before = Clock::now();
        std::forward<Func>(func)(std::forward<Args>(args)...);
        auto after = Clock::now();
        return Mark(after - before);
    }

    template < class Func, class... Args >
    static auto mark(Func&& func, Args&&... args)
        -> enable_if_type< !std::is_void<result_type<Func&&(Args&&...)>>::value,
                           std::pair<Mark, result_type<Func&&(Args&&...)>> >
    {
        auto before = Clock::now();
        auto result = std::forward<Func>(func)(std::forward<Args>(args)...);
        auto after = Clock::now();
        auto mark = Mark(after - before);
        return std::make_pair(mark, result);
    }
};

using Bench = GenericBench<std::chrono::steady_clock>;

#ifdef BENCHMARK_THREAD_CPUTIME
    using Thread = GenericBench<thread_clock>;
#endif // BENCHMARK_THREAD_CPUTIME

} // namespaces

#endif // BENCHMARK_BENCHMARK_HPP
