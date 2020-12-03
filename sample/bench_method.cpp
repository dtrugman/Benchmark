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

#include "benchmark.hpp"

using namespace bm;

class Configuration
{
public:
    bool Load(const std::string& input)
    {
        std::ifstream fin(input, std::ios::binary);
        if (!fin)
        {
            return false;
        }

        std::string word;
        while (fin >> word)
        {
            _config.push_back(word);
        }
        return true;
    }

private:
    std::vector<std::string> _config;
};

int main()
{
    std::string input("./sample/lipsum.txt");

    Configuration config;
    auto result = Bench::mark(std::mem_fn(&Configuration::Load), &config, input);
    std::cout << "Took " << result.first.count() << "ns" << std::endl;
    std::cout << "Success = " << result.second << std::endl;
    return 0;
}
