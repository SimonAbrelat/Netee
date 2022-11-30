cmake_minimum_required(VERSION 2.8.12)
PROJECT(game C CXX)

SET(CXX_STANDARD 20)
SET(CXX_MAKE_FLAGS "-fpermissive")
SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

include_directories("lib")
include_directories("lib/enet-1.3.17/include")

file(GLOB_RECURSE SOURCES_CPP "src/*.cpp")
file(GLOB_RECURSE HEADERS_HPP "src/*.hpp")
add_executable(game
  ${SOURCES_CPP}
  ${HEADERS_CPP}
)
target_link_libraries(game
  ${SDL2_LIBRARIES}
  "enet"
)
