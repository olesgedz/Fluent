set(target volk)
project(${target})

set(CMAKE_CXX_STANDARD 17)

set(srcs volk.c)

add_library(${target} STATIC ${srcs})

set(includePath "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(${target}
		PUBLIC
        ${includePath}

        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)