---
title: Scanline Effect - 扫描线效果
category: Post Processing
complexity: Simple
performance: Low
tags:
- QtQuick3D
- Shader
- Retro
- CRT
- 中文
---

# Scanline Effect - 扫描线效果

## 特效简介

扫描线效果模拟CRT显示器的水平扫描线，创建复古科幻的视觉风格。

**适用场景：** CRT显示器、复古游戏、科幻UI、黑客风格

**性能：** 极低开销

## 技术原理

```glsl
// 基于屏幕Y坐标的扫描线
float scanline = sin(screenPos.y * frequency) * 0.5 + 0.5;
scanline = mix(1.0 - intensity, 1.0, scanline);
color *= scanline;
```

## 着色器实现

### 原生GLSL

**顶点着色器 (scanline.vert):**
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
out vec2 texCoord;
out vec4 screenPos;

void main() {
    texCoord = qt_TexCoord0;
    worldPos = (qt_ModelMatrix * vec4(qt_Vertex, 1.0)).xyz;
    normal = normalize((qt_NormalMatrix * vec4(qt_Normal, 0.0)).xyz);
    
    // 屏幕空间位置
    screenPos = qt_Matrix * vec4(qt_Vertex, 1.0);
    
    gl_Position = screenPos;
}
```

**片段着色器 (scanline.frag):**
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
    vec4 baseColor;
    float scanlineFrequency;     // 扫描线密度
    float scanlineIntensity;     // 扫描线强度
    float scanlineSpeed;         // 扫描线移动速度
};

in vec3 worldPos;
in vec3 normal;
in vec2 texCoord;
in vec4 screenPos;

out vec4 fragColor;

void main() {
    // 标准化屏幕坐标
    vec2 screenUV = (screenPos.xy / screenPos.w) * 0.5 + 0.5;
    
    // 扫描线计算
    float scanline = sin((screenUV.y + time * scanlineSpeed) * scanlineFrequency * 3.14159);
    scanline = scanline * 0.5 + 0.5;
    scanline = mix(1.0 - scanlineIntensity, 1.0, scanline);
    
    // 基础光照
    vec3 N = normalize(normal);
    vec3 L = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(N, L), 0.0);
    vec3 color = baseColor.rgb * (0.3 + 0.7 * NdotL);
    
    // 应用扫描线
    color *= scanline;
    
    fragColor = vec4(color, qt_Opacity);
}
```

### Shader Utilities

**片段着色器 (scanline_simple.frag):**
```glsl
VARYING vec2 vTexCoord;
VARYING vec4 vScreenPos;

void MAIN()
{
    vec2 screenUV = (vScreenPos.xy / vScreenPos.w) * 0.5 + 0.5;
    
    float scanline = sin((screenUV.y + time * scanlineSpeed) * scanlineFrequency * 3.14159);
    scanline = scanline * 0.5 + 0.5;
    scanline = mix(1.0 - scanlineIntensity, 1.0, scanline);
    
    vec3 color = baseColor.rgb * scanline;
    FRAGCOLOR = vec4(color, 1.0);
}
```

## QML集成

```qml
Model {
    source: "#Rectangle"
    materials: CustomMaterial {
        vertexShader: "qrc:/shaders/scanline.vert"
        fragmentShader: "qrc:/shaders/scanline.frag"
        
        property real time: 0.0
        property color baseColor: "#00ff00"
        property real scanlineFrequency: 100.0
        property real scanlineIntensity: 0.3
        property real scanlineSpeed: 0.5
        
        NumberAnimation on time {
            from: 0; to: 1; duration: 2000
            loops: Animation.Infinite
        }
    }
}
```

## 参数说明

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| scanlineFrequency | real | 100.0 | 扫描线密度 |
| scanlineIntensity | real | 0.3 | 扫描线强度 |
| scanlineSpeed | real | 0.5 | 移动速度 |

## 进阶技巧

### CRT曲面效果
```glsl
// 模拟CRT屏幕曲面
vec2 curve(vec2 uv) {
    uv = uv * 2.0 - 1.0;
    vec2 offset = abs(uv.yx) / vec2(6.0, 4.0);
    uv = uv + uv * offset * offset;
    uv = uv * 0.5 + 0.5;
    return uv;
}
```

### 色差效果
```glsl
// RGB通道分离
float r = texture(tex, uv + vec2(0.001, 0.0)).r;
float g = texture(tex, uv).g;
float b = texture(tex, uv - vec2(0.001, 0.0)).b;
vec3 color = vec3(r, g, b);
```

### 闪烁效果
```glsl
// 随机闪烁
float flicker = fract(sin(time * 10.0) * 43758.5453);
flicker = step(0.95, flicker);
color *= mix(1.0, 0.8, flicker);
```

## 延伸阅读

- CRT显示器原理
- 复古游戏美学

---

**下一步：** [Glitch Effect](Glitch-Effect.md)
