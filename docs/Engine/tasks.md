# 实施任务列表

本任务列表将设计文档转化为可执行的编码步骤。每个任务都是独立的、可测试的，并且按照依赖关系排序。

## 任务概览

整个重构分为 4 个主要阶段：
1. 创建抽象层和基础设施
2. 重构 TriangleRender 类
3. 重构 OpenGLItem 类
4. 测试和清理

---

## 阶段 1：创建抽象层和基础设施

- [ ] 1. 创建核心接口和数据类
  - 创建 `IRenderer.h` 接口文件，定义渲染器标准接口
  - 定义 `RenderError` 枚举和回调函数类型
  - 包含所有纯虚方法：`initialize()`, `render()`, `resize()`, `cleanup()`, `setErrorCallback()`, `getName()`
  - _需求: 1.1, 1.2, 1.3, 1.5_

- [ ] 2. 创建 RenderContext 类
  - 创建 `RenderContext.h` 和 `RenderContext.cpp` 文件
  - 实现封装视口尺寸、投影矩阵、时间信息的值对象
  - 实现不可变模式的 `withFrameNumber()` 和 `withDeltaTime()` 方法
  - 提供所有必要的 getter 方法
  - _需求: 3.1, 3.2, 3.4_

- [ ] 3. 创建 RenderConfig 类
  - 创建 `RenderConfig.h` 文件
  - 实现 Builder 模式的配置类，支持链式调用
  - 实现 `createTriangleConfig()` 静态工厂方法
  - 包含着色器路径、顶点数据、清除颜色、旋转速度等配置项
  - 将 `VertexData` 结构体移到此文件中
  - _需求: 7.1, 7.2, 7.3_

- [ ] 4. 创建 RendererFactory 类
  - 创建 `RendererFactory.h` 文件
  - 定义 `RendererType` 枚举
  - 实现静态工厂方法 `create(RendererType)` 和 `create(string)`
  - 返回 `std::unique_ptr<IRenderer>`
  - _需求: 2.1, 2.2, 2.5_

---

## 阶段 2：重构 TriangleRender 类

- [ ] 5. 修改 TriangleRender 头文件
  - 修改 `trianglerender.h`，移除 `QObject` 继承
  - 添加 `IRenderer` 接口继承
  - 移除 `Q_OBJECT` 宏和 `slots` 声明
  - 添加 `override` 关键字到所有接口方法
  - 添加私有成员：`m_errorCallback`, `m_initialized`, `m_clearColor`, `m_rotationSpeed`, `m_currentAngle`
  - 添加私有方法：`reportError()`
  - _需求: 1.2, 1.3, 5.1_

- [ ] 6. 重构 TriangleRender 实现文件
  - 修改 `trianglerender.cpp`
  - 重构 `initialize()` 方法以接受 `RenderConfig` 参数
  - 重构 `render()` 方法以接受 `RenderContext` 参数
  - 实现 `setErrorCallback()` 方法
  - 实现 `getName()` 方法返回 "TriangleRender"
  - 修改 `initializeShader()` 接受路径参数
  - 修改 `initializeTriangle()` 接受顶点数据参数
  - 实现 `reportError()` 方法调用错误回调
  - 从 `RenderContext` 获取投影矩阵和视口信息
  - 从配置中读取清除颜色和旋转速度
  - 移除所有 `qDebug()` 调用，改用 `reportError()`
  - _需求: 1.2, 3.2, 3.4, 5.1, 7.3, 8.1, 8.3_

- [ ]* 6.1 为 TriangleRender 编写单元测试
  - 创建 `test_trianglerender.cpp` 测试文件
  - 测试初始化成功和失败场景
  - 测试错误回调机制
  - 测试渲染方法调用
  - _需求: 8.3, 8.4_

---

## 阶段 3：重构 OpenGLItem 类

- [ ] 7. 修改 OpenGLItem 头文件
  - 修改 `openglItem.h`
  - 添加 `#include "IRenderer.h"`, `#include "RenderContext.h"`, `#include "RenderConfig.h"`
  - 将 `TriangleRender *m_triangleRender` 改为 `std::unique_ptr<IRenderer> m_renderer`
  - 添加成员变量：`RenderConfig m_config`, `QMatrix4x4 m_projectionMatrix`, `quint64 m_frameNumber`, `bool m_rendererInitialized`
  - 添加 Q_PROPERTY：`QString rendererType`
  - 添加公共方法：`setRenderer()`, `setRenderConfig()`
  - 添加信号：`renderError(const QString&)`, `rendererTypeChanged()`
  - 添加私有方法：`createRenderer()`, `initializeRenderer()`, `updateProjectionMatrix()`, `handleRenderError()`
  - _需求: 1.3, 2.3, 4.1, 6.1, 6.2_

- [ ] 8. 重构 OpenGLItem 实现文件
  - 修改 `openglItem.cpp`
  - 添加 `#include "RendererFactory.h"`
  - 实现 `createRenderer()` 方法使用工厂创建渲染器
  - 实现 `initializeRenderer()` 方法设置回调并初始化渲染器
  - 实现 `setRenderer()` 方法支持依赖注入
  - 实现 `setRenderConfig()` 方法
  - 实现 `handleRenderError()` 方法处理渲染错误
  - 实现 `updateProjectionMatrix()` 方法计算投影矩阵
  - 修改 `sync()` 方法使用新的初始化流程
  - 修改渲染循环创建 `RenderContext` 并传递给渲染器
  - 修改 `cleanup()` 方法调用渲染器的 `cleanup()` 并重置智能指针
  - 修改窗口尺寸变化处理，调用 `updateProjectionMatrix()` 和 `renderer->resize()`
  - 移除直接的 `new TriangleRender()` 调用
  - _需求: 2.3, 3.2, 3.3, 4.1, 4.2, 6.1, 8.4_

- [ ] 9. 实现渲染器类型切换功能
  - 在 `OpenGLItem` 中实现 `setRendererType()` 方法
  - 支持通过字符串切换渲染器类型
  - 在类型改变时重新创建渲染器
  - 发射 `rendererTypeChanged()` 信号
  - _需求: 1.4, 2.2_

- [ ]* 9.1 为 OpenGLItem 编写单元测试
  - 创建 `MockRenderer` 类用于测试
  - 创建 `test_openglitem.cpp` 测试文件
  - 测试依赖注入功能
  - 测试渲染器创建和初始化
  - 测试错误处理
  - 测试渲染循环调用
  - _需求: 6.3, 8.4_

---

## 阶段 4：集成和验证

- [ ] 10. 更新项目构建配置
  - 确保所有新文件添加到 CMakeLists.txt 或 .pro 文件中
  - 添加必要的 include 路径
  - 确保 C++11 或更高版本标准启用（用于智能指针和 lambda）
  - _需求: 4.1_

- [ ] 11. 创建默认配置和初始化代码
  - 在 `OpenGLItem` 构造函数中设置默认配置
  - 使用 `RenderConfig::createTriangleConfig()` 创建默认三角形配置
  - 设置默认渲染器类型为 "triangle"
  - _需求: 7.4_

- [ ] 12. 验证编译和运行
  - 编译整个项目，修复所有编译错误
  - 运行应用程序，验证三角形正常渲染
  - 验证旋转动画正常工作
  - 验证 FPS 计算正常
  - 验证窗口尺寸调整正常工作
  - _需求: 所有需求_

- [ ]* 12.1 编写集成测试
  - 创建 `test_integration.cpp` 测试文件
  - 测试 OpenGLItem 与真实 TriangleRender 的集成
  - 测试完整的渲染流程
  - 测试错误场景的端到端处理
  - _需求: 8.4, 8.5_

- [ ] 13. 性能验证和优化
  - 使用性能分析工具测量渲染性能
  - 对比重构前后的性能差异
  - 确保虚函数调用和智能指针没有显著性能影响
  - 如有必要，进行针对性优化
  - _需求: 所有需求_

- [ ] 14. 文档更新
  - 更新代码注释，说明新的架构
  - 添加使用示例到 README
  - 记录如何添加新的渲染器类型
  - 记录如何在 QML 中使用新的属性
  - _需求: 1.4, 2.4_

---

## 可选扩展任务

- [ ]* 15. 添加示例：创建新的渲染器
  - 创建 `CubeRender` 类作为示例
  - 实现 `IRenderer` 接口
  - 在 `RendererFactory` 中注册
  - 在 QML 中演示切换渲染器
  - _需求: 1.4, 2.4_

- [ ]* 16. 添加配置文件支持
  - 支持从 JSON 或 XML 文件加载渲染配置
  - 实现配置文件解析器
  - 允许在运行时重新加载配置
  - _需求: 7.5_

- [ ]* 17. 添加渲染统计信息
  - 在 `RenderContext` 中添加统计信息
  - 跟踪渲染时间、帧时间等
  - 通过回调或属性暴露给 UI 层
  - _需求: 3.5_

---

## 注意事项

### 依赖关系
- 任务 1-4 必须在任务 5-6 之前完成（创建接口后才能实现）
- 任务 5-6 必须在任务 7-9 之前完成（渲染器重构后才能修改使用方）
- 任务 10-11 应在任务 12 之前完成（构建配置和初始化）

### 测试策略
- 标记为 `*` 的测试任务是可选的，但强烈建议实施
- 每完成一个主要任务后，应进行编译验证
- 在阶段 4 之前不要删除旧代码，保持向后兼容

### 回滚策略
- 使用版本控制系统（Git）为每个阶段创建分支
- 每完成一个阶段后创建标签
- 如果遇到问题，可以回滚到上一个稳定状态

### 代码审查检查点
- 阶段 1 完成后：审查接口设计
- 阶段 2 完成后：审查 TriangleRender 重构
- 阶段 3 完成后：审查 OpenGLItem 重构
- 阶段 4 完成后：最终代码审查

## 预估工作量

- 阶段 1：2-3 小时（创建新文件和接口）
- 阶段 2：2-3 小时（重构 TriangleRender）
- 阶段 3：3-4 小时（重构 OpenGLItem）
- 阶段 4：2-3 小时（集成、测试、文档）

**总计：9-13 小时**（不包括可选任务）

如果包含所有测试任务，预估增加 4-6 小时。
