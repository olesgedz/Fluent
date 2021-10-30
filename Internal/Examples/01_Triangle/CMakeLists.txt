set(Target Triangle)
project(${Target})

add_executable(${Target} main.cpp)
target_link_libraries(${Target} PUBLIC Fluent)
target_compile_options(${Target} PUBLIC -g)