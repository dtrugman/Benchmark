cmake_minimum_required (VERSION 3.5)

project (benchmark)

# APPLE => MacOS
# WIN32 => Windows
# LINUX => Linux (The only one we have to define on our own)
if (UNIX AND NOT APPLE)
    set (LINUX TRUE)
endif ()

add_compile_options (-std=c++11 -Wall -Wextra -pedantic -Werror)

include_directories (include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

add_executable (bench_void_function sample/bench_void_function.cpp)
add_executable (bench_function sample/bench_function.cpp)
add_executable (bench_method sample/bench_method.cpp)
add_executable (probe_method sample/probe_method.cpp)

add_executable (ut test/main.cpp)

if (LINUX)
    target_link_libraries (ut pthread)
endif ()
