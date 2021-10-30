set(Target Triangle)
project(${Target})

add_executable(${Target} main.cpp)
target_link_libraries(${Target} PUBLIC Fluent)

if (UNIX)
	set(CompileOptions -g)
endif()

target_compile_options(${Target} PUBLIC ${CompileOptions})