set(Target Triangle)
project(${Target})

if (UNIX)
	set(CompileOptions -g)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Examples/${Target})

add_executable(${Target} main.cpp)
target_link_libraries(${Target} PUBLIC Fluent)
target_compile_options(${Target} PUBLIC ${CompileOptions})