# Prism 主程序

# 收集源文件
set(PRISM_SOURCES
    main.cpp

    # Core
    core/ConfigParser.h
    core/ConfigParser.cpp
    core/IniParser.h
    core/IniParser.cpp
    core/JsonParser.h
    core/JsonParser.cpp
    core/YamlParser.h
    core/YamlParser.cpp
    core/ParserFactory.h
    core/ParserFactory.cpp
    core/ProcessRunner.h
    core/ProcessRunner.cpp
    core/ProjectManager.h
    core/ProjectManager.cpp

    # UI
    ui/MainWindow.h
    ui/MainWindow.cpp
    ui/HomePage.h
    ui/HomePage.cpp
    ui/BasePage.h
    ui/BasePage.cpp
    ui/ConfigPage.h
    ui/ConfigPage.cpp
    ui/ProcessPage.h
    ui/ProcessPage.cpp
    # Utils
    utils/EncodingUtils.h
    utils/EncodingUtils.cpp
)

# 创建可执行文件
add_executable(Prism ${PRISM_SOURCES})

# 链接 Qt 库
target_link_libraries(Prism PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
)

# 链接 ElaWidgetTools（如果存在）
if(TARGET ElaWidgetTools)
    target_link_libraries(Prism PRIVATE ElaWidgetTools)

    # 自动包含 ElaWidgetTools 的头文件目录
    # 这样 VS 的 IntelliSense 就能识别 Ela* 类
    get_target_property(ELA_INCLUDE_DIRS ElaWidgetTools INTERFACE_INCLUDE_DIRECTORIES)
    if(ELA_INCLUDE_DIRS)
        target_include_directories(Prism PRIVATE ${ELA_INCLUDE_DIRS})
        message(STATUS "ElaWidgetTools include directories: ${ELA_INCLUDE_DIRS}")
    else()
        # 如果没有 INTERFACE_INCLUDE_DIRECTORIES，手动添加常见路径
        if(EXISTS ${CMAKE_SOURCE_DIR}/3rdparty/ElaWidgetTools/src)
            target_include_directories(Prism PRIVATE
                ${CMAKE_SOURCE_DIR}/3rdparty/ElaWidgetTools/src
            )
            message(STATUS "Manually added ElaWidgetTools include: 3rdparty/ElaWidgetTools/src")
        endif()
    endif()

    message(STATUS "Linking ElaWidgetTools to Prism")
endif()

# 链接 yaml-cpp（如果存在）
if(TARGET yaml-cpp)
    target_link_libraries(Prism PRIVATE yaml-cpp)

    # 自动包含 yaml-cpp 的头文件目录
    get_target_property(YAML_INCLUDE_DIRS yaml-cpp INTERFACE_INCLUDE_DIRECTORIES)
    if(YAML_INCLUDE_DIRS)
        target_include_directories(Prism PRIVATE ${YAML_INCLUDE_DIRS})
        message(STATUS "yaml-cpp include directories: ${YAML_INCLUDE_DIRS}")
    else()
        # 手动添加
        if(EXISTS ${CMAKE_SOURCE_DIR}/3rdparty/yaml-cpp/include)
            target_include_directories(Prism PRIVATE
                ${CMAKE_SOURCE_DIR}/3rdparty/yaml-cpp/include
            )
            message(STATUS "Manually added yaml-cpp include: 3rdparty/yaml-cpp/include")
        endif()
    endif()

    # 定义宏，表示 yaml-cpp 可用
    target_compile_definitions(Prism PRIVATE YAML_CPP_AVAILABLE)

    message(STATUS "Linking yaml-cpp to Prism")
endif()

# 包含目录
target_include_directories(Prism PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# Windows 专属设置
if(WIN32)
    # 设置为 Windows 应用程序（不显示控制台）
    # 如需调试输出，可改为 Console
    set_target_properties(Prism PROPERTIES
        WIN32_EXECUTABLE ON
    )

    # 添加应用图标（如果有）
    if(EXISTS ${CMAKE_SOURCE_DIR}/assets/icons/app.ico)
        target_sources(Prism PRIVATE ${CMAKE_SOURCE_DIR}/assets/icons/app.rc)
    endif()

    # ========================================
    # 自动部署 Qt 依赖（编译后自动执行）
    # ========================================
    # 1. 查找 windeployqt.exe
    find_program(WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt
        PATHS "${Qt5_DIR}/../../../bin"
        NO_DEFAULT_PATH
    )

    # 2. 检查是否找到
    if(NOT WINDEPLOYQT_EXECUTABLE)
        message(WARNING "windeployqt.exe not found! Qt dependencies will not be deployed automatically.")
        message(WARNING "Current Qt5_DIR: ${Qt5_DIR}")
    else()
        message(STATUS "Found windeployqt: ${WINDEPLOYQT_EXECUTABLE}")

        # 3. 添加编译后命令：自动部署 Qt 依赖
        add_custom_command(TARGET Prism POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE}
                --verbose 1
                "$<TARGET_FILE:Prism>"
            COMMENT "Deploying Qt dependencies..."
        )
    endif()
endif()

# Linux 专属设置
if(UNIX AND NOT APPLE)
    # 链接必要的系统库
    target_link_libraries(Prism PRIVATE pthread)
endif()

# 安装规则
install(TARGETS Prism
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装 assets
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets
    DESTINATION bin
)
