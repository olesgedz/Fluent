set(target tiny_imageformat)
project(${target})

set(CMAKE_CXX_STANDARD 23)

set(srcs tinyimageformat.cpp)

add_library(${target} STATIC ${srcs})

set(includePath "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(${target}
		PUBLIC
        ${includePath}

        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)