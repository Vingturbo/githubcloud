cmake_minimum_required(VERSION 3.20)
project(ghcd VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 1. 收集源码
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.c"
)

# 1.1 排除 FLTK 子模块，防止重复编译
list(FILTER SOURCES EXCLUDE REGEX "/src/lib/fltk/")

# 2. 配置 FLTK 构建选项（必须在 add_subdirectory 之前）
set(FLTK_BUILD_TEST         OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_EXAMPLES     OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_FLUID        OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_FLTK_OPTIONS OFF CACHE BOOL "" FORCE)

# 可选：静态链接 FLTK
set(FLTK_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# 3. 把 FLTK 源码树作为子项目接入
add_subdirectory(src/lib/fltk EXCLUDE_FROM_ALL)

# 4. 定义可执行文件目标
add_executable(ghcd ${SOURCES})

# 5. 链接 FLTK
target_link_libraries(ghcd PRIVATE fltk)

# 6. 优化选项
if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" OR CMAKE_BUILD_TYPE STREQUAL "Release")

    target_compile_options(ghcd PRIVATE
        -Os
        -ffunction-sections
        -fdata-sections
        -fvisibility=hidden
    )

    target_link_options(ghcd PRIVATE
        -Wl,--gc-sections
        -s
    )
endif()