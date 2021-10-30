set(Target Fluent)
project(${Target})

set(IncludePath ${CMAKE_CURRENT_SOURCE_DIR})

set(CoreSources
	Core/Application.cpp
	Core/Event.cpp
	Core/Input.cpp
	Core/Window.cpp
	Core/Log.cpp)

set(RendererSources 
	Renderer/Renderer.cpp
	Renderer/GraphicContext.cpp
	Renderer/DeviceAllocator.cpp
	Renderer/VirtualFrame.cpp
	Renderer/Image.cpp
	Renderer/Buffer.cpp
	Renderer/RenderPass.cpp
	Renderer/Framebuffer.cpp
	Renderer/Pipeline.cpp
	Renderer/Shader.cpp
	Renderer/ShaderReflection.cpp
	Renderer/CommandBuffer.cpp)

set(Sources 
	${CoreSources}
	${RendererSources})

find_package(Vulkan REQUIRED)

set(CommonLibs glfw glm spdlog vk_mem_alloc tiny_imageformat Vulkan::Vulkan)

if (MSVC)
    set(PlatformLibDirs $ENV{VULKAN_SDK}/Lib)
    set(PlatformLibs spirv-cross-cored spirv-cross-glsld)
elseif (UNIX AND NOT APPLE)
	set(PlatformLibs spirv-cross-core spirv-cross-glsl shaderc_shared)
	set(CompileOptions -g)
elseif(APPLE)
    set(PlatformLibDirs $ENV{VULKAN_SDK}/lib)
    set(PlatformLibs spirv-cross-core spirv-cross-glsl shaderc)
endif()

set(Libs ${CommonLibs} ${PlatformLibs})
set(LibDirs ${PlatformLibDirs})

add_library(${Target} ${Sources})

target_include_directories(${Target}
	PUBLIC
	${IncludePath}

	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

target_link_directories(${Target} PUBLIC ${LibDirs})
target_link_libraries(${Target} PUBLIC ${Libs})
target_compile_options(${Target} PUBLIC ${CompileOptions})