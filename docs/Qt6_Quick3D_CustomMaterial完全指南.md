# Qt6 Quick3D CustomMaterial 完全指南

## 目录
- [概述](#概述)
- [CustomMaterial 基础](#custommaterial-基础)
- [Shaded 模式详解](#shaded-模式详解)
- [PBR 渲染](#pbr-渲染)
- [IBL 基于图像的光照](#ibl-基于图像的光照)
- [高级着色技术](#高级着色技术)
- [性能优化](#性能优化)
- [完整实例](#完整实例)

---

## 概述

### 什么是 CustomMaterial

CustomMaterial 是 Qt Quick 3D 中最强大的材质系统，允许开发者通过自定义着色器代码实现任意复杂的渲染效果。与 PrincipledMaterial 和 DefaultMaterial 不同，CustomMaterial 提供了完全的着色器控制权。

### CustomMaterial 的三种模式

1. **Unshaded（无光照）**：完全自定义，不使用 Qt 的光照系统
2. **Shaded（有光照）**：集成 Qt 的光照系统，支持 PBR 工作流
3. **Transparent（透明）**：支持透明度混合的着色模式

### 核心优势

- 完全控制渲染管线
- 支持自定义 PBR 工作流
- 可访问 Qt 内置的光照和 IBL 系统
- 支持多 Pass 渲染
- 可实现高级效果（SSS、各向异性、清漆层等）

---

## CustomMaterial 基础

### 基本结构

```qml
import QtQuick
import QtQuick3D

CustomMaterial {
    // 材质属性
    property real metalness: 0.5
    property real roughness: 0.3
    property color baseColor: "white"
    
    // 纹理
    property TextureInput albedoMap: TextureInput {
        texture: Texture { source: "textures/albedo.png" }
    }
    
    // 着色器模式
    shadingMode: CustomMaterial.Shaded
    
    // 顶点着色器
    vertexShader: "shaders/custom.vert"
    
    // 片段着色器
    fragmentShader: "shaders/custom.frag"
}
```

### 着色器文件结构

**顶点着色器 (custom.vert)**
```glsl
VARYING vec3 vNormal;
VARYING vec3 vViewDir;
VARYING vec2 vTexCoord;

void MAIN()
{
    vNormal = NORMAL;
    vTexCoord = UV0;
    
    vec3 worldPos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
    vViewDir = CAMERA_POSITION - worldPos;
    
    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
}
```

**片段着色器 (custom.frag)**
```glsl
VARYING vec3 vNormal;
VARYING vec3 vViewDir;
VARYING vec2 vTexCoord;

void MAIN()
{
    vec3 normal = normalize(vNormal);
    vec3 viewDir = normalize(vViewDir);
    
    // 自定义着色逻辑
    BASE_COLOR = vec4(1.0, 0.0, 0.0, 1.0);
}
```

---

## Shaded 模式详解

### Shaded 模式的特点

Shaded 模式是 CustomMaterial 最常用的模式，它：
- 自动集成 Qt 的光照系统
- 支持多光源（方向光、点光源、聚光灯）
- 提供 PBR 材质输入接口
- 支持阴影接收和投射
- 集成 IBL 环境光照

### 内置变量和函数

#### 顶点着色器变量

```glsl
// 输入属性
VERTEX          // vec3: 顶点位置
NORMAL          // vec3: 法线
TANGENT         // vec3: 切线
BINORMAL        // vec3: 副法线
UV0, UV1        // vec2: 纹理坐标
COLOR           // vec4: 顶点颜色
JOINTS          // ivec4: 骨骼索引
WEIGHTS         // vec4: 骨骼权重

// 矩阵
MODEL_MATRIX                // mat4: 模型矩阵
VIEW_MATRIX                 // mat4: 视图矩阵
PROJECTION_MATRIX           // mat4: 投影矩阵
MODELVIEW_MATRIX            // mat4: 模型视图矩阵
MODELVIEWPROJECTION_MATRIX  // mat4: MVP 矩阵
NORMAL_MATRIX               // mat3: 法线矩阵

// 相机
CAMERA_POSITION             // vec3: 相机世界坐标
CAMERA_DIRECTION            // vec3: 相机方向

// 输出
POSITION                    // vec4: 裁剪空间位置（必须设置）
```

#### 片段着色器变量（Shaded 模式）

```glsl
// PBR 材质输出（必须设置）
BASE_COLOR                  // vec4: 基础颜色（albedo）
METALNESS                   // float: 金属度 [0,1]
ROUGHNESS                   // float: 粗糙度 [0,1]
SPECULAR_AMOUNT             // float: 镜面反射强度
SPECULAR_TINT               // vec3: 镜面反射色调

// 可选输出
EMISSIVE_COLOR              // vec3: 自发光颜色
NORMAL                      // vec3: 法线（用于法线贴图）
OPACITY                     // float: 不透明度
OCCLUSION                   // float: 环境光遮蔽

// 光照计算结果（只读）
DIFFUSE                     // vec3: 漫反射光照
SPECULAR                    // vec3: 镜面反射光照
IBL_DIFFUSE                 // vec3: IBL 漫反射
IBL_SPECULAR                // vec3: IBL 镜面反射

// 几何信息
VAR_WORLD_POSITION          // vec3: 世界空间位置
VAR_WORLD_NORMAL            // vec3: 世界空间法线
```

### Shaded 模式工作流程


```
1. 顶点着色器处理几何变换
2. 片段着色器设置 PBR 材质参数
3. Qt 自动执行光照计算
4. 可选：在 MAIN 后访问光照结果进行后处理
```

### 基础 Shaded 材质示例

**QML 定义**
```qml
CustomMaterial {
    id: shadedMaterial
    shadingMode: CustomMaterial.Shaded
    
    property real metalness: 0.0
    property real roughness: 0.5
    property color albedo: "#ffffff"
    
    property TextureInput albedoTexture: TextureInput {
        texture: Texture { source: "textures/albedo.png" }
    }
    
    property TextureInput normalTexture: TextureInput {
        texture: Texture { source: "textures/normal.png" }
    }
    
    property TextureInput roughnessTexture: TextureInput {
        texture: Texture { source: "textures/roughness.png" }
    }
    
    vertexShader: "shaders/pbr.vert"
    fragmentShader: "shaders/pbr.frag"
}
```

**顶点着色器 (pbr.vert)**
```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vTangent;
VARYING vec3 vBinormal;
VARYING vec2 vTexCoord;

void MAIN()
{
    vTexCoord = UV0;
    
    // 转换到世界空间
    vWorldPos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
    vNormal = normalize(NORMAL_MATRIX * NORMAL);
    vTangent = normalize(NORMAL_MATRIX * TANGENT);
    vBinormal = normalize(NORMAL_MATRIX * BINORMAL);
    
    // 输出裁剪空间位置
    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
}
```

**片段着色器 (pbr.frag)**
```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vTangent;
VARYING vec3 vBinormal;
VARYING vec2 vTexCoord;

void MAIN()
{
    // 采样纹理
    vec4 albedoSample = texture(albedoTexture, vTexCoord);
    vec3 normalSample = texture(normalTexture, vTexCoord).xyz * 2.0 - 1.0;
    float roughnessSample = texture(roughnessTexture, vTexCoord).r;
    
    // 构建 TBN 矩阵
    mat3 TBN = mat3(vTangent, vBinormal, vNormal);
    vec3 worldNormal = normalize(TBN * normalSample);
    
    // 设置 PBR 参数
    BASE_COLOR = vec4(albedoSample.rgb * albedo.rgb, 1.0);
    METALNESS = metalness;
    ROUGHNESS = roughness * roughnessSample;
    NORMAL = worldNormal;
    
    // Qt 会自动执行光照计算
}
```

---

## PBR 渲染

### PBR 理论基础

物理基础渲染（Physically Based Rendering）基于以下核心原则：

1. **能量守恒**：反射光不能超过入射光
2. **微表面理论**：表面由微小镜面组成
3. **菲涅尔效应**：掠射角反射更强

### PBR 材质参数

#### 基础颜色（Base Color / Albedo）
```glsl
// 金属：反射颜色
// 非金属：漫反射颜色
BASE_COLOR = vec4(albedoColor, 1.0);
```

#### 金属度（Metalness）
```glsl
// 0.0 = 非金属（电介质）
// 1.0 = 金属（导体）
METALNESS = metallicValue;
```

#### 粗糙度（Roughness）
```glsl
// 0.0 = 完全光滑（镜面）
// 1.0 = 完全粗糙（漫反射）
ROUGHNESS = roughnessValue;
```

### 完整 PBR 实现

**高级 PBR 片段着色器**
```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vTangent;
VARYING vec3 vBinormal;
VARYING vec2 vTexCoord;

// 常量
const float PI = 3.14159265359;
const vec3 F0_DIELECTRIC = vec3(0.04); // 非金属的基础反射率

// 法线分布函数 (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / denom;
}

// 几何遮蔽函数 (Smith's Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// 菲涅尔方程 (Schlick 近似)
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 带粗糙度的菲涅尔
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void MAIN()
{
    // 采样纹理
    vec4 albedoSample = texture(albedoTexture, vTexCoord);
    vec3 normalMap = texture(normalTexture, vTexCoord).xyz * 2.0 - 1.0;
    float metallic = texture(metallicTexture, vTexCoord).r * metalness;
    float roughness = texture(roughnessTexture, vTexCoord).r * roughnessValue;
    float ao = texture(aoTexture, vTexCoord).r;
    
    // 构建 TBN 矩阵并转换法线
    mat3 TBN = mat3(normalize(vTangent), normalize(vBinormal), normalize(vNormal));
    vec3 N = normalize(TBN * normalMap);
    vec3 V = normalize(CAMERA_POSITION - vWorldPos);
    
    // 计算基础反射率
    vec3 albedo = pow(albedoSample.rgb, vec3(2.2)); // sRGB to linear
    vec3 F0 = mix(F0_DIELECTRIC, albedo, metallic);
    
    // 设置 PBR 参数（Qt 会使用这些进行光照计算）
    BASE_COLOR = vec4(albedo, 1.0);
    METALNESS = metallic;
    ROUGHNESS = roughness;
    NORMAL = N;
    OCCLUSION = ao;
    
    // 可选：自定义光照计算
    // 在 Shaded 模式下，Qt 会自动计算 DIFFUSE 和 SPECULAR
    // 但你可以在此基础上添加额外效果
}
```


### PBR 纹理工作流

#### 金属/粗糙度工作流
```qml
CustomMaterial {
    property TextureInput albedoMap: TextureInput {
        texture: Texture { source: "pbr/albedo.png" }
    }
    property TextureInput metallicMap: TextureInput {
        texture: Texture { source: "pbr/metallic.png" }
    }
    property TextureInput roughnessMap: TextureInput {
        texture: Texture { source: "pbr/roughness.png" }
    }
    property TextureInput normalMap: TextureInput {
        texture: Texture { source: "pbr/normal.png" }
    }
    property TextureInput aoMap: TextureInput {
        texture: Texture { source: "pbr/ao.png" }
    }
}
```

#### 高度图和视差映射
```glsl
// 视差遮蔽映射 (Parallax Occlusion Mapping)
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    const float heightScale = 0.1;
    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    
    vec2 P = viewDir.xy * heightScale;
    vec2 deltaTexCoords = P / numLayers;
    
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(heightMap, currentTexCoords).r;
    
    while(currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(heightMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }
    
    // 插值以获得更精确的结果
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(heightMap, prevTexCoords).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);
    
    return prevTexCoords * weight + currentTexCoords * (1.0 - weight);
}

void MAIN()
{
    vec3 viewDir = normalize(CAMERA_POSITION - vWorldPos);
    vec3 viewDirTangent = normalize(transpose(TBN) * viewDir);
    
    // 应用视差映射
    vec2 texCoords = ParallaxMapping(vTexCoord, viewDirTangent);
    
    // 使用调整后的纹理坐标采样
    vec4 albedo = texture(albedoMap, texCoords);
    // ...
}
```

---

## IBL 基于图像的光照

### IBL 理论

基于图像的光照（Image-Based Lighting）使用环境贴图提供真实的环境光照和反射。

#### IBL 组成部分

1. **漫反射辐照度图（Diffuse Irradiance）**：预卷积的环境光
2. **镜面反射预滤波图（Specular Prefiltered）**：不同粗糙度的反射
3. **BRDF 查找表（BRDF LUT）**：菲涅尔和几何项的预计算

### Qt Quick 3D 中的 IBL 设置

#### 场景环境配置
```qml
View3D {
    environment: SceneEnvironment {
        backgroundMode: SceneEnvironment.SkyBox
        lightProbe: Texture {
            source: "maps/environment.hdr"
            mappingMode: Texture.LightProbe
        }
        probeExposure: 1.0
        probeHorizon: 0.0
        probeOrientation: Qt.vector3d(0, 0, 0)
    }
}
```

#### CustomMaterial 中访问 IBL

在 Shaded 模式下，Qt 自动提供 IBL 计算结果：

```glsl
void MAIN()
{
    // 设置 PBR 参数
    BASE_COLOR = vec4(albedo, 1.0);
    METALNESS = metallic;
    ROUGHNESS = roughness;
    NORMAL = normal;
    
    // Qt 自动计算 IBL 并填充以下变量：
    // IBL_DIFFUSE  - 环境漫反射
    // IBL_SPECULAR - 环境镜面反射
    
    // 可以在光照计算后访问和修改
}

// 在 MAIN 之后可以访问光照结果
void POST_PROCESS()
{
    // 访问最终光照
    vec3 ambient = IBL_DIFFUSE + IBL_SPECULAR;
    vec3 direct = DIFFUSE + SPECULAR;
    
    // 自定义混合
    FRAGCOLOR = vec4(ambient + direct, 1.0);
}
```

### 自定义 IBL 实现

如果需要完全控制 IBL 计算：

```glsl
// 采样环境贴图
vec3 SampleEnvironment(vec3 direction, float lod)
{
    // 使用 TextureCube 采样
    return textureLod(environmentMap, direction, lod).rgb;
}

// 计算 IBL 漫反射
vec3 CalculateIBLDiffuse(vec3 N, vec3 albedo, float metallic)
{
    vec3 irradiance = SampleEnvironment(N, 5.0); // 高 LOD = 模糊
    vec3 kD = mix(vec3(1.0), vec3(0.0), metallic);
    return kD * albedo * irradiance;
}

// 计算 IBL 镜面反射
vec3 CalculateIBLSpecular(vec3 N, vec3 V, vec3 F0, float roughness)
{
    vec3 R = reflect(-V, N);
    float lod = roughness * 5.0; // 根据粗糙度选择 mipmap
    
    vec3 prefilteredColor = SampleEnvironment(R, lod);
    vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    
    return prefilteredColor * (F0 * envBRDF.x + envBRDF.y);
}

void MAIN()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(CAMERA_POSITION - vWorldPos);
    
    // 计算 F0
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    
    // 计算 IBL
    vec3 iblDiffuse = CalculateIBLDiffuse(N, albedo, metallic);
    vec3 iblSpecular = CalculateIBLSpecular(N, V, F0, roughness);
    
    // 设置材质参数
    BASE_COLOR = vec4(albedo, 1.0);
    METALNESS = metallic;
    ROUGHNESS = roughness;
    
    // 添加自定义 IBL（如果不想使用 Qt 的自动 IBL）
    EMISSIVE_COLOR = iblDiffuse + iblSpecular;
}
```

### 动态环境反射

```qml
// 使用渲染目标创建动态环境贴图
Node {
    id: reflectionProbe
    
    // 6 个方向的相机
    Repeater {
        model: 6
        Node {
            PerspectiveCamera {
                id: cubeCamera
                fieldOfView: 90
                clipNear: 0.1
                clipFar: 100
            }
        }
    }
}

// 渲染到立方体贴图
Texture {
    id: dynamicEnvMap
    mappingMode: Texture.LightProbe
    // 每帧更新 6 个面
}
```

---

## 高级着色技术

### 1. 次表面散射（Subsurface Scattering, SSS）

模拟光线在半透明材质内部的散射效果（皮肤、蜡烛、玉石等）。

```glsl
// 简化的 SSS 实现
vec3 SubsurfaceScattering(vec3 L, vec3 V, vec3 N, vec3 thickness)
{
    // 透射光计算
    vec3 H = normalize(L + N * sssDistortion);
    float VdotH = pow(clamp(dot(V, -H), 0.0, 1.0), sssPower);
    
    // 基于厚度的衰减
    vec3 sssColor = sssColor * VdotH * thickness;
    
    return sssColor * sssScale;
}

void MAIN()
{
    // 采样厚度图
    float thickness = texture(thicknessMap, vTexCoord).r;
    
    // 标准 PBR 设置
    BASE_COLOR = vec4(albedo, 1.0);
    METALNESS = 0.0; // SSS 材质通常是非金属
    ROUGHNESS = roughness;
    NORMAL = normal;
    
    // 计算 SSS（需要在光照计算后）
    vec3 L = normalize(lightDirection);
    vec3 V = normalize(CAMERA_POSITION - vWorldPos);
    vec3 N = normalize(normal);
    
    vec3 sss = SubsurfaceScattering(L, V, N, vec3(thickness));
    
    // 添加到自发光
    EMISSIVE_COLOR = sss;
}
```

### 2. 各向异性反射（Anisotropic Reflection）

模拟拉丝金属、毛发等具有方向性的反射。

```glsl
// 各向异性 GGX 分布
float DistributionGGXAnisotropic(vec3 N, vec3 H, vec3 T, vec3 B, float roughnessX, float roughnessY)
{
    float NdotH = dot(N, H);
    if (NdotH <= 0.0) return 0.0;
    
    float TdotH = dot(T, H);
    float BdotH = dot(B, H);
    
    float a2x = roughnessX * roughnessX;
    float a2y = roughnessY * roughnessY;
    
    float denom = (TdotH * TdotH) / a2x + (BdotH * BdotH) / a2y + NdotH * NdotH;
    
    return 1.0 / (PI * a2x * a2y * denom * denom);
}

void MAIN()
{
    // 各向异性方向（通常沿切线）
    vec3 T = normalize(vTangent);
    vec3 B = normalize(vBinormal);
    vec3 N = normalize(vNormal);
    
    // 各向异性粗糙度
    float roughnessX = roughness * anisotropy;
    float roughnessY = roughness;
    
    BASE_COLOR = vec4(albedo, 1.0);
    METALNESS = metallic;
    ROUGHNESS = roughness;
    NORMAL = N;
    
    // 各向异性效果需要自定义光照计算
    // 或通过修改 ROUGHNESS 和 NORMAL 来近似
}
```


### 3. 清漆层（Clear Coat）

模拟汽车漆面、碳纤维等具有透明涂层的材质。

```glsl
// 清漆层 BRDF
vec3 ClearCoatBRDF(vec3 N, vec3 V, vec3 L, float clearCoat, float clearCoatRoughness)
{
    vec3 H = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);
    
    // 清漆层使用固定的 F0 (0.04)
    vec3 F = FresnelSchlick(VdotH, vec3(0.04));
    float D = DistributionGGX(N, H, clearCoatRoughness);
    float G = GeometrySmith(N, V, L, clearCoatRoughness);
    
    vec3 specular = (D * F * G) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001);
    
    return specular * clearCoat;
}

void MAIN()
{
    // 基础 PBR 层
    BASE_COLOR = vec4(albedo, 1.0);
    METALNESS = metallic;
    ROUGHNESS = roughness;
    NORMAL = normal;
    
    // 清漆层参数
    float clearCoat = texture(clearCoatMap, vTexCoord).r * clearCoatStrength;
    float clearCoatRoughness = texture(clearCoatRoughnessMap, vTexCoord).r * clearCoatRoughnessValue;
    
    // 清漆层法线（可以与基础层不同）
    vec3 clearCoatNormal = texture(clearCoatNormalMap, vTexCoord).xyz * 2.0 - 1.0;
    clearCoatNormal = normalize(TBN * clearCoatNormal);
    
    // 清漆层会在光照计算后添加
    // 可以通过 EMISSIVE_COLOR 或后处理添加
}
```

### 4. 布料着色（Cloth Shading）

专门用于织物材质的着色模型。

```glsl
// Charlie 布料 BRDF
float DistributionCharlie(float NdotH, float roughness)
{
    float invR = 1.0 / roughness;
    float cos2h = NdotH * NdotH;
    float sin2h = 1.0 - cos2h;
    
    return (2.0 + invR) * pow(sin2h, invR * 0.5) / (2.0 * PI);
}

// 布料菲涅尔（Sheen）
vec3 ClothFresnel(float VdotH, vec3 sheenColor)
{
    return sheenColor * pow(1.0 - VdotH, 5.0);
}

void MAIN()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(CAMERA_POSITION - vWorldPos);
    
    // 布料特有参数
    vec3 sheenColor = texture(sheenColorMap, vTexCoord).rgb * sheenColorValue;
    float sheenRoughness = texture(sheenRoughnessMap, vTexCoord).r;
    
    BASE_COLOR = vec4(albedo, 1.0);
    METALNESS = 0.0; // 布料通常是非金属
    ROUGHNESS = roughness;
    NORMAL = N;
    
    // Sheen 效果通过自发光添加
    float NdotV = max(dot(N, V), 0.0);
    vec3 sheen = ClothFresnel(NdotV, sheenColor);
    EMISSIVE_COLOR = sheen;
}
```

### 5. 透明和折射

```glsl
// 折射计算
vec3 Refraction(vec3 I, vec3 N, float ior)
{
    float cosi = clamp(dot(I, N), -1.0, 1.0);
    float etai = 1.0; // 空气
    float etat = ior; // 材质
    
    vec3 n = N;
    if (cosi < 0.0) {
        cosi = -cosi;
    } else {
        float temp = etai;
        etai = etat;
        etat = temp;
        n = -N;
    }
    
    float eta = etai / etat;
    float k = 1.0 - eta * eta * (1.0 - cosi * cosi);
    
    return k < 0.0 ? vec3(0.0) : eta * I + (eta * cosi - sqrt(k)) * n;
}

void MAIN()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(CAMERA_POSITION - vWorldPos);
    vec3 I = -V;
    
    // 计算折射方向
    vec3 refractDir = Refraction(I, N, ior);
    
    // 采样环境贴图或背景
    vec3 refractColor = SampleEnvironment(refractDir, roughness * 5.0);
    
    // 菲涅尔混合反射和折射
    float fresnel = FresnelSchlick(max(dot(N, V), 0.0), vec3(0.04)).r;
    
    BASE_COLOR = vec4(albedo, 1.0);
    METALNESS = 0.0;
    ROUGHNESS = roughness;
    OPACITY = 1.0 - transmission; // 透明度
    
    // 混合折射颜色
    EMISSIVE_COLOR = refractColor * transmission * (1.0 - fresnel);
}
```

### 6. 体积雾和大气散射

```glsl
// 简化的体积雾
vec3 ApplyFog(vec3 color, float distance, vec3 fogColor, float fogDensity)
{
    float fogAmount = 1.0 - exp(-distance * fogDensity);
    return mix(color, fogColor, fogAmount);
}

// 大气散射（Rayleigh + Mie）
vec3 AtmosphericScattering(vec3 viewDir, vec3 sunDir, float distance)
{
    float sunDot = dot(viewDir, sunDir);
    
    // Rayleigh 散射（蓝色天空）
    vec3 rayleigh = vec3(0.26, 0.41, 0.58) * (1.0 + sunDot * sunDot);
    
    // Mie 散射（太阳周围的光晕）
    float mie = pow(max(sunDot, 0.0), 8.0);
    
    float scatter = exp(-distance * 0.01);
    return (rayleigh + mie * vec3(1.0, 0.9, 0.8)) * (1.0 - scatter);
}

void MAIN()
{
    BASE_COLOR = vec4(albedo, 1.0);
    METALNESS = metallic;
    ROUGHNESS = roughness;
    
    // 计算到相机的距离
    float distance = length(CAMERA_POSITION - vWorldPos);
    
    // 应用大气效果（在后处理中）
    vec3 viewDir = normalize(vWorldPos - CAMERA_POSITION);
    vec3 atmosphere = AtmosphericScattering(viewDir, lightDirection, distance);
    
    EMISSIVE_COLOR = atmosphere * atmosphereStrength;
}
```

### 7. 程序化纹理

```glsl
// Perlin 噪声
float PerlinNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Voronoi 噪声
vec2 VoronoiNoise(vec2 p)
{
    vec2 n = floor(p);
    vec2 f = fract(p);
    
    float minDist = 1.0;
    vec2 minPoint;
    
    for(int j = -1; j <= 1; j++)
    for(int i = -1; i <= 1; i++)
    {
        vec2 neighbor = vec2(float(i), float(j));
        vec2 point = hash2(n + neighbor);
        vec2 diff = neighbor + point - f;
        float dist = length(diff);
        
        if(dist < minDist)
        {
            minDist = dist;
            minPoint = point;
        }
    }
    
    return vec2(minDist, 0.0);
}

// 程序化大理石纹理
vec3 MarbleTexture(vec3 pos)
{
    float n = PerlinNoise(pos.xy * 2.0);
    n += 0.5 * PerlinNoise(pos.xy * 4.0);
    n += 0.25 * PerlinNoise(pos.xy * 8.0);
    
    float pattern = sin((pos.x + n) * 10.0);
    pattern = pattern * 0.5 + 0.5;
    
    vec3 color1 = vec3(0.9, 0.9, 0.9);
    vec3 color2 = vec3(0.3, 0.3, 0.3);
    
    return mix(color1, color2, pattern);
}

void MAIN()
{
    // 使用世界坐标生成程序化纹理
    vec3 worldPos = vWorldPos;
    vec3 proceduralColor = MarbleTexture(worldPos * textureScale);
    
    BASE_COLOR = vec4(proceduralColor, 1.0);
    METALNESS = metallic;
    ROUGHNESS = roughness;
}
```

### 8. 三平面投影（Triplanar Mapping）

解决 UV 展开问题，适用于地形等复杂几何体。

```glsl
// 三平面投影采样
vec4 TriplanarMapping(sampler2D tex, vec3 pos, vec3 normal, float scale)
{
    // 计算混合权重
    vec3 blending = abs(normal);
    blending = normalize(max(blending, 0.00001));
    float b = (blending.x + blending.y + blending.z);
    blending /= vec3(b, b, b);
    
    // 三个平面的采样
    vec4 xaxis = texture(tex, pos.yz * scale);
    vec4 yaxis = texture(tex, pos.xz * scale);
    vec4 zaxis = texture(tex, pos.xy * scale);
    
    // 混合
    return xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;
}

void MAIN()
{
    vec3 worldPos = vWorldPos;
    vec3 worldNormal = normalize(vNormal);
    
    // 使用三平面投影采样所有纹理
    vec4 albedo = TriplanarMapping(albedoTexture, worldPos, worldNormal, triplanarScale);
    vec4 normalMap = TriplanarMapping(normalTexture, worldPos, worldNormal, triplanarScale);
    float roughness = TriplanarMapping(roughnessTexture, worldPos, worldNormal, triplanarScale).r;
    
    BASE_COLOR = albedo;
    METALNESS = metallic;
    ROUGHNESS = roughness;
    NORMAL = normalize(worldNormal + (normalMap.xyz * 2.0 - 1.0));
}
```

---

## 性能优化

### 1. LOD 和着色器复杂度

```qml
// 根据距离使用不同的材质
Model {
    property real distanceToCamera: 0.0
    
    materials: distanceToCamera < 10.0 ? highQualityMaterial : 
               distanceToCamera < 50.0 ? mediumQualityMaterial : 
               lowQualityMaterial
    
    CustomMaterial {
        id: highQualityMaterial
        // 完整的 PBR + SSS + Clear Coat
    }
    
    CustomMaterial {
        id: mediumQualityMaterial
        // 标准 PBR
    }
    
    CustomMaterial {
        id: lowQualityMaterial
        // 简化的光照模型
    }
}
```

### 2. 着色器优化技巧

```glsl
// 避免分支
// 不好的做法
if (useTexture) {
    color = texture(tex, uv);
} else {
    color = baseColor;
}

// 好的做法
float useTexFlag = float(useTexture);
color = mix(baseColor, texture(tex, uv), useTexFlag);

// 预计算常量
const float PI = 3.14159265359;
const float INV_PI = 0.31830988618;

// 使用内置函数
// 不好：pow(x, 2.0)
// 好：x * x

// 不好：pow(x, 0.5)
// 好：sqrt(x)

// 早期退出
void MAIN()
{
    if (OPACITY < 0.01) {
        discard;
        return;
    }
    
    // 其他计算...
}
```


### 3. 纹理优化

```qml
// 使用压缩纹理格式
Texture {
    source: "textures/albedo.ktx" // KTX 格式支持压缩
    generateMipmaps: true
    mipFilter: Texture.Linear
}

// 合并通道减少纹理数量
// R: Metallic, G: Roughness, B: AO
Texture {
    id: packedTexture
    source: "textures/packed_mroa.png"
}

// 在着色器中解包
void MAIN()
{
    vec3 packed = texture(packedTexture, vTexCoord).rgb;
    float metallic = packed.r;
    float roughness = packed.g;
    float ao = packed.b;
}
```

### 4. 实例化渲染

```qml
// 使用 Instancing 渲染大量相同物体
Model {
    source: "#Cube"
    instancing: InstanceList {
        instances: [
            InstanceListEntry { position: Qt.vector3d(0, 0, 0) },
            InstanceListEntry { position: Qt.vector3d(2, 0, 0) },
            // ... 更多实例
        ]
    }
    materials: CustomMaterial {
        // 共享材质
    }
}
```

---

## 完整实例

### 实例 1：高级 PBR 金属材质

**QML 定义**
```qml
import QtQuick
import QtQuick3D

CustomMaterial {
    id: advancedMetalMaterial
    
    shadingMode: CustomMaterial.Shaded
    
    // 材质参数
    property real metalness: 1.0
    property real roughness: 0.3
    property color baseColor: "#c0c0c0"
    property real anisotropy: 0.5
    property real anisotropyRotation: 0.0
    
    // 纹理输入
    property TextureInput albedoMap: TextureInput {
        texture: Texture {
            source: "textures/metal_albedo.png"
            generateMipmaps: true
        }
    }
    
    property TextureInput normalMap: TextureInput {
        texture: Texture {
            source: "textures/metal_normal.png"
            generateMipmaps: true
        }
    }
    
    property TextureInput roughnessMap: TextureInput {
        texture: Texture {
            source: "textures/metal_roughness.png"
            generateMipmaps: true
        }
    }
    
    property TextureInput aoMap: TextureInput {
        texture: Texture {
            source: "textures/metal_ao.png"
            generateMipmaps: true
        }
    }
    
    vertexShader: "shaders/advanced_metal.vert"
    fragmentShader: "shaders/advanced_metal.frag"
}
```

**顶点着色器 (advanced_metal.vert)**
```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vTangent;
VARYING vec3 vBinormal;
VARYING vec2 vTexCoord;
VARYING vec3 vViewDir;

void MAIN()
{
    vTexCoord = UV0;
    
    // 世界空间变换
    vec4 worldPos = MODEL_MATRIX * vec4(VERTEX, 1.0);
    vWorldPos = worldPos.xyz;
    
    // 法线、切线、副法线
    vNormal = normalize(NORMAL_MATRIX * NORMAL);
    vTangent = normalize(NORMAL_MATRIX * TANGENT);
    vBinormal = normalize(NORMAL_MATRIX * BINORMAL);
    
    // 视图方向
    vViewDir = CAMERA_POSITION - vWorldPos;
    
    // 输出位置
    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
}
```

**片段着色器 (advanced_metal.frag)**
```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vTangent;
VARYING vec3 vBinormal;
VARYING vec2 vTexCoord;
VARYING vec3 vViewDir;

const float PI = 3.14159265359;

// 各向异性 GGX
float DistributionGGXAnisotropic(vec3 N, vec3 H, vec3 T, vec3 B, float roughnessT, float roughnessB)
{
    float NdotH = max(dot(N, H), 0.0);
    float TdotH = dot(T, H);
    float BdotH = dot(B, H);
    
    float a2T = roughnessT * roughnessT;
    float a2B = roughnessB * roughnessB;
    
    float denom = (TdotH * TdotH) / a2T + (BdotH * BdotH) / a2B + NdotH * NdotH;
    
    return 1.0 / (PI * a2T * a2B * denom * denom + 0.0001);
}

void MAIN()
{
    // 采样纹理
    vec4 albedoSample = texture(albedoMap, vTexCoord);
    vec3 normalSample = texture(normalMap, vTexCoord).xyz * 2.0 - 1.0;
    float roughnessSample = texture(roughnessMap, vTexCoord).r;
    float aoSample = texture(aoMap, vTexCoord).r;
    
    // 构建 TBN 矩阵
    vec3 T = normalize(vTangent);
    vec3 B = normalize(vBinormal);
    vec3 N = normalize(vNormal);
    
    // 旋转各向异性方向
    float angle = anisotropyRotation * PI * 2.0;
    vec3 anisoT = cos(angle) * T + sin(angle) * B;
    vec3 anisoB = -sin(angle) * T + cos(angle) * B;
    
    // 应用法线贴图
    mat3 TBN = mat3(T, B, N);
    vec3 worldNormal = normalize(TBN * normalSample);
    
    // 各向异性粗糙度
    float roughnessT = roughness * roughnessSample * (1.0 + anisotropy);
    float roughnessB = roughness * roughnessSample * (1.0 - anisotropy);
    
    // 设置 PBR 参数
    BASE_COLOR = vec4(albedoSample.rgb * baseColor.rgb, 1.0);
    METALNESS = metalness;
    ROUGHNESS = mix(roughnessT, roughnessB, 0.5); // Qt 使用平均值
    NORMAL = worldNormal;
    OCCLUSION = aoSample;
    
    // 各向异性效果需要在光照计算中实现
    // 这里我们通过调整法线来近似
    vec3 V = normalize(vViewDir);
    vec3 anisoNormal = worldNormal;
    
    // 根据视角调整法线以模拟各向异性
    float VdotT = dot(V, anisoT);
    float VdotB = dot(V, anisoB);
    anisoNormal += anisoT * VdotT * anisotropy * 0.5;
    anisoNormal += anisoB * VdotB * anisotropy * 0.5;
    
    NORMAL = normalize(anisoNormal);
}
```

### 实例 2：玻璃材质（折射 + 反射）

**QML 定义**
```qml
CustomMaterial {
    id: glassMaterial
    
    shadingMode: CustomMaterial.Shaded
    
    property real ior: 1.5 // 折射率
    property real thickness: 1.0
    property color tintColor: Qt.rgba(0.9, 0.95, 1.0, 1.0)
    property real roughness: 0.05
    
    property TextureInput normalMap: TextureInput {
        texture: Texture { source: "textures/glass_normal.png" }
    }
    
    vertexShader: "shaders/glass.vert"
    fragmentShader: "shaders/glass.frag"
    
    // 透明混合
    cullMode: Material.NoCulling
    depthDrawMode: Material.OpaqueOnlyDepthDraw
}
```

**片段着色器 (glass.frag)**
```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vTangent;
VARYING vec3 vBinormal;
VARYING vec2 vTexCoord;

const float PI = 3.14159265359;

// 菲涅尔方程（精确版本）
float FresnelDielectric(float cosThetaI, float ior)
{
    float etaI = 1.0; // 空气
    float etaT = ior; // 玻璃
    
    float sinThetaT = etaI / etaT * sqrt(max(0.0, 1.0 - cosThetaI * cosThetaI));
    
    if (sinThetaT >= 1.0) {
        return 1.0; // 全反射
    }
    
    float cosThetaT = sqrt(max(0.0, 1.0 - sinThetaT * sinThetaT));
    
    float Rs = ((etaT * cosThetaI) - (etaI * cosThetaT)) / 
               ((etaT * cosThetaI) + (etaI * cosThetaT));
    float Rp = ((etaI * cosThetaI) - (etaT * cosThetaT)) / 
               ((etaI * cosThetaI) + (etaT * cosThetaT));
    
    return (Rs * Rs + Rp * Rp) / 2.0;
}

void MAIN()
{
    // 法线处理
    vec3 normalSample = texture(normalMap, vTexCoord).xyz * 2.0 - 1.0;
    mat3 TBN = mat3(normalize(vTangent), normalize(vBinormal), normalize(vNormal));
    vec3 N = normalize(TBN * normalSample);
    
    vec3 V = normalize(CAMERA_POSITION - vWorldPos);
    float NdotV = max(dot(N, V), 0.0);
    
    // 计算菲涅尔
    float fresnel = FresnelDielectric(NdotV, ior);
    
    // 玻璃的吸收（Beer-Lambert 定律）
    vec3 absorption = exp(-tintColor.rgb * thickness);
    
    // 设置 PBR 参数
    BASE_COLOR = vec4(tintColor.rgb, 1.0);
    METALNESS = 0.0;
    ROUGHNESS = roughness;
    NORMAL = N;
    OPACITY = 0.1 + fresnel * 0.9; // 基于菲涅尔的透明度
    
    // 添加色散效果（可选）
    vec3 dispersion = vec3(
        FresnelDielectric(NdotV, ior - 0.01),
        FresnelDielectric(NdotV, ior),
        FresnelDielectric(NdotV, ior + 0.01)
    );
    
    EMISSIVE_COLOR = (dispersion - fresnel) * 0.1;
}
```

### 实例 3：皮肤材质（SSS）

**QML 定义**
```qml
CustomMaterial {
    id: skinMaterial
    
    shadingMode: CustomMaterial.Shaded
    
    property color skinColor: "#f4c2a0"
    property real roughness: 0.4
    property real sssStrength: 0.5
    property color sssColor: "#ff6347"
    
    property TextureInput albedoMap: TextureInput {
        texture: Texture { source: "textures/skin_albedo.png" }
    }
    
    property TextureInput normalMap: TextureInput {
        texture: Texture { source: "textures/skin_normal.png" }
    }
    
    property TextureInput thicknessMap: TextureInput {
        texture: Texture { source: "textures/skin_thickness.png" }
    }
    
    vertexShader: "shaders/skin.vert"
    fragmentShader: "shaders/skin.frag"
}
```

**片段着色器 (skin.frag)**
```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vTangent;
VARYING vec3 vBinormal;
VARYING vec2 vTexCoord;

const float PI = 3.14159265359;

// 次表面散射近似
vec3 SubsurfaceScattering(vec3 L, vec3 V, vec3 N, float thickness, vec3 sssColor)
{
    // 透射光方向
    vec3 H = normalize(L + N * 0.3); // distortion
    float VdotH = pow(clamp(dot(V, -H), 0.0, 1.0), 3.0); // power
    
    // 基于厚度的衰减
    float attenuation = (1.0 - thickness) * 0.5 + 0.5;
    
    return sssColor * VdotH * attenuation;
}

// 双叶 BRDF（用于皮肤）
float DualLobeBRDF(vec3 N, vec3 V, vec3 L, float roughness1, float roughness2, float mix)
{
    vec3 H = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0);
    
    // 两个镜面波瓣
    float D1 = pow(NdotH, 1.0 / (roughness1 * roughness1));
    float D2 = pow(NdotH, 1.0 / (roughness2 * roughness2));
    
    return mix * D1 + (1.0 - mix) * D2;
}

void MAIN()
{
    // 采样纹理
    vec4 albedoSample = texture(albedoMap, vTexCoord);
    vec3 normalSample = texture(normalMap, vTexCoord).xyz * 2.0 - 1.0;
    float thickness = texture(thicknessMap, vTexCoord).r;
    
    // 法线处理
    mat3 TBN = mat3(normalize(vTangent), normalize(vBinormal), normalize(vNormal));
    vec3 N = normalize(TBN * normalSample);
    
    vec3 V = normalize(CAMERA_POSITION - vWorldPos);
    
    // 设置基础 PBR 参数
    BASE_COLOR = vec4(albedoSample.rgb * skinColor.rgb, 1.0);
    METALNESS = 0.0; // 皮肤是非金属
    ROUGHNESS = roughness;
    NORMAL = N;
    
    // SSS 会在光照计算后添加
    // 这里我们通过降低粗糙度来模拟皮肤的油脂层
    float oilLayer = 0.3;
    ROUGHNESS = mix(roughness, oilLayer, 0.3);
}
```


### 实例 4：汽车漆面（Clear Coat + Flakes）

**QML 定义**
```qml
CustomMaterial {
    id: carPaintMaterial
    
    shadingMode: CustomMaterial.Shaded
    
    // 基础漆层
    property color baseColor: "#cc0000"
    property real metalness: 0.8
    property real roughness: 0.6
    
    // 清漆层
    property real clearCoat: 1.0
    property real clearCoatRoughness: 0.05
    
    // 金属薄片
    property real flakeSize: 0.5
    property real flakeDensity: 0.8
    property color flakeColor: "#silver"
    
    property TextureInput flakeNormalMap: TextureInput {
        texture: Texture {
            source: "textures/flakes_normal.png"
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
        }
    }
    
    vertexShader: "shaders/car_paint.vert"
    fragmentShader: "shaders/car_paint.frag"
}
```

**片段着色器 (car_paint.frag)**
```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vTangent;
VARYING vec3 vBinormal;
VARYING vec2 vTexCoord;

const float PI = 3.14159265359;

// 随机函数
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

// 金属薄片效果
vec3 MetallicFlakes(vec2 uv, vec3 N, vec3 V, float size, float density)
{
    vec2 flakeUV = uv / size;
    vec2 flakeID = floor(flakeUV);
    vec2 flakeLocal = fract(flakeUV);
    
    // 随机薄片方向
    float random = hash(flakeID);
    if (random > density) return vec3(0.0);
    
    // 薄片法线扰动
    vec3 flakeNormal = texture(flakeNormalMap, flakeUV * 10.0).xyz * 2.0 - 1.0;
    flakeNormal.xy *= 0.5; // 减小扰动强度
    
    vec3 perturbedN = normalize(N + flakeNormal);
    
    // 镜面反射
    vec3 R = reflect(-V, perturbedN);
    float spec = pow(max(dot(R, V), 0.0), 50.0);
    
    return flakeColor.rgb * spec * random;
}

// 清漆层 BRDF
vec3 ClearCoatLayer(vec3 N, vec3 V, vec3 L, float clearCoat, float clearCoatRoughness)
{
    vec3 H = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float VdotH = max(dot(V, H), 0.0);
    
    // 清漆层使用固定的 F0
    float F0 = 0.04;
    float F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);
    
    // GGX 分布
    float a = clearCoatRoughness * clearCoatRoughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    float D = a2 / (PI * denom * denom);
    
    // 几何项
    float k = (clearCoatRoughness + 1.0) * (clearCoatRoughness + 1.0) / 8.0;
    float G1 = NdotV / (NdotV * (1.0 - k) + k);
    float G2 = NdotL / (NdotL * (1.0 - k) + k);
    float G = G1 * G2;
    
    return vec3(D * F * G / (4.0 * NdotV * NdotL + 0.001)) * clearCoat;
}

void MAIN()
{
    // 法线处理
    vec3 N = normalize(vNormal);
    vec3 T = normalize(vTangent);
    vec3 B = normalize(vBinormal);
    mat3 TBN = mat3(T, B, N);
    
    vec3 V = normalize(CAMERA_POSITION - vWorldPos);
    
    // 金属薄片效果
    vec3 flakes = MetallicFlakes(vTexCoord, N, V, flakeSize, flakeDensity);
    
    // 基础漆层颜色（混合薄片）
    vec3 paintColor = mix(baseColor.rgb, flakes, 0.3);
    
    // 设置 PBR 参数
    BASE_COLOR = vec4(paintColor, 1.0);
    METALNESS = metalness;
    ROUGHNESS = roughness;
    NORMAL = N;
    
    // 清漆层会增加镜面反射
    // 通过降低粗糙度来模拟
    ROUGHNESS = mix(roughness, clearCoatRoughness, clearCoat * 0.5);
    
    // 添加额外的镜面高光（薄片）
    EMISSIVE_COLOR = flakes * 0.5;
}
```

### 实例 5：全息材质

**QML 定义**
```qml
CustomMaterial {
    id: holographicMaterial
    
    shadingMode: CustomMaterial.Shaded
    
    property real time: 0.0
    property color color1: "#ff00ff"
    property color color2: "#00ffff"
    property color color3: "#ffff00"
    property real frequency: 5.0
    property real speed: 1.0
    
    NumberAnimation on time {
        from: 0
        to: 100
        duration: 100000
        loops: Animation.Infinite
    }
    
    vertexShader: "shaders/holographic.vert"
    fragmentShader: "shaders/holographic.frag"
}
```

**片段着色器 (holographic.frag)**
```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec2 vTexCoord;

const float PI = 3.14159265359;

// 彩虹色谱
vec3 Rainbow(float t)
{
    t = fract(t);
    float r = abs(t * 6.0 - 3.0) - 1.0;
    float g = 2.0 - abs(t * 6.0 - 2.0);
    float b = 2.0 - abs(t * 6.0 - 4.0);
    return clamp(vec3(r, g, b), 0.0, 1.0);
}

// 菲涅尔彩虹
vec3 FresnelRainbow(vec3 N, vec3 V, float offset)
{
    float fresnel = 1.0 - max(dot(N, V), 0.0);
    fresnel = pow(fresnel, 2.0);
    
    float hue = fresnel + offset;
    return Rainbow(hue);
}

// 全息干涉条纹
float HolographicPattern(vec2 uv, float time, float frequency)
{
    float pattern = 0.0;
    
    // 多层干涉
    pattern += sin(uv.x * frequency + time) * 0.5;
    pattern += sin(uv.y * frequency * 1.3 - time * 0.7) * 0.3;
    pattern += sin((uv.x + uv.y) * frequency * 0.7 + time * 1.5) * 0.2;
    
    return pattern * 0.5 + 0.5;
}

void MAIN()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(CAMERA_POSITION - vWorldPos);
    
    // 计算菲涅尔彩虹
    float timeOffset = time * speed * 0.1;
    vec3 rainbow = FresnelRainbow(N, V, timeOffset);
    
    // 全息干涉图案
    float pattern = HolographicPattern(vTexCoord, time * speed, frequency);
    
    // 混合多个颜色
    vec3 color = mix(color1.rgb, color2.rgb, pattern);
    color = mix(color, color3.rgb, sin(time * speed * 0.5) * 0.5 + 0.5);
    
    // 应用彩虹效果
    color = mix(color, rainbow, 0.5);
    
    // 闪烁效果
    float sparkle = pow(pattern, 10.0);
    
    // 设置材质参数
    BASE_COLOR = vec4(color, 1.0);
    METALNESS = 0.9;
    ROUGHNESS = 0.1;
    NORMAL = N;
    EMISSIVE_COLOR = rainbow * sparkle * 2.0;
}
```

### 实例 6：体积云材质

**QML 定义**
```qml
CustomMaterial {
    id: volumetricCloudMaterial
    
    shadingMode: CustomMaterial.Unshaded // 使用 Unshaded 进行体积渲染
    
    property real time: 0.0
    property real density: 0.5
    property real absorption: 0.3
    property color cloudColor: "#ffffff"
    property vector3d windDirection: Qt.vector3d(1, 0, 0)
    property real windSpeed: 0.1
    
    property TextureInput noiseTexture: TextureInput {
        texture: Texture {
            source: "textures/noise3d.png"
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
        }
    }
    
    cullMode: Material.NoCulling
    depthDrawMode: Material.NeverDepthDraw
    
    vertexShader: "shaders/volumetric_cloud.vert"
    fragmentShader: "shaders/volumetric_cloud.frag"
}
```

**片段着色器 (volumetric_cloud.frag)**
```glsl
VARYING vec3 vWorldPos;
VARYING vec3 vNormal;
VARYING vec3 vLocalPos;
VARYING vec2 vTexCoord;

const int MAX_STEPS = 64;
const float STEP_SIZE = 0.05;

// 3D 噪声采样
float SampleNoise3D(vec3 pos)
{
    // 使用 2D 纹理模拟 3D 噪声
    vec2 uv1 = pos.xy + vec2(0.0, pos.z);
    vec2 uv2 = pos.xy + vec2(0.0, pos.z + 1.0);
    
    float noise1 = texture(noiseTexture, uv1).r;
    float noise2 = texture(noiseTexture, uv2).r;
    
    return mix(noise1, noise2, fract(pos.z));
}

// FBM（分形布朗运动）
float FBM(vec3 pos, int octaves)
{
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    
    for (int i = 0; i < octaves; i++)
    {
        value += SampleNoise3D(pos * frequency) * amplitude;
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    
    return value;
}

// 云密度函数
float CloudDensity(vec3 pos)
{
    // 添加风的影响
    vec3 windOffset = windDirection * time * windSpeed;
    vec3 samplePos = pos + windOffset;
    
    // 多层噪声
    float baseNoise = FBM(samplePos * 0.5, 4);
    float detailNoise = FBM(samplePos * 2.0, 3);
    
    // 组合噪声
    float cloudShape = baseNoise - detailNoise * 0.3;
    
    // 应用密度参数
    cloudShape = (cloudShape - (1.0 - density)) / density;
    
    return clamp(cloudShape, 0.0, 1.0);
}

// 光线步进
vec4 RayMarch(vec3 rayOrigin, vec3 rayDir)
{
    vec3 pos = rayOrigin;
    float transmittance = 1.0;
    vec3 scattering = vec3(0.0);
    
    for (int i = 0; i < MAX_STEPS; i++)
    {
        // 采样密度
        float density = CloudDensity(pos);
        
        if (density > 0.01)
        {
            // 吸收
            float absorb = exp(-density * absorption * STEP_SIZE);
            transmittance *= absorb;
            
            // 散射（简化的光照）
            vec3 lightDir = normalize(vec3(1.0, 1.0, 0.5));
            float lightDensity = CloudDensity(pos + lightDir * STEP_SIZE * 2.0);
            float lightEnergy = exp(-lightDensity * absorption);
            
            scattering += cloudColor.rgb * density * transmittance * lightEnergy * STEP_SIZE;
            
            // 早期退出
            if (transmittance < 0.01) break;
        }
        
        pos += rayDir * STEP_SIZE;
        
        // 边界检查
        if (any(lessThan(pos, vec3(-1.0))) || any(greaterThan(pos, vec3(1.0))))
            break;
    }
    
    return vec4(scattering, 1.0 - transmittance);
}

void MAIN()
{
    vec3 rayOrigin = vLocalPos;
    vec3 rayDir = normalize(vWorldPos - CAMERA_POSITION);
    
    // 执行光线步进
    vec4 cloud = RayMarch(rayOrigin, rayDir);
    
    // 输出颜色
    BASE_COLOR = cloud;
    OPACITY = cloud.a;
}
```

---

## 高级技巧和最佳实践

### 1. 多 Pass 渲染

```qml
// 第一个 Pass：深度预处理
CustomMaterial {
    id: depthPrepass
    shadingMode: CustomMaterial.Unshaded
    
    fragmentShader: "
        void MAIN() {
            BASE_COLOR = vec4(0.0);
            OPACITY = 1.0;
        }
    "
}

// 第二个 Pass：主渲染
CustomMaterial {
    id: mainPass
    shadingMode: CustomMaterial.Shaded
    // 主要的着色逻辑
}
```

### 2. 动态参数绑定

```qml
CustomMaterial {
    id: dynamicMaterial
    
    property real animatedValue: 0.0
    
    SequentialAnimation on animatedValue {
        loops: Animation.Infinite
        NumberAnimation { from: 0; to: 1; duration: 2000 }
        NumberAnimation { from: 1; to: 0; duration: 2000 }
    }
    
    // 在着色器中使用 animatedValue
}
```

### 3. 条件编译

```glsl
// 使用预处理器指令
#ifdef USE_NORMAL_MAP
    vec3 normal = texture(normalMap, vTexCoord).xyz * 2.0 - 1.0;
#else
    vec3 normal = vec3(0.0, 0.0, 1.0);
#endif

#ifdef HIGH_QUALITY
    const int SAMPLES = 32;
#else
    const int SAMPLES = 8;
#endif
```


### 4. 调试技巧

```glsl
// 可视化法线
void DEBUG_NORMALS()
{
    vec3 N = normalize(vNormal);
    BASE_COLOR = vec4(N * 0.5 + 0.5, 1.0);
}

// 可视化 UV
void DEBUG_UV()
{
    BASE_COLOR = vec4(vTexCoord, 0.0, 1.0);
}

// 可视化深度
void DEBUG_DEPTH()
{
    float depth = gl_FragCoord.z;
    BASE_COLOR = vec4(vec3(depth), 1.0);
}

// 可视化光照项
void DEBUG_LIGHTING()
{
    // 只显示漫反射
    BASE_COLOR = vec4(DIFFUSE, 1.0);
    
    // 只显示镜面反射
    // BASE_COLOR = vec4(SPECULAR, 1.0);
    
    // 只显示 IBL
    // BASE_COLOR = vec4(IBL_DIFFUSE + IBL_SPECULAR, 1.0);
}

void MAIN()
{
    // 正常渲染
    BASE_COLOR = vec4(albedo, 1.0);
    METALNESS = metallic;
    ROUGHNESS = roughness;
    
    // 取消注释以调试
    // DEBUG_NORMALS();
    // DEBUG_UV();
    // DEBUG_LIGHTING();
}
```

### 5. 着色器库和复用

**创建着色器库文件 (shader_lib.glsl)**
```glsl
// 常用常量
#ifndef SHADER_LIB_GLSL
#define SHADER_LIB_GLSL

const float PI = 3.14159265359;
const float INV_PI = 0.31830988618;
const float EPSILON = 0.00001;

// 常用函数
float Square(float x) { return x * x; }
vec3 Square(vec3 x) { return x * x; }

float Pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

// GGX 分布
float DistributionGGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

// Smith 几何函数
float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

// Schlick 菲涅尔
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * Pow5(1.0 - cosTheta);
}

// 带粗糙度的菲涅尔
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * Pow5(1.0 - cosTheta);
}

// sRGB 转换
vec3 SRGBToLinear(vec3 srgb)
{
    return pow(srgb, vec3(2.2));
}

vec3 LinearToSRGB(vec3 linear)
{
    return pow(linear, vec3(1.0 / 2.2));
}

// 色调映射
vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

#endif // SHADER_LIB_GLSL
```

**在着色器中使用**
```glsl
// 在片段着色器开头包含
// #include "shader_lib.glsl"  // Qt Quick 3D 不直接支持 #include

// 替代方案：复制粘贴或使用 QML 的 shaderInfo
```

### 6. 性能分析

```qml
// 使用简化版本进行性能测试
CustomMaterial {
    id: performanceTest
    
    property bool useComplexShader: true
    
    fragmentShader: useComplexShader ? 
        "shaders/complex.frag" : 
        "shaders/simple.frag"
    
    // 运行时切换以比较性能
}

// 在 QML 中测量帧率
Item {
    property int frameCount: 0
    property real lastTime: 0
    property real fps: 0
    
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            var currentTime = Date.now()
            fps = frameCount / ((currentTime - lastTime) / 1000.0)
            console.log("FPS:", fps)
            frameCount = 0
            lastTime = currentTime
        }
    }
    
    View3D {
        onBeforeRendering: frameCount++
    }
}
```

---

## 常见问题和解决方案

### 问题 1：法线贴图不工作

**原因**：TBN 矩阵构建错误或法线空间不匹配

**解决方案**：
```glsl
// 确保正确构建 TBN 矩阵
vec3 T = normalize(vTangent);
vec3 B = normalize(vBinormal);
vec3 N = normalize(vNormal);

// 检查切线空间的正交性
T = normalize(T - dot(T, N) * N); // Gram-Schmidt 正交化
B = cross(N, T);

mat3 TBN = mat3(T, B, N);

// 正确转换法线
vec3 normalMap = texture(normalTexture, vTexCoord).xyz * 2.0 - 1.0;
vec3 worldNormal = normalize(TBN * normalMap);

NORMAL = worldNormal;
```

### 问题 2：PBR 材质看起来不真实

**原因**：
- 纹理未进行 gamma 校正
- 金属度/粗糙度值不正确
- 缺少环境光照

**解决方案**：
```glsl
// 1. 正确处理 sRGB 纹理
vec3 albedo = pow(texture(albedoMap, vTexCoord).rgb, vec3(2.2));

// 2. 确保金属度是 0 或 1（或接近）
float metallic = texture(metallicMap, vTexCoord).r;
metallic = step(0.5, metallic); // 二值化

// 3. 粗糙度范围合理
float roughness = clamp(texture(roughnessMap, vTexCoord).r, 0.04, 1.0);

// 4. 确保场景有 IBL
// 在 QML 中设置 lightProbe
```

### 问题 3：透明材质渲染顺序错误

**原因**：透明物体需要从后向前排序

**解决方案**：
```qml
Model {
    materials: CustomMaterial {
        // 设置正确的混合模式
        depthDrawMode: Material.OpaquePrePass
        
        // 或者使用双面渲染
        cullMode: Material.NoCulling
    }
    
    // 手动设置渲染顺序
    z: distanceToCamera
}
```

### 问题 4：着色器编译错误

**常见错误**：
```glsl
// 错误：未声明的变量
BASE_COLOR = vec4(color, 1.0); // 'color' 未定义

// 正确：使用 VARYING 传递
VARYING vec3 vColor;
BASE_COLOR = vec4(vColor, 1.0);

// 错误：类型不匹配
float value = texture(tex, uv); // texture 返回 vec4

// 正确：
float value = texture(tex, uv).r;

// 错误：在 Shaded 模式使用 gl_FragColor
gl_FragColor = vec4(1.0);

// 正确：使用 BASE_COLOR
BASE_COLOR = vec4(1.0);
```

### 问题 5：性能问题

**优化策略**：
```glsl
// 1. 减少纹理采样
// 不好：多次采样同一纹理
float r = texture(tex, uv).r;
float g = texture(tex, uv).g;
float b = texture(tex, uv).b;

// 好：一次采样
vec3 rgb = texture(tex, uv).rgb;

// 2. 避免动态分支
// 不好：
if (condition) {
    result = expensiveFunction();
}

// 好：
result = mix(0.0, expensiveFunction(), float(condition));

// 3. 预计算
// 不好：在片段着色器中计算常量
float value = sin(3.14159 * 0.5);

// 好：在顶点着色器或 CPU 端计算

// 4. 使用 LOD
float lod = distance(vWorldPos, CAMERA_POSITION) / 10.0;
vec4 color = textureLod(tex, uv, lod);
```
****
---

## 参考资源

### 官方文档
- Qt Quick 3D CustomMaterial: https://doc.qt.io/qt-6/qml-qtquick3d-custommaterial.html
- Qt Quick 3D Shaders: https://doc.qt.io/qt-6/qtquick3d-tool-shaders.html

### PBR 理论
- "Physically Based Rendering" by Matt Pharr, Wenzel Jakob, and Greg Humphreys
- "Real-Time Rendering" by Tomas Akenine-Möller et al.
- Disney's BRDF Explorer: https://github.com/wdas/brdf

### 在线工具
- Shadertoy: https://www.shadertoy.com/
- GLSL Sandbox: http://glslsandbox.com/
- PBR Texture Converter: https://www.materialx.org/

### 纹理资源
- Poly Haven: https://polyhaven.com/
- CC0 Textures: https://cc0textures.com/
- Substance Source: https://source.substance3d.com/

---

## 总结

Qt Quick 3D 的 CustomMaterial 提供了强大而灵活的材质系统，特别是 Shaded 模式完美集成了 PBR 工作流和 IBL。通过本指南，你应该能够：

1. **理解 CustomMaterial 的三种模式**及其适用场景
2. **掌握 Shaded 模式**的内置变量和工作流程
3. **实现完整的 PBR 渲染**，包括金属/粗糙度工作流
4. **使用 IBL** 创建真实的环境光照和反射
5. **实现高级效果**：SSS、各向异性、清漆层、体积渲染等
6. **优化性能**，编写高效的着色器代码
7. **调试和解决**常见问题

CustomMaterial 的强大之处在于它既提供了 Qt 的便利性（自动光照计算、IBL 集成），又保留了完全的着色器控制权。通过合理使用这些特性，你可以创建出媲美专业游戏引擎的视觉效果。

记住：
- 从简单开始，逐步添加复杂度
- 始终考虑性能影响
- 利用 Qt 的内置功能（Shaded 模式的自动光照）
- 参考真实世界的材质属性
- 多测试、多迭代

祝你在 Qt Quick 3D 的 3D 渲染之旅中取得成功！
