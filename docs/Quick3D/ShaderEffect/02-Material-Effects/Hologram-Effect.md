---
title: Hologram Effect - 全息效果
category: Material Effects
complexity: Medium
performance: Medium
tags:
- QtQuick3D
- Shader
- CustomMaterial
- Scanline
- Fresnel
- 中文
---

# Hologram Effect - 全息效果

## 特效简介

全息效果模拟科幻电影中的全息投影视觉，通过扫描线、边缘光、透明度和闪烁等技术，创建未来感十足的显示效果。这是科幻游戏和UI中最受欢迎的视觉效果之一。

**视觉特征：**
- 半透明的幽灵般外观
- 水平扫描线
- 边缘发光（菲涅尔效果）
- 周期性闪烁
- 科幻感的颜色（青色、蓝色）

**适用场景：**
- 科幻游戏UI和HUD
- 全息投影和通讯
- 未来科技展示
- AR/VR界面元素
- 数据可视化

**性能特点：**
- 中等性能开销
- 需要透明度混合
- 适合中高端设备

## 技术原理

### 核心技术

全息效果由多个技术组合而成：

1. **扫描线（Scanlines）**
```glsl
float scanline = sin(worldPos.y * frequency + time * speed);
scanline = scanline * 0.5 + 0.5;  // 映射到0-1
```

2. **菲涅尔边缘光（Fresnel）**
```glsl
vec3 viewDir = normalize(cameraPos - worldPos);
float fresnel = 1.0 - max(dot(normal, viewDir), 0.0);
fresnel = pow(fresnel, fresnelPower);
```

3. **闪烁（Flicker）**
```glsl
float flicker = sin(time * flickerSpeed) * 0.5 + 0.5;
flicker = mix(0.8, 1.0, flicker);
```

4. **透明度**
```glsl
float alpha = baseAlpha * fresnel * flicker * scanline;
```

### 实现步骤

1. **计算扫描线** - 基于世界坐标Y值
2. **计算菲涅尔** - 基于视角和法线
3. **计算闪烁** - 基于时间的周期函数
4. **组合效果** - 混合所有效果
5. **应用透明度** - 设置混合模式

## 着色器实现

### 方式一：原生GLSL（完整控制）

#### 顶点着色器 (hologram.vert)

```glsl
#version 440

layout(location = 0) in vec3 qt_Vertex;
layout(location = 1) in vec3 qt_Normal;
layout(location = 2) in vec2 qt_TexCoord0;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    mat4 qt_ModelMatrix;
    mat4 qt_ViewMatrix;
    mat4 qt_ProjectionMatrix;
    mat4 qt_NormalMatrix;
    float qt_Opacity;
};

out vec3 worldPos;
out vec3 normal;
out vec3 viewDir;
out vec2 texCoord;

void main() {
    texCoord = qt_TexCoord0;
    
    // 世界空间位置
    worldPos = (qt_ModelMatrix * vec4(qt_Vertex, 1.0)).xyz;
    
    // 世界空间法线
    normal = normalize((qt_NormalMatrix * vec4(qt_Normal, 0.0)).xyz);
    
    // 视角方向（从顶点指向相机）
    vec3 cameraPos = -qt_ViewMatrix[3].xyz;
    viewDir = normalize(cameraPos - worldPos);
    
    gl_Position = qt_Matrix * vec4(qt_Vertex, 1.0);
}
```

#### 片段着色器 (hologram.frag)

```glsl
#version 440

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    mat4 qt_ModelMatrix;
    mat4 qt_ViewMatrix;
    mat4 qt_ProjectionMatrix;
    mat4 qt_NormalMatrix;
    float qt_Opacity;
    
    // 自定义属性
    float time;
    vec4 hologramColor;          // 全息颜色
    float scanlineFrequency;     // 扫描线频率
    float scanlineSpeed;         // 扫描线速度
    float scanlineIntensity;     // 扫描线强度
    float fresnelPower;          // 菲涅尔强度
    float flickerSpeed;          // 闪烁速度
    float baseAlpha;             // 基础透明度
};

in vec3 worldPos;
in vec3 normal;
in vec3 viewDir;
in vec2 texCoord;

out vec4 fragColor;

void main() {
    // 标准化向量
    vec3 N = normalize(normal);
    vec3 V = normalize(viewDir);
    
    // 1. 扫描线效果
    float scanline = sin(worldPos.y * scanlineFrequency + time * scanlineSpeed);
    scanline = scanline * 0.5 + 0.5;  // 映射到[0,1]
    scanline = mix(1.0 - scanlineIntensity, 1.0, scanline);
    
    // 2. 菲涅尔边缘光
    float fresnel = 1.0 - max(dot(N, V), 0.0);
    fresnel = pow(fresnel, fresnelPower);
    
    // 3. 闪烁效果
    float flicker = sin(time * flickerSpeed) * 0.5 + 0.5;
    flicker = mix(0.85, 1.0, flicker);
    
    // 4. 组合所有效果
    vec3 color = hologramColor.rgb;
    
    // 增强边缘
    color += vec3(fresnel * 0.5);
    
    // 应用扫描线
    color *= scanline;
    
    // 应用闪烁
    color *= flicker;
    
    // 5. 计算最终透明度
    float alpha = baseAlpha;
    alpha *= (0.3 + fresnel * 0.7);  // 边缘更不透明
    alpha *= flicker;
    alpha *= qt_Opacity;
    
    fragColor = vec4(color, alpha);
}
```

### 方式二：Shader Utilities（简化语法）

#### 顶点着色器 (hologram_simple.vert)

```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vViewDir;

void MAIN()
{
    // 世界空间位置
    vWorldPos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
    
    // 世界空间法线
    vNormal = normalize(NORMAL);
    
    // 视角方向
    vec3 cameraPos = CAMERA_POSITION;
    vViewDir = normalize(cameraPos - vWorldPos);
}
```

#### 片段着色器 (hologram_simple.frag)

```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vViewDir;

void MAIN()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(vViewDir);
    
    // 扫描线
    float scanline = sin(vWorldPos.y * scanlineFrequency + time * scanlineSpeed);
    scanline = scanline * 0.5 + 0.5;
    scanline = mix(1.0 - scanlineIntensity, 1.0, scanline);
    
    // 菲涅尔
    float fresnel = 1.0 - max(dot(N, V), 0.0);
    fresnel = pow(fresnel, fresnelPower);
    
    // 闪烁
    float flicker = sin(time * flickerSpeed) * 0.5 + 0.5;
    flicker = mix(0.85, 1.0, flicker);
    
    // 组合效果
    vec3 color = hologramColor.rgb;
    color += vec3(fresnel * 0.5);
    color *= scanline * flicker;
    
    // 透明度
    float alpha = baseAlpha * (0.3 + fresnel * 0.7) * flicker;
    
    FRAGCOLOR = vec4(color, alpha);
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
    title: "Hologram Effect"

    View3D {
        anchors.fill: parent
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 6)
        }
        
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            
            materials: CustomMaterial {
                id: hologramMaterial
                vertexShader: "qrc:/shaders/hologram.vert"
                fragmentShader: "qrc:/shaders/hologram.frag"
                
                // 透明度混合
                sourceBlend: CustomMaterial.SrcAlpha
                destinationBlend: CustomMaterial.OneMinusSrcAlpha
                depthDrawMode: CustomMaterial.NeverDepthDraw
                cullMode: CustomMaterial.NoCulling
                
                property real time: 0.0
                property color hologramColor: "#00ffff"
                property real scanlineFrequency: 20.0
                property real scanlineSpeed: 2.0
                property real scanlineIntensity: 0.3
                property real fresnelPower: 3.0
                property real flickerSpeed: 8.0
                property real baseAlpha: 0.6
                
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
        
        environment: SceneEnvironment {
            clearColor: "#000000"
            backgroundMode: SceneEnvironment.Color
        }
    }
}
```

## 参数说明

### 核心参数

| 参数名 | 类型 | 默认值 | 作用 | 调整建议 |
| ------ | ---- | ------ | ---- | -------- |
| time | real | 0.0 | 时间参数 | 持续增加 |
| hologramColor | color | 青色 | 全息颜色 | 青色/蓝色最佳 |
| scanlineFrequency | real | 20.0 | 扫描线密度 | 10-50 |
| scanlineSpeed | real | 2.0 | 扫描线速度 | 1-5 |
| scanlineIntensity | real | 0.3 | 扫描线强度 | 0.1-0.5 |
| fresnelPower | real | 3.0 | 边缘光强度 | 2-5 |
| flickerSpeed | real | 8.0 | 闪烁速度 | 5-15 |
| baseAlpha | real | 0.6 | 基础透明度 | 0.4-0.8 |

## 完整示例

### 文件树

```
hologram-effect-example/
├── main.cpp
├── main.qml
├── shaders/
│   ├── hologram.vert
│   ├── hologram.frag
│   ├── hologram_simple.vert
│   └── hologram_simple.frag
└── CMakeLists.txt
```


### main.qml（完整版 - 精简）

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

Window {
    width: 1200; height: 800; visible: true
    title: "Hologram Effect - 全息效果"

    View3D {
        anchors.fill: parent
        
        PerspectiveCamera { position: Qt.vector3d(0, 0, 8) }
        
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2.5, 2.5, 2.5)
            
            materials: CustomMaterial {
                id: hologramMaterial
                vertexShader: "qrc:/shaders/hologram.vert"
                fragmentShader: "qrc:/shaders/hologram.frag"
                
                sourceBlend: CustomMaterial.SrcAlpha
                destinationBlend: CustomMaterial.OneMinusSrcAlpha
                depthDrawMode: CustomMaterial.NeverDepthDraw
                cullMode: CustomMaterial.NoCulling
                
                property real time: 0.0
                property color hologramColor: "#00ffff"
                property real scanlineFrequency: 20.0
                property real scanlineSpeed: 2.0
                property real scanlineIntensity: 0.3
                property real fresnelPower: 3.0
                property real flickerSpeed: 8.0
                property real baseAlpha: 0.6
                
                NumberAnimation on time {
                    from: 0; to: 100; duration: 50000
                    loops: Animation.Infinite
                }
            }
        }
        
        DirectionalLight { eulerRotation.x: -45 }
        environment: SceneEnvironment {
            clearColor: "#000000"
            backgroundMode: SceneEnvironment.Color
        }
    }
}
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(HologramEffectExample)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Quick3D)

qt_add_executable(HologramEffectExample main.cpp)
qt_add_qml_module(HologramEffectExample
    URI HologramEffectExample
    VERSION 1.0
    QML_FILES main.qml
    RESOURCES
        shaders/hologram.vert
        shaders/hologram.frag
        shaders/hologram_simple.vert
        shaders/hologram_simple.frag
)

target_link_libraries(HologramEffectExample PRIVATE
    Qt6::Core Qt6::Quick Qt6::Quick3D
)
```

## 性能优化

- **透明度排序**：从后往前渲染
- **双面渲染**：禁用背面剔除
- **Early-Z**：透明物体无法使用
- **混合模式**：使用标准Alpha混合

## 常见问题

### 问题1：边缘不发光
**解决**：增加fresnelPower值或检查法线计算

### 问题2：闪烁太快
**解决**：降低flickerSpeed值

### 问题3：扫描线不明显
**解决**：增加scanlineIntensity或调整scanlineFrequency

## 进阶技巧

### 数字雨效果
```glsl
// 添加垂直流动的数字效果
float digitalRain = fract(texCoord.y * 10.0 - time * 0.5);
digitalRain = step(0.9, digitalRain);
color += vec3(digitalRain * 0.3);
```

### 故障效果
```glsl
// 随机故障闪烁
float glitch = step(0.95, fract(sin(time * 10.0) * 43758.5453));
alpha *= mix(1.0, 0.3, glitch);
```

## 延伸阅读

1. **菲涅尔效果原理** - https://en.wikipedia.org/wiki/Fresnel_equations
2. **扫描线技术** - https://www.shadertoy.com/view/4sX3Rn
3. **全息投影视觉设计** - 科幻电影参考

---

**下一步：** [Rim Light Effect](../03-Lighting-Effects/Rim-Light-Effect.md)
