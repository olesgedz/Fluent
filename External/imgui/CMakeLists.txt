set(target imgui)
project(${target})

set(CMAKE_CXX_STANDARD 23)

set(srcs 
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    backends/imgui_impl_vulkan.cpp
    backends/imgui_impl_glfw.cpp)

add_library(${target} STATIC ${srcs})

target_link_libraries(${target} PUBLIC glfw)

set(includePaths 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends)

target_include_directories(${target}
		PUBLIC
        ${includePaths}

        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)