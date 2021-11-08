set(Target 07_Compute)
project(${Target})

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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Examples/${Target})

add_executable(${Target} main.cpp)
target_link_libraries(${Target} PUBLIC Fluent)
target_compile_options(${Target} PUBLIC ${CompileOptions})