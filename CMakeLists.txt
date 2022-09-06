cmake_minimum_required(VERSION 2.8.12)
project(game)

set_property(TARGET tgt PROPERTY CXX_STANDARD 17)

include_directories("libs/fpm/include")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "src/*.hpp")
add_executable(game 
  ${SOURCES}
  ${HEADERS}
)  
target_link_libraries(game ${SDL2_LIBRARIES})
