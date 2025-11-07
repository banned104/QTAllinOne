# QQuickItem 详解与实战

> 本文档详细介绍 Qt6 中 QQuickItem 的核心概念、属性、方法、信号以及实战应用，帮助开发者深入理解 QML 可视化元素的基础。

## 目录

1. [QQuickItem 概述](#1-quickitem-概述)
2. [几何与定位](#2-几何与定位)
3. [锚点布局](#3-锚点布局)
4. [变换与效果](#4-变换与效果)
5. [可见性与不透明度](#5-可见性与不透明度)
6. [输入处理](#6-输入处理)
7. [焦点管理](#7-焦点管理)
8. [层次结构](#8-层次结构)
9. [渲染与绘制](#9-渲染与绘制)
10. [自定义 Item](#10-自定义-item)
11. [性能优化](#11-性能优化)
12. [实战案例](#12-实战案例)

---

## 1. QQuickItem 概述

### 1.1 什么是 QQuickItem？

QQuickItem 是所有 QML 可视化元素的基类，提供了位置、大小、变换、输入处理等基础功能。

**继承关系**:
```
QObject
    ↓
QQuickItem
    ↓
Rectangle, Text, Image, MouseArea, etc.
```

### 1.2 基本使用

```qml
import QtQuick

Item {
    // ========== 基础属性 ==========
    id: myItem
    
    // 几何属性
    x: 100
    y: 100
    width: 200
    height: 150
    
    // 可见性
    visible: true
    opacity: 1.0
    
    // 层级
    z: 0
    
    // 子项
    Rectangle {
        anchors.fill: parent
        color: "#4080ff"
    }
}
```

### 1.3 Item 的核心特性

- ✅ **几何管理**: 位置、大小、边界
- ✅ **布局系统**: 锚点、定位器
- ✅ **变换**: 旋转、缩放、平移
- ✅ **输入处理**: 鼠标、键盘、触摸
- ✅ **焦点管理**: 焦点链、焦点作用域
- ✅ **渲染控制**: 裁剪、层、缓存
- ✅ **层次结构**: 父子关系、兄弟顺序

---

## 2. 几何与定位

### 2.1 位置属性

```qml
import QtQuick

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    // ========== 绝对定位 ==========
    Rectangle {
        id: absoluteItem
        x: 100              // X 坐标（相对于父项）
        y: 100              // Y 坐标（相对于父项）
        width: 100
        height: 100
        color: "#ff6b6b"
        
        Text {
            anchors.centerIn: parent
            text: `(${parent.x}, ${parent.y})`
            color: "white"
        }
    }
    
    // ========== 相对定位 ==========
    Rectangle {
        x: absoluteItem.x + absoluteItem.width + 20
        y: absoluteItem.y
        width: 100
        height: 100
        color: "#4ecdc4"
    }
    
    // ========== 使用 Qt.point ==========
    Rectangle {
        property point position: Qt.point(300, 100)
        x: position.x
        y: position.y
        width: 100
        height: 100
        color: "#ffe66d"
    }
}
```

### 2.2 尺寸属性

```qml
Item {
    // ========== 显式尺寸 ==========
    width: 200
    height: 150
    
    // ========== 隐式尺寸（只读）==========
    // implicitWidth: 自动计算的宽度
    // implicitHeight: 自动计算的高度
    
    // ========== 尺寸约束 ==========
    // 注意：Item 本身不支持 minimum/maximum
    // 但可以通过绑定实现
    
    property real minWidth: 100
    property real maxWidth: 400
    
    width: Math.max(minWidth, Math.min(maxWidth, desiredWidth))
    
    // ========== 监听尺寸变化 ==========
    onWidthChanged: {
        console.log("宽度改变:", width)
    }
    
    onHeightChanged: {
        console.log("高度改变:", height)
    }
}
```

### 2.3 边界矩形

```qml
Item {
    id: container
    width: 400
    height: 300
    
    Rectangle {
        id: child
        x: 50
        y: 50
        width: 100
        height: 100
        color: "#4080ff"
        rotation: 45
    }
    
    // ========== 边界矩形属性 ==========
    Component.onCompleted: {
        // 子项在父坐标系中的边界
        console.log("childrenRect:", childrenRect)
        // childrenRect.x, childrenRect.y
        // childrenRect.width, childrenRect.height
        
        // 项的边界矩形（包含变换）
        let bounds = child.mapRectToItem(container, 
                                        Qt.rect(0, 0, child.width, child.height))
        console.log("变换后边界:", bounds)
    }
}
```

### 2.4 坐标映射

```qml
Item {
    id: root
    width: 800
    height: 600
    
    Item {
        id: parent1
        x: 100
        y: 100
        width: 200
        height: 200
        
        Rectangle {
            id: child
            x: 50
            y: 50
            width: 100
            height: 100
            color: "#4080ff"
            
            MouseArea {
                anchors.fill: parent
                onClicked: (mouse) => {
                    // ========== 坐标转换 ==========
                    
                    // 1. 映射到父项坐标系
                    let parentPos = mapToItem(parent1, mouse.x, mouse.y)
                    console.log("父项坐标:", parentPos)
                    
                    // 2. 映射到根项坐标系
                    let rootPos = mapToItem(root, mouse.x, mouse.y)
                    console.log("根项坐标:", rootPos)
                    
                    // 3. 映射到全局坐标系
                    let globalPos = mapToGlobal(mouse.x, mouse.y)
                    console.log("全局坐标:", globalPos)
                    
                    // 4. 从其他项映射
                    let fromParent = mapFromItem(parent1, 0, 0)
                    console.log("从父项映射:", fromParent)
                }
            }
        }
    }
}
```

---

## 3. 锚点布局

### 3.1 基础锚点

```qml
Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    // ========== 锚点到父项 ==========
    Rectangle {
        id: topLeft
        width: 100
        height: 100
        color: "#ff6b6b"
        
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
    }
    
    Rectangle {
        id: topRight
        width: 100
        height: 100
        color: "#4ecdc4"
        
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 20
    }
    
    Rectangle {
        id: bottomLeft
        width: 100
        height: 100
        color: "#ffe66d"
        
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 20
    }
    
    Rectangle {
        id: bottomRight
        width: 100
        height: 100
        color: "#a8e6cf"
        
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 20
    }
    
    // ========== 居中 ==========
    Rectangle {
        width: 150
        height: 150
        color: "#ff8b94"
        
        anchors.centerIn: parent
    }
}
```

### 3.2 锚点到兄弟项

```qml
Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Rectangle {
        id: reference
        x: 100
        y: 100
        width: 150
        height: 150
        color: "#4080ff"
        
        Text {
            anchors.centerIn: parent
            text: "参考项"
            color: "white"
        }
    }
    
    // ========== 锚点到兄弟项 ==========
    
    // 在右侧
    Rectangle {
        width: 100
        height: 100
        color: "#ff6b6b"
        
        anchors.left: reference.right
        anchors.top: reference.top
        anchors.leftMargin: 20
        
        Text {
            anchors.centerIn: parent
            text: "右侧"
            color: "white"
        }
    }
    
    // 在下方
    Rectangle {
        width: 100
        height: 100
        color: "#4ecdc4"
        
        anchors.top: reference.bottom
        anchors.left: reference.left
        anchors.topMargin: 20
        
        Text {
            anchors.centerIn: parent
            text: "下方"
            color: "white"
        }
    }
    
    // 垂直居中对齐
    Rectangle {
        width: 100
        height: 100
        color: "#ffe66d"
        
        anchors.left: reference.right
        anchors.verticalCenter: reference.verticalCenter
        anchors.leftMargin: 20
        
        Text {
            anchors.centerIn: parent
            text: "垂直居中"
            color: "white"
        }
    }
}
```

### 3.3 锚点填充

```qml
Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Rectangle {
        id: container
        anchors.centerIn: parent
        width: 400
        height: 300
        color: "#2c3e50"
        
        // ========== 填充父项 ==========
        Rectangle {
            anchors.fill: parent
            anchors.margins: 20
            color: "#4080ff"
            opacity: 0.5
        }
        
        // ========== 填充到特定项 ==========
        Rectangle {
            anchors.fill: container
            anchors.margins: 40
            color: "#ff6b6b"
            opacity: 0.5
        }
    }
}
```

### 3.4 锚点边距

```qml
Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Rectangle {
        anchors.fill: parent
        
        // ========== 统一边距 ==========
        anchors.margins: 50
        
        // ========== 单独边距 ==========
        // anchors.leftMargin: 50
        // anchors.rightMargin: 50
        // anchors.topMargin: 50
        // anchors.bottomMargin: 50
        
        color: "#4080ff"
    }
}
```

---

## 4. 变换与效果

### 4.1 基础变换

```qml
import QtQuick

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Row {
        anchors.centerIn: parent
        spacing: 50
        
        // ========== 旋转 ==========
        Rectangle {
            width: 100
            height: 100
            color: "#ff6b6b"
            rotation: 45  // 度数
            
            Text {
                anchors.centerIn: parent
                text: "旋转 45°"
                rotation: -45  // 反向旋转文本
            }
        }
        
        // ========== 缩放 ==========
        Rectangle {
            width: 100
            height: 100
            color: "#4ecdc4"
            scale: 1.5
            
            Text {
                anchors.centerIn: parent
                text: "缩放 1.5x"
                scale: 1 / 1.5  // 反向缩放文本
            }
        }
        
        // ========== 不透明度 ==========
        Rectangle {
            width: 100
            height: 100
            color: "#ffe66d"
            opacity: 0.5
            
            Text {
                anchors.centerIn: parent
                text: "50% 透明"
            }
        }
    }
}
```

### 4.2 变换原点

```qml
Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Row {
        anchors.centerIn: parent
        spacing: 50
        
        // ========== 默认原点（中心）==========
        Rectangle {
            width: 100
            height: 100
            color: "#ff6b6b"
            rotation: 45
            
            // 默认 transformOrigin: Item.Center
        }
        
        // ========== 左上角原点 ==========
        Rectangle {
            width: 100
            height: 100
            color: "#4ecdc4"
            rotation: 45
            transformOrigin: Item.TopLeft
        }
        
        // ========== 右下角原点 ==========
        Rectangle {
            width: 100
            height: 100
            color: "#ffe66d"
            rotation: 45
            transformOrigin: Item.BottomRight
        }
    }
    
    // transformOrigin 可选值：
    // Item.TopLeft, Item.Top, Item.TopRight
    // Item.Left, Item.Center, Item.Right
    // Item.BottomLeft, Item.Bottom, Item.BottomRight
}
```

### 4.3 高级变换

```qml
import QtQuick

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Rectangle {
        anchors.centerIn: parent
        width: 200
        height: 200
        color: "#4080ff"
        
        // ========== 变换列表 ==========
        transform: [
            // 旋转变换
            Rotation {
                angle: 45
                origin.x: 100
                origin.y: 100
                axis { x: 0; y: 1; z: 0 }  // 绕 Y 轴旋转
            },
            
            // 缩放变换
            Scale {
                xScale: 1.2
                yScale: 0.8
                origin.x: 100
                origin.y: 100
            },
            
            // 平移变换
            Translate {
                x: 50
                y: 20
            }
        ]
        
        Text {
            anchors.centerIn: parent
            text: "复合变换"
            color: "white"
            font.pixelSize: 16
        }
    }
}
```

### 4.4 3D 变换

```qml
import QtQuick

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Row {
        anchors.centerIn: parent
        spacing: 100
        
        // ========== 绕 X 轴旋转 ==========
        Rectangle {
            width: 150
            height: 150
            color: "#ff6b6b"
            
            transform: Rotation {
                angle: 60
                axis { x: 1; y: 0; z: 0 }
                origin { x: 75; y: 75 }
            }
            
            Text {
                anchors.centerIn: parent
                text: "X 轴旋转"
                color: "white"
            }
        }
        
        // ========== 绕 Y 轴旋转 ==========
        Rectangle {
            width: 150
            height: 150
            color: "#4ecdc4"
            
            transform: Rotation {
                angle: 60
                axis { x: 0; y: 1; z: 0 }
                origin { x: 75; y: 75 }
            }
            
            Text {
                anchors.centerIn: parent
                text: "Y 轴旋转"
                color: "white"
            }
        }
        
        // ========== 透视效果 ==========
        Rectangle {
            width: 150
            height: 150
            color: "#ffe66d"
            
            transform: [
                Rotation {
                    angle: 45
                    axis { x: 0; y: 1; z: 0 }
                    origin { x: 75; y: 75 }
                },
                Scale {
                    xScale: 0.8
                    origin { x: 75; y: 75 }
                }
            ]
            
            Text {
                anchors.centerIn: parent
                text: "透视"
                color: "white"
            }
        }
    }
}
```

---

## 5. 可见性与不透明度

### 5.1 可见性控制

```qml
import QtQuick
import QtQuick.Controls

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        // ========== visible 属性 ==========
        Rectangle {
            id: visibleItem
            width: 200
            height: 100
            color: "#4080ff"
            visible: visibleCheck.checked  // 控制可见性
            
            Text {
                anchors.centerIn: parent
                text: "可见/不可见"
                color: "white"
            }
        }
        
        CheckBox {
            id: visibleCheck
            text: "显示矩形"
            checked: true
        }
        
        Text {
            text: `visible: ${visibleItem.visible}`
            font.pixelSize: 14
        }
        
        Text {
            text: "注意：visible=false 时，项不占用布局空间"
            font.pixelSize: 12
            color: "#666666"
        }
    }
}
```

### 5.2 不透明度

```qml
import QtQuick
import QtQuick.Controls

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        // ========== opacity 属性 ==========
        Rectangle {
            width: 200
            height: 100
            color: "#4080ff"
            opacity: opacitySlider.value
            
            Text {
                anchors.centerIn: parent
                text: `不透明度: ${(parent.opacity * 100).toFixed(0)}%`
                color: "white"
            }
        }
        
        Slider {
            id: opacitySlider
            width: 200
            from: 0
            to: 1
            value: 1
        }
        
        Text {
            text: "注意：opacity 影响项及其所有子项"
            font.pixelSize: 12
            color: "#666666"
        }
    }
}
```

### 5.3 启用状态

```qml
import QtQuick
import QtQuick.Controls

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        // ========== enabled 属性 ==========
        Rectangle {
            width: 200
            height: 100
            color: enabledCheck.checked ? "#4080ff" : "#cccccc"
            enabled: enabledCheck.checked
            
            Text {
                anchors.centerIn: parent
                text: parent.enabled ? "已启用" : "已禁用"
                color: "white"
            }
            
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log("点击")
                }
            }
        }
        
        CheckBox {
            id: enabledCheck
            text: "启用"
            checked: true
        }
        
        Text {
            text: "注意：enabled=false 时，项不响应输入事件"
            font.pixelSize: 12
            color: "#666666"
        }
    }
}
```

---

## 6. 输入处理

### 6.1 鼠标输入

```qml
import QtQuick

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Rectangle {
        id: mouseItem
        anchors.centerIn: parent
        width: 200
        height: 200
        color: mouseArea.pressed ? "#ff6b6b" : "#4080ff"
        
        // 平滑颜色过渡
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
        
        Text {
            anchors.centerIn: parent
            text: "点击我"
            color: "white"
            font.pixelSize: 20
        }
        
        // ========== MouseArea ==========
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            
            // 启用悬停
            hoverEnabled: true
            
            // 接受的按钮
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            
            // 鼠标按下
            onPressed: (mouse) => {
                console.log("按下:", mouse.x, mouse.y, mouse.button)
            }
            
            // 鼠标释放
            onReleased: (mouse) => {
                console.log("释放:", mouse.x, mouse.y)
            }
            
            // 鼠标点击
            onClicked: (mouse) => {
                console.log("点击:", mouse.x, mouse.y)
            }
            
            // 鼠标双击
            onDoubleClicked: (mouse) => {
                console.log("双击:", mouse.x, mouse.y)
            }
            
            // 鼠标移动
            onPositionChanged: (mouse) => {
                if (pressed) {
                    console.log("拖拽:", mouse.x, mouse.y)
                }
            }
            
            // 鼠标进入
            onEntered: {
                console.log("鼠标进入")
                mouseItem.scale = 1.1
            }
            
            // 鼠标离开
            onExited: {
                console.log("鼠标离开")
                mouseItem.scale = 1.0
            }
            
            // 滚轮
            onWheel: (wheel) => {
                console.log("滚轮:", wheel.angleDelta.y)
            }
        }
        
        // 平滑缩放
        Behavior on scale {
            NumberAnimation { duration: 200 }
        }
    }
}
```

### 6.2 拖放

```qml
import QtQuick

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    // ========== 可拖拽项 ==========
    Rectangle {
        id: draggableItem
        x: 100
        y: 100
        width: 100
        height: 100
        color: "#4080ff"
        
        Text {
            anchors.centerIn: parent
            text: "拖我"
            color: "white"
        }
        
        MouseArea {
            id: dragArea
            anchors.fill: parent
            
            // 拖拽属性
            drag.target: draggableItem
            drag.axis: Drag.XAndYAxis
            drag.minimumX: 0
            drag.maximumX: parent.parent.width - draggableItem.width
            drag.minimumY: 0
            drag.maximumY: parent.parent.height - draggableItem.height
            
            onPressed: {
                draggableItem.z = 1  // 提升层级
            }
            
            onReleased: {
                draggableItem.z = 0
            }
        }
    }
    
    // ========== 放置目标 ==========
    Rectangle {
        x: 400
        y: 200
        width: 200
        height: 200
        color: dropArea.containsDrag ? "#4ecdc4" : "#e0e0e0"
        border.color: "#999999"
        border.width: 2
        
        Text {
            anchors.centerIn: parent
            text: "放置区域"
            font.pixelSize: 16
        }
        
        DropArea {
            id: dropArea
            anchors.fill: parent
            
            onEntered: (drag) => {
                console.log("进入放置区域")
            }
            
            onExited: {
                console.log("离开放置区域")
            }
            
            onDropped: (drop) => {
                console.log("放置")
            }
        }
    }
}
```

### 6.3 触摸输入

```qml
import QtQuick

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Rectangle {
        anchors.centerIn: parent
        width: 400
        height: 400
        color: "#4080ff"
        
        Text {
            anchors.centerIn: parent
            text: "多点触摸区域"
            color: "white"
            font.pixelSize: 20
        }
        
        // ========== 多点触摸 ==========
        MultiPointTouchArea {
            anchors.fill: parent
            
            // 最小/最大触摸点数
            minimumTouchPoints: 1
            maximumTouchPoints: 10
            
            // 触摸点更新
            onTouchUpdated: (touchPoints) => {
                console.log("触摸点数:", touchPoints.length)
                
                for (let i = 0; i < touchPoints.length; i++) {
                    let point = touchPoints[i]
                    console.log(`点 ${i}:`, point.x, point.y)
                }
            }
            
            // 手势识别
            onGestureStarted: (gesture) => {
                console.log("手势开始")
            }
            
            onUpdated: (touchPoints) => {
                // 双指缩放检测
                if (touchPoints.length === 2) {
                    let p1 = touchPoints[0]
                    let p2 = touchPoints[1]
                    
                    let dx = p2.x - p1.x
                    let dy = p2.y - p1.y
                    let distance = Math.sqrt(dx * dx + dy * dy)
                    
                    console.log("双指距离:", distance)
                }
            }
        }
    }
}
```

---

## 7. 焦点管理

### 7.1 焦点基础

```qml
import QtQuick
import QtQuick.Controls

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        // ========== 焦点项 ==========
        Rectangle {
            id: focusItem1
            width: 300
            height: 60
            color: focus ? "#4080ff" : "#cccccc"
            border.color: focus ? "#2c3e50" : "#999999"
            border.width: 2
            
            // 启用焦点
            focus: true
            
            Text {
                anchors.centerIn: parent
                text: "焦点项 1 " + (parent.focus ? "(有焦点)" : "(无焦点)")
                color: parent.focus ? "white" : "#666666"
            }
            
            // 键盘事件
            Keys.onPressed: (event) => {
                if (event.key === Qt.Key_Down) {
                    focusItem2.forceActiveFocus()
                    event.accepted = true
                }
            }
            
            MouseArea {
                anchors.fill: parent
                onClicked: parent.forceActiveFocus()
            }
        }
        
        Rectangle {
            id: focusItem2
            width: 300
            height: 60
            color: focus ? "#4ecdc4" : "#cccccc"
            border.color: focus ? "#2c3e50" : "#999999"
            border.width: 2
            
            Text {
                anchors.centerIn: parent
                text: "焦点项 2 " + (parent.focus ? "(有焦点)" : "(无焦点)")
                color: parent.focus ? "white" : "#666666"
            }
            
            Keys.onPressed: (event) => {
                if (event.key === Qt.Key_Up) {
                    focusItem1.forceActiveFocus()
                    event.accepted = true
                }
            }
            
            MouseArea {
                anchors.fill: parent
                onClicked: parent.forceActiveFocus()
            }
        }
        
        Text {
            text: "点击矩形获取焦点，使用上下键切换"
            font.pixelSize: 12
            color: "#666666"
        }
    }
}
```

### 7.2 焦点作用域

```qml
import QtQuick

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Row {
        anchors.centerIn: parent
        spacing: 50
        
        // ========== 焦点作用域 1 ==========
        FocusScope {
            id: scope1
            width: 200
            height: 300
            
            Rectangle {
                anchors.fill: parent
                color: scope1.activeFocus ? "#4080ff" : "#cccccc"
                border.color: "#2c3e50"
                border.width: 2
                
                Column {
                    anchors.centerIn: parent
                    spacing: 10
                    
                    Text {
                        text: "作用域 1"
                        font.pixelSize: 16
                        font.bold: true
                    }
                    
                    Rectangle {
                        width: 150
                        height: 40
                        color: input1.focus ? "#ff6b6b" : "#e0e0e0"
                        
                        TextInput {
                            id: input1
                            anchors.centerIn: parent
                            text: "输入框 1"
                            focus: true  // 作用域内的默认焦点
                        }
                    }
                    
                    Rectangle {
                        width: 150
                        height: 40
                        color: input2.focus ? "#ff6b6b" : "#e0e0e0"
                        
                        TextInput {
                            id: input2
                            anchors.centerIn: parent
                            text: "输入框 2"
                        }
                    }
                }
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: scope1.forceActiveFocus()
                }
            }
        }
        
        // ========== 焦点作用域 2 ==========
        FocusScope {
            id: scope2
            width: 200
            height: 300
            
            Rectangle {
                anchors.fill: parent
                color: scope2.activeFocus ? "#4ecdc4" : "#cccccc"
                border.color: "#2c3e50"
                border.width: 2
                
                Column {
                    anchors.centerIn: parent
                    spacing: 10
                    
                    Text {
                        text: "作用域 2"
                        font.pixelSize: 16
                        font.bold: true
                    }
                    
                    Rectangle {
                        width: 150
                        height: 40
                        color: input3.focus ? "#ffe66d" : "#e0e0e0"
                        
                        TextInput {
                            id: input3
                            anchors.centerIn: parent
                            text: "输入框 3"
                            focus: true
                        }
                    }
                }
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: scope2.forceActiveFocus()
                }
            }
        }
    }
}
```

---

## 8. 层次结构

### 8.1 父子关系

```qml
import QtQuick

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Item {
        id: parentItem
        x: 200
        y: 200
        width: 400
        height: 300
        
        // 父项边界（调试用）
        Rectangle {
            anchors.fill: parent
            color: "transparent"
            border.color: "#999999"
            border.width: 2
        }
        
        // ========== 子项 ==========
        Rectangle {
            id: child1
            x: 50
            y: 50
            width: 100
            height: 100
            color: "#4080ff"
            
            Text {
                anchors.centerIn: parent
                text: "子项 1"
                color: "white"
            }
        }
        
        Rectangle {
            id: child2
            x: 200
            y: 50
            width: 100
            height: 100
            color: "#4ecdc4"
            
            Text {
                anchors.centerIn: parent
                text: "子项 2"
                color: "white"
            }
        }
        
        Component.onCompleted: {
            // ========== 访问父子关系 ==========
            console.log("父项:", child1.parent === parentItem)  // true
            console.log("子项数量:", parentItem.children.length)
            console.log("子项列表:", parentItem.children)
            
            // 遍历子项
            for (let i = 0; i < parentItem.children.length; i++) {
                console.log("子项", i, ":", parentItem.children[i])
            }
        }
    }
}
```

### 8.2 Z 顺序

```qml
import QtQuick
import QtQuick.Controls

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Column {
        anchors.centerIn: parent
        spacing: -50  // 负间距使矩形重叠
        
        // ========== Z 顺序控制 ==========
        Rectangle {
            id: rect1
            width: 200
            height: 100
            color: "#ff6b6b"
            z: zSlider1.value
            
            Text {
                anchors.centerIn: parent
                text: `Z: ${parent.z}`
                color: "white"
            }
        }
        
        Rectangle {
            id: rect2
            width: 200
            height: 100
            color: "#4ecdc4"
            z: zSlider2.value
            
            Text {
                anchors.centerIn: parent
                text: `Z: ${parent.z}`
                color: "white"
            }
        }
        
        Rectangle {
            id: rect3
            width: 200
            height: 100
            color: "#ffe66d"
            z: zSlider3.value
            
            Text {
                anchors.centerIn: parent
                text: `Z: ${parent.z}`
                color: "white"
            }
        }
    }
    
    // Z 值控制
    Column {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 50
        spacing: 20
        
        Column {
            Text { text: "矩形 1 Z 值" }
            Slider {
                id: zSlider1
                from: 0
                to: 10
                value: 0
            }
        }
        
        Column {
            Text { text: "矩形 2 Z 值" }
            Slider {
                id: zSlider2
                from: 0
                to: 10
                value: 0
            }
        }
        
        Column {
            Text { text: "矩形 3 Z 值" }
            Slider {
                id: zSlider3
                from: 0
                to: 10
                value: 0
            }
        }
    }
}
```

### 8.3 动态创建和销毁

```qml
import QtQuick
import QtQuick.Controls

Rectangle {
    width: 800
    height: 600
    color: "#f0f0f0"
    
    Item {
        id: container
        anchors.fill: parent
        anchors.margins: 50
    }
    
    Column {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        spacing: 10
        
        Button {
            text: "创建矩形"
            onClicked: {
                // ========== 动态创建 ==========
                let component = Qt.createComponent("DynamicRect.qml")
                if (component.status === Component.Ready) {
                    let rect = component.createObject(container, {
                        "x": Math.random() * (container.width - 100),
                        "y": Math.random() * (container.height - 100),
                        "color": Qt.hsla(Math.random(), 1, 0.5, 1)
                    })
                    
                    if (rect === null) {
                        console.log("创建失败")
                    }
                } else if (component.status === Component.Error) {
                    console.log("错误:", component.errorString())
                }
            }
        }
        
        Button {
            text: "清除所有"
            onClicked: {
                // ========== 销毁子项 ==========
                for (let i = container.children.length - 1; i >= 0; i--) {
                    container.children[i].destroy()
                }
            }
        }
        
        Text {
            text: `当前矩形数量: ${container.children.length}`
        }
    }
}

// DynamicRect.qml
Rectangle {
    width: 100
    height: 100
    
    Text {
        anchors.centerIn: parent
        text: "动态"
        color: "white"
    }
    
    MouseArea {
        anchors.fill: parent
        onClicked: parent.destroy()  // 点击销毁
    }
}
```

---

## 总结

### 核心知识点

1. **几何与定位**
   - 位置、尺寸属性
   - 坐标映射
   - 边界矩形

2. **锚点布局**
   - 基础锚点
   - 锚点到兄弟项
   - 边距控制

3. **变换**
   - 旋转、缩放
   - 变换原点
   - 3D 变换

4. **输入处理**
   - 鼠标事件
   - 拖放
   - 触摸输入

5. **焦点管理**
   - 焦点链
   - 焦点作用域
   - 键盘导航

6. **层次结构**
   - 父子关系
   - Z 顺序
   - 动态创建

### 最佳实践

✅ **推荐做法**
- 使用锚点实现响应式布局
- 合理使用 Z 顺序
- 正确处理焦点
- 使用 Behavior 实现平滑动画
- 及时销毁不需要的项

❌ **避免做法**
- 不要过度嵌套
- 避免循环锚点依赖
- 不要忽略性能影响
- 避免内存泄漏

### 参考资源

- **官方文档**: https://doc.qt.io/qt-6/qquickitem.html
- **QML Item**: https://doc.qt.io/qt-6/qml-qtquick-item.html

---

**祝你在 QQuickItem 开发中取得成功！** 🚀

