cmake_minimum_required(VERSION 2.8.12)
project(game)

# set_property(TARGET tgt PROPERTY CXX_STANDARD 17)
SET(CXX_STANDARD 17)
SET(CXX_MAKE_FLAGS "-fpermissive")

find_package( Boost REQUIRED system)
include_directories( ${Boost_INCLUDE_DIRS} )

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# add_subdirectory(lib/dyad)
include_directories("lib")

file(GLOB SOURCES "**/*.cpp" "**/*.c")
file(GLOB HEADERS "**/*.hpp" "**/*.h")
add_executable(game
  ${SOURCES}
  ${HEADERS}
)
target_link_libraries(game ${SDL2_LIBRARIES})
