cmake_minimum_required(VERSION 3.20)

# 因为以后会用 .c 的依赖，所以把 C 加上
project(ghcd VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 1. 自动递归扫描 src 及其子目录下的所有 .c 和 .cpp 文件
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.c"
)

# 2. 把根目录的 main.cpp 加进去（假设 main.cpp 在根目录）
add_executable(ghcd
    ${SOURCES}
)