# Qt6 Quick 3D API 参考手册

> 本手册详细介绍 Qt6 Quick 3D 模块的所有核心 API，包括属性、方法、信号和完整的使用示例。

## 目录

### 核心组件
1. [View3D](#1-view3d)
2. [SceneEnvironment](#2-sceneenvironment)
3. [Node](#3-node)

### 相机
4. [Camera (基类)](#4-camera)
5. [PerspectiveCamera](#5-perspectivecamera)
6. [OrthographicCamera](#6-orthographiccamera)
7. [FrustumCamera](#7-frustumcamera)
8. [CustomCamera](#8-customcamera)

### 光源
9. [Light (基类)](#9-light)
10. [DirectionalLight](#10-directionallight)
11. [PointLight](#11-pointlight)
12. [SpotLight](#12-spotlight)

### 模型与几何
13. [Model](#13-model)
14. [Geometry](#14-geometry)
15. [Repeater3D](#15-repeater3d)
16. [Loader3D](#16-loader3d)
17. [InstanceList](#17-instancelist)

### 材质
18. [Material (基类)](#18-material)
19. [PrincipledMaterial](#19-principledmaterial)
20. [DefaultMaterial](#20-defaultmaterial)
21. [CustomMaterial](#21-custommaterial)
22. [SpecularGlossyMaterial](#22-specularglossymaterial)

### 纹理
23. [Texture](#23-texture)
24. [TextureInput](#24-textureinput)
25. [CubeMapTexture](#25-cubemaptexture)

### 粒子系统
26. [ParticleSystem3D](#26-particlesystem3d)
27. [ParticleEmitter3D](#27-particleemitter3d)
28. [SpriteParticle3D](#28-spriteparticle3d)
29. [ModelParticle3D](#29-modelparticle3d)
30. [Affector3D](#30-affector3d)

### 辅助组件
31. [Skeleton](#31-skeleton)
32. [Joint](#32-joint)
33. [MorphTarget](#33-morphtarget)
34. [BakedLightmap](#34-bakedlightmap)

---

## 1. View3D

**继承**: Item

**描述**: View3D 是 Qt Quick 3D 的主要容器，用于在 2D QML 场景中嵌入 3D 内容。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `camera` | Camera | null | 用于渲染场景的相机 |
| `environment` | SceneEnvironment | null | 场景环境配置 |
| `scene` | Node | null | 场景根节点 |
| `importScene` | Node | null | 导入的场景 |
| `renderMode` | enumeration | Offscreen | 渲染模式 |
| `renderStats` | RenderStats | null | 渲染统计信息（只读）|

#### renderMode 枚举值

```qml
View3D.Offscreen    // 离屏渲染（默认）
View3D.Underlay     // 作为底层渲染
View3D.Overlay      // 作为顶层渲染
View3D.Inline       // 内联渲染
```

### 方法

```qml
// 执行射线拾取
PickResult pick(real x, real y)

// 将屏幕坐标映射到 3D 位置
vector3d mapFrom3DScene(vector3d scenePos)

// 将 3D 位置映射到屏幕坐标
vector3d mapTo3DScene(vector3d viewPos)
```

### 完整示例

```qml
import QtQuick
import QtQuick3D

Window {
    width: 1280
    height: 720
    visible: true
    
    View3D {
        id: view3D
        anchors.fill: parent
        
        // ========== 环境配置 ==========
        environment: SceneEnvironment {
            clearColor: "#1a1a1a"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }
        
        // ========== 相机 ==========
        camera: mainCamera
        
        PerspectiveCamera {
            id: mainCamera
            position: Qt.vector3d(0, 100, 300)
            eulerRotation.x: -15
        }
        
        // ========== 光源 ==========
        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }
        
        // ========== 3D 内容 ==========
        Model {
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColor: "#4080ff"
            }
        }
        
        // ========== 鼠标拾取 ==========
        MouseArea {
            anchors.fill: parent
            
            onClicked: (mouse) => {
                // 执行射线拾取
                let result = view3D.pick(mouse.x, mouse.y)
                
                if (result.objectHit) {
                    console.log("拾取到对象:", result.objectHit)
                    console.log("拾取点:", result.position)
                    console.log("距离:", result.distance)
                    console.log("UV 坐标:", result.uvPosition)
                }
            }
        }
    }
}
```

---

## 2. SceneEnvironment

**继承**: Object3D

**描述**: 配置 3D 场景的环境设置，包括背景、抗锯齿、光照等。

### 属性

#### 背景设置

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `backgroundMode` | enumeration | Color | 背景模式 |
| `clearColor` | color | black | 清除颜色 |
| `lightProbe` | Texture | null | 环境光探针（IBL）|
| `probeExposure` | real | 1.0 | 探针曝光度 |
| `probeHorizon` | real | 0.0 | 探针地平线 |
| `probeOrientation` | vector3d | (0,0,0) | 探针方向 |

#### backgroundMode 枚举值

```qml
SceneEnvironment.Transparent  // 透明背景
SceneEnvironment.Color        // 纯色背景
SceneEnvironment.SkyBox       // 天空盒
```

#### 抗锯齿设置

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `antialiasingMode` | enumeration | NoAA | 抗锯齿模式 |
| `antialiasingQuality` | enumeration | High | 抗锯齿质量 |
| `temporalAAEnabled` | bool | false | 时间抗锯齿 |
| `temporalAAStrength` | real | 0.3 | 时间抗锯齿强度 |

#### antialiasingMode 枚举值

```qml
SceneEnvironment.NoAA           // 无抗锯齿
SceneEnvironment.SSAA           // 超采样抗锯齿
SceneEnvironment.MSAA           // 多重采样抗锯齿
SceneEnvironment.ProgressiveAA  // 渐进式抗锯齿
```

#### antialiasingQuality 枚举值

```qml
SceneEnvironment.Medium    // 中等质量
SceneEnvironment.High      // 高质量
SceneEnvironment.VeryHigh  // 非常高质量
```

#### 环境光遮蔽（AO）

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `aoEnabled` | bool | false | 启用 AO |
| `aoStrength` | real | 0.0 | AO 强度 (0-100) |
| `aoDistance` | real | 5.0 | AO 距离 |
| `aoSoftness` | real | 50.0 | AO 柔和度 |
| `aoDither` | bool | false | AO 抖动 |
| `aoSampleRate` | int | 2 | AO 采样率 |
| `aoBias` | real | 0.0 | AO 偏移 |

#### 色调映射

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `tonemapMode` | enumeration | Linear | 色调映射模式 |

#### tonemapMode 枚举值

```qml
SceneEnvironment.TonemapModeLinear      // 线性
SceneEnvironment.TonemapModeAces        // ACES
SceneEnvironment.TonemapModeHejlDawson  // Hejl-Dawson
SceneEnvironment.TonemapModeFilmic      // 电影
```

#### 雾效

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `fogEnabled` | bool | false | 启用雾效 |
| `fogColor` | color | white | 雾颜色 |
| `fogDensity` | real | 0.01 | 雾密度 |
| `fogStart` | real | 10.0 | 雾起始距离 |
| `fogEnd` | real | 1000.0 | 雾结束距离 |
| `fogDepthEnabled` | bool | true | 深度雾 |
| `fogHeightEnabled` | bool | false | 高度雾 |

#### 深度测试

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `depthTestEnabled` | bool | true | 启用深度测试 |
| `depthPrePassEnabled` | bool | false | 深度预处理 |

### 完整示例

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        // ========== 背景设置 ==========
        backgroundMode: SceneEnvironment.Color
        clearColor: "#1a1a1a"
        
        // 或使用天空盒
        // backgroundMode: SceneEnvironment.SkyBox
        // lightProbe: Texture {
        //     source: "maps/skybox.hdr"
        // }
        
        // ========== 抗锯齿 ==========
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
        
        // 时间抗锯齿（适合静态场景）
        temporalAAEnabled: false
        temporalAAStrength: 0.3
        
        // ========== 环境光遮蔽 ==========
        aoEnabled: true
        aoStrength: 50        // 0-100
        aoDistance: 5.0       // 影响范围
        aoSoftness: 50.0      // 柔和度
        aoDither: true        // 减少条带
        aoSampleRate: 4       // 采样率（越高越好但越慢）
        aoBias: 0.0           // 偏移
        
        // ========== 色调映射 ==========
        tonemapMode: SceneEnvironment.TonemapModeAces
        
        // ========== 雾效 ==========
        fogEnabled: true
        fogColor: "#808080"
        fogDensity: 0.01      // 密度
        fogStart: 100.0       // 起始距离
        fogEnd: 1000.0        // 结束距离
        fogDepthEnabled: true // 深度雾
        
        // ========== 深度设置 ==========
        depthTestEnabled: true
        depthPrePassEnabled: false  // 深度预处理（优化性能）
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 100, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    Model {
        source: "#Sphere"
        materials: PrincipledMaterial {
            baseColor: "#4080ff"
        }
    }
}
```

---

## 3. Node

**继承**: Object3D

**描述**: Node 是所有 3D 对象的基类，提供位置、旋转、缩放等基本变换功能。

### 属性

#### 变换属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `position` | vector3d | (0,0,0) | 位置 |
| `rotation` | quaternion | (1,0,0,0) | 旋转（四元数）|
| `eulerRotation` | vector3d | (0,0,0) | 欧拉角旋转（度）|
| `scale` | vector3d | (1,1,1) | 缩放 |
| `pivot` | vector3d | (0,0,0) | 轴心点 |
| `opacity` | real | 1.0 | 不透明度 |
| `visible` | bool | true | 可见性 |

#### 层次结构

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `parent` | Node | null | 父节点（只读）|

#### 方向向量

| 属性 | 类型 | 描述 |
|------|------|------|
| `forward` | vector3d | 前向向量（只读）|
| `up` | vector3d | 上向向量（只读）|
| `right` | vector3d | 右向向量（只读）|
| `scenePosition` | vector3d | 世界空间位置（只读）|
| `sceneRotation` | quaternion | 世界空间旋转（只读）|
| `sceneScale` | vector3d | 世界空间缩放（只读）|

### 方法

```qml
// 将局部坐标转换为场景坐标
vector3d mapPositionToScene(vector3d localPosition)

// 将场景坐标转换为局部坐标
vector3d mapPositionFromScene(vector3d scenePosition)

// 将局部方向转换为场景方向
vector3d mapDirectionToScene(vector3d localDirection)

// 将场景方向转换为局部方向
vector3d mapDirectionFromScene(vector3d sceneDirection)

// 旋转节点使其看向目标点
void lookAt(vector3d target)

// 旋转节点使其看向目标节点
void lookAt(Node target)
```

### 完整示例

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 500)
        eulerRotation.x: -20
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 父节点 ==========
    Node {
        id: parentNode
        position: Qt.vector3d(0, 0, 0)
        
        // 父节点旋转
        NumberAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 10000
            loops: Animation.Infinite
        }
        
        // ========== 子节点 1 ==========
        Model {
            source: "#Cube"
            position: Qt.vector3d(100, 0, 0)  // 相对于父节点
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            
            materials: PrincipledMaterial {
                baseColor: "#ff6b6b"
            }
            
            // 子节点自转
            NumberAnimation on eulerRotation.x {
                from: 0
                to: 360
                duration: 3000
                loops: Animation.Infinite
            }
        }
        
        // ========== 子节点 2 ==========
        Model {
            source: "#Sphere"
            position: Qt.vector3d(-100, 0, 0)
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            
            materials: PrincipledMaterial {
                baseColor: "#4ecdc4"
            }
        }
    }
    
    // ========== 独立节点（使用 lookAt）==========
    Model {
        id: lookAtModel
        source: "#Cone"
        position: Qt.vector3d(0, 100, 200)
        
        materials: PrincipledMaterial {
            baseColor: "#ffe66d"
        }
        
        // 始终看向父节点
        Component.onCompleted: {
            lookAt(parentNode)
        }
        
        // 更新 lookAt
        Connections {
            target: parentNode
            function onPositionChanged() {
                lookAtModel.lookAt(parentNode)
            }
        }
    }
    
    // ========== 坐标转换示例 ==========
    Model {
        id: transformModel
        source: "#Cylinder"
        position: Qt.vector3d(200, 0, 0)
        
        materials: PrincipledMaterial {
            baseColor: "#a8e6cf"
        }
        
        Component.onCompleted: {
            // 局部坐标转场景坐标
            let localPos = Qt.vector3d(0, 50, 0)
            let scenePos = mapPositionToScene(localPos)
            console.log("场景坐标:", scenePos)
            
            // 获取方向向量
            console.log("前向:", forward)
            console.log("上向:", up)
            console.log("右向:", right)
        }
    }
}
```



---

## 4. Camera

**继承**: Node

**描述**: Camera 是所有相机类型的基类，定义了相机的通用属性。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `frustumCullingEnabled` | bool | true | 启用视锥剔除 |
| `clipNear` | real | 10.0 | 近裁剪面 |
| `clipFar` | real | 10000.0 | 远裁剪面 |

### 方法

```qml
// 将屏幕坐标转换为 3D 射线方向
vector3d mapToViewport(vector3d scenePos)

// 将 3D 位置转换为屏幕坐标
vector3d mapFromViewport(vector3d viewportPos)
```

---

## 5. PerspectiveCamera

**继承**: Camera

**描述**: 透视相机，模拟人眼视角，具有透视效果（近大远小）。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `fieldOfView` | real | 60.0 | 视场角（度）|
| `fieldOfViewOrientation` | enumeration | Vertical | FOV 方向 |

#### fieldOfViewOrientation 枚举值

```qml
PerspectiveCamera.Vertical    // 垂直 FOV
PerspectiveCamera.Horizontal  // 水平 FOV
```

### 完整示例

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

Window {
    width: 1280
    height: 720
    visible: true
    title: "透视相机示例"
    
    View3D {
        id: view3D
        anchors.fill: parent
        
        environment: SceneEnvironment {
            clearColor: "#1a1a1a"
            backgroundMode: SceneEnvironment.Color
        }
        
        // ========== 透视相机 ==========
        PerspectiveCamera {
            id: camera
            
            // 位置
            position: Qt.vector3d(0, 100, 300)
            
            // 旋转
            eulerRotation.x: -15
            eulerRotation.y: 0
            
            // 视场角（FOV）
            fieldOfView: 60  // 30-120 度之间
            fieldOfViewOrientation: PerspectiveCamera.Vertical
            
            // 裁剪平面
            clipNear: 1      // 近裁剪面（太小会有深度精度问题）
            clipFar: 10000   // 远裁剪面
            
            // 视锥剔除
            frustumCullingEnabled: true
        }
        
        DirectionalLight {
            eulerRotation.x: -30
        }
        
        // 测试对象
        Repeater3D {
            model: 10
            
            Model {
                source: "#Cube"
                position: Qt.vector3d(0, 0, -index * 100)
                
                materials: PrincipledMaterial {
                    baseColor: Qt.hsla(index / 10, 1.0, 0.5, 1.0)
                }
            }
        }
    }
    
    // ========== FOV 控制 ==========
    Column {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        spacing: 10
        
        Rectangle {
            width: 250
            height: column.height + 20
            color: "#80000000"
            radius: 10
            
            Column {
                id: column
                anchors.centerIn: parent
                width: parent.width - 20
                spacing: 10
                
                Text {
                    text: "相机控制"
                    color: "white"
                    font.bold: true
                }
                
                Text {
                    text: "FOV: " + fovSlider.value.toFixed(0) + "°"
                    color: "white"
                }
                
                Slider {
                    id: fovSlider
                    width: parent.width
                    from: 30
                    to: 120
                    value: 60
                    onValueChanged: {
                        camera.fieldOfView = value
                    }
                }
                
                Text {
                    text: "近裁剪: " + nearSlider.value.toFixed(1)
                    color: "white"
                }
                
                Slider {
                    id: nearSlider
                    width: parent.width
                    from: 0.1
                    to: 100
                    value: 1
                    onValueChanged: {
                        camera.clipNear = value
                    }
                }
                
                Text {
                    text: "远裁剪: " + farSlider.value.toFixed(0)
                    color: "white"
                }
                
                Slider {
                    id: farSlider
                    width: parent.width
                    from: 100
                    to: 20000
                    value: 10000
                    onValueChanged: {
                        camera.clipFar = value
                    }
                }
            }
        }
    }
}
```

---

## 6. OrthographicCamera

**继承**: Camera

**描述**: 正交相机，没有透视效果，适合 CAD、建筑可视化等场景。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `horizontalMagnification` | real | 1.0 | 水平缩放 |
| `verticalMagnification` | real | 1.0 | 垂直缩放 |

### 完整示例

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    // ========== 正交相机 ==========
    OrthographicCamera {
        id: orthoCamera
        
        position: Qt.vector3d(0, 200, 200)
        eulerRotation.x: -45
        
        // 缩放（值越大，视野越大）
        horizontalMagnification: 2.0
        verticalMagnification: 2.0
        
        clipNear: 1
        clipFar: 1000
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // 测试对象（正交投影下没有透视效果）
    Repeater3D {
        model: 5
        
        Model {
            source: "#Cube"
            position: Qt.vector3d(index * 100 - 200, 0, 0)
            
            materials: PrincipledMaterial {
                baseColor: Qt.hsla(index / 5, 1.0, 0.5, 1.0)
            }
        }
    }
    
    // 缩放控制
    Slider {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        width: 300
        from: 0.5
        to: 5.0
        value: 2.0
        onValueChanged: {
            orthoCamera.horizontalMagnification = value
            orthoCamera.verticalMagnification = value
        }
    }
}
```

---

## 7. FrustumCamera

**继承**: Camera

**描述**: 自定义视锥相机，可以精确控制视锥的六个面。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `top` | real | 0.0 | 上边界 |
| `bottom` | real | 0.0 | 下边界 |
| `left` | real | 0.0 | 左边界 |
| `right` | real | 0.0 | 右边界 |

### 示例

```qml
FrustumCamera {
    position: Qt.vector3d(0, 0, 300)
    
    top: 100
    bottom: -100
    left: -100
    right: 100
    
    clipNear: 1
    clipFar: 1000
}
```

---

## 8. CustomCamera

**继承**: Camera

**描述**: 完全自定义的相机，可以设置自定义投影矩阵。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `projection` | matrix4x4 | identity | 投影矩阵 |

### 示例

```qml
CustomCamera {
    position: Qt.vector3d(0, 0, 300)
    
    // 设置自定义投影矩阵
    projection: Qt.matrix4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    )
}
```

---

## 9. Light

**继承**: Node

**描述**: Light 是所有光源的基类。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `color` | color | white | 光源颜色 |
| `ambientColor` | color | black | 环境光颜色 |
| `brightness` | real | 1.0 | 亮度 |
| `castsShadow` | bool | false | 投射阴影 |
| `shadowBias` | real | 0.0 | 阴影偏移 |
| `shadowFactor` | real | 5.0 | 阴影因子 |
| `shadowMapQuality` | enumeration | Medium | 阴影贴图质量 |
| `shadowMapFar` | real | 5000.0 | 阴影贴图远平面 |
| `shadowFilter` | real | 5.0 | 阴影过滤 |

#### shadowMapQuality 枚举值

```qml
Light.ShadowMapQualityLow      // 低质量 (256x256)
Light.ShadowMapQualityMedium   // 中等质量 (512x512)
Light.ShadowMapQualityHigh     // 高质量 (1024x1024)
Light.ShadowMapQualityVeryHigh // 非常高质量 (2048x2048)
```

---

## 10. DirectionalLight

**继承**: Light

**描述**: 方向光，模拟太阳光，光线平行。

### 属性

无额外属性（继承自 Light）

### 完整示例

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 500)
        eulerRotation.x: -20
    }
    
    // ========== 方向光 ==========
    DirectionalLight {
        id: sunLight
        
        // 光源方向（通过旋转控制）
        eulerRotation.x: -30  // 俯仰角
        eulerRotation.y: -70  // 偏航角
        
        // 颜色和亮度
        color: "#ffffff"      // 光源颜色
        brightness: 1.0       // 亮度（0.0-无穷大）
        
        // 环境光
        ambientColor: "#202020"  // 环境光颜色
        
        // 阴影设置
        castsShadow: true
        shadowMapQuality: Light.ShadowMapQualityHigh
        shadowBias: 0.01      // 防止阴影痤疮
        shadowFactor: 5.0     // 阴影柔和度
        shadowMapFar: 1000    // 阴影范围
        shadowFilter: 10.0    // 阴影过滤（PCF）
        
        // 动画演示
        SequentialAnimation on eulerRotation.y {
            running: animateSwitch.checked
            loops: Animation.Infinite
            NumberAnimation { from: -180; to: 180; duration: 10000 }
        }
    }
    
    // 测试对象
    Model {
        source: "#Sphere"
        position: Qt.vector3d(0, 50, 0)
        
        materials: PrincipledMaterial {
            baseColor: "#4080ff"
        }
        
        castsShadows: true  // 投射阴影
    }
    
    // 地面
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        scale: Qt.vector3d(10, 10, 1)
        
        materials: PrincipledMaterial {
            baseColor: "#404040"
        }
        
        receivesShadows: true  // 接收阴影
    }
    
    // 控制面板
    Column {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        spacing: 10
        
        Rectangle {
            width: 250
            height: column.height + 20
            color: "#80000000"
            radius: 10
            
            Column {
                id: column
                anchors.centerIn: parent
                width: parent.width - 20
                spacing: 10
                
                Text {
                    text: "方向光控制"
                    color: "white"
                    font.bold: true
                }
                
                // 亮度
                Text {
                    text: "亮度: " + brightnessSlider.value.toFixed(2)
                    color: "white"
                }
                Slider {
                    id: brightnessSlider
                    width: parent.width
                    from: 0
                    to: 3
                    value: 1.0
                    onValueChanged: sunLight.brightness = value
                }
                
                // 俯仰角
                Text {
                    text: "俯仰角: " + pitchSlider.value.toFixed(0) + "°"
                    color: "white"
                }
                Slider {
                    id: pitchSlider
                    width: parent.width
                    from: -90
                    to: 0
                    value: -30
                    onValueChanged: sunLight.eulerRotation.x = value
                }
                
                // 偏航角
                Text {
                    text: "偏航角: " + yawSlider.value.toFixed(0) + "°"
                    color: "white"
                }
                Slider {
                    id: yawSlider
                    width: parent.width
                    from: -180
                    to: 180
                    value: -70
                    onValueChanged: sunLight.eulerRotation.y = value
                }
                
                // 阴影质量
                Text {
                    text: "阴影质量"
                    color: "white"
                }
                ComboBox {
                    width: parent.width
                    model: ["低", "中", "高", "非常高"]
                    currentIndex: 2
                    onCurrentIndexChanged: {
                        switch(currentIndex) {
                            case 0: sunLight.shadowMapQuality = Light.ShadowMapQualityLow; break
                            case 1: sunLight.shadowMapQuality = Light.ShadowMapQualityMedium; break
                            case 2: sunLight.shadowMapQuality = Light.ShadowMapQualityHigh; break
                            case 3: sunLight.shadowMapQuality = Light.ShadowMapQualityVeryHigh; break
                        }
                    }
                }
                
                // 动画
                Switch {
                    id: animateSwitch
                    text: "自动旋转"
                }
            }
        }
    }
}
```

---

## 11. PointLight

**继承**: Light

**描述**: 点光源，从一个点向四周发光，有衰减效果。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `constantFade` | real | 1.0 | 常数衰减 |
| `linearFade` | real | 0.0 | 线性衰减 |
| `quadraticFade` | real | 1.0 | 二次衰减 |

### 衰减公式

```
衰减 = 1.0 / (constantFade + linearFade * distance + quadraticFade * distance²)
```

### 完整示例

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#0a0a0a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 500)
        eulerRotation.x: -20
    }
    
    // ========== 点光源 ==========
    PointLight {
        id: pointLight
        
        // 位置
        position: Qt.vector3d(0, 100, 0)
        
        // 颜色和亮度
        color: "#ff8800"
        brightness: 5.0  // 点光源通常需要更高的亮度
        
        // 衰减参数
        constantFade: 1.0      // 常数项
        linearFade: 0.0        // 线性项
        quadraticFade: 0.001   // 二次项（距离平方）
        
        // 阴影
        castsShadow: true
        shadowMapQuality: Light.ShadowMapQualityMedium
        
        // 移动动画
        SequentialAnimation on position.x {
            running: true
            loops: Animation.Infinite
            NumberAnimation { from: -200; to: 200; duration: 3000 }
            NumberAnimation { from: 200; to: -200; duration: 3000 }
        }
    }
    
    // 可视化光源位置
    Model {
        source: "#Sphere"
        position: pointLight.position
        scale: Qt.vector3d(0.2, 0.2, 0.2)
        
        materials: PrincipledMaterial {
            baseColor: pointLight.color
            lighting: PrincipledMaterial.NoLighting  // 自发光
        }
    }
    
    // 测试对象
    Repeater3D {
        model: 5
        
        Model {
            source: "#Cube"
            position: Qt.vector3d(index * 100 - 200, 0, 0)
            
            materials: PrincipledMaterial {
                baseColor: "#808080"
            }
            
            castsShadows: true
        }
    }
    
    // 地面
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        position: Qt.vector3d(0, -50, 0)
        scale: Qt.vector3d(20, 20, 1)
        
        materials: PrincipledMaterial {
            baseColor: "#2a2a2a"
        }
        
        receivesShadows: true
    }
}
```

---

## 12. SpotLight

**继承**: Light

**描述**: 聚光灯，锥形光束，类似手电筒或舞台灯光。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `coneAngle` | real | 40.0 | 锥角（度）|
| `innerConeAngle` | real | 30.0 | 内锥角（度）|
| `constantFade` | real | 1.0 | 常数衰减 |
| `linearFade` | real | 0.0 | 线性衰减 |
| `quadraticFade` | real | 1.0 | 二次衰减 |

### 完整示例

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#0a0a0a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 500)
        eulerRotation.x: -20
    }
    
    // ========== 聚光灯 ==========
    SpotLight {
        id: spotLight
        
        // 位置和方向
        position: Qt.vector3d(0, 200, 0)
        eulerRotation.x: -90  // 向下照射
        
        // 颜色和亮度
        color: "#00ff00"
        brightness: 10.0
        
        // 锥角设置
        coneAngle: 30         // 外锥角
        innerConeAngle: 20    // 内锥角（柔和边缘）
        
        // 衰减
        constantFade: 1.0
        linearFade: 0.0
        quadraticFade: 0.0005
        
        // 阴影
        castsShadow: true
        shadowMapQuality: Light.ShadowMapQualityHigh
        
        // 旋转动画
        SequentialAnimation on eulerRotation.y {
            running: true
            loops: Animation.Infinite
            NumberAnimation { from: 0; to: 360; duration: 5000 }
        }
    }
    
    // 可视化聚光灯
    Model {
        source: "#Cone"
        position: spotLight.position
        eulerRotation: spotLight.eulerRotation
        scale: Qt.vector3d(0.3, 0.5, 0.3)
        
        materials: PrincipledMaterial {
            baseColor: spotLight.color
            lighting: PrincipledMaterial.NoLighting
            opacity: 0.3
            alphaMode: PrincipledMaterial.Blend
        }
    }
    
    // 测试场景
    Repeater3D {
        model: 25
        
        Model {
            source: "#Cylinder"
            position: Qt.vector3d(
                (index % 5) * 100 - 200,
                0,
                Math.floor(index / 5) * 100 - 200
            )
            scale: Qt.vector3d(0.5, 1, 0.5)
            
            materials: PrincipledMaterial {
                baseColor: "#808080"
            }
            
            castsShadows: true
        }
    }
    
    // 地面
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        position: Qt.vector3d(0, -50, 0)
        scale: Qt.vector3d(20, 20, 1)
        
        materials: PrincipledMaterial {
            baseColor: "#2a2a2a"
        }
        
        receivesShadows: true
    }
    
    // 控制面板
    Column {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        spacing: 10
        
        Rectangle {
            width: 250
            height: column.height + 20
            color: "#80000000"
            radius: 10
            
            Column {
                id: column
                anchors.centerIn: parent
                width: parent.width - 20
                spacing: 10
                
                Text {
                    text: "聚光灯控制"
                    color: "white"
                    font.bold: true
                }
                
                Text {
                    text: "外锥角: " + coneSlider.value.toFixed(0) + "°"
                    color: "white"
                }
                Slider {
                    id: coneSlider
                    width: parent.width
                    from: 10
                    to: 90
                    value: 30
                    onValueChanged: spotLight.coneAngle = value
                }
                
                Text {
                    text: "内锥角: " + innerConeSlider.value.toFixed(0) + "°"
                    color: "white"
                }
                Slider {
                    id: innerConeSlider
                    width: parent.width
                    from: 5
                    to: coneSlider.value
                    value: 20
                    onValueChanged: spotLight.innerConeAngle = value
                }
                
                Text {
                    text: "亮度: " + brightnessSlider.value.toFixed(1)
                    color: "white"
                }
                Slider {
                    id: brightnessSlider
                    width: parent.width
                    from: 0
                    to: 20
                    value: 10
                    onValueChanged: spotLight.brightness = value
                }
            }
        }
    }
}
```



---

## 13. Model

**继承**: Node

**描述**: Model 是用于显示 3D 几何体的组件。

### 属性

#### 几何体

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `source` | url | null | 几何体源文件 |
| `geometry` | Geometry | null | 自定义几何体 |

#### 内置几何体

```qml
"#Cube"       // 立方体
"#Sphere"     // 球体
"#Cylinder"   // 圆柱体
"#Cone"       // 圆锥体
"#Rectangle"  // 矩形平面
```

#### 材质

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `materials` | list<Material> | [] | 材质列表 |

#### 阴影

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `castsShadows` | bool | true | 投射阴影 |
| `receivesShadows` | bool | true | 接收阴影 |

#### 拾取

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `pickable` | bool | true | 可拾取 |

#### 实例化

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `instancing` | Instancing | null | 实例化表 |
| `instanceRoot` | Node | null | 实例化根节点 |

#### 骨骼动画

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `skeleton` | Skeleton | null | 骨骼 |
| `inverseBindPoses` | list<matrix4x4> | [] | 逆绑定姿势 |

#### 变形目标

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `morphTargets` | list<MorphTarget> | [] | 变形目标列表 |

#### 边界

| 属性 | 类型 | 描述 |
|------|------|------|
| `bounds` | Bounds | 边界框（只读）|

### 完整示例

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 100, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
        castsShadow: true
    }
    
    // ========== 基础模型 ==========
    Model {
        id: basicModel
        
        // 使用内置几何体
        source: "#Sphere"
        
        // 位置、旋转、缩放
        position: Qt.vector3d(-100, 0, 0)
        eulerRotation.y: 45
        scale: Qt.vector3d(1, 1, 1)
        
        // 材质
        materials: PrincipledMaterial {
            baseColor: "#4080ff"
            metalness: 0.5
            roughness: 0.3
        }
        
        // 阴影
        castsShadows: true
        receivesShadows: true
        
        // 可拾取
        pickable: true
    }
    
    // ========== 多材质模型 ==========
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, 0, 0)
        
        // 多个材质（按子网格顺序应用）
        materials: [
            PrincipledMaterial {
                baseColor: "#ff6b6b"
            },
            PrincipledMaterial {
                baseColor: "#4ecdc4"
            }
        ]
    }
    
    // ========== 加载外部模型 ==========
    Model {
        source: "models/mymodel.mesh"
        position: Qt.vector3d(100, 0, 0)
        
        materials: PrincipledMaterial {
            baseColor: "#ffe66d"
            
            // 使用纹理
            baseColorMap: Texture {
                source: "textures/diffuse.jpg"
            }
        }
    }
    
    // ========== 地面（接收阴影）==========
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        position: Qt.vector3d(0, -50, 0)
        scale: Qt.vector3d(10, 10, 1)
        
        materials: PrincipledMaterial {
            baseColor: "#404040"
        }
        
        castsShadows: false    // 地面不投射阴影
        receivesShadows: true  // 但接收阴影
        pickable: false        // 不可拾取
    }
}
```

---

## 14. Geometry

**继承**: Object3D

**描述**: Geometry 用于定义自定义几何体。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `vertexData` | ArrayBuffer | null | 顶点数据 |
| `indexData` | ArrayBuffer | null | 索引数据 |
| `stride` | int | 0 | 顶点步长 |
| `primitiveType` | enumeration | Triangles | 图元类型 |

#### primitiveType 枚举值

```qml
Geometry.Points       // 点
Geometry.Lines        // 线
Geometry.LineStrip    // 线条
Geometry.Triangles    // 三角形
Geometry.TriangleStrip // 三角形条带
Geometry.TriangleFan  // 三角形扇
```

### 方法

```qml
// 添加顶点属性
void addAttribute(enumeration semantic, int offset, enumeration componentType)

// 设置顶点数据
void setVertexData(ArrayBuffer data)

// 设置索引数据
void setIndexData(ArrayBuffer data)

// 清除所有数据
void clear()
```

### 完整示例

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 0, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 自定义几何体 ==========
    Model {
        geometry: Geometry {
            id: customGeometry
            
            // 图元类型
            primitiveType: Geometry.Triangles
            
            Component.onCompleted: {
                // 创建三角形顶点数据
                // 格式：位置(x,y,z) + 法线(nx,ny,nz)
                let vertices = new Float32Array([
                    // 位置           // 法线
                    -50, -50, 0,     0, 0, 1,  // 顶点 0
                     50, -50, 0,     0, 0, 1,  // 顶点 1
                      0,  50, 0,     0, 0, 1   // 顶点 2
                ])
                
                // 设置顶点数据
                vertexData = vertices.buffer
                
                // 添加属性
                // 位置属性
                addAttribute(Geometry.Attribute.PositionSemantic,
                           0,  // 偏移
                           Geometry.Attribute.F32Type)
                
                // 法线属性
                addAttribute(Geometry.Attribute.NormalSemantic,
                           12,  // 偏移（3个float = 12字节）
                           Geometry.Attribute.F32Type)
                
                // 设置步长（6个float = 24字节）
                stride = 24
                
                // 索引数据（可选）
                let indices = new Uint16Array([0, 1, 2])
                indexData = indices.buffer
            }
        }
        
        materials: PrincipledMaterial {
            baseColor: "#4080ff"
        }
    }
}
```

---

## 15. Repeater3D

**继承**: Node

**描述**: Repeater3D 用于重复创建 3D 对象。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `model` | variant | 0 | 数据模型 |
| `delegate` | Component | null | 委托组件 |

### 完整示例

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 600)
        eulerRotation.x: -15
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 使用数字模型 ==========
    Repeater3D {
        model: 10
        
        Model {
            source: "#Cube"
            position: Qt.vector3d(index * 100 - 450, 0, 0)
            
            materials: PrincipledMaterial {
                baseColor: Qt.hsla(index / 10, 1.0, 0.5, 1.0)
            }
        }
    }
    
    // ========== 使用 ListModel ==========
    ListModel {
        id: dataModel
        ListElement { x: 0; y: 0; z: 0; color: "#ff6b6b" }
        ListElement { x: 100; y: 0; z: 0; color: "#4ecdc4" }
        ListElement { x: 200; y: 0; z: 0; color: "#ffe66d" }
    }
    
    Repeater3D {
        model: dataModel
        
        Model {
            source: "#Sphere"
            position: Qt.vector3d(model.x, model.y + 100, model.z)
            
            materials: PrincipledMaterial {
                baseColor: model.color
            }
        }
    }
    
    // ========== 网格布局 ==========
    Repeater3D {
        model: 25
        
        Model {
            source: "#Cylinder"
            position: Qt.vector3d(
                (index % 5) * 100 - 200,
                -100,
                Math.floor(index / 5) * 100 - 200
            )
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            
            materials: PrincipledMaterial {
                baseColor: Qt.hsla((index % 5) / 5, 1.0, 0.5, 1.0)
            }
        }
    }
}
```

---

## 16. Loader3D

**继承**: Node

**描述**: Loader3D 用于动态加载 3D 内容。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `source` | url | "" | QML 文件路径 |
| `sourceComponent` | Component | null | 源组件 |
| `active` | bool | true | 是否激活 |
| `asynchronous` | bool | false | 异步加载 |
| `item` | Object | null | 加载的项（只读）|
| `status` | enumeration | Null | 加载状态（只读）|
| `progress` | real | 0.0 | 加载进度（只读）|

### 完整示例

```qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 0, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    // ========== 动态加载器 ==========
    Loader3D {
        id: loader
        
        // 从文件加载
        // source: "MyModel.qml"
        
        // 或从组件加载
        sourceComponent: modelComponent
        
        // 激活状态
        active: true
        
        // 异步加载
        asynchronous: true
        
        // 监听状态
        onStatusChanged: {
            if (status === Loader.Ready) {
                console.log("加载完成")
            } else if (status === Loader.Error) {
                console.log("加载失败")
            }
        }
        
        // 监听进度
        onProgressChanged: {
            console.log("加载进度:", progress)
        }
    }
    
    // 定义组件
    Component {
        id: modelComponent
        
        Model {
            source: "#Sphere"
            
            materials: PrincipledMaterial {
                baseColor: "#4080ff"
            }
            
            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 5000
                loops: Animation.Infinite
            }
        }
    }
    
    // 控制按钮
    Column {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        spacing: 10
        
        Button {
            text: loader.active ? "卸载" : "加载"
            onClicked: {
                loader.active = !loader.active
            }
        }
        
        Button {
            text: "切换模型"
            onClicked: {
                loader.sourceComponent = (loader.sourceComponent === modelComponent) ?
                                        alternativeComponent : modelComponent
            }
        }
    }
    
    // 备选组件
    Component {
        id: alternativeComponent
        
        Model {
            source: "#Cube"
            
            materials: PrincipledMaterial {
                baseColor: "#ff6b6b"
            }
        }
    }
}
```

---

## 17. InstanceList

**继承**: Instancing

**描述**: InstanceList 用于实例化渲染，提高大量相同对象的渲染性能。

### 方法

```qml
// 添加实例
void append(object instance)

// 获取实例
object get(int index)

// 设置实例
void set(int index, object instance)

// 移除实例
void remove(int index, int count)
```

### 实例对象属性

```javascript
{
    position: Qt.vector3d(x, y, z),    // 位置
    scale: Qt.vector3d(sx, sy, sz),    // 缩放
    rotation: Qt.quaternion(...),      // 旋转（四元数）
    color: Qt.rgba(r, g, b, a),        // 颜色
    data: Qt.vector4d(...)             // 自定义数据
}
```

### 完整示例

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
        castsShadow: true
    }
    
    // ========== 实例化模型 ==========
    Model {
        source: "#Cube"
        
        // 使用实例化表
        instancing: InstanceList {
            id: instances
            
            Component.onCompleted: {
                // 创建 1000 个实例
                for (let i = 0; i < 1000; i++) {
                    let x = (Math.random() - 0.5) * 2000
                    let y = Math.random() * 200
                    let z = (Math.random() - 0.5) * 2000
                    
                    let scale = 0.5 + Math.random() * 0.5
                    
                    let hue = Math.random()
                    
                    // 添加实例
                    instances.append({
                        position: Qt.vector3d(x, y, z),
                        scale: Qt.vector3d(scale, scale, scale),
                        rotation: Qt.quaternion(1, 0, 0, 0),
                        color: Qt.hsla(hue, 1.0, 0.5, 1.0)
                    })
                }
            }
        }
        
        materials: PrincipledMaterial {
            baseColor: "#4080ff"
        }
        
        castsShadows: true
    }
    
    // 地面
    Model {
        source: "#Rectangle"
        eulerRotation.x: -90
        scale: Qt.vector3d(50, 50, 1)
        
        materials: PrincipledMaterial {
            baseColor: "#2a2a2a"
        }
        
        receivesShadows: true
    }
    
    // 性能信息
    Text {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
        color: "white"
        text: "实例化渲染：" + instances.instanceCount + " 个对象\n" +
              "绘制调用：1 次\n" +
              "性能：优秀"
        font.pixelSize: 14
    }
}
```

---

## 18. Material

**继承**: Object3D

**描述**: Material 是所有材质的基类。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `lighting` | enumeration | FragmentLighting | 光照模式 |
| `cullMode` | enumeration | BackFaceCulling | 剔除模式 |

#### lighting 枚举值

```qml
Material.NoLighting          // 无光照（自发光）
Material.FragmentLighting    // 片段光照
```

#### cullMode 枚举值

```qml
Material.BackFaceCulling     // 背面剔除
Material.FrontFaceCulling    // 正面剔除
Material.NoCulling           // 不剔除
```

---

## 19. PrincipledMaterial

**继承**: Material

**描述**: 基于物理的渲染（PBR）材质，提供真实感渲染。

### 基础属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `baseColor` | color | white | 基础颜色 |
| `metalness` | real | 0.0 | 金属度 (0-1) |
| `roughness` | real | 0.0 | 粗糙度 (0-1) |
| `opacity` | real | 1.0 | 不透明度 (0-1) |

### 纹理贴图

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `baseColorMap` | Texture | null | 基础颜色贴图 |
| `metalnessMap` | Texture | null | 金属度贴图 |
| `roughnessMap` | Texture | null | 粗糙度贴图 |
| `normalMap` | Texture | null | 法线贴图 |
| `occlusionMap` | Texture | null | 环境光遮蔽贴图 |
| `emissiveMap` | Texture | null | 自发光贴图 |
| `heightMap` | Texture | null | 高度贴图 |

### 通道选择

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `metalnessChannel` | enumeration | B | 金属度通道 |
| `roughnessChannel` | enumeration | G | 粗糙度通道 |
| `occlusionChannel` | enumeration | R | AO 通道 |
| `heightChannel` | enumeration | R | 高度通道 |

#### 通道枚举值

```qml
Material.R  // 红色通道
Material.G  // 绿色通道
Material.B  // 蓝色通道
Material.A  // Alpha 通道
```

### 高级属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `specularAmount` | real | 0.5 | 镜面反射量 |
| `specularTint` | real | 0.0 | 镜面色调 |
| `normalStrength` | real | 1.0 | 法线强度 |
| `occlusionAmount` | real | 1.0 | AO 强度 |
| `heightAmount` | real | 0.0 | 高度量 |
| `emissiveFactor` | vector3d | (0,0,0) | 自发光因子 |

### 透明度

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `alphaMode` | enumeration | Opaque | Alpha 模式 |
| `alphaCutoff` | real | 0.5 | Alpha 裁剪阈值 |

#### alphaMode 枚举值

```qml
PrincipledMaterial.Opaque  // 不透明
PrincipledMaterial.Mask    // Alpha 测试
PrincipledMaterial.Blend   // Alpha 混合
```

### 完整示例

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#1a1a1a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 0, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
        eulerRotation.y: -70
        brightness: 1.5
    }
    
    // ========== PBR 材质示例 ==========
    Model {
        source: "#Sphere"
        scale: Qt.vector3d(1.5, 1.5, 1.5)
        
        materials: PrincipledMaterial {
            // ========== 基础属性 ==========
            baseColor: "#4080ff"
            metalness: 0.8
            roughness: 0.2
            opacity: 1.0
            
            // ========== 纹理贴图 ==========
            baseColorMap: Texture {
                source: "textures/basecolor.jpg"
            }
            
            metalnessMap: Texture {
                source: "textures/metallic.jpg"
            }
            metalnessChannel: Material.R
            
            roughnessMap: Texture {
                source: "textures/roughness.jpg"
            }
            roughnessChannel: Material.G
            
            normalMap: Texture {
                source: "textures/normal.jpg"
            }
            normalStrength: 1.0
            
            occlusionMap: Texture {
                source: "textures/ao.jpg"
            }
            occlusionChannel: Material.R
            occlusionAmount: 1.0
            
            // ========== 高级属性 ==========
            specularAmount: 0.5
            specularTint: 0.0
            
            // ========== 自发光 ==========
            // emissiveMap: Texture {
            //     source: "textures/emissive.jpg"
            // }
            // emissiveFactor: Qt.vector3d(1, 1, 1)
            
            // ========== 透明度 ==========
            alphaMode: PrincipledMaterial.Opaque
            // alphaMode: PrincipledMaterial.Blend
            // opacity: 0.5
            
            // ========== 光照和剔除 ==========
            lighting: PrincipledMaterial.FragmentLighting
            cullMode: Material.BackFaceCulling
        }
        
        NumberAnimation on eulerRotation.y {
            from: 0
            to: 360
            duration: 10000
            loops: Animation.Infinite
        }
    }
}
```

---

## 20. DefaultMaterial

**继承**: Material

**描述**: 简单的材质系统，性能优于 PrincipledMaterial，但功能较少。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `diffuseColor` | color | white | 漫反射颜色 |
| `diffuseMap` | Texture | null | 漫反射贴图 |
| `specularTint` | color | white | 镜面反射色调 |
| `specularAmount` | real | 0.0 | 镜面反射量 |
| `specularRoughness` | real | 0.0 | 镜面粗糙度 |
| `specularMap` | Texture | null | 镜面反射贴图 |
| `emissiveColor` | color | black | 自发光颜色 |
| `emissiveMap` | Texture | null | 自发光贴图 |
| `opacity` | real | 1.0 | 不透明度 |
| `opacityMap` | Texture | null | 不透明度贴图 |
| `normalMap` | Texture | null | 法线贴图 |
| `bumpAmount` | real | 0.0 | 凹凸量 |

### 示例

```qml
Model {
    source: "#Sphere"
    
    materials: DefaultMaterial {
        diffuseColor: "#4080ff"
        specularTint: "#ffffff"
        specularAmount: 0.5
        specularRoughness: 0.3
        lighting: DefaultMaterial.FragmentLighting
    }
}
```

---

## 21. CustomMaterial

**继承**: Material

**描述**: 自定义材质，允许编写自定义着色器。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `vertexShader` | string | "" | 顶点着色器代码 |
| `fragmentShader` | string | "" | 片段着色器代码 |
| `shadingMode` | enumeration | Shaded | 着色模式 |
| `sourceBlend` | enumeration | NoBlend | 源混合模式 |
| `destinationBlend` | enumeration | NoBlend | 目标混合模式 |
| `depthDrawMode` | enumeration | OpaqueOnlyDepthDraw | 深度绘制模式 |

### 着色器内置变量

#### 顶点着色器

```glsl
// 输入
in vec3 VERTEX;           // 顶点位置
in vec3 NORMAL;           // 法线
in vec2 UV0;              // UV 坐标 0
in vec2 UV1;              // UV 坐标 1
in vec3 TANGENT;          // 切线
in vec3 BINORMAL;         // 副法线
in vec4 COLOR;            // 顶点颜色

// 输出
out vec4 POSITION;        // 裁剪空间位置

// Uniform
uniform mat4 MODEL_MATRIX;
uniform mat4 VIEW_MATRIX;
uniform mat4 PROJECTION_MATRIX;
uniform mat4 MODELVIEWPROJECTION_MATRIX;
uniform mat3 NORMAL_MATRIX;
uniform vec3 CAMERA_POSITION;
```

#### 片段着色器

```glsl
// 输出
out vec4 BASE_COLOR;      // 基础颜色
out vec4 EMISSIVE_COLOR;  // 自发光颜色
out float METALNESS;      // 金属度
out float ROUGHNESS;      // 粗糙度
out vec3 NORMAL;          // 法线
```

### 完整示例

```qml
import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    
    environment: SceneEnvironment {
        clearColor: "#0a0a0a"
        backgroundMode: SceneEnvironment.Color
    }
    
    PerspectiveCamera {
        position: Qt.vector3d(0, 0, 300)
    }
    
    DirectionalLight {
        eulerRotation.x: -30
    }
    
    Model {
        source: "#Sphere"
        scale: Qt.vector3d(1.5, 1.5, 1.5)
        
        materials: CustomMaterial {
            property real time: 0.0
            property color baseColor: "#4080ff"
            
            // 顶点着色器
            vertexShader: "
                varying vec3 vPosition;
                varying vec3 vNormal;
                
                uniform float time;
                
                void MAIN() {
                    vPosition = VERTEX.xyz;
                    vNormal = NORMAL;
                    
                    // 波浪变形
                    float wave = sin(VERTEX.x * 5.0 + time) * 
                                cos(VERTEX.z * 5.0 + time) * 0.1;
                    vec3 newPosition = VERTEX.xyz + NORMAL * wave;
                    
                    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(newPosition, 1.0);
                }
            "
            
            // 片段着色器
            fragmentShader: "
                varying vec3 vPosition;
                varying vec3 vNormal;
                
                uniform vec4 baseColor;
                uniform float time;
                
                void MAIN() {
                    vec3 normal = normalize(vNormal);
                    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
                    
                    float diff = max(dot(normal, lightDir), 0.0);
                    
                    vec3 color = baseColor.rgb * (0.3 + 0.7 * diff);
                    
                    BASE_COLOR = vec4(color, 1.0);
                }
            "
            
            shadingMode: CustomMaterial.Shaded
            
            NumberAnimation on time {
                from: 0
                to: 6.28318  // 2π
                duration: 3000
                loops: Animation.Infinite
            }
        }
    }
}
```

---

## 22. SpecularGlossyMaterial

**继承**: Material

**描述**: 镜面-光泽度材质模型（旧的 PBR 工作流）。

### 属性

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `albedoColor` | color | white | 反照率颜色 |
| `albedoMap` | Texture | null | 反照率贴图 |
| `specularColor` | color | white | 镜面反射颜色 |
| `specularMap` | Texture | null | 镜面反射贴图 |
| `glossiness` | real | 1.0 | 光泽度 |
| `glossinessMap` | Texture | null | 光泽度贴图 |
| `normalMap` | Texture | null | 法线贴图 |
| `occlusionMap` | Texture | null | AO 贴图 |
| `emissiveColor` | color | black | 自发光颜色 |
| `emissiveMap` | Texture | null | 自发光贴图 |
| `opacity` | real | 1.0 | 不透明度 |

### 示例

```qml
Model {
    source: "#Sphere"
    
    materials: SpecularGlossyMaterial {
        albedoColor: "#4080ff"
        specularColor: "#ffffff"
        glossiness: 0.8
        
        albedoMap: Texture {
            source: "textures/albedo.jpg"
        }
        
        specularMap: Texture {
            source: "textures/specular.jpg"
        }
    }
}
```

---

## 23. Texture

**继承**: Object3D

**描述**: 纹理对象，用于材质贴图。

### 属性

#### 纹理源

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `source` | url | "" | 纹理文件路径 |
| `sourceItem` | Item | null | QML Item 作为纹理源 |

#### 过滤模式

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `magFilter` | enumeration | Linear | 放大过滤 |
| `minFilter` | enumeration | Linear | 缩小过滤 |
| `mipFilter` | enumeration | None | Mipmap 过滤 |
| `generateMipmaps` | bool | false | 生成 Mipmap |

#### 过滤枚举值

```qml
Texture.Nearest  // 最近邻
Texture.Linear   // 线性
```

#### 环绕模式

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `tilingModeHorizontal` | enumeration | Repeat | 水平环绕 |
| `tilingModeVertical` | enumeration | Repeat | 垂直环绕 |

#### 环绕枚举值

```qml
Texture.ClampToEdge    // 边缘拉伸
Texture.Repeat         // 重复
Texture.MirroredRepeat // 镜像重复
```

#### UV 变换

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `scaleU` | real | 1.0 | U 方向缩放 |
| `scaleV` | real | 1.0 | V 方向缩放 |
| `positionU` | real | 0.0 | U 方向偏移 |
| `positionV` | real | 0.0 | V 方向偏移 |
| `rotationUV` | real | 0.0 | UV 旋转（度）|
| `pivotU` | real | 0.0 | U 轴心 |
| `pivotV` | real | 0.0 | V 轴心 |

#### 其他

| 属性 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| `mappingMode` | enumeration | UV | 映射模式 |
| `flipU` | bool | false | 水平翻转 |
| `flipV` | bool | false | 垂直翻转 |

### 完整示例

```qml
Model {
    source: "#Cube"
    
    materials: PrincipledMaterial {
        baseColorMap: Texture {
            // 纹理源
            source: "textures/wood.jpg"
            
            // 过滤模式
            magFilter: Texture.Linear
            minFilter: Texture.Linear
            mipFilter: Texture.Linear
            generateMipmaps: true
            
            // 环绕模式
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
            
            // UV 变换
            scaleU: 2.0      // 水平重复 2 次
            scaleV: 2.0      // 垂直重复 2 次
            positionU: 0.0   // 水平偏移
            positionV: 0.0   // 垂直偏移
            rotationUV: 0.0  // 旋转角度
            
            // 翻转
            flipU: false
            flipV: false
        }
    }
}
```

---

## 总结

本 API 参考手册涵盖了 Qt6 Quick 3D 的核心组件：

### 已完成的 API

1. ✅ **核心组件**: View3D, SceneEnvironment, Node
2. ✅ **相机**: Camera, PerspectiveCamera, OrthographicCamera, FrustumCamera, CustomCamera
3. ✅ **光源**: Light, DirectionalLight, PointLight, SpotLight
4. ✅ **模型**: Model, Geometry, Repeater3D, Loader3D, InstanceList
5. ✅ **材质**: Material, PrincipledMaterial, DefaultMaterial, CustomMaterial, SpecularGlossyMaterial
6. ✅ **纹理**: Texture

### 使用建议

- **初学者**: 从 View3D、PerspectiveCamera、DirectionalLight 和 PrincipledMaterial 开始
- **进阶**: 学习自定义几何体、CustomMaterial 和实例化渲染
- **性能优化**: 使用 InstanceList、LOD 和合理的阴影设置

### 参考资源

- **官方文档**: https://doc.qt.io/qt-6/qtquick3d-index.html
- **API 参考**: https://doc.qt.io/qt-6/qtquick3d-qmlmodule.html
- **示例代码**: Qt Creator → Examples → Qt Quick 3D

---

**注意**: 本手册持续更新中，更多 API（如粒子系统、骨骼动画等）将在后续版本中补充。

