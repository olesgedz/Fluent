cmake_minimum_required(VERSION 3.20)
project(Fluent)

# Set policies
cmake_policy(SET CMP0054 NEW) # ENABLE CMP0054: Only interpret if() arguments as variables or keywords when unquoted.
cmake_policy(SET CMP0042 NEW) # ENABLE CMP0042: MACOSX_RPATH is enabled by default.
cmake_policy(SET CMP0063 NEW) # ENABLE CMP0063: Honor visibility properties for all target types.
cmake_policy(SET CMP0077 NEW) # ENABLE CMP0077: option() honors normal variables

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE EQUAL "Debug")
    set(DebugMode True)
elseif(CMAKE_BUILD_TYPE EQUAL "Release")
    set(DebugMode False)
endif()

set(DebugMode True)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(External/volk)
add_subdirectory(External/glfw)
add_subdirectory(External/glm)
add_subdirectory(External/imgui)
add_subdirectory(External/spdlog)
add_subdirectory(External/SPIRV-Cross)
add_subdirectory(External/tiny_imageformat)
add_subdirectory(External/tiny_ktx)
add_subdirectory(External/vk_mem_alloc)
add_subdirectory(Internal/Sources)
add_subdirectory(Internal/Examples)
