# Qt6 QML 资源管理完全指南

> 本文档详细介绍 Qt6 中 QML 资源系统的使用方法，包括 .qrc 文件、qmldir 文件、CMake 配置等核心概念。

## 目录

1. [Qt6 资源系统概述](#1-qt6-资源系统概述)
2. [.qrc 文件详解](#2-qrc-文件详解)
3. [qmldir 文件详解](#3-qmldir-文件详解)
4. [Qt6 CMake QML 模块系统](#4-qt6-cmake-qml-模块系统)
5. [Import Path 机制](#5-import-path-机制)
6. [实战示例](#6-实战示例)
7. [常见问题与解决方案](#7-常见问题与解决方案)

---

## 1. Qt6 资源系统概述

### 1.1 什么是 Qt 资源系统？

Qt 资源系统（Qt Resource System）允许将文件（QML、图片、字体等）编译到可执行文件中，使用 `qrc:/` 或 `:` 前缀访问。

### 1.2 Qt6 的重大变化

| 特性 | Qt5 | Qt6 |
|------|-----|-----|
| QML 模块定义 | 手动编写 .qrc | 使用 `qt_add_qml_module()` |
| qmldir 生成 | 手动创建 | CMake 自动生成 |
| 资源前缀 | 自定义 | 标准化为 `/qt/qml/<ModuleName>` |
| 类型注册 | `qmlRegisterType()` | 自动注册（通过 CMake） |


---

## 2. .qrc 文件详解

### 2.1 .qrc 文件基本结构

.qrc（Qt Resource Collection）文件是 XML 格式，定义哪些文件应该被编译到应用程序中。

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <!-- qresource 定义一个资源组，prefix 是访问前缀 -->
    <qresource prefix="/images">
        <!-- file 标签指定要包含的文件 -->
        <file>logo.png</file>
        <file>background.jpg</file>
        
        <!-- alias 属性可以给文件起别名 -->
        <file alias="icon.png">assets/app_icon.png</file>
    </qresource>
    
    <qresource prefix="/qml">
        <file>Main.qml</file>
        <file>components/Button.qml</file>
    </qresource>
</RCC>
```

### 2.2 .qrc 文件关键元素

#### 2.2.1 `<RCC>` 根元素
```xml
<RCC version="1.0">
    <!-- version 属性指定 RCC 格式版本，通常为 1.0 -->
</RCC>
```

#### 2.2.2 `<qresource>` 资源组
```xml
<qresource prefix="/myprefix" lang="zh_CN">
    <!-- prefix: 资源访问前缀，必须以 / 开头 -->
    <!-- lang: 可选，指定语言/地区（用于国际化） -->
</qresource>
```

#### 2.2.3 `<file>` 文件条目
```xml
<!-- 基本用法：相对于 .qrc 文件的路径 -->
<file>image.png</file>

<!-- 使用 alias：在资源系统中使用不同的名称 -->
<file alias="myimage.png">resources/images/image.png</file>

<!-- 绝对路径（不推荐） -->
<file>D:/Projects/MyApp/image.png</file>
```


### 2.3 .qrc 文件完整示例

#### 示例 1：多媒体资源管理
```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <!-- 图片资源 -->
    <qresource prefix="/images">
        <file alias="logo">assets/company_logo.png</file>
        <file>icons/home.svg</file>
        <file>icons/settings.svg</file>
        <file>icons/user.svg</file>
    </qresource>
    
    <!-- 字体资源 -->
    <qresource prefix="/fonts">
        <file>fonts/Roboto-Regular.ttf</file>
        <file>fonts/Roboto-Bold.ttf</file>
    </qresource>
    
    <!-- QML 文件 -->
    <qresource prefix="/qml">
        <file>qml/Main.qml</file>
        <file>qml/pages/HomePage.qml</file>
        <file>qml/pages/SettingsPage.qml</file>
        <file>qml/components/CustomButton.qml</file>
    </qresource>
    
    <!-- 配置文件 -->
    <qresource prefix="/config">
        <file>config/app_settings.json</file>
        <file>config/themes.json</file>
    </qresource>
</RCC>
```

#### 示例 2：国际化资源
```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <!-- 中文资源 -->
    <qresource prefix="/i18n" lang="zh_CN">
        <file alias="strings.json">translations/strings_zh_CN.json</file>
        <file alias="help.html">help/help_zh_CN.html</file>
    </qresource>
    
    <!-- 英文资源 -->
    <qresource prefix="/i18n" lang="en_US">
        <file alias="strings.json">translations/strings_en_US.json</file>
        <file alias="help.html">help/help_en_US.html</file>
    </qresource>
    
    <!-- 默认资源（无 lang 属性） -->
    <qresource prefix="/i18n">
        <file alias="strings.json">translations/strings_en_US.json</file>
    </qresource>
</RCC>
```


### 2.4 在代码中访问 .qrc 资源

#### 2.4.1 C++ 中访问资源
```cpp
#include <QFile>
#include <QPixmap>
#include <QFontDatabase>

// 读取文本文件
QFile file(":/config/app_settings.json");
if (file.open(QIODevice::ReadOnly)) {
    QByteArray data = file.readAll();
    // 处理数据...
}

// 加载图片（两种前缀方式都可以）
QPixmap logo1(":/images/logo");           // 使用 :/ 前缀
QPixmap logo2("qrc:/images/logo");        // 使用 qrc:/ 前缀

// 加载字体
int fontId = QFontDatabase::addApplicationFont(":/fonts/Roboto-Regular.ttf");
if (fontId != -1) {
    QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont font(family);
}
```

#### 2.4.2 QML 中访问资源
```qml
import QtQuick

Item {
    // 加载图片
    Image {
        source: "qrc:/images/logo"        // 完整前缀
        // 或
        source: ":/images/logo"           // 简短前缀
    }
    
    // 加载字体
    FontLoader {
        id: customFont
        source: "qrc:/fonts/Roboto-Regular.ttf"
    }
    
    Text {
        font.family: customFont.name
        text: "Hello World"
    }
    
    // 加载其他 QML 文件
    Loader {
        source: "qrc:/qml/components/CustomButton.qml"
    }
}
```

### 2.5 .qrc 文件的编译过程

```
.qrc 文件 → rcc 工具 → .cpp 文件 → 编译器 → 可执行文件
```

1. **rcc（Resource Compiler）** 读取 .qrc 文件
2. 生成包含二进制数据的 C++ 源文件
3. 编译器将其编译到可执行文件中
4. 运行时通过 Qt 资源系统访问


---

## 3. qmldir 文件详解

### 3.1 什么是 qmldir？

`qmldir` 是 QML 模块的元数据文件，定义：
- 模块名称和版本
- 包含哪些 QML 类型
- C++ 插件信息
- 类型的文件位置

### 3.2 qmldir 文件基本结构

```qmldir
# 模块声明（必需）
module MyApp.Components

# 插件声明（如果有 C++ 插件）
plugin myappplugin

# 类名（C++ 插件的类名）
classname MyAppPlugin

# 设计器支持
designersupported

# 类型信息文件（用于 Qt Creator 代码补全）
typeinfo plugins.qmltypes

# 首选资源路径
prefer :/MyApp/Components/

# QML 类型声明
# 格式：TypeName Version.Subversion FilePath
Button 1.0 Button.qml
TextField 1.0 TextField.qml
CustomDialog 1.0 CustomDialog.qml

# 单例类型
singleton Theme 1.0 Theme.qml
singleton AppConfig 1.0 AppConfig.qml

# 内部类型（不导出，仅模块内部使用）
internal ButtonBackground ButtonBackground.qml

# JavaScript 资源
Utils 1.0 utils.js
```


### 3.3 qmldir 关键指令详解

#### 3.3.1 `module` - 模块声明
```qmldir
# 定义模块的完整名称（必需，且必须是第一行）
module MyCompany.MyApp.Components

# 使用时：
# import MyCompany.MyApp.Components 1.0
```

#### 3.3.2 `plugin` - C++ 插件
```qmldir
# 指定 C++ 插件的库名称（不含扩展名）
plugin myappplugin

# Windows: myappplugin.dll
# Linux: libmyappplugin.so
# macOS: libmyappplugin.dylib
```

#### 3.3.3 `classname` - 插件类名
```qmldir
# 指定实现 QQmlExtensionPlugin 的 C++ 类名
classname MyAppPlugin

# 对应 C++ 代码：
# class MyAppPlugin : public QQmlExtensionPlugin {
#     Q_OBJECT
#     Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
# };
```

#### 3.3.4 `typeinfo` - 类型信息
```qmldir
# 指定 .qmltypes 文件，用于 IDE 代码补全和类型检查
typeinfo plugins.qmltypes

# 生成 .qmltypes 文件：
# qmltyperegistrar --generate-qmltypes=plugins.qmltypes ...
```

#### 3.3.5 `prefer` - 首选路径
```qmldir
# 指定首选的资源路径前缀
prefer :/MyApp/Components/

# 当同时存在文件系统和资源系统中的文件时，优先使用资源系统
```

#### 3.3.6 `singleton` - 单例类型
```qmldir
# 声明单例类型（全局唯一实例）
singleton AppSettings 1.0 AppSettings.qml

# 使用时不需要实例化：
# import MyApp 1.0
# Text { text: AppSettings.userName }
```


#### 3.3.7 `internal` - 内部类型
```qmldir
# 声明内部类型（不导出到模块外部）
internal ButtonImpl ButtonImpl.qml

# 只能在模块内部使用，外部 import 后无法访问
```

#### 3.3.8 `optional` - 可选插件
```qmldir
# 标记插件为可选，加载失败不会报错
optional plugin myappplugin

# 用于平台特定的插件或可选功能
```

#### 3.3.9 `depends` - 模块依赖
```qmldir
# 声明依赖其他模块
depends QtQuick 2.15
depends QtQuick.Controls 2.15

# 确保依赖模块在当前模块之前加载
```

#### 3.3.10 `import` - 自动导入
```qmldir
# 自动导入其他模块
import QtQuick 2.15
import QtQuick.Controls 2.15

# 使用当前模块时，自动导入这些模块
```

### 3.4 qmldir 完整示例

#### 示例 1：纯 QML 组件库
```qmldir
# 模块名称
module MyApp.UI.Components

# 设计器支持
designersupported

# 首选资源路径
prefer :/MyApp/UI/Components/

# 单例：主题配置
singleton Theme 1.0 Theme.qml
singleton Icons 1.0 Icons.qml

# 基础组件
Button 1.0 Button.qml
TextField 1.0 TextField.qml
TextArea 1.0 TextArea.qml
CheckBox 1.0 CheckBox.qml
RadioButton 1.0 RadioButton.qml

# 复杂组件
Dialog 1.0 Dialog.qml
Drawer 1.0 Drawer.qml
TabBar 1.0 TabBar.qml

# 内部实现（不导出）
internal ButtonBackground ButtonBackground.qml
internal RippleEffect RippleEffect.qml

# JavaScript 工具
Utils 1.0 utils.js
Validator 1.0 validator.js
```


#### 示例 2：带 C++ 插件的模块（参考 HuskarUI）
```qmldir
# 模块名称
module HuskarUI.Basic

# C++ 插件配置
linktarget huskaruibasicplugin          # 链接目标名称
optional plugin huskaruibasicplugin     # 插件名称（可选加载）
classname HuskarUI_BasicPlugin          # 插件类名

# 设计器支持
designersupported

# 类型信息（用于 IDE）
typeinfo plugins.qmltypes

# 首选资源路径
prefer :/HuskarUI/Basic/

# QML 组件（从 imports 子目录加载）
HusButton 1.0 imports/HusButton.qml
HusWindow 1.0 imports/HusWindow.qml
HusInput 1.0 imports/HusInput.qml
HusCard 1.0 imports/HusCard.qml
HusMenu 1.0 imports/HusMenu.qml
HusDialog 1.0 imports/HusDialog.qml

# 更多组件...
```

#### 示例 3：多版本支持
```qmldir
module MyApp.Components

# 版本 1.0 的组件
Button 1.0 v1/Button.qml
TextField 1.0 v1/TextField.qml

# 版本 2.0 的组件（改进版）
Button 2.0 v2/Button.qml
TextField 2.0 v2/TextField.qml

# 版本 2.1 新增组件
DatePicker 2.1 v2/DatePicker.qml

# 使用时可以指定版本：
# import MyApp.Components 1.0  // 使用旧版本
# import MyApp.Components 2.1  // 使用新版本
```


### 3.5 qmldir 文件位置规则

```
项目结构示例：
MyApp/
├── CMakeLists.txt
├── main.cpp
├── Main.qml
└── components/
    ├── qmldir              ← qmldir 必须在模块根目录
    ├── Button.qml
    ├── TextField.qml
    └── imports/            ← 可选子目录
        └── Dialog.qml
```

**重要规则：**
1. qmldir 文件必须位于模块的根目录
2. 文件名必须是 `qmldir`（小写，无扩展名）
3. QML 文件路径相对于 qmldir 文件
4. 模块名称应与目录结构对应

### 3.6 qmldir 与 import 的关系

```qml
// 当你写：
import MyApp.Components 1.0

// Qt 会：
// 1. 在 Import Path 中查找 MyApp/Components/ 目录
// 2. 读取该目录下的 qmldir 文件
// 3. 根据 qmldir 加载对应的 QML 文件和插件
// 4. 注册所有声明的类型
```

**查找顺序：**
1. 资源系统（`qrc:/`）中的路径
2. `QQmlEngine::addImportPath()` 添加的路径
3. 应用程序目录
4. Qt 安装目录的 qml 文件夹


---

## 4. Qt6 CMake QML 模块系统

### 4.1 qt_add_qml_module() 详解

Qt6 引入了 `qt_add_qml_module()`，自动化处理 QML 模块的创建。

#### 4.1.1 基本语法
```cmake
qt_add_qml_module(target_name
    URI module.uri              # 模块 URI（必需）
    VERSION version             # 版本号（必需）
    QML_FILES file1.qml ...     # QML 文件列表
    RESOURCES res1.png ...      # 资源文件列表
    SOURCES cpp1.cpp ...        # C++ 源文件
    # ... 更多选项
)
```

#### 4.1.2 完整参数说明
```cmake
qt_add_qml_module(appMyApp
    # === 必需参数 ===
    URI MyApp                           # 模块 URI，对应 import 语句
    VERSION 1.0                         # 模块版本号
    
    # === QML 文件 ===
    QML_FILES                           # QML 文件列表
        Main.qml
        pages/HomePage.qml
        pages/SettingsPage.qml
        components/Button.qml
    
    # === 资源文件 ===
    RESOURCES                           # 非 QML 资源（图片、字体等）
        images/logo.png
        images/background.jpg
        fonts/Roboto.ttf
    
    # === C++ 源文件 ===
    SOURCES                             # C++ 实现文件
        myappbackend.cpp
        myappbackend.h
    
    # === 输出目录 ===
    OUTPUT_DIRECTORY                    # 生成文件的输出目录
        ${CMAKE_BINARY_DIR}/qml/MyApp
    
    # === 资源前缀 ===
    RESOURCE_PREFIX /qt/qml             # 资源系统中的前缀（默认）
    
    # === 导入路径 ===
    IMPORT_PATH                         # 额外的 QML 导入路径
        ${CMAKE_SOURCE_DIR}/qml
    
    # === 插件选项 ===
    NO_PLUGIN                           # 不生成 C++ 插件
    NO_GENERATE_PLUGIN_SOURCE           # 不自动生成插件源码
    
    # === 其他选项 ===
    NO_RESOURCE_TARGET_PATH             # 不使用目标路径作为资源路径
    NO_LINT                             # 禁用 QML 语法检查
    NO_CACHEGEN                         # 禁用 QML 缓存生成
    NO_GENERATE_QMLTYPES                # 不生成 .qmltypes 文件
    
    # === 依赖 ===
    DEPENDENCIES                        # 依赖的其他 QML 模块
        QtQuick
        QtQuick.Controls
)
```


### 4.2 CMake 完整示例

#### 示例 1：简单的 QML 应用
```cmake
cmake_minimum_required(VERSION 3.16)
project(MyQMLApp VERSION 1.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt6 包
find_package(Qt6 REQUIRED COMPONENTS Quick)

# Qt6 标准项目设置
qt_standard_project_setup()

# 创建可执行文件
qt_add_executable(appMyQMLApp
    main.cpp
)

# 添加 QML 模块
qt_add_qml_module(appMyQMLApp
    URI MyQMLApp                    # 模块名称
    VERSION 1.0                     # 版本号
    QML_FILES                       # QML 文件列表
        Main.qml
        pages/HomePage.qml
        pages/AboutPage.qml
    RESOURCES                       # 资源文件
        images/logo.png
        images/icon.png
)

# 链接 Qt 库
target_link_libraries(appMyQMLApp PRIVATE
    Qt6::Quick
)

# 设置可执行文件属性
set_target_properties(appMyQMLApp PROPERTIES
    WIN32_EXECUTABLE TRUE           # Windows 下不显示控制台
    MACOSX_BUNDLE TRUE              # macOS 下创建 Bundle
)

# 安装配置
install(TARGETS appMyQMLApp
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
```


#### 示例 2：多模块项目（参考本项目）
```cmake
cmake_minimum_required(VERSION 3.16)
project(QMLSQLite VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES 6.8)

# === 主应用程序 ===
qt_add_executable(appQMLSQLite
    main.cpp
)

# === 主 QML 模块 ===
qt_add_qml_module(appQMLSQLite
    URI QMLSQLite                   # 主模块 URI
    VERSION 1.0
    QML_FILES
        Main.qml                    # 主 QML 文件
)

# === 外部库配置（HuskarUI）===
# 包含头文件目录
target_include_directories(appQMLSQLite PRIVATE 
    HuskarUI/include
)

# 链接库目录
target_link_directories(appQMLSQLite PRIVATE 
    HuskarUI/lib/bin
)

# 链接库
target_link_libraries(appQMLSQLite PRIVATE 
    HuskarUIBasic                   # HuskarUI 库
    Qt6::Quick                      # Qt Quick 库
)

# === QML 导入路径配置 ===
# 设置 QML_IMPORT_PATH 供 Qt Creator 使用
set(QML_IMPORT_PATH 
    "${CMAKE_SOURCE_DIR}/HuskarUI/plugins" 
    CACHE STRING "" FORCE
)

# === 部署配置 ===
# 复制 HuskarUI 插件到构建目录
add_custom_command(TARGET appQMLSQLite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/HuskarUI/plugins
        $<TARGET_FILE_DIR:appQMLSQLite>/HuskarUI/plugins
    COMMENT "Copying HuskarUI plugins to build directory"
)

# 复制 HuskarUI DLL 到构建目录
add_custom_command(TARGET appQMLSQLite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/HuskarUI/lib/bin
        $<TARGET_FILE_DIR:appQMLSQLite>
    COMMENT "Copying HuskarUI DLLs to build directory"
)

# === 可执行文件属性 ===
set_target_properties(appQMLSQLite PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# === 安装配置 ===
include(GNUInstallDirs)
install(TARGETS appQMLSQLite
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
```


#### 示例 3：创建可复用的 QML 组件库
```cmake
cmake_minimum_required(VERSION 3.16)
project(MyUILibrary VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup()

# === 创建 QML 插件库 ===
qt_add_library(MyUILibrary STATIC)  # 或 SHARED

# === 添加 QML 模块 ===
qt_add_qml_module(MyUILibrary
    URI MyCompany.UI                # 模块 URI
    VERSION 1.0
    
    # QML 组件
    QML_FILES
        Button.qml
        TextField.qml
        Dialog.qml
        Theme.qml
    
    # 资源文件
    RESOURCES
        icons/check.svg
        icons/close.svg
    
    # C++ 后端（如果需要）
    SOURCES
        uibackend.cpp
        uibackend.h
    
    # 输出目录
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml/MyCompany/UI
)

# 链接 Qt 库
target_link_libraries(MyUILibrary PRIVATE
    Qt6::Quick
)

# === 安装配置 ===
# 安装库文件
install(TARGETS MyUILibrary
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# 安装 QML 文件和 qmldir
install(DIRECTORY ${CMAKE_BINARY_DIR}/qml/MyCompany/UI
    DESTINATION ${CMAKE_INSTALL_PREFIX}/qml/MyCompany
)
```

### 4.3 qt_add_qml_module() 自动生成的文件

当你调用 `qt_add_qml_module()` 时，CMake 会自动生成：

```
build/
├── QMLSQLite/
│   ├── qmldir                          # 自动生成的 qmldir 文件
│   └── QMLSQLite.qmltypes              # 类型信息文件
├── .qt/
│   └── rcc/
│       ├── qmake_QMLSQLite.qrc         # 自动生成的 .qrc 文件
│       └── qrc_qmake_QMLSQLite.cpp     # 编译后的资源 C++ 文件
└── appQMLSQLite.exe                    # 可执行文件
```


---

## 5. Import Path 机制

### 5.1 什么是 Import Path？

Import Path 是 QML 引擎查找模块的搜索路径列表。当你写 `import MyModule 1.0` 时，引擎会在这些路径中查找。

### 5.2 默认 Import Path

Qt 自动添加以下路径：

1. **资源系统路径**：`qrc:/qt/qml/`
2. **应用程序目录**：`QCoreApplication::applicationDirPath()`
3. **Qt 安装目录**：`<Qt安装路径>/qml/`
4. **环境变量**：`QML2_IMPORT_PATH` 环境变量指定的路径

### 5.3 在 C++ 中添加 Import Path

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    
    // 获取应用程序目录
    QString appDir = QCoreApplication::applicationDirPath();
    
    // ✅ 关键：必须在 load() 之前添加 Import Path
    
    // 方法 1：添加绝对路径
    engine.addImportPath(appDir + "/qml");
    engine.addImportPath(appDir + "/plugins");
    
    // 方法 2：添加相对路径（相对于应用程序目录）
    engine.addImportPath("./qml");
    
    // 方法 3：添加资源路径
    engine.addImportPath("qrc:/qml");
    
    // 方法 4：添加多个路径
    QStringList importPaths = {
        appDir + "/qml",
        appDir + "/plugins",
        "qrc:/qml"
    };
    for (const QString &path : importPaths) {
        engine.addImportPath(path);
    }
    
    // 调试：打印所有 Import Path
    qDebug() << "QML Import Paths:" << engine.importPathList();
    
    // 加载 QML（必须在 addImportPath 之后）
    engine.loadFromModule("MyApp", "Main");
    
    return app.exec();
}
```


### 5.4 Import Path 查找机制

当你写 `import MyCompany.MyApp.Components 1.0` 时：

```
1. 将模块名转换为路径：
   MyCompany.MyApp.Components → MyCompany/MyApp/Components

2. 在每个 Import Path 中查找：
   - /path1/MyCompany/MyApp/Components/qmldir
   - /path2/MyCompany/MyApp/Components/qmldir
   - qrc:/qt/qml/MyCompany/MyApp/Components/qmldir
   - ...

3. 找到 qmldir 后：
   - 读取模块配置
   - 加载 C++ 插件（如果有）
   - 注册 QML 类型
   - 加载依赖模块

4. 如果找不到：
   - 报错：module "MyCompany.MyApp.Components" is not installed
```

### 5.5 Import Path 最佳实践

#### ✅ 推荐做法
```cpp
// 1. 使用应用程序目录的绝对路径
QString appDir = QCoreApplication::applicationDirPath();
engine.addImportPath(appDir + "/qml");

// 2. 在加载 QML 之前添加
engine.addImportPath(...);  // 先添加路径
engine.load(...);           // 后加载 QML

// 3. 使用标准化的目录结构
// appDir/
// ├── app.exe
// └── qml/
//     └── MyCompany/
//         └── MyApp/
//             ├── qmldir
//             └── Main.qml
```

#### ❌ 避免的做法
```cpp
// 1. 不要在加载后添加路径（无效）
engine.load(...);           // ❌ 先加载
engine.addImportPath(...);  // ❌ 后添加（太晚了）

// 2. 不要使用相对路径 "./"（不可靠）
engine.addImportPath("./qml");  // ❌ 工作目录可能变化

// 3. 不要硬编码绝对路径
engine.addImportPath("C:/MyApp/qml");  // ❌ 不可移植
```


### 5.6 在 CMake 中配置 Import Path

```cmake
# 方法 1：设置 QML_IMPORT_PATH（供 Qt Creator 使用）
set(QML_IMPORT_PATH 
    "${CMAKE_SOURCE_DIR}/qml"
    "${CMAKE_SOURCE_DIR}/plugins"
    CACHE STRING "Qt Creator QML import paths" FORCE
)

# 方法 2：设置环境变量（运行时）
set_target_properties(appMyApp PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "QML2_IMPORT_PATH=${CMAKE_SOURCE_DIR}/qml"
)

# 方法 3：部署时复制到构建目录
add_custom_command(TARGET appMyApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/qml
        $<TARGET_FILE_DIR:appMyApp>/qml
)
```

### 5.7 调试 Import Path 问题

```cpp
#include <QQmlApplicationEngine>
#include <QDebug>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    
    // 启用 QML 调试输出
    qputenv("QML_IMPORT_TRACE", "1");
    
    // 打印所有 Import Path
    qDebug() << "=== QML Import Paths ===";
    for (const QString &path : engine.importPathList()) {
        qDebug() << "  -" << path;
    }
    
    // 添加自定义路径
    QString appDir = QCoreApplication::applicationDirPath();
    engine.addImportPath(appDir + "/qml");
    
    qDebug() << "=== After adding custom paths ===";
    for (const QString &path : engine.importPathList()) {
        qDebug() << "  -" << path;
    }
    
    // 加载 QML
    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));
    
    return app.exec();
}
```

**输出示例：**
```
=== QML Import Paths ===
  - qrc:/qt/qml
  - D:/MyApp/build/Release
  - D:/Qt/6.9.3/mingw_64/qml
=== After adding custom paths ===
  - D:/MyApp/build/Release/qml
  - qrc:/qt/qml
  - D:/MyApp/build/Release
  - D:/Qt/6.9.3/mingw_64/qml
```


---

## 6. 实战示例

### 6.1 示例 1：创建自定义 UI 组件库

#### 项目结构
```
MyUILibrary/
├── CMakeLists.txt
├── main.cpp
├── Main.qml
└── components/
    ├── Button.qml
    ├── TextField.qml
    └── Dialog.qml
```

#### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.16)
project(MyUILibrary VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup()

# 主应用程序
qt_add_executable(appMyUILibrary
    main.cpp
)

# 主模块
qt_add_qml_module(appMyUILibrary
    URI MyUILibrary
    VERSION 1.0
    QML_FILES Main.qml
)

# 组件库模块
qt_add_qml_module(ComponentsPlugin
    URI MyUILibrary.Components
    VERSION 1.0
    QML_FILES
        components/Button.qml
        components/TextField.qml
        components/Dialog.qml
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml/MyUILibrary/Components
)

target_link_libraries(appMyUILibrary PRIVATE
    Qt6::Quick
    ComponentsPluginplugin  # 链接组件库插件
)

set_target_properties(appMyUILibrary PROPERTIES
    WIN32_EXECUTABLE TRUE
)
```

#### main.cpp
```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    
    // 添加组件库路径
    QString appDir = QCoreApplication::applicationDirPath();
    engine.addImportPath(appDir + "/qml");
    
    // 加载主 QML
    engine.loadFromModule("MyUILibrary", "Main");
    
    return app.exec();
}
```

#### Main.qml
```qml
import QtQuick
import MyUILibrary.Components 1.0  // 导入自定义组件库

Window {
    width: 640
    height: 480
    visible: true
    title: "My UI Library Demo"
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        // 使用自定义组件
        Button {
            text: "Click Me"
            onClicked: dialog.open()
        }
        
        TextField {
            placeholderText: "Enter text..."
        }
    }
    
    Dialog {
        id: dialog
        title: "Hello"
        message: "This is a custom dialog!"
    }
}
```


### 6.2 示例 2：使用外部 QML 库（如 HuskarUI）

#### 项目结构
```
MyApp/
├── CMakeLists.txt
├── main.cpp
├── Main.qml
└── HuskarUI/                    # 外部库
    ├── include/
    │   └── husapp.h
    ├── lib/
    │   └── bin/
    │       └── HuskarUIBasic.dll
    └── plugins/
        └── HuskarUI/
            └── Basic/
                ├── qmldir
                ├── huskaruibasicplugin.dll
                └── imports/
                    ├── HusButton.qml
                    └── HusWindow.qml
```

#### CMakeLists.txt（完整配置）
```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup()

# === 创建可执行文件 ===
qt_add_executable(appMyApp
    main.cpp
)

# === 添加主 QML 模块 ===
qt_add_qml_module(appMyApp
    URI MyApp
    VERSION 1.0
    QML_FILES Main.qml
)

# === 配置 HuskarUI 库 ===
# 包含头文件
target_include_directories(appMyApp PRIVATE
    HuskarUI/include
)

# 链接库目录
target_link_directories(appMyApp PRIVATE
    HuskarUI/lib/bin
)

# 链接库
target_link_libraries(appMyApp PRIVATE
    HuskarUIBasic
    Qt6::Quick
)

# === 设置 QML 导入路径（供 Qt Creator 使用）===
set(QML_IMPORT_PATH
    "${CMAKE_SOURCE_DIR}/HuskarUI/plugins"
    CACHE STRING "" FORCE
)

# === 部署 HuskarUI 到构建目录 ===
# 复制插件
add_custom_command(TARGET appMyApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/HuskarUI/plugins
        $<TARGET_FILE_DIR:appMyApp>/HuskarUI/plugins
    COMMENT "Deploying HuskarUI plugins"
)

# 复制 DLL
add_custom_command(TARGET appMyApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/HuskarUI/lib/bin
        $<TARGET_FILE_DIR:appMyApp>
    COMMENT "Deploying HuskarUI DLLs"
)

# === 可执行文件属性 ===
set_target_properties(appMyApp PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
```


#### main.cpp（正确的加载顺序）
```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QDebug>
#include "HuskarUI/include/husapp.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // 设置 OpenGL（可选）
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QQuickWindow::setDefaultAlphaBuffer(true);
    
    QQmlApplicationEngine engine;
    
    // 初始化 HuskarUI
    HusApp::initialize(&engine);
    
    // ✅ 关键步骤 1：获取应用程序目录
    QString appDir = QCoreApplication::applicationDirPath();
    qDebug() << "Application Directory:" << appDir;
    
    // ✅ 关键步骤 2：添加 Import Path（必须在 load 之前）
    engine.addImportPath(appDir + "/HuskarUI/plugins");
    engine.addImportPath(appDir);
    
    // 调试：打印所有 Import Path
    qDebug() << "QML Import Paths:";
    for (const QString &path : engine.importPathList()) {
        qDebug() << "  -" << path;
    }
    
    // 连接错误处理
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { 
            qCritical() << "Failed to create QML object!";
            QCoreApplication::exit(-1); 
        },
        Qt::QueuedConnection
    );
    
    // ✅ 关键步骤 3：加载 QML（必须在 addImportPath 之后）
    engine.loadFromModule("MyApp", "Main");
    
    return app.exec();
}
```

#### Main.qml
```qml
import QtQuick
import HuskarUI.Basic  // 导入 HuskarUI 组件

HusWindow {
    width: 800
    height: 600
    visible: true
    title: "HuskarUI Demo"
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        HusButton {
            text: "Click Me"
            width: 200
            height: 50
            onClicked: {
                console.log("Button clicked!")
            }
        }
        
        HusInput {
            width: 300
            height: 40
            placeholderText: "Enter text..."
        }
        
        HusCard {
            width: 400
            height: 200
            
            Text {
                anchors.centerIn: parent
                text: "This is a HuskarUI Card"
                font.pixelSize: 16
            }
        }
    }
}
```


### 6.3 示例 3：创建单例配置模块

#### 项目结构
```
MyApp/
├── CMakeLists.txt
├── main.cpp
├── Main.qml
└── config/
    ├── AppConfig.qml      # 单例配置
    └── qmldir             # 手动创建
```

#### config/qmldir
```qmldir
module MyApp.Config

# 声明单例类型
singleton AppConfig 1.0 AppConfig.qml
```

#### config/AppConfig.qml
```qml
pragma Singleton
import QtQuick

QtObject {
    // 应用程序配置
    readonly property string appName: "My Application"
    readonly property string version: "1.0.0"
    
    // 主题配置
    readonly property color primaryColor: "#2196F3"
    readonly property color accentColor: "#FF5722"
    readonly property color backgroundColor: "#FFFFFF"
    
    // 字体配置
    readonly property int fontSize: 14
    readonly property string fontFamily: "Arial"
    
    // API 配置
    readonly property string apiBaseUrl: "https://api.example.com"
    readonly property int apiTimeout: 30000
    
    // 功能函数
    function formatDate(date) {
        return Qt.formatDateTime(date, "yyyy-MM-dd")
    }
    
    function log(message) {
        console.log("[" + appName + "]", message)
    }
}
```

#### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup()

qt_add_executable(appMyApp main.cpp)

# 主模块
qt_add_qml_module(appMyApp
    URI MyApp
    VERSION 1.0
    QML_FILES Main.qml
)

# 配置模块（包含单例）
qt_add_qml_module(ConfigModule
    URI MyApp.Config
    VERSION 1.0
    QML_FILES
        config/AppConfig.qml
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml/MyApp/Config
)

target_link_libraries(appMyApp PRIVATE
    Qt6::Quick
    ConfigModuleplugin
)

set_target_properties(appMyApp PROPERTIES
    WIN32_EXECUTABLE TRUE
)
```

#### Main.qml（使用单例）
```qml
import QtQuick
import MyApp.Config  // 导入配置模块

Window {
    width: 640
    height: 480
    visible: true
    title: AppConfig.appName + " v" + AppConfig.version  // 使用单例
    
    Rectangle {
        anchors.fill: parent
        color: AppConfig.backgroundColor  // 使用单例配置
        
        Column {
            anchors.centerIn: parent
            spacing: 20
            
            Text {
                text: "Welcome to " + AppConfig.appName
                font.pixelSize: AppConfig.fontSize * 2
                font.family: AppConfig.fontFamily
                color: AppConfig.primaryColor
            }
            
            Rectangle {
                width: 200
                height: 50
                color: AppConfig.accentColor
                radius: 5
                
                Text {
                    anchors.centerIn: parent
                    text: "Accent Color"
                    color: "white"
                }
            }
            
            Text {
                text: "API: " + AppConfig.apiBaseUrl
                font.pixelSize: AppConfig.fontSize
            }
            
            Component.onCompleted: {
                // 使用单例的函数
                AppConfig.log("Application started")
                console.log("Today:", AppConfig.formatDate(new Date()))
            }
        }
    }
}
```


---

## 7. 常见问题与解决方案

### 7.1 "module is not installed" 错误

**错误信息：**
```
module "MyModule" is not installed
```

**原因：**
- QML 引擎找不到模块的 qmldir 文件

**解决方案：**
```cpp
// 1. 检查 Import Path
qDebug() << engine.importPathList();

// 2. 确保在 load 之前添加路径
engine.addImportPath(appDir + "/qml");  // 先添加
engine.load(...);                        // 后加载

// 3. 检查目录结构
// qml/
// └── MyModule/
//     ├── qmldir          ← 必须存在
//     └── Component.qml
```

### 7.2 "Type is not a type" 错误

**错误信息：**
```
HusWindow is not a type
```

**原因：**
1. qmldir 文件中没有声明该类型
2. QML 文件路径不正确
3. Import Path 配置错误

**解决方案：**
```qmldir
# 检查 qmldir 文件
module HuskarUI.Basic

# 确保类型已声明
HusWindow 1.0 imports/HusWindow.qml  # ← 路径必须正确

# 检查文件是否存在
# HuskarUI/Basic/imports/HusWindow.qml
```

### 7.3 运行时找不到插件 DLL

**错误信息：**
```
Cannot load library: The specified module could not be found.
```

**原因：**
- 插件 DLL 不在可执行文件目录或系统路径中

**解决方案：**
```cmake
# 在 CMakeLists.txt 中添加部署命令
add_custom_command(TARGET appMyApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/plugins
        $<TARGET_FILE_DIR:appMyApp>/plugins
    COMMENT "Copying plugins to build directory"
)

# 或复制 DLL 到可执行文件目录
add_custom_command(TARGET appMyApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/lib/myplugin.dll
        $<TARGET_FILE_DIR:appMyApp>
)
```


### 7.4 Qt Creator 无法识别自定义类型

**问题：**
- 代码可以运行，但 Qt Creator 显示红色波浪线
- 没有代码补全

**解决方案：**
```cmake
# 在 CMakeLists.txt 中设置 QML_IMPORT_PATH
set(QML_IMPORT_PATH
    "${CMAKE_SOURCE_DIR}/qml"
    "${CMAKE_SOURCE_DIR}/plugins"
    CACHE STRING "" FORCE
)

# 或在 .pro 文件中（qmake）
QML_IMPORT_PATH += $$PWD/qml
QML_IMPORT_PATH += $$PWD/plugins
```

**手动配置：**
1. 打开 Qt Creator
2. 工具 → 选项 → Qt Quick → QML/JS 编辑
3. 添加 Import Path

### 7.5 资源文件过大导致编译慢

**问题：**
- .qrc 包含大量文件，编译时间长
- 生成的可执行文件过大

**解决方案 1：分离资源**
```cmake
# 将资源分成多个 .qrc 文件
qt_add_resources(appMyApp "images"
    PREFIX "/images"
    FILES
        images/logo.png
        images/icon.png
)

qt_add_resources(appMyApp "qml"
    PREFIX "/qml"
    FILES
        qml/Main.qml
        qml/Page1.qml
)
```

**解决方案 2：使用外部资源**
```cmake
# 不编译到可执行文件，而是部署到文件系统
add_custom_command(TARGET appMyApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:appMyApp>/resources
)
```

**解决方案 3：使用 QML 缓存**
```cmake
# 启用 QML 缓存编译
qt_add_qml_module(appMyApp
    URI MyApp
    VERSION 1.0
    QML_FILES Main.qml
    # NO_CACHEGEN  # 不要设置这个选项
)
```


### 7.6 跨平台路径问题

**问题：**
- Windows 使用反斜杠 `\`，Linux/macOS 使用正斜杠 `/`

**解决方案：**
```cpp
// ✅ 使用 Qt 的路径处理
#include <QDir>

QString appDir = QCoreApplication::applicationDirPath();
QString pluginPath = QDir(appDir).filePath("plugins");  // 自动处理分隔符
engine.addImportPath(pluginPath);

// ✅ 或使用正斜杠（Qt 会自动转换）
engine.addImportPath(appDir + "/plugins");  // 在所有平台都有效

// ❌ 避免使用反斜杠
engine.addImportPath(appDir + "\\plugins");  // 仅 Windows 有效
```

### 7.7 调试 QML 模块加载

**启用详细日志：**
```cpp
// 方法 1：在代码中设置
qputenv("QML_IMPORT_TRACE", "1");

// 方法 2：在命令行设置
// Windows CMD:
set QML_IMPORT_TRACE=1
app.exe

// Windows PowerShell:
$env:QML_IMPORT_TRACE=1
.\app.exe

// Linux/macOS:
export QML_IMPORT_TRACE=1
./app
```

**输出示例：**
```
QQmlImportDatabase::addImportPath: "D:/MyApp/build/qml"
QQmlImportDatabase::addImportPath: "qrc:/qt/qml"
QQmlImportDatabase::addToImport: "MyApp.Components" 1.0
QQmlImportDatabase::locateQmldir: "D:/MyApp/build/qml/MyApp/Components/qmldir"
QQmlImportDatabase::locateQmldir: found "D:/MyApp/build/qml/MyApp/Components/qmldir"
```

### 7.8 版本兼容性问题

**问题：**
- Qt5 项目迁移到 Qt6 后无法编译

**Qt5 vs Qt6 差异：**

| 特性 | Qt5 | Qt6 |
|------|-----|-----|
| QML 模块定义 | `qt5_add_resources()` | `qt_add_qml_module()` |
| 资源前缀 | 自定义 | `/qt/qml/<Module>` |
| 加载方式 | `engine.load(url)` | `engine.loadFromModule()` |
| qmldir | 手动创建 | 自动生成 |

**迁移步骤：**
```cmake
# Qt5 代码
qt5_add_resources(appMyApp resources.qrc)

# Qt6 代码
qt_add_qml_module(appMyApp
    URI MyApp
    VERSION 1.0
    QML_FILES Main.qml
)
```


---

## 8. 最佳实践总结

### 8.1 项目组织

```
MyApp/
├── CMakeLists.txt              # 主 CMake 配置
├── main.cpp                    # 应用程序入口
├── Main.qml                    # 主 QML 文件
├── qml/                        # QML 模块目录
│   ├── components/             # 组件模块
│   │   ├── qmldir
│   │   ├── Button.qml
│   │   └── TextField.qml
│   ├── pages/                  # 页面模块
│   │   ├── qmldir
│   │   ├── HomePage.qml
│   │   └── SettingsPage.qml
│   └── config/                 # 配置模块
│       ├── qmldir
│       └── AppConfig.qml
├── resources/                  # 资源文件
│   ├── images/
│   ├── fonts/
│   └── icons/
└── plugins/                    # 外部插件
    └── ThirdParty/
        ├── qmldir
        └── plugin.dll
```

### 8.2 命名规范

**模块命名：**
```qmldir
# ✅ 推荐：使用公司/组织前缀
module MyCompany.MyApp.Components

# ✅ 推荐：使用分层结构
module MyApp.UI.Basic
module MyApp.UI.Advanced
module MyApp.Data.Models

# ❌ 避免：过于简单的名称
module Components
module Utils
```

**文件命名：**
```
✅ 推荐：
- Button.qml          # 大写开头（QML 类型）
- AppConfig.qml       # 驼峰命名
- utils.js            # 小写（JavaScript）

❌ 避免：
- button.qml          # 小写开头
- app-config.qml      # 连字符
- Utils.JS            # 大写扩展名
```

### 8.3 性能优化

**1. 使用 QML 缓存编译**
```cmake
qt_add_qml_module(appMyApp
    URI MyApp
    VERSION 1.0
    QML_FILES Main.qml
    # 不要设置 NO_CACHEGEN
)
```

**2. 延迟加载组件**
```qml
// ✅ 使用 Loader 延迟加载
Loader {
    id: heavyComponentLoader
    active: false  // 需要时才加载
    source: "HeavyComponent.qml"
}

Button {
    text: "Load"
    onClicked: heavyComponentLoader.active = true
}
```

**3. 资源压缩**
```cmake
# 启用资源压缩
qt_add_resources(appMyApp "images"
    PREFIX "/images"
    OPTIONS -compress 9 -threshold 0
    FILES images/large_image.png
)
```


### 8.4 安全性考虑

**1. 避免硬编码敏感信息**
```qml
// ❌ 不要在 QML 中硬编码密钥
QtObject {
    property string apiKey: "sk_live_123456789"  // 危险！
}

// ✅ 从 C++ 后端获取
QtObject {
    property string apiKey: Backend.getApiKey()  // 安全
}
```

**2. 验证外部资源**
```cpp
// 加载外部 QML 前验证
QUrl qmlUrl = QUrl::fromUserInput(userInput);
if (qmlUrl.isValid() && qmlUrl.scheme() == "qrc") {
    engine.load(qmlUrl);
}
```

### 8.5 调试技巧

**1. 启用 QML 调试器**
```cmake
# 在 Debug 模式下启用 QML 调试
target_compile_definitions(appMyApp PRIVATE
    $<$<CONFIG:Debug>:QT_QML_DEBUG>
)
```

**2. 使用 console.log**
```qml
Component.onCompleted: {
    console.log("Component loaded")
    console.log("Import paths:", Qt.application.arguments)
}
```

**3. 检查类型信息**
```qml
MouseArea {
    onClicked: {
        console.log("Object type:", parent.toString())
        console.log("Properties:", Object.keys(parent))
    }
}
```

### 8.6 部署清单

**Windows 部署：**
```
MyApp/
├── MyApp.exe
├── Qt6Core.dll
├── Qt6Gui.dll
├── Qt6Qml.dll
├── Qt6Quick.dll
├── plugins/
│   └── MyPlugin.dll
├── qml/
│   └── MyApp/
│       ├── qmldir
│       └── Main.qml
└── resources/
    └── images/
```

**使用 windeployqt：**
```bash
# Windows
windeployqt.exe --qmldir path/to/qml MyApp.exe

# Linux
linuxdeployqt MyApp -qmldir=path/to/qml

# macOS
macdeployqt MyApp.app -qmldir=path/to/qml
```


---

## 9. 快速参考

### 9.1 常用 CMake 命令

```cmake
# 创建 QML 模块
qt_add_qml_module(target
    URI module.name
    VERSION 1.0
    QML_FILES file.qml
)

# 添加资源
qt_add_resources(target "name"
    PREFIX "/prefix"
    FILES file1 file2
)

# 设置 Import Path
set(QML_IMPORT_PATH "path" CACHE STRING "" FORCE)

# 部署文件
add_custom_command(TARGET target POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory src dst
)
```

### 9.2 常用 C++ 代码

```cpp
// 添加 Import Path
engine.addImportPath(path);

// 获取 Import Path 列表
QStringList paths = engine.importPathList();

// 加载 QML
engine.load(QUrl("qrc:/Main.qml"));
engine.loadFromModule("Module", "Main");

// 设置上下文属性
engine.rootContext()->setContextProperty("backend", &backend);

// 启用调试
qputenv("QML_IMPORT_TRACE", "1");
```

### 9.3 qmldir 语法速查

```qmldir
# 模块声明
module Module.Name

# 插件
plugin pluginname
classname PluginClass
optional plugin pluginname

# 类型
TypeName 1.0 TypeName.qml
singleton Single 1.0 Single.qml
internal Internal Internal.qml

# 其他
typeinfo file.qmltypes
prefer :/path/
depends Module 1.0
import Module 1.0
designersupported
```

### 9.4 资源路径格式

```
qrc:/path/to/file       # 完整格式
:/path/to/file          # 简短格式
qrc:///path/to/file     # 三斜杠格式（等同于两斜杠）
```

---

## 10. 参考资源

### 10.1 官方文档

- [Qt6 QML Modules](https://doc.qt.io/qt-6/qtqml-modules-topic.html)
- [Qt Resource System](https://doc.qt.io/qt-6/resources.html)
- [qt_add_qml_module](https://doc.qt.io/qt-6/qt-add-qml-module.html)
- [QML Import Path](https://doc.qt.io/qt-6/qtqml-syntax-imports.html)

### 10.2 相关工具

- **rcc**: Qt Resource Compiler
- **qmllint**: QML 语法检查工具
- **qmlformat**: QML 代码格式化工具
- **qmltyperegistrar**: 生成 .qmltypes 文件

### 10.3 环境变量

| 变量 | 说明 |
|------|------|
| `QML_IMPORT_TRACE` | 启用 Import 调试输出 |
| `QML2_IMPORT_PATH` | 额外的 Import Path |
| `QT_LOGGING_RULES` | 日志过滤规则 |
| `QT_QML_DEBUG` | 启用 QML 调试器 |

---

## 结语

Qt6 的 QML 资源管理系统虽然比 Qt5 更加自动化，但理解其底层机制对于解决问题至关重要。本文档涵盖了从基础概念到高级应用的完整内容，希望能帮助你更好地使用 Qt6 开发 QML 应用。

**关键要点：**
1. ✅ 使用 `qt_add_qml_module()` 而不是手动编写 .qrc
2. ✅ 在 `load()` 之前调用 `addImportPath()`
3. ✅ 使用 CMake 的 `POST_BUILD` 命令部署插件
4. ✅ 理解 qmldir 文件的作用和语法
5. ✅ 善用调试工具和日志输出

**遇到问题时：**
1. 检查 Import Path 列表
2. 验证 qmldir 文件内容
3. 确认文件路径正确
4. 启用 `QML_IMPORT_TRACE` 调试
5. 查看 Qt Creator 的 "Issues" 面板

---

*文档版本：1.0*  
*最后更新：2025-11-04*  
*适用于：Qt 6.x*
