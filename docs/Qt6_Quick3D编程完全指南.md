# Qt6 Quick 3D 编程完全指南

> 本指南详细介绍 Qt6 Quick 3D 的核心概念、组件使用、材质系统、光照、动画、性能优化等，帮助有 QML 基础的开发者精通 Quick 3D 开发。

## 目录

1. [Quick 3D 简介](#1-quick-3d-简介)
2. [环境搭建与第一个 3D 场景](#2-环境搭建与第一个-3d-场景)
3. [View3D 与场景结构](#3-view3d-与场景结构)
4. [相机系统](#4-相机系统)
5. [3D 模型与几何体](#5-3d-模型与几何体)
6. [材质系统](#6-材质系统)
7. [光照系统](#7-光照系统)
8. [纹理与贴图](#8-纹理与贴图)
9. [变换与动画](#9-变换与动画)
10. [交互与拾取](#10-交互与拾取)
11. [粒子系统](#11-粒子系统)
12. [自定义材质与效果](#12-自定义材质与效果)
13. [性能优化](#13-性能优化)
14. [实战项目](#14-实战项目)

---

## 1. Quick 3D 简介

### 1.1 什么是 Qt Quick 3D？

Qt Quick 3D 是 Qt 6 中的 3D 图形模块，提供了高级 API 用于在 QML 中创建 3D 内容。

**核心特性：**
- 🎨 声明式 3D 场景描述
- 🚀 基于现代图形 API（OpenGL、Vulkan、Metal、D3D）
- 🎭 物理基础渲染（PBR）
- 💡 实时光照和阴影
- 🎬 动画和粒子系统
- 🔧 与 2D QML 无缝集成

**与传统 OpenGL 的对比：**

| 特性 | Qt Quick 3D | 原生 OpenGL |
|------|-------------|-------------|
| **学习曲线** | 低（声明式） | 高（命令式） |
| **开发效率** | 高 | 低 |
| **代码量** | 少 | 多 |
| **灵活性** | 中 | 高 |
| **性能** | 优秀 | 最优 |
| **适用场景** | 应用 UI、可视化 | 游戏引擎、专业图形 |

### 1.2 Quick 3D 架构

```
QML 应用
    ↓
View3D (3D 视口)
    ↓
SceneEnvironment (环境配置)
    ↓
Node (场景节点树)
    ├── Camera (相机)
    ├── Light (光源)
    ├── Model (3D 模型)
    └── Node (容器节点)
```

---

## 2. 环境搭建与第一个 3D 场景

### 2.1 项目配置

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(Quick3DExample VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Quick3D)

qt_add_executable(Quick3DExample
    main.cpp
)

qt_add_qml_module(Quick3DExample
    URI Quick3DExample
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(Quick3DExample PRIVATE
    Qt6::Quick
    Qt6::Quick3D
)
```

```cpp
// main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    
    // 加载 QML
    const QUrl url(u"qrc:/Quick3DExample/Main.qml"_qs);
    engine.load(url);
    
    if (engine.rootObjects().isEmpty())
        return -1;
    
    return app.exec();
}
```

### 2.2 第一个 3D 场景

```qml
// Main.qml
import QtQuick
import QtQuick3D

Window {
    width: 1280
    height: 720
    visible: true
    title: "我的第一个 Quick 3D 场景"
    
    // 3D 视口 - 所有 3D 内容的容器
    View3D {
        anchors.fill: parent
        
        // 环境配置
        environment: SceneEnvironment {
            clearColor: "#222222"  // 背景颜色
            backgroundMode: SceneEnvironment.Color
        }
        
        // 相机 - 观察场景的视角
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 300)  // 相机位置
            // 默认看向 (0, 0, 0)
        }
        
        // 方向光 - 照亮场景
        DirectionalLight {
            eulerRotation.x: -30  // 光源角度
            brightness: 1.0       // 亮度
        }
        
        // 3D 模型 - 一个立方体
        Model {
            source: "#Cube"  // 内置几何体
            
            // 材质
            materials: PrincipledMaterial {
                baseColor: "#4080ff"  // 基础颜色
            }
            
            // 旋转动画
            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 5000
                loops: Animation.Infinite
            }
        }
    }
}
```

**代码解析：**
- `View3D`: 3D 视口，类似 2D 中的 Item
- `SceneEnvironment`: 配置场景环境（背景、抗锯齿等）
- `PerspectiveCamera`: 透视相机，模拟人眼视角
- `DirectionalLight`: 方向光，类似太阳光
- `Model`: 3D 模型，使用内置立方体
- `PrincipledMaterial`: PBR 材质

---

## 3. View3D 与场景结构

### 3.1 View3D 详解

```qml
import QtQuick
import QtQuick3D

Window {
    width: 1280
    height: 720
    visible: true
    
    View3D {
        id: view3D
        anchors.fill: parent
        
        // ========== 环境配置 ==========
        environment: SceneEnvironment {
            // 背景模式
            backgroundMode: SceneEnvironment.Color  // Color | SkyBox | Transparent
            clearColor: "#1a1a1a"
            
            // 抗锯齿
            antialiasingMode: SceneEnvironment.MSAA  // NoAA | SSAA | MSAA | ProgressiveAA
            antialiasingQuality: SceneEnvironment.High  // Medium | High | VeryHigh
            
            // 环境光遮蔽（AO）
            aoEnabled: true
            aoStrength: 50
            aoDistance: 5
            
            // 色调映射
            tonemapMode: SceneEnvironment.TonemapModeLinear
            
            // 雾效
            // fogEnabled: true
            // fogColor: "#808080"
            // fogDensity: 0.01
        }
        
        // ========== 相机 ==========
        camera: mainCamera
        
        PerspectiveCamera {
            id: mainCamera
            position: Qt.vector3d(0, 100, 300)
            eulerRotation.x: -15
        }
        
        // ========== 光源 ==========
        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            brightness: 1.0
            castsShadow: true  // 投射阴影
        }
        
        // ========== 场景内容 ==========
        Model {
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColor: "#ff6b6b"
                metalness: 0.8
                roughness: 0.2
            }
        }
    }
    
    // ========== 2D UI 叠加层 ==========
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
        width: 200
        height: 100
        color: "#80000000"
        radius: 10
        
        Column {
            anchors.centerIn: parent
            spacing: 5
            
            Text {
                text: "3D 场景信息"
                color: "white"
                font.bold: true
            }
            
            Text {
                text: "相机位置: " + mainCamera.position
                color: "white"
                font.pixelSize: 10
            }
        }
    }
}
```

### 3.2 场景节点树

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#222222"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 500)
        eulerRotation.x: -20
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 场景根节点 ==========
    Node {
        id: sceneRoot
        
        // 太阳系示例
        Node {
            id: solarSystem
            
            // 太阳
            Model {
                id: sun
                source: "#Sphere"
                scale: Qt.vector3d(3, 3, 3)
                
                materials: PrincipledMaterial {
                    baseColor: "#ffff00"
                    lighting: PrincipledMaterial.NoLighting  // 自发光
                }
                
                // 自转
                NumberAnimation on eulerRotation.y {
                    from: 0
                    to: 360
                    duration: 10000
                    loops: Animation.Infinite
                }
            }
            
            // 地球轨道
            Node {
                id: earthOrbit
                
                // 公转动画
                NumberAnimation on eulerRotation.y {
                    from: 0
                    to: 360
                    duration: 20000
                    loops: Animation.Infinite
                }
                
                // 地球
                Model {
                    id: earth
                    source: "#Sphere"
                    position: Qt.vector3d(150, 0, 0)
                    scale: Qt.vector3d(1, 1, 1)
                    
                    materials: PrincipledMaterial {
                        baseColor: "#4080ff"
                    }
                    
                    // 自转
                    NumberAnimation on eulerRotation.y {
                        from: 0
                        to: 360
                        duration: 5000
                        loops: Animation.Infinite
                    }
                    
                    // 月球轨道
                    Node {
                        id: moonOrbit
                        
                        NumberAnimation on eulerRotation.y {
                            from: 0
                            to: 360
                            duration: 5000
                            loops: Animation.Infinite
                        }
                        
                        // 月球
                        Model {
                            source: "#Sphere"
                            position: Qt.vector3d(30, 0, 0)
                            scale: Qt.vector3d(0.3, 0.3, 0.3)
                            
                            materials: PrincipledMaterial {
                                baseColor: "#cccccc"
                            }
                        }
                    }
                }
            }
        }
    }
}
```

**节点树概念：**
- `Node` 是场景图的基本单位
- 子节点继承父节点的变换（位置、旋转、缩放）
- 通过节点树可以创建复杂的层次结构



---

## 4. 相机系统

### 4.1 透视相机（PerspectiveCamera）

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    // ========== 透视相机 ==========
    PerspectiveCamera {
        id: camera
        
        // 位置
        position: Qt.vector3d(0, 100, 300)
        
        // 旋转（欧拉角，单位：度）
        eulerRotation.x: -15  // 俯仰角
        eulerRotation.y: 0    // 偏航角
        eulerRotation.z: 0    // 翻滚角
        
        // 视场角（FOV）
        fieldOfView: 60  // 默认 60 度
        
        // 裁剪平面
        clipNear: 1      // 近裁剪面
        clipFar: 10000   // 远裁剪面
        
        // 或者使用 lookAt 方法
        Component.onCompleted: {
            // 让相机看向特定点
            // lookAt(Qt.vector3d(0, 0, 0))
        }
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // 参考网格
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        scale: Qt.vector3d(10, 10, 1)
        materials: PrincipledMaterial {
            baseColor: "#404040"
        }
    }
    
    // 测试立方体
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, 50, 0)
        materials: PrincipledMaterial {
            baseColor: "#ff6b6b"
        }
    }
}
```

### 4.2 正交相机（OrthographicCamera）

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    // ========== 正交相机 ==========
    OrthographicCamera {
        id: orthoCamera
        
        position: Qt.vector3d(0, 200, 200)
        eulerRotation.x: -45
        
        // 正交投影范围
        horizontalMagnification: 2.0  // 水平缩放
        verticalMagnification: 2.0    // 垂直缩放
        
        clipNear: 1
        clipFar: 1000
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // 3D 对象（正交投影下没有透视效果）
    Repeater3D {
        model: 5
        
        Model {
            source: "#Cube"
            position: Qt.vector3d(index * 60 - 120, 0, 0)
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            
            materials: PrincipledMaterial {
                baseColor: Qt.hsla(index / 5, 1.0, 0.5, 1.0)
            }
        }
    }
}
```

### 4.3 交互式相机控制

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

Window {
    width: 1280
    height: 720
    visible: true
    title: "交互式相机控制"
    
    View3D {
        id: view3D
        anchors.fill: parent
        
        environment: SceneEnvironment {
            clearColor: "#1a1a1a"
            backgroundMode: SceneEnvironment.Color
        }
        
        // ========== 可控制的相机 ==========
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 100, 300)
            
            // 使用四元数或欧拉角
            property real yaw: 0
            property real pitch: -15
            
            eulerRotation.x: pitch
            eulerRotation.y: yaw
        }
        
        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }
        
        // 场景内容
        Model {
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: "#4080ff"
            }
        }
        
        // 地面
        Model {
            source: "#Rectangle"
            eulerRotation.x: -90
            position: Qt.vector3d(0, -50, 0)
            scale: Qt.vector3d(10, 10, 1)
            materials: PrincipledMaterial {
                baseColor: "#404040"
            }
        }
        
        // ========== 鼠标控制 ==========
        MouseArea {
            anchors.fill: parent
            
            property real lastX: 0
            property real lastY: 0
            property bool isDragging: false
            
            onPressed: (mouse) => {
                lastX = mouse.x
                lastY = mouse.y
                isDragging = true
            }
            
            onReleased: {
                isDragging = false
            }
            
            onPositionChanged: (mouse) => {
                if (isDragging) {
                    // 计算鼠标移动量
                    let deltaX = mouse.x - lastX
                    let deltaY = mouse.y - lastY
                    
                    // 更新相机角度
                    camera.yaw += deltaX * 0.2
                    camera.pitch -= deltaY * 0.2
                    
                    // 限制俯仰角
                    camera.pitch = Math.max(-89, Math.min(89, camera.pitch))
                    
                    lastX = mouse.x
                    lastY = mouse.y
                }
            }
            
            // 滚轮缩放
            onWheel: (wheel) => {
                let delta = wheel.angleDelta.y / 120
                let zoomSpeed = 20
                
                // 计算相机前向向量
                let rad = camera.yaw * Math.PI / 180
                let forward = Qt.vector3d(
                    Math.sin(rad),
                    0,
                    -Math.cos(rad)
                )
                
                // 移动相机
                camera.position = Qt.vector3d(
                    camera.position.x + forward.x * delta * zoomSpeed,
                    camera.position.y,
                    camera.position.z + forward.z * delta * zoomSpeed
                )
            }
        }
    }
    
    // ========== 控制面板 ==========
    Column {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        spacing: 10
        
        Rectangle {
            width: 250
            height: column.height + 20
            color: "#80000000"
            radius: 10
            
            Column {
                id: column
                anchors.centerIn: parent
                width: parent.width - 20
                spacing: 10
                
                Text {
                    text: "相机控制"
                    color: "white"
                    font.bold: true
                    font.pixelSize: 16
                }
                
                Text {
                    text: "位置: " + 
                          camera.position.x.toFixed(1) + ", " +
                          camera.position.y.toFixed(1) + ", " +
                          camera.position.z.toFixed(1)
                    color: "white"
                    font.pixelSize: 12
                }
                
                Text {
                    text: "偏航角: " + camera.yaw.toFixed(1) + "°"
                    color: "white"
                    font.pixelSize: 12
                }
                
                Text {
                    text: "俯仰角: " + camera.pitch.toFixed(1) + "°"
                    color: "white"
                    font.pixelSize: 12
                }
                
                Button {
                    text: "重置相机"
                    onClicked: {
                        camera.position = Qt.vector3d(0, 100, 300)
                        camera.yaw = 0
                        camera.pitch = -15
                    }
                }
            }
        }
    }
    
    // 操作提示
    Text {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 20
        text: "鼠标拖拽：旋转视角\n滚轮：缩放"
        color: "white"
        font.pixelSize: 14
    }
}
```

---

## 5. 3D 模型与几何体

### 5.1 内置几何体

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 600)
        eulerRotation.x: -15
    }
    
    DirectionalLight {
        eulerRotation.x: -30
        eulerRotation.y: -70
        brightness: 1.0
    }
    
    // ========== 内置几何体展示 ==========
    
    // 立方体
    Model {
        source: "#Cube"
        position: Qt.vector3d(-200, 0, 0)
        materials: PrincipledMaterial {
            baseColor: "#ff6b6b"
        }
    }
    
    // 球体
    Model {
        source: "#Sphere"
        position: Qt.vector3d(-100, 0, 0)
        materials: PrincipledMaterial {
            baseColor: "#4ecdc4"
        }
    }
    
    // 圆柱体
    Model {
        source: "#Cylinder"
        position: Qt.vector3d(0, 0, 0)
        materials: PrincipledMaterial {
            baseColor: "#ffe66d"
        }
    }
    
    // 圆锥体
    Model {
        source: "#Cone"
        position: Qt.vector3d(100, 0, 0)
        materials: PrincipledMaterial {
            baseColor: "#a8e6cf"
        }
    }
    
    // 矩形平面
    Model {
        source: "#Rectangle"
        position: Qt.vector3d(200, 0, 0)
        materials: PrincipledMaterial {
            baseColor: "#ff8b94"
        }
    }
    
    // 地面
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        position: Qt.vector3d(0, -50, 0)
        scale: Qt.vector3d(20, 20, 1)
        materials: PrincipledMaterial {
            baseColor: "#2a2a2a"
        }
    }
}
```

### 5.2 加载外部模型

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 100, 300)
        eulerRotation.x: -15
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 加载 3D 模型文件 ==========
    
    // 支持的格式：.mesh (Qt 专有格式)
    Model {
        id: customModel
        source: "models/mymodel.mesh"  // 从资源加载
        
        position: Qt.vector3d(0, 0, 0)
        scale: Qt.vector3d(1, 1, 1)
        eulerRotation.y: 45
        
        materials: PrincipledMaterial {
            baseColor: "#4080ff"
            metalness: 0.5
            roughness: 0.3
        }
        
        // 旋转动画
        NumberAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 10000
            loops: Animation.Infinite
        }
    }
    
    // 注意：Qt Quick 3D 主要使用 .mesh 格式
    // 其他格式（如 .obj, .fbx, .gltf）需要先转换为 .mesh
    // 使用 Balsam 工具转换：
    // balsam mymodel.obj
}
```

### 5.3 程序化几何体

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 100, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 自定义几何体 ==========
    Model {
        id: customGeometry
        
        geometry: CustomGeometry {
            id: geom
            
            // 顶点数据
            vertices: {
                // 三角形顶点（x, y, z）
                return new Float32Array([
                    // 前面
                    -50, -50,  50,
                     50, -50,  50,
                     50,  50,  50,
                    -50,  50,  50,
                    
                    // 后面
                    -50, -50, -50,
                    -50,  50, -50,
                     50,  50, -50,
                     50, -50, -50
                ])
            }
            
            // 索引数据
            indices: {
                return new Uint16Array([
                    // 前面
                    0, 1, 2,  2, 3, 0,
                    // 后面
                    4, 5, 6,  6, 7, 4,
                    // 左面
                    4, 0, 3,  3, 5, 4,
                    // 右面
                    1, 7, 6,  6, 2, 1,
                    // 上面
                    3, 2, 6,  6, 5, 3,
                    // 下面
                    4, 7, 1,  1, 0, 4
                ])
            }
            
            // 法线数据
            normals: {
                return new Float32Array([
                    // 前面法线
                    0, 0, 1,
                    0, 0, 1,
                    0, 0, 1,
                    0, 0, 1,
                    
                    // 后面法线
                    0, 0, -1,
                    0, 0, -1,
                    0, 0, -1,
                    0, 0, -1
                ])
            }
        }
        
        materials: PrincipledMaterial {
            baseColor: "#4080ff"
        }
        
        NumberAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 5000
            loops: Animation.Infinite
        }
    }
}
```

### 5.4 模型实例化（Instancing）

```qml
import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 500)
        eulerRotation.x: -20
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 实例化渲染（高性能） ==========
    Model {
        source: "#Cube"
        
        // 使用实例化表
        instancing: InstanceList {
            id: instances
            
            // 创建 100 个实例
            Component.onCompleted: {
                for (let i = 0; i < 10; i++) {
                    for (let j = 0; j < 10; j++) {
                        instances.append({
                            position: Qt.vector3d(i * 50 - 225, 0, j * 50 - 225),
                            scale: Qt.vector3d(0.8, 0.8, 0.8),
                            rotation: Qt.quaternion(1, 0, 0, 0),
                            color: Qt.hsla(i / 10, 1.0, 0.5, 1.0)
                        })
                    }
                }
            }
        }
        
        materials: PrincipledMaterial {
            baseColor: "#4080ff"
        }
    }
    
    // 性能对比：
    // - 普通方式：100 个 Model = 100 次绘制调用
    // - 实例化：100 个实例 = 1 次绘制调用
}
```



---

## 6. 材质系统

### 6.1 PrincipledMaterial（PBR 材质）

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 100, 400)
        eulerRotation.x: -10
    }
    
    DirectionalLight {
        eulerRotation.x: -30
        eulerRotation.y: -70
        brightness: 1.5
    }
    
    // ========== PBR 材质参数展示 ==========
    
    Row {
        spacing: 80
        x: -200
        
        // 金属材质
        Model {
            source: "#Sphere"
            
            materials: PrincipledMaterial {
                // 基础颜色
                baseColor: "#ffd700"
                
                // 金属度（0.0 = 非金属，1.0 = 金属）
                metalness: 1.0
                
                // 粗糙度（0.0 = 光滑，1.0 = 粗糙）
                roughness: 0.2
                
                // 光照模式
                lighting: PrincipledMaterial.FragmentLighting
            }
            
            Text {
                text: "金属"
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                y: 100
            }
        }
        
        // 塑料材质
        Model {
            source: "#Sphere"
            
            materials: PrincipledMaterial {
                baseColor: "#ff6b6b"
                metalness: 0.0  // 非金属
                roughness: 0.5
            }
            
            Text {
                text: "塑料"
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                y: 100
            }
        }
        
        // 粗糙金属
        Model {
            source: "#Sphere"
            
            materials: PrincipledMaterial {
                baseColor: "#808080"
                metalness: 1.0
                roughness: 0.8  // 高粗糙度
            }
            
            Text {
                text: "粗糙金属"
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                y: 100
            }
        }
        
        // 玻璃材质
        Model {
            source: "#Sphere"
            
            materials: PrincipledMaterial {
                baseColor: "#ffffff"
                metalness: 0.0
                roughness: 0.0
                
                // 透明度
                opacity: 0.3
                
                // 混合模式
                alphaMode: PrincipledMaterial.Blend
            }
            
            Text {
                text: "玻璃"
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                y: 100
            }
        }
        
        // 自发光材质
        Model {
            source: "#Sphere"
            
            materials: PrincipledMaterial {
                baseColor: "#00ff00"
                
                // 自发光（不受光照影响）
                lighting: PrincipledMaterial.NoLighting
            }
            
            Text {
                text: "自发光"
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                y: 100
            }
        }
    }
}
```

### 6.2 材质属性详解

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

Window {
    width: 1280
    height: 720
    visible: true
    title: "材质属性交互演示"
    
    View3D {
        id: view3D
        anchors.fill: parent
        anchors.rightMargin: 300
        
        environment: SceneEnvironment {
            clearColor: "#1a1a1a"
            backgroundMode: SceneEnvironment.Color
        }
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 300)
        }
        
        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            brightness: 1.5
        }
        
        // 测试球体
        Model {
            id: testSphere
            source: "#Sphere"
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            
            materials: PrincipledMaterial {
                id: material
                
                // 基础属性
                baseColor: colorPicker.color
                metalness: metalnessSlider.value
                roughness: roughnessSlider.value
                
                // 高级属性
                specularAmount: specularSlider.value
                specularTint: specularTintSlider.value
                
                // 法线强度
                normalStrength: normalStrengthSlider.value
                
                // 遮挡强度
                occlusionAmount: occlusionSlider.value
                
                // 透明度
                opacity: opacitySlider.value
                alphaMode: opacitySlider.value < 1.0 ? 
                          PrincipledMaterial.Blend : 
                          PrincipledMaterial.Opaque
            }
            
            // 旋转动画
            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 10000
                loops: Animation.Infinite
                running: autoRotateCheck.checked
            }
        }
    }
    
    // ========== 控制面板 ==========
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 300
        color: "#2a2a2a"
        
        Flickable {
            anchors.fill: parent
            anchors.margins: 10
            contentHeight: controlColumn.height
            
            Column {
                id: controlColumn
                width: parent.width
                spacing: 15
                
                Text {
                    text: "材质控制面板"
                    color: "white"
                    font.bold: true
                    font.pixelSize: 18
                }
                
                // 颜色选择
                GroupBox {
                    title: "基础颜色"
                    width: parent.width
                    
                    Column {
                        width: parent.width
                        spacing: 5
                        
                        Rectangle {
                            id: colorPicker
                            width: parent.width
                            height: 50
                            color: "#4080ff"
                            border.color: "white"
                            border.width: 2
                            radius: 5
                            
                            property color color: "#4080ff"
                            
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    // 循环切换预设颜色
                                    let colors = [
                                        "#4080ff", "#ff6b6b", "#ffd700",
                                        "#4ecdc4", "#95e1d3", "#ff8b94"
                                    ]
                                    let idx = colors.indexOf(parent.color.toString())
                                    parent.color = colors[(idx + 1) % colors.length]
                                }
                            }
                        }
                    }
                }
                
                // 金属度
                GroupBox {
                    title: "金属度: " + metalnessSlider.value.toFixed(2)
                    width: parent.width
                    
                    Slider {
                        id: metalnessSlider
                        width: parent.width
                        from: 0
                        to: 1
                        value: 0.5
                    }
                }
                
                // 粗糙度
                GroupBox {
                    title: "粗糙度: " + roughnessSlider.value.toFixed(2)
                    width: parent.width
                    
                    Slider {
                        id: roughnessSlider
                        width: parent.width
                        from: 0
                        to: 1
                        value: 0.5
                    }
                }
                
                // 镜面反射
                GroupBox {
                    title: "镜面反射: " + specularSlider.value.toFixed(2)
                    width: parent.width
                    
                    Slider {
                        id: specularSlider
                        width: parent.width
                        from: 0
                        to: 1
                        value: 0.5
                    }
                }
                
                // 镜面色调
                GroupBox {
                    title: "镜面色调: " + specularTintSlider.value.toFixed(2)
                    width: parent.width
                    
                    Slider {
                        id: specularTintSlider
                        width: parent.width
                        from: 0
                        to: 1
                        value: 0.0
                    }
                }
                
                // 法线强度
                GroupBox {
                    title: "法线强度: " + normalStrengthSlider.value.toFixed(2)
                    width: parent.width
                    
                    Slider {
                        id: normalStrengthSlider
                        width: parent.width
                        from: 0
                        to: 2
                        value: 1.0
                    }
                }
                
                // 遮挡
                GroupBox {
                    title: "遮挡: " + occlusionSlider.value.toFixed(2)
                    width: parent.width
                    
                    Slider {
                        id: occlusionSlider
                        width: parent.width
                        from: 0
                        to: 1
                        value: 1.0
                    }
                }
                
                // 透明度
                GroupBox {
                    title: "透明度: " + opacitySlider.value.toFixed(2)
                    width: parent.width
                    
                    Slider {
                        id: opacitySlider
                        width: parent.width
                        from: 0
                        to: 1
                        value: 1.0
                    }
                }
                
                // 自动旋转
                CheckBox {
                    id: autoRotateCheck
                    text: "自动旋转"
                    checked: true
                }
                
                // 预设按钮
                GroupBox {
                    title: "材质预设"
                    width: parent.width
                    
                    Column {
                        width: parent.width
                        spacing: 5
                        
                        Button {
                            text: "金属"
                            width: parent.width
                            onClicked: {
                                colorPicker.color = "#ffd700"
                                metalnessSlider.value = 1.0
                                roughnessSlider.value = 0.2
                                opacitySlider.value = 1.0
                            }
                        }
                        
                        Button {
                            text: "塑料"
                            width: parent.width
                            onClicked: {
                                colorPicker.color = "#ff6b6b"
                                metalnessSlider.value = 0.0
                                roughnessSlider.value = 0.5
                                opacitySlider.value = 1.0
                            }
                        }
                        
                        Button {
                            text: "玻璃"
                            width: parent.width
                            onClicked: {
                                colorPicker.color = "#ffffff"
                                metalnessSlider.value = 0.0
                                roughnessSlider.value = 0.0
                                opacitySlider.value = 0.3
                            }
                        }
                        
                        Button {
                            text: "橡胶"
                            width: parent.width
                            onClicked: {
                                colorPicker.color = "#2a2a2a"
                                metalnessSlider.value = 0.0
                                roughnessSlider.value = 0.9
                                opacitySlider.value = 1.0
                            }
                        }
                    }
                }
            }
        }
    }
}
```

### 6.3 DefaultMaterial（简单材质）

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 0, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== DefaultMaterial（性能更好，功能较少） ==========
    Model {
        source: "#Sphere"
        
        materials: DefaultMaterial {
            // 漫反射颜色
            diffuseColor: "#4080ff"
            
            // 镜面反射颜色
            specularTint: "#ffffff"
            
            // 光泽度
            specularAmount: 0.5
            specularRoughness: 0.3
            
            // 光照模式
            lighting: DefaultMaterial.FragmentLighting
        }
    }
}
```

---

## 7. 光照系统

### 7.1 方向光（DirectionalLight）

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 100, 300)
        eulerRotation.x: -15
    }
    
    // ========== 方向光（模拟太阳光） ==========
    DirectionalLight {
        id: sunLight
        
        // 光源方向（通过旋转控制）
        eulerRotation.x: -30  // 俯仰角
        eulerRotation.y: -70  // 偏航角
        
        // 亮度
        brightness: 1.0
        
        // 颜色
        color: "#ffffff"
        
        // 环境光颜色
        ambientColor: "#202020"
        
        // 投射阴影
        castsShadow: true
        
        // 阴影质量
        shadowMapQuality: Light.ShadowMapQualityHigh
        
        // 阴影偏移（防止阴影痤疮）
        shadowBias: 0.01
        
        // 阴影范围
        shadowMapFar: 1000
        
        // 动画演示
        SequentialAnimation on eulerRotation.y {
            running: true
            loops: Animation.Infinite
            NumberAnimation { from: -180; to: 180; duration: 10000 }
        }
    }
    
    // 测试场景
    Model {
        source: "#Sphere"
        position: Qt.vector3d(0, 50, 0)
        materials: PrincipledMaterial {
            baseColor: "#4080ff"
        }
        castsShadows: true
    }
    
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        scale: Qt.vector3d(10, 10, 1)
        materials: PrincipledMaterial {
            baseColor: "#404040"
        }
        receivesShadows: true
    }
}
```

### 7.2 点光源（PointLight）

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#0a0a0a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 500)
        eulerRotation.x: -20
    }
    
    // ========== 点光源（向四周发光） ==========
    PointLight {
        id: pointLight
        
        // 位置
        position: Qt.vector3d(0, 100, 0)
        
        // 颜色
        color: "#ff8800"
        
        // 亮度
        brightness: 5.0
        
        // 衰减参数
        constantFade: 1.0      // 常数衰减
        linearFade: 0.0        // 线性衰减
        quadraticFade: 0.001   // 二次衰减
        
        // 投射阴影
        castsShadow: true
        shadowMapQuality: Light.ShadowMapQualityMedium
        
        // 光源移动动画
        SequentialAnimation on position.x {
            running: true
            loops: Animation.Infinite
            NumberAnimation { from: -200; to: 200; duration: 3000 }
            NumberAnimation { from: 200; to: -200; duration: 3000 }
        }
    }
    
    // 可视化光源位置
    Model {
        source: "#Sphere"
        position: pointLight.position
        scale: Qt.vector3d(0.2, 0.2, 0.2)
        materials: PrincipledMaterial {
            baseColor: pointLight.color
            lighting: PrincipledMaterial.NoLighting
        }
    }
    
    // 测试对象
    Repeater3D {
        model: 5
        Model {
            source: "#Cube"
            position: Qt.vector3d(index * 100 - 200, 0, 0)
            materials: PrincipledMaterial {
                baseColor: "#808080"
            }
            castsShadows: true
        }
    }
    
    // 地面
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        position: Qt.vector3d(0, -50, 0)
        scale: Qt.vector3d(20, 20, 1)
        materials: PrincipledMaterial {
            baseColor: "#2a2a2a"
        }
        receivesShadows: true
    }
}
```

### 7.3 聚光灯（SpotLight）

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#0a0a0a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 500)
        eulerRotation.x: -20
    }
    
    // ========== 聚光灯（锥形光束） ==========
    SpotLight {
        id: spotLight
        
        // 位置
        position: Qt.vector3d(0, 200, 0)
        
        // 方向（向下照射）
        eulerRotation.x: -90
        
        // 颜色
        color: "#00ff00"
        
        // 亮度
        brightness: 10.0
        
        // 锥角（内锥角）
        coneAngle: 30
        
        // 外锥角（柔和边缘）
        innerConeAngle: 20
        
        // 衰减
        constantFade: 1.0
        linearFade: 0.0
        quadraticFade: 0.0005
        
        // 投射阴影
        castsShadow: true
        shadowMapQuality: Light.ShadowMapQualityHigh
        
        // 旋转动画
        SequentialAnimation on eulerRotation.y {
            running: true
            loops: Animation.Infinite
            NumberAnimation { from: 0; to: 360; duration: 5000 }
        }
    }
    
    // 可视化聚光灯
    Model {
        source: "#Cone"
        position: spotLight.position
        eulerRotation: spotLight.eulerRotation
        scale: Qt.vector3d(0.3, 0.5, 0.3)
        materials: PrincipledMaterial {
            baseColor: spotLight.color
            lighting: PrincipledMaterial.NoLighting
            opacity: 0.3
            alphaMode: PrincipledMaterial.Blend
        }
    }
    
    // 测试场景
    Repeater3D {
        model: 25
        Model {
            source: "#Cylinder"
            position: Qt.vector3d(
                (index % 5) * 100 - 200,
                0,
                Math.floor(index / 5) * 100 - 200
            )
            scale: Qt.vector3d(0.5, 1, 0.5)
            materials: PrincipledMaterial {
                baseColor: "#808080"
            }
            castsShadows: true
        }
    }
    
    // 地面
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        position: Qt.vector3d(0, -50, 0)
        scale: Qt.vector3d(20, 20, 1)
        materials: PrincipledMaterial {
            baseColor: "#2a2a2a"
        }
        receivesShadows: true
    }
}
```

### 7.4 多光源场景

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#0a0a0a"
        backgroundMode: SceneEnvironment.Color
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 300, 600)
        eulerRotation.x: -25
    }
    
    // ========== 主光源（太阳） ==========
    DirectionalLight {
        eulerRotation.x: -45
        eulerRotation.y: -30
        color: "#ffffee"
        brightness: 0.5
        castsShadow: true
        shadowMapQuality: Light.ShadowMapQualityHigh
    }
    
    // ========== 三个彩色点光源 ==========
    PointLight {
        position: Qt.vector3d(-150, 100, 0)
        color: "#ff0000"
        brightness: 3.0
        quadraticFade: 0.002
        
        SequentialAnimation on position.y {
            running: true
            loops: Animation.Infinite
            NumberAnimation { from: 50; to: 150; duration: 2000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 150; to: 50; duration: 2000; easing.type: Easing.InOutQuad }
        }
    }
    
    PointLight {
        position: Qt.vector3d(0, 100, 0)
        color: "#00ff00"
        brightness: 3.0
        quadraticFade: 0.002
        
        SequentialAnimation on position.y {
            running: true
            loops: Animation.Infinite
            NumberAnimation { from: 50; to: 150; duration: 2000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 150; to: 50; duration: 2000; easing.type: Easing.InOutQuad }
            PauseAnimation { duration: 500 }
        }
    }
    
    PointLight {
        position: Qt.vector3d(150, 100, 0)
        color: "#0000ff"
        brightness: 3.0
        quadraticFade: 0.002
        
        SequentialAnimation on position.y {
            running: true
            loops: Animation.Infinite
            NumberAnimation { from: 50; to: 150; duration: 2000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 150; to: 50; duration: 2000; easing.type: Easing.InOutQuad }
            PauseAnimation { duration: 1000 }
        }
    }
    
    // ========== 中心球体 ==========
    Model {
        source: "#Sphere"
        scale: Qt.vector3d(2, 2, 2)
        materials: PrincipledMaterial {
            baseColor: "#ffffff"
            metalness: 0.9
            roughness: 0.1
        }
        
        NumberAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 10000
            loops: Animation.Infinite
        }
    }
    
    // ========== 地面 ==========
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        position: Qt.vector3d(0, -100, 0)
        scale: Qt.vector3d(20, 20, 1)
        materials: PrincipledMaterial {
            baseColor: "#1a1a1a"
            metalness: 0.5
            roughness: 0.5
        }
        receivesShadows: true
    }
}
```



---

## 8. 纹理与贴图

### 8.1 基础纹理映射

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 0, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
        eulerRotation.y: -70
    }
    
    // ========== 纹理映射 ==========
    Model {
        source: "#Cube"
        
        materials: PrincipledMaterial {
            // 基础颜色贴图（漫反射贴图）
            baseColorMap: Texture {
                source: "textures/wood_diffuse.jpg"
                
                // 纹理过滤
                magFilter: Texture.Linear    // 放大过滤
                minFilter: Texture.Linear    // 缩小过滤
                mipFilter: Texture.Linear    // Mipmap 过滤
                
                // 纹理环绕模式
                tilingModeHorizontal: Texture.Repeat  // 水平重复
                tilingModeVertical: Texture.Repeat    // 垂直重复
                
                // UV 缩放
                scaleU: 1.0
                scaleV: 1.0
                
                // UV 偏移
                positionU: 0.0
                positionV: 0.0
                
                // 旋转（度）
                rotationUV: 0.0
            }
            
            metalness: 0.0
            roughness: 0.8
        }
        
        // 旋转动画
        NumberAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 10000
            loops: Animation.Infinite
        }
    }
}
```

### 8.2 PBR 纹理集

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 100, 300)
        eulerRotation.x: -15
    }
    
    DirectionalLight {
        eulerRotation.x: -30
        eulerRotation.y: -70
        brightness: 1.5
    }
    
    // ========== 完整 PBR 材质 ==========
    Model {
        source: "#Sphere"
        scale: Qt.vector3d(2, 2, 2)
        
        materials: PrincipledMaterial {
            // 1. 基础颜色贴图（Albedo/Diffuse）
            baseColorMap: Texture {
                source: "textures/metal_basecolor.jpg"
            }
            
            // 2. 金属度贴图（Metallic）
            metalnessMap: Texture {
                source: "textures/metal_metallic.jpg"
            }
            metalnessChannel: Material.R  // 使用红色通道
            
            // 3. 粗糙度贴图（Roughness）
            roughnessMap: Texture {
                source: "textures/metal_roughness.jpg"
            }
            roughnessChannel: Material.R
            
            // 4. 法线贴图（Normal Map）
            normalMap: Texture {
                source: "textures/metal_normal.jpg"
            }
            normalStrength: 1.0  // 法线强度
            
            // 5. 环境光遮蔽贴图（AO）
            occlusionMap: Texture {
                source: "textures/metal_ao.jpg"
            }
            occlusionChannel: Material.R
            occlusionAmount: 1.0
            
            // 6. 高度贴图（Height/Displacement）
            heightMap: Texture {
                source: "textures/metal_height.jpg"
            }
            heightChannel: Material.R
            heightAmount: 0.1
            
            // 7. 自发光贴图（Emissive）
            // emissiveMap: Texture {
            //     source: "textures/emissive.jpg"
            // }
            // emissiveFactor: Qt.vector3d(1, 1, 1)
        }
        
        NumberAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 15000
            loops: Animation.Infinite
        }
    }
}
```

### 8.3 程序化纹理

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 0, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 使用 QML Item 作为纹理 ==========
    Model {
        source: "#Cube"
        
        materials: PrincipledMaterial {
            baseColorMap: Texture {
                // 使用 QML Item 作为纹理源
                sourceItem: Rectangle {
                    width: 512
                    height: 512
                    
                    // 棋盘格图案
                    Grid {
                        anchors.fill: parent
                        rows: 8
                        columns: 8
                        
                        Repeater {
                            model: 64
                            Rectangle {
                                width: 64
                                height: 64
                                color: (Math.floor(index / 8) + index % 8) % 2 === 0 ? 
                                      "#ffffff" : "#000000"
                            }
                        }
                    }
                }
            }
            
            metalness: 0.0
            roughness: 0.5
        }
        
        NumberAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 5000
            loops: Animation.Infinite
        }
    }
    
    // ========== 动态纹理 ==========
    Model {
        source: "#Sphere"
        position: Qt.vector3d(150, 0, 0)
        
        materials: PrincipledMaterial {
            baseColorMap: Texture {
                sourceItem: Rectangle {
                    width: 256
                    height: 256
                    color: "#1a1a1a"
                    
                    // 动画文本
                    Text {
                        anchors.centerIn: parent
                        text: "Qt Quick 3D"
                        color: "white"
                        font.pixelSize: 32
                        font.bold: true
                        
                        // 颜色动画
                        SequentialAnimation on color {
                            running: true
                            loops: Animation.Infinite
                            ColorAnimation { from: "#ff0000"; to: "#00ff00"; duration: 1000 }
                            ColorAnimation { from: "#00ff00"; to: "#0000ff"; duration: 1000 }
                            ColorAnimation { from: "#0000ff"; to: "#ff0000"; duration: 1000 }
                        }
                    }
                }
            }
        }
        
        NumberAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 5000
            loops: Animation.Infinite
        }
    }
}
```

### 8.4 立方体贴图（天空盒）

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    // ========== 天空盒环境 ==========
    environment: SceneEnvironment {
        backgroundMode: SceneEnvironment.SkyBox
        
        // 立方体贴图
        lightProbe: Texture {
            source: "maps/skybox.hdr"  // HDR 环境贴图
            mappingMode: Texture.Environment
        }
        
        // 或使用 6 张图片
        // skyBoxCubeMap: CubeMapTexture {
        //     source: "maps/skybox_#.jpg"  // #会被替换为 px, nx, py, ny, pz, nz
        // }
        
        probeExposure: 1.0
        probeHorizon: 0.0
        
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
    }
    
    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(0, 0, 300)
        
        // 相机可以旋转查看天空盒
        property real yaw: 0
        property real pitch: 0
        
        eulerRotation.x: pitch
        eulerRotation.y: yaw
    }
    
    DirectionalLight {
        eulerRotation.x: -30
        eulerRotation.y: -70
    }
    
    // 反射球体（展示环境反射）
    Model {
        source: "#Sphere"
        scale: Qt.vector3d(1.5, 1.5, 1.5)
        
        materials: PrincipledMaterial {
            baseColor: "#ffffff"
            metalness: 1.0      // 完全金属
            roughness: 0.0      // 完全光滑（镜面反射）
        }
    }
    
    // 鼠标控制
    MouseArea {
        anchors.fill: parent
        
        property real lastX: 0
        property real lastY: 0
        
        onPressed: (mouse) => {
            lastX = mouse.x
            lastY = mouse.y
        }
        
        onPositionChanged: (mouse) => {
            if (pressed) {
                camera.yaw += (mouse.x - lastX) * 0.2
                camera.pitch -= (mouse.y - lastY) * 0.2
                camera.pitch = Math.max(-89, Math.min(89, camera.pitch))
                lastX = mouse.x
                lastY = mouse.y
            }
        }
    }
}
```

---

## 9. 变换与动画

### 9.1 基础变换

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 600)
        eulerRotation.x: -15
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 位置变换 ==========
    Model {
        id: cube1
        source: "#Cube"
        position: Qt.vector3d(-200, 0, 0)  // x, y, z
        
        materials: PrincipledMaterial {
            baseColor: "#ff6b6b"
        }
        
        // 位置动画
        SequentialAnimation on position.y {
            running: true
            loops: Animation.Infinite
            NumberAnimation { from: 0; to: 100; duration: 1000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 100; to: 0; duration: 1000; easing.type: Easing.InOutQuad }
        }
    }
    
    // ========== 旋转变换 ==========
    Model {
        id: cube2
        source: "#Cube"
        position: Qt.vector3d(-100, 0, 0)
        
        // 欧拉角旋转（度）
        eulerRotation.x: 0
        eulerRotation.y: 45
        eulerRotation.z: 0
        
        materials: PrincipledMaterial {
            baseColor: "#4ecdc4"
        }
        
        // 旋转动画
        NumberAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 3000
            loops: Animation.Infinite
        }
    }
    
    // ========== 缩放变换 ==========
    Model {
        id: cube3
        source: "#Cube"
        position: Qt.vector3d(0, 0, 0)
        
        // 缩放（x, y, z）
        scale: Qt.vector3d(1, 1, 1)
        
        materials: PrincipledMaterial {
            baseColor: "#ffe66d"
        }
        
        // 缩放动画
        SequentialAnimation on scale {
            running: true
            loops: Animation.Infinite
            Vector3dAnimation {
                from: Qt.vector3d(1, 1, 1)
                to: Qt.vector3d(1.5, 1.5, 1.5)
                duration: 1000
                easing.type: Easing.InOutQuad
            }
            Vector3dAnimation {
                from: Qt.vector3d(1.5, 1.5, 1.5)
                to: Qt.vector3d(1, 1, 1)
                duration: 1000
                easing.type: Easing.InOutQuad
            }
        }
    }
    
    // ========== 组合变换 ==========
    Model {
        id: cube4
        source: "#Cube"
        position: Qt.vector3d(100, 0, 0)
        eulerRotation.y: 45
        scale: Qt.vector3d(0.8, 0.8, 0.8)
        
        materials: PrincipledMaterial {
            baseColor: "#a8e6cf"
        }
        
        // 复杂动画
        ParallelAnimation {
            running: true
            loops: Animation.Infinite
            
            NumberAnimation {
                target: cube4
                property: "position.y"
                from: 0
                to: 100
                duration: 2000
                easing.type: Easing.InOutQuad
            }
            
            NumberAnimation {
                target: cube4
                property: "eulerRotation.y"
                from: 0
                to: 360
                duration: 2000
            }
        }
    }
    
    // ========== 四元数旋转 ==========
    Model {
        id: cube5
        source: "#Cube"
        position: Qt.vector3d(200, 0, 0)
        
        // 使用四元数（避免万向锁）
        rotation: Qt.quaternion(1, 0, 0, 0)
        
        materials: PrincipledMaterial {
            baseColor: "#ff8b94"
        }
        
        // 四元数动画
        SequentialAnimation on rotation {
            running: true
            loops: Animation.Infinite
            QuaternionAnimation {
                from: Qt.quaternion(1, 0, 0, 0)
                to: Qt.quaternion(0.707, 0.707, 0, 0)
                duration: 1000
            }
            QuaternionAnimation {
                from: Qt.quaternion(0.707, 0.707, 0, 0)
                to: Qt.quaternion(1, 0, 0, 0)
                duration: 1000
            }
        }
    }
}
```

### 9.2 路径动画

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 300, 600)
        eulerRotation.x: -25
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 圆形路径动画 ==========
    Model {
        id: orbitingSphere
        source: "#Sphere"
        scale: Qt.vector3d(0.5, 0.5, 0.5)
        
        property real angle: 0
        property real radius: 200
        
        // 根据角度计算位置
        position: Qt.vector3d(
            Math.cos(angle * Math.PI / 180) * radius,
            50,
            Math.sin(angle * Math.PI / 180) * radius
        )
        
        materials: PrincipledMaterial {
            baseColor: "#ff6b6b"
        }
        
        // 角度动画
        NumberAnimation on angle {
            from: 0
            to: 360
            duration: 5000
            loops: Animation.Infinite
        }
    }
    
    // ========== 8字形路径 ==========
    Model {
        id: figure8Sphere
        source: "#Sphere"
        scale: Qt.vector3d(0.5, 0.5, 0.5)
        
        property real t: 0
        
        position: Qt.vector3d(
            150 * Math.sin(t * Math.PI / 180),
            50,
            150 * Math.sin(2 * t * Math.PI / 180)
        )
        
        materials: PrincipledMaterial {
            baseColor: "#4ecdc4"
        }
        
        NumberAnimation on t {
            from: 0
            to: 360
            duration: 5000
            loops: Animation.Infinite
        }
    }
    
    // ========== 螺旋路径 ==========
    Model {
        id: spiralSphere
        source: "#Sphere"
        scale: Qt.vector3d(0.5, 0.5, 0.5)
        
        property real angle: 0
        property real height: 0
        
        position: Qt.vector3d(
            100 * Math.cos(angle * Math.PI / 180),
            height,
            100 * Math.sin(angle * Math.PI / 180)
        )
        
        materials: PrincipledMaterial {
            baseColor: "#ffe66d"
        }
        
        ParallelAnimation {
            running: true
            loops: Animation.Infinite
            
            NumberAnimation {
                target: spiralSphere
                property: "angle"
                from: 0
                to: 720
                duration: 5000
            }
            
            SequentialAnimation {
                NumberAnimation {
                    target: spiralSphere
                    property: "height"
                    from: -100
                    to: 100
                    duration: 2500
                }
                NumberAnimation {
                    target: spiralSphere
                    property: "height"
                    from: 100
                    to: -100
                    duration: 2500
                }
            }
        }
    }
    
    // 中心参考点
    Model {
        source: "#Sphere"
        scale: Qt.vector3d(0.2, 0.2, 0.2)
        materials: PrincipledMaterial {
            baseColor: "#ffffff"
            lighting: PrincipledMaterial.NoLighting
        }
    }
}
```

### 9.3 骨骼动画（Skeleton Animation）

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 100, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 带骨骼动画的模型 ==========
    Model {
        id: animatedModel
        source: "models/character.mesh"  // 包含骨骼的模型
        
        // 骨骼
        skeleton: Skeleton {
            id: skeleton
            
            Joint {
                id: rootJoint
                index: 0
                skeletonRoot: skeleton
                
                Joint {
                    id: childJoint
                    index: 1
                    skeletonRoot: skeleton
                    eulerRotation.z: 45
                    
                    // 关节动画
                    SequentialAnimation on eulerRotation.z {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation { from: -45; to: 45; duration: 1000 }
                        NumberAnimation { from: 45; to: -45; duration: 1000 }
                    }
                }
            }
        }
        
        materials: PrincipledMaterial {
            baseColor: "#4080ff"
        }
    }
}
```



---

## 10. 交互与拾取

### 10.1 鼠标拾取（Picking）

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

Window {
    width: 1280
    height: 720
    visible: true
    title: "3D 对象拾取"
    
    View3D {
        id: view3D
        anchors.fill: parent
        
        environment: SceneEnvironment {
            clearColor: "#1a1a1a"
            backgroundMode: SceneEnvironment.Color
        }
        
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 200, 500)
            eulerRotation.x: -20
        }
        
        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }
        
        // ========== 可拾取的对象 ==========
        Repeater3D {
            model: 9
            
            Model {
                id: pickableModel
                source: "#Cube"
                
                property int modelIndex: index
                property bool isSelected: false
                property color originalColor: Qt.hsla(index / 9, 1.0, 0.5, 1.0)
                
                position: Qt.vector3d(
                    (index % 3) * 150 - 150,
                    0,
                    Math.floor(index / 3) * 150 - 150
                )
                
                scale: isSelected ? Qt.vector3d(1.2, 1.2, 1.2) : Qt.vector3d(1, 1, 1)
                
                materials: PrincipledMaterial {
                    baseColor: isSelected ? "#ffffff" : originalColor
                    metalness: isSelected ? 0.8 : 0.2
                    roughness: 0.3
                }
                
                // 平滑缩放动画
                Behavior on scale {
                    Vector3dAnimation {
                        duration: 200
                        easing.type: Easing.OutBack
                    }
                }
                
                // 选中时旋转
                NumberAnimation on eulerRotation.y {
                    running: isSelected
                    from: 0
                    to: 360
                    duration: 2000
                    loops: Animation.Infinite
                }
            }
        }
        
        // 地面
        Model {
            source: "#Rectangle"
            eulerRotation.x: -90
            position: Qt.vector3d(0, -50, 0)
            scale: Qt.vector3d(20, 20, 1)
            materials: PrincipledMaterial {
                baseColor: "#2a2a2a"
            }
            pickable: false  // 地面不可拾取
        }
    }
    
    // ========== 鼠标交互 ==========
    MouseArea {
        anchors.fill: parent
        
        onClicked: (mouse) => {
            // 执行射线拾取
            let pickResult = view3D.pick(mouse.x, mouse.y)
            
            if (pickResult.objectHit) {
                // 获取被点击的对象
                let hitObject = pickResult.objectHit
                
                // 切换选中状态
                hitObject.isSelected = !hitObject.isSelected
                
                // 输出拾取信息
                console.log("拾取到对象:", hitObject.modelIndex)
                console.log("拾取点:", pickResult.position)
                console.log("UV 坐标:", pickResult.uvPosition)
                console.log("距离:", pickResult.distance)
                
                // 更新信息面板
                infoText.text = `选中对象 ${hitObject.modelIndex}\n` +
                               `位置: ${pickResult.position.x.toFixed(1)}, ` +
                               `${pickResult.position.y.toFixed(1)}, ` +
                               `${pickResult.position.z.toFixed(1)}\n` +
                               `距离: ${pickResult.distance.toFixed(1)}`
            } else {
                // 取消所有选中
                for (let i = 0; i < 9; i++) {
                    let model = view3D.children[i + 2]  // 跳过相机和光源
                    if (model.isSelected !== undefined) {
                        model.isSelected = false
                    }
                }
                
                infoText.text = "未选中任何对象"
            }
        }
        
        // 悬停效果
        hoverEnabled: true
        
        onPositionChanged: (mouse) => {
            let pickResult = view3D.pick(mouse.x, mouse.y)
            
            if (pickResult.objectHit) {
                cursorShape = Qt.PointingHandCursor
            } else {
                cursorShape = Qt.ArrowCursor
            }
        }
    }
    
    // ========== 信息面板 ==========
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
        width: 300
        height: 120
        color: "#80000000"
        radius: 10
        
        Column {
            anchors.centerIn: parent
            spacing: 10
            
            Text {
                text: "3D 对象拾取"
                color: "white"
                font.bold: true
                font.pixelSize: 16
            }
            
            Text {
                id: infoText
                text: "点击立方体进行选择"
                color: "white"
                font.pixelSize: 12
            }
            
            Text {
                text: "提示：点击空白处取消选择"
                color: "#aaaaaa"
                font.pixelSize: 10
            }
        }
    }
}
```

### 10.2 拖拽交互

```qml
import QtQuick
import QtQuick3D

Window {
    width: 1280
    height: 720
    visible: true
    title: "3D 对象拖拽"
    
    View3D {
        id: view3D
        anchors.fill: parent
        
        environment: SceneEnvironment {
            clearColor: "#1a1a1a"
            backgroundMode: SceneEnvironment.Color
        }
        
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 200, 500)
            eulerRotation.x: -20
        }
        
        DirectionalLight {
            eulerRotation.x: -30
        }
        
        // 可拖拽的对象
        Model {
            id: draggableObject
            source: "#Sphere"
            position: Qt.vector3d(0, 0, 0)
            
            property bool isDragging: false
            
            materials: PrincipledMaterial {
                baseColor: isDragging ? "#ff6b6b" : "#4080ff"
            }
            
            Behavior on baseColor {
                ColorAnimation { duration: 200 }
            }
        }
        
        // 地面参考
        Model {
            source: "#Rectangle"
            eulerRotation.x: -90
            position: Qt.vector3d(0, -50, 0)
            scale: Qt.vector3d(10, 10, 1)
            materials: PrincipledMaterial {
                baseColor: "#2a2a2a"
            }
            pickable: false
        }
    }
    
    // ========== 拖拽逻辑 ==========
    MouseArea {
        anchors.fill: parent
        
        property var draggedObject: null
        property vector3d dragOffset: Qt.vector3d(0, 0, 0)
        
        onPressed: (mouse) => {
            let pickResult = view3D.pick(mouse.x, mouse.y)
            
            if (pickResult.objectHit === draggableObject) {
                draggedObject = draggableObject
                draggedObject.isDragging = true
                
                // 计算拖拽偏移
                dragOffset = Qt.vector3d(
                    pickResult.position.x - draggableObject.position.x,
                    pickResult.position.y - draggableObject.position.y,
                    pickResult.position.z - draggableObject.position.z
                )
            }
        }
        
        onPositionChanged: (mouse) => {
            if (draggedObject) {
                // 将屏幕坐标转换为 3D 坐标
                let pickResult = view3D.pick(mouse.x, mouse.y)
                
                if (pickResult.objectHit) {
                    // 更新对象位置
                    draggedObject.position = Qt.vector3d(
                        pickResult.position.x - dragOffset.x,
                        pickResult.position.y - dragOffset.y,
                        pickResult.position.z - dragOffset.z
                    )
                }
            }
        }
        
        onReleased: {
            if (draggedObject) {
                draggedObject.isDragging = false
                draggedObject = null
            }
        }
    }
    
    Text {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 20
        text: "拖拽球体移动"
        color: "white"
        font.pixelSize: 14
    }
}
```

### 10.3 多点触控

```qml
import QtQuick
import QtQuick3D

Window {
    width: 1280
    height: 720
    visible: true
    title: "多点触控"
    
    View3D {
        id: view3D
        anchors.fill: parent
        
        environment: SceneEnvironment {
            clearColor: "#1a1a1a"
            backgroundMode: SceneEnvironment.Color
        }
        
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 100, 300)
            eulerRotation.x: -15
            
            property real distance: 300
        }
        
        DirectionalLight {
            eulerRotation.x: -30
        }
        
        Model {
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: "#4080ff"
            }
            
            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 10000
                loops: Animation.Infinite
            }
        }
    }
    
    // ========== 多点触控处理 ==========
    MultiPointTouchArea {
        anchors.fill: parent
        
        property real initialDistance: 0
        property real initialCameraDistance: 0
        
        // 单指旋转
        onTouchUpdated: (touchPoints) => {
            if (touchPoints.length === 1) {
                let point = touchPoints[0]
                if (point.previousX !== 0) {
                    let deltaX = point.x - point.previousX
                    let deltaY = point.y - point.previousY
                    
                    camera.eulerRotation.y += deltaX * 0.2
                    camera.eulerRotation.x -= deltaY * 0.2
                    camera.eulerRotation.x = Math.max(-89, Math.min(89, camera.eulerRotation.x))
                }
            }
            // 双指缩放
            else if (touchPoints.length === 2) {
                let point1 = touchPoints[0]
                let point2 = touchPoints[1]
                
                let dx = point2.x - point1.x
                let dy = point2.y - point1.y
                let distance = Math.sqrt(dx * dx + dy * dy)
                
                if (initialDistance === 0) {
                    initialDistance = distance
                    initialCameraDistance = camera.distance
                } else {
                    let scale = distance / initialDistance
                    camera.distance = initialCameraDistance / scale
                    camera.distance = Math.max(100, Math.min(1000, camera.distance))
                    
                    camera.position.z = camera.distance
                }
            }
        }
        
        onReleased: {
            initialDistance = 0
        }
    }
}
```

---

## 11. 粒子系统

### 11.1 基础粒子发射器

```qml
import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#0a0a0a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 100, 500)
        eulerRotation.x: -10
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 粒子系统 ==========
    ParticleSystem3D {
        id: particleSystem
        
        // 粒子发射器
        ParticleEmitter3D {
            id: emitter
            
            // 发射器位置
            position: Qt.vector3d(0, 0, 0)
            
            // 粒子
            particle: ModelParticle3D {
                // 粒子模型
                delegate: Model {
                    source: "#Sphere"
                    scale: Qt.vector3d(0.1, 0.1, 0.1)
                    
                    materials: PrincipledMaterial {
                        baseColor: "#ff6b6b"
                        lighting: PrincipledMaterial.NoLighting
                    }
                }
                
                // 粒子数量上限
                maxAmount: 1000
                
                // 颜色变化
                colorVariation: Qt.vector4d(0.5, 0.5, 0.5, 0.0)
            }
            
            // 发射速率（粒子/秒）
            emitRate: 100
            
            // 粒子生命周期（毫秒）
            lifeSpan: 3000
            lifeSpanVariation: 500
            
            // 发射方向
            velocity: VectorDirection3D {
                direction: Qt.vector3d(0, 100, 0)
                directionVariation: Qt.vector3d(50, 20, 50)
            }
            
            // 粒子大小
            particleScale: 1.0
            particleScaleVariation: 0.5
            
            // 粒子旋转
            particleRotation: Qt.vector3d(0, 0, 0)
            particleRotationVariation: Qt.vector3d(180, 180, 180)
            particleRotationVelocity: Qt.vector3d(0, 50, 0)
            particleRotationVelocityVariation: Qt.vector3d(20, 20, 20)
        }
        
        // ========== 重力影响 ==========
        Gravity3D {
            direction: Qt.vector3d(0, -1, 0)
            magnitude: 100
        }
        
        // ========== 粒子衰减 ==========
        Attractor3D {
            position: Qt.vector3d(0, -100, 0)
            particles: [emitter.particle]
            
            // 吸引力
            magnitude: 50
            
            // 影响范围
            useCachedPositions: false
        }
    }
    
    // 参考地面
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        position: Qt.vector3d(0, -100, 0)
        scale: Qt.vector3d(10, 10, 1)
        materials: PrincipledMaterial {
            baseColor: "#1a1a1a"
        }
    }
}
```

### 11.2 火焰效果

```qml
import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#0a0a0a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 100, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
        brightness: 0.5
    }
    
    // ========== 火焰粒子系统 ==========
    ParticleSystem3D {
        // 火焰发射器
        ParticleEmitter3D {
            id: fireEmitter
            position: Qt.vector3d(0, 0, 0)
            
            particle: SpriteParticle3D {
                sprite: Texture {
                    source: "textures/particle.png"  // 粒子纹理
                }
                
                maxAmount: 500
                
                // 颜色渐变（从黄色到红色到透明）
                color: "#ffff00"
                colorVariation: Qt.vector4d(0.3, 0.3, 0.0, 0.0)
            }
            
            emitRate: 100
            lifeSpan: 2000
            lifeSpanVariation: 500
            
            // 向上发射
            velocity: VectorDirection3D {
                direction: Qt.vector3d(0, 150, 0)
                directionVariation: Qt.vector3d(30, 20, 30)
            }
            
            particleScale: 2.0
            particleScaleVariation: 1.0
            
            // 粒子随时间缩小
            particleEndScale: 0.1
        }
        
        // 向上的力
        Wander3D {
            particles: [fireEmitter.particle]
            uniqueAmount: Qt.vector3d(10, 0, 10)
            uniquePace: Qt.vector3d(1, 0, 1)
        }
        
        // 粒子衰减（淡出效果）
        ParticleAffector3D {
            particles: [fireEmitter.particle]
            
            // 随时间改变颜色
            // 实现火焰从黄色->橙色->红色->透明的渐变
        }
    }
    
    // 火把模型
    Model {
        source: "#Cylinder"
        position: Qt.vector3d(0, -50, 0)
        scale: Qt.vector3d(0.3, 1, 0.3)
        materials: PrincipledMaterial {
            baseColor: "#8b4513"
        }
    }
}
```

### 11.3 爆炸效果

```qml
import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick.Controls

Window {
    width: 1280
    height: 720
    visible: true
    title: "粒子爆炸效果"
    
    View3D {
        anchors.fill: parent
        
        environment: SceneEnvironment {
            clearColor: "#0a0a0a"
            backgroundMode: SceneEnvironment.Color
        }
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 100, 500)
        }
        
        DirectionalLight {
            eulerRotation.x: -30
            brightness: 0.5
        }
        
        // ========== 爆炸粒子系统 ==========
        ParticleSystem3D {
            id: explosionSystem
            
            ParticleEmitter3D {
                id: explosionEmitter
                position: Qt.vector3d(0, 0, 0)
                
                particle: ModelParticle3D {
                    delegate: Model {
                        source: "#Sphere"
                        scale: Qt.vector3d(0.2, 0.2, 0.2)
                        materials: PrincipledMaterial {
                            baseColor: "#ff6b00"
                            lighting: PrincipledMaterial.NoLighting
                        }
                    }
                    
                    maxAmount: 200
                    colorVariation: Qt.vector4d(0.5, 0.3, 0.0, 0.0)
                }
                
                // 爆发式发射
                emitBursts: [
                    EmitBurst3D {
                        time: 0
                        amount: 200
                    }
                ]
                
                lifeSpan: 2000
                lifeSpanVariation: 500
                
                // 向四周爆炸
                velocity: TargetDirection3D {
                    position: Qt.vector3d(0, 0, 0)
                    normalized: true
                    magnitude: 200
                    magnitudeVariation: 100
                }
                
                particleScale: 1.0
                particleEndScale: 0.0  // 粒子逐渐消失
            }
            
            // 重力
            Gravity3D {
                direction: Qt.vector3d(0, -1, 0)
                magnitude: 200
            }
        }
    }
    
    // 触发按钮
    Button {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        text: "触发爆炸"
        
        onClicked: {
            // 重新触发爆炸
            explosionEmitter.burst(200)
        }
    }
}
```



---

## 12. 自定义材质与效果

### 12.1 CustomMaterial 基础

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 0, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 自定义材质 ==========
    Model {
        source: "#Sphere"
        scale: Qt.vector3d(1.5, 1.5, 1.5)
        
        materials: CustomMaterial {
            // ========== 顶点着色器 ==========
            vertexShader: "
                varying vec3 vPosition;
                varying vec3 vNormal;
                varying vec2 vTexCoord;
                
                void MAIN() {
                    // 获取顶点位置
                    vPosition = VERTEX.xyz;
                    vNormal = NORMAL;
                    vTexCoord = UV0;
                    
                    // 变换顶点位置
                    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
                }
            "
            
            // ========== 片段着色器 ==========
            fragmentShader: "
                varying vec3 vPosition;
                varying vec3 vNormal;
                varying vec2 vTexCoord;
                
                void MAIN() {
                    // 简单的光照计算
                    vec3 normal = normalize(vNormal);
                    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
                    
                    float diff = max(dot(normal, lightDir), 0.0);
                    
                    // 基于位置的颜色
                    vec3 color = vec3(
                        (vPosition.x + 50.0) / 100.0,
                        (vPosition.y + 50.0) / 100.0,
                        (vPosition.z + 50.0) / 100.0
                    );
                    
                    // 应用光照
                    color = color * (0.3 + 0.7 * diff);
                    
                    BASE_COLOR = vec4(color, 1.0);
                }
            "
            
            // 着色模式
            shadingMode: CustomMaterial.Shaded
        }
        
        NumberAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 10000
            loops: Animation.Infinite
        }
    }
}
```

### 12.2 动画材质

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#0a0a0a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 0, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 波浪动画材质 ==========
    Model {
        source: "#Sphere"
        scale: Qt.vector3d(1.5, 1.5, 1.5)
        
        materials: CustomMaterial {
            property real time: 0.0
            property real waveAmplitude: 0.1
            property real waveFrequency: 5.0
            property color baseColor: "#4080ff"
            
            // 顶点着色器 - 创建波浪效果
            vertexShader: "
                varying vec3 vPosition;
                varying vec3 vNormal;
                
                uniform float time;
                uniform float waveAmplitude;
                uniform float waveFrequency;
                
                void MAIN() {
                    vPosition = VERTEX.xyz;
                    vNormal = NORMAL;
                    
                    // 波浪变形
                    float wave = sin(VERTEX.x * waveFrequency + time) * 
                                cos(VERTEX.z * waveFrequency + time) * 
                                waveAmplitude;
                    
                    vec3 newPosition = VERTEX.xyz + NORMAL * wave;
                    
                    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(newPosition, 1.0);
                }
            "
            
            // 片段着色器
            fragmentShader: "
                varying vec3 vPosition;
                varying vec3 vNormal;
                
                uniform vec4 baseColor;
                uniform float time;
                
                void MAIN() {
                    vec3 normal = normalize(vNormal);
                    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
                    
                    float diff = max(dot(normal, lightDir), 0.0);
                    
                    // 动态颜色
                    vec3 color = baseColor.rgb;
                    color += vec3(
                        sin(time + vPosition.x) * 0.2,
                        cos(time + vPosition.y) * 0.2,
                        sin(time + vPosition.z) * 0.2
                    );
                    
                    color = color * (0.3 + 0.7 * diff);
                    
                    BASE_COLOR = vec4(color, 1.0);
                }
            "
            
            shadingMode: CustomMaterial.Shaded
            
            // 时间动画
            NumberAnimation on time {
                from: 0
                to: Math.PI * 2
                duration: 3000
                loops: Animation.Infinite
            }
        }
    }
}
```

### 12.3 全息效果

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#0a0a0a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 100, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
        brightness: 0.5
    }
    
    // ========== 全息材质 ==========
    Model {
        source: "#Sphere"
        scale: Qt.vector3d(1.5, 1.5, 1.5)
        
        materials: CustomMaterial {
            property real time: 0.0
            property color hologramColor: "#00ffff"
            property real scanlineSpeed: 2.0
            property real glitchIntensity: 0.1
            
            vertexShader: "
                varying vec3 vPosition;
                varying vec3 vNormal;
                varying vec3 vViewDir;
                
                void MAIN() {
                    vPosition = VERTEX.xyz;
                    vNormal = NORMAL;
                    
                    // 计算视线方向
                    vec4 worldPos = MODEL_MATRIX * vec4(VERTEX, 1.0);
                    vViewDir = normalize(CAMERA_POSITION - worldPos.xyz);
                    
                    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
                }
            "
            
            fragmentShader: "
                varying vec3 vPosition;
                varying vec3 vNormal;
                varying vec3 vViewDir;
                
                uniform vec4 hologramColor;
                uniform float time;
                uniform float scanlineSpeed;
                uniform float glitchIntensity;
                
                void MAIN() {
                    vec3 normal = normalize(vNormal);
                    
                    // 菲涅尔效果（边缘发光）
                    float fresnel = pow(1.0 - max(dot(vViewDir, normal), 0.0), 3.0);
                    
                    // 扫描线效果
                    float scanline = sin(vPosition.y * 20.0 - time * scanlineSpeed) * 0.5 + 0.5;
                    
                    // 故障效果
                    float glitch = sin(time * 10.0 + vPosition.y * 5.0) * glitchIntensity;
                    
                    // 组合效果
                    vec3 color = hologramColor.rgb;
                    color *= (fresnel + 0.2);
                    color *= (scanline * 0.5 + 0.5);
                    color += vec3(glitch);
                    
                    // 透明度
                    float alpha = fresnel * 0.7 + scanline * 0.3;
                    
                    BASE_COLOR = vec4(color, alpha);
                }
            "
            
            shadingMode: CustomMaterial.Unshaded
            cullMode: Material.NoCulling
            depthDrawMode: Material.NeverDepthDraw
            
            // 透明混合
            sourceBlend: CustomMaterial.SrcAlpha
            destinationBlend: CustomMaterial.One
            
            NumberAnimation on time {
                from: 0
                to: Math.PI * 2
                duration: 2000
                loops: Animation.Infinite
            }
        }
        
        NumberAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 10000
            loops: Animation.Infinite
        }
    }
}
```

### 12.4 溶解效果

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 0, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 溶解材质 ==========
    Model {
        source: "#Sphere"
        scale: Qt.vector3d(1.5, 1.5, 1.5)
        
        materials: CustomMaterial {
            property real dissolveAmount: 0.0
            property color edgeColor: "#ff6b00"
            property real edgeWidth: 0.1
            
            vertexShader: "
                varying vec3 vPosition;
                varying vec3 vNormal;
                varying vec2 vTexCoord;
                
                void MAIN() {
                    vPosition = VERTEX.xyz;
                    vNormal = NORMAL;
                    vTexCoord = UV0;
                    
                    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
                }
            "
            
            fragmentShader: "
                varying vec3 vPosition;
                varying vec3 vNormal;
                varying vec2 vTexCoord;
                
                uniform float dissolveAmount;
                uniform vec4 edgeColor;
                uniform float edgeWidth;
                
                // 简单的噪声函数
                float noise(vec3 p) {
                    return fract(sin(dot(p, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
                }
                
                void MAIN() {
                    // 生成噪声值
                    float noiseValue = noise(vPosition * 5.0);
                    
                    // 溶解判断
                    if (noiseValue < dissolveAmount) {
                        discard;  // 丢弃片段
                    }
                    
                    // 边缘发光
                    vec3 normal = normalize(vNormal);
                    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
                    float diff = max(dot(normal, lightDir), 0.0);
                    
                    vec3 color = vec3(0.3, 0.5, 0.8) * (0.3 + 0.7 * diff);
                    
                    // 在溶解边缘添加发光效果
                    if (noiseValue < dissolveAmount + edgeWidth) {
                        float edgeFactor = (noiseValue - dissolveAmount) / edgeWidth;
                        color = mix(edgeColor.rgb, color, edgeFactor);
                    }
                    
                    BASE_COLOR = vec4(color, 1.0);
                }
            "
            
            shadingMode: CustomMaterial.Shaded
            
            // 溶解动画
            SequentialAnimation on dissolveAmount {
                running: true
                loops: Animation.Infinite
                
                NumberAnimation {
                    from: 0.0
                    to: 1.0
                    duration: 3000
                }
                
                PauseAnimation { duration: 500 }
                
                NumberAnimation {
                    from: 1.0
                    to: 0.0
                    duration: 3000
                }
                
                PauseAnimation { duration: 500 }
            }
        }
    }
}
```

---

## 13. 性能优化

### 13.1 LOD（细节层次）

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(0, 100, 500)
        eulerRotation.x: -10
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== LOD 模型 ==========
    Repeater3D {
        model: 20
        
        Node {
            id: lodNode
            position: Qt.vector3d(
                (index % 5) * 200 - 400,
                0,
                Math.floor(index / 5) * 200 - 300
            )
            
            // 计算到相机的距离
            property real distanceToCamera: {
                let dx = position.x - camera.position.x
                let dy = position.y - camera.position.y
                let dz = position.z - camera.position.z
                return Math.sqrt(dx*dx + dy*dy + dz*dz)
            }
            
            // 高细节模型（近距离）
            Model {
                source: "#Sphere"
                visible: lodNode.distanceToCamera < 300
                
                // 高细节材质
                materials: PrincipledMaterial {
                    baseColor: "#4080ff"
                    metalness: 0.8
                    roughness: 0.2
                    
                    normalMap: Texture {
                        source: "textures/normal.jpg"
                    }
                }
            }
            
            // 中等细节模型（中距离）
            Model {
                source: "#Sphere"
                visible: lodNode.distanceToCamera >= 300 && lodNode.distanceToCamera < 600
                
                // 简化材质
                materials: PrincipledMaterial {
                    baseColor: "#4080ff"
                    metalness: 0.5
                    roughness: 0.5
                }
            }
            
            // 低细节模型（远距离）
            Model {
                source: "#Cube"  // 使用更简单的几何体
                visible: lodNode.distanceToCamera >= 600
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                
                // 最简单的材质
                materials: DefaultMaterial {
                    diffuseColor: "#4080ff"
                }
            }
        }
    }
    
    // 性能信息
    Text {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
        color: "white"
        text: "LOD 优化演示\n移动相机查看不同细节层次"
    }
}
```

### 13.2 实例化渲染

```qml
import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 500, 1000)
        eulerRotation.x: -25
    }
    
    DirectionalLight {
        eulerRotation.x: -30
        castsShadow: true
    }
    
    // ========== 实例化渲染（高性能） ==========
    Model {
        source: "#Cube"
        
        // 实例化表
        instancing: InstanceList {
            id: instances
            
            Component.onCompleted: {
                // 创建 1000 个实例
                for (let i = 0; i < 1000; i++) {
                    let x = (Math.random() - 0.5) * 2000
                    let y = Math.random() * 200
                    let z = (Math.random() - 0.5) * 2000
                    
                    let scale = 0.5 + Math.random() * 0.5
                    
                    let hue = Math.random()
                    
                    instances.append({
                        position: Qt.vector3d(x, y, z),
                        scale: Qt.vector3d(scale, scale, scale),
                        rotation: Qt.quaternion(1, 0, 0, 0),
                        color: Qt.hsla(hue, 1.0, 0.5, 1.0)
                    })
                }
            }
        }
        
        materials: PrincipledMaterial {
            baseColor: "#4080ff"
        }
        
        castsShadows: true
    }
    
    // 地面
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        scale: Qt.vector3d(50, 50, 1)
        materials: PrincipledMaterial {
            baseColor: "#2a2a2a"
        }
        receivesShadows: true
    }
    
    // 性能对比说明
    Text {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
        color: "white"
        text: "实例化渲染：1000 个对象\n" +
              "绘制调用：1 次\n" +
              "性能：优秀"
        font.pixelSize: 14
    }
}
```

### 13.3 视锥剔除

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(0, 200, 500)
        eulerRotation.x: -20
        
        // 视锥参数
        fieldOfView: 60
        clipNear: 1
        clipFar: 2000
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 大量对象（自动视锥剔除） ==========
    Repeater3D {
        model: 100
        
        Model {
            source: "#Cube"
            position: Qt.vector3d(
                (index % 10) * 150 - 675,
                0,
                Math.floor(index / 10) * 150 - 675
            )
            
            materials: PrincipledMaterial {
                baseColor: Qt.hsla(index / 100, 1.0, 0.5, 1.0)
            }
            
            // Qt Quick 3D 自动执行视锥剔除
            // 不在视野内的对象不会被渲染
        }
    }
    
    // 相机控制
    MouseArea {
        anchors.fill: parent
        
        property real lastX: 0
        property real lastY: 0
        
        onPressed: (mouse) => {
            lastX = mouse.x
            lastY = mouse.y
        }
        
        onPositionChanged: (mouse) => {
            if (pressed) {
                camera.eulerRotation.y += (mouse.x - lastX) * 0.2
                camera.eulerRotation.x -= (mouse.y - lastY) * 0.2
                lastX = mouse.x
                lastY = mouse.y
            }
        }
    }
    
    Text {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
        color: "white"
        text: "视锥剔除演示\n" +
              "拖拽鼠标旋转相机\n" +
              "只有视野内的对象会被渲染"
    }
}
```

### 13.4 性能监控

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

Window {
    width: 1280
    height: 720
    visible: true
    title: "性能监控"
    
    View3D {
        id: view3D
        anchors.fill: parent
        anchors.rightMargin: 300
        
        environment: SceneEnvironment {
            clearColor: "#1a1a1a"
            backgroundMode: SceneEnvironment.Color
        }
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 200, 500)
            eulerRotation.x: -20
        }
        
        DirectionalLight {
            eulerRotation.x: -30
        }
        
        // 测试场景
        Repeater3D {
            model: objectCountSlider.value
            
            Model {
                source: "#Sphere"
                position: Qt.vector3d(
                    (Math.random() - 0.5) * 1000,
                    Math.random() * 200,
                    (Math.random() - 0.5) * 1000
                )
                
                materials: PrincipledMaterial {
                    baseColor: Qt.hsla(Math.random(), 1.0, 0.5, 1.0)
                }
                
                NumberAnimation on eulerRotation.y {
                    from: 0
                    to: 360
                    duration: 5000 + Math.random() * 5000
                    loops: Animation.Infinite
                }
            }
        }
    }
    
    // ========== 性能面板 ==========
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 300
        color: "#2a2a2a"
        
        Column {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20
            
            Text {
                text: "性能监控"
                color: "white"
                font.bold: true
                font.pixelSize: 18
            }
            
            // FPS 显示
            Rectangle {
                width: parent.width
                height: 80
                color: "#1a1a1a"
                radius: 5
                
                Column {
                    anchors.centerIn: parent
                    spacing: 5
                    
                    Text {
                        text: "FPS"
                        color: "#888888"
                        font.pixelSize: 12
                    }
                    
                    Text {
                        id: fpsText
                        text: fpsCounter.fps.toFixed(1)
                        color: fpsCounter.fps > 50 ? "#00ff00" : 
                               fpsCounter.fps > 30 ? "#ffff00" : "#ff0000"
                        font.pixelSize: 32
                        font.bold: true
                    }
                }
            }
            
            // 对象数量控制
            GroupBox {
                title: "对象数量: " + objectCountSlider.value
                width: parent.width
                
                Slider {
                    id: objectCountSlider
                    width: parent.width
                    from: 10
                    to: 500
                    value: 50
                    stepSize: 10
                }
            }
            
            // 统计信息
            GroupBox {
                title: "统计信息"
                width: parent.width
                
                Column {
                    width: parent.width
                    spacing: 5
                    
                    Text {
                        text: "对象数: " + objectCountSlider.value
                        color: "white"
                        font.pixelSize: 12
                    }
                    
                    Text {
                        text: "帧时间: " + fpsCounter.frameTime.toFixed(2) + " ms"
                        color: "white"
                        font.pixelSize: 12
                    }
                    
                    Text {
                        text: "目标: 60 FPS (16.67 ms)"
                        color: "#888888"
                        font.pixelSize: 10
                    }
                }
            }
            
            // 优化建议
            GroupBox {
                title: "优化建议"
                width: parent.width
                
                Column {
                    width: parent.width
                    spacing: 5
                    
                    Text {
                        text: fpsCounter.fps < 30 ? 
                              "⚠ 性能较低\n• 减少对象数量\n• 使用实例化\n• 降低阴影质量" :
                              fpsCounter.fps < 50 ?
                              "⚡ 性能一般\n• 考虑使用 LOD\n• 优化材质" :
                              "✓ 性能良好"
                        color: "white"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }
                }
            }
        }
    }
    
    // FPS 计数器
    QtObject {
        id: fpsCounter
        property int frameCount: 0
        property real lastTime: Date.now()
        property real fps: 60
        property real frameTime: 16.67
        
        function update() {
            frameCount++
            let currentTime = Date.now()
            let deltaTime = currentTime - lastTime
            
            if (deltaTime >= 1000) {
                fps = frameCount * 1000 / deltaTime
                frameTime = deltaTime / frameCount
                frameCount = 0
                lastTime = currentTime
            }
        }
    }
    
    Timer {
        interval: 16
        running: true
        repeat: true
        onTriggered: fpsCounter.update()
    }
}
```



---

## 14. 实战项目

### 14.1 3D 产品展示器

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 1280
    height: 720
    visible: true
    title: "3D 产品展示器"
    
    View3D {
        id: view3D
        anchors.fill: parent
        
        environment: SceneEnvironment {
            clearColor: "#f5f5f5"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
            
            // 环境光遮蔽
            aoEnabled: true
            aoStrength: 50
        }
        
        // ========== 相机设置 ==========
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 100, 300)
            
            property real targetYaw: 0
            property real targetPitch: -15
            property real currentYaw: 0
            property real currentPitch: -15
            
            // 平滑相机移动
            Behavior on currentYaw {
                NumberAnimation { duration: 500; easing.type: Easing.OutCubic }
            }
            Behavior on currentPitch {
                NumberAnimation { duration: 500; easing.type: Easing.OutCubic }
            }
            
            eulerRotation.x: currentPitch
            eulerRotation.y: currentYaw
        }
        
        // ========== 光照设置 ==========
        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            brightness: 1.0
            castsShadow: true
            shadowMapQuality: Light.ShadowMapQualityHigh
        }
        
        // 补光
        PointLight {
            position: Qt.vector3d(-200, 100, 200)
            brightness: 0.5
            color: "#ffffff"
        }
        
        PointLight {
            position: Qt.vector3d(200, 100, 200)
            brightness: 0.5
            color: "#ffffff"
        }
        
        // ========== 产品模型 ==========
        Node {
            id: productNode
            
            Model {
                id: productModel
                source: "#Sphere"  // 替换为实际产品模型
                scale: Qt.vector3d(1.5, 1.5, 1.5)
                
                materials: PrincipledMaterial {
                    id: productMaterial
                    baseColor: materialColorPicker.selectedColor
                    metalness: metalnessSlider.value
                    roughness: roughnessSlider.value
                    
                    // 可选：添加纹理
                    // baseColorMap: Texture { source: "textures/product.jpg" }
                }
                
                // 自动旋转
                NumberAnimation on eulerRotation.y {
                    running: autoRotateSwitch.checked
                    from: 0
                    to: 360
                    duration: 10000
                    loops: Animation.Infinite
                }
            }
        }
        
        // ========== 展示台 ==========
        Model {
            source: "#Cylinder"
            position: Qt.vector3d(0, -80, 0)
            scale: Qt.vector3d(2, 0.2, 2)
            
            materials: PrincipledMaterial {
                baseColor: "#ffffff"
                metalness: 0.9
                roughness: 0.1
            }
            
            receivesShadows: true
        }
        
        // 背景墙
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0, 0, -200)
            scale: Qt.vector3d(10, 10, 1)
            
            materials: PrincipledMaterial {
                baseColor: "#e0e0e0"
            }
        }
        
        // 地面
        Model {
            source: "#Rectangle"
            eulerRotation.x: -90
            position: Qt.vector3d(0, -100, 0)
            scale: Qt.vector3d(20, 20, 1)
            
            materials: PrincipledMaterial {
                baseColor: "#f5f5f5"
            }
            
            receivesShadows: true
        }
        
        // ========== 鼠标交互 ==========
        MouseArea {
            anchors.fill: parent
            
            property real lastX: 0
            property real lastY: 0
            property bool isDragging: false
            
            onPressed: (mouse) => {
                lastX = mouse.x
                lastY = mouse.y
                isDragging = true
            }
            
            onReleased: {
                isDragging = false
            }
            
            onPositionChanged: (mouse) => {
                if (isDragging && !autoRotateSwitch.checked) {
                    let deltaX = mouse.x - lastX
                    let deltaY = mouse.y - lastY
                    
                    camera.targetYaw += deltaX * 0.3
                    camera.targetPitch -= deltaY * 0.3
                    camera.targetPitch = Math.max(-89, Math.min(0, camera.targetPitch))
                    
                    camera.currentYaw = camera.targetYaw
                    camera.currentPitch = camera.targetPitch
                    
                    lastX = mouse.x
                    lastY = mouse.y
                }
            }
            
            onWheel: (wheel) => {
                let delta = wheel.angleDelta.y / 120
                let newZ = camera.position.z - delta * 20
                camera.position.z = Math.max(150, Math.min(500, newZ))
            }
        }
    }
    
    // ========== 控制面板 ==========
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 320
        color: "#ffffff"
        
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            color: "#f8f8f8"
        }
        
        Flickable {
            anchors.fill: parent
            anchors.margins: 15
            contentHeight: controlColumn.height
            clip: true
            
            Column {
                id: controlColumn
                width: parent.width
                spacing: 20
                
                // 标题
                Text {
                    text: "产品展示控制"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#333333"
                }
                
                Rectangle {
                    width: parent.width
                    height: 1
                    color: "#e0e0e0"
                }
                
                // 视角预设
                GroupBox {
                    title: "视角预设"
                    width: parent.width
                    
                    GridLayout {
                        width: parent.width
                        columns: 2
                        rowSpacing: 8
                        columnSpacing: 8
                        
                        Button {
                            text: "正面"
                            Layout.fillWidth: true
                            onClicked: {
                                camera.targetYaw = 0
                                camera.targetPitch = -15
                            }
                        }
                        
                        Button {
                            text: "背面"
                            Layout.fillWidth: true
                            onClicked: {
                                camera.targetYaw = 180
                                camera.targetPitch = -15
                            }
                        }
                        
                        Button {
                            text: "左侧"
                            Layout.fillWidth: true
                            onClicked: {
                                camera.targetYaw = -90
                                camera.targetPitch = -15
                            }
                        }
                        
                        Button {
                            text: "右侧"
                            Layout.fillWidth: true
                            onClicked: {
                                camera.targetYaw = 90
                                camera.targetPitch = -15
                            }
                        }
                        
                        Button {
                            text: "顶部"
                            Layout.fillWidth: true
                            onClicked: {
                                camera.targetYaw = 0
                                camera.targetPitch = -60
                            }
                        }
                        
                        Button {
                            text: "重置"
                            Layout.fillWidth: true
                            onClicked: {
                                camera.targetYaw = 0
                                camera.targetPitch = -15
                                camera.position.z = 300
                            }
                        }
                    }
                }
                
                // 自动旋转
                Switch {
                    id: autoRotateSwitch
                    text: "自动旋转"
                    checked: false
                }
                
                Rectangle {
                    width: parent.width
                    height: 1
                    color: "#e0e0e0"
                }
                
                // 材质控制
                Text {
                    text: "材质设置"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#333333"
                }
                
                // 颜色选择
                GroupBox {
                    title: "颜色"
                    width: parent.width
                    
                    Column {
                        width: parent.width
                        spacing: 10
                        
                        Flow {
                            id: materialColorPicker
                            width: parent.width
                            spacing: 8
                            
                            property color selectedColor: "#4080ff"
                            
                            Repeater {
                                model: [
                                    "#4080ff", "#ff6b6b", "#ffd700",
                                    "#4ecdc4", "#95e1d3", "#ff8b94",
                                    "#a8e6cf", "#dcedc1", "#ffd3b6"
                                ]
                                
                                Rectangle {
                                    width: 40
                                    height: 40
                                    color: modelData
                                    radius: 20
                                    border.width: materialColorPicker.selectedColor === modelData ? 3 : 1
                                    border.color: materialColorPicker.selectedColor === modelData ? "#333333" : "#cccccc"
                                    
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            materialColorPicker.selectedColor = modelData
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                // 金属度
                GroupBox {
                    title: "金属度: " + metalnessSlider.value.toFixed(2)
                    width: parent.width
                    
                    Slider {
                        id: metalnessSlider
                        width: parent.width
                        from: 0
                        to: 1
                        value: 0.5
                    }
                }
                
                // 粗糙度
                GroupBox {
                    title: "粗糙度: " + roughnessSlider.value.toFixed(2)
                    width: parent.width
                    
                    Slider {
                        id: roughnessSlider
                        width: parent.width
                        from: 0
                        to: 1
                        value: 0.3
                    }
                }
                
                Rectangle {
                    width: parent.width
                    height: 1
                    color: "#e0e0e0"
                }
                
                // 操作提示
                GroupBox {
                    title: "操作提示"
                    width: parent.width
                    
                    Column {
                        width: parent.width
                        spacing: 5
                        
                        Text {
                            text: "• 拖拽鼠标旋转视角"
                            font.pixelSize: 12
                            color: "#666666"
                        }
                        
                        Text {
                            text: "• 滚轮缩放"
                            font.pixelSize: 12
                            color: "#666666"
                        }
                        
                        Text {
                            text: "• 使用预设快速切换视角"
                            font.pixelSize: 12
                            color: "#666666"
                        }
                    }
                }
            }
        }
    }
}
```

### 14.2 3D 数据可视化

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

Window {
    width: 1280
    height: 720
    visible: true
    title: "3D 数据可视化"
    
    // 模拟数据
    ListModel {
        id: dataModel
        
        Component.onCompleted: {
            // 生成随机数据
            for (let i = 0; i < 10; i++) {
                for (let j = 0; j < 10; j++) {
                    append({
                        x: i,
                        z: j,
                        value: Math.random() * 100
                    })
                }
            }
        }
    }
    
    View3D {
        id: view3D
        anchors.fill: parent
        
        environment: SceneEnvironment {
            clearColor: "#1a1a1a"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }
        
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(500, 600, 800)
            eulerRotation.x: -35
            eulerRotation.y: -30
        }
        
        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: -45
            brightness: 1.0
        }
        
        // ========== 3D 柱状图 ==========
        Repeater3D {
            model: dataModel
            
            Model {
                source: "#Cube"
                
                property real barHeight: model.value * 5
                
                position: Qt.vector3d(
                    model.x * 100 - 450,
                    barHeight / 2,
                    model.z * 100 - 450
                )
                
                scale: Qt.vector3d(0.8, barHeight / 100, 0.8)
                
                materials: PrincipledMaterial {
                    baseColor: Qt.hsla(model.value / 100, 1.0, 0.5, 1.0)
                    metalness: 0.3
                    roughness: 0.7
                }
                
                // 平滑动画
                Behavior on barHeight {
                    NumberAnimation {
                        duration: 500
                        easing.type: Easing.OutBack
                    }
                }
                
                // 鼠标悬停
                property bool hovered: false
                
                Behavior on scale {
                    Vector3dAnimation {
                        duration: 200
                    }
                }
            }
        }
        
        // 网格地面
        Model {
            source: "#Rectangle"
            eulerRotation.x: -90
            scale: Qt.vector3d(20, 20, 1)
            
            materials: DefaultMaterial {
                diffuseColor: "#2a2a2a"
            }
        }
        
        // 坐标轴
        // X 轴（红色）
        Model {
            source: "#Cylinder"
            position: Qt.vector3d(0, 0, -500)
            eulerRotation.z: 90
            scale: Qt.vector3d(0.05, 5, 0.05)
            materials: PrincipledMaterial {
                baseColor: "#ff0000"
                lighting: PrincipledMaterial.NoLighting
            }
        }
        
        // Y 轴（绿色）
        Model {
            source: "#Cylinder"
            position: Qt.vector3d(-500, 0, -500)
            scale: Qt.vector3d(0.05, 5, 0.05)
            materials: PrincipledMaterial {
                baseColor: "#00ff00"
                lighting: PrincipledMaterial.NoLighting
            }
        }
        
        // Z 轴（蓝色）
        Model {
            source: "#Cylinder"
            position: Qt.vector3d(-500, 0, 0)
            eulerRotation.x: 90
            scale: Qt.vector3d(0.05, 5, 0.05)
            materials: PrincipledMaterial {
                baseColor: "#0000ff"
                lighting: PrincipledMaterial.NoLighting
            }
        }
        
        // 相机控制
        MouseArea {
            anchors.fill: parent
            
            property real lastX: 0
            property real lastY: 0
            
            onPressed: (mouse) => {
                lastX = mouse.x
                lastY = mouse.y
            }
            
            onPositionChanged: (mouse) => {
                if (pressed) {
                    camera.eulerRotation.y += (mouse.x - lastX) * 0.2
                    camera.eulerRotation.x -= (mouse.y - lastY) * 0.2
                    camera.eulerRotation.x = Math.max(-89, Math.min(0, camera.eulerRotation.x))
                    lastX = mouse.x
                    lastY = mouse.y
                }
            }
        }
    }
    
    // 控制面板
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
        width: 250
        height: 150
        color: "#80000000"
        radius: 10
        
        Column {
            anchors.centerIn: parent
            spacing: 15
            
            Text {
                text: "3D 数据可视化"
                color: "white"
                font.bold: true
                font.pixelSize: 16
            }
            
            Button {
                text: "刷新数据"
                onClicked: {
                    // 更新数据
                    for (let i = 0; i < dataModel.count; i++) {
                        dataModel.setProperty(i, "value", Math.random() * 100)
                    }
                }
            }
            
            Text {
                text: "数据点: " + dataModel.count
                color: "white"
                font.pixelSize: 12
            }
        }
    }
    
    // 图例
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 20
        width: 200
        height: 80
        color: "#80000000"
        radius: 10
        
        Row {
            anchors.centerIn: parent
            spacing: 10
            
            Column {
                spacing: 5
                
                Text {
                    text: "值范围"
                    color: "white"
                    font.pixelSize: 12
                }
                
                Rectangle {
                    width: 150
                    height: 20
                    
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#ff0000" }
                        GradientStop { position: 0.5; color: "#ffff00" }
                        GradientStop { position: 1.0; color: "#00ff00" }
                    }
                }
                
                Row {
                    width: 150
                    
                    Text {
                        text: "0"
                        color: "white"
                        font.pixelSize: 10
                    }
                    
                    Item { width: 60; height: 1 }
                    
                    Text {
                        text: "50"
                        color: "white"
                        font.pixelSize: 10
                    }
                    
                    Item { width: 50; height: 1 }
                    
                    Text {
                        text: "100"
                        color: "white"
                        font.pixelSize: 10
                    }
                }
            }
        }
    }
}
```

---

## 总结

### 核心知识点回顾

1. **Quick 3D 基础**
   - View3D 是 3D 内容的容器
   - SceneEnvironment 配置场景环境
   - 节点树组织 3D 对象

2. **相机系统**
   - PerspectiveCamera：透视投影
   - OrthographicCamera：正交投影
   - 相机控制和交互

3. **3D 模型**
   - 内置几何体：#Cube, #Sphere, #Cylinder 等
   - 加载外部模型：.mesh 格式
   - 实例化渲染提升性能

4. **材质系统**
   - PrincipledMaterial：PBR 材质
   - DefaultMaterial：简单材质
   - CustomMaterial：自定义着色器

5. **光照**
   - DirectionalLight：方向光
   - PointLight：点光源
   - SpotLight：聚光灯
   - 阴影配置

6. **纹理**
   - 基础纹理映射
   - PBR 纹理集
   - 程序化纹理
   - 立方体贴图

7. **动画**
   - 属性动画
   - 路径动画
   - 骨骼动画

8. **交互**
   - 鼠标拾取
   - 拖拽
   - 多点触控

9. **粒子系统**
   - 粒子发射器
   - 粒子效果器
   - 特效制作

10. **性能优化**
    - LOD 细节层次
    - 实例化渲染
    - 视锥剔除
    - 性能监控

### 最佳实践

✅ **DO（推荐）**
- 使用 PBR 材质获得真实感
- 合理使用 LOD 优化性能
- 实例化渲染大量相同对象
- 启用阴影提升真实感
- 使用环境光遮蔽（AO）
- 合理设置相机裁剪平面
- 使用 MSAA 抗锯齿

❌ **DON'T（避免）**
- 不要创建过多独立的 Model
- 避免过高的阴影质量设置
- 不要忽视性能监控
- 避免过度使用透明材质
- 不要在每帧更新大量对象

### 学习路径建议

1. **入门阶段**（1-2 周）
   - 掌握 View3D 和基础组件
   - 学习相机和光照
   - 使用内置几何体

2. **进阶阶段**（2-3 周）
   - 深入材质系统
   - 学习纹理映射
   - 掌握动画技术

3. **高级阶段**（3-4 周）
   - 自定义材质和着色器
   - 粒子系统
   - 性能优化

4. **实战阶段**（持续）
   - 完成实际项目
   - 优化用户体验
   - 探索高级特性

### 常见问题

**Q: Quick 3D 和原生 OpenGL 如何选择？**
A: Quick 3D 适合应用 UI 和数据可视化，开发效率高；原生 OpenGL 适合游戏和专业图形应用，性能最优。

**Q: 如何提升渲染性能？**
A: 使用实例化、LOD、视锥剔除，减少绘制调用，优化材质和阴影。

**Q: 支持哪些 3D 模型格式？**
A: 主要使用 .mesh 格式，其他格式需要用 Balsam 工具转换。

**Q: 如何调试 3D 场景？**
A: 使用 Qt Creator 的 3D 编辑器，启用性能监控，检查节点树结构。

### 参考资源

- **官方文档**: https://doc.qt.io/qt-6/qtquick3d-index.html
- **示例代码**: Qt Creator → Examples → Qt Quick 3D
- **社区论坛**: https://forum.qt.io/
- **视频教程**: Qt 官方 YouTube 频道

---

**恭喜你完成 Qt6 Quick 3D 编程完全指南！** 🎉

现在你已经掌握了 Quick 3D 的核心知识，可以开始创建令人惊叹的 3D 应用了。记住，实践是最好的老师，多动手编写代码，多尝试不同的效果，你会越来越熟练。

祝你在 3D 开发之路上取得成功！🚀
