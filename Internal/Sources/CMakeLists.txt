set(Target Fluent)
project(${Target})

set(IncludePath ${CMAKE_CURRENT_SOURCE_DIR})

set(CoreSources
	Core/Application.cpp
	Core/Event.cpp
	Core/Input.cpp
	Core/Window.cpp
	Core/Log.cpp
	Core/FileSystem.cpp)

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
    set(PlatformLibs spirv-cross-cored spirv-cross-glsld shaderc_shared)
elseif (UNIX AND NOT APPLE)
	set(PlatformLibs spirv-cross-core spirv-cross-glsl shaderc_shared)
elseif(APPLE)
    set(PlatformLibDirs $ENV{VULKAN_SDK}/lib)
    set(PlatformLibs spirv-cross-core spirv-cross-glsl shaderc_shared)
endif()

if (CMAKE_BUILD_TYPE EQUAL Debug)
	if (MSVC)
		set(CompileOptions /DEBUG)
	else()
		set(CompileOptions -g)
	endif()
else()
	if (MSVC)
		set(CompileOptions /O2)
	else()
		set(CompileOptions -O3)
	endif()
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