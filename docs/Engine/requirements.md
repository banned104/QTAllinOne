# 需求文档

## 简介

本项目是一个基于 Qt Quick 和 OpenGL 的三角形渲染应用。当前架构中，`TriangleRender` 类和 `OpenGLItem` 类存在紧密耦合问题：`OpenGLItem` 直接创建、管理和调用 `TriangleRender` 的实例，而 `TriangleRender` 的生命周期完全依赖于 `OpenGLItem`。这种双向依赖导致代码难以测试、扩展和维护。

本需求文档旨在定义一个解耦方案，通过引入抽象层和设计模式，降低两个类之间的耦合度，提高代码的可维护性、可测试性和可扩展性。

## 术语表

- **OpenGLItem**: Qt Quick 项，负责与 QML 场景图集成，管理渲染生命周期和用户交互
- **TriangleRender**: OpenGL 渲染器类，负责具体的 OpenGL 渲染逻辑（着色器、缓冲区、绘制调用）
- **Renderer Interface**: 抽象渲染器接口，定义渲染器的标准行为契约
- **Renderer Factory**: 渲染器工厂，负责创建具体的渲染器实例
- **Rendering Context**: 渲染上下文，封装渲染所需的环境信息（窗口尺寸、时间等）
- **Event System**: 事件系统，用于渲染器与视图层之间的松耦合通信

## 需求

### 需求 1：抽象渲染器接口

**用户故事：** 作为开发者，我希望定义一个抽象的渲染器接口，以便可以在不修改 OpenGLItem 的情况下替换不同的渲染实现

#### 验收标准

1. THE System SHALL 定义一个纯虚接口类 `IRenderer`，包含 `initialize()`、`render()`、`resize(int width, int height)` 和 `cleanup()` 方法
2. THE `TriangleRender` SHALL 实现 `IRenderer` 接口，而不是直接被 `OpenGLItem` 依赖
3. THE `OpenGLItem` SHALL 仅依赖于 `IRenderer` 接口，而不是具体的 `TriangleRender` 类
4. WHEN 需要添加新的渲染器时，THE System SHALL 允许创建新的 `IRenderer` 实现类，而无需修改 `OpenGLItem` 代码
5. THE `IRenderer` 接口 SHALL 不包含任何 Qt 特定的依赖（如 QObject），以保持接口的纯粹性

### 需求 2：工厂模式创建渲染器

**用户故事：** 作为开发者，我希望使用工厂模式来创建渲染器实例，以便将对象创建逻辑与使用逻辑分离

#### 验收标准

1. THE System SHALL 提供一个 `RendererFactory` 类，负责创建 `IRenderer` 实例
2. THE `RendererFactory` SHALL 支持通过枚举类型或字符串标识符来指定要创建的渲染器类型
3. THE `OpenGLItem` SHALL 通过 `RendererFactory` 获取渲染器实例，而不是直接使用 `new` 操作符
4. WHEN 添加新的渲染器类型时，THE `RendererFactory` SHALL 仅需要修改工厂类，而不影响 `OpenGLItem` 的代码
5. THE `RendererFactory` SHALL 返回 `IRenderer` 接口指针，而不是具体类型的指针

### 需求 3：渲染上下文封装

**用户故事：** 作为开发者，我希望将渲染所需的上下文信息封装到一个独立的类中，以便减少渲染器与外部环境的直接依赖

#### 验收标准

1. THE System SHALL 定义一个 `RenderContext` 类，封装窗口尺寸、投影矩阵、时间信息等渲染上下文数据
2. THE `IRenderer` 接口的 `render()` 方法 SHALL 接受 `RenderContext` 对象作为参数
3. THE `OpenGLItem` SHALL 负责创建和更新 `RenderContext` 对象，并将其传递给渲染器
4. THE `TriangleRender` SHALL 从 `RenderContext` 获取所需的上下文信息，而不是直接访问外部状态
5. WHEN 窗口尺寸改变时，THE `OpenGLItem` SHALL 更新 `RenderContext` 中的尺寸信息，并通知渲染器

### 需求 4：生命周期管理解耦

**用户故事：** 作为开发者，我希望使用智能指针管理渲染器的生命周期，以便避免手动内存管理和悬空指针问题

#### 验收标准

1. THE `OpenGLItem` SHALL 使用 `std::unique_ptr<IRenderer>` 来持有渲染器实例
2. WHEN `OpenGLItem` 被销毁时，THE System SHALL 自动释放渲染器资源
3. THE `cleanup()` 方法 SHALL 在渲染器被销毁前调用，以确保 OpenGL 资源正确释放
4. THE System SHALL 不使用裸指针来管理渲染器的生命周期
5. THE `RendererFactory` SHALL 返回 `std::unique_ptr<IRenderer>`，明确所有权转移

### 需求 5：信号与槽解耦

**用户故事：** 作为开发者，我希望渲染器不直接依赖 Qt 的信号槽机制，以便提高渲染器的可移植性和可测试性

#### 验收标准

1. THE `IRenderer` 接口 SHALL 不继承自 `QObject`，不使用 Qt 的信号槽机制
2. WHERE 渲染器需要向外部通知事件时，THE System SHALL 使用回调函数或观察者模式
3. THE `OpenGLItem` SHALL 通过设置回调函数来接收渲染器的事件通知
4. THE `TriangleRender` SHALL 通过回调函数报告渲染状态或错误，而不是发射信号
5. THE System SHALL 保持渲染器的纯 C++ 特性，不依赖 Qt 元对象系统

### 需求 6：依赖注入支持

**用户故事：** 作为开发者，我希望支持依赖注入，以便在单元测试中可以注入模拟的渲染器实现

#### 验收标准

1. THE `OpenGLItem` SHALL 提供一个 `setRenderer(std::unique_ptr<IRenderer> renderer)` 方法，允许外部注入渲染器
2. WHERE 未显式注入渲染器时，THE `OpenGLItem` SHALL 使用默认的工厂方法创建渲染器
3. THE System SHALL 允许在测试环境中注入模拟渲染器（Mock Renderer），而不执行真实的 OpenGL 调用
4. THE 注入的渲染器 SHALL 遵循与工厂创建的渲染器相同的生命周期管理规则
5. THE `OpenGLItem` SHALL 在接受注入的渲染器后，调用其 `initialize()` 方法

### 需求 7：配置与策略分离

**用户故事：** 作为开发者，我希望将渲染配置（如着色器路径、顶点数据）与渲染逻辑分离，以便更容易地修改渲染参数

#### 验收标准

1. THE System SHALL 定义一个 `RenderConfig` 类，封装着色器文件路径、顶点数据、渲染参数等配置信息
2. THE `IRenderer` 接口的 `initialize()` 方法 SHALL 接受 `RenderConfig` 对象作为参数
3. THE `TriangleRender` SHALL 从 `RenderConfig` 读取配置信息，而不是硬编码在实现中
4. THE `OpenGLItem` SHALL 负责创建和传递 `RenderConfig` 对象给渲染器
5. WHEN 需要修改渲染配置时，THE System SHALL 仅需要修改 `RenderConfig` 对象，而不需要修改渲染器代码

### 需求 8：错误处理标准化

**用户故事：** 作为开发者，我希望有统一的错误处理机制，以便更好地诊断和处理渲染过程中的错误

#### 验收标准

1. THE `IRenderer` 接口的方法 SHALL 返回布尔值或错误码，指示操作是否成功
2. THE System SHALL 定义一个 `RenderError` 枚举类型，表示不同类型的渲染错误
3. THE `TriangleRender` SHALL 在初始化或渲染失败时，返回相应的错误码
4. THE `OpenGLItem` SHALL 检查渲染器方法的返回值，并在错误发生时采取适当的处理措施
5. WHERE 渲染器需要提供详细的错误信息时，THE System SHALL 通过回调函数传递错误消息字符串
