# QML 开发核心概念与细节指南

> 本文档深入讲解 QML 开发中的核心概念、容易混淆的细节、最佳实践和常见陷阱。

## 目录

1. [QQuickWindow 与窗口系统](#1-qquickwindow-与窗口系统)
2. [尺寸系统详解](#2-尺寸系统详解)
3. [布局系统](#3-布局系统)
4. [锚点系统](#4-锚点系统)
5. [属性绑定机制](#5-属性绑定机制)
6. [信号与槽](#6-信号与槽)
7. [组件生命周期](#7-组件生命周期)
8. [性能优化](#8-性能优化)
9. [常见陷阱与解决方案](#9-常见陷阱与解决方案)

---

## 1. QQuickWindow 与窗口系统

### 1.1 Window vs ApplicationWindow

```qml
import QtQuick
import QtQuick.Controls

// ✅ Window - 基础窗口
Window {
    width: 640
    height: 480
    visible: true
    title: "Basic Window"
    
    // Window 的特性：
    // - 最基础的窗口类型
    // - 没有内置的菜单栏、工具栏等
    // - 适合简单应用
    
    Rectangle {
        anchors.fill: parent
        color: "lightblue"
    }
}

// ✅ ApplicationWindow - 应用程序窗口
ApplicationWindow {
    width: 800
    height: 600
    visible: true
    title: "Application Window"
    
    // ApplicationWindow 的特性：
    // - 继承自 Window
    // - 提供 header、footer、menuBar 等
    // - 适合复杂应用
    
    header: ToolBar {
        Label {
            text: "Header"
            anchors.centerIn: parent
        }
    }
    
    footer: ToolBar {
        Label {
            text: "Footer"
            anchors.centerIn: parent
        }
    }
    
    // 内容区域
    Rectangle {
        anchors.fill: parent
        color: "white"
    }
}
```

### 1.2 Window 的关键属性

```qml
Window {
    // === 尺寸相关 ===
    width: 640                      // 窗口宽度
    height: 480                     // 窗口高度
    minimumWidth: 400               // 最小宽度
    minimumHeight: 300              // 最小高度
    maximumWidth: 1920              // 最大宽度
    maximumHeight: 1080             // 最大高度
    
    // === 可见性 ===
    visible: true                   // 是否可见
    visibility: Window.Windowed     // 窗口状态
    // - Window.Windowed: 正常窗口
    // - Window.Minimized: 最小化
    // - Window.Maximized: 最大化
    // - Window.FullScreen: 全屏
    // - Window.AutomaticVisibility: 自动
    
    // === 窗口标志 ===
    flags: Qt.Window | Qt.FramelessWindowHint
    // - Qt.Window: 普通窗口
    // - Qt.Dialog: 对话框
    // - Qt.Popup: 弹出窗口
    // - Qt.FramelessWindowHint: 无边框
    // - Qt.WindowStaysOnTopHint: 置顶
    
    // === 模态性 ===
    modality: Qt.NonModal           // 非模态
    // - Qt.NonModal: 非模态
    // - Qt.WindowModal: 窗口模态
    // - Qt.ApplicationModal: 应用程序模态
    
    // === 透明度 ===
    opacity: 1.0                    // 窗口不透明度 (0.0-1.0)
    color: "transparent"            // 背景颜色（支持透明）
    
    // === 位置 ===
    x: 100                          // 屏幕 X 坐标
    y: 100                          // 屏幕 Y 坐标
    
    // === 其他 ===
    title: "My Window"              // 窗口标题
    transientParent: parentWindow   // 父窗口（用于对话框）
}
```


### 1.3 窗口事件处理

```qml
Window {
    width: 640
    height: 480
    visible: true
    
    // ✅ 窗口关闭事件
    onClosing: function(close) {
        console.log("Window is closing")
        
        // 可以阻止关闭
        if (hasUnsavedChanges) {
            close.accepted = false
            confirmDialog.open()
        }
    }
    
    // ✅ 窗口激活/失活
    onActiveChanged: {
        if (active) {
            console.log("Window activated")
        } else {
            console.log("Window deactivated")
        }
    }
    
    // ✅ 窗口可见性变化
    onVisibilityChanged: {
        console.log("Visibility:", visibility)
    }
    
    // ✅ 窗口尺寸变化
    onWidthChanged: console.log("Width:", width)
    onHeightChanged: console.log("Height:", height)
    
    // ✅ 屏幕变化
    onScreenChanged: {
        console.log("Screen:", screen.name)
        console.log("DPI:", screen.pixelDensity)
    }
}
```

### 1.4 多窗口管理

```qml
// MainWindow.qml
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: mainWindow
    width: 800
    height: 600
    visible: true
    title: "Main Window"
    
    Button {
        text: "Open Dialog"
        anchors.centerIn: parent
        onClicked: {
            // ✅ 创建子窗口
            var component = Qt.createComponent("DialogWindow.qml")
            if (component.status === Component.Ready) {
                var dialog = component.createObject(mainWindow, {
                    transientParent: mainWindow  // 设置父窗口
                })
                dialog.show()
            }
        }
    }
    
    // ✅ 使用 Loader 动态加载窗口
    Loader {
        id: dialogLoader
        active: false
        sourceComponent: Window {
            width: 400
            height: 300
            visible: true
            modality: Qt.ApplicationModal
            title: "Dialog"
            
            onClosing: dialogLoader.active = false
        }
    }
    
    Button {
        text: "Open Loader Dialog"
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: dialogLoader.active = true
    }
}

// DialogWindow.qml
import QtQuick
import QtQuick.Controls

Window {
    width: 400
    height: 300
    modality: Qt.WindowModal
    title: "Dialog Window"
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        Label {
            text: "This is a dialog window"
        }
        
        Button {
            text: "Close"
            onClicked: close()
        }
    }
}
```

---

## 2. 尺寸系统详解

### 2.1 width/height vs implicitWidth/implicitHeight

这是一个非常重要且容易混淆的概念！

```qml
import QtQuick

Rectangle {
    // ❌ 常见误解：认为 implicitWidth/Height 会设置默认大小
    implicitWidth: 200
    implicitHeight: 100
    color: "pink"
    
    // 问题：这个 Rectangle 实际上没有大小！
    // 原因：implicitWidth/Height 只是"建议"大小，不是实际大小
}

// ✅ 正确理解：
Rectangle {
    // 1. implicitWidth/Height 是"隐式"大小
    //    - 当没有显式设置 width/height 时的"建议"大小
    //    - 主要用于布局系统和父子组件尺寸计算
    implicitWidth: 200
    implicitHeight: 100
    
    // 2. 如果想要实际大小，必须显式设置 width/height
    width: implicitWidth    // 使用隐式宽度作为实际宽度
    height: implicitHeight  // 使用隐式高度作为实际高度
    
    color: "pink"
}

// ✅ 更好的做法：直接设置 width/height
Rectangle {
    width: 200
    height: 100
    color: "pink"
}
```

### 2.2 隐式尺寸的真正用途

```qml
import QtQuick
import QtQuick.Controls

// ✅ 场景 1：自定义组件提供默认尺寸
// MyButton.qml
Rectangle {
    id: root
    
    // 提供默认尺寸（建议）
    implicitWidth: 120
    implicitHeight: 40
    
    // 实际尺寸可以被外部覆盖
    width: implicitWidth
    height: implicitHeight
    
    color: "lightblue"
    radius: 5
    
    property alias text: label.text
    
    Text {
        id: label
        anchors.centerIn: parent
    }
}

// 使用自定义组件
Item {
    MyButton {
        // ✅ 不设置尺寸，使用默认的 120x40
        text: "Default Size"
    }
    
    MyButton {
        // ✅ 覆盖默认尺寸
        width: 200
        height: 60
        text: "Custom Size"
    }
}

// ✅ 场景 2：根据内容自动调整大小
Rectangle {
    // 不设置固定宽度，使用内容的隐式宽度
    implicitWidth: contentItem.implicitWidth + 20  // 内容宽度 + 边距
    implicitHeight: contentItem.implicitHeight + 20
    
    width: implicitWidth
    height: implicitHeight
    
    color: "lightgray"
    
    Text {
        id: contentItem
        anchors.centerIn: parent
        text: "Dynamic Size"
    }
}

// ✅ 场景 3：布局中的尺寸提示
RowLayout {
    spacing: 10
    
    Rectangle {
        // 布局会参考 implicitWidth 来分配空间
        implicitWidth: 100
        implicitHeight: 50
        
        // Layout.fillWidth 会覆盖 implicitWidth
        Layout.fillWidth: true
        Layout.preferredHeight: implicitHeight
        
        color: "lightblue"
    }
    
    Rectangle {
        implicitWidth: 150
        implicitHeight: 50
        
        Layout.preferredWidth: implicitWidth
        Layout.preferredHeight: implicitHeight
        
        color: "lightgreen"
    }
}
```


### 2.3 尺寸系统完整对比

```qml
Rectangle {
    // === 显式尺寸（Explicit Size）===
    width: 200                      // 明确指定的宽度
    height: 100                     // 明确指定的高度
    // 特点：
    // - 优先级最高
    // - 直接设置组件的实际大小
    // - 会覆盖 implicitWidth/Height
    
    // === 隐式尺寸（Implicit Size）===
    implicitWidth: 150              // 建议的宽度
    implicitHeight: 80              // 建议的高度
    // 特点：
    // - 仅在没有显式设置 width/height 时生效
    // - 用于布局系统的尺寸计算
    // - 可以被 width/height 覆盖
    
    // === 内容尺寸（Content Size）===
    // 某些组件有内置的内容尺寸
    // 例如：Text、Image 等
    
    Text {
        // Text 的 implicitWidth/Height 由文本内容决定
        text: "Hello World"
        // implicitWidth = 文本渲染宽度
        // implicitHeight = 字体高度
        
        // 如果不设置 width，Text 会使用 implicitWidth
        // 如果设置了 width，文本可能会换行或截断
    }
    
    Image {
        source: "image.png"
        // Image 的 implicitWidth/Height 由图片尺寸决定
        // implicitWidth = 图片原始宽度
        // implicitHeight = 图片原始高度
        
        // 如果不设置 width/height，显示原始大小
        // 如果设置了，图片会缩放
    }
}
```

### 2.4 尺寸优先级规则

```qml
Item {
    // 优先级从高到低：
    // 1. 显式设置的 width/height
    // 2. 锚点约束（anchors）
    // 3. 布局约束（Layout.xxx）
    // 4. implicitWidth/implicitHeight
    // 5. 默认值（通常为 0）
    
    Rectangle {
        id: rect1
        // ✅ 情况 1：只有 implicitWidth
        implicitWidth: 100
        implicitHeight: 50
        // 结果：宽度 = 0，高度 = 0（因为没有显式设置）
        color: "red"
    }
    
    Rectangle {
        id: rect2
        // ✅ 情况 2：implicitWidth + width
        implicitWidth: 100
        implicitHeight: 50
        width: implicitWidth    // 显式使用 implicitWidth
        height: implicitHeight
        // 结果：宽度 = 100，高度 = 50
        color: "blue"
    }
    
    Rectangle {
        id: rect3
        // ✅ 情况 3：implicitWidth + 锚点
        implicitWidth: 100
        implicitHeight: 50
        anchors.fill: parent
        // 结果：填充父容器（锚点优先级高于 implicitWidth）
        color: "green"
    }
    
    Rectangle {
        id: rect4
        // ✅ 情况 4：width + 锚点冲突
        width: 100              // 显式宽度
        anchors.left: parent.left
        anchors.right: parent.right
        // 结果：锚点优先，width 被忽略
        // 警告：Cannot anchor to an item that isn't a parent or sibling
        color: "yellow"
    }
}
```

### 2.5 实战：创建自适应组件

```qml
// CustomCard.qml - 自适应卡片组件
import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    
    // ✅ 提供默认尺寸
    implicitWidth: Math.max(200, contentColumn.implicitWidth + 40)
    implicitHeight: contentColumn.implicitHeight + 40
    
    // 使用隐式尺寸作为实际尺寸（可被外部覆盖）
    width: implicitWidth
    height: implicitHeight
    
    color: "white"
    border.color: "#e0e0e0"
    border.width: 1
    radius: 8
    
    property alias title: titleLabel.text
    property alias description: descLabel.text
    property alias iconSource: icon.source
    
    Column {
        id: contentColumn
        anchors.centerIn: parent
        spacing: 10
        
        Image {
            id: icon
            width: 48
            height: 48
            anchors.horizontalCenter: parent.horizontalCenter
        }
        
        Label {
            id: titleLabel
            font.pixelSize: 16
            font.bold: true
        }
        
        Label {
            id: descLabel
            font.pixelSize: 12
            color: "#666"
            wrapMode: Text.WordWrap
            width: Math.min(implicitWidth, 300)
        }
    }
}

// 使用示例
Item {
    CustomCard {
        // ✅ 使用默认尺寸
        title: "Card 1"
        description: "Short text"
        iconSource: "icon.png"
    }
    
    CustomCard {
        // ✅ 覆盖尺寸
        width: 400
        title: "Card 2"
        description: "This is a much longer description that will wrap to multiple lines"
        iconSource: "icon.png"
    }
}
```

---

## 3. 布局系统

### 3.1 定位器（Positioners）vs 布局（Layouts）

```qml
import QtQuick
import QtQuick.Layouts

Item {
    width: 800
    height: 600
    
    // === 定位器（Positioners）===
    // 特点：简单、快速、但功能有限
    
    // ✅ Row - 水平排列
    Row {
        spacing: 10
        
        Rectangle { width: 50; height: 50; color: "red" }
        Rectangle { width: 50; height: 50; color: "green" }
        Rectangle { width: 50; height: 50; color: "blue" }
        
        // 限制：
        // - 不能自动填充空间
        // - 不能设置对齐方式（除了 spacing）
        // - 不能设置拉伸因子
    }
    
    // ✅ Column - 垂直排列
    Column {
        spacing: 10
        
        Rectangle { width: 100; height: 30; color: "red" }
        Rectangle { width: 100; height: 30; color: "green" }
        Rectangle { width: 100; height: 30; color: "blue" }
    }
    
    // ✅ Grid - 网格排列
    Grid {
        columns: 3
        spacing: 10
        
        Repeater {
            model: 9
            Rectangle {
                width: 50
                height: 50
                color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
            }
        }
    }
    
    // ✅ Flow - 流式布局
    Flow {
        width: 300
        spacing: 10
        
        Repeater {
            model: 10
            Rectangle {
                width: 60
                height: 40
                color: "lightblue"
                border.color: "blue"
            }
        }
        // 特点：自动换行
    }
}
```


### 3.2 布局系统（Layouts）详解

```qml
import QtQuick
import QtQuick.Layouts

Item {
    width: 800
    height: 600
    
    // === RowLayout - 水平布局 ===
    RowLayout {
        anchors.fill: parent
        spacing: 10
        
        Rectangle {
            color: "red"
            // ✅ Layout 附加属性
            Layout.fillWidth: true          // 填充剩余宽度
            Layout.fillHeight: true         // 填充高度
            Layout.minimumWidth: 100        // 最小宽度
            Layout.maximumWidth: 300        // 最大宽度
            Layout.preferredWidth: 200      // 首选宽度
            Layout.alignment: Qt.AlignCenter // 对齐方式
        }
        
        Rectangle {
            color: "green"
            Layout.preferredWidth: 150
            Layout.fillHeight: true
        }
        
        Rectangle {
            color: "blue"
            Layout.preferredWidth: 100
            Layout.fillHeight: true
        }
    }
    
    // === ColumnLayout - 垂直布局 ===
    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        
        Rectangle {
            color: "red"
            Layout.fillWidth: true
            Layout.preferredHeight: 100
        }
        
        Rectangle {
            color: "green"
            Layout.fillWidth: true
            Layout.fillHeight: true  // 占据剩余空间
        }
        
        Rectangle {
            color: "blue"
            Layout.fillWidth: true
            Layout.preferredHeight: 50
        }
    }
    
    // === GridLayout - 网格布局 ===
    GridLayout {
        anchors.fill: parent
        columns: 3                  // 列数
        rows: 2                     // 行数（可选）
        columnSpacing: 10
        rowSpacing: 10
        
        Rectangle {
            color: "red"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.row: 0           // 指定行
            Layout.column: 0        // 指定列
            Layout.rowSpan: 1       // 跨行数
            Layout.columnSpan: 2    // 跨列数
        }
        
        Rectangle {
            color: "green"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        
        Rectangle {
            color: "blue"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.columnSpan: 3    // 跨 3 列
        }
    }
}
```

### 3.3 Layout 附加属性详解

```qml
RowLayout {
    Rectangle {
        color: "red"
        
        // === 尺寸控制 ===
        Layout.minimumWidth: 50         // 最小宽度（不能小于此值）
        Layout.maximumWidth: 200        // 最大宽度（不能大于此值）
        Layout.preferredWidth: 100      // 首选宽度（理想宽度）
        
        Layout.minimumHeight: 30
        Layout.maximumHeight: 100
        Layout.preferredHeight: 50
        
        // === 填充控制 ===
        Layout.fillWidth: true          // 填充可用宽度
        Layout.fillHeight: false        // 不填充高度
        
        // === 对齐方式 ===
        Layout.alignment: Qt.AlignCenter
        // - Qt.AlignLeft
        // - Qt.AlignRight
        // - Qt.AlignTop
        // - Qt.AlignBottom
        // - Qt.AlignHCenter
        // - Qt.AlignVCenter
        // - Qt.AlignCenter (HCenter | VCenter)
        
        // === 边距 ===
        Layout.leftMargin: 10
        Layout.rightMargin: 10
        Layout.topMargin: 5
        Layout.bottomMargin: 5
        Layout.margins: 10              // 设置所有边距
        
        // === 网格布局专用 ===
        Layout.row: 0                   // 所在行
        Layout.column: 0                // 所在列
        Layout.rowSpan: 1               // 跨行数
        Layout.columnSpan: 1            // 跨列数
    }
}
```

### 3.4 布局尺寸计算规则

```qml
RowLayout {
    width: 500
    spacing: 10
    
    // 尺寸计算优先级：
    // 1. minimumWidth/Height（硬性限制）
    // 2. maximumWidth/Height（硬性限制）
    // 3. preferredWidth/Height（首选值）
    // 4. fillWidth/Height（填充剩余空间）
    // 5. implicitWidth/Height（组件建议值）
    
    Rectangle {
        color: "red"
        // 场景 1：固定尺寸
        Layout.preferredWidth: 100
        Layout.preferredHeight: 50
        // 结果：宽度 = 100
    }
    
    Rectangle {
        color: "green"
        // 场景 2：填充剩余空间
        Layout.fillWidth: true
        Layout.preferredHeight: 50
        // 结果：宽度 = (500 - 100 - 150 - 20) = 230
        //       (总宽度 - 其他固定宽度 - 间距)
    }
    
    Rectangle {
        color: "blue"
        // 场景 3：有最小/最大限制的填充
        Layout.fillWidth: true
        Layout.minimumWidth: 50
        Layout.maximumWidth: 150
        Layout.preferredHeight: 50
        // 结果：宽度 = min(150, 剩余空间)
    }
}
```

### 3.5 嵌套布局

```qml
import QtQuick
import QtQuick.Layouts

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    
    // ✅ 复杂布局：嵌套使用
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10
        
        // 顶部工具栏
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            
            Rectangle {
                color: "lightblue"
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                Text {
                    anchors.centerIn: parent
                    text: "Toolbar"
                }
            }
        }
        
        // 中间内容区（左右分栏）
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            
            // 左侧边栏
            Rectangle {
                color: "lightgray"
                Layout.preferredWidth: 200
                Layout.fillHeight: true
                
                Text {
                    anchors.centerIn: parent
                    text: "Sidebar"
                }
            }
            
            // 右侧主内容区
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10
                
                Rectangle {
                    color: "white"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    Text {
                        anchors.centerIn: parent
                        text: "Main Content"
                    }
                }
                
                Rectangle {
                    color: "lightyellow"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 100
                    
                    Text {
                        anchors.centerIn: parent
                        text: "Bottom Panel"
                    }
                }
            }
        }
        
        // 底部状态栏
        Rectangle {
            color: "lightgreen"
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            
            Text {
                anchors.centerIn: parent
                text: "Status Bar"
            }
        }
    }
}
```


---

## 4. 锚点系统

### 4.1 锚点基础

```qml
Item {
    width: 400
    height: 300
    
    Rectangle {
        id: rect1
        width: 100
        height: 100
        color: "red"
        
        // ✅ 基本锚点
        anchors.left: parent.left           // 左边对齐父容器左边
        anchors.top: parent.top             // 顶部对齐父容器顶部
        anchors.leftMargin: 20              // 左边距
        anchors.topMargin: 20               // 顶边距
    }
    
    Rectangle {
        id: rect2
        width: 100
        height: 100
        color: "blue"
        
        // ✅ 相对于兄弟元素
        anchors.left: rect1.right           // 左边对齐 rect1 的右边
        anchors.top: rect1.top              // 顶部对齐 rect1 的顶部
        anchors.leftMargin: 10
    }
    
    Rectangle {
        id: rect3
        width: 100
        height: 100
        color: "green"
        
        // ✅ 居中对齐
        anchors.centerIn: parent            // 在父容器中居中
        // 等价于：
        // anchors.horizontalCenter: parent.horizontalCenter
        // anchors.verticalCenter: parent.verticalCenter
    }
    
    Rectangle {
        id: rect4
        color: "yellow"
        
        // ✅ 填充父容器
        anchors.fill: parent                // 填充整个父容器
        anchors.margins: 20                 // 所有边距
        // 等价于：
        // anchors.left: parent.left
        // anchors.right: parent.right
        // anchors.top: parent.top
        // anchors.bottom: parent.bottom
    }
}
```

### 4.2 锚点的所有属性

```qml
Rectangle {
    // === 边缘锚点 ===
    anchors.left: parent.left               // 左边
    anchors.right: parent.right             // 右边
    anchors.top: parent.top                 // 顶部
    anchors.bottom: parent.bottom           // 底部
    
    // === 中心锚点 ===
    anchors.horizontalCenter: parent.horizontalCenter  // 水平居中
    anchors.verticalCenter: parent.verticalCenter      // 垂直居中
    anchors.centerIn: parent                           // 完全居中
    
    // === 基线锚点（用于文本对齐）===
    anchors.baseline: otherText.baseline
    
    // === 填充 ===
    anchors.fill: parent                    // 填充目标
    
    // === 边距 ===
    anchors.leftMargin: 10                  // 左边距
    anchors.rightMargin: 10                 // 右边距
    anchors.topMargin: 10                   // 顶边距
    anchors.bottomMargin: 10                // 底边距
    anchors.horizontalCenterOffset: 0       // 水平中心偏移
    anchors.verticalCenterOffset: 0         // 垂直中心偏移
    anchors.baselineOffset: 0               // 基线偏移
    anchors.margins: 10                     // 所有边距
    
    // === 对齐到中心线 ===
    anchors.alignWhenCentered: true         // 居中时对齐到整数像素
}
```

### 4.3 锚点常见陷阱

```qml
Item {
    width: 400
    height: 300
    
    Rectangle {
        id: rect1
        color: "red"
        
        // ❌ 陷阱 1：锚点与显式尺寸冲突
        width: 100                          // 显式宽度
        anchors.left: parent.left
        anchors.right: parent.right         // 冲突！
        // 结果：width 被忽略，使用锚点确定的宽度
        // 警告：Cannot specify left, right, and width at the same time
        
        // ✅ 正确做法：只用锚点或只用 width
        // 方式 1：只用锚点
        anchors.left: parent.left
        anchors.right: parent.right
        
        // 方式 2：只用 width
        width: 100
        anchors.left: parent.left
    }
    
    Rectangle {
        id: rect2
        color: "blue"
        
        // ❌ 陷阱 2：循环依赖
        anchors.left: rect3.right
        
        Rectangle {
            id: rect3
            anchors.left: rect2.right       // 循环！
            // 错误：QML Rectangle: Binding loop detected
        }
        
        // ✅ 正确做法：避免循环引用
    }
    
    Rectangle {
        id: rect4
        color: "green"
        
        // ❌ 陷阱 3：锚点到非父/兄弟元素
        Item {
            id: container
            Rectangle {
                id: innerRect
            }
        }
        
        // anchors.left: innerRect.left     // 错误！
        // 警告：Cannot anchor to an item that isn't a parent or sibling
        
        // ✅ 正确做法：只能锚点到父元素或兄弟元素
        anchors.left: container.left        // OK（兄弟）
        anchors.left: parent.left           // OK（父）
    }
    
    Rectangle {
        id: rect5
        color: "yellow"
        
        // ❌ 陷阱 4：fill 与其他锚点混用
        anchors.fill: parent
        anchors.topMargin: 20               // OK
        anchors.top: parent.top             // 冲突！
        // fill 已经设置了 top，不能再单独设置
    }
}
```

### 4.4 动态锚点

```qml
Item {
    width: 400
    height: 300
    
    Rectangle {
        id: movableRect
        width: 100
        height: 100
        color: "red"
        
        // ✅ 根据条件改变锚点
        states: [
            State {
                name: "left"
                AnchorChanges {
                    target: movableRect
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                }
            },
            State {
                name: "right"
                AnchorChanges {
                    target: movableRect
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        ]
        
        transitions: Transition {
            AnchorAnimation {
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }
        
        MouseArea {
            anchors.fill: parent
            onClicked: {
                movableRect.state = (movableRect.state === "left") ? "right" : "left"
            }
        }
    }
}
```

### 4.5 锚点 vs 布局 vs 定位

```qml
Item {
    width: 600
    height: 400
    
    // === 方式 1：使用锚点 ===
    Rectangle {
        id: method1
        color: "red"
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: parent.verticalCenter
        anchors.margins: 10
        
        // 优点：
        // - 灵活，可以精确控制位置
        // - 性能好
        // - 支持动画
        // 缺点：
        // - 代码较多
        // - 复杂布局难以维护
    }
    
    // === 方式 2：使用布局 ===
    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        Rectangle {
            color: "green"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        
        Rectangle {
            color: "blue"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        
        // 优点：
        // - 自动计算尺寸
        // - 响应式设计
        // - 代码简洁
        // 缺点：
        // - 性能略低于锚点
        // - 灵活性较低
    }
    
    // === 方式 3：使用绝对定位 ===
    Rectangle {
        color: "yellow"
        x: 10
        y: 10
        width: 100
        height: 100
        
        // 优点：
        // - 最简单
        // - 性能最好
        // 缺点：
        // - 不响应父容器尺寸变化
        // - 不适合响应式设计
    }
}
```


---

## 5. 属性绑定机制

### 5.1 属性绑定基础

```qml
Item {
    width: 400
    height: 300
    
    Rectangle {
        id: rect1
        width: 100
        height: 100
        color: "red"
    }
    
    Rectangle {
        id: rect2
        // ✅ 属性绑定：自动更新
        width: rect1.width * 2              // 绑定到 rect1.width
        height: rect1.height * 2
        color: "blue"
        
        // 当 rect1.width 改变时，rect2.width 自动更新
    }
    
    Text {
        // ✅ 复杂表达式绑定
        text: "Rect1: " + rect1.width + "x" + rect1.height
        // 任何依赖的属性改变，text 都会自动更新
    }
    
    Rectangle {
        id: rect3
        // ✅ 条件绑定
        color: rect1.width > 150 ? "green" : "yellow"
        width: Math.max(rect1.width, rect2.width)
        height: 50
    }
}
```

### 5.2 绑定的破坏与恢复

```qml
Item {
    Rectangle {
        id: rect
        width: 100
        height: 100
        color: "red"
        
        // ✅ 初始绑定
        x: parent.width / 2 - width / 2
        
        MouseArea {
            anchors.fill: parent
            onClicked: {
                // ❌ 直接赋值会破坏绑定
                rect.x = 200
                // 现在 rect.x 不再跟随 parent.width 变化
            }
        }
    }
    
    Rectangle {
        id: rect2
        width: 100
        height: 100
        color: "blue"
        
        // ✅ 初始绑定
        x: parent.width / 2 - width / 2
        
        MouseArea {
            anchors.fill: parent
            onClicked: {
                // ✅ 使用 Qt.binding() 保持绑定
                rect2.x = Qt.binding(function() {
                    return parent.width / 2 - rect2.width / 2
                })
            }
        }
    }
    
    Rectangle {
        id: rect3
        width: 100
        height: 100
        color: "green"
        
        property bool centered: true
        
        // ✅ 使用属性控制绑定
        x: centered ? parent.width / 2 - width / 2 : 0
        
        MouseArea {
            anchors.fill: parent
            onClicked: {
                // ✅ 改变控制属性，不破坏绑定
                rect3.centered = !rect3.centered
            }
        }
    }
}
```

### 5.3 单向绑定 vs 双向绑定

```qml
import QtQuick
import QtQuick.Controls

Item {
    // ✅ 单向绑定（默认）
    Slider {
        id: slider
        from: 0
        to: 100
        value: 50
    }
    
    Text {
        // 单向：Text 跟随 Slider，但 Slider 不跟随 Text
        text: "Value: " + slider.value
    }
    
    // ✅ 双向绑定（使用 Binding）
    Rectangle {
        id: rect
        width: 200
        height: 100
        color: "lightblue"
        
        property real myValue: 50
        
        Slider {
            id: slider2
            anchors.centerIn: parent
            from: 0
            to: 100
            value: rect.myValue         // 初始值来自 rect.myValue
        }
        
        // 双向绑定
        Binding {
            target: rect
            property: "myValue"
            value: slider2.value
            // 现在：slider2 改变 → rect.myValue 改变
            //      rect.myValue 改变 → slider2 改变
        }
        
        Text {
            anchors.top: slider2.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Value: " + rect.myValue
        }
    }
}
```

### 5.4 延迟绑定与 Binding 对象

```qml
Item {
    Rectangle {
        id: rect
        width: 100
        height: 100
        color: "red"
        
        property bool followMouse: false
        
        // ✅ 使用 Binding 对象控制绑定
        Binding {
            target: rect
            property: "x"
            value: mouseArea.mouseX - rect.width / 2
            when: rect.followMouse      // 条件绑定
            // 只有当 followMouse 为 true 时，绑定才生效
        }
        
        Binding {
            target: rect
            property: "y"
            value: mouseArea.mouseY - rect.height / 2
            when: rect.followMouse
        }
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        
        onClicked: rect.followMouse = !rect.followMouse
    }
    
    Text {
        text: rect.followMouse ? "Following mouse" : "Click to follow"
        anchors.bottom: parent.bottom
    }
}
```

### 5.5 绑定循环检测

```qml
Item {
    Rectangle {
        id: rect1
        // ❌ 绑定循环示例
        width: rect2.width + 10
        height: 100
        color: "red"
    }
    
    Rectangle {
        id: rect2
        width: rect1.width + 10         // 循环！
        height: 100
        color: "blue"
        
        // 错误：QML Rectangle: Binding loop detected for property "width"
    }
    
    // ✅ 解决方案 1：使用中间变量
    property int baseWidth: 100
    
    Rectangle {
        id: rect3
        width: baseWidth
        height: 100
        color: "green"
    }
    
    Rectangle {
        id: rect4
        width: baseWidth + 20
        height: 100
        color: "yellow"
    }
    
    // ✅ 解决方案 2：重新设计逻辑
    Rectangle {
        id: rect5
        width: 100
        height: 100
        color: "purple"
        
        Rectangle {
            id: rect6
            width: parent.width + 20    // 依赖父元素，不循环
            height: 100
            color: "orange"
        }
    }
}
```

### 5.6 属性别名（Property Alias）

```qml
// CustomButton.qml
import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    width: 120
    height: 40
    color: mouseArea.pressed ? "#0066cc" : "#0080ff"
    radius: 5
    
    // ✅ 属性别名：暴露内部属性
    property alias text: label.text
    property alias textColor: label.color
    property alias fontSize: label.font.pixelSize
    
    // ✅ 信号别名
    signal clicked()
    
    Text {
        id: label
        anchors.centerIn: parent
        color: "white"
        font.pixelSize: 14
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: root.clicked()
    }
}

// 使用
Item {
    CustomButton {
        // ✅ 直接访问别名属性
        text: "Click Me"
        textColor: "yellow"
        fontSize: 16
        
        onClicked: console.log("Button clicked!")
    }
}
```

### 5.7 属性绑定性能优化

```qml
Item {
    // ❌ 性能问题：频繁计算复杂表达式
    Rectangle {
        id: badExample
        width: 100
        height: 100
        
        // 每次 parent.width 改变都会重新计算整个表达式
        color: Qt.rgba(
            Math.sin(parent.width / 100),
            Math.cos(parent.height / 100),
            Math.tan(parent.width / parent.height),
            1.0
        )
    }
    
    // ✅ 优化：使用中间属性缓存结果
    Rectangle {
        id: goodExample
        width: 100
        height: 100
        
        property real hue: parent.width / 360
        property real saturation: 0.8
        property real lightness: 0.5
        
        color: Qt.hsla(hue, saturation, lightness, 1.0)
        // 只有 hue 改变时才重新计算颜色
    }
    
    // ✅ 优化：避免不必要的绑定
    Rectangle {
        id: optimized
        width: 100
        height: 100
        
        // 如果值不会改变，直接赋值而不是绑定
        color: "#ff0000"                // 静态值
        // 而不是：color: Qt.rgba(1, 0, 0, 1)  // 不必要的绑定
    }
}
```


---

## 6. 信号与槽

### 6.1 信号基础

```qml
// CustomComponent.qml
import QtQuick

Rectangle {
    id: root
    width: 100
    height: 100
    color: "lightblue"
    
    // ✅ 声明自定义信号
    signal clicked()                        // 无参数信号
    signal valueChanged(int newValue)       // 带参数信号
    signal dataReady(string data, bool success)  // 多参数信号
    
    MouseArea {
        anchors.fill: parent
        onClicked: {
            // ✅ 发射信号
            root.clicked()
            root.valueChanged(42)
            root.dataReady("Hello", true)
        }
    }
}

// 使用
Item {
    CustomComponent {
        // ✅ 连接信号（方式 1：on<SignalName>）
        onClicked: {
            console.log("Clicked!")
        }
        
        onValueChanged: function(newValue) {
            console.log("Value:", newValue)
        }
        
        onDataReady: function(data, success) {
            console.log("Data:", data, "Success:", success)
        }
    }
    
    CustomComponent {
        id: component2
    }
    
    // ✅ 连接信号（方式 2：Connections）
    Connections {
        target: component2
        
        function onClicked() {
            console.log("Component2 clicked!")
        }
        
        function onValueChanged(newValue) {
            console.log("Component2 value:", newValue)
        }
    }
}
```

### 6.2 信号处理器命名规则

```qml
Rectangle {
    // 信号名：clicked
    // 处理器名：onClicked（首字母大写，加 on 前缀）
    
    signal mySignal()           // 信号
    onMySignal: { }             // 处理器
    
    signal dataReady()
    onDataReady: { }
    
    signal valueChanged()
    onValueChanged: { }
    
    // ❌ 常见错误
    signal mySignal()
    onmySignal: { }             // 错误：首字母应该大写
    onMySignalHandler: { }      // 错误：不需要 Handler 后缀
}
```

### 6.3 信号参数

```qml
Rectangle {
    id: root
    
    // ✅ 带参数的信号
    signal itemSelected(int index, string name, var data)
    
    MouseArea {
        anchors.fill: parent
        onClicked: {
            // 发射信号时传递参数
            root.itemSelected(0, "Item 1", {key: "value"})
        }
    }
}

Item {
    Rectangle {
        id: myRect
        
        onItemSelected: function(index, name, data) {
            console.log("Index:", index)
            console.log("Name:", name)
            console.log("Data:", JSON.stringify(data))
        }
    }
    
    // ✅ 使用 Connections 时的参数
    Connections {
        target: myRect
        
        function onItemSelected(index, name, data) {
            // 参数名必须与信号声明一致
            console.log(index, name, data)
        }
    }
}
```

### 6.4 connect() 和 disconnect()

```qml
import QtQuick

Item {
    Rectangle {
        id: sender
        signal mySignal(string message)
        
        MouseArea {
            anchors.fill: parent
            onClicked: sender.mySignal("Hello")
        }
    }
    
    Component.onCompleted: {
        // ✅ 动态连接信号
        sender.mySignal.connect(function(message) {
            console.log("Received:", message)
        })
        
        // ✅ 连接到另一个对象的方法
        sender.mySignal.connect(receiver.handleMessage)
        
        // ✅ 断开连接
        // sender.mySignal.disconnect(receiver.handleMessage)
    }
    
    QtObject {
        id: receiver
        
        function handleMessage(message) {
            console.log("Receiver got:", message)
        }
    }
}
```

### 6.5 信号转发

```qml
// ParentComponent.qml
import QtQuick

Rectangle {
    id: root
    width: 200
    height: 200
    
    // ✅ 声明信号
    signal buttonClicked()
    
    Rectangle {
        id: button
        width: 100
        height: 40
        color: "lightblue"
        
        signal clicked()
        
        MouseArea {
            anchors.fill: parent
            onClicked: button.clicked()
        }
        
        // ✅ 转发内部信号到外部
        onClicked: root.buttonClicked()
    }
}

// 使用
Item {
    ParentComponent {
        onButtonClicked: {
            console.log("Button in parent was clicked!")
        }
    }
}
```

---

## 7. 组件生命周期

### 7.1 生命周期信号

```qml
import QtQuick

Rectangle {
    id: root
    width: 200
    height: 200
    color: "lightblue"
    
    // ✅ Component.onCompleted - 组件创建完成
    Component.onCompleted: {
        console.log("1. Component completed")
        console.log("   - 所有属性已初始化")
        console.log("   - 所有子组件已创建")
        console.log("   - 可以安全访问所有属性")
        
        // 适合做初始化工作
        loadData()
        startTimer()
    }
    
    // ✅ Component.onDestruction - 组件即将销毁
    Component.onDestruction: {
        console.log("2. Component destruction")
        console.log("   - 组件即将被销毁")
        console.log("   - 清理资源的最后机会")
        
        // 适合做清理工作
        saveData()
        stopTimer()
    }
    
    function loadData() {
        console.log("Loading data...")
    }
    
    function saveData() {
        console.log("Saving data...")
    }
    
    function startTimer() {
        console.log("Starting timer...")
    }
    
    function stopTimer() {
        console.log("Stopping timer...")
    }
}
```

### 7.2 属性初始化顺序

```qml
Rectangle {
    id: root
    
    // 初始化顺序：
    // 1. 属性声明和默认值
    property int value1: 10
    property int value2: value1 * 2     // 可以引用之前声明的属性
    
    // 2. 绑定表达式
    width: value1 * 10
    height: value2 * 5
    
    // 3. 子组件创建
    Rectangle {
        id: child
        width: parent.width / 2
        height: parent.height / 2
    }
    
    // 4. Component.onCompleted（最后执行）
    Component.onCompleted: {
        console.log("All properties initialized")
        console.log("value1:", value1)      // 10
        console.log("value2:", value2)      // 20
        console.log("width:", width)        // 100
        console.log("child.width:", child.width)  // 50
    }
}
```

### 7.3 动态创建和销毁

```qml
import QtQuick

Item {
    id: root
    width: 400
    height: 300
    
    property var dynamicObjects: []
    
    // ✅ 方式 1：使用 Qt.createComponent() 和 createObject()
    function createDynamic1() {
        var component = Qt.createComponent("DynamicItem.qml")
        
        if (component.status === Component.Ready) {
            var obj = component.createObject(root, {
                x: Math.random() * 300,
                y: Math.random() * 200,
                color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
            })
            
            if (obj === null) {
                console.log("Error creating object")
            } else {
                dynamicObjects.push(obj)
            }
        } else if (component.status === Component.Error) {
            console.log("Error loading component:", component.errorString())
        }
    }
    
    // ✅ 方式 2：使用 Component 元素
    Component {
        id: dynamicComponent
        
        Rectangle {
            width: 50
            height: 50
            color: "red"
            
            MouseArea {
                anchors.fill: parent
                onClicked: parent.destroy()  // 点击销毁自己
            }
        }
    }
    
    function createDynamic2() {
        var obj = dynamicComponent.createObject(root, {
            x: Math.random() * 300,
            y: Math.random() * 200
        })
        dynamicObjects.push(obj)
    }
    
    // ✅ 销毁对象
    function destroyAll() {
        for (var i = 0; i < dynamicObjects.length; i++) {
            dynamicObjects[i].destroy()  // 销毁对象
        }
        dynamicObjects = []
    }
    
    // ✅ 延迟销毁
    function destroyWithDelay(obj) {
        obj.destroy(1000)  // 1 秒后销毁
    }
    
    Row {
        spacing: 10
        
        Button {
            text: "Create (Method 1)"
            onClicked: root.createDynamic1()
        }
        
        Button {
            text: "Create (Method 2)"
            onClicked: root.createDynamic2()
        }
        
        Button {
            text: "Destroy All"
            onClicked: root.destroyAll()
        }
    }
}
```

### 7.4 Loader 动态加载

```qml
import QtQuick
import QtQuick.Controls

Item {
    width: 400
    height: 300
    
    // ✅ Loader - 延迟加载和动态切换
    Loader {
        id: loader
        anchors.centerIn: parent
        
        // 方式 1：加载 QML 文件
        source: "HeavyComponent.qml"
        
        // 方式 2：加载 Component
        // sourceComponent: myComponent
        
        // 控制加载
        active: true                    // false 时不加载
        asynchronous: true              // 异步加载（不阻塞 UI）
        
        // 加载状态
        onStatusChanged: {
            if (loader.status === Loader.Ready) {
                console.log("Loaded successfully")
                // 访问加载的对象
                loader.item.someProperty = "value"
            } else if (loader.status === Loader.Error) {
                console.log("Load error")
            }
        }
        
        // 加载进度
        onProgressChanged: {
            console.log("Loading:", loader.progress * 100 + "%")
        }
    }
    
    Component {
        id: myComponent
        Rectangle {
            width: 200
            height: 100
            color: "lightblue"
            property string someProperty: ""
        }
    }
    
    Row {
        anchors.bottom: parent.bottom
        spacing: 10
        
        Button {
            text: "Load Component"
            onClicked: {
                loader.sourceComponent = myComponent
            }
        }
        
        Button {
            text: "Load File"
            onClicked: {
                loader.source = "HeavyComponent.qml"
            }
        }
        
        Button {
            text: "Unload"
            onClicked: {
                loader.active = false
                // 或 loader.source = ""
                // 或 loader.sourceComponent = undefined
            }
        }
    }
}
```


---

## 8. 性能优化

### 8.1 避免不必要的绑定

```qml
Item {
    // ❌ 性能问题：不必要的绑定
    Rectangle {
        width: 100 + 0              // 不必要的表达式
        height: parent.height * 1   // 不必要的计算
        color: true ? "red" : "blue"  // 静态条件
    }
    
    // ✅ 优化：使用静态值
    Rectangle {
        width: 100
        height: parent.height
        color: "red"
    }
    
    // ❌ 性能问题：复杂的绑定表达式
    Text {
        text: "Value: " + (Math.sin(slider.value) * 100).toFixed(2) + " units"
        // 每次 slider.value 改变都会重新计算整个表达式
    }
    
    // ✅ 优化：分解复杂表达式
    property real calculatedValue: Math.sin(slider.value) * 100
    Text {
        text: "Value: " + calculatedValue.toFixed(2) + " units"
    }
}
```

### 8.2 使用 Loader 延迟加载

```qml
Item {
    // ❌ 性能问题：一次性加载所有内容
    TabBar {
        id: tabBar
        TabButton { text: "Tab 1" }
        TabButton { text: "Tab 2" }
        TabButton { text: "Tab 3" }
    }
    
    StackLayout {
        currentIndex: tabBar.currentIndex
        
        // 所有页面都会立即创建，即使不可见
        HeavyPage1 { }
        HeavyPage2 { }
        HeavyPage3 { }
    }
    
    // ✅ 优化：使用 Loader 延迟加载
    StackLayout {
        currentIndex: tabBar.currentIndex
        
        Loader {
            active: StackLayout.index === StackLayout.currentIndex
            sourceComponent: HeavyPage1 { }
        }
        
        Loader {
            active: StackLayout.index === StackLayout.currentIndex
            sourceComponent: HeavyPage2 { }
        }
        
        Loader {
            active: StackLayout.index === StackLayout.currentIndex
            sourceComponent: HeavyPage3 { }
        }
    }
}
```

### 8.3 ListView 性能优化

```qml
import QtQuick

Item {
    // ✅ ListView 性能优化技巧
    ListView {
        id: listView
        anchors.fill: parent
        
        model: 10000  // 大量数据
        
        // 1. 使用缓存
        cacheBuffer: 500            // 缓存屏幕外的项目（像素）
        
        // 2. 异步创建
        asynchronous: true          // 异步创建委托
        
        // 3. 限制高亮动画
        highlightMoveDuration: 0    // 禁用高亮移动动画（如果不需要）
        
        // 4. 简化委托
        delegate: Rectangle {
            width: listView.width
            height: 50
            color: index % 2 ? "#f0f0f0" : "white"
            
            // ✅ 使用简单的 Text 而不是 Label
            Text {
                anchors.centerIn: parent
                text: "Item " + index
                // 避免复杂的绑定和计算
            }
            
            // ❌ 避免在委托中使用复杂组件
            // Label { }  // Label 比 Text 重
            // Image { }  // 如果不需要，避免图片
        }
        
        // 5. 使用 section 分组（如果需要）
        section.property: "category"
        section.criteria: ViewSection.FullString
        section.delegate: Rectangle {
            width: listView.width
            height: 30
            color: "lightgray"
            Text {
                anchors.centerIn: parent
                text: section
                font.bold: true
            }
        }
    }
}
```

### 8.4 避免过度使用 anchors

```qml
Item {
    // ❌ 性能问题：过度使用 anchors
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 10
        anchors.bottomMargin: 10
    }
    
    // ✅ 优化：使用 anchors.fill + margins
    Rectangle {
        anchors.fill: parent
        anchors.margins: 10
    }
    
    // ✅ 或使用显式尺寸（性能最好）
    Rectangle {
        x: 10
        y: 10
        width: parent.width - 20
        height: parent.height - 20
    }
}
```

### 8.5 使用 FastBlur 替代 GaussianBlur

```qml
import QtQuick
import QtGraphicalEffects

Item {
    Image {
        id: sourceImage
        source: "image.png"
    }
    
    // ❌ 性能较差：GaussianBlur
    GaussianBlur {
        anchors.fill: sourceImage
        source: sourceImage
        radius: 8
        samples: 16
        // 高质量但慢
    }
    
    // ✅ 性能更好：FastBlur
    FastBlur {
        anchors.fill: sourceImage
        source: sourceImage
        radius: 32
        // 质量略低但快得多
    }
}
```

### 8.6 减少状态变化

```qml
Item {
    Rectangle {
        id: rect
        width: 100
        height: 100
        
        // ❌ 性能问题：频繁的状态变化
        states: [
            State {
                name: "state1"
                PropertyChanges { target: rect; color: "red" }
            },
            State {
                name: "state2"
                PropertyChanges { target: rect; color: "blue" }
            }
        ]
        
        Timer {
            interval: 16  // 60 FPS
            running: true
            repeat: true
            onTriggered: {
                rect.state = (rect.state === "state1") ? "state2" : "state1"
                // 频繁切换状态会影响性能
            }
        }
    }
    
    // ✅ 优化：直接修改属性
    Rectangle {
        id: rect2
        width: 100
        height: 100
        
        property bool toggle: false
        color: toggle ? "red" : "blue"
        
        Timer {
            interval: 16
            running: true
            repeat: true
            onTriggered: rect2.toggle = !rect2.toggle
        }
    }
}
```

---

## 9. 常见陷阱与解决方案

### 9.1 陷阱：undefined 属性访问

```qml
Item {
    property var myObject: null
    
    // ❌ 陷阱：访问 null/undefined 的属性
    Text {
        text: myObject.name  // 错误！myObject 是 null
        // TypeError: Cannot read property 'name' of null
    }
    
    // ✅ 解决方案 1：使用条件运算符
    Text {
        text: myObject ? myObject.name : "N/A"
    }
    
    // ✅ 解决方案 2：使用默认值
    Text {
        text: (myObject && myObject.name) || "Unknown"
    }
    
    // ✅ 解决方案 3：使用 Binding
    Text {
        id: nameText
        text: "N/A"
        
        Binding {
            target: nameText
            property: "text"
            value: myObject.name
            when: myObject !== null
        }
    }
}
```

### 9.2 陷阱：JavaScript 数组和对象的响应性

```qml
Item {
    // ❌ 陷阱：修改数组/对象不会触发绑定更新
    property var myArray: [1, 2, 3]
    property var myObject: {name: "John", age: 30}
    
    Text {
        text: "Array length: " + myArray.length
        // 修改 myArray 不会更新这个 Text
    }
    
    Component.onCompleted: {
        myArray.push(4)         // 不会触发更新
        myObject.age = 31       // 不会触发更新
    }
    
    // ✅ 解决方案：重新赋值整个数组/对象
    function addToArray(value) {
        var temp = myArray.slice()  // 复制数组
        temp.push(value)
        myArray = temp              // 重新赋值，触发更新
    }
    
    function updateObject(key, value) {
        var temp = Object.assign({}, myObject)  // 复制对象
        temp[key] = value
        myObject = temp             // 重新赋值，触发更新
    }
    
    // ✅ 或使用 ListModel（对于数组）
    ListModel {
        id: listModel
        ListElement { value: 1 }
        ListElement { value: 2 }
        ListElement { value: 3 }
    }
    
    Text {
        text: "Model count: " + listModel.count
        // 修改 listModel 会自动更新
    }
    
    Component.onCompleted: {
        listModel.append({value: 4})  // 会触发更新
    }
}
```

### 9.3 陷阱：父子组件的 id 作用域

```qml
// ParentComponent.qml
Item {
    id: parent
    
    Rectangle {
        id: childRect
        width: 100
        height: 100
    }
    
    // ✅ 可以访问子组件的 id
    Text {
        text: "Child width: " + childRect.width
    }
}

// Main.qml
Item {
    ParentComponent {
        id: myParent
    }
    
    // ❌ 陷阱：不能直接访问孙子组件的 id
    Text {
        // text: "Width: " + childRect.width  // 错误！childRect 不在作用域内
    }
    
    // ✅ 解决方案：通过属性暴露
    // 修改 ParentComponent.qml
    // property alias childWidth: childRect.width
    
    Text {
        text: "Width: " + myParent.childWidth
    }
}
```

### 9.4 陷阱：Timer 的 running 属性

```qml
Item {
    // ❌ 陷阱：Timer 默认不运行
    Timer {
        interval: 1000
        repeat: true
        onTriggered: console.log("Tick")
        // 不会执行！因为 running 默认为 false
    }
    
    // ✅ 解决方案：设置 running: true
    Timer {
        interval: 1000
        repeat: true
        running: true  // 必须设置
        onTriggered: console.log("Tick")
    }
    
    // ✅ 或在 Component.onCompleted 中启动
    Timer {
        id: myTimer
        interval: 1000
        repeat: true
        onTriggered: console.log("Tick")
    }
    
    Component.onCompleted: {
        myTimer.start()
    }
}
```

### 9.5 陷阱：MouseArea 的事件传播

```qml
Item {
    Rectangle {
        width: 200
        height: 200
        color: "lightblue"
        
        MouseArea {
            anchors.fill: parent
            onClicked: console.log("Parent clicked")
            
            Rectangle {
                width: 100
                height: 100
                anchors.centerIn: parent
                color: "red"
                
                // ❌ 陷阱：子 MouseArea 会阻止父 MouseArea
                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("Child clicked")
                    // 点击红色区域只会输出 "Child clicked"
                }
            }
        }
    }
    
    // ✅ 解决方案：使用 propagateComposedEvents
    Rectangle {
        width: 200
        height: 200
        color: "lightgreen"
        
        MouseArea {
            anchors.fill: parent
            onClicked: console.log("Parent clicked")
            
            Rectangle {
                width: 100
                height: 100
                anchors.centerIn: parent
                color: "blue"
                
                MouseArea {
                    anchors.fill: parent
                    propagateComposedEvents: true  // 允许事件传播
                    onClicked: {
                        console.log("Child clicked")
                        mouse.accepted = false  // 不接受事件，传递给父级
                    }
                }
            }
        }
    }
}
```

### 9.6 陷阱：Image 的异步加载

```qml
Item {
    // ❌ 陷阱：假设图片立即加载完成
    Image {
        id: myImage
        source: "large_image.png"
    }
    
    Component.onCompleted: {
        console.log("Width:", myImage.width)  // 可能是 0！
        // 图片可能还没加载完成
    }
    
    // ✅ 解决方案：监听 status
    Image {
        id: myImage2
        source: "large_image.png"
        asynchronous: true  // 异步加载
        
        onStatusChanged: {
            if (status === Image.Ready) {
                console.log("Image loaded, width:", width)
            } else if (status === Image.Error) {
                console.log("Image load error")
            }
        }
    }
    
    // ✅ 显示加载状态
    Image {
        id: myImage3
        source: "large_image.png"
        asynchronous: true
        
        BusyIndicator {
            anchors.centerIn: parent
            running: myImage3.status === Image.Loading
        }
        
        Text {
            anchors.centerIn: parent
            text: "Load Error"
            visible: myImage3.status === Image.Error
        }
    }
}
```


### 9.7 陷阱：z-order 和层叠顺序

```qml
Item {
    width: 400
    height: 300
    
    // QML 的 z-order 规则：
    // 1. 后声明的元素在上层
    // 2. z 属性值大的在上层
    
    Rectangle {
        id: rect1
        x: 50
        y: 50
        width: 100
        height: 100
        color: "red"
        z: 0  // 默认 z 值
    }
    
    Rectangle {
        id: rect2
        x: 100
        y: 100
        width: 100
        height: 100
        color: "blue"
        z: 0
        // rect2 在 rect1 上层（后声明）
    }
    
    Rectangle {
        id: rect3
        x: 150
        y: 150
        width: 100
        height: 100
        color: "green"
        z: -1
        // rect3 在最下层（z 值最小）
    }
    
    // ✅ 动态改变层叠顺序
    MouseArea {
        anchors.fill: parent
        onClicked: {
            // 点击时将 rect1 提到最上层
            rect1.z = Math.max(rect2.z, rect3.z) + 1
        }
    }
}
```

### 9.8 陷阱：颜色字符串格式

```qml
Rectangle {
    // ✅ 正确的颜色格式
    color: "#ff0000"                    // 十六进制 RGB
    color: "#80ff0000"                  // 十六进制 ARGB（带透明度）
    color: "red"                        // 颜色名称
    color: Qt.rgba(1, 0, 0, 1)         // RGBA 函数
    color: Qt.hsla(0, 1, 0.5, 1)       // HSLA 函数
    
    // ❌ 常见错误
    // color: "ff0000"                  // 错误：缺少 #
    // color: "#ff00"                   // 错误：格式不对
    // color: "rgb(255, 0, 0)"          // 错误：不支持 CSS 格式
    
    // ✅ 动态颜色
    property real hue: 0.5
    color: Qt.hsla(hue, 1, 0.5, 1)
    
    // ✅ 颜色混合
    color: Qt.lighter("red", 1.5)       // 变亮
    color: Qt.darker("blue", 1.5)       // 变暗
    color: Qt.tint("white", "#80ff0000") // 混合颜色
}
```

### 9.9 陷阱：文本截断和换行

```qml
Rectangle {
    width: 200
    height: 100
    
    // ❌ 陷阱：文本超出边界
    Text {
        text: "This is a very long text that will overflow the container"
        // 文本会超出 Rectangle 边界
    }
    
    // ✅ 解决方案 1：设置宽度和换行
    Text {
        width: parent.width
        text: "This is a very long text that will wrap to multiple lines"
        wrapMode: Text.WordWrap
        // - Text.NoWrap: 不换行（默认）
        // - Text.WordWrap: 单词边界换行
        // - Text.WrapAnywhere: 任意位置换行
        // - Text.Wrap: 优先单词边界，必要时任意位置
    }
    
    // ✅ 解决方案 2：截断文本
    Text {
        width: parent.width
        text: "This is a very long text that will be truncated"
        elide: Text.ElideRight
        // - Text.ElideNone: 不截断
        // - Text.ElideLeft: 左侧截断
        // - Text.ElideMiddle: 中间截断
        // - Text.ElideRight: 右侧截断（显示 ...）
    }
    
    // ✅ 解决方案 3：限制最大行数
    Text {
        width: parent.width
        text: "This is a very long text that will wrap but limited to 2 lines"
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        elide: Text.ElideRight  // 超出部分显示 ...
    }
}
```

### 9.10 陷阱：Repeater 的 model 更新

```qml
Item {
    // ❌ 陷阱：修改数组不会更新 Repeater
    property var items: [1, 2, 3]
    
    Column {
        Repeater {
            model: items
            delegate: Text {
                text: "Item: " + modelData
            }
        }
    }
    
    Component.onCompleted: {
        items.push(4)  // 不会更新 Repeater！
    }
    
    // ✅ 解决方案 1：重新赋值数组
    function addItem(value) {
        var temp = items.slice()
        temp.push(value)
        items = temp  // 触发更新
    }
    
    // ✅ 解决方案 2：使用 ListModel
    ListModel {
        id: listModel
        ListElement { value: 1 }
        ListElement { value: 2 }
        ListElement { value: 3 }
    }
    
    Column {
        Repeater {
            model: listModel
            delegate: Text {
                text: "Item: " + model.value
            }
        }
    }
    
    Component.onCompleted: {
        listModel.append({value: 4})  // 会自动更新
    }
}
```

---

## 10. 快速参考

### 10.1 尺寸属性对比

| 属性 | 说明 | 何时使用 |
|------|------|---------|
| `width/height` | 显式尺寸 | 需要固定大小时 |
| `implicitWidth/Height` | 隐式尺寸（建议） | 提供默认尺寸，可被覆盖 |
| `Layout.preferredWidth/Height` | 布局首选尺寸 | 在布局中使用 |
| `Layout.minimumWidth/Height` | 布局最小尺寸 | 限制最小值 |
| `Layout.maximumWidth/Height` | 布局最大尺寸 | 限制最大值 |
| `anchors.fill` | 填充目标 | 填充父容器或其他元素 |

### 10.2 常用信号

```qml
// 鼠标事件
MouseArea {
    onClicked: { }
    onPressed: { }
    onReleased: { }
    onDoubleClicked: { }
    onEntered: { }
    onExited: { }
    onPositionChanged: { }
}

// 键盘事件
Item {
    focus: true
    Keys.onPressed: function(event) { }
    Keys.onReleased: function(event) { }
}

// 组件生命周期
Component.onCompleted: { }
Component.onDestruction: { }

// 属性变化
onWidthChanged: { }
onHeightChanged: { }
onVisibleChanged: { }
```

### 10.3 性能优化清单

- [ ] 使用 `Loader` 延迟加载重型组件
- [ ] ListView 设置 `cacheBuffer` 和 `asynchronous`
- [ ] 避免复杂的属性绑定表达式
- [ ] 使用 `FastBlur` 替代 `GaussianBlur`
- [ ] 减少不必要的 `anchors` 使用
- [ ] 使用 `visible: false` 而不是 `opacity: 0`（如果不需要动画）
- [ ] 避免在委托中使用复杂组件
- [ ] 使用 `Image.asynchronous: true` 异步加载图片
- [ ] 避免频繁的状态切换
- [ ] 使用 `Qt.createQmlObject()` 而不是 `eval()`

### 10.4 调试技巧

```qml
Item {
    // 1. 使用 console.log
    Component.onCompleted: {
        console.log("Value:", someValue)
        console.warn("Warning message")
        console.error("Error message")
    }
    
    // 2. 打印对象信息
    onClicked: {
        console.log(JSON.stringify(myObject, null, 2))
    }
    
    // 3. 使用 debugger 语句（需要调试器）
    onClicked: {
        debugger;  // 断点
    }
    
    // 4. 可视化边界
    Rectangle {
        border.color: "red"
        border.width: 1
        color: "transparent"
    }
    
    // 5. 显示 FPS
    Text {
        text: "FPS: " + (1000 / frameTime).toFixed(1)
        property real frameTime: 16
        
        Timer {
            interval: 16
            running: true
            repeat: true
            property real lastTime: Date.now()
            onTriggered: {
                var now = Date.now()
                parent.frameTime = now - lastTime
                lastTime = now
            }
        }
    }
}
```

---

## 11. 最佳实践总结

### 11.1 组件设计原则

1. **单一职责**：每个组件只做一件事
2. **可复用性**：通过属性和信号暴露接口
3. **封装性**：隐藏内部实现细节
4. **可配置性**：提供合理的默认值和配置选项

```qml
// ✅ 好的组件设计
// CustomButton.qml
Rectangle {
    id: root
    
    // 公开属性
    property alias text: label.text
    property color normalColor: "#0080ff"
    property color hoverColor: "#0066cc"
    property color pressedColor: "#004d99"
    
    // 公开信号
    signal clicked()
    signal pressed()
    signal released()
    
    // 提供默认尺寸
    implicitWidth: 120
    implicitHeight: 40
    width: implicitWidth
    height: implicitHeight
    
    // 内部实现
    color: mouseArea.pressed ? pressedColor :
           mouseArea.containsMouse ? hoverColor : normalColor
    radius: 5
    
    Text {
        id: label
        anchors.centerIn: parent
        color: "white"
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
        onPressed: root.pressed()
        onReleased: root.released()
    }
}
```

### 11.2 命名约定

```qml
Item {
    // 属性：驼峰命名
    property int itemCount: 0
    property string userName: ""
    property bool isEnabled: true
    
    // 信号：驼峰命名，动词开头
    signal dataLoaded()
    signal itemClicked(int index)
    signal valueChanged(real newValue)
    
    // 函数：驼峰命名，动词开头
    function loadData() { }
    function updateValue(value) { }
    function calculateTotal() { }
    
    // ID：驼峰命名，描述性
    Rectangle {
        id: mainContainer
    }
    
    Button {
        id: submitButton
    }
}
```

### 11.3 文件组织

```
MyApp/
├── Main.qml                    # 主入口
├── qml/
│   ├── components/             # 可复用组件
│   │   ├── CustomButton.qml
│   │   ├── CustomCard.qml
│   │   └── CustomDialog.qml
│   ├── pages/                  # 页面
│   │   ├── HomePage.qml
│   │   ├── SettingsPage.qml
│   │   └── AboutPage.qml
│   ├── styles/                 # 样式和主题
│   │   ├── Theme.qml
│   │   └── Colors.qml
│   └── utils/                  # 工具和辅助
│       ├── Utils.js
│       └── Constants.qml
└── resources/                  # 资源文件
    ├── images/
    ├── fonts/
    └── icons/
```

---

## 结语

QML 是一个强大而灵活的 UI 框架，但也有许多细节需要注意。理解这些核心概念和常见陷阱，可以帮助你：

1. ✅ 避免常见错误和性能问题
2. ✅ 编写更清晰、更易维护的代码
3. ✅ 充分利用 QML 的声明式特性
4. ✅ 创建高性能、响应式的用户界面

**关键要点：**

- **尺寸系统**：理解 `implicitWidth/Height` 只是建议，需要显式设置 `width/height` 才能生效
- **布局系统**：优先使用 Layouts 而不是 Positioners，除非是简单场景
- **锚点系统**：避免锚点与显式尺寸冲突，注意循环依赖
- **属性绑定**：理解绑定的自动更新和破坏机制
- **性能优化**：使用 Loader 延迟加载，优化 ListView，避免复杂绑定
- **常见陷阱**：注意 null 检查、数组响应性、事件传播等问题

记住：**声明式编程的核心是描述"是什么"，而不是"怎么做"**。充分利用 QML 的绑定和信号机制，让代码更简洁、更易维护。

---

*文档版本：1.0*  
*最后更新：2025-11-04*  
*适用于：Qt 6.x / Qt Quick*  
*作者：Kiro AI Assistant*
