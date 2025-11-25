cmake_minimum_required(VERSION 3.15)

project(Prism VERSION 1.0.0 LANGUAGES CXX)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 导出编译命令用于 IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 多配置生成器支持（如 Visual Studio）
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
endforeach()

# MSVC 专属设置
if(MSVC)
    # UTF-8 编码支持（防止中文乱码）
    add_compile_options(/utf-8)
    # 多核编译
    add_compile_options(/MP)
    # 禁用一些警告
    add_compile_options(/wd4251 /wd4819)
    # 设置警告级别
    add_compile_options(/W3)
    # 启用异常处理
    add_compile_options(/EHsc)
endif()

# GCC/Clang 专属设置
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Qt 设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找 Qt5
find_package(Qt5 5.14 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
)

# 高分屏支持
if(Qt5_FOUND AND Qt5Core_VERSION VERSION_GREATER_EQUAL 5.14)
    message(STATUS "Qt version: ${Qt5Core_VERSION}")
    add_definitions(-DQT_ENABLE_HIGHDPI_SCALING)
endif()

# 添加第三方库目录
add_subdirectory(3rdparty)

# 添加源代码目录
add_subdirectory(src)

# 复制 assets 到构建目录
add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/bin/assets
    COMMENT "Copying assets to build directory..."
)

# 设置启动项目（仅 Visual Studio）
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Prism)
endif()

# 打印配置信息
message(STATUS "========================================")
message(STATUS "Prism Configuration Summary")
message(STATUS "========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Qt Version: ${Qt5Core_VERSION}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "========================================")
