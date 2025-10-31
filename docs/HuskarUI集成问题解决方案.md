# HuskarUI 集成问题解决方案

## 问题诊断

### 错误信息
```
HusWindow is not a type
```

### 根本原因
1. **Import Path 顺序错误**：在 `main.cpp` 中，`engine.addImportPath()` 在 `engine.loadFromModule()` **之后**调用，导致 QML 引擎加载 Main.qml 时找不到 HuskarUI 模块
2. **运行时缺少插件文件**：构建目录中没有 HuskarUI 的 QML 插件（qmldir、.dll/.so 文件）
3. **CMake 未配置插件部署**：没有将 HuskarUI/plugins 目录复制到构建输出目录

## 解决方案

### 方案 1：修复 main.cpp 中的加载顺序（必须）

**问题代码：**
```cpp
// ❌ 错误：先加载 QML，后添加 Import Path
engine.loadFromModule("QMLSQLite", "Main");
engine.addImportPath("./HuskarUI");
```

**修复后：**
```cpp
// ✅ 正确：先添加 Import Path，后加载 QML
engine.addImportPath("./HuskarUI");
engine.loadFromModule("QMLSQLite", "Main");
```

### 方案 2：在 CMakeLists.txt 中配置插件部署（推荐）

在 CMakeLists.txt 末尾添加以下代码，将 HuskarUI 插件复制到构建目录：

```cmake
# 部署 HuskarUI 插件到构建目录
add_custom_command(TARGET appQMLSQLite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/HuskarUI/plugins
        $<TARGET_FILE_DIR:appQMLSQLite>/HuskarUI/plugins
    COMMENT "Copying HuskarUI plugins to build directory"
)

# 部署 HuskarUI DLL 到构建目录（如果需要）
add_custom_command(TARGET appQMLSQLite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/HuskarUI/lib/bin
        $<TARGET_FILE_DIR:appQMLSQLite>
    COMMENT "Copying HuskarUI DLLs to build directory"
)
```

### 方案 3：简化 main.cpp 中的 Import Path 配置

**当前代码问题：**
- 路径过多且冗余
- 使用相对路径 `./` 不可靠

**优化后的代码：**
```cpp
// 获取应用程序目录
QString appDir = QCoreApplication::applicationDirPath();
std::cout << "Application Dir: " << appDir.toStdString() << std::endl;

// 添加 HuskarUI 插件路径（必须在 loadFromModule 之前）
engine.addImportPath(appDir + "/HuskarUI/plugins");
engine.addImportPath(appDir);

// 加载主 QML 文件
engine.loadFromModule("QMLSQLite", "Main");
```

## 完整修复步骤

### 步骤 1：修改 main.cpp
### 步骤 2：修改 CMakeLists.txt
### 步骤 3：重新构建项目

## 验证方法

运行程序后，检查控制台输出：
```
Application Dir: D:/QML/015_QMLSQLite/QMLSQLite/build/Desktop_Qt_6_9_3_MinGW_64_bit-Release
```

确认该目录下存在：
- `HuskarUI/plugins/HuskarUI/Basic/` 目录
- `qmldir` 文件
- `HuskarUIBasic.dll`（或 .so）

## 额外建议

1. **使用 QML_IMPORT_PATH 环境变量**（开发时）：
   - 在 Qt Creator 的 "Projects" -> "Run" -> "Environment" 中添加：
   - `QML_IMPORT_PATH=D:/QML/015_QMLSQLite/QMLSQLite/HuskarUI/plugins`

2. **检查 HuskarUI 插件结构**：
   确保 `HuskarUI/plugins/HuskarUI/Basic/` 目录包含：
   - `qmldir` 文件
   - QML 类型定义文件
   - 插件动态库文件

3. **调试 Import Path**：
   在 main.cpp 中添加调试输出：
   ```cpp
   qDebug() << "Import paths:" << engine.importPathList();
   ```
