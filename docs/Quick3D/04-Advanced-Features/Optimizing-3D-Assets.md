---
title: Optimizing 3D Assets
tags:
- QtQuick3D
- Optimization
- 中文
---

# Optimizing 3D Assets

## 是什么

Optimizing 3D Assets（优化3D资源）是指优化3D模型、纹理和材质，以提升渲染性能、减少内存占用和加快加载速度。

## 常用属性一览表

### 优化技术

| 技术 | 描述 | 性能提升 | 实施难度 |
| ---- | ---- | -------- | -------- |
| LOD★ | 细节层次 | 高 | 中 |
| 网格简化 | 减少顶点数 | 高 | 低 |
| 纹理压缩 | 压缩纹理格式 | 中 | 低 |
| 实例化 | 重用几何体 | 高 | 低 |
| 裁剪 | 视锥体裁剪 | 高 | 自动 |

★ 标记表示高效技术

## 最小可运行示例

main.qml:
```qml
import QtQuick
import QtQuick3D

Window {
    width: 1280
    height: 720
    visible: true

    View3D {
        anchors.fill: parent
        
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#1a1a2e"
        }
        
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 500)
        }
        
        DirectionalLight { }
        
        // LOD 系统
        Node {
            id: lodObject
            
            property real distanceToCamera: {
                let dx = position.x - camera.position.x
                let dy = position.y - camera.position.y
                let dz = position.z - camera.position.z
                return Math.sqrt(dx*dx + dy*dy + dz*dz)
            }
            
            // 高细节
            Model {
                source: "high_detail.mesh"
                visible: parent.distanceToCamera < 200
            }
            
            // 中细节
            Model {
                source: "medium_detail.mesh"
                visible: parent.distanceToCamera >= 200 && 
                         parent.distanceToCamera < 500
            }
            
            // 低细节
            Model {
                source: "low_detail.mesh"
                visible: parent.distanceToCamera >= 500
            }
        }
        
        // 实例化渲染
        Model {
            source: "#Cube"
            instancing: RandomInstancing {
                instanceCount: 1000
                position: InstanceRange {
                    from: Qt.vector3d(-500, -500, -500)
                    to: Qt.vector3d(500, 500, 500)
                }
            }
            materials: PrincipledMaterial {
                baseColor: "#3498db"
            }
        }
    }
}
```

## 实战技巧

### 1. 使用 Balsam 工具

```bash
# 转换和优化模型
balsam model.fbx --optimize
```

### 2. 网格简化

```qml
// 在3D软件中简化网格
// 目标：移动设备 < 10K 顶点
//      桌面设备 < 50K 顶点
```

### 3. 纹理优化

```qml
Texture {
    // 使用压缩格式
    source: "texture.ktx"
    
    // 生成 mipmap
    generateMipmaps: true
    
    // 合理的尺寸
    // 移动设备: 512x512 或 1024x1024
    // 桌面设备: 2048x2048 或 4096x4096
}
```

### 4. 材质优化

```qml
PrincipledMaterial {
    // 只使用必要的贴图
    baseColorMap: Texture { source: "diffuse.png" }
    // 避免不必要的贴图
    // normalMap: ...
    // roughnessMap: ...
}
```

### 5. 批处理优化

```qml
// 共享材质
property var sharedMaterial: PrincipledMaterial { }

Repeater3D {
    model: 100
    Model {
        materials: sharedMaterial  // 共享材质减少绘制调用
    }
}
```

## 延伸阅读

- [Optimizing-2D-Assets.md](./Optimizing-2D-Assets.md)
- [Instanced-Rendering.md](./Instanced-Rendering.md)
- [Balsam-Asset-Import-Tool.md](./Balsam-Asset-Import-Tool.md)
