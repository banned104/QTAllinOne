cmake_minimum_required(VERSION 3.16)

project(QMLSQLite VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick OpenGL)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appQMLSQLite
    main.cpp
)

qt_add_qml_module(appQMLSQLite
    URI QMLSQLite
    VERSION 1.0
    SOURCES
        src/OpenGL/opengl_item.cpp src/OpenGL/opengl_item.hpp
        src/OpenGL/render.cpp src/OpenGL/render.hpp
        src/CPP/VirtualFunctionTest.hpp
        src/CPP/FactoryTest.hpp
        src/CPP/VirtualInherit.hpp
    QML_FILES
        Main.qml
        src/QML_Files/Buttons/ThreeDSwitch.qml
    RESOURCES
        qml_files.qrc

)


target_include_directories( appQMLSQLite PRIVATE
                        HuskarUI/include
                        src/CPP/
                    )
# if (MSVC)
# target_link_directories(appQMLSQLite PRIVATE "D:/MySoftwares/QTCreator17/6.9.3/msvc2022_64/lib")
# else()
# # D:\MySoftwares\QTCreator17\6.9.3\mingw_64
# target_link_directories(appQMLSQLite PRIVATE "D:/MySoftwares/QTCreator17/6.9.3/mingw_64/lib")
# endif()

# target_link_libraries(appQMLSQLite PRIVATE HuskarUIBasicd)      # d 后缀 为Debug版本
set(QML_IMPORT_PATH "${CMAKE_SOURCE_DIR}/HuskarUI/plugins" CACHE STRING "" FORCE)
target_link_directories( appQMLSQLite PRIVATE HuskarUI/lib/bin)
target_link_libraries(appQMLSQLite PRIVATE HuskarUIBasic)      # d 后缀 为Debug版本

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appQMLSQLite PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appQMLSQLite
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appQMLSQLite
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appQMLSQLite
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ✅ 关键修复：部署 HuskarUI 插件到构建目录
# 将 HuskarUI/plugins 目录复制到可执行文件所在目录
add_custom_command(TARGET appQMLSQLite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/HuskarUI/plugins
        $<TARGET_FILE_DIR:appQMLSQLite>/HuskarUI/plugins
    COMMENT "Copying HuskarUI plugins to build directory"
)

# 将 HuskarUI 动态库复制到可执行文件所在目录
add_custom_command(TARGET appQMLSQLite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/HuskarUI/lib/bin
        $<TARGET_FILE_DIR:appQMLSQLite>
    COMMENT "Copying HuskarUI DLLs to build directory"
)

