cmake_minimum_required(VERSION 3.20)

project(epoll_serv)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} INTERFACE flags)

add_executable(main_serv  ${PROJECT_SOURCE_DIR}/src/main_server.cpp)

target_link_libraries(main_serv PRIVATE ${PROJECT_NAME})

