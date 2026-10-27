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

# 2. 定义可执行文件目标 (必须先有目标！)
add_executable(ghcd ${SOURCES})

# 4. 针对 Release 或 MinSizeRel 模式应用极致优化
if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" OR CMAKE_BUILD_TYPE STREQUAL "Release")
    
    # 编译选项
    target_compile_options(ghcd PRIVATE
        -Os                      # 优化大小
        -ffunction-sections      # 函数分段
        -fdata-sections          # 数据分段
        # -fno-rtti                # 禁用 RTTI（你目前没用到，可以开）
        # -fno-exceptions          # 禁用异常（你目前没用到，可以开。但注意 json 库可能依赖异常）
        -fvisibility=hidden      # 隐藏内部符号
    )

    # 链接选项
    target_link_options(ghcd PRIVATE
        -Wl,--gc-sections        # 链接器开启垃圾回收，剔除无用代码
        -s                       # 链接时去除符号表
        # -flto                    # 启用链接时优化（如果遇到报错可以先注释掉）
    )
endif()