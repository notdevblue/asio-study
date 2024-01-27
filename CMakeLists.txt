cmake_minimum_required(VERSION 3.27)

project(
    asio_study
    VERSION 0.1
    DESCRIPTION "공부 프로젝트"
    LANGUAGES CXX
)

add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/NetClient/SimpleClient.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/NetCommon)