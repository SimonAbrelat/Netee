cmake_minimum_required(VERSION 2.8.12)
PROJECT(game C CXX)

# set_property(TARGET tgt PROPERTY CXX_STANDARD 17)
SET(CXX_STANDARD 20)
SET(CXX_MAKE_FLAGS "-fpermissive")

find_package( Boost REQUIRED system)
include_directories( ${Boost_INCLUDE_DIRS} )

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

#add_subdirectory("lib/dyad")
include_directories("lib")
include_directories("lib/udt4/src/")

file(GLOB_RECURSE SOURCES_CPP "src/*.cpp")
file(GLOB_RECURSE LIB_CPP "lib/udt4/src/*.cpp")
file(GLOB_RECURSE HEADERS_HPP "src/*.hpp")
file(GLOB_RECURSE LIB_H "lib/udt4/src/*.h")
add_executable(game
  ${SOURCES_CPP}
  ${HEADERS_CPP}
  ${LIB_CPP}
  ${LIB_H}
)
target_link_libraries(game
  ${SDL2_LIBRARIES}

)
