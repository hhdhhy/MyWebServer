
cmake_minimum_required (VERSION 3.16)

project(server_test)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
# set(CMAKE_CXX_FLAGS "-O3")  
# set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG") 
# set(CMAKE_CXX_FLAGS_DEBUG "-O3 -g")

# set(CMAKE_C_FLAGS "-O3")  
# set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG") 
# set(CMAKE_C_FLAGS_DEBUG "-O3 -g")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

aux_source_directory(src SCR_LIST)
aux_source_directory(example EXAM_LIST)

include_directories(include)

# 合并两个源文件列表
list(APPEND ALL_SOURCES ${SCR_LIST} ${EXAM_LIST})
add_executable(server_test ${ALL_SOURCES})

find_package(Threads REQUIRED)
target_link_libraries(server_test  PRIVATE Threads::Threads)

