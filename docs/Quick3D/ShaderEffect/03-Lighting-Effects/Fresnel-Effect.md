---
title: Fresnel Effect - 菲涅尔效果
category: Lighting Effects
complexity: Medium
performance: Low
tags:
- QtQuick3D
- Shader
- CustomMaterial
- Fresnel
- Reflection
- 中文
---

# Fresnel Effect - 菲涅尔效果

## 特效简介

菲涅尔效果（Fresnel Effect）模拟真实世界中的光学现象：当观察角度接近表面切线时，反射率增加。这个效果在玻璃、水面、金属等材质上非常明显，是实现真实感渲染的关键技术之一。

**视觉特征：**
- 边缘更强的反射
- 视角依赖的反射强度
- 中心区域更透明或暗淡
- 真实的物理光学效果

**适用场景：**
- 玻璃和透明材质
- 水面和液体
- 金属反射
- 宝石和晶体
- 肥皂泡和薄膜
- 车漆和光滑表面

**性能特点：**
- 低性能开销
- 简单的数学计算
- 适合所有设备

## 技术原理

### 菲涅尔方程

完整的菲涅尔方程非常复杂，在实时渲染中通常使用Schlick近似：

```
F = F0 + (1 - F0) * (1 - cos(θ))^5
```

其中：
- F0：垂直入射时的反射率（材质属性）
- θ：视角向量与法线的夹角
- cos(θ) = dot(N, V)

### 简化版本

更简单的版本（类似边缘光）：

```
fresnel = pow(1 - dot(N, V), power)
```

### 物理意义

- **正面观察**：反射率低，可以看到更多折射/透射
- **侧面观察**：反射率高，表面像镜子一样反射

### 不同材质的F0值

| 材质 | F0值 | 说明 |
|------|------|------|
| 水 | 0.02 | 低反射率 |
| 玻璃 | 0.04 | 低反射率 |
| 塑料 | 0.04-0.05 | 低反射率 |
| 宝石 | 0.05-0.17 | 中等反射率 |
| 金属 | 0.5-1.0 | 高反射率 |

## 着色器实现

### 方式一：原生GLSL（完整控制）

#### 顶点着色器 (fresnel.vert)

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
    
    // 视角方向
    vec3 cameraPos = -transpose(mat3(qt_ViewMatrix)) * qt_ViewMatrix[3].xyz;
    viewDir = normalize(cameraPos - worldPos);
    
    gl_Position = qt_Matrix * vec4(qt_Vertex, 1.0);
}
```

#### 片段着色器 (fresnel.frag)

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
    vec4 baseColor;              // 基础颜色（折射/透射颜色）
    vec4 fresnelColor;           // 菲涅尔颜色（反射颜色）
    float fresnelPower;          // 菲涅尔强度
    float fresnelBias;           // 菲涅尔偏移（F0）
    float fresnelScale;          // 菲涅尔缩放
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
    
    // 计算菲涅尔项
    float NdotV = max(dot(N, V), 0.0);
    
    // Schlick近似
    float fresnel = fresnelBias + fresnelScale * pow(1.0 - NdotV, fresnelPower);
    fresnel = clamp(fresnel, 0.0, 1.0);
    
    // 混合基础颜色和菲涅尔颜色
    vec3 finalColor = mix(baseColor.rgb, fresnelColor.rgb, fresnel);
    
    // 可选：添加简单光照
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(N, lightDir), 0.0);
    finalColor *= (0.3 + 0.7 * NdotL);
    
    fragColor = vec4(finalColor, qt_Opacity);
}
```

### 方式二：Shader Utilities（简化语法）

#### 顶点着色器 (fresnel_simple.vert)

```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vViewDir;

void MAIN()
{
    vWorldPos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
    vNormal = normalize((NORMAL_MATRIX * vec4(NORMAL, 0.0)).xyz);
    
    vec3 cameraPos = CAMERA_POSITION;
    vViewDir = normalize(cameraPos - vWorldPos);
}
```

#### 片段着色器 (fresnel_simple.frag)

```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vViewDir;

void MAIN()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(vViewDir);
    
    // 菲涅尔计算
    float NdotV = max(dot(N, V), 0.0);
    float fresnel = fresnelBias + fresnelScale * pow(1.0 - NdotV, fresnelPower);
    fresnel = clamp(fresnel, 0.0, 1.0);
    
    // 混合颜色
    vec3 finalColor = mix(baseColor.rgb, fresnelColor.rgb, fresnel);
    
    // 简单光照
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(N, lightDir), 0.0);
    finalColor *= (0.3 + 0.7 * NdotL);
    
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
    title: "Fresnel Effect"

    View3D {
        anchors.fill: parent
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 8)
        }
        
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2.5, 2.5, 2.5)
            
            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                PropertyAnimation { to: 360; duration: 15000 }
            }
            
            materials: CustomMaterial {
                id: fresnelMaterial
                vertexShader: "qrc:/shaders/fresnel.vert"
                fragmentShader: "qrc:/shaders/fresnel.frag"
                
                property color baseColor: "#1e40af"
                property color fresnelColor: "#7dd3fc"
                property real fresnelPower: 5.0
                property real fresnelBias: 0.0
                property real fresnelScale: 1.0
            }
        }
        
        DirectionalLight {
            eulerRotation.x: -45
        }
        
        environment: SceneEnvironment {
            clearColor: "#0f172a"
            backgroundMode: SceneEnvironment.Color
        }
    }
}
```

## 参数说明

### 核心参数

| 参数名 | 类型 | 默认值 | 作用 | 调整建议 |
| ------ | ---- | ------ | ---- | -------- |
| baseColor | color | 深蓝 | 基础颜色（中心） | 通常是暗色 |
| fresnelColor | color | 浅蓝 | 反射颜色（边缘） | 通常是亮色 |
| fresnelPower | real | 5.0 | 菲涅尔强度 | 1.0-10.0 |
| fresnelBias | real | 0.0 | 最小反射率（F0） | 0.0-0.5 |
| fresnelScale | real | 1.0 | 反射率范围 | 0.5-2.0 |

### 材质预设

**玻璃效果：**
```qml
property color baseColor: "#e0f2fe"
property color fresnelColor: "#ffffff"
property real fresnelPower: 5.0
property real fresnelBias: 0.04
property real fresnelScale: 0.96
```

**水面效果：**
```qml
property color baseColor: "#0c4a6e"
property color fresnelColor: "#7dd3fc"
property real fresnelPower: 3.0
property real fresnelBias: 0.02
property real fresnelScale: 0.98
```

**金属效果：**
```qml
property color baseColor: "#78716c"
property color fresnelColor: "#fafaf9"
property real fresnelPower: 2.0
property real fresnelBias: 0.5
property real fresnelScale: 0.5
```

## 完整示例

### 文件树

```
fresnel-effect-example/
├── main.cpp
├── main.qml
├── shaders/
│   ├── fresnel.vert
│   ├── fresnel.frag
│   ├── fresnel_simple.vert
│   └── fresnel_simple.frag
└── CMakeLists.txt
```


### main.qml（完整版 - 精简）

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

Window {
    width: 1200; height: 800; visible: true
    title: "Fresnel Effect - 菲涅尔效果"

    View3D {
        anchors.fill: parent
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 2, 10)
            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                running: autoRotate.checked
                PropertyAnimation { to: 360; duration: 20000 }
            }
        }
        
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2.5, 2.5, 2.5)
            
            materials: CustomMaterial {
                id: fresnelMaterial
                vertexShader: useSimple.checked ? 
                    "qrc:/shaders/fresnel_simple.vert" : "qrc:/shaders/fresnel.vert"
                fragmentShader: useSimple.checked ? 
                    "qrc:/shaders/fresnel_simple.frag" : "qrc:/shaders/fresnel.frag"
                
                property color baseColor: "#1e40af"
                property color fresnelColor: "#7dd3fc"
                property real fresnelPower: 5.0
                property real fresnelBias: 0.0
                property real fresnelScale: 1.0
            }
        }
        
        DirectionalLight { eulerRotation.x: -45 }
        environment: SceneEnvironment {
            clearColor: "#0f172a"
            backgroundMode: SceneEnvironment.Color
        }
    }
    
    // 控制面板（精简版）
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20
        width: 300
        height: col.height + 40
        color: "#f0f0f0"
        radius: 10
        opacity: 0.95
        
        Column {
            id: col
            anchors.centerIn: parent
            spacing: 10
            width: parent.width - 40
            
            Text { text: "菲涅尔参数"; font.pixelSize: 18; font.bold: true }
            CheckBox { id: useSimple; text: "Shader Utilities" }
            CheckBox { id: autoRotate; text: "自动旋转"; checked: true }
            
            Column {
                width: parent.width
                Text { text: "强度: " + fresnelMaterial.fresnelPower.toFixed(1) }
                Slider { width: parent.width; from: 1; to: 10; value: 5
                    onValueChanged: fresnelMaterial.fresnelPower = value }
            }
            
            Column {
                width: parent.width
                Text { text: "偏移(F0): " + fresnelMaterial.fresnelBias.toFixed(2) }
                Slider { width: parent.width; from: 0; to: 0.5; value: 0
                    onValueChanged: fresnelMaterial.fresnelBias = value }
            }
            
            Text { text: "材质预设"; font.bold: true }
            Row {
                spacing: 5
                Button { text: "玻璃"; onClicked: {
                    fresnelMaterial.baseColor = "#e0f2fe"
                    fresnelMaterial.fresnelColor = "#ffffff"
                    fresnelMaterial.fresnelPower = 5.0
                    fresnelMaterial.fresnelBias = 0.04
                }}
                Button { text: "水面"; onClicked: {
                    fresnelMaterial.baseColor = "#0c4a6e"
                    fresnelMaterial.fresnelColor = "#7dd3fc"
                    fresnelMaterial.fresnelPower = 3.0
                    fresnelMaterial.fresnelBias = 0.02
                }}
                Button { text: "金属"; onClicked: {
                    fresnelMaterial.baseColor = "#78716c"
                    fresnelMaterial.fresnelColor = "#fafaf9"
                    fresnelMaterial.fresnelPower = 2.0
                    fresnelMaterial.fresnelBias = 0.5
                }}
            }
        }
    }
}
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(FresnelEffectExample)
find_package(Qt6 REQUIRED COMPONENTS Core Quick Quick3D)
qt_add_executable(FresnelEffectExample main.cpp)
qt_add_qml_module(FresnelEffectExample
    URI FresnelEffectExample VERSION 1.0
    QML_FILES main.qml
    RESOURCES
        shaders/fresnel.vert shaders/fresnel.frag
        shaders/fresnel_simple.vert shaders/fresnel_simple.frag
)
target_link_libraries(FresnelEffectExample PRIVATE Qt6::Core Qt6::Quick Qt6::Quick3D)
```

## 性能优化

- **极低开销**：仅简单数学运算
- **无纹理**：不需要额外资源
- **适合所有设备**：包括移动端

## 常见问题

### 问题1：边缘不明显
**解决**：增加fresnelPower或调整fresnelScale

### 问题2：整体太亮/太暗
**解决**：调整fresnelBias（F0值）

### 问题3：效果不真实
**解决**：使用物理正确的F0值（见材质预设）

## 进阶技巧

### 彩虹菲涅尔
```glsl
// 基于菲涅尔因子的彩虹色
vec3 rainbow = vec3(
    sin(fresnel * 3.14159 + 0.0) * 0.5 + 0.5,
    sin(fresnel * 3.14159 + 2.094) * 0.5 + 0.5,
    sin(fresnel * 3.14159 + 4.189) * 0.5 + 0.5
);
vec3 finalColor = mix(baseColor.rgb, rainbow, fresnel);
```

### 环境反射
```glsl
// 使用菲涅尔控制环境贴图反射强度
vec3 reflectDir = reflect(-V, N);
vec3 envColor = texture(envMap, reflectDir).rgb;
vec3 finalColor = mix(baseColor.rgb, envColor, fresnel);
```

## 延伸阅读

1. **菲涅尔方程** - https://en.wikipedia.org/wiki/Fresnel_equations
2. **Schlick近似** - https://en.wikipedia.org/wiki/Schlick%27s_approximation
3. **PBR材质** - https://learnopengl.com/PBR/Theory

---

**下一步：** [Flow Effect](../04-Animation-Effects/Flow-Effect.md)
