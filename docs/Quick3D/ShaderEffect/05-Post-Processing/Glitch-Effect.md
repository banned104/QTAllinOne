---
title: Glitch Effect - 故障效果
category: Post Processing
complexity: Medium
performance: Medium
tags:
- QtQuick3D
- Shader
- Glitch
- Digital
- 中文
---

# Glitch Effect - 故障效果

## 特效简介

故障效果模拟数字信号干扰、损坏和错误，创建赛博朋克风格的视觉效果。

**适用场景：** 信号干扰、损坏效果、赛博朋克、黑客主题、转场动画

**性能：** 中等开销

## 技术原理

```glsl
// 随机位移
float glitch = step(0.9, fract(sin(time * 10.0 + uv.y * 100.0) * 43758.5453));
vec2 offset = vec2(glitch * 0.1, 0.0);

// RGB通道分离
float r = texture(tex, uv + offset).r;
float g = texture(tex, uv).g;
float b = texture(tex, uv - offset).b;
```

## 着色器实现

### 原生GLSL

**顶点着色器 (glitch.vert):**
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
    
    float time;
    float glitchIntensity;
};

out vec3 worldPos;
out vec3 normal;
out vec2 texCoord;

void main() {
    texCoord = qt_TexCoord0;
    
    // 随机顶点位移
    float glitch = step(0.95, fract(sin(time * 10.0 + qt_Vertex.y * 100.0) * 43758.5453));
    vec3 offset = vec3(glitch * glitchIntensity * 0.1, 0.0, 0.0);
    
    vec3 glitchedVertex = qt_Vertex + offset;
    worldPos = (qt_ModelMatrix * vec4(glitchedVertex, 1.0)).xyz;
    normal = normalize((qt_NormalMatrix * vec4(qt_Normal, 0.0)).xyz);
    
    gl_Position = qt_Matrix * vec4(glitchedVertex, 1.0);
}
```

**片段着色器 (glitch.frag):**
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
    float glitchIntensity;
    vec4 baseColor;
    float colorSeparation;       // RGB分离强度
    float blockSize;             // 块状故障大小
};

in vec3 worldPos;
in vec3 normal;
in vec2 texCoord;

out vec4 fragColor;

// 随机函数
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main() {
    vec2 uv = texCoord;
    
    // 块状故障
    vec2 blockUV = floor(uv * blockSize) / blockSize;
    float blockGlitch = step(0.95, random(blockUV + time));
    
    // RGB通道分离
    vec2 rOffset = vec2(blockGlitch * colorSeparation, 0.0);
    vec2 bOffset = vec2(-blockGlitch * colorSeparation, 0.0);
    
    // 基础颜色（模拟纹理采样）
    vec3 color = baseColor.rgb;
    
    // 应用RGB分离
    float r = color.r;
    float g = color.g;
    float b = color.b;
    
    // 随机颜色故障
    if (blockGlitch > 0.5) {
        color = vec3(random(uv + time), random(uv + time + 0.1), random(uv + time + 0.2));
    }
    
    // 水平条纹故障
    float stripe = step(0.9, fract(uv.y * 50.0 + time * 10.0));
    color = mix(color, vec3(1.0), stripe * glitchIntensity * 0.5);
    
    // 简单光照
    vec3 N = normalize(normal);
    vec3 L = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(N, L), 0.0);
    color *= (0.3 + 0.7 * NdotL);
    
    fragColor = vec4(color, qt_Opacity);
}
```

### Shader Utilities

**片段着色器 (glitch_simple.frag):**
```glsl
VARYING vec2 vTexCoord;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void MAIN()
{
    vec2 uv = vTexCoord;
    
    // 块状故障
    vec2 blockUV = floor(uv * blockSize) / blockSize;
    float blockGlitch = step(0.95, random(blockUV + time));
    
    // 基础颜色
    vec3 color = baseColor.rgb;
    
    // 随机颜色故障
    if (blockGlitch > 0.5) {
        color = vec3(random(uv + time), random(uv + time + 0.1), random(uv + time + 0.2));
    }
    
    // 水平条纹
    float stripe = step(0.9, fract(uv.y * 50.0 + time * 10.0));
    color = mix(color, vec3(1.0), stripe * glitchIntensity * 0.5);
    
    FRAGCOLOR = vec4(color, 1.0);
}
```

## QML集成

```qml
Model {
    source: "#Cube"
    materials: CustomMaterial {
        vertexShader: "qrc:/shaders/glitch.vert"
        fragmentShader: "qrc:/shaders/glitch.frag"
        
        property real time: 0.0
        property real glitchIntensity: 1.0
        property color baseColor: "#00ffff"
        property real colorSeparation: 0.02
        property real blockSize: 20.0
        
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
| glitchIntensity | real | 1.0 | 故障强度 |
| colorSeparation | real | 0.02 | RGB分离强度 |
| blockSize | real | 20.0 | 块状故障大小 |

## 进阶技巧

### 数字雨故障
```glsl
// 垂直数字流
float digitalRain = fract(uv.y * 10.0 - time * 2.0);
digitalRain = step(0.9, digitalRain);
color += vec3(digitalRain * 0.5);
```

### 扫描线故障
```glsl
// 移动的扫描线
float scanline = abs(sin((uv.y - time * 0.5) * 50.0));
scanline = smoothstep(0.9, 1.0, scanline);
color += vec3(scanline * 0.3);
```

### 像素化故障
```glsl
// 像素化效果
vec2 pixelUV = floor(uv * pixelSize) / pixelSize;
color = texture(tex, pixelUV).rgb;
```

### 时间控制的故障
```glsl
// 间歇性故障
float glitchTrigger = step(0.95, fract(sin(floor(time * 2.0)) * 43758.5453));
glitchIntensity *= glitchTrigger;
```

## 实际应用

### 转场效果
```qml
SequentialAnimation {
    PropertyAnimation {
        target: glitchMaterial
        property: "glitchIntensity"
        from: 0.0; to: 2.0
        duration: 500
    }
    // 切换场景
    PropertyAnimation {
        target: glitchMaterial
        property: "glitchIntensity"
        from: 2.0; to: 0.0
        duration: 500
    }
}
```

### 损坏状态
```qml
// 根据生命值显示故障
property real health: 100.0
property real glitchIntensity: (100.0 - health) / 100.0
```

## 性能优化

- **随机函数**：使用简单的伪随机
- **条件分支**：最小化if语句
- **计算频率**：降低blockSize减少计算

## 延伸阅读

- 数字信号处理
- 赛博朋克美学
- 随机数生成算法

---

**完成！** 返回 [README](../README.md) 查看所有特效
