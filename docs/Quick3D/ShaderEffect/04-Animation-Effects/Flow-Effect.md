---
title: Flow Effect - 流动效果
category: Animation Effects
complexity: Simple
performance: Low
tags:
- QtQuick3D
- Shader
- UV Animation
- 中文
---

# Flow Effect - 流动效果

## 特效简介

流动效果通过UV坐标动画创建纹理滚动效果，模拟能量流、河流、传送带等流动现象。

**适用场景：** 能量流、河流、传送带、激光、数据流

**性能：** 极低开销

## 技术原理

```glsl
// UV偏移
vec2 flowUV = texCoord + vec2(time * flowSpeed, 0.0);
vec4 color = texture(flowTexture, flowUV);
```

## 着色器实现

### 原生GLSL

**顶点着色器 (flow.vert):**
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

out vec3 normal;
out vec2 texCoord;

void main() {
    texCoord = qt_TexCoord0;
    normal = normalize((qt_NormalMatrix * vec4(qt_Normal, 0.0)).xyz);
    gl_Position = qt_Matrix * vec4(qt_Vertex, 1.0);
}
```

**片段着色器 (flow.frag):**
```glsl
#version 440
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    mat4 qt_ModelMatrix;
    mat4 qt_ViewMatrix;
    mat4 qt_ProjectionMatrix;
    mat4 qt_NormalMatrix;
    float qt_Opacity;
    
    float time;
    vec2 flowDirection;      // 流动方向
    float flowSpeed;         // 流动速度
    vec4 tintColor;          // 着色
};

layout(binding = 1) uniform sampler2D flowTexture;

in vec3 normal;
in vec2 texCoord;
out vec4 fragColor;

void main() {
    // UV动画
    vec2 flowUV = texCoord + flowDirection * time * flowSpeed;
    
    // 采样纹理
    vec4 texColor = texture(flowTexture, flowUV);
    
    // 应用着色
    vec3 finalColor = texColor.rgb * tintColor.rgb;
    
    fragColor = vec4(finalColor, texColor.a * qt_Opacity);
}
```

### Shader Utilities

**片段着色器 (flow_simple.frag):**
```glsl
VARYING vec2 vTexCoord;

void MAIN()
{
    vec2 flowUV = vTexCoord + flowDirection * time * flowSpeed;
    vec4 texColor = texture(flowTexture, flowUV);
    FRAGCOLOR = vec4(texColor.rgb * tintColor.rgb, texColor.a);
}
```

## QML集成

```qml
Model {
    source: "#Rectangle"
    materials: CustomMaterial {
        vertexShader: "qrc:/shaders/flow.vert"
        fragmentShader: "qrc:/shaders/flow.frag"
        
        property real time: 0.0
        property vector2d flowDirection: Qt.vector2d(1.0, 0.0)
        property real flowSpeed: 0.5
        property color tintColor: "#00ffff"
        property Texture flowTexture: Texture {
            source: "qrc:/textures/flow_pattern.png"
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
        }
        
        NumberAnimation on time {
            from: 0; to: 100; duration: 50000
            loops: Animation.Infinite
        }
    }
}
```

## 参数说明

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| time | real | 0.0 | 时间参数 |
| flowDirection | vector2d | (1,0) | 流动方向 |
| flowSpeed | real | 0.5 | 流动速度 |
| tintColor | color | 青色 | 着色颜色 |

## 进阶技巧

### 双层流动
```glsl
vec2 uv1 = texCoord + flowDirection * time * flowSpeed;
vec2 uv2 = texCoord + flowDirection * time * flowSpeed * 0.5;
vec4 layer1 = texture(flowTexture, uv1);
vec4 layer2 = texture(flowTexture, uv2);
vec4 combined = mix(layer1, layer2, 0.5);
```

### 扭曲流动
```glsl
float distortion = sin(texCoord.y * 10.0 + time) * 0.1;
vec2 flowUV = texCoord + vec2(time * flowSpeed + distortion, 0.0);
```

## 延伸阅读

- UV动画技术
- 纹理平铺和重复

---

**下一步：** [Pulse Effect](Pulse-Effect.md)
