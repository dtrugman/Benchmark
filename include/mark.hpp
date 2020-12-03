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

#ifndef BENCHMARK_MARK_HPP
#define BENCHMARK_MARK_HPP

#include <ratio>
#include <chrono>
#include <sstream>
#include <functional>

namespace bm
{

class Mark
{
public: // Types
    using nanoseconds  = std::chrono::nanoseconds;
    using microseconds = std::chrono::microseconds;
    using milliseconds = std::chrono::milliseconds;
    using seconds      = std::chrono::seconds;
    using minutes      = std::chrono::minutes;
    using hours        = std::chrono::hours;

    using overflow_callback = std::function<void(const Mark&)>;

public: // C'tors
    explicit Mark(overflow_callback cb = nullptr) :
        _overflow_callback(cb)
    {
        clear();
    }

    template < class Rep, class Period >
    explicit Mark(const std::chrono::duration<Rep, Period>& duration)
    {
        clear();
        (void)add(duration);
    }

public: // Copy operations
    Mark(const Mark&) = default;
    Mark& operator=(const Mark&) = default;

public: // Move operations
    Mark(Mark&& rhs)
    {
        *this = std::move(rhs);
    }

    Mark& operator=(Mark&& rhs)
    {
        *this = rhs;
        rhs.clear();
        return *this;
    }

public: // Overloaded operators
    Mark& operator+=(const Mark& rhs)
    {
        return add(rhs);
    }

    template < class Rep, class Period >
    Mark& operator+=(const std::chrono::duration<Rep, Period>& duration)
    {
        return add(duration);
    }

    // Avoid overloading comparison operators.
    // Such operations are confusing when aggregations come into play.
    // Is it expected to compare total values or average ones?
    // Just convert to the relevant units and then compare.

    friend std::ostream& operator<<(std::ostream& out, const Mark& mark);

public: // Total, accumulated time getters
    template < class ToDuration >
    int64_t as() const
    {
        return std::chrono::duration_cast<ToDuration>(_total).count();
    }

    int64_t as_nanoseconds()  const { return as<nanoseconds >(); }
    int64_t as_microseconds() const { return as<microseconds>(); }
    int64_t as_milliseconds() const { return as<milliseconds>(); }
    int64_t as_seconds()      const { return as<seconds     >(); }
    int64_t as_minutes()      const { return as<minutes     >(); }
    int64_t as_hours()        const { return as<hours       >(); }

    int64_t iterations() const { return _iterations; }

public: // Min/Max/Avg getters
    Mark average() const
    {
        return (_iterations == 0) ? Mark() : Mark(_total/_iterations);
    }

    Mark minimal() const { return Mark(_min); }
    Mark maximal() const { return Mark(_max); }

public: // Methods
    void clear()
    {
        _min = (nanoseconds::max)();
        _max = (nanoseconds::min)();
        _total = nanoseconds(0);
        _iterations = 0;
    }

    std::string to_string() const
    {
        std::ostringstream out;
        out << *this;
        return out.str();
    }

private: // Methods
    template < class Rep, class Period >
    Mark& add(const std::chrono::duration<Rep, Period>& duration)
    {
        auto ns = std::chrono::duration_cast<nanoseconds>(duration);
        return add(1, ns, ns, ns);
    }

    Mark& add(const Mark& rhs)
    {
        return add(rhs._iterations, rhs._total, rhs._max, rhs._min);
    }

    Mark& add(uint64_t iterations,
              const nanoseconds& total,
              const nanoseconds& max,
              const nanoseconds& min)
    {
        if (_total + total < _total)
        {
            if (_overflow_callback)
            {
                _overflow_callback(*this);
            }

            _total      = total;
            _iterations = iterations;
        }
        else
        {
            _total      += total;
            _iterations += iterations;
        }

        _max = (std::max)(_max, max);
        _min = (std::min)(_min, min);

        return *this;
    }

private: // Members
    nanoseconds       _min;
    nanoseconds       _max;
    nanoseconds       _total;
    uint64_t          _iterations;
    overflow_callback _overflow_callback;
};

inline std::ostream& operator<<(std::ostream& out, const Mark& mark)
{
    out << "Total of " << mark._total.count() << "ns after " << mark._iterations << " iterations";
    return out;
}

} // namespace bm

#endif // BENCHMARK_MARK_HPP
