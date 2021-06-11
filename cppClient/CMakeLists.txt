cmake_minimum_required(VERSION 3.6)
project(hk_interface)

set(CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(hk_interface main.cpp)
include_directories(D://william/work/opencvc++/opencv/build/include)
link_libraries(D://william/work/opencvc++/opencv/build/x64/vc15/lib)
add_compile_definitions(ASIO_STANDALONE _WEBSOCKETPP_CPP11_RANDOM_DEVICE_ _WEBSOCKETPP_CPP11_TYPE_TRAITS_)

target_link_libraries(hk_interface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dll/HCCore.lib
                                   ${CMAKE_CURRENT_SOURCE_DIR}/dll/HCNetSDK.lib
                                   ${CMAKE_CURRENT_SOURCE_DIR}/dll/PlayCtrl.lib 
                                   D://william/work/opencvc++/opencv/build/x64/vc15/lib/opencv_world451d.lib)
message(STATUS "end...")