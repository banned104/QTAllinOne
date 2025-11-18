---
title: Post Processing
tags:
- QtQuick3D
- Effects
- 中文
---

# Post Processing

## 是什么

Post Processing（后处理）是指在3D场景渲染完成后，对渲染结果进行的图像处理操作。Qt Quick 3D 提供了完整的后处理管线，支持多种内置效果和自定义效果。

```mermaid
graph LR
    A[Scene Render] --> B[Effect 1]
    B --> C[Effect 2]
    C --> D[Effect N]
    D --> E[Final Image]
```

## 常用属性一览表

### SceneEnvironment 后处理属性

| 属性名 | 类型 | 默认值 | 取值范围 | 作用 | 性能/质量提示 |
| ------ | ---- | ------ | -------- | ---- | -------------- |
| effects★ | list | [] | Effect列表 | 后处理效果链 | 按顺序执行 |

★ 标记表示高频使用属性

## 最小可运行示例

main.qml:
```qml
import QtQuick
import QtQuick3D
import QtQuick3D.Effects

Window {
    width: 1280
    height: 720
    visible: true

    View3D {
        anchors.fill: parent
        
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#1a1a2e"
            
            effects: [
                Bloom { bloomIntensity: 0.8 },
                Vignette { vignetteRadius: 0.7 }
            ]
        }
        
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 300)
        }
        
        DirectionalLight { }
        
        Model {
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColor: "#e74c3c"
                emissiveFactor: Qt.vector3d(1, 0, 0)
            }
        }
    }
}
```

## 延伸阅读

- [Post-Processing-Effects.md](./Post-Processing-Effects.md)
- [Scene-Rendering.md](./Scene-Rendering.md)
