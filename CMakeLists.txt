cmake_minimum_required(VERSION 3.0)
project(cpp-bekend CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCE "src/*.cpp" "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCE})