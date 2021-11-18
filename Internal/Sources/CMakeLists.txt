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
	Renderer/CommandBuffer.cpp
	Renderer/DescriptorSetLayout.cpp
	Renderer/DescriptorSet.cpp
	Renderer/Sampler.cpp
	Renderer/StagingBuffer.cpp)

set(MathSources
	Math/Math.cpp)

set(UiSources
	UI/UIContext.cpp)

set(Sources 
	${CoreSources}
	${RendererSources}
	${MathSources}
	${UiSources})

set(CommonLibs glfw glm imgui spdlog vk_mem_alloc tiny_imageformat tiny_ktx)

if (MSVC)
    set(PlatformLibDirs $ENV{VULKAN_SDK}/Lib)
    set(PlatformLibs spirv-cross-cored spirv-cross-glsld shaderc_shared vulkan-1.lib)
elseif (UNIX AND NOT APPLE)
	set(PlatformLibs spirv-cross-core spirv-cross-glsl shaderc_shared vulkan.so)
elseif(APPLE)
    set(PlatformLibDirs $ENV{VULKAN_SDK}/lib)
    set(PlatformLibs spirv-cross-core spirv-cross-glsl shaderc_shared vulkan.dylib)
endif()

if (DebugMode)
	if (MSVC)
		set(CompileOptions /DEBUG)
	else()
		set(CompileOptions -g)
	endif()
else()
	if (MSVC)
		# set(CompileOptions /O2)
		# TODO Fix on windows
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
	${CMAKE_CURRENT_SOURCE_DIR}/../External/Vulkan-Headers

	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

target_link_directories(${Target} PUBLIC ${LibDirs})
target_link_libraries(${Target} PUBLIC ${Libs})
target_compile_options(${Target} PUBLIC ${CompileOptions})