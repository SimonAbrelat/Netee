cmake_minimum_required(VERSION 2.8.12)
PROJECT(game C CXX)

# set_property(TARGET tgt PROPERTY CXX_STANDARD 17)
SET(CXX_STANDARD 20)
SET(CXX_MAKE_FLAGS "-fpermissive")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

find_package( Boost REQUIRED system)
include_directories( ${Boost_INCLUDE_DIRS} )

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

#add_subdirectory("lib/dyad")
include_directories("lib")

file(GLOB_RECURSE SOURCES_CPP "src/*.cpp")
file(GLOB_RECURSE SOURCES_C "lib/**/*.c")
file(GLOB_RECURSE HEADERS_HPP "src/*.hpp")
file(GLOB_RECURSE HEADERS_H "lib/**/*.h")
add_executable(game
  ${SOURCES_CPP}
  ${HEADERS_CPP}
  ${SOURCES_C}
  ${HEADERS_C}
)
target_link_libraries(game
  ${SDL2_LIBRARIES}
)
