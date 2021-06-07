project(ServerTest)
set(CMAKE_CXX_STANDARD 17)
add_executable(ServerTest server_main.cpp WebSocketCommon.hpp ServerEndPoint.hpp metaConnection.hpp)
