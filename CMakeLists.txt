cmake_minimum_required(VERSION 2.8)
project(tjbot)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_C_FLAGS_MINSIZEREL "")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "")
set(CMAKE_C_FLAGS_RELEASE "")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fomit-frame-pointer -m32 -fPIC -fvisibility=hidden -Ofast -s -march=native")
set(CMAKE_SHARED_LINKER_FLAGS "-m32 -lm")

set(SOURCE_FILES tjbot.c)

add_library(tjbot SHARED ${SOURCE_FILES})