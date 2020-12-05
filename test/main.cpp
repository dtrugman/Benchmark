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

// This tells Catch to provide a main() and should exist only here
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <chrono>
#include <thread>
#include <random>

#include "benchmark.hpp"

using namespace std;
using namespace bm;

static std::random_device rd;
static std::mt19937 gen(rd());

void REQUIRE_SINGLE_VALUE(const Mark& mark, const Mark::nanoseconds& ns)
{
    REQUIRE(mark.iterations() == 1);
    REQUIRE(mark.as_nanoseconds() == ns.count());
    REQUIRE(mark.minimal().as_nanoseconds() == ns.count());
    REQUIRE(mark.maximal().as_nanoseconds() == ns.count());
}

TEST_CASE("Mark clearing", "[mark]")
{
    SECTION("Clearing")
    {
        Mark::nanoseconds ns(300);
        Mark mark(ns);

        REQUIRE_SINGLE_VALUE(mark, ns);
        mark.clear();

        REQUIRE(mark.iterations() == 0);
        REQUIRE(mark.as_nanoseconds() == 0);
        REQUIRE(mark.minimal().as_nanoseconds() == Mark::nanoseconds::max().count());
        REQUIRE(mark.maximal().as_nanoseconds() == Mark::nanoseconds::min().count());
    }
}

TEST_CASE("Mark averaging", "[mark]")
{
    Mark mark;

    size_t values = 10000;
    int64_t sum = 0;
    int64_t max_value = Mark::nanoseconds::max().count() / values;
    auto dis = std::uniform_int_distribution<uint64_t>(0, max_value);
    for (size_t i = 0; i < values; i++)
    {
        uint64_t val = dis(gen);
        sum += val;

        mark += std::chrono::nanoseconds(val);
    }


    REQUIRE(mark.average().as_nanoseconds() == (sum / values));
}

TEST_CASE("Mark max", "[mark]")
{
    Mark mark;

    SECTION("Max of no duration")
    {
        REQUIRE(mark.maximal().as_nanoseconds() <= Mark::nanoseconds::min().count());
    }

    SECTION("Max of random durations")
    {
        auto values = 100;
        auto max_value = Mark::nanoseconds::max().count() / values;
        auto dis = std::uniform_int_distribution<uint64_t>(0, max_value);

        auto max = Mark::nanoseconds::min();

        for (auto i = 0; i < values; i++)
        {
            auto val = std::chrono::nanoseconds(dis(gen));
            max = (std::max)(max, val);

            mark += val;
        }

        REQUIRE(mark.maximal().as_nanoseconds() == max.count());
    }
}

TEST_CASE("Mark min", "[mark]")
{
    Mark mark;

    SECTION("Min of no duration")
    {
        REQUIRE(mark.minimal().as_nanoseconds() <= Mark::nanoseconds::max().count());
    }

    SECTION("Min of random durations")
    {
        auto values = 100;
        auto max_value = Mark::nanoseconds::max().count() / values;
        auto dis = std::uniform_int_distribution<uint64_t>(0, max_value);
        auto min = std::chrono::nanoseconds::max();

        for (auto i = 0; i < values; i++)
        {
            auto val = std::chrono::nanoseconds(dis(gen));
            min = (std::min)(min, val);

            mark += val;
        }

        REQUIRE(mark.minimal().as_nanoseconds() == min.count());
    }
}

TEST_CASE("Mark overflow protection", "[mark]")
{
    Mark mark;

    SECTION("Overflow protection when adding nanoseconds")
    {
        static const unsigned VALUE = 1000;

        auto max = std::chrono::nanoseconds::max();
        mark += max;
        REQUIRE_SINGLE_VALUE(mark, max);

        mark += std::chrono::nanoseconds(VALUE);
        REQUIRE(mark.as_nanoseconds() == VALUE);
        REQUIRE(mark.iterations() == 1);
        REQUIRE(mark.maximal().as_nanoseconds() == max.count());
        REQUIRE(mark.minimal().as_nanoseconds() == VALUE);
    }

    SECTION("Overflow protection when adding mark")
    {
        auto max = std::chrono::nanoseconds::max();
        mark += max;
        REQUIRE_SINGLE_VALUE(mark, max);

        static const unsigned VALUE = 1000;
        Mark addition;
        addition += Mark::nanoseconds(VALUE);
        addition += Mark::nanoseconds(VALUE);

        mark += addition;
        REQUIRE(mark.as_nanoseconds() == addition.as_nanoseconds());
        REQUIRE(mark.iterations() == addition.iterations());
        REQUIRE(mark.maximal().as_nanoseconds() == max.count());
        REQUIRE(mark.minimal().as_nanoseconds() == VALUE);
    }
}

TEST_CASE("Benchmarking", "[benchmark]")
{
    Mark mark;
    std::chrono::milliseconds delay(100);

    SECTION("Measure sleep")
    {
        mark = Bench::mark([delay]() { std::this_thread::sleep_for(delay); });
    }

    REQUIRE(mark.as_milliseconds() >= delay.count());
}

TEST_CASE("Transparency", "[benchmark]")
{
    SECTION("Return primitive")
    {
        auto res = Bench::mark([]() { return 1 + 2; });
        REQUIRE(res.second == 3);
    }

    SECTION("Return object")
    {
        const char *text = "Benchmark :)";
        auto res = Bench::mark([text]() { return std::string(text); });
        REQUIRE(res.second.compare(text) == 0);
    }
}

#ifdef BENCHMARK_THREAD_CPUTIME

TEST_CASE("Thread clock supported", "[benchmark][thread]")
{
    REQUIRE(thread_clock::supported());
}

TEST_CASE("Thread benchmarking", "[benchmark][thread]")
{
    Mark mark;
    std::chrono::milliseconds delay(10);

    // Sleep is not considered as CPU time spent on the sleeping thread
    SECTION("Sleep for 10 milliseconds")
    {
        mark = Thread::mark([delay]() { std::this_thread::sleep_for(delay); });
    }

    REQUIRE(mark.as_milliseconds() < delay.count());
}

#endif // BENCHMARK_THREAD_CPUTIME

TEST_CASE("Scoped benchmarking", "[benchmark]")
{
    Mark mark;
    std::chrono::milliseconds delay(30);
    int iterations = 15;

    SECTION("Manual termination")
    {
        for (auto i = 0; i < iterations; i++)
        {
            std::this_thread::sleep_for(delay);
            Bench::Probe probe(mark);
            std::this_thread::sleep_for(delay);
            probe.done();
            std::this_thread::sleep_for(delay);
        }
    }

    SECTION("Scoped termination")
    {
        for (auto i = 0; i < iterations; i++)
        {
            std::this_thread::sleep_for(delay);
            {
                Bench::Probe probe(mark);
                std::this_thread::sleep_for(delay);
            }
            std::this_thread::sleep_for(delay);
        }
    }

    REQUIRE(mark.iterations() == iterations);

    // Sleep is non deterministic, OS can't sleep exactly Xms
    int64_t min_delay = delay.count();
    int64_t max_delay = 2 * delay.count();

    auto avg_ms = mark.average().as_milliseconds();
    REQUIRE(avg_ms >= min_delay);
    REQUIRE(avg_ms <  max_delay);

    auto total_ms = mark.as_milliseconds();
    REQUIRE(total_ms >= min_delay * iterations);
    REQUIRE(total_ms <  max_delay * iterations);
}
