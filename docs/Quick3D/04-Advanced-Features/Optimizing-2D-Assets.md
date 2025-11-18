---
title: Optimizing 2D Assets
tags:
- QtQuick3D
- Optimization
- 中文
---

# Optimizing 2D Assets

## 是什么

Optimizing 2D Assets（优化2D资源）是指优化在3D场景中使用的2D纹理、图像和UI元素，以提升性能和减少内存占用。

## 常用属性一览表

### Texture 优化属性

| 属性名 | 类型 | 默认值 | 取值范围 | 作用 | 性能/质量提示 |
| ------ | ---- | ------ | -------- | ---- | -------------- |
| generateMipmaps★ | bool | false | true/false | 生成mipmap | 改善远距离质量 |
| mipFilter | enum | None | None/Nearest/Linear | Mipmap过滤 | 影响质量 |
| magFilter | enum | Linear | Nearest/Linear | 放大过滤 | 影响近距离质量 |
| minFilter | enum | Linear | Nearest/Linear | 缩小过滤 | 影响远距离质量 |

★ 标记表示高频使用属性

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
            position: Qt.vector3d(0, 0, 300)
        }
        
        DirectionalLight { }
        
        Model {
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    source: "texture.png"
                    
                    // 优化设置
                    generateMipmaps: true
                    mipFilter: Texture.Linear
                    magFilter: Texture.Linear
                    minFilter: Texture.Linear
                }
            }
        }
    }
}
```

## 实战技巧

### 1. 纹理压缩

```qml
Texture {
    source: "texture.ktx"  // 使用压缩格式
    generateMipmaps: true
}
```

### 2. 纹理尺寸优化

```qml
// 使用2的幂次方尺寸
// 512x512, 1024x1024, 2048x2048
```

### 3. 纹理图集

```qml
// 将多个小纹理合并到一个大纹理
```

## 延伸阅读

- [Optimizing-3D-Assets.md](./Optimizing-3D-Assets.md)
- [Qt Quick 3D - Asset Conditioning](https://doc.qt.io/qt-6/quick3d-asset-conditioning.html)
