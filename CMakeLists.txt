cmake_minimum_required(VERSION 3.14)

project(Automata)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3")

add_executable(lnfa
    src/main.cpp
    src/lnfa.cpp
    src/automaton.cpp
)

target_include_directories(lnfa
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
