cmake_minimum_required(VERSION 3.6)
project(WilliamNet)
message("now_pos_is: "${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/asio-1.18.1/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utility)
include_directories(D://william/work/opencvc++/opencv/build/include)
link_libraries(D://william/work/opencvc++/opencv/build/x64/vc15/lib)
add_subdirectory(${CMAKE_SOURCE_DIR}/ServerTest)
add_subdirectory(${CMAKE_SOURCE_DIR}/cppClient)

