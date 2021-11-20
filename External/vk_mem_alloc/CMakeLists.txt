set(target vk_mem_alloc)
project(${target})

set(CMAKE_CXX_STANDARD 20)

set(srcs vk_mem_alloc.cpp)

add_library(vk_mem_alloc STATIC ${srcs})

if (APPLE)
    target_compile_options(${target} PUBLIC -Wno-nullability-completeness)
endif()

set(includePath "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_CURRENT_SOURCE_DIR}/../volk/)

target_include_directories(${target}
        PUBLIC
        ${includePath}

        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)