cmake_minimum_required(VERSION 3.20)

project(epoll_serv_client)

add_library(flags INTERFACE)

target_compile_features(flags INTERFACE cxx_std_20)

target_compile_options(flags INTERFACE -Wall;-Wextra)

target_include_directories(flags INTERFACE ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(server)
add_subdirectory(client)