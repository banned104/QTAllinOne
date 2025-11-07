# QQuickWindow 详解与实战

> 本文档详细介绍 Qt6 中 QQuickWindow 的核心概念、属性、方法、信号以及实战应用，帮助开发者深入理解和使用 QML 窗口系统。

## 目录

1. [QQuickWindow 概述](#1-quickwindow-概述)
2. [窗口基础属性](#2-窗口基础属性)
3. [窗口状态管理](#3-窗口状态管理)
4. [窗口几何与位置](#4-窗口几何与位置)
5. [窗口外观与样式](#5-窗口外观与样式)
6. [窗口渲染与性能](#6-窗口渲染与性能)
7. [窗口事件处理](#7-窗口事件处理)
8. [多窗口管理](#8-多窗口管理)
9. [窗口与屏幕](#9-窗口与屏幕)
10. [高级特性](#10-高级特性)
11. [实战案例](#11-实战案例)

---

## 1. QQuickWindow 概述

### 1.1 什么是 QQuickWindow？

QQuickWindow 是 Qt Quick 的顶层窗口类，提供了在屏幕上显示 QML 内容的窗口。

**继承关系**:
```
QObject
    ↓
QWindow
    ↓
QQuickWindow
    ↓
Window (QML)
```

### 1.2 基本使用

```qml
import QtQuick
import QtQuick.Controls

// Window 是 QQuickWindow 的 QML 封装
Window {
    id: mainWindow
    
    // 基础属性
    width: 1280
    height: 720
    visible: true
    title: "QQuickWindow 示例"
    
    // 窗口内容
    Rectangle {
        anchors.fill: parent
        color: "#f0f0f0"
        
        Text {
            anchors.centerIn: parent
            text: "Hello, QQuickWindow!"
            font.pixelSize: 24
        }
    }
}
```

### 1.3 C++ 中使用 QQuickWindow

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    
    // 获取根窗口
    QObject *rootObject = engine.rootObjects().first();
    QQuickWindow *window = qobject_cast<QQuickWindow*>(rootObject);
    
    if (window) {
        // 设置窗口属性
        window->setTitle("从 C++ 设置标题");
        window->setWidth(1280);
        window->setHeight(720);
        
        // 连接信号
        QObject::connect(window, &QQuickWindow::widthChanged, [window]() {
            qDebug() << "窗口宽度改变:" << window->width();
        });
    }
    
    return app.exec();
}
```

---

## 2. 窗口基础属性

### 2.1 尺寸属性

```qml
Window {
    // ========== 窗口尺寸 ==========
    width: 1280              // 窗口宽度
    height: 720              // 窗口高度
    
    minimumWidth: 800        // 最小宽度
    minimumHeight: 600       // 最小高度
    
    maximumWidth: 1920       // 最大宽度
    maximumHeight: 1080      // 最大高度
    
    // ========== 内容项尺寸（只读）==========
    // contentItem.width    // 内容区域宽度
    // contentItem.height   // 内容区域高度
    
    // 监听尺寸变化
    onWidthChanged: {
        console.log("窗口宽度:", width)
    }
    
    onHeightChanged: {
        console.log("窗口高度:", height)
    }
}
```

### 2.2 标题与图标

```qml
Window {
    // ========== 窗口标题 ==========
    title: "我的应用程序"
    
    // 动态标题
    property int documentCount: 5
    title: `我的应用 - ${documentCount} 个文档`
    
    // ========== 窗口图标（需要在 C++ 中设置）==========
    Component.onCompleted: {
        // 在 QML 中无法直接设置图标
        // 需要在 C++ 中使用 setIcon()
    }
}
```

```cpp
// C++ 中设置窗口图标
#include <QIcon>

QQuickWindow *window = qobject_cast<QQuickWindow*>(rootObject);
if (window) {
    window->setIcon(QIcon(":/icons/app_icon.png"));
}
```

### 2.3 可见性

```qml
Window {
    // ========== 可见性 ==========
    visible: true            // 窗口是否可见
    
    // 显示/隐藏窗口
    function toggleVisibility() {
        visible = !visible
    }
    
    // 监听可见性变化
    onVisibleChanged: {
        console.log("窗口可见性:", visible)
    }
    
    // 窗口激活状态（只读）
    // active: bool
    
    onActiveChanged: {
        console.log("窗口激活状态:", active)
        if (active) {
            console.log("窗口获得焦点")
        } else {
            console.log("窗口失去焦点")
        }
    }
}
```

---

## 3. 窗口状态管理

### 3.1 窗口可见性状态

```qml
import QtQuick
import QtQuick.Controls

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    title: "窗口状态管理"
    
    // ========== 窗口可见性 ==========
    // 可选值：
    // - Window.Windowed      // 正常窗口
    // - Window.Minimized     // 最小化
    // - Window.Maximized     // 最大化
    // - Window.FullScreen    // 全屏
    // - Window.AutomaticVisibility // 自动
    // - Window.Hidden        // 隐藏
    
    visibility: Window.Windowed
    
    // 监听状态变化
    onVisibilityChanged: {
        switch(visibility) {
            case Window.Windowed:
                console.log("正常窗口")
                break
            case Window.Minimized:
                console.log("最小化")
                break
            case Window.Maximized:
                console.log("最大化")
                break
            case Window.FullScreen:
                console.log("全屏")
                break
            case Window.Hidden:
                console.log("隐藏")
                break
        }
    }
    
    Column {
        anchors.centerIn: parent
        spacing: 10
        
        Button {
            text: "正常窗口"
            onClicked: window.visibility = Window.Windowed
        }
        
        Button {
            text: "最小化"
            onClicked: window.visibility = Window.Minimized
        }
        
        Button {
            text: "最大化"
            onClicked: window.visibility = Window.Maximized
        }
        
        Button {
            text: "全屏"
            onClicked: window.visibility = Window.FullScreen
        }
        
        Button {
            text: "退出全屏"
            onClicked: window.visibility = Window.Windowed
        }
        
        Button {
            text: "隐藏窗口"
            onClicked: window.visibility = Window.Hidden
        }
        
        Button {
            text: "显示窗口"
            onClicked: {
                window.visibility = Window.Windowed
                window.show()
            }
        }
    }
}
```

### 3.2 窗口方法

```qml
Window {
    id: window
    
    // ========== 窗口显示方法 ==========
    
    function showWindow() {
        show()              // 显示窗口
    }
    
    function hideWindow() {
        hide()              // 隐藏窗口
    }
    
    function showMinimized() {
        showMinimized()     // 最小化显示
    }
    
    function showMaximized() {
        showMaximized()     // 最大化显示
    }
    
    function showFullScreen() {
        showFullScreen()    // 全屏显示
    }
    
    function showNormal() {
        showNormal()        // 正常显示
    }
    
    function raiseWindow() {
        raise()             // 提升窗口到最前
    }
    
    function lowerWindow() {
        lower()             // 降低窗口到最后
    }
    
    function closeWindow() {
        close()             // 关闭窗口
    }
    
    // ========== 窗口关闭事件 ==========
    onClosing: (close) => {
        console.log("窗口即将关闭")
        
        // 可以阻止关闭
        // close.accepted = false
        
        // 显示确认对话框
        if (!confirmClose()) {
            close.accepted = false
        }
    }
    
    function confirmClose() {
        // 返回 true 允许关闭，false 阻止关闭
        return true
    }
}
```

---

## 4. 窗口几何与位置

### 4.1 窗口位置

```qml
import QtQuick
import QtQuick.Controls

Window {
    id: window
    width: 800
    height: 600
    visible: true
    
    // ========== 窗口位置 ==========
    x: 100                  // 窗口 X 坐标
    y: 100                  // 窗口 Y 坐标
    
    // 监听位置变化
    onXChanged: console.log("X:", x)
    onYChanged: console.log("Y:", y)
    
    Column {
        anchors.centerIn: parent
        spacing: 10
        
        Text {
            text: `窗口位置: (${window.x}, ${window.y})`
            font.pixelSize: 16
        }
        
        Row {
            spacing: 10
            
            Button {
                text: "移到左上角"
                onClicked: {
                    window.x = 0
                    window.y = 0
                }
            }
            
            Button {
                text: "移到屏幕中心"
                onClicked: {
                    // 获取屏幕尺寸
                    let screen = window.screen
                    window.x = (screen.width - window.width) / 2
                    window.y = (screen.height - window.height) / 2
                }
            }
            
            Button {
                text: "移到右下角"
                onClicked: {
                    let screen = window.screen
                    window.x = screen.width - window.width
                    window.y = screen.height - window.height
                }
            }
        }
    }
}
```

### 4.2 窗口边距与框架

```qml
Window {
    // ========== 窗口标志 ==========
    flags: Qt.Window                    // 窗口标志
    
    // 常用标志组合：
    // Qt.Window                        // 标准窗口
    // Qt.Dialog                        // 对话框
    // Qt.Popup                         // 弹出窗口
    // Qt.Tool                          // 工具窗口
    // Qt.FramelessWindowHint           // 无边框窗口
    // Qt.WindowStaysOnTopHint          // 窗口置顶
    // Qt.WindowCloseButtonHint         // 显示关闭按钮
    // Qt.WindowMinimizeButtonHint      // 显示最小化按钮
    // Qt.WindowMaximizeButtonHint      // 显示最大化按钮
    
    // 无边框窗口示例
    flags: Qt.Window | Qt.FramelessWindowHint
    
    // 置顶窗口
    flags: Qt.Window | Qt.WindowStaysOnTopHint
    
    // 自定义标题栏窗口
    flags: Qt.Window | Qt.FramelessWindowHint
    
    // 自定义标题栏
    Rectangle {
        id: titleBar
        width: parent.width
        height: 40
        color: "#2c3e50"
        
        Text {
            anchors.centerIn: parent
            text: window.title
            color: "white"
            font.pixelSize: 14
        }
        
        // 拖动窗口
        MouseArea {
            anchors.fill: parent
            property point clickPos: Qt.point(0, 0)
            
            onPressed: (mouse) => {
                clickPos = Qt.point(mouse.x, mouse.y)
            }
            
            onPositionChanged: (mouse) => {
                if (pressed) {
                    let delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
                    window.x += delta.x
                    window.y += delta.y
                }
            }
        }
        
        // 关闭按钮
        Rectangle {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 5
            width: 30
            height: 30
            color: closeMouseArea.containsMouse ? "#e74c3c" : "transparent"
            radius: 3
            
            Text {
                anchors.centerIn: parent
                text: "×"
                color: "white"
                font.pixelSize: 20
            }
            
            MouseArea {
                id: closeMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: window.close()
            }
        }
    }
}
```

---

## 5. 窗口外观与样式

### 5.1 窗口颜色

```qml
Window {
    width: 800
    height: 600
    visible: true
    
    // ========== 窗口背景颜色 ==========
    color: "#f0f0f0"        // 窗口背景色
    
    // 透明窗口
    color: "transparent"
    
    // 注意：透明窗口需要设置窗口标志
    flags: Qt.Window | Qt.FramelessWindowHint
    
    // 半透明效果
    opacity: 0.9            // 窗口不透明度 (0.0-1.0)
}
```

### 5.2 窗口模态

```qml
import QtQuick
import QtQuick.Controls

Window {
    id: mainWindow
    width: 800
    height: 600
    visible: true
    title: "主窗口"
    
    Button {
        anchors.centerIn: parent
        text: "打开模态对话框"
        onClicked: modalDialog.show()
    }
    
    // ========== 模态对话框 ==========
    Window {
        id: modalDialog
        width: 400
        height: 300
        title: "模态对话框"
        
        // 模态类型
        modality: Qt.ApplicationModal  // 应用程序模态
        // modality: Qt.WindowModal     // 窗口模态
        // modality: Qt.NonModal         // 非模态
        
        // 设置父窗口
        transientParent: mainWindow
        
        Column {
            anchors.centerIn: parent
            spacing: 20
            
            Text {
                text: "这是一个模态对话框"
                font.pixelSize: 16
            }
            
            Button {
                text: "关闭"
                onClicked: modalDialog.close()
            }
        }
    }
}
```

### 5.3 窗口阴影与效果

```qml
import QtQuick
import QtQuick.Effects

Window {
    width: 800
    height: 600
    visible: true
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint
    
    // 主内容区域
    Rectangle {
        anchors.fill: parent
        anchors.margins: 20
        color: "white"
        radius: 10
        
        // 阴影效果
        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: "#80000000"
            shadowBlur: 0.5
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 5
        }
        
        Text {
            anchors.centerIn: parent
            text: "带阴影的窗口"
            font.pixelSize: 24
        }
    }
}
```

---

## 6. 窗口渲染与性能

### 6.1 渲染相关属性

```qml
Window {
    // ========== 渲染设置 ==========
    
    // 颜色格式
    // color: "transparent" 需要设置 format
    // format: Window.RGBA8888
    
    // 渲染模式（只读）
    // renderTarget: Window.FramebufferObject
    
    // 场景图渲染
    // sceneGraphError: 渲染错误信号
    
    // 监听渲染错误
    onSceneGraphError: (error, message) => {
        console.error("场景图错误:", error, message)
    }
    
    // 帧交换信号
    onFrameSwapped: {
        // 每次帧交换时触发
        // 可用于性能监控
    }
    
    // 渲染前信号
    onBeforeRendering: {
        // 在渲染前执行自定义 OpenGL 代码
    }
    
    // 渲染后信号
    onAfterRendering: {
        // 在渲染后执行自定义 OpenGL 代码
    }
}
```

### 6.2 性能优化

```qml
Window {
    width: 1280
    height: 720
    visible: true
    
    // ========== 性能优化设置 ==========
    
    // 1. 启用持久化场景图
    // persistentSceneGraph: true
    
    // 2. 启用持久化 OpenGL 上下文
    // persistentOpenGLContext: true
    
    // 3. 设置渲染目标
    // renderTarget: Window.FramebufferObject
    
    // FPS 计数器
    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        width: 100
        height: 50
        color: "#80000000"
        radius: 5
        
        Column {
            anchors.centerIn: parent
            
            Text {
                text: "FPS"
                color: "white"
                font.pixelSize: 12
            }
            
            Text {
                id: fpsText
                text: fpsCounter.fps.toFixed(1)
                color: fpsCounter.fps > 50 ? "#00ff00" : "#ff0000"
                font.pixelSize: 20
                font.bold: true
            }
        }
    }
    
    // FPS 计数器逻辑
    QtObject {
        id: fpsCounter
        property int frameCount: 0
        property real lastTime: Date.now()
        property real fps: 60
        
        function update() {
            frameCount++
            let currentTime = Date.now()
            let deltaTime = currentTime - lastTime
            
            if (deltaTime >= 1000) {
                fps = frameCount * 1000 / deltaTime
                frameCount = 0
                lastTime = currentTime
            }
        }
    }
    
    Timer {
        interval: 16  // ~60 FPS
        running: true
        repeat: true
        onTriggered: fpsCounter.update()
    }
}
```



---

## 7. 窗口事件处理

### 7.1 键盘事件

```qml
Window {
    width: 800
    height: 600
    visible: true
    title: "键盘事件处理"
    
    // 焦点设置
    focus: true
    
    // ========== 键盘事件 ==========
    Keys.onPressed: (event) => {
        console.log("按键按下:", event.key, event.text)
        
        // 常用按键
        if (event.key === Qt.Key_Escape) {
            console.log("ESC 键")
            event.accepted = true
        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            console.log("回车键")
        } else if (event.key === Qt.Key_Space) {
            console.log("空格键")
        }
        
        // 修饰键
        if (event.modifiers & Qt.ControlModifier) {
            console.log("Ctrl 键被按下")
        }
        if (event.modifiers & Qt.ShiftModifier) {
            console.log("Shift 键被按下")
        }
        if (event.modifiers & Qt.AltModifier) {
            console.log("Alt 键被按下")
        }
        
        // 快捷键
        if (event.key === Qt.Key_S && (event.modifiers & Qt.ControlModifier)) {
            console.log("Ctrl+S 保存")
            event.accepted = true
        }
    }
    
    Keys.onReleased: (event) => {
        console.log("按键释放:", event.key)
    }
    
    Rectangle {
        anchors.fill: parent
        color: "#f0f0f0"
        
        Text {
            anchors.centerIn: parent
            text: "按任意键测试\n" +
                  "ESC - 退出\n" +
                  "Ctrl+S - 保存"
            font.pixelSize: 16
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
```

### 7.2 鼠标事件

```qml
Window {
    width: 800
    height: 600
    visible: true
    title: "鼠标事件处理"
    
    MouseArea {
        anchors.fill: parent
        
        // 启用悬停
        hoverEnabled: true
        
        // 鼠标按下
        onPressed: (mouse) => {
            console.log("鼠标按下:", mouse.x, mouse.y)
            console.log("按钮:", mouse.button)
            // mouse.button: Qt.LeftButton, Qt.RightButton, Qt.MiddleButton
        }
        
        // 鼠标释放
        onReleased: (mouse) => {
            console.log("鼠标释放:", mouse.x, mouse.y)
        }
        
        // 鼠标点击
        onClicked: (mouse) => {
            console.log("鼠标点击:", mouse.x, mouse.y)
        }
        
        // 鼠标双击
        onDoubleClicked: (mouse) => {
            console.log("鼠标双击:", mouse.x, mouse.y)
        }
        
        // 鼠标移动
        onPositionChanged: (mouse) => {
            if (pressed) {
                console.log("拖拽:", mouse.x, mouse.y)
            }
        }
        
        // 鼠标进入
        onEntered: {
            console.log("鼠标进入窗口")
        }
        
        // 鼠标离开
        onExited: {
            console.log("鼠标离开窗口")
        }
        
        // 滚轮事件
        onWheel: (wheel) => {
            console.log("滚轮:", wheel.angleDelta.y)
        }
    }
}
```

### 7.3 触摸事件

```qml
Window {
    width: 800
    height: 600
    visible: true
    title: "触摸事件处理"
    
    MultiPointTouchArea {
        anchors.fill: parent
        
        // 触摸点更新
        onTouchUpdated: (touchPoints) => {
            console.log("触摸点数量:", touchPoints.length)
            
            for (let i = 0; i < touchPoints.length; i++) {
                let point = touchPoints[i]
                console.log(`触摸点 ${i}:`, point.x, point.y)
            }
        }
        
        // 手势识别
        onGestureStarted: (gesture) => {
            console.log("手势开始")
        }
        
        onUpdated: (touchPoints) => {
            // 双指缩放
            if (touchPoints.length === 2) {
                let point1 = touchPoints[0]
                let point2 = touchPoints[1]
                
                let dx = point2.x - point1.x
                let dy = point2.y - point1.y
                let distance = Math.sqrt(dx * dx + dy * dy)
                
                console.log("双指距离:", distance)
            }
        }
    }
}
```

---

## 8. 多窗口管理

### 8.1 创建多个窗口

```qml
import QtQuick
import QtQuick.Controls

Window {
    id: mainWindow
    width: 800
    height: 600
    visible: true
    title: "主窗口"
    
    Column {
        anchors.centerIn: parent
        spacing: 10
        
        Button {
            text: "打开子窗口"
            onClicked: {
                let component = Qt.createComponent("ChildWindow.qml")
                if (component.status === Component.Ready) {
                    let window = component.createObject(mainWindow)
                    window.show()
                }
            }
        }
        
        Button {
            text: "打开设置窗口"
            onClicked: settingsWindow.show()
        }
        
        Button {
            text: "打开关于窗口"
            onClicked: aboutWindow.show()
        }
    }
    
    // ========== 设置窗口 ==========
    Window {
        id: settingsWindow
        width: 600
        height: 400
        title: "设置"
        modality: Qt.ApplicationModal
        
        Column {
            anchors.centerIn: parent
            spacing: 20
            
            Text {
                text: "设置窗口"
                font.pixelSize: 20
            }
            
            Button {
                text: "关闭"
                onClicked: settingsWindow.close()
            }
        }
    }
    
    // ========== 关于窗口 ==========
    Window {
        id: aboutWindow
        width: 400
        height: 300
        title: "关于"
        modality: Qt.ApplicationModal
        
        Column {
            anchors.centerIn: parent
            spacing: 20
            
            Text {
                text: "关于应用程序"
                font.pixelSize: 20
            }
            
            Text {
                text: "版本 1.0.0"
                font.pixelSize: 14
            }
            
            Button {
                text: "关闭"
                onClicked: aboutWindow.close()
            }
        }
    }
}
```

### 8.2 窗口间通信

```qml
// MainWindow.qml
Window {
    id: mainWindow
    width: 800
    height: 600
    visible: true
    title: "主窗口"
    
    // 共享数据
    property string sharedData: "来自主窗口的数据"
    
    // 信号
    signal dataChanged(string newData)
    
    Column {
        anchors.centerIn: parent
        spacing: 10
        
        TextField {
            id: dataInput
            placeholderText: "输入数据"
        }
        
        Button {
            text: "更新数据"
            onClicked: {
                mainWindow.sharedData = dataInput.text
                mainWindow.dataChanged(dataInput.text)
            }
        }
        
        Button {
            text: "打开子窗口"
            onClicked: {
                let component = Qt.createComponent("ChildWindow.qml")
                let window = component.createObject(mainWindow, {
                    "parentWindow": mainWindow
                })
                window.show()
            }
        }
    }
}

// ChildWindow.qml
Window {
    id: childWindow
    width: 400
    height: 300
    title: "子窗口"
    
    property var parentWindow: null
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        Text {
            text: parentWindow ? parentWindow.sharedData : "无数据"
            font.pixelSize: 16
        }
        
        Button {
            text: "关闭"
            onClicked: childWindow.close()
        }
    }
    
    Connections {
        target: parentWindow
        function onDataChanged(newData) {
            console.log("接收到新数据:", newData)
        }
    }
}
```

---

## 9. 窗口与屏幕

### 9.1 屏幕信息

```qml
import QtQuick
import QtQuick.Controls

Window {
    id: window
    width: 800
    height: 600
    visible: true
    title: "屏幕信息"
    
    Column {
        anchors.centerIn: parent
        spacing: 10
        
        Text {
            text: "屏幕信息"
            font.pixelSize: 20
            font.bold: true
        }
        
        Text {
            text: `屏幕名称: ${window.screen.name}`
        }
        
        Text {
            text: `屏幕尺寸: ${window.screen.width} x ${window.screen.height}`
        }
        
        Text {
            text: `可用区域: ${window.screen.desktopAvailableWidth} x ${window.screen.desktopAvailableHeight}`
        }
        
        Text {
            text: `DPI: ${window.screen.pixelDensity}`
        }
        
        Text {
            text: `设备像素比: ${window.screen.devicePixelRatio}`
        }
        
        Text {
            text: `方向: ${getOrientationName(window.screen.orientation)}`
        }
        
        Button {
            text: "移到屏幕中心"
            onClicked: {
                window.x = (window.screen.width - window.width) / 2
                window.y = (window.screen.height - window.height) / 2
            }
        }
    }
    
    function getOrientationName(orientation) {
        switch(orientation) {
            case Qt.PortraitOrientation: return "竖屏"
            case Qt.LandscapeOrientation: return "横屏"
            case Qt.InvertedPortraitOrientation: return "倒置竖屏"
            case Qt.InvertedLandscapeOrientation: return "倒置横屏"
            default: return "未知"
        }
    }
}
```

### 9.2 多屏幕支持

```qml
import QtQuick
import QtQuick.Controls

Window {
    width: 800
    height: 600
    visible: true
    title: "多屏幕支持"
    
    Column {
        anchors.centerIn: parent
        spacing: 10
        
        Text {
            text: "可用屏幕"
            font.pixelSize: 20
            font.bold: true
        }
        
        Repeater {
            model: Qt.application.screens
            
            Button {
                text: `移到屏幕 ${index + 1}: ${modelData.name}`
                onClicked: {
                    // 移动窗口到指定屏幕
                    window.screen = modelData
                    
                    // 居中显示
                    window.x = modelData.virtualX + (modelData.width - window.width) / 2
                    window.y = modelData.virtualY + (modelData.height - window.height) / 2
                }
            }
        }
    }
}
```

---

## 10. 高级特性

### 10.1 窗口截图

```qml
import QtQuick
import QtQuick.Controls

Window {
    id: window
    width: 800
    height: 600
    visible: true
    title: "窗口截图"
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        Rectangle {
            width: 200
            height: 200
            color: "#4080ff"
            
            Text {
                anchors.centerIn: parent
                text: "截图内容"
                color: "white"
                font.pixelSize: 20
            }
        }
        
        Button {
            text: "截图"
            onClicked: {
                // 抓取窗口内容
                window.grabWindow()
            }
        }
    }
    
    function grabWindow() {
        // 使用 grabToImage 截图
        window.contentItem.grabToImage(function(result) {
            // 保存图片
            let success = result.saveToFile("screenshot.png")
            if (success) {
                console.log("截图已保存")
            }
        })
    }
}
```

### 10.2 自定义窗口装饰

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Window {
    id: window
    width: 800
    height: 600
    visible: true
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint
    
    // ========== 自定义窗口装饰 ==========
    Rectangle {
        id: windowFrame
        anchors.fill: parent
        anchors.margins: 10
        color: "#ffffff"
        radius: 10
        
        // 阴影效果
        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: "#40000000"
            shadowBlur: 0.8
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 5
        }
        
        // 标题栏
        Rectangle {
            id: titleBar
            width: parent.width
            height: 40
            color: "#2c3e50"
            radius: 10
            
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: parent.radius
                color: parent.color
            }
            
            Text {
                anchors.centerIn: parent
                text: window.title
                color: "white"
                font.pixelSize: 14
            }
            
            // 拖动区域
            MouseArea {
                anchors.fill: parent
                property point clickPos: Qt.point(0, 0)
                
                onPressed: (mouse) => {
                    clickPos = Qt.point(mouse.x, mouse.y)
                }
                
                onPositionChanged: (mouse) => {
                    if (pressed) {
                        let delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
                        window.x += delta.x
                        window.y += delta.y
                    }
                }
                
                onDoubleClicked: {
                    if (window.visibility === Window.Maximized) {
                        window.showNormal()
                    } else {
                        window.showMaximized()
                    }
                }
            }
            
            // 窗口控制按钮
            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 5
                spacing: 5
                
                // 最小化
                Rectangle {
                    width: 30
                    height: 30
                    color: minMouseArea.containsMouse ? "#34495e" : "transparent"
                    radius: 3
                    
                    Text {
                        anchors.centerIn: parent
                        text: "−"
                        color: "white"
                        font.pixelSize: 16
                    }
                    
                    MouseArea {
                        id: minMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: window.showMinimized()
                    }
                }
                
                // 最大化/还原
                Rectangle {
                    width: 30
                    height: 30
                    color: maxMouseArea.containsMouse ? "#34495e" : "transparent"
                    radius: 3
                    
                    Text {
                        anchors.centerIn: parent
                        text: window.visibility === Window.Maximized ? "❐" : "□"
                        color: "white"
                        font.pixelSize: 14
                    }
                    
                    MouseArea {
                        id: maxMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            if (window.visibility === Window.Maximized) {
                                window.showNormal()
                            } else {
                                window.showMaximized()
                            }
                        }
                    }
                }
                
                // 关闭
                Rectangle {
                    width: 30
                    height: 30
                    color: closeMouseArea.containsMouse ? "#e74c3c" : "transparent"
                    radius: 3
                    
                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        color: "white"
                        font.pixelSize: 20
                    }
                    
                    MouseArea {
                        id: closeMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: window.close()
                    }
                }
            }
        }
        
        // 内容区域
        Rectangle {
            anchors.top: titleBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            color: "#ecf0f1"
            
            Text {
                anchors.centerIn: parent
                text: "自定义窗口装饰"
                font.pixelSize: 24
            }
        }
        
        // 调整大小区域
        MouseArea {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: 20
            height: 20
            cursorShape: Qt.SizeFDiagCursor
            
            property point clickPos: Qt.point(0, 0)
            
            onPressed: (mouse) => {
                clickPos = Qt.point(mouse.x, mouse.y)
            }
            
            onPositionChanged: (mouse) => {
                if (pressed) {
                    let delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
                    window.width += delta.x
                    window.height += delta.y
                }
            }
        }
    }
}
```

---

## 11. 实战案例

### 11.1 完整的应用程序窗口

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: appWindow
    width: 1280
    height: 720
    visible: true
    title: "完整应用程序示例"
    
    // ========== 菜单栏 ==========
    menuBar: MenuBar {
        Menu {
            title: "文件"
            MenuItem {
                text: "新建"
                onTriggered: console.log("新建")
            }
            MenuItem {
                text: "打开"
                onTriggered: console.log("打开")
            }
            MenuSeparator {}
            MenuItem {
                text: "退出"
                onTriggered: Qt.quit()
            }
        }
        
        Menu {
            title: "编辑"
            MenuItem { text: "撤销" }
            MenuItem { text: "重做" }
        }
        
        Menu {
            title: "帮助"
            MenuItem {
                text: "关于"
                onTriggered: aboutDialog.open()
            }
        }
    }
    
    // ========== 工具栏 ==========
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            
            ToolButton {
                text: "新建"
                icon.source: "qrc:/icons/new.png"
            }
            
            ToolButton {
                text: "打开"
                icon.source: "qrc:/icons/open.png"
            }
            
            ToolButton {
                text: "保存"
                icon.source: "qrc:/icons/save.png"
            }
            
            ToolSeparator {}
            
            ToolButton {
                text: "撤销"
                icon.source: "qrc:/icons/undo.png"
            }
            
            ToolButton {
                text: "重做"
                icon.source: "qrc:/icons/redo.png"
            }
            
            Item { Layout.fillWidth: true }
            
            Label {
                text: "就绪"
            }
        }
    }
    
    // ========== 状态栏 ==========
    footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            
            Label {
                text: `行: 1  列: 1`
            }
            
            Item { Layout.fillWidth: true }
            
            Label {
                text: `${new Date().toLocaleString()}`
            }
        }
    }
    
    // ========== 主内容区域 ==========
    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        
        // 侧边栏
        Rectangle {
            SplitView.minimumWidth: 200
            SplitView.preferredWidth: 250
            color: "#f0f0f0"
            
            ListView {
                anchors.fill: parent
                model: ["项目 1", "项目 2", "项目 3"]
                delegate: ItemDelegate {
                    width: parent.width
                    text: modelData
                }
            }
        }
        
        // 主编辑区
        Rectangle {
            SplitView.fillWidth: true
            color: "white"
            
            TextArea {
                anchors.fill: parent
                anchors.margins: 10
                placeholderText: "在此输入内容..."
            }
        }
    }
    
    // ========== 关于对话框 ==========
    Dialog {
        id: aboutDialog
        title: "关于"
        modal: true
        anchors.centerIn: parent
        
        Column {
            spacing: 20
            
            Text {
                text: "应用程序名称"
                font.pixelSize: 20
                font.bold: true
            }
            
            Text {
                text: "版本 1.0.0"
            }
            
            Text {
                text: "© 2024 公司名称"
            }
        }
        
        standardButtons: Dialog.Ok
    }
}
```

### 11.2 启动画面

```qml
import QtQuick
import QtQuick.Controls

Window {
    id: splashWindow
    width: 600
    height: 400
    visible: true
    color: "transparent"
    flags: Qt.SplashScreen | Qt.FramelessWindowHint
    
    Rectangle {
        anchors.fill: parent
        color: "#2c3e50"
        radius: 10
        
        Column {
            anchors.centerIn: parent
            spacing: 30
            
            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "qrc:/icons/logo.png"
                width: 128
                height: 128
            }
            
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "应用程序名称"
                color: "white"
                font.pixelSize: 24
                font.bold: true
            }
            
            ProgressBar {
                width: 300
                value: loadProgress.value
            }
            
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "正在加载..."
                color: "white"
                font.pixelSize: 14
            }
        }
    }
    
    // 模拟加载进度
    QtObject {
        id: loadProgress
        property real value: 0.0
    }
    
    Timer {
        interval: 50
        running: true
        repeat: true
        onTriggered: {
            loadProgress.value += 0.02
            if (loadProgress.value >= 1.0) {
                stop()
                // 显示主窗口
                let component = Qt.createComponent("MainWindow.qml")
                let mainWindow = component.createObject()
                mainWindow.show()
                // 关闭启动画面
                splashWindow.close()
            }
        }
    }
}
```

---

## 总结

### 核心知识点

1. **窗口基础**
   - 尺寸、位置、标题
   - 可见性和激活状态
   - 窗口标志和模态

2. **窗口状态**
   - 正常、最小化、最大化、全屏
   - 显示/隐藏方法
   - 关闭事件处理

3. **窗口外观**
   - 背景颜色和透明度
   - 自定义标题栏
   - 窗口阴影效果

4. **事件处理**
   - 键盘事件
   - 鼠标事件
   - 触摸事件

5. **多窗口**
   - 创建子窗口
   - 窗口间通信
   - 模态对话框

6. **屏幕管理**
   - 屏幕信息获取
   - 多屏幕支持
   - DPI 适配

### 最佳实践

✅ **推荐做法**
- 合理设置最小/最大尺寸
- 使用 ApplicationWindow 构建完整应用
- 正确处理窗口关闭事件
- 实现响应式布局
- 监控性能指标

❌ **避免做法**
- 不要频繁改变窗口大小
- 避免过度使用透明效果
- 不要忽略多屏幕场景
- 避免阻塞 UI 线程

### 参考资源

- **官方文档**: https://doc.qt.io/qt-6/qquickwindow.html
- **QML Window**: https://doc.qt.io/qt-6/qml-qtquick-window.html
- **示例代码**: Qt Creator → Examples → Qt Quick

---

**祝你在 QQuickWindow 开发中取得成功！** 🚀
