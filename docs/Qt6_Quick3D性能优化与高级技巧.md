# Qt6 Quick 3D 性能优化与高级技巧

> 本文档深入探讨 Qt6 Quick 3D 的性能优化策略，从 GPU 架构、OpenGL/Vulkan 渲染管线、GLSL 着色器优化，到内存管理、并行计算等底层原理，结合图形学理论与工程实践，帮助开发者构建高性能的 3D 应用。

## 目录

### 第一部分：理论基础
1. [GPU 架构与渲染管线深度解析](#1-gpu-架构与渲染管线深度解析)
2. [性能分析方法论](#2-性能分析方法论)
3. [渲染方程与光照模型](#3-渲染方程与光照模型)

### 第二部分：渲染优化
4. [Draw Call 与批处理深度优化](#4-draw-call-与批处理深度优化)
5. [几何体处理与空间数据结构](#5-几何体处理与空间数据结构)
6. [着色器编程与 GPU 计算优化](#6-着色器编程与-gpu-计算优化)
7. [纹理系统与内存带宽优化](#7-纹理系统与内存带宽优化)

### 第三部分：高级技术
8. [光照系统与全局光照](#8-光照系统与全局光照)
9. [阴影技术深度剖析](#9-阴影技术深度剖析)
10. [后处理与屏幕空间技术](#10-后处理与屏幕空间技术)
11. [内存管理与资源流送](#11-内存管理与资源流送)

### 第四部分：工程实践
12. [CPU-GPU 并行与异步计算](#12-cpu-gpu-并行与异步计算)
13. [多线程渲染架构](#13-多线程渲染架构)
14. [性能分析工具与调试技术](#14-性能分析工具与调试技术)
15. [跨平台优化策略](#15-跨平台优化策略)

---

## 第一部分：理论基础

## 1. GPU 架构与渲染管线深度解析

### 1.1 现代 GPU 架构原理

#### 1.1.1 GPU 与 CPU 的本质区别

**CPU 架构特点**：
- 少量强大的核心（4-64 核）
- 复杂的控制逻辑（分支预测、乱序执行）
- 大容量缓存（L1/L2/L3）
- 低延迟设计
- 适合串行任务

**GPU 架构特点**：
- 大量简单的核心（数千个 CUDA 核心/流处理器）
- 简单的控制逻辑
- 小容量缓存，高带宽内存
- 高吞吐量设计
- 适合并行任务

```
CPU 设计哲学：让一个任务尽可能快
GPU 设计哲学：让大量任务同时执行

示例：
CPU: 1 个工人，速度极快，可以做任何复杂工作
GPU: 1000 个工人，每个速度较慢，但只能做简单重复工作
```

#### 1.1.2 GPU 内存层次结构

```
┌─────────────────────────────────────────┐
│         寄存器 (Registers)               │  最快，每个线程私有
│         ~32KB per SM                     │  延迟: 1 cycle
├─────────────────────────────────────────┤
│      共享内存 (Shared Memory)            │  快，线程块共享
│         ~48-96KB per SM                  │  延迟: ~20 cycles
├─────────────────────────────────────────┤
│      L1 缓存 (L1 Cache)                  │  中等，SM 私有
│         ~128KB per SM                    │  延迟: ~30 cycles
├─────────────────────────────────────────┤
│      L2 缓存 (L2 Cache)                  │  中等，全局共享
│         ~4-6MB                           │  延迟: ~200 cycles
├─────────────────────────────────────────┤
│      全局内存 (Global Memory/VRAM)       │  慢，但容量大
│         8-24GB                           │  延迟: ~400-800 cycles
└─────────────────────────────────────────┘
```

**性能影响**：
- 寄存器访问：1 cycle
- 共享内存访问：~20 cycles
- L1 缓存命中：~30 cycles
- L2 缓存命中：~200 cycles
- 全局内存访问：~400-800 cycles

**优化策略**：
1. 最大化寄存器使用
2. 利用共享内存减少全局内存访问
3. 保证内存访问的合并（Coalesced Access）
4. 避免 Bank Conflicts

#### 1.1.3 SIMT 执行模型

**SIMT (Single Instruction, Multiple Threads)**：

```
Warp (32 threads) 执行模型：

时钟周期 1:  所有 32 个线程执行 指令 1
时钟周期 2:  所有 32 个线程执行 指令 2
时钟周期 3:  所有 32 个线程执行 指令 3

如果有分支：
if (threadId < 16) {
    // 指令 A
} else {
    // 指令 B
}

实际执行：
时钟周期 1:  前 16 个线程执行指令 A，后 16 个线程空闲
时钟周期 2:  前 16 个线程空闲，后 16 个线程执行指令 B

结果：分支导致 50% 的性能损失！
```

**分支发散（Branch Divergence）问题**：

```glsl
// ❌ 差：严重的分支发散
void main() {
    if (gl_FragCoord.x < 512.0) {
        // 复杂计算 A (100 条指令)
        color = complexCalculationA();
    } else {
        // 复杂计算 B (100 条指令)
        color = complexCalculationB();
    }
}
// 性能：200 条指令（两个分支都要执行）

// ✅ 好：避免分支
void main() {
    float mask = step(512.0, gl_FragCoord.x);
    vec3 colorA = complexCalculationA();
    vec3 colorB = complexCalculationB();
    color = mix(colorA, colorB, mask);
}
// 性能：仍然 200 条指令，但可以并行

// ✅ 更好：重构算法避免分支
void main() {
    // 统一的计算路径
    color = unifiedCalculation(gl_FragCoord.x);
}
// 性能：100 条指令
```

### 1.2 OpenGL/Vulkan 渲染管线详解

#### 1.2.1 完整渲染管线

```
应用程序 (CPU)
    ↓
┌─────────────────────────────────────────┐
│  1. 顶点规范 (Vertex Specification)      │
│     - 顶点数据 (VBO)                     │
│     - 索引数据 (IBO)                     │
│     - 顶点属性布局 (VAO)                 │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│  2. 顶点着色器 (Vertex Shader)           │  可编程
│     - 顶点变换                           │
│     - 计算 varying 变量                  │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│  3. 曲面细分 (Tessellation) [可选]       │  可编程
│     - 细分控制着色器 (TCS)               │
│     - 细分评估着色器 (TES)               │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│  4. 几何着色器 (Geometry Shader) [可选]  │  可编程
│     - 图元生成/删除                      │
│     - 图元类型转换                       │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│  5. 图元装配 (Primitive Assembly)        │  固定功能
│     - 组装三角形                         │
│     - 面剔除 (Face Culling)              │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│  6. 光栅化 (Rasterization)               │  固定功能
│     - 三角形扫描转换                     │
│     - 生成片段                           │
│     - 插值 varying 变量                  │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│  7. 片段着色器 (Fragment Shader)         │  可编程
│     - 计算像素颜色                       │
│     - 纹理采样                           │
│     - 光照计算                           │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│  8. 逐片段操作 (Per-Fragment Ops)        │  固定功能
│     - 深度测试 (Depth Test)              │
│     - 模板测试 (Stencil Test)            │
│     - 混合 (Blending)                    │
└─────────────────────────────────────────┘
    ↓
帧缓冲 (Framebuffer)
```

#### 1.2.2 顶点处理阶段深度分析

**顶点着色器的职责**：

```glsl
#version 450 core

// 输入：顶点属性
layout(location = 0) in vec3 aPosition;   // 位置
layout(location = 1) in vec3 aNormal;     // 法线
layout(location = 2) in vec2 aTexCoord;   // 纹理坐标
layout(location = 3) in vec3 aTangent;    // 切线
layout(location = 4) in vec3 aBitangent;  // 副切线

// Uniform：变换矩阵
uniform mat4 uModelMatrix;       // 模型矩阵
uniform mat4 uViewMatrix;        // 视图矩阵
uniform mat4 uProjectionMatrix;  // 投影矩阵
uniform mat3 uNormalMatrix;      // 法线矩阵

// 输出：传递给片段着色器
out VS_OUT {
    vec3 FragPos;        // 世界空间位置
    vec3 Normal;         // 世界空间法线
    vec2 TexCoord;       // 纹理坐标
    mat3 TBN;            // 切线空间矩阵
} vs_out;

void main() {
    // 1. 计算世界空间位置
    vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
    vs_out.FragPos = worldPos.xyz;
    
    // 2. 计算裁剪空间位置（最终输出）
    gl_Position = uProjectionMatrix * uViewMatrix * worldPos;
    
    // 3. 变换法线到世界空间
    vs_out.Normal = normalize(uNormalMatrix * aNormal);
    
    // 4. 传递纹理坐标
    vs_out.TexCoord = aTexCoord;
    
    // 5. 构建 TBN 矩阵（用于法线贴图）
    vec3 T = normalize(uNormalMatrix * aTangent);
    vec3 B = normalize(uNormalMatrix * aBitangent);
    vec3 N = vs_out.Normal;
    vs_out.TBN = mat3(T, B, N);
}
```

**性能关键点**：

1. **矩阵乘法开销**：
```glsl
// ❌ 差：重复计算
gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);

// ✅ 好：预计算 MVP 矩阵
uniform mat4 uMVP;  // CPU 端预计算
gl_Position = uMVP * vec4(aPosition, 1.0);
```

2. **法线矩阵计算**：
```cpp
// CPU 端计算法线矩阵
glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
```

3. **顶点属性压缩**：
```cpp
// ❌ 差：float 精度（12 字节）
struct Vertex {
    float x, y, z;  // 12 bytes
};

// ✅ 好：half float（6 字节）
struct Vertex {
    uint16_t x, y, z;  // 6 bytes, 解码：float(x) / 65535.0
};

// ✅ 更好：10-10-10-2 格式（4 字节）
struct Vertex {
    uint32_t packed;  // 4 bytes
    // x: 10 bits, y: 10 bits, z: 10 bits, w: 2 bits
};
```

#### 1.2.3 光栅化阶段

**三角形扫描转换算法**：

```
三角形光栅化过程：

1. 边缘方程 (Edge Equations)：
   对于三角形的每条边，计算边缘函数：
   E(x, y) = (y0 - y1) * x + (x1 - x0) * y + (x0*y1 - x1*y0)
   
   如果 E(x, y) > 0，点在边的右侧
   如果 E(x, y) < 0，点在边的左侧
   如果 E(x, y) = 0，点在边上

2. 包围盒 (Bounding Box)：
   计算三角形的最小包围盒
   只测试包围盒内的像素

3. 像素测试：
   for (int y = minY; y <= maxY; y++) {
       for (int x = minX; x <= maxX; x++) {
           if (insideTriangle(x, y)) {
               generateFragment(x, y);
           }
       }
   }

4. 属性插值 (Attribute Interpolation)：
   使用重心坐标插值顶点属性
   
   重心坐标 (α, β, γ)：
   α = Area(P, V1, V2) / Area(V0, V1, V2)
   β = Area(V0, P, V2) / Area(V0, V1, V2)
   γ = Area(V0, V1, P) / Area(V0, V1, V2)
   
   插值属性：
   attribute = α * attr0 + β * attr1 + γ * attr2
```

**透视校正插值**：

```glsl
// 线性插值（错误）
vec2 texCoord = α * tc0 + β * tc1 + γ * tc2;

// 透视校正插值（正确）
float w0 = 1.0 / gl_Position0.w;
float w1 = 1.0 / gl_Position1.w;
float w2 = 1.0 / gl_Position2.w;

vec2 tc0_persp = tc0 * w0;
vec2 tc1_persp = tc1 * w1;
vec2 tc2_persp = tc2 * w2;

float w_interp = α * w0 + β * w1 + γ * w2;
vec2 tc_interp = α * tc0_persp + β * tc1_persp + γ * tc2_persp;

vec2 texCoord = tc_interp / w_interp;
```

### 1.3 Qt Quick 3D 的渲染架构

#### 1.3.1 场景图 (Scene Graph) 架构

```
QML 场景
    ↓
┌─────────────────────────────────────────┐
│  QML 引擎 (JavaScript VM)                │
│  - 属性绑定                              │
│  - 动画系统                              │
│  - 事件处理                              │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│  Scene Graph (C++)                       │
│  - 节点树                                │
│  - 批处理                                │
│  - 材质系统                              │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│  Qt Quick 3D Runtime                     │
│  - 3D 场景管理                           │
│  - 光照计算                              │
│  - 阴影渲染                              │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│  RHI (Rendering Hardware Interface)      │
│  - OpenGL                                │
│  - Vulkan                                │
│  - Direct3D 11/12                        │
│  - Metal                                 │
└─────────────────────────────────────────┘
    ↓
GPU
```

#### 1.3.2 渲染线程模型

```cpp
// Qt Quick 的线程模型

主线程 (GUI Thread):
- QML 对象创建
- 属性更新
- 事件处理
- JavaScript 执行

渲染线程 (Render Thread):
- 场景图同步
- OpenGL 调用
- 帧缓冲操作
- GPU 命令提交

同步点:
┌─────────────┐         ┌─────────────┐
│  GUI Thread │         │Render Thread│
└──────┬──────┘         └──────┬──────┘
       │                       │
       │  1. 更新属性           │
       ├──────────────────────>│
       │                       │
       │  2. 同步场景图         │
       │<──────────────────────┤
       │  (阻塞)                │
       │                       │
       │                       │  3. 渲染
       │                       ├────────>
       │                       │
       │  4. 交换缓冲           │
       │<──────────────────────┤
       │                       │
```

**性能影响**：
- 同步点会导致线程等待
- 减少同步频率可以提升性能
- 使用 `QQuickItem::updatePaintNode()` 优化同步


## 2. 性能分析方法论

### 2.1 性能瓶颈的本质

**Amdahl 定律**：
```
加速比 = 1 / ((1 - P) + P/S)

其中：
P = 可并行化部分的比例
S = 并行化后的加速倍数

示例：
如果 90% 的代码可以并行化，并行化后快 10 倍：
加速比 = 1 / ((1 - 0.9) + 0.9/10) = 1 / 0.19 ≈ 5.26 倍

结论：优化瓶颈部分才有意义！
```

**性能分析的三个维度**：

1. **时间维度**：
   - CPU 时间
   - GPU 时间
   - 内存访问时间
   - I/O 时间

2. **空间维度**：
   - CPU 内存占用
   - GPU 显存占用
   - 缓存占用
   - 带宽占用

3. **并行维度**：
   - CPU 核心利用率
   - GPU 占用率
   - 内存带宽利用率
   - 指令级并行度

### 2.2 GPU 性能指标深度解析

#### 2.2.1 填充率 (Fill Rate)

**定义**：GPU 每秒能够处理的像素数量

```
填充率 = GPU 频率 × ROP 数量 × 每时钟周期像素数

示例（NVIDIA RTX 3080）：
- GPU 频率：1.71 GHz
- ROP 数量：96
- 每时钟周期：1 像素

填充率 = 1.71 × 10^9 × 96 × 1 = 164 GPixels/s
```

**测试填充率瓶颈**：

```qml
// 测试 1：全屏简单着色器
CustomMaterial {
    fragmentShader: "
        void MAIN() {
            BASE_COLOR = vec4(1.0, 0.0, 0.0, 1.0);
        }
    "
}
// 如果 FPS 很低，说明填充率是瓶颈

// 测试 2：降低分辨率
View3D {
    width: parent.width / 2
    height: parent.height / 2
}
// 如果 FPS 提升 4 倍，确认是填充率瓶颈
```

#### 2.2.2 纹理填充率 (Texture Fill Rate)

```
纹理填充率 = GPU 频率 × 纹理单元数量 × 每时钟周期纹理数

示例（NVIDIA RTX 3080）：
- GPU 频率：1.71 GHz
- 纹理单元：272
- 每时钟周期：4 纹理

纹理填充率 = 1.71 × 10^9 × 272 × 4 = 1.86 TTexels/s
```

**优化纹理采样**：

```glsl
// ❌ 差：多次纹理采样
vec4 color = texture(tex, uv);
vec4 color2 = texture(tex, uv + vec2(0.01, 0.0));
vec4 color3 = texture(tex, uv + vec2(0.0, 0.01));
vec4 result = (color + color2 + color3) / 3.0;

// ✅ 好：使用 Mipmap
vec4 color = textureLod(tex, uv, 1.0);  // 使用更低的 mip 级别

// ✅ 更好：使用纹理图集
vec4 color = texture(atlas, uv * scale + offset);
```

#### 2.2.3 内存带宽 (Memory Bandwidth)

```
内存带宽 = 内存频率 × 内存位宽 / 8

示例（NVIDIA RTX 3080）：
- 内存频率：19 Gbps
- 内存位宽：320 bit

内存带宽 = 19 × 10^9 × 320 / 8 = 760 GB/s
```

**带宽计算示例**：

```
渲染 1920×1080 @ 60 FPS，RGBA8 格式：

每帧数据量：
- 颜色缓冲：1920 × 1080 × 4 = 8.3 MB
- 深度缓冲：1920 × 1080 × 4 = 8.3 MB
- 总计：16.6 MB

每秒带宽需求：
16.6 MB × 60 = 996 MB/s

如果有 4 个 render target：
996 MB/s × 4 = 3.98 GB/s

加上纹理读取（假设 10 GB/s）：
总带宽：3.98 + 10 = 13.98 GB/s

带宽利用率：13.98 / 760 = 1.8%
```

### 2.3 性能分析工具链

#### 2.3.1 GPU 性能计数器

```cpp
// OpenGL 性能查询
GLuint query;
glGenQueries(1, &query);

// 开始查询
glBeginQuery(GL_TIME_ELAPSED, query);

// 渲染代码
renderScene();

// 结束查询
glEndQuery(GL_TIME_ELAPSED);

// 获取结果
GLuint64 timeElapsed;
glGetQueryObjectui64v(query, GL_QUERY_RESULT, &timeElapsed);

qDebug() << "GPU Time:" << timeElapsed / 1000000.0 << "ms";
```

#### 2.3.2 RenderDoc 深度分析

**使用 RenderDoc 分析 Qt Quick 3D**：

```bash
# 1. 启用 RenderDoc 捕获
export QSG_RHI_BACKEND=opengl
export QSG_RHI_DEBUG_LAYER=1

# 2. 运行应用
renderdoc ./YourApp

# 3. 分析关键指标：
# - Draw Call 数量
# - 顶点数/三角形数
# - 纹理绑定次数
# - 着色器切换次数
# - GPU 时间分布
```

**关键分析点**：

1. **Draw Call 分析**：
```
Event Browser:
├─ Clear (0.1ms)
├─ Draw #1: Skybox (0.5ms)
│   ├─ Vertex Count: 36
│   ├─ Texture Bindings: 1
│   └─ Shader: skybox.vert/frag
├─ Draw #2: Terrain (2.3ms)  ← 瓶颈！
│   ├─ Vertex Count: 1,000,000
│   ├─ Texture Bindings: 4
│   └─ Shader: terrain.vert/frag
└─ Draw #3: UI (0.2ms)
```

2. **纹理分析**：
```
Texture Viewer:
- BaseColor: 2048×2048 RGBA8 (16MB)
- Normal: 2048×2048 RGBA8 (16MB)
- Roughness: 2048×2048 R8 (4MB)
- Metallic: 2048×2048 R8 (4MB)

优化建议：
- 合并 Roughness 和 Metallic 到一张纹理
- 使用压缩格式 (DXT5)
```

3. **着色器分析**：
```
Shader Debugger:
- ALU Instructions: 256
- Texture Instructions: 8
- Branch Instructions: 12  ← 优化点
- Register Pressure: High

优化建议：
- 减少分支
- 降低寄存器使用
```


## 3. 渲染方程与光照模型

### 3.1 渲染方程 (Rendering Equation)

**Kajiya 渲染方程**：

```
L_o(p, ω_o) = L_e(p, ω_o) + ∫_Ω f_r(p, ω_i, ω_o) L_i(p, ω_i) (n · ω_i) dω_i

其中：
L_o(p, ω_o) = 出射辐射度（从点 p 沿方向 ω_o）
L_e(p, ω_o) = 自发光
f_r(p, ω_i, ω_o) = BRDF（双向反射分布函数）
L_i(p, ω_i) = 入射辐射度（从方向 ω_i 到点 p）
n · ω_i = 余弦项（Lambert 定律）
Ω = 半球积分域
```

**物理意义**：
- 一个点的出射光 = 自发光 + 所有方向入射光经过表面反射的总和
- 这是一个递归方程（入射光本身也是其他点的出射光）
- 实时渲染无法精确求解，需要近似

### 3.2 BRDF 模型深度解析

#### 3.2.1 Lambert 漫反射

```glsl
// Lambert BRDF
vec3 lambertDiffuse(vec3 albedo) {
    return albedo / PI;  // 归一化
}

// 完整的漫反射计算
vec3 diffuse = lambertDiffuse(baseColor) * lightColor * max(dot(N, L), 0.0);
```

**为什么除以 π？**

```
能量守恒：
入射能量 = 出射能量

入射辐照度：E_i = L_i * cos(θ)
出射辐射度：L_o = ρ * E_i / π

其中 ρ 是反照率（albedo）
除以 π 是为了保证能量守恒
```

#### 3.2.2 Cook-Torrance 镜面反射

```glsl
// Cook-Torrance BRDF
vec3 cookTorranceBRDF(vec3 N, vec3 V, vec3 L, vec3 H, 
                      float roughness, float metallic, vec3 F0) {
    // 1. 法线分布函数 (NDF) - GGX/Trowbridge-Reitz
    float D = distributionGGX(N, H, roughness);
    
    // 2. 几何函数 (Geometry Function) - Smith
    float G = geometrySmith(N, V, L, roughness);
    
    // 3. 菲涅尔方程 (Fresnel Equation) - Schlick 近似
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    // Cook-Torrance 镜面反射
    vec3 numerator = D * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    return specular;
}

// GGX 法线分布函数
float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / denom;
}

// Smith 几何函数
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / denom;
}

// Fresnel-Schlick 近似
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
```

**物理意义**：

1. **法线分布函数 (D)**：
   - 描述微表面法线的分布
   - roughness 越大，分布越宽
   - 决定高光的大小和形状

2. **几何函数 (G)**：
   - 描述微表面的自遮挡
   - 粗糙表面有更多自遮挡
   - 防止能量增加

3. **菲涅尔项 (F)**：
   - 描述不同角度的反射率
   - 掠射角反射率更高
   - 金属和非金属有不同的 F0

#### 3.2.3 完整的 PBR 着色器

```glsl
#version 450 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

// 材质参数
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// 光照参数
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 camPos;

const float PI = 3.14159265359;

// ... (前面的 BRDF 函数)

void main() {
    // 1. 采样材质属性
    vec3 albedo = pow(texture(albedoMap, TexCoord).rgb, vec3(2.2));  // sRGB to linear
    float metallic = texture(metallicMap, TexCoord).r;
    float roughness = texture(roughnessMap, TexCoord).r;
    float ao = texture(aoMap, TexCoord).r;
    
    // 2. 计算法线（法线贴图）
    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - FragPos);
    
    // 3. 计算 F0（基础反射率）
    vec3 F0 = vec3(0.04);  // 非金属的基础反射率
    F0 = mix(F0, albedo, metallic);  // 金属使用 albedo 作为 F0
    
    // 4. 反射率方程
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) {
        // 计算每个光源的贡献
        vec3 L = normalize(lightPositions[i] - FragPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;
        
        // Cook-Torrance BRDF
        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        
        // 能量守恒
        vec3 kS = F;  // 镜面反射比例
        vec3 kD = vec3(1.0) - kS;  // 漫反射比例
        kD *= 1.0 - metallic;  // 金属没有漫反射
        
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    
    // 5. 环境光照（简化）
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    // 6. 最终颜色
    vec3 color = ambient + Lo;
    
    // 7. HDR 色调映射
    color = color / (color + vec3(1.0));  // Reinhard
    
    // 8. Gamma 校正
    color = pow(color, vec3(1.0/2.2));  // Linear to sRGB
    
    FragColor = vec4(color, 1.0);
}
```

### 3.3 全局光照近似

#### 3.3.1 环境光遮蔽 (Ambient Occlusion)

**SSAO (Screen Space Ambient Occlusion)**：

```glsl
// SSAO 片段着色器
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform vec3 samples[64];

const int kernelSize = 64;
const float radius = 0.5;
const float bias = 0.025;

void main() {
    // 获取位置和法线
    vec3 fragPos = texture(gPosition, TexCoord).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoord).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoord * noiseScale).xyz);
    
    // 创建 TBN 矩阵
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    // 采样半球
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i) {
        // 获取采样位置
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * radius;
        
        // 投影到屏幕空间
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        
        // 获取采样深度
        float sampleDepth = texture(gPosition, offset.xy).z;
        
        // 范围检查和累积
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    
    occlusion = 1.0 - (occlusion / kernelSize);
    FragColor = vec4(vec3(occlusion), 1.0);
}
```

**HBAO (Horizon-Based Ambient Occlusion)**：

```glsl
// HBAO - 更高质量的 AO
float horizonBasedAO(vec3 P, vec3 N, vec2 uv) {
    const int numDirections = 8;
    const int numSteps = 4;
    
    float ao = 0.0;
    float angleStep = 2.0 * PI / float(numDirections);
    
    for(int i = 0; i < numDirections; i++) {
        float angle = float(i) * angleStep;
        vec2 direction = vec2(cos(angle), sin(angle));
        
        float maxHorizonAngle = -PI / 2.0;
        
        for(int j = 1; j <= numSteps; j++) {
            vec2 sampleUV = uv + direction * float(j) * stepSize;
            vec3 samplePos = texture(gPosition, sampleUV).xyz;
            
            vec3 diff = samplePos - P;
            float horizonAngle = atan(diff.z / length(diff.xy));
            maxHorizonAngle = max(maxHorizonAngle, horizonAngle);
        }
        
        float sinHorizon = sin(maxHorizonAngle);
        ao += clamp(dot(N, vec3(direction, sinHorizon)), 0.0, 1.0);
    }
    
    return ao / float(numDirections);
}
```

#### 3.3.2 图像基础光照 (IBL)

**预计算辐照度贴图**：

```glsl
// 卷积环境贴图生成辐照度贴图
#version 450 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main() {
    vec3 N = normalize(WorldPos);
    
    vec3 irradiance = vec3(0.0);
    
    // 半球采样
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));
    
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            // 球面坐标转笛卡尔坐标
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            
            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    FragColor = vec4(irradiance, 1.0);
}
```

**预过滤环境贴图（镜面反射）**：

```glsl
// 预过滤不同粗糙度级别的环境贴图
vec3 prefilterEnvMap(vec3 R, float roughness) {
    const uint SAMPLE_COUNT = 1024u;
    vec3 N = R;
    vec3 V = R;
    
    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);
    
    for(uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec2 Xi = hammersley(i, SAMPLE_COUNT);
        vec3 H = importanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        
        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0) {
            prefilteredColor += texture(environmentMap, L).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    
    return prefilteredColor / totalWeight;
}

// Hammersley 序列
vec2 hammersley(uint i, uint N) {
    uint bits = i;
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return vec2(float(i) / float(N), rdi);
}

// 重要性采样 GGX
vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;
    
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
    
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
    
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}
```


## 第二部分：渲染优化

## 4. Draw Call 与批处理深度优化

### 4.1 Draw Call 的底层机制

#### 4.1.1 OpenGL Draw Call 流程

```cpp
// 一个典型的 Draw Call 涉及的操作

// 1. 绑定 VAO (Vertex Array Object)
glBindVertexArray(vao);  // ~100 CPU cycles

// 2. 绑定着色器程序
glUseProgram(shaderProgram);  // ~200 CPU cycles

// 3. 设置 Uniform 变量
glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);  // ~50 cycles × N uniforms

// 4. 绑定纹理
glActiveTexture(GL_TEXTURE0);  // ~50 cycles
glBindTexture(GL_TEXTURE_2D, texture);  // ~100 cycles × N textures

// 5. 设置渲染状态
glEnable(GL_DEPTH_TEST);  // ~50 cycles
glEnable(GL_BLEND);  // ~50 cycles
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // ~50 cycles

// 6. 发起绘制调用
glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);  // ~500 cycles

// 总开销：~1500-2000 CPU cycles
// 在 3GHz CPU 上：~0.5-0.7 微秒
// 1000 个 Draw Calls：~0.5-0.7 毫秒（仅 CPU 端）
```

**驱动层开销**：
```
应用程序 glDrawElements()
    ↓
OpenGL 驱动验证
    ├─ 检查状态一致性
    ├─ 验证着色器
    ├─ 验证纹理绑定
    └─ 验证缓冲区
    ↓
命令缓冲区
    ├─ 批处理命令
    ├─ 优化状态切换
    └─ DMA 传输准备
    ↓
GPU 命令队列
    ├─ 命令解析
    ├─ 资源绑定
    └─ 执行绘制
```

### 4.2 实例化渲染深度优化

#### 4.2.1 GPU Instancing 原理

```glsl
// 顶点着色器 - 实例化版本
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// 实例化属性（每个实例不同）
layout(location = 3) in mat4 aInstanceMatrix;  // 实例变换矩阵
layout(location = 7) in vec4 aInstanceColor;   // 实例颜色

uniform mat4 uViewProjection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 InstanceColor;

void main() {
    // 使用实例矩阵变换顶点
    vec4 worldPos = aInstanceMatrix * vec4(aPosition, 1.0);
    gl_Position = uViewProjection * worldPos;
    
    FragPos = worldPos.xyz;
    Normal = mat3(aInstanceMatrix) * aNormal;
    TexCoord = aTexCoord;
    InstanceColor = aInstanceColor;
}
```

```cpp
// C++ 端设置实例化数据
struct InstanceData {
    glm::mat4 transform;
    glm::vec4 color;
};

std::vector<InstanceData> instances(10000);

// 生成实例数据
for(int i = 0; i < 10000; i++) {
    instances[i].transform = glm::translate(glm::mat4(1.0f), 
        glm::vec3(rand() % 1000, rand() % 100, rand() % 1000));
    instances[i].color = glm::vec4(rand() / float(RAND_MAX), 
        rand() / float(RAND_MAX), rand() / float(RAND_MAX), 1.0f);
}

// 创建实例化 VBO
GLuint instanceVBO;
glGenBuffers(1, &instanceVBO);
glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceData), 
             instances.data(), GL_STATIC_DRAW);

// 设置实例化属性
glBindVertexArray(vao);

// 变换矩阵（4 个 vec4）
for(int i = 0; i < 4; i++) {
    glEnableVertexAttribArray(3 + i);
    glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, 
                         sizeof(InstanceData), 
                         (void*)(sizeof(glm::vec4) * i));
    glVertexAttribDivisor(3 + i, 1);  // 每个实例更新一次
}

// 颜色
glEnableVertexAttribArray(7);
glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 
                     sizeof(InstanceData), 
                     (void*)(sizeof(glm::mat4)));
glVertexAttribDivisor(7, 1);

// 绘制所有实例（1 次 Draw Call）
glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, 10000);
```

**性能对比**：
```
传统方式（10000 个对象）：
- Draw Calls: 10000
- CPU 时间: ~7ms
- GPU 时间: ~5ms
- 总时间: ~12ms
- FPS: ~83

实例化方式（10000 个实例）：
- Draw Calls: 1
- CPU 时间: ~0.01ms
- GPU 时间: ~5ms
- 总时间: ~5ms
- FPS: ~200

性能提升：2.4 倍
```

#### 4.2.2 间接绘制 (Indirect Drawing)

```cpp
// 间接绘制命令结构
struct DrawElementsIndirectCommand {
    GLuint count;         // 索引数量
    GLuint instanceCount; // 实例数量
    GLuint firstIndex;    // 起始索引
    GLuint baseVertex;    // 基础顶点
    GLuint baseInstance;  // 基础实例
};

// 创建间接绘制缓冲
std::vector<DrawElementsIndirectCommand> commands;

// 添加多个绘制命令
commands.push_back({36, 1000, 0, 0, 0});      // 1000 个立方体
commands.push_back({192, 500, 36, 0, 1000});  // 500 个球体
commands.push_back({24, 2000, 228, 0, 1500}); // 2000 个平面

GLuint indirectBuffer;
glGenBuffers(1, &indirectBuffer);
glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
glBufferData(GL_DRAW_INDIRECT_BUFFER, 
             commands.size() * sizeof(DrawElementsIndirectCommand),
             commands.data(), GL_STATIC_DRAW);

// 多重间接绘制（1 次 Draw Call 绘制 3500 个对象）
glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 
                           nullptr, commands.size(), 0);
```

**GPU 驱动的渲染**：
```glsl
// 计算着色器：视锥剔除 + 生成绘制命令
#version 450 core

layout(local_size_x = 256) in;

struct ObjectData {
    mat4 transform;
    vec4 boundingSphere;  // xyz: center, w: radius
};

struct DrawCommand {
    uint count;
    uint instanceCount;
    uint firstIndex;
    uint baseVertex;
    uint baseInstance;
};

layout(std430, binding = 0) readonly buffer ObjectBuffer {
    ObjectData objects[];
};

layout(std430, binding = 1) writeonly buffer CommandBuffer {
    DrawCommand commands[];
};

layout(std430, binding = 2) buffer CounterBuffer {
    uint visibleCount;
};

uniform mat4 uViewProjection;
uniform vec4 uFrustumPlanes[6];

bool isVisible(vec4 sphere) {
    // 视锥剔除
    for(int i = 0; i < 6; i++) {
        float dist = dot(uFrustumPlanes[i], vec4(sphere.xyz, 1.0));
        if(dist < -sphere.w) return false;
    }
    return true;
}

void main() {
    uint objectId = gl_GlobalInvocationID.x;
    if(objectId >= objects.length()) return;
    
    ObjectData obj = objects[objectId];
    
    // 变换包围球到世界空间
    vec4 worldSphere = vec4((obj.transform * vec4(obj.boundingSphere.xyz, 1.0)).xyz,
                           obj.boundingSphere.w);
    
    // 视锥剔除
    if(isVisible(worldSphere)) {
        uint index = atomicAdd(visibleCount, 1);
        commands[index].count = 36;  // 立方体索引数
        commands[index].instanceCount = 1;
        commands[index].firstIndex = 0;
        commands[index].baseVertex = 0;
        commands[index].baseInstance = objectId;
    }
}
```

### 4.3 批处理策略

#### 4.3.1 静态批处理

```cpp
// 合并多个静态网格到一个大网格
class StaticBatcher {
public:
    struct MeshData {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        glm::mat4 transform;
    };
    
    struct BatchedMesh {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        GLuint vao, vbo, ibo;
    };
    
    BatchedMesh batchMeshes(const std::vector<MeshData>& meshes) {
        BatchedMesh result;
        uint32_t vertexOffset = 0;
        
        for(const auto& mesh : meshes) {
            // 变换顶点到世界空间
            for(const auto& v : mesh.vertices) {
                Vertex transformed = v;
                glm::vec4 pos = mesh.transform * glm::vec4(v.position, 1.0);
                transformed.position = glm::vec3(pos);
                transformed.normal = glm::mat3(mesh.transform) * v.normal;
                result.vertices.push_back(transformed);
            }
            
            // 调整索引
            for(uint32_t idx : mesh.indices) {
                result.indices.push_back(idx + vertexOffset);
            }
            
            vertexOffset += mesh.vertices.size();
        }
        
        // 创建 OpenGL 缓冲
        createBuffers(result);
        
        return result;
    }
    
private:
    void createBuffers(BatchedMesh& mesh) {
        glGenVertexArrays(1, &mesh.vao);
        glGenBuffers(1, &mesh.vbo);
        glGenBuffers(1, &mesh.ibo);
        
        glBindVertexArray(mesh.vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, 
                    mesh.vertices.size() * sizeof(Vertex),
                    mesh.vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                    mesh.indices.size() * sizeof(uint32_t),
                    mesh.indices.data(), GL_STATIC_DRAW);
        
        // 设置顶点属性...
    }
};
```

#### 4.3.2 动态批处理

```cpp
// 动态批处理：每帧合并相似对象
class DynamicBatcher {
public:
    struct DrawBatch {
        GLuint vao, vbo, ibo;
        uint32_t indexCount;
        Material* material;
        std::vector<glm::mat4> transforms;
    };
    
    void beginFrame() {
        batches.clear();
    }
    
    void submitMesh(Mesh* mesh, Material* material, const glm::mat4& transform) {
        // 查找或创建批次
        DrawBatch* batch = findOrCreateBatch(mesh, material);
        batch->transforms.push_back(transform);
    }
    
    void flush() {
        for(auto& batch : batches) {
            // 更新实例化缓冲
            updateInstanceBuffer(batch);
            
            // 绑定材质
            batch.material->bind();
            
            // 绘制批次
            glBindVertexArray(batch.vao);
            glDrawElementsInstanced(GL_TRIANGLES, batch.indexCount,
                                   GL_UNSIGNED_INT, 0, 
                                   batch.transforms.size());
        }
    }
    
private:
    std::vector<DrawBatch> batches;
    
    DrawBatch* findOrCreateBatch(Mesh* mesh, Material* material) {
        // 查找相同网格和材质的批次
        for(auto& batch : batches) {
            if(batch.material == material && 
               batch.vao == mesh->getVAO()) {
                return &batch;
            }
        }
        
        // 创建新批次
        DrawBatch newBatch;
        newBatch.vao = mesh->getVAO();
        newBatch.vbo = mesh->getVBO();
        newBatch.ibo = mesh->getIBO();
        newBatch.indexCount = mesh->getIndexCount();
        newBatch.material = material;
        batches.push_back(newBatch);
        
        return &batches.back();
    }
    
    void updateInstanceBuffer(DrawBatch& batch) {
        // 更新实例化变换矩阵
        glBindBuffer(GL_ARRAY_BUFFER, batch.vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                       batch.transforms.size() * sizeof(glm::mat4),
                       batch.transforms.data());
    }
};
```

### 4.4 Qt Quick 3D 中的批处理优化

```qml
import QtQuick
import QtQuick3D

View3D {
    // ✅ 策略 1：共享材质
    PrincipledMaterial {
        id: sharedMaterial
        baseColor: "#4080ff"
        metalness: 0.5
        roughness: 0.3
    }
    
    // ✅ 策略 2：使用实例化
    Model {
        source: "#Cube"
        materials: sharedMaterial
        
        instancing: InstanceList {
            id: instances
            Component.onCompleted: {
                for(let i = 0; i < 1000; i++) {
                    instances.append({
                        position: Qt.vector3d(
                            Math.random() * 1000,
                            Math.random() * 100,
                            Math.random() * 1000
                        ),
                        scale: Qt.vector3d(1, 1, 1),
                        rotation: Qt.quaternion(1, 0, 0, 0),
                        color: Qt.rgba(1, 1, 1, 1)
                    })
                }
            }
        }
    }
    
    // ✅ 策略 3：合并静态网格
    Model {
        id: staticBatch
        geometry: CustomGeometry {
            id: batchedGeometry
            
            Component.onCompleted: {
                // 合并多个静态网格
                let vertices = [];
                let indices = [];
                
                // ... 合并逻辑
                
                vertexData = createVertexBuffer(vertices);
                indexData = createIndexBuffer(indices);
            }
        }
        materials: sharedMaterial
    }
}
```


## 5. 几何体处理与空间数据结构

### 5.1 空间分割算法

#### 5.1.1 八叉树 (Octree)

```cpp
// 八叉树实现
class Octree {
public:
    struct Node {
        AABB bounds;
        std::vector<GameObject*> objects;
        Node* children[8] = {nullptr};
        bool isLeaf = true;
        int depth = 0;
    };
    
    Octree(const AABB& worldBounds, int maxDepth = 8, int maxObjects = 8)
        : maxDepth(maxDepth), maxObjectsPerNode(maxObjects) {
        root = new Node();
        root->bounds = worldBounds;
    }
    
    void insert(GameObject* obj) {
        insertRecursive(root, obj);
    }
    
    std::vector<GameObject*> query(const Frustum& frustum) {
        std::vector<GameObject*> result;
        queryRecursive(root, frustum, result);
        return result;
    }
    
private:
    Node* root;
    int maxDepth;
    int maxObjectsPerNode;
    
    void insertRecursive(Node* node, GameObject* obj) {
        // 如果是叶子节点且未达到分割条件
        if(node->isLeaf) {
            node->objects.push_back(obj);
            
            // 检查是否需要分割
            if(node->objects.size() > maxObjectsPerNode && 
               node->depth < maxDepth) {
                subdivide(node);
            }
            return;
        }
        
        // 找到对象所属的子节点
        int octant = getOctant(node, obj->getBounds());
        if(octant != -1) {
            insertRecursive(node->children[octant], obj);
        } else {
            // 跨越多个子节点，存储在当前节点
            node->objects.push_back(obj);
        }
    }
    
    void subdivide(Node* node) {
        glm::vec3 center = node->bounds.getCenter();
        glm::vec3 halfSize = node->bounds.getHalfSize();
        
        // 创建 8 个子节点
        for(int i = 0; i < 8; i++) {
            node->children[i] = new Node();
            node->children[i]->depth = node->depth + 1;
            
            glm::vec3 offset(
                (i & 1) ? halfSize.x : -halfSize.x,
                (i & 2) ? halfSize.y : -halfSize.y,
                (i & 4) ? halfSize.z : -halfSize.z
            );
            
            glm::vec3 childCenter = center + offset * 0.5f;
            node->children[i]->bounds = AABB(childCenter, halfSize * 0.5f);
        }
        
        // 重新分配对象到子节点
        std::vector<GameObject*> remainingObjects;
        for(auto* obj : node->objects) {
            int octant = getOctant(node, obj->getBounds());
            if(octant != -1) {
                insertRecursive(node->children[octant], obj);
            } else {
                remainingObjects.push_back(obj);
            }
        }
        
        node->objects = remainingObjects;
        node->isLeaf = false;
    }
    
    int getOctant(Node* node, const AABB& bounds) {
        glm::vec3 center = node->bounds.getCenter();
        glm::vec3 objCenter = bounds.getCenter();
        
        // 检查对象是否完全在某个象限内
        int octant = 0;
        if(objCenter.x > center.x) octant |= 1;
        if(objCenter.y > center.y) octant |= 2;
        if(objCenter.z > center.z) octant |= 4;
        
        // 验证对象完全在该象限内
        if(node->children[octant] && 
           node->children[octant]->bounds.contains(bounds)) {
            return octant;
        }
        
        return -1;  // 跨越多个象限
    }
    
    void queryRecursive(Node* node, const Frustum& frustum, 
                       std::vector<GameObject*>& result) {
        // 视锥剔除
        if(!frustum.intersects(node->bounds)) {
            return;
        }
        
        // 添加当前节点的对象
        for(auto* obj : node->objects) {
            if(frustum.intersects(obj->getBounds())) {
                result.push_back(obj);
            }
        }
        
        // 递归查询子节点
        if(!node->isLeaf) {
            for(int i = 0; i < 8; i++) {
                if(node->children[i]) {
                    queryRecursive(node->children[i], frustum, result);
                }
            }
        }
    }
};
```

#### 5.1.2 BVH (Bounding Volume Hierarchy)

```cpp
// BVH 实现 - 用于光线追踪和碰撞检测
class BVH {
public:
    struct Node {
        AABB bounds;
        Node* left = nullptr;
        Node* right = nullptr;
        std::vector<Triangle*> triangles;
        
        bool isLeaf() const { return left == nullptr && right == nullptr; }
    };
    
    BVH(std::vector<Triangle*>& tris) {
        root = buildRecursive(tris, 0);
    }
    
    bool intersect(const Ray& ray, Hit& hit) {
        return intersectRecursive(root, ray, hit);
    }
    
private:
    Node* root;
    
    Node* buildRecursive(std::vector<Triangle*>& triangles, int depth) {
        Node* node = new Node();
        
        // 计算包围盒
        node->bounds = computeBounds(triangles);
        
        // 叶子节点条件
        if(triangles.size() <= 4 || depth > 20) {
            node->triangles = triangles;
            return node;
        }
        
        // 选择分割轴（最长轴）
        int axis = node->bounds.getLongestAxis();
        
        // 按中位数分割
        std::nth_element(triangles.begin(), 
                        triangles.begin() + triangles.size() / 2,
                        triangles.end(),
                        [axis](Triangle* a, Triangle* b) {
                            return a->getCenter()[axis] < b->getCenter()[axis];
                        });
        
        size_t mid = triangles.size() / 2;
        std::vector<Triangle*> leftTris(triangles.begin(), 
                                       triangles.begin() + mid);
        std::vector<Triangle*> rightTris(triangles.begin() + mid, 
                                        triangles.end());
        
        // 递归构建子树
        node->left = buildRecursive(leftTris, depth + 1);
        node->right = buildRecursive(rightTris, depth + 1);
        
        return node;
    }
    
    bool intersectRecursive(Node* node, const Ray& ray, Hit& hit) {
        if(!node->bounds.intersect(ray)) {
            return false;
        }
        
        if(node->isLeaf()) {
            bool hitAny = false;
            for(auto* tri : node->triangles) {
                if(tri->intersect(ray, hit)) {
                    hitAny = true;
                }
            }
            return hitAny;
        }
        
        bool hitLeft = intersectRecursive(node->left, ray, hit);
        bool hitRight = intersectRecursive(node->right, ray, hit);
        
        return hitLeft || hitRight;
    }
    
    AABB computeBounds(const std::vector<Triangle*>& triangles) {
        AABB bounds;
        for(auto* tri : triangles) {
            bounds.expand(tri->v0);
            bounds.expand(tri->v1);
            bounds.expand(tri->v2);
        }
        return bounds;
    }
};
```

### 5.2 网格简化算法

#### 5.2.1 边折叠 (Edge Collapse)

```cpp
// Quadric Error Metrics 网格简化
class MeshSimplifier {
public:
    struct Edge {
        int v0, v1;
        glm::mat4 Q;  // Quadric error matrix
        float cost;
        glm::vec3 optimalPos;
    };
    
    Mesh simplify(const Mesh& input, float targetRatio) {
        int targetVertexCount = input.vertices.size() * targetRatio;
        
        // 1. 计算每个顶点的 Quadric 矩阵
        std::vector<glm::mat4> vertexQuadrics(input.vertices.size(), 
                                             glm::mat4(0.0f));
        computeVertexQuadrics(input, vertexQuadrics);
        
        // 2. 构建边列表并计算折叠代价
        std::vector<Edge> edges;
        buildEdges(input, vertexQuadrics, edges);
        
        // 3. 使用优先队列进行边折叠
        std::priority_queue<Edge, std::vector<Edge>, EdgeComparator> queue;
        for(const auto& edge : edges) {
            queue.push(edge);
        }
        
        Mesh result = input;
        while(result.vertices.size() > targetVertexCount && !queue.empty()) {
            Edge edge = queue.top();
            queue.pop();
            
            // 执行边折叠
            collapseEdge(result, edge, vertexQuadrics);
            
            // 更新受影响的边
            updateAffectedEdges(result, edge, vertexQuadrics, queue);
        }
        
        return result;
    }
    
private:
    void computeVertexQuadrics(const Mesh& mesh, 
                              std::vector<glm::mat4>& quadrics) {
        // 对每个三角形计算平面方程
        for(const auto& tri : mesh.triangles) {
            glm::vec3 v0 = mesh.vertices[tri.v0];
            glm::vec3 v1 = mesh.vertices[tri.v1];
            glm::vec3 v2 = mesh.vertices[tri.v2];
            
            // 计算平面方程 ax + by + cz + d = 0
            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            float d = -glm::dot(normal, v0);
            
            glm::vec4 plane(normal, d);
            
            // 构建 Quadric 矩阵 Q = pp^T
            glm::mat4 Q = glm::outerProduct(plane, plane);
            
            // 累加到顶点
            quadrics[tri.v0] += Q;
            quadrics[tri.v1] += Q;
            quadrics[tri.v2] += Q;
        }
    }
    
    float computeEdgeCost(const glm::vec3& v0, const glm::vec3& v1,
                         const glm::mat4& Q, glm::vec3& optimalPos) {
        // 求解最优位置：Q * v = 0
        glm::mat4 A = Q;
        A[3] = glm::vec4(0, 0, 0, 1);
        
        glm::mat4 invA = glm::inverse(A);
        glm::vec4 optimal = invA * glm::vec4(0, 0, 0, 1);
        
        optimalPos = glm::vec3(optimal);
        
        // 计算误差
        glm::vec4 v(optimalPos, 1.0f);
        float error = glm::dot(v, Q * v);
        
        return error;
    }
    
    void collapseEdge(Mesh& mesh, const Edge& edge, 
                     std::vector<glm::mat4>& quadrics) {
        // 将 v1 合并到 v0
        mesh.vertices[edge.v0] = edge.optimalPos;
        quadrics[edge.v0] = edge.Q;
        
        // 更新所有引用 v1 的三角形
        for(auto& tri : mesh.triangles) {
            if(tri.v0 == edge.v1) tri.v0 = edge.v0;
            if(tri.v1 == edge.v1) tri.v1 = edge.v0;
            if(tri.v2 == edge.v1) tri.v2 = edge.v0;
        }
        
        // 移除退化三角形
        mesh.triangles.erase(
            std::remove_if(mesh.triangles.begin(), mesh.triangles.end(),
                [](const Triangle& tri) {
                    return tri.v0 == tri.v1 || tri.v1 == tri.v2 || tri.v2 == tri.v0;
                }),
            mesh.triangles.end()
        );
    }
};
```

## 总结与最佳实践

### 性能优化金字塔

```
                    ┌─────────────────┐
                    │  微优化 (5%)    │  着色器指令优化
                    │                 │  寄存器分配
                    └─────────────────┘
                ┌───────────────────────┐
                │  算法优化 (20%)       │  空间分割
                │                       │  LOD 系统
                └───────────────────────┘
            ┌───────────────────────────────┐
            │  渲染优化 (30%)               │  批处理
            │                               │  实例化
            └───────────────────────────────┘
        ┌───────────────────────────────────────┐
        │  架构优化 (45%)                       │  Draw Call 优化
        │                                       │  内存管理
        │                                       │  并行计算
        └───────────────────────────────────────┘
```

### 核心优化原则

1. **测量优先**：先测量，再优化
2. **瓶颈优化**：优化最慢的部分
3. **权衡取舍**：质量 vs 性能
4. **平台适配**：针对目标平台优化
5. **持续迭代**：性能优化是持续过程

### 关键性能指标

| 指标 | 移动端 | 桌面端 | 说明 |
|------|--------|--------|------|
| FPS | 30-60 | 60-144 | 帧率目标 |
| Draw Calls | <50 | <200 | 绘制调用 |
| 三角形数 | <50K | <200K | 每帧三角形 |
| 纹理内存 | <256MB | <1GB | 显存占用 |
| 着色器指令 | <100 | <500 | 片段着色器 |

### 参考资源

**书籍**：
- Real-Time Rendering 4th Edition
- GPU Gems 系列
- Game Engine Architecture
- Physically Based Rendering

**在线资源**：
- Khronos OpenGL Wiki
- Learn OpenGL (learnopengl.com)
- Shadertoy (shadertoy.com)
- GPU Open (gpuopen.com)

**工具**：
- RenderDoc - 图形调试
- Nsight Graphics - NVIDIA 分析
- PIX - DirectX 调试
- Qt Creator Profiler

---

**版本**: Qt 6.x  
**最后更新**: 2024年  
**作者**: 图形学与性能优化专家

本文档涵盖了从 GPU 架构、渲染管线、光照模型到具体优化技术的完整知识体系，适合有一定基础的开发者深入学习。

