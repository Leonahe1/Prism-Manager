# 第三方库集成
# 说明：请将以下库放入对应目录：
# - ElaWidgetTools -> 3rdparty/ElaWidgetTools
# - yaml-cpp -> 3rdparty/yaml-cpp

# ElaWidgetTools (https://github.com/Liniyous/ElaWidgetTools)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ElaWidgetTools/CMakeLists.txt)
    message(STATUS "Found ElaWidgetTools")
    add_subdirectory(ElaWidgetTools)
else()
    message(WARNING "ElaWidgetTools not found in 3rdparty/ElaWidgetTools")
    message(WARNING "Please clone: git clone https://github.com/Liniyous/ElaWidgetTools.git")
endif()

# yaml-cpp
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp/CMakeLists.txt)
    message(STATUS "Found yaml-cpp")
    # 禁用 yaml-cpp 的测试和工具
    set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
    add_subdirectory(yaml-cpp)
else()
    message(WARNING "yaml-cpp not found in 3rdparty/yaml-cpp")
    message(WARNING "Please clone: git clone https://github.com/jbeder/yaml-cpp.git")
endif()
