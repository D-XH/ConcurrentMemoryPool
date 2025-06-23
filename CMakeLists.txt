
cmake_minimum_required(VERSION 3.10)

project(memory_pool CXX)

set(root ${CMAKE_SOURCE_DIR})
set(test_dir ${root}/test)
set(src_dir ${root}/src)
set(inc_dir ${root}/inc)

include_directories(${inc_dir})
aux_source_directory(${src_dir} src_list)
aux_source_directory(${test_dir} test_file)

add_executable(test ${test_file} ${src_list})