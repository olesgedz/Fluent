set(target vk_mem_alloc)
project(${target})

set(CMAKE_CXX_STANDARD 20)

set(srcs vk_mem_alloc.cpp)

add_library(vk_mem_alloc STATIC ${srcs})

find_package(Vulkan REQUIRED)
target_link_libraries(${target} PUBLIC Vulkan::Vulkan)

if (APPLE)
    target_compile_options(${target} PUBLIC -Wno-nullability-completeness)
endif()

set(includePath "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(${target}
        PUBLIC
        ${includePath}

        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)