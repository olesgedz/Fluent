set(target stb_image)
project(${target})

set(CMAKE_CXX_STANDARD 23)

set(srcs stb_image.cpp)

add_library(${target} STATIC ${srcs})

set(includePath "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(${target}
		PUBLIC
        ${includePath}

        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)