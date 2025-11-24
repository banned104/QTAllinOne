---
title: Wave Effect - 波浪效果
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

# Wave Effect - 波浪效果

## 特效简介

波浪效果通过在顶点着色器中对顶点位置进行正弦波变换，创建流畅的波浪动画。这是最基础也是最常用的顶点动画效果之一，广泛应用于水面模拟、旗帜飘动、布料动画等场景。

**视觉特征：**
- 平滑的波浪起伏
- 可控的波浪频率和振幅
- 基于时间的连续动画
- 自然的流动感

**适用场景：**
- 水面和海洋效果
- 旗帜和布料飘动
- 能量场和力场
- 地形起伏动画
- 音频可视化

**性能特点：**
- 低性能开销（仅顶点计算）
- 适合移动设备
- 可以应用于高面数模型

## 技术原理

### 数学基础

波浪效果基于正弦函数的周期性特性：

```
y = A * sin(f * x + t)
```

其中：
- `A` (amplitude) - 振幅，控制波浪高度
- `f` (frequency) - 频率，控制波浪密度
- `x` - 空间位置
- `t` (time) - 时间，控制波浪移动

### 实现步骤

1. **顶点位移** - 在顶点着色器中计算正弦波位移
2. **法线修正** - 根据波浪斜率计算新的法线方向
3. **光照计算** - 在片段着色器中使用修正后的法线进行光照

### 法线计算

波浪表面的法线需要根据波浪的切线方向计算：

```
切线 = (1, cos(f * x + t) * f * A, 0)
法线 = normalize(cross(切线, (0, 0, 1)))
```

## 着色器实现

### 顶点着色器 (wave.vert)

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
    float amplitude;             // 波浪振幅
    float frequency;             // 波浪频率
    float speed;                 // 波浪速度
};

// 输出到片段着色器
out vec3 worldPos;               // 世界空间位置
out vec3 normal;                 // 修正后的法线
out vec2 texCoord;               // UV坐标

void main() {
    texCoord = qt_TexCoord0;
    
    // 复制顶点位置
    vec3 pos = qt_Vertex;
    
    // 计算波浪位移
    // 使用x坐标和时间计算正弦波
    float wave = sin(pos.x * frequency + time * speed) * amplitude;
    
    // 应用位移到y轴（高度）
    pos.y += wave;
    
    // 计算波浪的切线方向
    // 切线的y分量是波浪函数的导数
    float dx = cos(pos.x * frequency + time * speed) * frequency * amplitude;
    vec3 tangent = normalize(vec3(1.0, dx, 0.0));
    
    // 通过切线和z轴叉乘计算法线
    normal = normalize(cross(tangent, vec3(0.0, 0.0, 1.0)));
    
    // 转换法线到世界空间
    normal = normalize((qt_NormalMatrix * vec4(normal, 0.0)).xyz);
    
    // 计算世界空间位置
    worldPos = (qt_ModelMatrix * vec4(pos, 1.0)).xyz;
    
    // 输出裁剪空间位置
    gl_Position = qt_Matrix * vec4(pos, 1.0);
}
```

### 片段着色器 (wave.frag)

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
    float amplitude;
    float frequency;
    float speed;
    vec4 baseColor;              // 基础颜色
    vec4 waveColor;              // 波峰颜色
};

// 从顶点着色器输入
in vec3 worldPos;
in vec3 normal;
in vec2 texCoord;

// 输出颜色
out vec4 fragColor;

void main() {
    // 简单的方向光照
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(normal, lightDir), 0.0);
    
    // 基于高度的颜色混合
    // 波峰使用waveColor，波谷使用baseColor
    float heightFactor = (worldPos.y + amplitude) / (2.0 * amplitude);
    heightFactor = clamp(heightFactor, 0.0, 1.0);
    vec3 color = mix(baseColor.rgb, waveColor.rgb, heightFactor);
    
    // 应用光照
    // 环境光 + 漫反射光
    vec3 ambient = color * 0.3;
    vec3 diffuse = color * 0.7 * NdotL;
    vec3 finalColor = ambient + diffuse;
    
    // 输出最终颜色
    fragColor = vec4(finalColor, qt_Opacity);
}
```

## QML集成

### 基础示例

```qml
import QtQuick
import QtQuick3D

Window {
    width: 1200
    height: 800
    visible: true
    title: "Wave Effect Example"

    View3D {
        anchors.fill: parent
        
        // 相机设置
        PerspectiveCamera {
            position: Qt.vector3d(0, 3, 8)
            eulerRotation: Qt.vector3d(-20, 0, 0)
        }
        
        // 波浪平面
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(5, 5, 1)
            
            // 细分平面以获得更平滑的波浪
            // 注意：需要足够的顶点密度才能看到波浪效果
            
            materials: CustomMaterial {
                id: waveMaterial
                
                // 着色器文件路径
                vertexShader: "qrc:/shaders/wave.vert"
                fragmentShader: "qrc:/shaders/wave.frag"
                
                // 波浪参数
                property real time: 0.0
                property real amplitude: 0.3
                property real frequency: 3.0
                property real speed: 2.0
                property color baseColor: "#1e3a8a"  // 深蓝色
                property color waveColor: "#60a5fa"  // 浅蓝色
                
                // 时间动画
                NumberAnimation on time {
                    from: 0
                    to: 100
                    duration: 50000
                    loops: Animation.Infinite
                }
            }
        }
        
        // 光照
        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            brightness: 1.0
        }
        
        // 环境设置
        environment: SceneEnvironment {
            clearColor: "#87ceeb"  // 天蓝色背景
            backgroundMode: SceneEnvironment.Color
        }
    }
}
```

## 参数说明

### 核心参数

| 参数名 | 类型 | 默认值 | 作用 | 调整建议 |
| ------ | ---- | ------ | ---- | -------- |
| time | real | 0.0 | 时间参数，驱动波浪动画 | 使用NumberAnimation持续增加 |
| amplitude | real | 0.3 | 波浪振幅（高度） | 0.1-1.0，过大会显得不自然 |
| frequency | real | 3.0 | 波浪频率（密度） | 1.0-10.0，影响波浪数量 |
| speed | real | 2.0 | 波浪移动速度 | 0.5-5.0，影响动画快慢 |
| baseColor | color | 深蓝 | 波谷颜色 | 根据场景调整 |
| waveColor | color | 浅蓝 | 波峰颜色 | 通常比baseColor更亮 |

### 参数效果

**amplitude（振幅）：**
- 小值(0.1-0.3)：平缓的涟漪
- 中值(0.3-0.6)：明显的波浪
- 大值(0.6-1.0)：剧烈的起伏

**frequency（频率）：**
- 小值(1.0-3.0)：稀疏的长波
- 中值(3.0-6.0)：适中的波浪密度
- 大值(6.0-10.0)：密集的短波

**speed（速度）：**
- 小值(0.5-1.5)：缓慢流动
- 中值(1.5-3.0)：正常速度
- 大值(3.0-5.0)：快速流动

## 完整示例

### 文件树

```
wave-effect-example/
├── main.cpp
├── main.qml
├── shaders/
│   ├── wave.vert
│   └── wave.frag
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
    title: "Wave Effect - 波浪效果示例"

    View3D {
        id: view3D
        anchors.fill: parent
        
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 3, 8)
            eulerRotation: Qt.vector3d(-20, 0, 0)
        }
        
        // 主波浪平面
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(6, 6, 1)
            
            materials: CustomMaterial {
                id: waveMaterial
                vertexShader: "qrc:/shaders/wave.vert"
                fragmentShader: "qrc:/shaders/wave.frag"
                
                property real time: 0.0
                property real amplitude: 0.4
                property real frequency: 4.0
                property real speed: 2.0
                property color baseColor: "#1e3a8a"
                property color waveColor: "#60a5fa"
                
                NumberAnimation on time {
                    from: 0
                    to: 100
                    duration: 50000
                    loops: Animation.Infinite
                }
            }
        }
        
        // 参考立方体（显示波浪高度）
        Model {
            source: "#Cube"
            position: Qt.vector3d(0, 0, 0)
            scale: Qt.vector3d(0.3, 0.3, 0.3)
            
            materials: PrincipledMaterial {
                baseColor: "#ff6b6b"
                metalness: 0.5
                roughness: 0.3
            }
        }
        
        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            brightness: 1.2
            castsShadow: false
        }
        
        environment: SceneEnvironment {
            clearColor: "#87ceeb"
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
        width: 280
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
                text: "波浪参数控制"
                font.pixelSize: 18
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            // 振幅控制
            Column {
                width: parent.width
                spacing: 5
                
                Text {
                    text: "振幅 (Amplitude): " + waveMaterial.amplitude.toFixed(2)
                    font.pixelSize: 14
                }
                
                Slider {
                    width: parent.width
                    from: 0.1
                    to: 1.0
                    value: 0.4
                    onValueChanged: waveMaterial.amplitude = value
                }
            }
            
            // 频率控制
            Column {
                width: parent.width
                spacing: 5
                
                Text {
                    text: "频率 (Frequency): " + waveMaterial.frequency.toFixed(1)
                    font.pixelSize: 14
                }
                
                Slider {
                    width: parent.width
                    from: 1.0
                    to: 10.0
                    value: 4.0
                    onValueChanged: waveMaterial.frequency = value
                }
            }
            
            // 速度控制
            Column {
                width: parent.width
                spacing: 5
                
                Text {
                    text: "速度 (Speed): " + waveMaterial.speed.toFixed(1)
                    font.pixelSize: 14
                }
                
                Slider {
                    width: parent.width
                    from: 0.5
                    to: 5.0
                    value: 2.0
                    onValueChanged: waveMaterial.speed = value
                }
            }
            
            // 颜色预设
            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter
                
                Button {
                    text: "海洋"
                    onClicked: {
                        waveMaterial.baseColor = "#1e3a8a"
                        waveMaterial.waveColor = "#60a5fa"
                    }
                }
                
                Button {
                    text: "岩浆"
                    onClicked: {
                        waveMaterial.baseColor = "#7f1d1d"
                        waveMaterial.waveColor = "#fbbf24"
                    }
                }
                
                Button {
                    text: "草地"
                    onClicked: {
                        waveMaterial.baseColor = "#166534"
                        waveMaterial.waveColor = "#86efac"
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
        text: "使用滑块调整波浪参数\n点击颜色预设快速切换主题"
        color: "white"
        font.pixelSize: 14
        style: Text.Outline
        styleColor: "black"
    }
}
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(WaveEffectExample VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Quick3D)

qt_add_executable(WaveEffectExample
    main.cpp
)

qt_add_qml_module(WaveEffectExample
    URI WaveEffectExample
    VERSION 1.0
    QML_FILES
        main.qml
    RESOURCES
        shaders/wave.vert
        shaders/wave.frag
)

target_link_libraries(WaveEffectExample PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Quick3D
)

# Windows平台设置
if(WIN32)
    set_target_properties(WaveEffectExample PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
```

### 预期效果

运行程序后，你将看到：
- 一个平滑起伏的波浪平面
- 波浪从左向右连续流动
- 波峰和波谷有不同的颜色
- 可以通过滑块实时调整波浪参数
- 三个颜色预设按钮可快速切换主题

## 性能优化

### 优化建议

1. **顶点密度**
   - 使用合适的网格细分度
   - 过少的顶点会导致波浪不平滑
   - 过多的顶点会影响性能
   - 建议：50x50到100x100的网格

2. **计算优化**
   - 波浪计算在顶点着色器中进行，性能开销低
   - 避免在片段着色器中进行复杂计算
   - 使用内置函数（sin, cos）而不是自定义实现

3. **参数调整**
   - 降低frequency可以减少视觉复杂度
   - 适当的amplitude避免过度变形
   - 合理的speed保持流畅动画

### 性能分析

- **顶点着色器开销**：低（简单的三角函数计算）
- **片段着色器开销**：低（基础光照计算）
- **适用设备**：所有设备，包括移动设备
- **推荐面数**：2500-10000个三角形

## 常见问题

### 问题1：波浪不平滑，呈现锯齿状

**原因：** 网格顶点密度不足

**解决方案：**
```qml
// 方案1：使用更细分的网格
Model {
    // 使用自定义网格或更高细分度的模型
    source: "qrc:/models/subdivided_plane.mesh"
}

// 方案2：在Qt Quick 3D中，Rectangle默认细分度较低
// 考虑使用外部建模软件创建高细分平面
```

### 问题2：波浪移动方向不对

**原因：** 时间参数或频率设置问题

**解决方案：**
```glsl
// 在顶点着色器中调整波浪方向
// 沿X轴移动
float wave = sin(pos.x * frequency + time * speed);

// 沿Z轴移动
float wave = sin(pos.z * frequency + time * speed);

// 对角线移动
float wave = sin((pos.x + pos.z) * frequency + time * speed);
```

### 问题3：法线计算不正确，光照异常

**原因：** 法线计算公式错误或未正确转换到世界空间

**解决方案：**
```glsl
// 确保法线计算正确
float dx = cos(pos.x * frequency + time * speed) * frequency * amplitude;
vec3 tangent = normalize(vec3(1.0, dx, 0.0));
normal = normalize(cross(tangent, vec3(0.0, 0.0, 1.0)));

// 转换到世界空间
normal = normalize((qt_NormalMatrix * vec4(normal, 0.0)).xyz);
```

### 问题4：性能问题

**原因：** 网格面数过多或着色器计算复杂

**解决方案：**
1. 降低网格细分度
2. 使用LOD（细节层次）系统
3. 对远处的波浪使用简化版本
4. 考虑使用法线贴图代替几何变形

## 调试技巧

### 可视化调试

```glsl
// 在片段着色器中可视化不同的值

// 1. 可视化法线
fragColor = vec4(normal * 0.5 + 0.5, 1.0);

// 2. 可视化高度
float h = (worldPos.y + amplitude) / (2.0 * amplitude);
fragColor = vec4(vec3(h), 1.0);

// 3. 可视化UV坐标
fragColor = vec4(texCoord, 0.0, 1.0);

// 4. 可视化光照
fragColor = vec4(vec3(NdotL), 1.0);
```

### 参数测试

```qml
// 使用极端值测试
property real amplitude: 2.0  // 测试大振幅
property real frequency: 20.0 // 测试高频率
property real speed: 10.0     // 测试高速度
```

## 进阶技巧

### 多重波浪叠加

```glsl
// 在顶点着色器中叠加多个波浪
float wave1 = sin(pos.x * frequency + time * speed) * amplitude;
float wave2 = sin(pos.x * frequency * 2.0 + time * speed * 1.5) * amplitude * 0.5;
float wave3 = sin(pos.z * frequency * 0.5 + time * speed * 0.8) * amplitude * 0.3;

pos.y += wave1 + wave2 + wave3;
```

### 方向性波浪

```glsl
// 指定波浪传播方向
vec2 waveDir = normalize(vec2(1.0, 0.5));  // 波浪方向
float wave = sin(dot(pos.xz, waveDir) * frequency + time * speed) * amplitude;
```

### 衰减波浪

```glsl
// 基于距离的波浪衰减
float dist = length(pos.xz);
float attenuation = exp(-dist * 0.1);  // 指数衰减
float wave = sin(pos.x * frequency + time * speed) * amplitude * attenuation;
```

## 延伸阅读

1. **Qt Quick 3D CustomMaterial 官方文档**  
   详细了解CustomMaterial的所有功能和属性设置  
   链接：https://doc.qt.io/qt-6/qml-qtquick3d-custommaterial.html

2. **正弦波和三角函数**  
   深入理解波浪效果背后的数学原理  
   链接：https://en.wikipedia.org/wiki/Sine_wave

3. **Gerstner Waves**  
   更真实的海洋波浪模拟算法  
   链接：https://catlikecoding.com/unity/tutorials/flow/waves/

4. **GPU Gems - Chapter 1: Effective Water Simulation**  
   高级水面渲染技术  
   链接：https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models

5. **The Book of Shaders - Patterns**  
   学习更多着色器图案和动画技巧  
   链接：https://thebookofshaders.com/09/

---

**下一步：** 尝试 [Twist Effect](../01-Basic-Transform/Twist-Effect.md) 学习旋转变换效果
