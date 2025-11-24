---
title: Rim Light Effect - 边缘光效果
category: Lighting Effects
complexity: Simple
performance: Low
tags:
- QtQuick3D
- Shader
- CustomMaterial
- Rim Lighting
- View-Dependent
- 中文
---

# Rim Light Effect - 边缘光效果

## 特效简介

边缘光效果（Rim Lighting）是一种视角依赖的光照技术，在物体边缘产生明亮的轮廓光，增强物体的立体感和视觉吸引力。这是游戏和影视中最常用的视觉增强技术之一，可以让角色和物体从背景中脱颖而出。

**视觉特征：**
- 物体边缘的明亮轮廓
- 视角依赖（从不同角度观察效果不同）
- 可控的边缘宽度和强度
- 增强物体的立体感

**适用场景：**
- 角色高光和轮廓
- 选中物体的视觉反馈
- 增强物体的可见性
- 魔法效果和能量场
- 夜间场景的轮廓光
- UI元素的视觉强调

**性能特点：**
- 极低性能开销
- 仅需简单的点积计算
- 适合所有设备包括移动端

## 技术原理

### 数学基础

边缘光基于视角向量和法线向量的点积：

```
rimFactor = 1.0 - dot(normal, viewDir)
```

当视角方向与法线方向垂直时（边缘），点积接近0，rimFactor接近1。
当视角方向与法线方向平行时（正面），点积接近1，rimFactor接近0。

### 增强公式

使用幂函数控制边缘宽度：

```
rimFactor = pow(1.0 - dot(normal, viewDir), rimPower)
```

- rimPower越大，边缘越窄
- rimPower越小，边缘越宽

### 实现步骤

1. **计算视角向量** - 从片段位置指向相机
2. **计算点积** - 视角向量与法线的点积
3. **计算边缘因子** - 使用1减去点积，再应用幂函数
4. **应用边缘光** - 将边缘因子乘以边缘光颜色
5. **混合到最终颜色** - 叠加或混合到基础颜色

## 着色器实现

### 方式一：原生GLSL（完整控制）

#### 顶点着色器 (rim_light.vert)

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
};

// 输出到片段着色器
out vec3 worldPos;               // 世界空间位置
out vec3 normal;                 // 世界空间法线
out vec3 viewDir;                // 视角方向
out vec2 texCoord;               // UV坐标

void main() {
    texCoord = qt_TexCoord0;
    
    // 计算世界空间位置
    worldPos = (qt_ModelMatrix * vec4(qt_Vertex, 1.0)).xyz;
    
    // 转换法线到世界空间
    normal = normalize((qt_NormalMatrix * vec4(qt_Normal, 0.0)).xyz);
    
    // 计算视角方向（从片段指向相机）
    // 从视图矩阵提取相机位置
    vec3 cameraPos = vec3(
        qt_ViewMatrix[0][3],
        qt_ViewMatrix[1][3],
        qt_ViewMatrix[2][3]
    );
    // 反转视图矩阵的旋转部分来获取相机世界位置
    cameraPos = -transpose(mat3(qt_ViewMatrix)) * cameraPos;
    viewDir = normalize(cameraPos - worldPos);
    
    // 输出裁剪空间位置
    gl_Position = qt_Matrix * vec4(qt_Vertex, 1.0);
}
```

#### 片段着色器 (rim_light.frag)

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
    vec4 baseColor;              // 基础颜色
    vec4 rimColor;               // 边缘光颜色
    float rimPower;              // 边缘光强度（控制宽度）
    float rimIntensity;          // 边缘光亮度
};

// 从顶点着色器输入
in vec3 worldPos;
in vec3 normal;
in vec3 viewDir;
in vec2 texCoord;

// 输出颜色
out vec4 fragColor;

void main() {
    // 标准化向量
    vec3 N = normalize(normal);
    vec3 V = normalize(viewDir);
    
    // 基础光照（简单的方向光）
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(N, lightDir), 0.0);
    
    // 计算基础颜色（环境光 + 漫反射）
    vec3 ambient = baseColor.rgb * 0.3;
    vec3 diffuse = baseColor.rgb * 0.7 * NdotL;
    vec3 litColor = ambient + diffuse;
    
    // 计算边缘光因子
    float NdotV = max(dot(N, V), 0.0);
    float rimFactor = 1.0 - NdotV;
    
    // 使用幂函数控制边缘宽度
    rimFactor = pow(rimFactor, rimPower);
    
    // 应用边缘光强度
    rimFactor *= rimIntensity;
    
    // 将边缘光添加到最终颜色
    vec3 rimLight = rimColor.rgb * rimFactor;
    vec3 finalColor = litColor + rimLight;
    
    // 输出最终颜色
    fragColor = vec4(finalColor, qt_Opacity);
}
```

### 方式二：Shader Utilities（简化语法）

#### 顶点着色器 (rim_light_simple.vert)

```glsl
// 传递变量到片段着色器
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vViewDir;

void MAIN()
{
    // 世界空间位置
    vWorldPos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
    
    // 世界空间法线
    vNormal = normalize((NORMAL_MATRIX * vec4(NORMAL, 0.0)).xyz);
    
    // 视角方向
    vec3 cameraPos = CAMERA_POSITION;
    vViewDir = normalize(cameraPos - vWorldPos);
}
```

#### 片段着色器 (rim_light_simple.frag)

```glsl
// 从顶点着色器接收
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vViewDir;

void MAIN()
{
    // 标准化向量
    vec3 N = normalize(vNormal);
    vec3 V = normalize(vViewDir);
    
    // 基础光照
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(N, lightDir), 0.0);
    vec3 litColor = baseColor.rgb * (0.3 + 0.7 * NdotL);
    
    // 边缘光
    float NdotV = max(dot(N, V), 0.0);
    float rimFactor = pow(1.0 - NdotV, rimPower);
    rimFactor *= rimIntensity;
    
    // 组合
    vec3 rimLight = rimColor.rgb * rimFactor;
    vec3 finalColor = litColor + rimLight;
    
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
    title: "Rim Light Effect - 原生GLSL"

    View3D {
        anchors.fill: parent
        
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 8)
            lookAt: Qt.vector3d(0, 0, 0)
        }
        
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            
            // 添加旋转动画以展示视角依赖效果
            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                PropertyAnimation { to: 360; duration: 10000 }
            }
            
            materials: CustomMaterial {
                id: rimLightMaterial
                vertexShader: "qrc:/shaders/rim_light.vert"
                fragmentShader: "qrc:/shaders/rim_light.frag"
                
                property color baseColor: "#3b82f6"
                property color rimColor: "#fbbf24"
                property real rimPower: 3.0
                property real rimIntensity: 2.0
            }
        }
        
        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
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
    title: "Rim Light Effect - Shader Utilities"

    View3D {
        anchors.fill: parent
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 8)
        }
        
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            
            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                PropertyAnimation { to: 360; duration: 10000 }
            }
            
            materials: CustomMaterial {
                vertexShader: "qrc:/shaders/rim_light_simple.vert"
                fragmentShader: "qrc:/shaders/rim_light_simple.frag"
                
                property color baseColor: "#3b82f6"
                property color rimColor: "#fbbf24"
                property real rimPower: 3.0
                property real rimIntensity: 2.0
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
| baseColor | color | 蓝色 | 物体基础颜色 | 根据场景调整 |
| rimColor | color | 橙色 | 边缘光颜色 | 通常使用对比色或高亮色 |
| rimPower | real | 3.0 | 边缘宽度控制 | 1.0-10.0，越大边缘越窄 |
| rimIntensity | real | 2.0 | 边缘光亮度 | 0.5-5.0，控制发光强度 |

### 参数效果

**rimPower（边缘宽度）：**
- 小值(1.0-2.0)：宽边缘，柔和过渡
- 中值(2.0-4.0)：适中边缘
- 大值(4.0-10.0)：窄边缘，锐利轮廓

**rimIntensity（亮度）：**
- 小值(0.5-1.5)：微妙的边缘光
- 中值(1.5-3.0)：明显的边缘光
- 大值(3.0-5.0)：强烈的发光效果

### 颜色搭配建议

| 基础颜色 | 推荐边缘光颜色 | 效果 |
|---------|---------------|------|
| 蓝色 | 橙色/黄色 | 对比强烈，科幻感 |
| 红色 | 白色/黄色 | 火焰效果 |
| 绿色 | 青色/白色 | 能量场 |
| 紫色 | 粉色/白色 | 魔法效果 |
| 灰色 | 蓝色/青色 | 冷光效果 |

## 完整示例

### 文件树

```
rim-light-effect-example/
├── main.cpp
├── main.qml
├── shaders/
│   ├── rim_light.vert
│   ├── rim_light.frag
│   ├── rim_light_simple.vert
│   └── rim_light_simple.frag
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
    title: "Rim Light Effect - 边缘光效果示例"

    View3D {
        id: view3D
        anchors.fill: parent
        
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 2, 10)
            lookAt: Qt.vector3d(0, 0, 0)
            
            // 相机环绕动画
            SequentialAnimation on eulerRotation.y {
                id: cameraAnimation
                loops: Animation.Infinite
                running: autoRotate.checked
                PropertyAnimation { to: 360; duration: 20000 }
            }
        }
        
        // 主展示球体
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2.5, 2.5, 2.5)
            
            materials: CustomMaterial {
                id: rimLightMaterial
                vertexShader: useSimpleShader.checked ? 
                    "qrc:/shaders/rim_light_simple.vert" : "qrc:/shaders/rim_light.vert"
                fragmentShader: useSimpleShader.checked ? 
                    "qrc:/shaders/rim_light_simple.frag" : "qrc:/shaders/rim_light.frag"
                
                property color baseColor: "#3b82f6"
                property color rimColor: "#fbbf24"
                property real rimPower: 3.0
                property real rimIntensity: 2.0
            }
        }
        
        // 对比球体（无边缘光）
        Model {
            source: "#Sphere"
            position: Qt.vector3d(-4, 0, 0)
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            
            materials: PrincipledMaterial {
                baseColor: rimLightMaterial.baseColor
                metalness: 0.0
                roughness: 0.5
            }
        }
        
        // 参考立方体
        Model {
            source: "#Cube"
            position: Qt.vector3d(4, 0, 0)
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            
            materials: CustomMaterial {
                vertexShader: useSimpleShader.checked ? 
                    "qrc:/shaders/rim_light_simple.vert" : "qrc:/shaders/rim_light.vert"
                fragmentShader: useSimpleShader.checked ? 
                    "qrc:/shaders/rim_light_simple.frag" : "qrc:/shaders/rim_light.frag"
                
                property color baseColor: "#10b981"
                property color rimColor: "#fbbf24"
                property real rimPower: rimLightMaterial.rimPower
                property real rimIntensity: rimLightMaterial.rimIntensity
            }
        }
        
        // 地面
        Model {
            source: "#Rectangle"
            y: -3
            scale: Qt.vector3d(15, 15, 1)
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
            clearColor: "#0f172a"
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
        width: 320
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
                text: "边缘光参数控制"
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
            
            // 相机自动旋转
            CheckBox {
                id: autoRotate
                text: "相机自动旋转"
                checked: true
            }
            
            // 边缘宽度控制
            Column {
                width: parent.width
                spacing: 5
                
                Text {
                    text: "边缘宽度 (Rim Power): " + rimLightMaterial.rimPower.toFixed(1)
                    font.pixelSize: 14
                }
                
                Slider {
                    width: parent.width
                    from: 1.0
                    to: 10.0
                    value: 3.0
                    onValueChanged: rimLightMaterial.rimPower = value
                }
                
                Text {
                    text: "提示：值越大，边缘越窄"
                    font.pixelSize: 11
                    color: "#666"
                }
            }
            
            // 边缘亮度控制
            Column {
                width: parent.width
                spacing: 5
                
                Text {
                    text: "边缘亮度 (Intensity): " + rimLightMaterial.rimIntensity.toFixed(1)
                    font.pixelSize: 14
                }
                
                Slider {
                    width: parent.width
                    from: 0.5
                    to: 5.0
                    value: 2.0
                    onValueChanged: rimLightMaterial.rimIntensity = value
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
                    text: "经典"
                    onClicked: {
                        rimLightMaterial.baseColor = "#3b82f6"
                        rimLightMaterial.rimColor = "#fbbf24"
                    }
                }
                
                Button {
                    text: "火焰"
                    onClicked: {
                        rimLightMaterial.baseColor = "#dc2626"
                        rimLightMaterial.rimColor = "#fbbf24"
                    }
                }
                
                Button {
                    text: "冰霜"
                    onClicked: {
                        rimLightMaterial.baseColor = "#0ea5e9"
                        rimLightMaterial.rimColor = "#7dd3fc"
                    }
                }
            }
            
            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter
                
                Button {
                    text: "能量"
                    onClicked: {
                        rimLightMaterial.baseColor = "#8b5cf6"
                        rimLightMaterial.rimColor = "#22d3ee"
                    }
                }
                
                Button {
                    text: "自然"
                    onClicked: {
                        rimLightMaterial.baseColor = "#16a34a"
                        rimLightMaterial.rimColor = "#84cc16"
                    }
                }
                
                Button {
                    text: "暗影"
                    onClicked: {
                        rimLightMaterial.baseColor = "#1e293b"
                        rimLightMaterial.rimColor = "#a855f7"
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
        text: "左侧：无边缘光对比\n中间：边缘光效果\n右侧：立方体边缘光\n\n边缘光是视角依赖的\n旋转相机查看不同角度效果"
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
project(RimLightEffectExample VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Quick3D)

qt_add_executable(RimLightEffectExample
    main.cpp
)

qt_add_qml_module(RimLightEffectExample
    URI RimLightEffectExample
    VERSION 1.0
    QML_FILES
        main.qml
    RESOURCES
        shaders/rim_light.vert
        shaders/rim_light.frag
        shaders/rim_light_simple.vert
        shaders/rim_light_simple.frag
)

target_link_libraries(RimLightEffectExample PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Quick3D
)

# Windows平台设置
if(WIN32)
    set_target_properties(RimLightEffectExample PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
```

### 预期效果

运行程序后，你将看到：
- 三个物体：左侧无边缘光对比球体，中间主展示球体，右侧立方体
- 主球体有明显的边缘发光效果
- 相机自动环绕，展示视角依赖特性
- 可以实时调整边缘宽度和亮度
- 六个颜色预设快速切换不同主题

## 性能优化

### 优化建议

1. **计算位置**
   - 边缘光计算在片段着色器中进行
   - 计算量极小（一次点积 + 一次pow）
   - 无需特殊优化

2. **向量标准化**
   - 确保法线和视角向量已标准化
   - 避免重复标准化
   - 在顶点着色器中预计算可能的值

3. **精度设置**
   - 移动设备可使用mediump精度
   - 桌面设备使用highp获得更好效果

### 性能分析

- **顶点着色器开销**：低（向量计算）
- **片段着色器开销**：极低（简单数学运算）
- **内存占用**：极低（无纹理）
- **适用设备**：所有设备，包括低端移动设备
- **推荐面数**：无限制

### 与其他效果对比

| 效果 | 性能开销 | 视觉影响 | 适用场景 |
|------|----------|----------|----------|
| 边缘光 | 极低 | 中等 | 所有场景 |
| 菲涅尔 | 低 | 高 | 反射材质 |
| 发光 | 中等 | 高 | 特效 |
| 阴影 | 高 | 高 | 真实感 |

## 常见问题

### 问题1：边缘光不明显

**原因：** rimPower值太大或rimIntensity太小

**解决方案：**
```qml
// 降低rimPower使边缘更宽
property real rimPower: 2.0

// 增加rimIntensity使边缘更亮
property real rimIntensity: 3.0
```

### 问题2：边缘光覆盖整个物体

**原因：** rimPower值太小

**解决方案：**
```qml
// 增加rimPower使边缘更窄
property real rimPower: 5.0
```

### 问题3：边缘光颜色不对

**原因：** 颜色混合方式不当

**解决方案：**
```glsl
// 使用加法混合（当前方式）
vec3 finalColor = litColor + rimLight;

// 或使用乘法混合
vec3 finalColor = litColor * (1.0 + rimLight);

// 或使用lerp混合
vec3 finalColor = mix(litColor, rimColor.rgb, rimFactor);
```

### 问题4：法线计算错误

**原因：** 法线未正确转换到世界空间

**解决方案：**
```glsl
// 确保使用法线矩阵
normal = normalize((qt_NormalMatrix * vec4(qt_Normal, 0.0)).xyz);

// 不要直接使用模型矩阵
// 错误：normal = (qt_ModelMatrix * vec4(qt_Normal, 0.0)).xyz;
```

### 问题5：视角方向计算错误

**原因：** 相机位置提取不正确

**解决方案：**
```glsl
// 方法1：从视图矩阵提取（当前方式）
vec3 cameraPos = -transpose(mat3(qt_ViewMatrix)) * qt_ViewMatrix[3].xyz;

// 方法2：在QML中传递相机位置
// property vec3 cameraPosition: camera.position
```

## 调试技巧

### 可视化调试

```glsl
// 在片段着色器中可视化不同的值

// 1. 可视化法线
FRAGCOLOR = vec4(vNormal * 0.5 + 0.5, 1.0);

// 2. 可视化视角方向
FRAGCOLOR = vec4(vViewDir * 0.5 + 0.5, 1.0);

// 3. 可视化点积
float NdotV = max(dot(vNormal, vViewDir), 0.0);
FRAGCOLOR = vec4(vec3(NdotV), 1.0);

// 4. 可视化边缘因子
float rimFactor = pow(1.0 - NdotV, rimPower);
FRAGCOLOR = vec4(vec3(rimFactor), 1.0);

// 5. 只显示边缘光
FRAGCOLOR = vec4(rimColor.rgb * rimFactor, 1.0);
```

### 参数测试

```qml
// 测试极端值
property real rimPower: 1.0    // 测试最宽边缘
property real rimPower: 10.0   // 测试最窄边缘
property real rimIntensity: 10.0  // 测试强发光
```

## 进阶技巧

### 双色边缘光

```glsl
// 内外两层不同颜色的边缘
float rimFactor = pow(1.0 - NdotV, rimPower);

// 外层边缘（宽）
float outerRim = pow(1.0 - NdotV, rimPower * 0.5);
vec3 outerColor = vec3(1.0, 0.5, 0.0);  // 橙色

// 内层边缘（窄）
float innerRim = pow(1.0 - NdotV, rimPower * 1.5);
vec3 innerColor = vec3(1.0, 1.0, 0.0);  // 黄色

// 混合
vec3 rimLight = mix(outerColor * outerRim, innerColor * innerRim, innerRim);
```

### 动态边缘光

```glsl
// 边缘光强度随时间变化
float pulse = sin(time * 3.0) * 0.5 + 0.5;
float dynamicIntensity = rimIntensity * (0.5 + pulse * 0.5);
vec3 rimLight = rimColor.rgb * rimFactor * dynamicIntensity;
```

### 方向性边缘光

```glsl
// 只在特定方向显示边缘光（例如顶部）
vec3 upDir = vec3(0.0, 1.0, 0.0);
float upFactor = max(dot(N, upDir), 0.0);
float rimFactor = pow(1.0 - NdotV, rimPower) * upFactor;
```

### 距离衰减边缘光

```glsl
// 边缘光强度随距离衰减
float dist = length(cameraPos - worldPos);
float attenuation = 1.0 / (1.0 + dist * 0.1);
vec3 rimLight = rimColor.rgb * rimFactor * rimIntensity * attenuation;
```

### 基于高度的边缘光

```glsl
// 边缘光颜色随高度变化
float heightFactor = (worldPos.y + 2.0) / 4.0;
vec3 bottomColor = vec3(0.0, 0.5, 1.0);  // 蓝色
vec3 topColor = vec3(1.0, 1.0, 0.0);     // 黄色
vec3 heightRimColor = mix(bottomColor, topColor, heightFactor);
vec3 rimLight = heightRimColor * rimFactor * rimIntensity;
```

### 与其他效果组合

```glsl
// 边缘光 + 扫描线
float scanline = sin(worldPos.y * 20.0 + time * 2.0) * 0.5 + 0.5;
vec3 rimLight = rimColor.rgb * rimFactor * rimIntensity * scanline;

// 边缘光 + 闪烁
float flicker = sin(time * 8.0) * 0.5 + 0.5;
flicker = mix(0.8, 1.0, flicker);
vec3 rimLight = rimColor.rgb * rimFactor * rimIntensity * flicker;
```

## 实际应用案例

### 1. 角色选中效果

```qml
Model {
    id: character
    property bool selected: false
    
    materials: CustomMaterial {
        vertexShader: "qrc:/shaders/rim_light.vert"
        fragmentShader: "qrc:/shaders/rim_light.frag"
        
        property color baseColor: "#808080"
        property color rimColor: character.selected ? "#00ff00" : "#000000"
        property real rimPower: 3.0
        property real rimIntensity: character.selected ? 3.0 : 0.0
        
        Behavior on rimIntensity {
            NumberAnimation { duration: 300 }
        }
    }
}
```

### 2. 能量护盾效果

```qml
Model {
    source: "#Sphere"
    scale: Qt.vector3d(3, 3, 3)
    
    materials: CustomMaterial {
        vertexShader: "qrc:/shaders/rim_light.vert"
        fragmentShader: "qrc:/shaders/rim_light.frag"
        
        sourceBlend: CustomMaterial.SrcAlpha
        destinationBlend: CustomMaterial.One
        cullMode: CustomMaterial.NoCulling
        
        property color baseColor: "#0080ff"
        property color rimColor: "#00ffff"
        property real rimPower: 2.0
        property real rimIntensity: 4.0
        
        // 脉冲动画
        SequentialAnimation on rimIntensity {
            loops: Animation.Infinite
            PropertyAnimation { to: 6.0; duration: 1000 }
            PropertyAnimation { to: 3.0; duration: 1000 }
        }
    }
}
```

### 3. 夜间轮廓光

```qml
// 在黑暗场景中为角色添加轮廓光
Model {
    source: "character.mesh"
    
    materials: CustomMaterial {
        vertexShader: "qrc:/shaders/rim_light.vert"
        fragmentShader: "qrc:/shaders/rim_light.frag"
        
        property color baseColor: "#404040"
        property color rimColor: "#4080ff"  // 月光色
        property real rimPower: 4.0
        property real rimIntensity: 2.5
    }
}
```

## 延伸阅读

1. **Qt Quick 3D CustomMaterial 官方文档**  
   详细了解CustomMaterial的所有功能  
   链接：https://doc.qt.io/qt-6/qml-qtquick3d-custommaterial.html

2. **Rim Lighting 技术详解**  
   深入理解边缘光的数学原理和应用  
   链接：https://en.wikipedia.org/wiki/Rim_lighting

3. **View-Dependent Lighting**  
   学习视角依赖光照的各种技术  
   链接：https://learnopengl.com/Advanced-Lighting/Advanced-Lighting

4. **GPU Gems - Rim Lighting Techniques**  
   高级边缘光技术和优化  
   链接：https://developer.nvidia.com/gpugems/gpugems/contributors

5. **Unity Rim Lighting Tutorial**  
   可视化理解边缘光效果  
   链接：https://www.youtube.com/results?search_query=rim+lighting+shader

---

**下一步：** 尝试 [Fresnel Effect](../03-Lighting-Effects/Fresnel-Effect.md) 学习菲涅尔反射效果
