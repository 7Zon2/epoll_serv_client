cmake_minimum_required(VERSION 3.20)

project(epoll_client)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} INTERFACE flags)

add_executable(main_client  ${PROJECT_SOURCE_DIR}/src/main_client.cpp)

target_link_libraries(main_client PRIVATE ${PROJECT_NAME})

