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

namespace bm {

template < class Clock,
           class Rep    = typename Clock::rep,
           class Period = typename Clock::period >
class GenericBench
{
public:
    template< class T >
    using result_type = typename std::result_of<T>::type;

    template< bool B, class T = void >
    using enable_if_type = typename std::enable_if<B, T>::type;

    using duration_type = std::chrono::duration<Rep, Period>;

public:
    GenericBench() = delete;

public:
    template < class Func, class... Args >
    static auto mark(Func&& func, Args&&... args)
        -> enable_if_type< std::is_void<result_type<Func&&(Args&&...)>>::value,
                           duration_type >
    {
        auto before = Clock::now();
        std::forward<Func>(func)(std::forward<Args>(args)...);
        auto after = Clock::now();
        return std::chrono::duration_cast<duration_type>(after - before);
    }

    template < class Func, class... Args >
    static auto mark(Func&& func, Args&&... args)
        -> enable_if_type< !std::is_void<result_type<Func&&(Args&&...)>>::value,
                           std::pair<duration_type, result_type<Func&&(Args&&...)>> >
    {
        auto before = Clock::now();
        auto result = std::forward<Func>(func)(std::forward<Args>(args)...);
        auto after = Clock::now();
        auto duration = std::chrono::duration_cast<duration_type>(after - before);
        return std::make_pair(duration, result);
    }
};

using Bench = GenericBench<std::chrono::steady_clock>;

} // namespace

#endif // BENCHMARK_BENCHMARK_HPP
