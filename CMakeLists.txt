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
	"static/*.css"
	"static/*.js"
	"templates/*.html"
)

add_executable(${PROJECT_NAME} ${SOURCE} ${ASSETS})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PUBLIC vendor/rapidjson/include)
target_link_libraries(${PROJECT_NAME} Ws2_32)
