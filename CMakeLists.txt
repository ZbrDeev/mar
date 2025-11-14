cmake_minimum_required(VERSION 3.5)

project(mar VERSION 1.0.0)

file(GLOB mar_src CONFIGURE_DEPENDS "mar/*.h" "mar/*.c")
add_executable(mar ${mar_src})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(mar PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(mar PRIVATE -Wall -Wextra -Wpedantic)
else()
    message(FATAL_ERROR "Incorrect CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}")
endif()