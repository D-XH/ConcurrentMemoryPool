
cmake_minimum_required(VERSION 3.10)

project(memory_pool CXX)

set(root ${CMAKE_SOURCE_DIR})
set(src_dir ${root}/src)
set(inc_dir ${root}/inc)
# set(test_file ${root}/test/test.cc)
# set(test_file ${root}/test/benchmark.cc)
set(test_file ${root}/test/unitTest.cc)

include_directories(${inc_dir})
aux_source_directory(${src_dir} src_list)

add_executable(test ${test_file} ${src_list})
target_link_libraries(test pthread)
target_compile_options(test PRIVATE -g)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")