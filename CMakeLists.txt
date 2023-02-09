cmake_minimum_required(VERSION 3.0)
project(cpp-bekend CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCE "src/*.cpp" "src/*.h")
file(GLOB_RECURSE ASSETS
	"static/*.jpg"
	"static/*.png"
	"static/*.gif"
	"static/*.svg"
	"static/*.html"
	"static/*.css"
	"static/*.js"
)

add_executable(${PROJECT_NAME} ${SOURCE} ${ASSETS})

target_include_directories(${PROJECT_NAME} PRIVATE src)
