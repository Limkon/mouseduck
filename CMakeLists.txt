cmake_minimum_required(VERSION 3.10)

project(SysTool)

set(CMAKE_C_STANDARD 99)

# 隐藏控制台黑框 (使用 Windows 子系统)
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
elseif(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif()

# 包含 include 目录，使编译器能找到头文件
include_directories(${CMAKE_SOURCE_DIR}/include)

# 收集所有源文件与资源文件，指定相对路径
set(SOURCES
    src/main.c
    src/action.c
    src/worker.c
    resources/resource.rc
)

# 生成可执行文件
add_executable(SysRunner ${SOURCES})

# 链接 Windows API 库
target_link_libraries(SysRunner User32.lib)
