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
if(TARGET elawidgettools)
    target_link_libraries(Prism PRIVATE elawidgettools)
    message(STATUS "Linking ElaWidgetTools to Prism")
endif()

# 链接 yaml-cpp（如果存在）
if(TARGET yaml-cpp)
    target_link_libraries(Prism PRIVATE yaml-cpp)
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
