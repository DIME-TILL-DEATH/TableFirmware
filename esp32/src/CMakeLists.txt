# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources 
    ${CMAKE_SOURCE_DIR}/src/*.*
    ${CMAKE_SOURCE_DIR}/net/*.*)

idf_component_register(SRCS ${app_sources})


component_compile_options(-Wno-error=format= -Wno-format)
