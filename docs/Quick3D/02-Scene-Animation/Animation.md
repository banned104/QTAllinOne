---
title: Animation
tags:
- QtQuick3D
- Animation
- 中文
---

# Animation

## 是什么

Animation 是 Qt Quick 3D 中的动画系统基础，它继承了 Qt Quick 2D 的动画框架并扩展了对 3D 属性的支持。Qt Quick 3D 的动画系统支持属性动画、关键帧动画、时间轴动画等多种类型，可以对 3D 对象的位置、旋转、缩放、材质属性等进行平滑的动画过渡。

```
Animation System Architecture
├── Animation (基类)
│   ├── PropertyAnimation ← 属性插值动画
│   │   ├── NumberAnimation
│   │   ├── Vector3dAnimation
│   │   └── ColorAnimation
│   ├── SequentialAnimation ← 顺序动画组
│   ├── ParallelAnimation ← 并行动画组
│   └── Timeline ← 时间轴动画
│       └── KeyframeGroup
│           └── Keyframe
```

动画系统在渲染管线中的位置是在场景图更新阶段，动画引擎会在每一帧计算属性值并更新对象状态，然后这些更新的对象参与后续的渲染流程。

## 常用属性一览表

| 属性名 | 类型 | 默认值 | 取值范围 | 作用 | 性能/质量提示 |
| ------ | ---- | ------ | -------- | ---- | -------------- |
| duration★ | int | 250 | 0-∞ (ms) | 动画持续时间 | 避免过短的持续时间，可能导致跳帧 |
| easing★ | EasingCurve | Easing.Linear | 缓动类型 | 控制动画的加速度曲线 | 复杂缓动函数会增加 CPU 开销 |
| loops★ | int | 1 | 1/Animation.Infinite | 循环次数 | 无限循环时注意内存管理 |
| running★ | bool | false | true/false | 动画运行状态 | 及时停止不需要的动画以节省资源 |
| paused | bool | false | true/false | 动画暂停状态 | 暂停比停止更节省资源 |

<details>
<summary>点击查看完整属性列表</summary>

| 属性名 | 类型 | 默认值 | 取值范围 | 作用 | 性能/质量提示 |
| ------ | ---- | ------ | -------- | ---- | -------------- |
| duration | int | 250 | 0-∞ (ms) | 动画持续时间 | 建议最小 16ms |
| easing | EasingCurve | Easing.Linear | 缓动类型 | 动画曲线 | 使用预定义类型 |
| loops | int | 1 | 1/Animation.Infinite | 循环次数 | 注意内存泄漏 |
| running | bool | false | true/false | 运行状态 | 及时清理 |
| paused | bool | false | true/false | 暂停状态 | 优于 stop/start |
| alwaysRunToEnd | bool | false | true/false | 强制完成 | 避免动画中断 |

</details>

## 属性详解

### duration★ - 动画持续时间

duration 控制动画的总时长，单位为毫秒。合理的持续时间对用户体验至关重要：

```qml
// 快速响应动画 (100-300ms)
NumberAnimation {
    target: model
    property: "y"
    duration: 200
    easing.type: Easing.OutQuad
}

// 中等时长动画 (300-800ms)
Vector3dAnimation {
    target: model
    property: "position"
    duration: 500
    easing.type: Easing.InOutCubic
}

// 长时间动画 (1000ms+)
RotationAnimation {
    target: model
    property: "eulerRotation.y"
    duration: 2000
    from: 0
    to: 360
    loops: Animation.Infinite
}
```

性能考虑：
- 持续时间过短（<16ms）可能导致跳帧
- 同时运行的长动画过多会影响性能
- 移动设备上建议缩短动画时间

### easing★ - 缓动曲线

easing 属性控制动画的加速度变化，影响动画的感觉和流畅度：

```qml
// 线性动画 - 匀速运动
PropertyAnimation {
    easing.type: Easing.Linear
    // 适用于：旋转、循环动画
}

// 缓入缓出 - 自然感觉
PropertyAnimation {
    easing.type: Easing.InOutQuad
    // 适用于：位置移动、缩放
}

// 弹性效果 - 生动有趣
PropertyAnimation {
    easing.type: Easing.OutElastic
    easing.amplitude: 1.0
    easing.period: 0.3
    // 适用于：UI 反馈、强调效果
}

// 自定义贝塞尔曲线
PropertyAnimation {
    easing.type: Easing.Bezier
    easing.bezierCurve: [0.25, 0.1, 0.25, 1.0]
    // 精确控制动画曲线
}
```

常用缓动类型选择：
- `Easing.OutQuad`: 快速开始，缓慢结束，适合 UI 响应
- `Easing.InOutCubic`: 平滑的加速和减速，适合位置动画
- `Easing.OutBack`: 超调效果，适合强调动画
- `Easing.OutBounce`: 弹跳效果，适合有趣的交互

### loops★ - 循环控制

loops 属性控制动画的重复次数，支持有限循环和无限循环：

```qml
// 单次动画
PropertyAnimation {
    loops: 1  // 默认值
}

// 有限循环
PropertyAnimation {
    loops: 3
    onFinished: console.log("动画完成")
}

// 无限循环
PropertyAnimation {
    loops: Animation.Infinite
    // 注意：需要手动停止以释放资源
}

// 条件循环控制
PropertyAnimation {
    id: rotationAnim
    loops: Animation.Infinite
    running: mouseArea.containsMouse  // 鼠标悬停时运行
}
```

### running★ - 运行状态控制

running 属性控制动画的启动和停止，是动画生命周期管理的关键：

```qml
PropertyAnimation {
    id: myAnimation
    running: false  // 初始不运行
    
    // 通过信号控制
    Connections {
        target: someObject
        function onTriggerAnimation() {
            myAnimation.running = true
        }
    }
    
    // 动画完成后自动停止
    onFinished: {
        running = false
        console.log("动画完成")
    }
}

// 状态驱动的动画
State {
    name: "expanded"
    PropertyChanges {
        target: expandAnimation
        running: true
    }
}
```

## 最小可运行示例

创建一个综合动画演示，包含位置、旋转、缩放和材质动画：

**文件树：**
```
animation-demo/
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
    title: "Qt Quick 3D Animation Demo"

    View3D {
        anchors.fill: parent
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 10)
        }
        
        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: 30
        }
        
        // 主要演示对象
        Model {
            id: animatedCube
            source: "#Cube"
            
            materials: DefaultMaterial {
                id: cubeMaterial
                diffuseColor: "red"
            }
            
            // 1. 位置动画 - 8字形轨迹
            SequentialAnimation {
                loops: Animation.Infinite
                running: true
                
                ParallelAnimation {
                    NumberAnimation {
                        target: animatedCube
                        property: "x"
                        from: -3; to: 3
                        duration: 2000
                        easing.type: Easing.InOutSine
                    }
                    NumberAnimation {
                        target: animatedCube
                        property: "y"
                        from: -1; to: 1
                        duration: 1000
                        easing.type: Easing.InOutSine
                    }
                }
                
                ParallelAnimation {
                    NumberAnimation {
                        target: animatedCube
                        property: "x"
                        from: 3; to: -3
                        duration: 2000
                        easing.type: Easing.InOutSine
                    }
                    NumberAnimation {
                        target: animatedCube
                        property: "y"
                        from: 1; to: -1
                        duration: 1000
                        easing.type: Easing.InOutSine
                    }
                }
            }
            
            // 2. 旋转动画 - 连续旋转
            RotationAnimation {
                target: animatedCube
                property: "eulerRotation.y"
                from: 0; to: 360
                duration: 3000
                loops: Animation.Infinite
                running: true
            }
            
            // 3. 缩放动画 - 呼吸效果
            SequentialAnimation {
                loops: Animation.Infinite
                running: true
                
                NumberAnimation {
                    target: animatedCube
                    property: "scale.x"
                    from: 1.0; to: 1.5
                    duration: 1500
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    target: animatedCube
                    property: "scale.x"
                    from: 1.5; to: 1.0
                    duration: 1500
                    easing.type: Easing.InOutQuad
                }
            }
            
            // 4. 材质颜色动画
            SequentialAnimation {
                loops: Animation.Infinite
                running: true
                
                ColorAnimation {
                    target: cubeMaterial
                    property: "diffuseColor"
                    from: "red"; to: "blue"
                    duration: 2000
                }
                ColorAnimation {
                    target: cubeMaterial
                    property: "diffuseColor"
                    from: "blue"; to: "green"
                    duration: 2000
                }
                ColorAnimation {
                    target: cubeMaterial
                    property: "diffuseColor"
                    from: "green"; to: "red"
                    duration: 2000
                }
            }
        }
        
        // 关键帧动画示例
        Model {
            id: keyframeCube
            source: "#Cube"
            x: 5
            
            materials: DefaultMaterial {
                diffuseColor: "yellow"
            }
            
            Timeline {
                id: timeline
                startFrame: 0
                endFrame: 1000
                currentFrame: 0
                
                KeyframeGroup {
                    target: keyframeCube
                    property: "y"
                    
                    Keyframe { frame: 0; value: -2 }
                    Keyframe { frame: 250; value: 2; easing.type: Easing.OutBounce }
                    Keyframe { frame: 500; value: 0 }
                    Keyframe { frame: 750; value: -2; easing.type: Easing.InQuad }
                    Keyframe { frame: 1000; value: -2 }
                }
                
                KeyframeGroup {
                    target: keyframeCube
                    property: "eulerRotation.z"
                    
                    Keyframe { frame: 0; value: 0 }
                    Keyframe { frame: 500; value: 180 }
                    Keyframe { frame: 1000; value: 360 }
                }
            }
            
            // 时间轴动画控制
            NumberAnimation {
                target: timeline
                property: "currentFrame"
                from: 0; to: 1000
                duration: 4000
                loops: Animation.Infinite
                running: true
            }
        }
        
        // 交互控制的动画
        Model {
            id: interactiveCube
            source: "#Cube"
            x: -5
            
            materials: DefaultMaterial {
                diffuseColor: "purple"
            }
            
            // 点击触发的弹跳动画
            PropertyAnimation {
                id: bounceAnimation
                target: interactiveCube
                property: "y"
                from: 0; to: 3
                duration: 300
                easing.type: Easing.OutBounce
                
                onFinished: {
                    // 返回动画
                    PropertyAnimation {
                        target: interactiveCube
                        property: "y"
                        to: 0
                        duration: 300
                        easing.type: Easing.InQuad
                        running: true
                    }
                }
            }
        }
    }
    
    // 控制界面
    Column {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
        spacing: 10
        
        Text {
            text: "动画控制"
            font.bold: true
            color: "white"
        }
        
        Button {
            text: "触发弹跳"
            onClicked: bounceAnimation.running = true
        }
        
        Button {
            text: animatedCube.visible ? "隐藏主动画" : "显示主动画"
            onClicked: animatedCube.visible = !animatedCube.visible
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
project(animation-demo)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Quick3D)

qt_add_executable(animation-demo main.cpp)
qt_add_qml_module(animation-demo
    URI AnimationDemo
    VERSION 1.0
    QML_FILES main.qml
)

target_link_libraries(animation-demo Qt6::Core Qt6::Quick Qt6::Quick3D)
```

**预期效果：**
显示三个立方体：红色立方体执行复合动画（8字轨迹+旋转+缩放+颜色变化），黄色立方体执行关键帧动画，紫色立方体响应点击交互。

## 踩坑与调试

### 常见问题

**1. 动画性能问题**
```
警告: Animation system is dropping frames, consider reducing animation complexity
```

解决方案：
```qml
// 错误：同时运行过多复杂动画
ParallelAnimation {
    NumberAnimation { target: obj1; property: "x"; duration: 100 }
    NumberAnimation { target: obj2; property: "y"; duration: 100 }
    NumberAnimation { target: obj3; property: "z"; duration: 100 }
    // ... 50+ 个动画
}

// 正确：分批执行或使用更高效的方法
SequentialAnimation {
    ParallelAnimation {
        // 第一批：最重要的动画
        NumberAnimation { target: obj1; property: "x"; duration: 100 }
        NumberAnimation { target: obj2; property: "y"; duration: 100 }
    }
    ParallelAnimation {
        // 第二批：次要动画
        NumberAnimation { target: obj3; property: "z"; duration: 100 }
    }
}
```

**2. 内存泄漏 - 无限循环动画**
```qml
// 错误：无限动画没有清理机制
PropertyAnimation {
    loops: Animation.Infinite
    running: true
    // 组件销毁时动画仍在运行
}

// 正确：添加清理机制
PropertyAnimation {
    id: infiniteAnim
    loops: Animation.Infinite
    running: true
    
    Component.onDestruction: {
        infiniteAnim.stop()
    }
}
```

**3. 动画冲突问题**
```qml
// 错误：多个动画同时修改同一属性
PropertyAnimation {
    target: model
    property: "x"
    to: 100
    running: true
}
PropertyAnimation {
    target: model
    property: "x"  // 冲突！
    to: -100
    running: true
}

// 正确：使用动画组或状态管理
State {
    name: "moveRight"
    PropertyChanges { target: model; x: 100 }
    transitions: Transition {
        PropertyAnimation { property: "x"; duration: 500 }
    }
}
```

**4. 关键帧动画时序问题**
```qml
// 错误：关键帧时间不连续
KeyframeGroup {
    Keyframe { frame: 0; value: 0 }
    Keyframe { frame: 100; value: 50 }
    Keyframe { frame: 300; value: 100 }  // 跳跃过大
}

// 正确：平滑的关键帧分布
KeyframeGroup {
    Keyframe { frame: 0; value: 0 }
    Keyframe { frame: 100; value: 50 }
    Keyframe { frame: 200; value: 75 }   // 添加中间帧
    Keyframe { frame: 300; value: 100 }
}
```

### 调试技巧

**1. 启用动画调试**
```cpp
// 在 main.cpp 中添加
QLoggingCategory::setFilterRules("qt.quick3d.animation.debug=true");
```

**2. 动画状态监控**
```qml
PropertyAnimation {
    id: debugAnim
    onStarted: console.log("动画开始:", target, property)
    onFinished: console.log("动画结束:", target, property)
    onPaused: console.log("动画暂停")
    onResumed: console.log("动画恢复")
}
```

**3. 性能分析工具**
```qml
// 帧率监控
Text {
    text: "FPS: " + (1000 / frameTimer.interval).toFixed(1)
    
    Timer {
        id: frameTimer
        interval: 16  // 60 FPS
        repeat: true
        running: true
        property real lastTime: Date.now()
        onTriggered: {
            var now = Date.now()
            interval = now - lastTime
            lastTime = now
        }
    }
}
```

**4. 动画曲线可视化**
```qml
// 简单的缓动曲线测试
Rectangle {
    width: 400; height: 200
    
    Rectangle {
        id: testBall
        width: 20; height: 20
        color: "red"
        
        PropertyAnimation {
            target: testBall
            property: "x"
            from: 0; to: 380
            duration: 2000
            easing.type: Easing.OutBounce  // 测试不同缓动类型
            loops: Animation.Infinite
            running: true
        }
    }
}
```

## 延伸阅读

- [Qt Quick Animation and Transitions](https://doc.qt.io/qt-6/qtquick-statesanimations-animations.html) - Qt Quick 动画系统完整文档
- [QML Animation Best Practices](https://doc.qt.io/qt-6/qtquick-performance.html#animations) - 动画性能优化最佳实践

## 附录：完整属性清单

<details>
<summary>Animation 基类完整属性列表</summary>

### 通用动画属性

| 属性名 | 类型 | 默认值 | 说明 |
| ------ | ---- | ------ | ---- |
| duration | int | 250 | 动画持续时间(ms) |
| easing | EasingCurve | Easing.Linear | 缓动曲线 |
| loops | int | 1 | 循环次数 |
| running | bool | false | 运行状态 |
| paused | bool | false | 暂停状态 |
| alwaysRunToEnd | bool | false | 强制完成 |

### PropertyAnimation 特有属性

| 属性名 | 类型 | 默认值 | 说明 |
| ------ | ---- | ------ | ---- |
| target | QtObject | null | 目标对象 |
| property | string | "" | 目标属性 |
| from | var | undefined | 起始值 |
| to | var | undefined | 结束值 |

### Timeline 特有属性

| 属性名 | 类型 | 默认值 | 说明 |
| ------ | ---- | ------ | ---- |
| startFrame | real | 0 | 开始帧 |
| endFrame | real | 0 | 结束帧 |
| currentFrame | real | 0 | 当前帧 |

### 信号

| 信号名 | 说明 |
| ------ | ---- |
| started() | 动画开始 |
| finished() | 动画完成 |
| paused() | 动画暂停 |
| resumed() | 动画恢复 |

### 方法

| 方法名 | 参数 | 说明 |
| ------ | ---- | ---- |
| start() | - | 开始动画 |
| stop() | - | 停止动画 |
| pause() | - | 暂停动画 |
| resume() | - | 恢复动画 |
| restart() | - | 重新开始 |

</details>