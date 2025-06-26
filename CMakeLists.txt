cmake_minimum_required(VERSION 3.31)
project(mini_jvm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

file(GLOB_RECURSE JVM_SRC CONFIGURE_DEPENDS "src/*.cpp")

add_executable(mini-jvm ${JVM_SRC}
)
target_include_directories(mini-jvm PUBLIC src)