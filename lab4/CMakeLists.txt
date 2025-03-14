cmake_minimum_required(VERSION 3.16)
project(lab04)
set(CMAKE_CXX_STANDARD 14)
# 使用 cmake 的 flex 模块
include(FindFLEX)
if(FLEX_FOUND)
message("Info: flex found!")
else()
message("Error: flex not found!")
endif()
# 为了 flex 新增头文件搜索路径
include_directories(${CMAKE_SOURCE_DIR})
# 指定 flex 编译目标
FLEX_TARGET(MyScanner lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
add_compile_options(-pedantic)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)
add_executable(Compilerlab4
main.cpp
token.cpp
generate.cpp
postfix.cpp
${FLEX_MyScanner_OUTPUTS}
)
target_compile_features(Compilerlab4 PRIVATE cxx_std_14)