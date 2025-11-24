---
title: Dissolve Effect - 溶解效果
category: Material Effects
complexity: Medium
performance: Medium
tags:
- QtQuick3D
- Shader
- CustomMaterial
- Alpha Test
- Texture Sampling
- 中文
---

# Dissolve Effect - 溶解效果

## 特效简介

溶解效果通过噪声纹理和Alpha测试，创建物体逐渐消失或出现的视觉效果。这是游戏和影视中最常用的特效之一，可以模拟物体的消失、传送、死亡等场景，配合边缘发光可以产生非常炫酷的视觉效果。

**视觉特征：**
- 不规则的溶解边缘
- 可控的溶解进度
- 边缘发光效果
- 平滑的过渡动画

**适用场景：**
- 角色死亡和消失效果
- 传送门和传送效果
- 物体生成和销毁
- 转场动画
- 魔法效果和能量消散

**性能特点：**
- 中等性能开销（纹理采样 + discard）
- 需要一张噪声纹理
- discard操作会影响Early-Z优化
- 适合中高端设备

## 技术原理

### 核心算法

溶解效果基于噪声纹理采样和阈值比较：

```
noise = texture(noiseTexture, uv).r
if (noise < dissolveAmount) {
    discard;  // 丢弃该像素
}
```

### 边缘发光原理

在溶解边缘附近添加发光效果：

```
edgeWidth = 0.1
if (noise < dissolveAmount + edgeWidth) {
    // 在边缘区域，混合发光颜色
    edgeFactor = smoothstep(dissolveAmount, dissolveAmount + edgeWidth, noise)
    color = mix(glowColor, baseColor, edgeFactor)
}
```

### 实现步骤

1. **纹理采样** - 从噪声纹理中采样灰度值
2. **阈值测试** - 比较噪声值与溶解阈值
3. **像素丢弃** - 使用discard丢弃低于阈值的像素
4. **边缘检测** - 检测溶解边缘区域
5. **边缘发光** - 在边缘区域混合发光颜色

## 着色器实现

### 方式一：原生GLSL（完整控制）

#### 顶点着色器 (dissolve.vert)

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
out vec3 normal;                 // 法线
out vec2 texCoord;               // UV坐标

void main() {
    texCoord = qt_TexCoord0;
    
    // 转换法线到世界空间
    normal = normalize((qt_NormalMatrix * vec4(qt_Normal, 0.0)).xyz);
    
    // 计算世界空间位置
    worldPos = (qt_ModelMatrix * vec4(qt_Vertex, 1.0)).xyz;
    
    // 输出裁剪空间位置
    gl_Position = qt_Matrix * vec4(qt_Vertex, 1.0);
}
```

#### 片段着色器 (dissolve.frag)

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
    float dissolveAmount;        // 溶解程度 (0.0 = 完全显示, 1.0 = 完全消失)
    float edgeWidth;             // 边缘宽度
    vec4 baseColor;              // 基础颜色
    vec4 edgeColor;              // 边缘发光颜色
    float edgeIntensity;         // 边缘发光强度
};

// 噪声纹理
layout(binding = 1) uniform sampler2D noiseTexture;

// 从顶点着色器输入
in vec3 worldPos;
in vec3 normal;
in vec2 texCoord;

// 输出颜色
out vec4 fragColor;

void main() {
    // 采样噪声纹理
    float noise = texture(noiseTexture, texCoord).r;
    
    // Alpha测试：如果噪声值小于溶解阈值，丢弃该像素
    if (noise < dissolveAmount) {
        discard;
    }
    
    // 标准化法线
    vec3 N = normalize(normal);
    
    // 简单的方向光照
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(N, lightDir), 0.0);
    
    // 基础光照颜色
    vec3 ambient = baseColor.rgb * 0.3;
    vec3 diffuse = baseColor.rgb * 0.7 * NdotL;
    vec3 litColor = ambient + diffuse;
    
    // 边缘发光效果
    // 计算到溶解边缘的距离
    float edgeDistance = noise - dissolveAmount;
    
    // 如果在边缘区域内
    if (edgeDistance < edgeWidth) {
        // 使用smoothstep创建平滑过渡
        float edgeFactor = smoothstep(0.0, edgeWidth, edgeDistance);
        
        // 混合边缘发光颜色
        vec3 glowColor = edgeColor.rgb * edgeIntensity;
        litColor = mix(glowColor, litColor, edgeFactor);
    }
    
    // 输出最终颜色
    fragColor = vec4(litColor, qt_Opacity);
}
```

### 方式二：Shader Utilities（简化语法）

#### 顶点着色器 (dissolve_simple.vert)

```glsl
// 传递变量到片段着色器
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec2 vTexCoord;

void MAIN()
{
    // 传递UV坐标
    vTexCoord = UV0;
    
    // 传递世界空间位置和法线
    vWorldPos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
    vNormal = normalize(NORMAL);
}
```

#### 片段着色器 (dissolve_simple.frag)

```glsl
// 从顶点着色器接收
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec2 vTexCoord;

void MAIN()
{
    // 采样噪声纹理
    float noise = texture(noiseTexture, vTexCoord).r;
    
    // Alpha测试
    if (noise < dissolveAmount) {
        discard;
    }
    
    // 标准化法线
    vec3 N = normalize(vNormal);
    
    // 简单光照
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(N, lightDir), 0.0);
    
    // 基础颜色
    vec3 litColor = baseColor.rgb * (0.3 + 0.7 * NdotL);
    
    // 边缘发光
    float edgeDistance = noise - dissolveAmount;
    if (edgeDistance < edgeWidth) {
        float edgeFactor = smoothstep(0.0, edgeWidth, edgeDistance);
        vec3 glowColor = edgeColor.rgb * edgeIntensity;
        litColor = mix(glowColor, litColor, edgeFactor);
    }
    
    // 输出颜色
    FRAGCOLOR = vec4(litColor, 1.0);
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
    title: "Dissolve Effect - 原生GLSL"

    View3D {
        anchors.fill: parent
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 6)
        }
        
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            
            materials: CustomMaterial {
                id: dissolveMaterial
                vertexShader: "qrc:/shaders/dissolve.vert"
                fragmentShader: "qrc:/shaders/dissolve.frag"
                
                property real dissolveAmount: 0.0
                property real edgeWidth: 0.1
                property color baseColor: "#3b82f6"
                property color edgeColor: "#fbbf24"
                property real edgeIntensity: 3.0
                
                property Texture noiseTexture: Texture {
                    source: "qrc:/textures/noise.png"
                    generateMipmaps: true
                    minFilter: Texture.Linear
                    magFilter: Texture.Linear
                }
                
                SequentialAnimation on dissolveAmount {
                    loops: Animation.Infinite
                    PropertyAnimation { to: 1.0; duration: 3000 }
                    PauseAnimation { duration: 500 }
                    PropertyAnimation { to: 0.0; duration: 3000 }
                    PauseAnimation { duration: 500 }
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
    title: "Dissolve Effect - Shader Utilities"

    View3D {
        anchors.fill: parent
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 6)
        }
        
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            
            materials: CustomMaterial {
                vertexShader: "qrc:/shaders/dissolve_simple.vert"
                fragmentShader: "qrc:/shaders/dissolve_simple.frag"
                
                property real dissolveAmount: 0.0
                property real edgeWidth: 0.1
                property color baseColor: "#3b82f6"
                property color edgeColor: "#fbbf24"
                property real edgeIntensity: 3.0
                
                property Texture noiseTexture: Texture {
                    source: "qrc:/textures/noise.png"
                }
                
                SequentialAnimation on dissolveAmount {
                    loops: Animation.Infinite
                    PropertyAnimation { to: 1.0; duration: 3000 }
                    PropertyAnimation { to: 0.0; duration: 3000 }
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
| dissolveAmount | real | 0.0 | 溶解程度 (0.0-1.0) | 0.0=完全显示, 1.0=完全消失 |
| edgeWidth | real | 0.1 | 边缘发光宽度 | 0.05-0.2，过大会显得不自然 |
| baseColor | color | 蓝色 | 物体基础颜色 | 根据场景调整 |
| edgeColor | color | 橙色 | 边缘发光颜色 | 通常使用高亮色 |
| edgeIntensity | real | 3.0 | 边缘发光强度 | 1.0-5.0，影响发光亮度 |
| noiseTexture | Texture | - | 噪声纹理 | 使用灰度噪声图 |

### 参数效果

**dissolveAmount（溶解程度）：**
- 0.0：物体完全显示
- 0.3：开始出现溶解
- 0.5：溶解一半
- 0.7：大部分消失
- 1.0：完全消失

**edgeWidth（边缘宽度）：**
- 小值(0.02-0.05)：细边缘
- 中值(0.05-0.15)：适中边缘
- 大值(0.15-0.3)：宽边缘

**edgeIntensity（发光强度）：**
- 小值(1.0-2.0)：柔和发光
- 中值(2.0-4.0)：明显发光
- 大值(4.0-8.0)：强烈发光

## 噪声纹理

### 纹理要求

溶解效果需要一张噪声纹理，推荐特性：

- **格式**：灰度图或RGB（只使用R通道）
- **分辨率**：512x512 或 1024x1024
- **内容**：随机噪声图案
- **无缝**：最好是可平铺的无缝纹理

### 生成噪声纹理

可以使用以下工具生成：

1. **Photoshop/GIMP**：滤镜 → 渲染 → 云彩/噪声
2. **在线工具**：https://www.filterforge.com/
3. **程序生成**：Perlin噪声、Simplex噪声

### 示例噪声纹理代码（Python）

```python
import numpy as np
from PIL import Image

# 生成Perlin噪声
def generate_noise(width, height):
    noise = np.random.rand(height, width) * 255
    return noise.astype(np.uint8)

# 创建512x512噪声图
noise = generate_noise(512, 512)
img = Image.fromarray(noise, mode='L')
img.save('noise.png')
```

## 完整示例

### 文件树

```
dissolve-effect-example/
├── main.cpp
├── main.qml
├── shaders/
│   ├── dissolve.vert
│   ├── dissolve.frag
│   ├── dissolve_simple.vert
│   └── dissolve_simple.frag
├── textures/
│   └── noise.png
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
    title: "Dissolve Effect - 溶解效果示例"

    View3D {
        id: view3D
        anchors.fill: parent
        
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 8)
            lookAt: Qt.vector3d(0, 0, 0)
        }
        
        // 溶解球体
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2.5, 2.5, 2.5)
            
            materials: CustomMaterial {
                id: dissolveMaterial
                vertexShader: useSimpleShader.checked ? 
                    "qrc:/shaders/dissolve_simple.vert" : "qrc:/shaders/dissolve.vert"
                fragmentShader: useSimpleShader.checked ? 
                    "qrc:/shaders/dissolve_simple.frag" : "qrc:/shaders/dissolve.frag"
                
                property real dissolveAmount: 0.0
                property real edgeWidth: 0.1
                property color baseColor: "#3b82f6"
                property color edgeColor: "#fbbf24"
                property real edgeIntensity: 3.0
                
                property Texture noiseTexture: Texture {
                    source: "qrc:/textures/noise.png"
                    generateMipmaps: true
                    minFilter: Texture.Linear
                    magFilter: Texture.Linear
                }
                
                SequentialAnimation on dissolveAmount {
                    id: dissolveAnimation
                    loops: Animation.Infinite
                    running: autoAnimate.checked
                    
                    PropertyAnimation { to: 1.0; duration: 3000; easing.type: Easing.InOutQuad }
                    PauseAnimation { duration: 500 }
                    PropertyAnimation { to: 0.0; duration: 3000; easing.type: Easing.InOutQuad }
                    PauseAnimation { duration: 500 }
                }
            }
        }
        
        // 参考立方体
        Model {
            source: "#Cube"
            position: Qt.vector3d(0, -3, 0)
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            
            materials: PrincipledMaterial {
                baseColor: "#64748b"
                metalness: 0.5
                roughness: 0.3
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
                text: "溶解参数控制"
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
            
            // 自动动画
            CheckBox {
                id: autoAnimate
                text: "自动动画"
                checked: true
            }
            
            // 溶解程度控制
            Column {
                width: parent.width
                spacing: 5
                
                Text {
                    text: "溶解程度 (Dissolve): " + dissolveMaterial.dissolveAmount.toFixed(2)
                    font.pixelSize: 14
                }
                
                Slider {
                    width: parent.width
                    from: 0.0
                    to: 1.0
                    value: dissolveMaterial.dissolveAmount
                    enabled: !autoAnimate.checked
                    onValueChanged: {
                        if (!autoAnimate.checked) {
                            dissolveMaterial.dissolveAmount = value
                        }
                    }
                }
            }
            
            // 边缘宽度控制
            Column {
                width: parent.width
                spacing: 5
                
                Text {
                    text: "边缘宽度 (Edge Width): " + dissolveMaterial.edgeWidth.toFixed(2)
                    font.pixelSize: 14
                }
                
                Slider {
                    width: parent.width
                    from: 0.02
                    to: 0.3
                    value: 0.1
                    onValueChanged: dissolveMaterial.edgeWidth = value
                }
            }
            
            // 发光强度控制
            Column {
                width: parent.width
                spacing: 5
                
                Text {
                    text: "发光强度 (Intensity): " + dissolveMaterial.edgeIntensity.toFixed(1)
                    font.pixelSize: 14
                }
                
                Slider {
                    width: parent.width
                    from: 1.0
                    to: 8.0
                    value: 3.0
                    onValueChanged: dissolveMaterial.edgeIntensity = value
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
                    text: "火焰"
                    onClicked: {
                        dissolveMaterial.baseColor = "#dc2626"
                        dissolveMaterial.edgeColor = "#fbbf24"
                    }
                }
                
                Button {
                    text: "冰霜"
                    onClicked: {
                        dissolveMaterial.baseColor = "#0ea5e9"
                        dissolveMaterial.edgeColor = "#7dd3fc"
                    }
                }
                
                Button {
                    text: "能量"
                    onClicked: {
                        dissolveMaterial.baseColor = "#8b5cf6"
                        dissolveMaterial.edgeColor = "#22d3ee"
                    }
                }
            }
            
            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter
                
                Button {
                    text: "毒素"
                    onClicked: {
                        dissolveMaterial.baseColor = "#16a34a"
                        dissolveMaterial.edgeColor = "#84cc16"
                    }
                }
                
                Button {
                    text: "暗影"
                    onClicked: {
                        dissolveMaterial.baseColor = "#1e293b"
                        dissolveMaterial.edgeColor = "#a855f7"
                    }
                }
                
                Button {
                    text: "圣光"
                    onClicked: {
                        dissolveMaterial.baseColor = "#fef3c7"
                        dissolveMaterial.edgeColor = "#fbbf24"
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
        text: "使用滑块调整溶解参数\n取消自动动画可手动控制\n点击颜色预设快速切换主题"
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
project(DissolveEffectExample VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Quick3D)

qt_add_executable(DissolveEffectExample
    main.cpp
)

qt_add_qml_module(DissolveEffectExample
    URI DissolveEffectExample
    VERSION 1.0
    QML_FILES
        main.qml
    RESOURCES
        shaders/dissolve.vert
        shaders/dissolve.frag
        shaders/dissolve_simple.vert
        shaders/dissolve_simple.frag
        textures/noise.png
)

target_link_libraries(DissolveEffectExample PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Quick3D
)

# Windows平台设置
if(WIN32)
    set_target_properties(DissolveEffectExample PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
```

### 预期效果

运行程序后，你将看到：
- 一个球体持续进行溶解和重现动画
- 溶解边缘有明亮的发光效果
- 可以实时切换两种着色器实现
- 可以手动控制溶解进度
- 可以调整边缘宽度和发光强度
- 六个颜色预设快速切换不同主题

## 性能优化

### 优化建议

1. **纹理优化**
   - 使用压缩纹理格式（DXT/BC）
   - 合理的纹理分辨率（512x512通常足够）
   - 启用mipmap减少采样开销
   - 使用线性过滤而非各向异性过滤

2. **discard优化**
   - discard会禁用Early-Z优化
   - 尽量减少discard的使用范围
   - 考虑使用Alpha Blend代替discard（性能更好但效果不同）
   - 从后往前渲染透明物体

3. **边缘计算优化**
   - 使用smoothstep而非if-else分支
   - 预计算常量值
   - 避免复杂的数学运算

4. **纹理采样优化**
   - 只采样一次噪声纹理
   - 使用合适的纹理过滤模式
   - 考虑使用低精度纹理（R8而非RGBA8）

### 性能分析

- **顶点着色器开销**：极低（仅传递数据）
- **片段着色器开销**：中等（纹理采样 + discard）
- **纹理内存**：低（单张噪声纹理）
- **适用设备**：中高端设备
- **推荐面数**：1000-10000个三角形

### 性能对比

| 操作 | 性能影响 | 替代方案 |
|------|----------|----------|
| discard | 中等 | Alpha Blend |
| 纹理采样 | 低 | 程序化噪声（更慢） |
| smoothstep | 极低 | if-else（更慢） |
| 边缘发光 | 低 | 无 |

## 常见问题

### 问题1：溶解边缘有锯齿

**原因：** 噪声纹理分辨率不足或过滤模式不当

**解决方案：**
```qml
property Texture noiseTexture: Texture {
    source: "qrc:/textures/noise.png"
    generateMipmaps: true        // 生成mipmap
    minFilter: Texture.Linear    // 线性过滤
    magFilter: Texture.Linear
}
```

### 问题2：溶解效果不明显

**原因：** 噪声纹理对比度不足或dissolveAmount范围不对

**解决方案：**
```glsl
// 增强噪声对比度
float noise = texture(noiseTexture, texCoord).r;
noise = pow(noise, 0.5);  // 调整对比度

// 或者调整dissolveAmount范围
if (noise < dissolveAmount * 1.2 - 0.1) {
    discard;
}
```

### 问题3：边缘发光太弱或太强

**原因：** edgeIntensity或edgeWidth参数不合适

**解决方案：**
```qml
// 调整参数
property real edgeWidth: 0.08        // 减小边缘宽度
property real edgeIntensity: 5.0     // 增加发光强度

// 或在着色器中调整
vec3 glowColor = edgeColor.rgb * edgeIntensity * 2.0;
```

### 问题4：性能问题

**原因：** discard操作过多或纹理采样开销大

**解决方案：**
```glsl
// 方案1：提前退出
float noise = texture(noiseTexture, texCoord).r;
if (noise < dissolveAmount - edgeWidth) {
    discard;  // 提前丢弃，避免后续计算
}

// 方案2：使用Alpha Blend代替discard
float alpha = smoothstep(dissolveAmount - 0.1, dissolveAmount + 0.1, noise);
fragColor = vec4(litColor, alpha);
```

### 问题5：Shader Utilities版本纹理采样失败

**原因：** 纹理绑定或采样语法错误

**解决方案：**
```glsl
// 确保纹理正确声明
// 在QML中
property Texture noiseTexture: Texture { ... }

// 在着色器中直接使用
float noise = texture(noiseTexture, vTexCoord).r;
```

## 调试技巧

### 可视化调试

```glsl
// 在片段着色器中可视化不同的值

// 1. 可视化噪声纹理
FRAGCOLOR = vec4(vec3(noise), 1.0);

// 2. 可视化溶解阈值
float threshold = step(dissolveAmount, noise);
FRAGCOLOR = vec4(vec3(threshold), 1.0);

// 3. 可视化边缘区域
float edgeDistance = noise - dissolveAmount;
float inEdge = step(0.0, edgeDistance) * step(edgeDistance, edgeWidth);
FRAGCOLOR = vec4(vec3(inEdge), 1.0);

// 4. 可视化边缘因子
float edgeFactor = smoothstep(0.0, edgeWidth, edgeDistance);
FRAGCOLOR = vec4(vec3(edgeFactor), 1.0);
```

### 参数测试

```qml
// 测试极端值
property real dissolveAmount: 0.5   // 固定在中间
property real edgeWidth: 0.5        // 测试大边缘
property real edgeIntensity: 10.0   // 测试强发光
```

## 进阶技巧

### 多层溶解

```glsl
// 使用多个噪声层创建更复杂的溶解图案
float noise1 = texture(noiseTexture, texCoord).r;
float noise2 = texture(noiseTexture, texCoord * 2.0).g;
float combinedNoise = noise1 * 0.7 + noise2 * 0.3;

if (combinedNoise < dissolveAmount) {
    discard;
}
```

### 方向性溶解

```glsl
// 基于位置的溶解（从下往上）
float heightFactor = (vWorldPos.y + 2.0) / 4.0;
float adjustedThreshold = dissolveAmount + heightFactor * 0.3;

if (noise < adjustedThreshold) {
    discard;
}
```

### 动态边缘颜色

```glsl
// 边缘颜色随时间变化
float hue = fract(time * 0.1);
vec3 dynamicEdgeColor = hsv2rgb(vec3(hue, 1.0, 1.0));
vec3 glowColor = dynamicEdgeColor * edgeIntensity;
```

### 粒子溶解

```glsl
// 在溶解边缘生成粒子效果
if (edgeDistance < edgeWidth * 0.5) {
    // 添加随机扰动
    vec2 offset = vec2(
        sin(vWorldPos.x * 10.0 + time) * 0.1,
        cos(vWorldPos.y * 10.0 + time) * 0.1
    );
    // 使用扰动后的位置
}
```

### 双色边缘

```glsl
// 内外两层不同颜色的边缘
float outerEdge = smoothstep(0.0, edgeWidth * 0.5, edgeDistance);
float innerEdge = smoothstep(edgeWidth * 0.5, edgeWidth, edgeDistance);

vec3 outerColor = edgeColor.rgb * edgeIntensity;
vec3 innerColor = vec3(1.0, 1.0, 1.0) * edgeIntensity * 2.0;

vec3 edgeGlow = mix(outerColor, innerColor, outerEdge);
litColor = mix(edgeGlow, litColor, innerEdge);
```

## 延伸阅读

1. **Qt Quick 3D CustomMaterial 官方文档**  
   详细了解CustomMaterial的纹理绑定和采样  
   链接：https://doc.qt.io/qt-6/qml-qtquick3d-custommaterial.html

2. **Alpha Test vs Alpha Blend**  
   理解discard和透明度混合的区别  
   链接：https://learnopengl.com/Advanced-OpenGL/Blending

3. **Perlin Noise 和程序化纹理**  
   学习如何生成程序化噪声  
   链接：https://en.wikipedia.org/wiki/Perlin_noise

4. **GPU Gems - Chapter 16: Dissolve Effects**  
   深入了解溶解效果的高级技术  
   链接：https://developer.nvidia.com/gpugems/gpugems/contributors

5. **Shader Graph Dissolve Tutorial**  
   可视化理解溶解效果的实现  
   链接：https://www.youtube.com/results?search_query=shader+dissolve+effect

---

**下一步：** 尝试 [Hologram Effect](../02-Material-Effects/Hologram-Effect.md) 学习全息投影效果
