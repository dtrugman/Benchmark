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

#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <random>
#include <map>

#include "benchmark.hpp"

// NOTE:
// Example based on https://en.cppreference.com/w/cpp/numeric/random/random_device

using namespace bm;

class RNG
{
public:
    int generate(int from, int to)
    {
        Bench::Probe probe(_generate_mark);

		std::uniform_int_distribution<int> dist(from, to);
        return dist(_rd);
    }

    friend std::ostream& operator<<(std::ostream& out, RNG& rng);

private:
    Mark _generate_mark;
    std::random_device _rd;
};

inline std::ostream& operator<<(std::ostream& out, RNG& rng)
{
    out << "Generated " << rng._generate_mark.iterations() << " numbers:\n"
        << "Total time = " << rng._generate_mark.as_nanoseconds() << "ns\n"
        << "Average time = " << rng._generate_mark.average().as_nanoseconds() << "ns\n"
        << "Max time = " << rng._generate_mark.maximal().as_nanoseconds() << "ns\n"
        << "Min time = " << rng._generate_mark.minimal().as_nanoseconds() << "ns\n";
    return out;
}

int main()
{
    RNG rng;

    static const int from = 0;
    static const int to   = 9;

    std::map<int, int> hist;
    for (auto i = 0; i < 20000; ++i)
    {
        ++hist[rng.generate(from, to)];
    }

    std::cout << "Generation histogram:\n";
    for (auto h : hist)
    {
        std::cout << h.first << " : " << std::string(h.second / 100, '*') << '\n';
    }
    std::cout << "\n" << rng;

    return 0;
}
