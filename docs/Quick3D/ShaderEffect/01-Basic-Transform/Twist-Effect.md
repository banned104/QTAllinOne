---
title: Twist Effect - 扭曲效果
category: Basic Transform
complexity: Simple
performance: Low
tags:
- QtQuick3D
- Shader
- CustomMaterial
- Vertex Animation
- 中文
---

# Twist Effect - 扭曲效果

## 特效简介

扭曲效果通过在顶点着色器中对顶点进行旋转变换，创建螺旋扭曲的动画效果。这是一种经典的顶点动画技术，可以产生富有动感的视觉效果，广泛应用于转场动画、特殊效果和创意展示。

**视觉特征：**
- 螺旋状的扭曲变形
- 可控的扭曲强度和范围
- 基于高度的渐进式旋转
- 流畅的动画过渡

**适用场景：**
- 转场动画和过渡效果
- 龙卷风、漩涡效果
- 创意文字和Logo动画
- 魔法效果和能量场
- 产品展示动画

**性能特点：**
- 低性能开销（仅顶点计算）
- 适合移动设备
- 可应用于任意几何体

## 技术原理

### 数学基础

扭曲效果基于绕Y轴的旋转变换，旋转角度随高度变化：

```
angle = y * twistAmount
```

2D旋转矩阵（绕Y轴）：
```
x' = x * cos(angle) - z * sin(angle)
z' = x * sin(angle) + z * cos(angle)
y' = y
```

### 实现步骤

1. **计算旋转角度** - 基于顶点的Y坐标计算旋转角度
2. **应用旋转变换** - 使用旋转矩阵变换X和Z坐标
3. **法线变换** - 同步旋转法线向量
4. **光照计算** - 使用变换后的法线进行光照

### 扭曲公式

```glsl
// 计算旋转角度（基于高度）
float angle = position.y * twistAmount + time * speed;

// 应用旋转
float cosA = cos(angle);
float sinA = sin(angle);
vec3 twisted;
twisted.x = position.x * cosA - position.z * sinA;
twisted.z = position.x * sinA + position.z * cosA;
twisted.y = position.y;
```

## 着色器实现

### 方式一：原生GLSL（完整控制）

适合需要精确控制和复杂效果的场景。

#### 顶点着色器 (twist.vert)

```glsl
#version 440

// 顶点属性输入
layout(location = 0) in vec3 qt_Vertex;
layout(location = 1) in vec3 qt_Normal;
layout(location = 2) in vec2 qt_TexCoord0;

// Uniform缓冲区
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;              // MVP矩阵
    mat4 qt_ModelMatrix;         // 模型矩阵
    mat4 qt_ViewMatrix;          // 视图矩阵
    mat4 qt_ProjectionMatrix;    // 投影矩阵
    mat4 qt_NormalMatrix;        // 法线矩阵
    float qt_Opacity;            // 不透明度
    
    // 自定义属性
    float time;                  // 时间参数
    float twistAmount;           // 扭曲强度
    float speed;                 // 旋转速度
};

// 输出到片段着色器
out vec3 worldPos;               // 世界空间位置
out vec3 normal;                 // 变换后的法线
out vec2 texCoord;               // UV坐标

void main() {
    texCoord = qt_TexCoord0;
    
    // 复制顶点位置
    vec3 pos = qt_Vertex;
    
    // 计算旋转角度（基于Y坐标和时间）
    float angle = pos.y * twistAmount + time * speed;
    
    // 计算旋转的sin和cos值
    float cosA = cos(angle);
    float sinA = sin(angle);
    
    // 应用绕Y轴的旋转变换
    vec3 twistedPos;
    twistedPos.x = pos.x * cosA - pos.z * sinA;
    twistedPos.z = pos.x * sinA + pos.z * cosA;
    twistedPos.y = pos.y;
    
    // 同样变换法线
    vec3 twistedNormal;
    twistedNormal.x = qt_Normal.x * cosA - qt_Normal.z * sinA;
    twistedNormal.z = qt_Normal.x * sinA + qt_Normal.z * cosA;
    twistedNormal.y = qt_Normal.y;
    
    // 转换法线到世界空间
    normal = normalize((qt_NormalMatrix * vec4(twistedNormal, 0.0)).xyz);
    
    // 计算世界空间位置
    worldPos = (qt_ModelMatrix * vec4(twistedPos, 1.0)).xyz;
    
    // 输出裁剪空间位置
    gl_Position = qt_Matrix * vec4(twistedPos, 1.0);
}
```

#### 片段着色器 (twist.frag)

```glsl
#version 440

// Uniform缓冲区
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    mat4 qt_ModelMatrix;
    mat4 qt_ViewMatrix;
    mat4 qt_ProjectionMatrix;
    mat4 qt_NormalMatrix;
    float qt_Opacity;
    
    // 自定义属性
    float time;
    float twistAmount;
    float speed;
    vec4 baseColor;              // 基础颜色
    vec4 highlightColor;         // 高光颜色
};

// 从顶点着色器输入
in vec3 worldPos;
in vec3 normal;
in vec2 texCoord;

// 输出颜色
out vec4 fragColor;

void main() {
    // 标准化法线
    vec3 N = normalize(normal);
    
    // 方向光照
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(N, lightDir), 0.0);
    
    // 基于高度的颜色渐变
    float heightFactor = (worldPos.y + 2.0) / 4.0;
    heightFactor = clamp(heightFactor, 0.0, 1.0);
    vec3 color = mix(baseColor.rgb, highlightColor.rgb, heightFactor);
    
    // 应用光照
    vec3 ambient = color * 0.3;
    vec3 diffuse = color * 0.7 * NdotL;
    vec3 finalColor = ambient + diffuse;
    
    // 输出最终颜色
    fragColor = vec4(finalColor, qt_Opacity);
}
```

### 方式二：Shader Utilities（简化语法）

适合快速原型和简单效果，使用Qt Quick 3D的高级抽象。

#### 顶点着色器 (twist_simple.vert)

```glsl
// 自定义属性
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;

void MAIN()
{
    // 获取顶点位置
    vec3 pos = VERTEX;
    
    // 计算旋转角度
    float angle = pos.y * twistAmount + time * speed;
    float cosA = cos(angle);
    float sinA = sin(angle);
    
    // 应用扭曲变换
    vec3 twistedPos;
    twistedPos.x = pos.x * cosA - pos.z * sinA;
    twistedPos.z = pos.x * sinA + pos.z * cosA;
    twistedPos.y = pos.y;
    
    // 变换法线
    vec3 twistedNormal;
    twistedNormal.x = NORMAL.x * cosA - NORMAL.z * sinA;
    twistedNormal.z = NORMAL.x * sinA + NORMAL.z * cosA;
    twistedNormal.y = NORMAL.y;
    
    // 传递到片段着色器
    vWorldPos = (MODEL_MATRIX * vec4(twistedPos, 1.0)).xyz;
    vNormal = normalize((NORMAL_MATRIX * vec4(twistedNormal, 0.0)).xyz);
    
    // 设置顶点位置
    VERTEX = twistedPos;
}
```

#### 片段着色器 (twist_simple.frag)

```glsl
// 从顶点着色器接收
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;

void MAIN()
{
    // 标准化法线
    vec3 N = normalize(vNormal);
    
    // 简单光照
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(N, lightDir), 0.0);
    
    // 基于高度的颜色
    float heightFactor = (vWorldPos.y + 2.0) / 4.0;
    heightFactor = clamp(heightFactor, 0.0, 1.0);
    vec3 color = mix(baseColor.rgb, highlightColor.rgb, heightFactor);
    
    // 应用光照
    vec3 finalColor = color * (0.3 + 0.7 * NdotL);
    
    // 输出颜色
    FRAGCOLOR = vec4(finalColor, 1.0);
}
```

## QML集成

### 使用原生GLSL

```qml
import QtQuick
import QtQuick3D

Window {
    width: 1200
    height: 800
    visible: true
    title: "Twist Effect - 原生GLSL"

    View3D {
        anchors.fill: parent
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 8)
            lookAt: Qt.vector3d(0, 0, 0)
        }
        
        Model {
            source: "#Cylinder"
            scale: Qt.vector3d(1, 2, 1)
            
            materials: CustomMaterial {
                id: twistMaterial
                vertexShader: "qrc:/shaders/twist.vert"
                fragmentShader: "qrc:/shaders/twist.frag"
                
                property real time: 0.0
                property real twistAmount: 2.0
                property real speed: 1.0
                property color baseColor: "#8b5cf6"
                property color highlightColor: "#c084fc"
                
                NumberAnimation on time {
                    from: 0
                    to: 100
                    duration: 50000
                    loops: Animation.Infinite
                }
            }
        }
        
        DirectionalLight {
            eulerRotation.x: -45
            brightness: 1.0
        }
        
        environment: SceneEnvironment {
            clearColor: "#1a1a2e"
            backgroundMode: SceneEnvironment.Color
        }
    }
}
```

### 使用Shader Utilities

```qml
import QtQuick
import QtQuick3D

Window {
    width: 1200
    height: 800
    visible: true
    title: "Twist Effect - Shader Utilities"

    View3D {
        anchors.fill: parent
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 8)
        }
        
        Model {
            source: "#Cylinder"
            scale: Qt.vector3d(1, 2, 1)
            
            materials: CustomMaterial {
                vertexShader: "qrc:/shaders/twist_simple.vert"
                fragmentShader: "qrc:/shaders/twist_simple.frag"
                
                property real time: 0.0
                property real twistAmount: 2.0
                property real speed: 1.0
                property color baseColor: "#8b5cf6"
                property color highlightColor: "#c084fc"
                
                NumberAnimation on time {
                    from: 0
                    to: 100
                    duration: 50000
                    loops: Animation.Infinite
                }
            }
        }
        
        DirectionalLight {
            eulerRotation.x: -45
        }
    }
}
```

## 参数说明

### 核心参数

| 参数名 | 类型 | 默认值 | 作用 | 调整建议 |
| ------ | ---- | ------ | ---- | -------- |
| time | real | 0.0 | 时间参数，驱动旋转动画 | 使用NumberAnimation持续增加 |
| twistAmount | real | 2.0 | 扭曲强度（每单位高度的旋转弧度） | 1.0-5.0，过大会过度扭曲 |
| speed | real | 1.0 | 旋转速度 | 0.5-3.0，影响动画快慢 |
| baseColor | color | 紫色 | 底部颜色 | 根据场景调整 |
| highlightColor | color | 浅紫 | 顶部颜色 | 通常比baseColor更亮 |

### 参数效果

**twistAmount（扭曲强度）：**
- 小值(0.5-1.5)：轻微扭曲
- 中值(1.5-3.0)：明显的螺旋效果
- 大值(3.0-5.0)：强烈的扭曲变形

**speed（速度）：**
- 小值(0.3-0.8)：缓慢旋转
- 中值(0.8-2.0)：正常速度
- 大值(2.0-5.0)：快速旋转

## 完整示例

### 文件树

```
twist-effect-example/
├── main.cpp
├── main.qml
├── shaders/
│   ├── twist.vert          # 原生GLSL顶点着色器
│   ├── twist.frag          # 原生GLSL片段着色器
│   ├── twist_simple.vert   # Shader Utilities顶点着色器
│   └── twist_simple.frag   # Shader Utilities片段着色器
└── CMakeLists.txt
```


### main.cpp

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    
    if (engine.rootObjects().isEmpty())
        return -1;
    
    return app.exec();
}
```

### main.qml（完整版）

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

Window {
    width: 1200
    height: 800
    visible: true
    title: "Twist Effect - 扭曲效果示例"

    View3D {
        id: view3D
        anchors.fill: parent
        
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 10)
            lookAt: Qt.vector3d(0, 0, 0)
        }
        
        // 扭曲圆柱体
        Model {
            source: "#Cylinder"
            scale: Qt.vector3d(1.5, 3, 1.5)
            
            materials: CustomMaterial {
                id: twistMaterial
                vertexShader: useSimpleShader.checked ? 
                    "qrc:/shaders/twist_simple.vert" : "qrc:/shaders/twist.vert"
                fragmentShader: useSimpleShader.checked ? 
                    "qrc:/shaders/twist_simple.frag" : "qrc:/shaders/twist.frag"
                
                property real time: 0.0
                property real twistAmount: 2.0
                property real speed: 1.0
                property color baseColor: "#8b5cf6"
                property color highlightColor: "#c084fc"
                
                NumberAnimation on time {
                    from: 0
                    to: 100
                    duration: 50000
                    loops: Animation.Infinite
                    running: animationRunning.checked
                }
            }
        }
        
        // 参考网格
        Model {
            source: "#Rectangle"
            y: -3
            scale: Qt.vector3d(10, 10, 1)
            eulerRotation.x: -90
            
            materials: PrincipledMaterial {
                baseColor: "#2a2a3e"
                metalness: 0.0
                roughness: 0.8
            }
        }
        
        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            brightness: 1.2
        }
        
        environment: SceneEnvironment {
            clearColor: "#1a1a2e"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }
    }
    
    // 参数控制面板
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20
        width: 300
        height: controlColumn.height + 40
        color: "#f0f0f0"
        radius: 10
        opacity: 0.95
        
        Column {
            id: controlColumn
            anchors.centerIn: parent
            spacing: 15
            width: parent.width - 40
            
            Text {
                text: "扭曲参数控制"
                font.pixelSize: 18
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            // 着色器类型选择
            CheckBox {
                id: useSimpleShader
                text: "使用 Shader Utilities"
                checked: false
            }
            
            // 动画控制
            CheckBox {
                id: animationRunning
                text: "启用动画"
                checked: true
            }
            
            // 扭曲强度控制
            Column {
                width: parent.width
                spacing: 5
                
                Text {
                    text: "扭曲强度 (Twist Amount): " + twistMaterial.twistAmount.toFixed(2)
                    font.pixelSize: 14
                }
                
                Slider {
                    width: parent.width
                    from: 0.5
                    to: 5.0
                    value: 2.0
                    onValueChanged: twistMaterial.twistAmount = value
                }
            }
            
            // 速度控制
            Column {
                width: parent.width
                spacing: 5
                
                Text {
                    text: "旋转速度 (Speed): " + twistMaterial.speed.toFixed(1)
                    font.pixelSize: 14
                }
                
                Slider {
                    width: parent.width
                    from: 0.3
                    to: 5.0
                    value: 1.0
                    onValueChanged: twistMaterial.speed = value
                }
            }
            
            // 颜色预设
            Text {
                text: "颜色预设"
                font.pixelSize: 14
                font.bold: true
            }
            
            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter
                
                Button {
                    text: "紫色"
                    onClicked: {
                        twistMaterial.baseColor = "#8b5cf6"
                        twistMaterial.highlightColor = "#c084fc"
                    }
                }
                
                Button {
                    text: "蓝色"
                    onClicked: {
                        twistMaterial.baseColor = "#1e40af"
                        twistMaterial.highlightColor = "#60a5fa"
                    }
                }
                
                Button {
                    text: "绿色"
                    onClicked: {
                        twistMaterial.baseColor = "#166534"
                        twistMaterial.highlightColor = "#4ade80"
                    }
                }
            }
            
            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter
                
                Button {
                    text: "橙色"
                    onClicked: {
                        twistMaterial.baseColor = "#c2410c"
                        twistMaterial.highlightColor = "#fb923c"
                    }
                }
                
                Button {
                    text: "粉色"
                    onClicked: {
                        twistMaterial.baseColor = "#be185d"
                        twistMaterial.highlightColor = "#f472b6"
                    }
                }
                
                Button {
                    text: "青色"
                    onClicked: {
                        twistMaterial.baseColor = "#0e7490"
                        twistMaterial.highlightColor = "#22d3ee"
                    }
                }
            }
        }
    }
    
    // 使用说明
    Text {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        text: "使用滑块调整扭曲参数\n切换着色器类型查看不同实现\n点击颜色预设快速切换主题"
        color: "white"
        font.pixelSize: 14
        style: Text.Outline
        styleColor: "black"
        horizontalAlignment: Text.AlignRight
    }
}
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(TwistEffectExample VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Quick3D)

qt_add_executable(TwistEffectExample
    main.cpp
)

qt_add_qml_module(TwistEffectExample
    URI TwistEffectExample
    VERSION 1.0
    QML_FILES
        main.qml
    RESOURCES
        shaders/twist.vert
        shaders/twist.frag
        shaders/twist_simple.vert
        shaders/twist_simple.frag
)

target_link_libraries(TwistEffectExample PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Quick3D
)

# Windows平台设置
if(WIN32)
    set_target_properties(TwistEffectExample PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
```

### 预期效果

运行程序后，你将看到：
- 一个螺旋扭曲的圆柱体
- 圆柱体持续旋转动画
- 从底部到顶部的颜色渐变
- 可以实时切换两种着色器实现
- 可以通过滑块调整扭曲参数
- 六个颜色预设按钮快速切换主题

## 两种着色器方式对比

### 原生GLSL vs Shader Utilities

| 特性 | 原生GLSL | Shader Utilities |
|------|----------|------------------|
| 语法复杂度 | 较高 | 较低 |
| 学习曲线 | 需要GLSL基础 | 更容易上手 |
| 代码长度 | 较长 | 较短 |
| 功能完整性 | 完全控制 | 基本功能 |
| 调试难度 | 标准工具支持 | 依赖Qt工具 |
| 可移植性 | 高（标准GLSL） | 低（Qt特有） |
| 性能 | 可精细优化 | 自动优化 |
| 适用场景 | 复杂效果、生产环境 | 快速原型、简单效果 |

### 何时使用哪种方式？

**使用原生GLSL：**
- 需要精确控制uniform布局
- 复杂的顶点变换
- 需要与其他OpenGL代码集成
- 生产环境的性能优化
- 需要跨平台移植

**使用Shader Utilities：**
- 快速原型验证
- 简单的视觉效果
- 学习着色器基础
- 不需要底层控制

## 性能优化

### 优化建议

1. **顶点密度**
   - 圆柱体使用适当的细分度
   - 过少的顶点会导致扭曲不平滑
   - 推荐：32-64个径向细分

2. **计算优化**
   - sin/cos计算在顶点着色器中进行
   - 避免在片段着色器中重复计算
   - 使用内置函数而非自定义实现

3. **参数调整**
   - 合理的twistAmount避免过度变形
   - 适当的speed保持流畅动画
   - 避免极端参数值

### 性能分析

- **顶点着色器开销**：低（简单的三角函数和矩阵运算）
- **片段着色器开销**：低（基础光照计算）
- **适用设备**：所有设备，包括移动设备
- **推荐面数**：1000-5000个三角形

## 常见问题

### 问题1：扭曲效果不明显

**原因：** 模型顶点密度不足或twistAmount值太小

**解决方案：**
```qml
// 方案1：增加扭曲强度
property real twistAmount: 3.0  // 增大值

// 方案2：使用更高细分度的模型
Model {
    source: "#Cylinder"
    // 确保圆柱体有足够的细分
}

// 方案3：使用自定义高细分网格
```

### 问题2：扭曲后模型变形严重

**原因：** twistAmount值过大

**解决方案：**
```qml
// 降低扭曲强度
property real twistAmount: 1.5  // 使用较小的值

// 或者限制扭曲范围
float angle = clamp(pos.y * twistAmount, -3.14, 3.14);
```

### 问题3：法线计算不正确，光照异常

**原因：** 法线未同步旋转或转换错误

**解决方案：**
```glsl
// 确保法线也进行相同的旋转变换
vec3 twistedNormal;
twistedNormal.x = qt_Normal.x * cosA - qt_Normal.z * sinA;
twistedNormal.z = qt_Normal.x * sinA + qt_Normal.z * cosA;
twistedNormal.y = qt_Normal.y;

// 转换到世界空间
normal = normalize((qt_NormalMatrix * vec4(twistedNormal, 0.0)).xyz);
```

### 问题4：Shader Utilities版本不工作

**原因：** 变量声明或语法错误

**解决方案：**
```glsl
// 确保使用正确的关键字
VARYING vec3 vNormal;  // 不是 varying
void MAIN() { }        // 不是 void main()
VERTEX = pos;          // 不是 gl_Position
```

## 调试技巧

### 可视化调试

```glsl
// 在片段着色器中可视化不同的值

// 1. 可视化旋转角度
float angle = vWorldPos.y * twistAmount;
FRAGCOLOR = vec4(vec3(angle / 6.28), 1.0);

// 2. 可视化法线
FRAGCOLOR = vec4(vNormal * 0.5 + 0.5, 1.0);

// 3. 可视化高度
float h = (vWorldPos.y + 2.0) / 4.0;
FRAGCOLOR = vec4(vec3(h), 1.0);
```

### 参数测试

```qml
// 使用极端值测试
property real twistAmount: 10.0  // 测试大扭曲
property real speed: 10.0        // 测试高速度

// 使用负值测试反向扭曲
property real twistAmount: -2.0
```

## 进阶技巧

### 非线性扭曲

```glsl
// 使用平方或指数函数创建非线性扭曲
float angle = pow(abs(pos.y), 2.0) * sign(pos.y) * twistAmount;
```

### 局部扭曲

```glsl
// 只在特定高度范围内扭曲
float heightFactor = smoothstep(-1.0, 1.0, pos.y);
float angle = pos.y * twistAmount * heightFactor;
```

### 多轴扭曲

```glsl
// 同时绕X轴和Y轴扭曲
float angleY = pos.y * twistAmount;
float angleX = pos.x * twistAmount * 0.5;

// 先绕Y轴旋转
vec3 temp;
temp.x = pos.x * cos(angleY) - pos.z * sin(angleY);
temp.z = pos.x * sin(angleY) + pos.z * cos(angleY);
temp.y = pos.y;

// 再绕X轴旋转
vec3 twisted;
twisted.x = temp.x;
twisted.y = temp.y * cos(angleX) - temp.z * sin(angleX);
twisted.z = temp.y * sin(angleX) + temp.z * cos(angleX);
```

### 波浪扭曲组合

```glsl
// 结合波浪和扭曲效果
float wave = sin(pos.y * 3.0 + time) * 0.2;
pos.x += wave;
pos.z += wave;

// 然后应用扭曲
float angle = pos.y * twistAmount;
// ... 扭曲变换
```

## 延伸阅读

1. **Qt Quick 3D CustomMaterial 官方文档**  
   详细了解CustomMaterial的所有功能和属性设置  
   链接：https://doc.qt.io/qt-6/qml-qtquick3d-custommaterial.html

2. **Qt Quick 3D Shader Utilities**  
   了解Shader Utilities的语法和使用方法  
   链接：https://doc.qt.io/qt-6/qtquick3d-custom.html

3. **旋转矩阵数学原理**  
   深入理解3D旋转变换的数学基础  
   链接：https://en.wikipedia.org/wiki/Rotation_matrix

4. **顶点动画技术**  
   学习更多顶点着色器动画技巧  
   链接：https://www.lighthouse3d.com/tutorials/glsl-tutorial/vertex-shader/

5. **The Book of Shaders - Transformations**  
   学习着色器中的变换技术  
   链接：https://thebookofshaders.com/08/

---

**下一步：** 尝试 [Dissolve Effect](../02-Material-Effects/Dissolve-Effect.md) 学习材质溶解效果
