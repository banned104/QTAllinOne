---
title: Repeater3D
tags:
- QtQuick3D
- Scene
- 中文
---

# Repeater3D

## 是什么

Repeater3D 是 Qt Quick 3D 中用于创建多个相同或相似 3D 对象实例的组件。它类似于 Qt Quick 2D 中的 Repeater，但专门针对 3D 场景进行了优化。Repeater3D 可以根据数据模型或数值范围自动生成多个 3D 对象，常用于创建网格、阵列、粒子系统等需要大量重复对象的场景。

```
3D Scene Graph
├── View3D
│   └── Node
│       └── Repeater3D ← 批量创建 3D 对象
│           ├── Model (instance 0)
│           ├── Model (instance 1)
│           ├── Model (instance 2)
│           └── ...
```

Repeater3D 在渲染管线中位于场景图构建阶段，它会在场景图遍历时动态创建子对象，这些对象随后参与正常的渲染流程。

## 常用属性一览表

| 属性名 | 类型 | 默认值 | 取值范围 | 作用 | 性能/质量提示 |
| ------ | ---- | ------ | -------- | ---- | -------------- |
| count★ | int | 0 | 0-∞ | 指定要创建的对象数量 | 大量对象时考虑使用 Instanced Rendering |
| model★ | var | undefined | Array/ListModel/数值 | 数据源，决定创建对象的数量和数据 | 使用 ListModel 比 JavaScript Array 性能更好 |
| delegate★ | Component | null | QML Component | 定义每个重复对象的模板 | 保持 delegate 简单以提高性能 |

<details>
<summary>点击查看完整属性列表</summary>

| 属性名 | 类型 | 默认值 | 取值范围 | 作用 | 性能/质量提示 |
| ------ | ---- | ------ | -------- | ---- | -------------- |
| count | int | 0 | 0-∞ | 指定要创建的对象数量 | 避免过大的数值 |
| model | var | undefined | Array/ListModel/数值 | 数据源 | 优先使用 ListModel |
| delegate | Component | null | QML Component | 对象模板 | 避免复杂的嵌套结构 |

</details>

## 属性详解

### count★ - 对象数量控制

count 属性直接控制 Repeater3D 创建的对象数量。当只需要创建固定数量的相同对象时使用：

```qml
Repeater3D {
    count: 100
    delegate: Model {
        source: "#Cube"
        x: index * 2
        materials: DefaultMaterial { diffuseColor: "red" }
    }
}
```

性能考虑：
- count 超过 1000 时建议使用 Instanced Rendering
- 动态修改 count 会触发对象重建，避免频繁修改

### model★ - 数据驱动的对象创建

model 属性支持多种数据源类型，提供更灵活的对象创建方式：

**数值模型**：
```qml
Repeater3D {
    model: 50  // 等同于 count: 50
    delegate: Model { /* ... */ }
}
```

**数组模型**：
```qml
Repeater3D {
    model: [
        {color: "red", scale: 1.0},
        {color: "green", scale: 1.5},
        {color: "blue", scale: 2.0}
    ]
    delegate: Model {
        source: "#Sphere"
        materials: DefaultMaterial { 
            diffuseColor: modelData.color 
        }
        scale: Qt.vector3d(modelData.scale, modelData.scale, modelData.scale)
    }
}
```

**ListModel**：
```qml
ListModel {
    id: objectModel
    ListElement { posX: 0; posY: 0; color: "red" }
    ListElement { posX: 2; posY: 0; color: "green" }
    ListElement { posX: 4; posY: 0; color: "blue" }
}

Repeater3D {
    model: objectModel
    delegate: Model {
        source: "#Cube"
        x: model.posX
        y: model.posY
        materials: DefaultMaterial { diffuseColor: model.color }
    }
}
```

### delegate★ - 对象模板定义

delegate 定义了每个重复对象的结构和行为。在 delegate 中可以使用以下特殊属性：

- `index`: 当前对象的索引（0 开始）
- `model`: 当前对象对应的模型数据
- `modelData`: 当使用简单数组时的数据项

```qml
Repeater3D {
    model: 10
    delegate: Model {
        source: "#Cube"
        x: (index % 5) * 3        // 5x2 网格排列
        z: Math.floor(index / 5) * 3
        y: Math.sin(index * 0.5)  // 波浪形高度
        
        materials: DefaultMaterial {
            diffuseColor: Qt.hsla(index / 10, 1.0, 0.5, 1.0)
        }
        
        // 旋转动画，每个对象有不同的延迟
        RotationAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 2000
            loops: Animation.Infinite
            running: true
            // 基于索引的延迟，创造波浪效果
            Component.onCompleted: {
                startTime = index * 100
            }
        }
    }
}
```

## 最小可运行示例

创建一个 5x5 的立方体网格，每个立方体有不同的颜色和高度：

**文件树：**
```
repeater3d-demo/
├── main.qml
├── CMakeLists.txt
└── main.cpp
```

**完整代码：**

main.qml:
```qml
import QtQuick
import QtQuick3D

Window {
    width: 800
    height: 600
    visible: true
    title: "Repeater3D Demo"

    View3D {
        anchors.fill: parent
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 10, 15)
            eulerRotation.x: -30
        }
        
        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
        }
        
        // 创建 5x5 立方体网格
        Repeater3D {
            model: 25
            delegate: Model {
                source: "#Cube"
                
                // 网格位置计算
                x: (index % 5 - 2) * 3
                z: (Math.floor(index / 5) - 2) * 3
                y: Math.random() * 2  // 随机高度
                
                // 基于位置的颜色
                materials: DefaultMaterial {
                    diffuseColor: Qt.hsla(
                        (index % 5) / 5,  // 色相基于 X 位置
                        0.8,
                        0.5 + (Math.floor(index / 5) / 5) * 0.3,  // 亮度基于 Z 位置
                        1.0
                    )
                }
                
                // 旋转动画
                RotationAnimation on eulerRotation.y {
                    from: 0
                    to: 360
                    duration: 3000 + (index * 50)  // 不同速度
                    loops: Animation.Infinite
                    running: true
                }
            }
        }
    }
}
```

main.cpp:
```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    return app.exec();
}
```

CMakeLists.txt:
```cmake
cmake_minimum_required(VERSION 3.16)
project(repeater3d-demo)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Quick3D)

qt_add_executable(repeater3d-demo main.cpp)
qt_add_qml_module(repeater3d-demo
    URI RepeaterDemo
    VERSION 1.0
    QML_FILES main.qml
)

target_link_libraries(repeater3d-demo Qt6::Core Qt6::Quick Qt6::Quick3D)
```

**预期效果：**
显示一个 5x5 的彩色立方体网格，每个立方体有不同的颜色和高度，并以不同速度旋转。

## 踩坑与调试

### 常见问题

**1. 性能问题 - 对象数量过多**
```
警告: Repeater3D created 5000 objects, consider using instanced rendering
```

解决方案：
- 当对象数量超过 1000 时，考虑使用 InstancedRendering
- 使用 LOD (Level of Detail) 技术
- 实现视锥体剔除

**2. 内存泄漏 - 动态修改 model**
```qml
// 错误做法
Timer {
    interval: 100
    repeat: true
    onTriggered: {
        repeater.model = Math.random() * 100  // 频繁重建对象
    }
}

// 正确做法
Timer {
    interval: 1000  // 降低频率
    repeat: true
    onTriggered: {
        if (Math.abs(newCount - repeater.count) > 10) {
            repeater.model = newCount  // 只在显著变化时更新
        }
    }
}
```

**3. delegate 中的绑定问题**
```qml
// 错误：index 在 Component.onCompleted 中可能未定义
delegate: Model {
    Component.onCompleted: {
        console.log("Index:", index)  // 可能输出 undefined
    }
}

// 正确：使用属性绑定
delegate: Model {
    property int myIndex: index
    Component.onCompleted: {
        console.log("Index:", myIndex)
    }
}
```

### 调试技巧

**1. 启用对象计数调试**
```cpp
// 在 main.cpp 中添加
QLoggingCategory::setFilterRules("qt.quick3d.repeater.debug=true");
```

**2. 监控对象创建**
```qml
Repeater3D {
    model: 100
    onItemAdded: function(index, item) {
        console.log("Created item", index, "at position", item.position)
    }
    onItemRemoved: function(index, item) {
        console.log("Removed item", index)
    }
}
```

**3. 性能分析**
```qml
// 使用 Timer 测量创建时间
Timer {
    id: perfTimer
    property real startTime
    
    function measureCreation() {
        startTime = Date.now()
        repeater.model = 1000
    }
    
    interval: 16
    onTriggered: {
        if (repeater.count === 1000) {
            console.log("Creation time:", Date.now() - startTime, "ms")
            stop()
        }
    }
}
```

## 延伸阅读

- [Qt Quick 3D Instanced Rendering](https://doc.qt.io/qt-6/qml-qtquick3d-instancedrendering.html) - 大量对象的高性能渲染方案
- [QML Performance Best Practices](https://doc.qt.io/qt-6/qtquick-performance.html) - QML 性能优化指南，包含 Repeater 优化技巧

## 附录：完整属性清单

<details>
<summary>Repeater3D 完整属性列表</summary>

### 继承自 Node3D 的属性

| 属性名 | 类型 | 默认值 | 说明 |
| ------ | ---- | ------ | ---- |
| position | vector3d | (0,0,0) | 3D 位置 |
| eulerRotation | vector3d | (0,0,0) | 欧拉角旋转 |
| scale | vector3d | (1,1,1) | 缩放比例 |
| visible | bool | true | 可见性 |

### Repeater3D 特有属性

| 属性名 | 类型 | 默认值 | 说明 |
| ------ | ---- | ------ | ---- |
| count | int | 0 | 重复对象数量 |
| model | var | undefined | 数据模型 |
| delegate | Component | null | 对象模板 |

### 信号

| 信号名 | 参数 | 说明 |
| ------ | ---- | ---- |
| itemAdded | int index, Item item | 对象创建时触发 |
| itemRemoved | int index, Item item | 对象移除时触发 |

### 方法

| 方法名 | 参数 | 返回值 | 说明 |
| ------ | ---- | ------ | ---- |
| itemAt | int index | Item | 获取指定索引的对象 |

</details>