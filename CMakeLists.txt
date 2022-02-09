cmake_minimum_required(VERSION 3.5)

project(PWSS-TPK-Example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(TPK SHARED
	sockbase.hpp sockbase.cpp
	server.hpp server.cpp
	client.hpp client.cpp)

add_executable(TPK_serwer main_s.cpp)
add_executable(TPK_klient main_c.cpp)

target_compile_definitions(TPK PUBLIC TPK)
target_link_libraries(TPK_serwer PUBLIC TPK)
target_link_libraries(TPK_klient PUBLIC TPK)
