---
title: Pulse Effect - 脉冲效果
category: Animation Effects
complexity: Simple
performance: Low
tags:
- QtQuick3D
- Shader
- Animation
- 中文
---

# Pulse Effect - 脉冲效果

## 特效简介

脉冲效果通过周期性的缩放和发光创建心跳、呼吸、警告等动画效果。

**适用场景：** 心跳、呼吸、警告、能量波动、UI反馈

**性能：** 极低开销

## 技术原理

```glsl
// 脉冲因子
float pulse = sin(time * pulseSpeed) * 0.5 + 0.5;
pulse = mix(minIntensity, maxIntensity, pulse);

// 应用到颜色
vec3 color = baseColor.rgb * pulse;
```

## 着色器实现

### 原生GLSL

**顶点着色器 (pulse.vert):**
```glsl
#version 440
layout(location = 0) in vec3 qt_Vertex;
layout(location = 1) in vec3 qt_Normal;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    mat4 qt_ModelMatrix;
    mat4 qt_ViewMatrix;
    mat4 qt_ProjectionMatrix;
    mat4 qt_NormalMatrix;
    float qt_Opacity;
    
    float time;
    float pulseSpeed;
    float pulseAmount;       // 缩放幅度
};

out vec3 normal;

void main() {
    // 脉冲缩放
    float pulse = sin(time * pulseSpeed) * 0.5 + 0.5;
    float scale = 1.0 + pulse * pulseAmount;
    
    vec3 scaledVertex = qt_Vertex * scale;
    normal = normalize((qt_NormalMatrix * vec4(qt_Normal, 0.0)).xyz);
    gl_Position = qt_Matrix * vec4(scaledVertex, 1.0);
}
```

**片段着色器 (pulse.frag):**
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
    float pulseSpeed;
    float pulseAmount;
    vec4 baseColor;
    vec4 pulseColor;
    float minIntensity;
    float maxIntensity;
};

in vec3 normal;
out vec4 fragColor;

void main() {
    // 脉冲强度
    float pulse = sin(time * pulseSpeed) * 0.5 + 0.5;
    pulse = mix(minIntensity, maxIntensity, pulse);
    
    // 混合颜色
    vec3 color = mix(baseColor.rgb, pulseColor.rgb, pulse * 0.5);
    color *= pulse;
    
    // 简单光照
    vec3 N = normalize(normal);
    vec3 L = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(N, L), 0.0);
    color *= (0.3 + 0.7 * NdotL);
    
    fragColor = vec4(color, qt_Opacity);
}
```

### Shader Utilities

**顶点着色器 (pulse_simple.vert):**
```glsl
void MAIN()
{
    float pulse = sin(time * pulseSpeed) * 0.5 + 0.5;
    float scale = 1.0 + pulse * pulseAmount;
    VERTEX = VERTEX * scale;
}
```

**片段着色器 (pulse_simple.frag):**
```glsl
VARYING vec3 vNormal;

void MAIN()
{
    float pulse = sin(time * pulseSpeed) * 0.5 + 0.5;
    pulse = mix(minIntensity, maxIntensity, pulse);
    
    vec3 color = mix(baseColor.rgb, pulseColor.rgb, pulse * 0.5);
    color *= pulse;
    
    FRAGCOLOR = vec4(color, 1.0);
}
```

## QML集成

```qml
Model {
    source: "#Sphere"
    materials: CustomMaterial {
        vertexShader: "qrc:/shaders/pulse.vert"
        fragmentShader: "qrc:/shaders/pulse.frag"
        
        property real time: 0.0
        property real pulseSpeed: 3.0
        property real pulseAmount: 0.2
        property color baseColor: "#dc2626"
        property color pulseColor: "#fbbf24"
        property real minIntensity: 0.5
        property real maxIntensity: 1.5
        
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
| pulseSpeed | real | 3.0 | 脉冲速度 |
| pulseAmount | real | 0.2 | 缩放幅度 |
| minIntensity | real | 0.5 | 最小亮度 |
| maxIntensity | real | 1.5 | 最大亮度 |

## 进阶技巧

### 心跳效果
```glsl
// 双峰脉冲（模拟心跳）
float t = time * pulseSpeed;
float pulse1 = exp(-mod(t, 1.0) * 5.0);
float pulse2 = exp(-mod(t + 0.3, 1.0) * 5.0);
float heartbeat = pulse1 + pulse2 * 0.5;
```

### 呼吸效果
```glsl
// 平滑的呼吸曲线
float breath = sin(time * pulseSpeed) * 0.5 + 0.5;
breath = smoothstep(0.0, 1.0, breath);
breath = smoothstep(0.0, 1.0, breath);  // 双重smoothstep更平滑
```

### 警告闪烁
```glsl
// 快速闪烁
float warning = step(0.5, fract(time * pulseSpeed * 2.0));
```

## 延伸阅读

- 周期函数动画
- Easing函数

---

**下一步：** [Scanline Effect](../05-Post-Processing/Scanline-Effect.md)
