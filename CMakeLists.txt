cmake_minimum_required(VERSION 2.8)
project(listaccount)

include(sdk/contrib/cmake/CMakeLists.txt)
include_directories(sdk/include)
link_libraries(Mega)

find_package(Threads)

add_executable(listaccount main.cpp)

if (MSVC)
  set(CMAKE_CXX_FLAGS "/W4 /bigobj /EHsc /wd4503 /wd4996 /wd4702 /wd4100")
else()
  set(CMAKE_CXX_FLAGS "-pedantic -Wall -Wextra -Wconversion")
  if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
  endif()
endif()
