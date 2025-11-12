# Qt6 Quick 3D 性能优化与高级技巧

> 本文档深入探讨 Qt6 Quick 3D 的性能优化策略、Draw Call 优化、内存管理、渲染管线优化等高级主题，帮助开发者构建高性能的 3D 应用。

## 目录

1. [性能分析基础](#1-性能分析基础)
2. [Draw Call 优化](#2-draw-call-优化)
3. [几何体优化](#3-几何体优化)
4. [材质与着色器优化](#4-材质与着色器优化)
5. [纹理优化](#5-纹理优化)
6. [光照与阴影优化](#6-光照与阴影优化)
7. [内存管理](#7-内存管理)
8. [渲染管线优化](#8-渲染管线优化)
9. [CPU 与 GPU 协同优化](#9-cpu-与-gpu-协同优化)
10. [高级技巧与最佳实践](#10-高级技巧与最佳实践)

---

## 1. 性能分析基础

### 1.1 性能指标

**关键性能指标（KPI）**：

| 指标 | 目标值 | 说明 |
|------|--------|------|
| **FPS** | ≥60 | 帧率，流畅度的直接体现 |
| **帧时间** | ≤16.67ms | 单帧渲染时间（60 FPS） |
| **Draw Calls** | <100 | 绘制调用次数，越少越好 |
| **三角形数** | <100K | 每帧渲染的三角形数量 |
| **纹理内存** | <512MB | 纹理占用的显存 |
| **顶点数** | <500K | 每帧处理的顶点数 |

### 1.2 性能瓶颈识别

**CPU 瓶颈特征**：
- FPS 不随分辨率变化
- 降低渲染质量无效果
- 减少对象数量有明显改善

**GPU 瓶颈特征**：
- FPS 随分辨率变化明显
- 降低渲染质量有改善
- 减少像素着色器复杂度有效

**内存瓶颈特征**：
- 加载时卡顿
- 纹理切换时掉帧
- 内存占用持续增长

### 1.3 性能分析工具

**Qt 内置工具**：
- `QSG_RENDER_TIMING=1` - 显示渲染时间
- `QSG_VISUALIZE=overdraw` - 可视化过度绘制
- `QSG_VISUALIZE=batches` - 可视化批次
- `QT_LOGGING_RULES="qt.scenegraph.*=true"` - 场景图日志

**使用示例**：
```bash
# Windows
set QSG_RENDER_TIMING=1
set QSG_VISUALIZE=batches
YourApp.exe

# Linux/Mac
QSG_RENDER_TIMING=1 QSG_VISUALIZE=batches ./YourApp
```

---

## 2. Draw Call 优化

### 2.1 什么是 Draw Call？

Draw Call 是 CPU 向 GPU 发送的绘制命令。每次 Draw Call 都有开销：
- CPU 准备数据
- 驱动验证状态
- GPU 切换状态

**性能影响**：
- 1000 个 Draw Calls ≈ 10-20ms（CPU 时间）
- 100 个 Draw Calls ≈ 1-2ms
- 10 个 Draw Calls ≈ 0.1-0.2ms

### 2.2 实例化渲染（Instancing）

**最有效的 Draw Call 优化方法**

```qml
import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 500, 1000)
        eulerRotation.x: -25
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ❌ 错误方式：1000 个 Model = 1000 次 Draw Call
    // Repeater3D {
    //     model: 1000
    //     Model {
    //         source: "#Cube"
    //         position: Qt.vector3d(...)
    //     }
    // }
    
    // ✅ 正确方式：1000 个实例 = 1 次 Draw Call
    Model {
        source: "#Cube"
        
        instancing: InstanceList {
            id: instances
            
            Component.onCompleted: {
                for (let i = 0; i < 1000; i++) {
                    instances.append({
                        position: Qt.vector3d(
                            (Math.random() - 0.5) * 2000,
                            Math.random() * 200,
                            (Math.random() - 0.5) * 2000
                        ),
                        scale: Qt.vector3d(1, 1, 1),
                        rotation: Qt.quaternion(1, 0, 0, 0),
                        color: Qt.hsla(Math.random(), 1.0, 0.5, 1.0)
                    })
                }
            }
        }
        
        materials: PrincipledMaterial {
            baseColor: "#4080ff"
        }
    }
}
```

**性能对比**：
- 传统方式：1000 个 Model = 1000 Draw Calls ≈ 15ms
- 实例化：1000 个实例 = 1 Draw Call ≈ 0.1ms
- **性能提升：150 倍**



### 2.3 材质批处理

**相同材质的对象会自动批处理**

```qml
// ✅ 好：共享材质，可以批处理
PrincipledMaterial {
    id: sharedMaterial
    baseColor: "#4080ff"
    metalness: 0.5
    roughness: 0.3
}

Repeater3D {
    model: 100
    Model {
        source: "#Cube"
        materials: sharedMaterial  // 共享材质
    }
}

// ❌ 差：每个对象独立材质，无法批处理
Repeater3D {
    model: 100
    Model {
        source: "#Cube"
        materials: PrincipledMaterial {  // 独立材质
            baseColor: "#4080ff"
        }
    }
}
```

### 2.4 减少状态切换

**状态切换的开销**：
- 材质切换：高
- 纹理切换：中
- 着色器切换：高
- 混合模式切换：低

**优化策略**：
1. 按材质排序对象
2. 合并相似材质
3. 使用纹理图集
4. 减少透明对象



---

## 3. 几何体优化

### 3.1 LOD（Level of Detail）

**根据距离使用不同细节级别的模型**

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(0, 100, 500)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // LOD 系统
    Repeater3D {
        model: 50
        
        Node {
            id: lodNode
            position: Qt.vector3d(
                (index % 10) * 100 - 450,
                0,
                Math.floor(index / 10) * 100 - 200
            )
            
            property real distance: {
                let dx = position.x - camera.position.x
                let dy = position.y - camera.position.y
                let dz = position.z - camera.position.z
                return Math.sqrt(dx*dx + dy*dy + dz*dz)
            }
            
            // 高细节（近距离）- 10000 三角形
            Model {
                source: "models/high_detail.mesh"
                visible: lodNode.distance < 200
                materials: PrincipledMaterial {
                    baseColor: "#4080ff"
                    normalMap: Texture { source: "normal.jpg" }
                    roughnessMap: Texture { source: "roughness.jpg" }
                }
            }
            
            // 中等细节（中距离）- 2000 三角形
            Model {
                source: "models/medium_detail.mesh"
                visible: lodNode.distance >= 200 && lodNode.distance < 500
                materials: PrincipledMaterial {
                    baseColor: "#4080ff"
                }
            }
            
            // 低细节（远距离）- 500 三角形
            Model {
                source: "#Cube"
                visible: lodNode.distance >= 500
                materials: DefaultMaterial {
                    diffuseColor: "#4080ff"
                }
            }
        }
    }
}
```

**LOD 策略**：
- 近距离（<200）：完整模型 + 完整材质
- 中距离（200-500）：简化模型 + 简化材质
- 远距离（>500）：极简模型 + 基础材质



### 3.2 视锥剔除（Frustum Culling）

**Qt Quick 3D 自动执行视锥剔除**

```qml
// 自动剔除：不在相机视野内的对象不会被渲染
PerspectiveCamera {
    id: camera
    position: Qt.vector3d(0, 0, 300)
    
    // 视锥参数
    fieldOfView: 60
    clipNear: 1      // 近裁剪面
    clipFar: 1000    // 远裁剪面
    
    // 启用视锥剔除（默认开启）
    frustumCullingEnabled: true
}

// 所有 Model 自动参与视锥剔除
// 不在视锥内的对象不会提交 Draw Call
```

**优化建议**：
- 合理设置 `clipFar`，不要过大
- 使用 `clipNear` 避免近平面穿透
- 大场景考虑空间分割（Octree/BSP）

### 3.3 遮挡剔除（Occlusion Culling）

**手动实现简单的遮挡剔除**

```qml
Node {
    id: occluder
    
    // 大型遮挡物（如建筑）
    Model {
        source: "models/building.mesh"
    }
    
    // 被遮挡的对象
    Repeater3D {
        model: 100
        
        Model {
            id: occludedObject
            source: "#Cube"
            
            // 简单的遮挡检测
            visible: {
                // 如果在遮挡物后面，隐藏
                let behindOccluder = position.z > occluder.position.z + 50
                return !behindOccluder
            }
        }
    }
}
```

### 3.4 几何体简化

**减少顶点和三角形数量**

| 模型类型 | 三角形数 | 适用场景 |
|----------|----------|----------|
| 高精度 | 10K-100K | 主角、近景物体 |
| 中精度 | 1K-10K | 普通物体 |
| 低精度 | 100-1K | 远景、背景 |
| 极简 | <100 | 占位符、碰撞体 |

**工具推荐**：
- Blender：Decimate Modifier
- Simplygon
- MeshLab



---

## 4. 材质与着色器优化

### 4.1 材质选择

**性能对比**：

| 材质类型 | 性能 | 功能 | 适用场景 |
|----------|------|------|----------|
| DefaultMaterial | 最快 | 基础 | 简单场景、移动端 |
| PrincipledMaterial | 中等 | 完整 PBR | 桌面端、高质量 |
| CustomMaterial | 可变 | 自定义 | 特殊效果 |

```qml
// ✅ 移动端/低端设备：使用 DefaultMaterial
Model {
    source: "#Sphere"
    materials: DefaultMaterial {
        diffuseColor: "#4080ff"
        specularAmount: 0.5
    }
}

// ✅ 桌面端/高端设备：使用 PrincipledMaterial
Model {
    source: "#Sphere"
    materials: PrincipledMaterial {
        baseColor: "#4080ff"
        metalness: 0.8
        roughness: 0.2
    }
}
```

### 4.2 着色器复杂度优化

**片段着色器是性能瓶颈**

```qml
// ❌ 差：复杂的片段着色器
CustomMaterial {
    fragmentShader: "
        void MAIN() {
            // 多次纹理采样
            vec4 tex1 = texture(tex1Map, UV0);
            vec4 tex2 = texture(tex2Map, UV0);
            vec4 tex3 = texture(tex3Map, UV0);
            
            // 复杂计算
            for (int i = 0; i < 10; i++) {
                // 循环计算
            }
            
            // 多次光照计算
            vec3 color = vec3(0.0);
            for (int i = 0; i < 8; i++) {
                color += calculateLight(i);
            }
            
            BASE_COLOR = vec4(color, 1.0);
        }
    "
}

// ✅ 好：简化的片段着色器
CustomMaterial {
    fragmentShader: "
        void MAIN() {
            // 单次纹理采样
            vec4 tex = texture(texMap, UV0);
            
            // 简单光照
            vec3 normal = normalize(vNormal);
            vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
            float diff = max(dot(normal, lightDir), 0.0);
            
            vec3 color = tex.rgb * (0.3 + 0.7 * diff);
            BASE_COLOR = vec4(color, 1.0);
        }
    "
}
```

**优化原则**：
- 减少纹理采样次数
- 避免循环和分支
- 将计算移到顶点着色器
- 使用查找表（LUT）替代复杂计算



---

## 5. 纹理优化

### 5.1 纹理尺寸

**纹理内存占用计算**：
```
内存 = 宽 × 高 × 每像素字节数 × Mipmap 系数

示例：
- 2048×2048 RGBA8：2048 × 2048 × 4 × 1.33 ≈ 22MB
- 1024×1024 RGBA8：1024 × 1024 × 4 × 1.33 ≈ 5.5MB
- 512×512 RGBA8：512 × 512 × 4 × 1.33 ≈ 1.4MB
```

**推荐尺寸**：

| 用途 | 推荐尺寸 | 说明 |
|------|----------|------|
| 主角纹理 | 2048×2048 | 高质量 |
| 普通物体 | 1024×1024 | 标准质量 |
| 小物体 | 512×512 | 中等质量 |
| UI/图标 | 256×256 | 低质量 |

```qml
// ✅ 好：合理的纹理尺寸
Model {
    source: "#Sphere"
    materials: PrincipledMaterial {
        baseColorMap: Texture {
            source: "textures/diffuse_1024.jpg"  // 1024×1024
            generateMipmaps: true
        }
    }
}

// ❌ 差：过大的纹理
Model {
    source: "#Sphere"
    scale: Qt.vector3d(0.1, 0.1, 0.1)  // 小物体
    materials: PrincipledMaterial {
        baseColorMap: Texture {
            source: "textures/diffuse_4096.jpg"  // 4096×4096 浪费！
        }
    }
}
```

### 5.2 纹理压缩

**压缩格式对比**：

| 格式 | 压缩比 | 质量 | 支持平台 |
|------|--------|------|----------|
| PNG/JPG | 无/有损 | 高 | 所有 |
| DDS (DXT1) | 6:1 | 中 | Desktop |
| DDS (DXT5) | 4:1 | 高 | Desktop |
| ETC2 | 4:1 | 高 | Mobile |
| ASTC | 可变 | 可变 | Modern |

```qml
// ✅ 使用压缩纹理
Texture {
    source: "textures/diffuse.dds"  // DXT 压缩
    // 或
    source: "textures/diffuse.ktx"  // KTX 容器
}
```

### 5.3 Mipmap

**Mipmap 的重要性**：
- 减少纹理采样开销
- 避免摩尔纹
- 提升远距离渲染质量

```qml
Texture {
    source: "textures/diffuse.jpg"
    
    // ✅ 启用 Mipmap（推荐）
    generateMipmaps: true
    mipFilter: Texture.Linear
    
    // 过滤模式
    magFilter: Texture.Linear  // 放大过滤
    minFilter: Texture.Linear  // 缩小过滤
}
```

### 5.4 纹理图集（Texture Atlas）

**合并多个小纹理到一张大纹理**

```qml
// ❌ 差：多个小纹理 = 多次纹理切换
Model { materials: PrincipledMaterial { baseColorMap: Texture { source: "icon1.png" } } }
Model { materials: PrincipledMaterial { baseColorMap: Texture { source: "icon2.png" } } }
Model { materials: PrincipledMaterial { baseColorMap: Texture { source: "icon3.png" } } }

// ✅ 好：纹理图集 = 1 次纹理绑定
Texture {
    id: atlas
    source: "textures/atlas.png"  // 包含所有小纹理
}

// 使用 UV 偏移访问不同区域
Model {
    materials: PrincipledMaterial {
        baseColorMap: atlas
        // 通过 UV 变换访问图集的不同部分
    }
}
```



---

## 6. 光照与阴影优化

### 6.1 光源数量优化

**光源性能开销**：

| 光源类型 | 性能开销 | 说明 |
|----------|----------|------|
| DirectionalLight | 低 | 全局光照，开销固定 |
| PointLight | 中 | 影响范围内的对象 |
| SpotLight | 高 | 需要计算锥形范围 |

```qml
// ✅ 好：少量光源
DirectionalLight {
    eulerRotation.x: -30
    brightness: 1.0
    castsShadow: true
}

PointLight {
    position: Qt.vector3d(0, 100, 0)
    brightness: 2.0
}

// ❌ 差：过多光源
Repeater3D {
    model: 50  // 50 个点光源！
    PointLight {
        position: Qt.vector3d(...)
        brightness: 1.0
    }
}
```

**优化策略**：
- 限制光源数量（<5 个）
- 使用烘焙光照（Baked Lighting）
- 使用光照贴图（Lightmap）
- 远距离光源使用环境光代替

### 6.2 阴影优化

**阴影是最大的性能杀手之一**

```qml
DirectionalLight {
    eulerRotation.x: -30
    
    // 阴影开关
    castsShadow: true  // 开启阴影
    
    // ✅ 阴影质量设置
    shadowMapQuality: Light.ShadowMapQualityMedium  // 低/中/高/非常高
    
    // 阴影范围
    shadowMapFar: 500  // ✅ 减小范围提升性能
    
    // 阴影过滤
    shadowFilter: 5.0  // ✅ 降低过滤质量
    
    // 阴影偏移
    shadowBias: 0.01
}
```

**阴影质量对比**：

| 质量 | 分辨率 | 性能开销 | 适用场景 |
|------|--------|----------|----------|
| Low | 256×256 | 低 | 移动端 |
| Medium | 512×512 | 中 | 桌面端标准 |
| High | 1024×1024 | 高 | 桌面端高质量 |
| VeryHigh | 2048×2048 | 极高 | 截图/演示 |

**优化技巧**：
```qml
// 1. 只对重要对象投射阴影
Model {
    source: "#Sphere"
    castsShadows: true   // 主要对象
    receivesShadows: true
}

Model {
    source: "#Cube"
    castsShadows: false  // 次要对象不投射
    receivesShadows: true
}

// 2. 地面只接收阴影
Model {
    source: "#Rectangle"
    castsShadows: false
    receivesShadows: true
}

// 3. 远距离对象不参与阴影
Model {
    source: "#Sphere"
    position: Qt.vector3d(0, 0, -1000)
    castsShadows: false  // 太远，不需要阴影
    receivesShadows: false
}
```



---

## 7. 内存管理

### 7.1 资源加载策略

**延迟加载（Lazy Loading）**

```qml
// ✅ 使用 Loader3D 延迟加载
Loader3D {
    id: modelLoader
    active: false  // 初始不加载
    
    sourceComponent: Component {
        Model {
            source: "models/large_model.mesh"
            materials: PrincipledMaterial {
                baseColor: "#4080ff"
            }
        }
    }
}

// 需要时才加载
Button {
    text: "加载模型"
    onClicked: {
        modelLoader.active = true
    }
}
```

### 7.2 资源卸载

**及时释放不用的资源**

```qml
Loader3D {
    id: dynamicLoader
    active: true
    source: "MyModel.qml"
}

// 卸载资源
Button {
    text: "卸载"
    onClicked: {
        dynamicLoader.active = false  // 释放资源
    }
}
```

### 7.3 纹理内存管理

**监控纹理内存占用**

```qml
QtObject {
    id: memoryMonitor
    
    property real textureMemory: 0  // MB
    
    function calculateTextureMemory() {
        // 估算纹理内存
        // 2048×2048 RGBA = 16MB
        // 1024×1024 RGBA = 4MB
        // 512×512 RGBA = 1MB
    }
}
```

**优化策略**：
- 使用纹理压缩
- 动态加载/卸载纹理
- 使用纹理流送（Texture Streaming）
- 限制纹理总大小

---

## 8. 渲染管线优化

### 8.1 抗锯齿优化

**抗锯齿性能对比**：

| 模式 | 性能开销 | 质量 | 说明 |
|------|----------|------|------|
| NoAA | 0% | 低 | 无抗锯齿 |
| MSAA | 20-40% | 高 | 多重采样 |
| SSAA | 100-300% | 最高 | 超采样 |
| ProgressiveAA | 可变 | 高 | 渐进式 |

```qml
SceneEnvironment {
    // ✅ 移动端：关闭或使用低质量
    antialiasingMode: SceneEnvironment.NoAA
    
    // ✅ 桌面端：MSAA 中等质量
    antialiasingMode: SceneEnvironment.MSAA
    antialiasingQuality: SceneEnvironment.Medium
    
    // ❌ 避免：SSAA（性能杀手）
    // antialiasingMode: SceneEnvironment.SSAA
}
```

### 8.2 环境光遮蔽（AO）

**AO 性能开销较大**

```qml
SceneEnvironment {
    // AO 设置
    aoEnabled: true
    aoStrength: 50      // ✅ 降低强度
    aoDistance: 5.0
    aoSoftness: 50.0
    aoSampleRate: 2     // ✅ 降低采样率（2/4）
    aoDither: true
}
```

### 8.3 后处理效果

**谨慎使用后处理**

```qml
SceneEnvironment {
    // ✅ 基础设置
    clearColor: "#1a1a1a"
    backgroundMode: SceneEnvironment.Color
    
    // ✅ 色调映射（开销低）
    tonemapMode: SceneEnvironment.TonemapModeLinear
    
    // ⚠️ 雾效（中等开销）
    fogEnabled: false  // 不需要时关闭
    
    // ⚠️ 时间抗锯齿（高开销）
    temporalAAEnabled: false  // 静态场景可用
}
```



---

## 9. CPU 与 GPU 协同优化

### 9.1 避免 CPU-GPU 同步

**同步点会导致性能下降**

```qml
// ❌ 差：频繁读取 GPU 数据
Timer {
    interval: 16
    running: true
    repeat: true
    onTriggered: {
        // 读取渲染结果（强制同步）
        let result = view3D.pick(100, 100)
        // CPU 等待 GPU 完成渲染
    }
}

// ✅ 好：减少同步频率
Timer {
    interval: 100  // 降低频率
    running: true
    repeat: true
    onTriggered: {
        let result = view3D.pick(100, 100)
    }
}
```

### 9.2 异步更新

**使用双缓冲避免阻塞**

```qml
QtObject {
    id: dataBuffer
    
    // 双缓冲
    property var frontBuffer: []
    property var backBuffer: []
    
    function update() {
        // 在后台缓冲区准备数据
        backBuffer = generateNewData()
    }
    
    function swap() {
        // 交换缓冲区（快速操作）
        let temp = frontBuffer
        frontBuffer = backBuffer
        backBuffer = temp
    }
}
```

### 9.3 批量更新

**减少更新频率**

```qml
// ❌ 差：每帧更新所有对象
Timer {
    interval: 16
    running: true
    repeat: true
    onTriggered: {
        for (let i = 0; i < 1000; i++) {
            objects[i].position = calculatePosition(i)
        }
    }
}

// ✅ 好：分批更新
Timer {
    interval: 16
    running: true
    repeat: true
    
    property int batchIndex: 0
    property int batchSize: 100
    
    onTriggered: {
        // 每帧只更新 100 个对象
        let start = batchIndex * batchSize
        let end = Math.min(start + batchSize, 1000)
        
        for (let i = start; i < end; i++) {
            objects[i].position = calculatePosition(i)
        }
        
        batchIndex = (batchIndex + 1) % 10
    }
}
```

---

## 10. 高级技巧与最佳实践

### 10.1 性能分级系统

**根据设备性能调整质量**

```qml
import QtQuick
import QtQuick3D

Window {
    id: root
    
    // 性能等级
    enum PerformanceLevel {
        Low,
        Medium,
        High,
        Ultra
    }
    
    property int performanceLevel: detectPerformanceLevel()
    
    function detectPerformanceLevel() {
        // 简单的性能检测
        // 实际应用中可以运行基准测试
        
        // 检测 GPU
        // 检测内存
        // 检测 CPU
        
        return PerformanceLevel.Medium
    }
    
    View3D {
        anchors.fill: parent
        
        environment: SceneEnvironment {
            clearColor: "#1a1a1a"
            backgroundMode: SceneEnvironment.Color
            
            // 根据性能等级调整设置
            antialiasingMode: {
                switch(root.performanceLevel) {
                    case PerformanceLevel.Low:
                        return SceneEnvironment.NoAA
                    case PerformanceLevel.Medium:
                        return SceneEnvironment.MSAA
                    case PerformanceLevel.High:
                    case PerformanceLevel.Ultra:
                        return SceneEnvironment.MSAA
                }
            }
            
            antialiasingQuality: {
                switch(root.performanceLevel) {
                    case PerformanceLevel.Low:
                    case PerformanceLevel.Medium:
                        return SceneEnvironment.Medium
                    case PerformanceLevel.High:
                        return SceneEnvironment.High
                    case PerformanceLevel.Ultra:
                        return SceneEnvironment.VeryHigh
                }
            }
            
            aoEnabled: root.performanceLevel >= PerformanceLevel.High
        }
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 100, 300)
        }
        
        DirectionalLight {
            eulerRotation.x: -30
            castsShadow: root.performanceLevel >= PerformanceLevel.Medium
            
            shadowMapQuality: {
                switch(root.performanceLevel) {
                    case PerformanceLevel.Low:
                        return Light.ShadowMapQualityLow
                    case PerformanceLevel.Medium:
                        return Light.ShadowMapQualityMedium
                    case PerformanceLevel.High:
                        return Light.ShadowMapQualityHigh
                    case PerformanceLevel.Ultra:
                        return Light.ShadowMapQualityVeryHigh
                }
            }
        }
    }
}
```



### 10.2 性能优化检查清单

**渲染优化**：
- [ ] 使用实例化渲染（Instancing）
- [ ] 共享材质减少 Draw Calls
- [ ] 实现 LOD 系统
- [ ] 启用视锥剔除
- [ ] 合理设置相机裁剪平面

**几何体优化**：
- [ ] 控制三角形数量（<100K/帧）
- [ ] 使用简化模型
- [ ] 移除不可见面
- [ ] 合并静态网格

**材质优化**：
- [ ] 选择合适的材质类型
- [ ] 简化着色器
- [ ] 减少纹理采样
- [ ] 避免透明材质过度使用

**纹理优化**：
- [ ] 使用合理的纹理尺寸
- [ ] 启用纹理压缩
- [ ] 生成 Mipmap
- [ ] 使用纹理图集

**光照优化**：
- [ ] 限制光源数量（<5）
- [ ] 降低阴影质量
- [ ] 减小阴影范围
- [ ] 使用烘焙光照

**内存优化**：
- [ ] 延迟加载资源
- [ ] 及时卸载不用的资源
- [ ] 监控内存占用
- [ ] 使用对象池

### 10.3 性能目标

**不同平台的性能目标**：

| 平台 | FPS 目标 | Draw Calls | 三角形数 | 纹理内存 |
|------|----------|------------|----------|----------|
| 高端桌面 | 60+ | <200 | <200K | <1GB |
| 中端桌面 | 60 | <100 | <100K | <512MB |
| 低端桌面 | 30-60 | <50 | <50K | <256MB |
| 高端移动 | 60 | <50 | <50K | <256MB |
| 中端移动 | 30-60 | <30 | <30K | <128MB |
| 低端移动 | 30 | <20 | <20K | <64MB |

### 10.4 调试技巧

**环境变量**：
```bash
# 显示渲染时间
QSG_RENDER_TIMING=1

# 可视化批次
QSG_VISUALIZE=batches

# 可视化过度绘制
QSG_VISUALIZE=overdraw

# 可视化变化
QSG_VISUALIZE=changes

# 显示 FPS
QSG_RENDER_LOOP=basic
```

**性能分析工具**：
- Qt Creator Profiler
- RenderDoc（图形调试）
- Nsight Graphics（NVIDIA）
- PIX（DirectX）
- Xcode Instruments（macOS/iOS）

### 10.5 常见性能问题

**问题 1：FPS 不稳定**
- 原因：垃圾回收、资源加载
- 解决：预加载资源、对象池

**问题 2：启动慢**
- 原因：同步加载大量资源
- 解决：异步加载、延迟加载

**问题 3：内存持续增长**
- 原因：资源泄漏
- 解决：及时释放、使用 Loader3D

**问题 4：Draw Calls 过多**
- 原因：对象过多、材质不共享
- 解决：实例化、材质合并

**问题 5：GPU 占用高**
- 原因：着色器复杂、分辨率高
- 解决：简化着色器、降低分辨率

---

## 总结

### 核心优化原则

1. **减少 Draw Calls**
   - 使用实例化渲染
   - 共享材质
   - 合并网格

2. **降低几何复杂度**
   - 实现 LOD
   - 视锥剔除
   - 遮挡剔除

3. **优化材质和着色器**
   - 选择合适的材质类型
   - 简化着色器逻辑
   - 减少纹理采样

4. **管理纹理内存**
   - 合理的纹理尺寸
   - 使用压缩格式
   - 启用 Mipmap

5. **优化光照和阴影**
   - 限制光源数量
   - 降低阴影质量
   - 使用烘焙光照

6. **内存管理**
   - 延迟加载
   - 及时释放
   - 对象池

### 性能优化流程

1. **测量**：使用性能分析工具找出瓶颈
2. **优化**：针对瓶颈进行优化
3. **验证**：测试优化效果
4. **迭代**：重复上述过程

### 最后的建议

- **过早优化是万恶之源**：先实现功能，再优化性能
- **测量比猜测重要**：使用工具而不是凭感觉
- **平衡质量和性能**：找到最佳平衡点
- **针对目标平台优化**：不同平台有不同的优化策略

记住：**性能优化是一个持续的过程，需要不断测试和调整**。

---

## 参考资源

- **Qt 官方文档**：https://doc.qt.io/qt-6/qtquick3d-index.html
- **Qt 性能优化指南**：https://doc.qt.io/qt-6/qtquick3d-tool-balsam.html
- **图形编程最佳实践**：GPU Gems 系列
- **实时渲染**：Real-Time Rendering 4th Edition

祝你的 3D 应用性能优异！🚀
