# Qt6 OpenGL 编程完全指南

> 本文档详细介绍 Qt6 中的 OpenGL 编程，涵盖 C++ OpenGL 集成、QML 3D 渲染、着色器编程、纹理处理、3D 模型加载等核心技术。

## 目录

1. [OpenGL 编程概述](#1-opengl-编程概述)
2. [Qt OpenGL 模块架构](#2-qt-opengl-模块架构)
3. [QOpenGLWidget 详解](#3-qopenglwidget-详解)
4. [OpenGL 着色器编程](#4-opengl-着色器编程)
5. [纹理与材质](#5-纹理与材质)
6. [3D 变换与相机](#6-3d-变换与相机)
7. [光照与阴影](#7-光照与阴影)
8. [QML 中的 OpenGL](#8-qml-中的-opengl)
9. [Qt Quick 3D](#9-qt-quick-3d)
10. [自定义 QML OpenGL 渲染](#10-自定义-qml-opengl-渲染)
11. [帧缓冲与离屏渲染](#11-帧缓冲与离屏渲染)
12. [性能优化与最佳实践](#12-性能优化与最佳实践)

---

## 1. OpenGL 编程概述

### 1.1 OpenGL 版本与 Qt

Qt 6 支持多个 OpenGL 版本和图形 API：

| API | 版本 | 平台 | 特点 |
|-----|------|------|------|
| **OpenGL** | 2.0+ | 桌面 | 传统图形 API |
| **OpenGL ES** | 2.0/3.0+ | 移动/嵌入式 | 精简版 OpenGL |
| **Vulkan** | 1.0+ | 跨平台 | 现代低级 API |
| **Metal** | - | macOS/iOS | Apple 专有 |
| **Direct3D** | 11/12 | Windows | Microsoft 专有 |

### 1.2 Qt OpenGL 模块配置

```cmake
# CMakeLists.txt
find_package(Qt6 REQUIRED COMPONENTS OpenGL OpenGLWidgets)
target_link_libraries(myapp PRIVATE 
    Qt6::OpenGL 
    Qt6::OpenGLWidgets
)

# 或在 .pro 文件中
QT += opengl openglwidgets
```

```cpp
// 包含必要的头文件
#include <QOpenGLWidget>           // OpenGL 窗口部件
#include <QOpenGLFunctions>        // OpenGL 函数
#include <QOpenGLShaderProgram>    // 着色器程序
#include <QOpenGLTexture>          // 纹理
#include <QOpenGLBuffer>           // 缓冲区对象
#include <QOpenGLVertexArrayObject> // VAO
#include <QOpenGLFramebufferObject> // FBO
#include <QMatrix4x4>              // 矩阵运算
#include <QVector3D>               // 3D 向量
#include <QQuaternion>             // 四元数
```

### 1.3 OpenGL 渲染管线

```
顶点数据
    ↓
顶点着色器 (Vertex Shader)
    ↓
图元装配 (Primitive Assembly)
    ↓
几何着色器 (Geometry Shader) [可选]
    ↓
光栅化 (Rasterization)
    ↓
片段着色器 (Fragment Shader)
    ↓
测试与混合 (Tests & Blending)
    ↓
帧缓冲 (Framebuffer)
```

---

## 2. Qt OpenGL 模块架构

### 2.1 核心类层次结构

```
QOpenGLWidget (OpenGL 窗口部件)
    ↓
QOpenGLFunctions (OpenGL 函数包装)
    ↓
QOpenGLShaderProgram (着色器程序管理)
    ↓
QOpenGLBuffer (VBO/IBO 管理)
    ↓
QOpenGLVertexArrayObject (VAO 管理)
    ↓
QOpenGLTexture (纹理管理)
    ↓
QOpenGLFramebufferObject (FBO 管理)
```

### 2.2 OpenGL 上下文管理

```cpp
#include <QOpenGLContext>
#include <QSurfaceFormat>

class OpenGLContextManager {
public:
    // ✅ 设置 OpenGL 上下文格式
    static void setupDefaultFormat() {
        QSurfaceFormat format;
        
        // 设置 OpenGL 版本
        format.setVersion(3, 3);  // OpenGL 3.3
        format.setProfile(QSurfaceFormat::CoreProfile);
        // - CoreProfile: 核心模式（推荐）
        // - CompatibilityProfile: 兼容模式
        
        // 设置颜色缓冲
        format.setRedBufferSize(8);
        format.setGreenBufferSize(8);
        format.setBlueBufferSize(8);
        format.setAlphaBufferSize(8);
        
        // 设置深度缓冲
        format.setDepthBufferSize(24);
        
        // 设置模板缓冲
        format.setStencilBufferSize(8);
        
        // 设置采样
        format.setSamples(4);  // 4x MSAA
        
        // 启用垂直同步
        format.setSwapInterval(1);
        
        // 设置为默认格式
        QSurfaceFormat::setDefaultFormat(format);
        
        qDebug() << "OpenGL format configured:";
        qDebug() << "  Version:" << format.majorVersion() << "." << format.minorVersion();
        qDebug() << "  Profile:" << (format.profile() == QSurfaceFormat::CoreProfile ? "Core" : "Compatibility");
        qDebug() << "  Samples:" << format.samples();
    }
    
    // ✅ 检查 OpenGL 信息
    static void printOpenGLInfo(QOpenGLContext *context) {
        if (!context) {
            qDebug() << "No OpenGL context";
            return;
        }
        
        QOpenGLFunctions *f = context->functions();
        
        qDebug() << "OpenGL Information:";
        qDebug() << "  Vendor:" << (const char*)f->glGetString(GL_VENDOR);
        qDebug() << "  Renderer:" << (const char*)f->glGetString(GL_RENDERER);
        qDebug() << "  Version:" << (const char*)f->glGetString(GL_VERSION);
        qDebug() << "  GLSL Version:" << (const char*)f->glGetString(GL_SHADING_LANGUAGE_VERSION);
        
        // 获取扩展
        GLint numExtensions;
        f->glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
        qDebug() << "  Extensions:" << numExtensions;
    }
};

// 在 main() 中设置
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 设置默认 OpenGL 格式
    OpenGLContextManager::setupDefaultFormat();
    
    // 创建窗口...
    
    return app.exec();
}
```

---

## 3. QOpenGLWidget 详解

### 3.1 基础 OpenGL 窗口部件

```cpp
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class BasicOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
    
public:
    explicit BasicOpenGLWidget(QWidget *parent = nullptr)
        : QOpenGLWidget(parent)
    {
        // 设置更新间隔（60 FPS）
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, QOverload<>::of(&BasicOpenGLWidget::update));
        timer->start(16);  // ~60 FPS
    }
    
    ~BasicOpenGLWidget() {
        makeCurrent();
        
        // 清理 OpenGL 资源
        m_vbo.destroy();
        m_vao.destroy();
        delete m_program;
        
        doneCurrent();
    }
    
protected:
    // ✅ 初始化 OpenGL（只调用一次）
    void initializeGL() override {
        // 初始化 OpenGL 函数
        initializeOpenGLFunctions();
        
        // 打印 OpenGL 信息
        qDebug() << "OpenGL Version:" << (const char*)glGetString(GL_VERSION);
        qDebug() << "GLSL Version:" << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        
        // 设置清除颜色
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        
        // 启用深度测试
        glEnable(GL_DEPTH_TEST);
        
        // 初始化着色器
        initShaders();
        
        // 初始化几何体
        initGeometry();
    }
    
    // ✅ 窗口大小改变
    void resizeGL(int w, int h) override {
        // 设置视口
        glViewport(0, 0, w, h);
        
        // 更新投影矩阵
        m_projection.setToIdentity();
        m_projection.perspective(45.0f, float(w) / float(h), 0.1f, 100.0f);
    }
    
    // ✅ 渲染场景
    void paintGL() override {
        // 清除缓冲
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 绑定着色器程序
        m_program->bind();
        
        // 设置变换矩阵
        QMatrix4x4 model;
        model.rotate(m_rotation, 0.0f, 1.0f, 0.0f);
        
        QMatrix4x4 view;
        view.translate(0.0f, 0.0f, -5.0f);
        
        m_program->setUniformValue("model", model);
        m_program->setUniformValue("view", view);
        m_program->setUniformValue("projection", m_projection);
        
        // 绘制几何体
        m_vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        m_vao.release();
        
        m_program->release();
        
        // 更新旋转角度
        m_rotation += 1.0f;
    }
    
private:
    void initShaders() {
        // 创建着色器程序
        m_program = new QOpenGLShaderProgram(this);
        
        // 顶点着色器
        const char *vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aColor;
            
            out vec3 ourColor;
            
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            
            void main() {
                gl_Position = projection * view * model * vec4(aPos, 1.0);
                ourColor = aColor;
            }
        )";
        
        // 片段着色器
        const char *fragmentShaderSource = R"(
            #version 330 core
            in vec3 ourColor;
            out vec4 FragColor;
            
            void main() {
                FragColor = vec4(ourColor, 1.0);
            }
        )";
        
        // 编译着色器
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
            qDebug() << "Vertex shader compilation failed:" << m_program->log();
        }
        
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
            qDebug() << "Fragment shader compilation failed:" << m_program->log();
        }
        
        // 链接着色器程序
        if (!m_program->link()) {
            qDebug() << "Shader program linking failed:" << m_program->log();
        }
    }
    
    void initGeometry() {
        // 三角形顶点数据（位置 + 颜色）
        float vertices[] = {
            // 位置              // 颜色
            -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // 左下 - 红色
             0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // 右下 - 绿色
             0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // 顶部 - 蓝色
        };
        
        // 创建 VAO
        m_vao.create();
        m_vao.bind();
        
        // 创建 VBO
        m_vbo.create();
        m_vbo.bind();
        m_vbo.allocate(vertices, sizeof(vertices));
        
        // 设置顶点属性指针
        // 位置属性
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        
        // 颜色属性
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        
        m_vbo.release();
        m_vao.release();
    }
    
private:
    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_vao;
    
    QMatrix4x4 m_projection;
    float m_rotation = 0.0f;
};

// 使用示例
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 设置 OpenGL 格式
    OpenGLContextManager::setupDefaultFormat();
    
    BasicOpenGLWidget widget;
    widget.resize(800, 600);
    widget.show();
    
    return app.exec();
}
```

### 3.2 高级 OpenGL 窗口部件（带交互）

```cpp
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>

class AdvancedOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
    
public:
    explicit AdvancedOpenGLWidget(QWidget *parent = nullptr);
    ~AdvancedOpenGLWidget();
    
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    // 鼠标交互
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    
private:
    void initShaders();
    void initCubeGeometry();
    void updateCamera();
    
    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_ebo{QOpenGLBuffer::IndexBuffer};
    QOpenGLVertexArrayObject m_vao;
    
    // 相机参数
    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;
    
    QVector3D m_cameraPos{0.0f, 0.0f, 5.0f};
    QVector3D m_cameraFront{0.0f, 0.0f, -1.0f};
    QVector3D m_cameraUp{0.0f, 1.0f, 0.0f};
    
    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_fov = 45.0f;
    
    // 交互状态
    bool m_mousePressed = false;
    QPoint m_lastMousePos;
    
    // 动画
    float m_time = 0.0f;
};

AdvancedOpenGLWidget::AdvancedOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    // 动画定时器
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        m_time += 0.016f;  // 16ms
        update();
    });
    timer->start(16);
}

AdvancedOpenGLWidget::~AdvancedOpenGLWidget() {
    makeCurrent();
    m_vbo.destroy();
    m_ebo.destroy();
    m_vao.destroy();
    delete m_program;
    doneCurrent();
}

void AdvancedOpenGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    initShaders();
    initCubeGeometry();
    
    qDebug() << "OpenGL initialized";
}

void AdvancedOpenGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    m_projection.setToIdentity();
    m_projection.perspective(m_fov, float(w) / float(h), 0.1f, 100.0f);
}

void AdvancedOpenGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_program->bind();
    
    // 更新相机
    updateCamera();
    
    // 更新模型矩阵
    m_model.setToIdentity();
    m_model.rotate(m_time * 50.0f, 0.5f, 1.0f, 0.0f);
    
    // 设置 uniform
    m_program->setUniformValue("model", m_model);
    m_program->setUniformValue("view", m_view);
    m_program->setUniformValue("projection", m_projection);
    m_program->setUniformValue("time", m_time);
    
    // 光照
    m_program->setUniformValue("lightPos", QVector3D(2.0f, 2.0f, 2.0f));
    m_program->setUniformValue("viewPos", m_cameraPos);
    m_program->setUniformValue("lightColor", QVector3D(1.0f, 1.0f, 1.0f));
    m_program->setUniformValue("objectColor", QVector3D(1.0f, 0.5f, 0.31f));
    
    // 绘制立方体
    m_vao.bind();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    m_vao.release();
    
    m_program->release();
}

void AdvancedOpenGLWidget::initShaders() {
    m_program = new QOpenGLShaderProgram(this);
    
    // 顶点着色器
    const char *vertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        
        out vec3 FragPos;
        out vec3 Normal;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";
    
    // 片段着色器（Phong 光照）
    const char *fragmentShader = R"(
        #version 330 core
        in vec3 FragPos;
        in vec3 Normal;
        
        out vec4 FragColor;
        
        uniform vec3 lightPos;
        uniform vec3 viewPos;
        uniform vec3 lightColor;
        uniform vec3 objectColor;
        
        void main() {
            // 环境光
            float ambientStrength = 0.1;
            vec3 ambient = ambientStrength * lightColor;
            
            // 漫反射
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;
            
            // 镜面反射
            float specularStrength = 0.5;
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = specularStrength * spec * lightColor;
            
            vec3 result = (ambient + diffuse + specular) * objectColor;
            FragColor = vec4(result, 1.0);
        }
    )";
    
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);
    m_program->link();
}

void AdvancedOpenGLWidget::initCubeGeometry() {
    // 立方体顶点（位置 + 法线）
    float vertices[] = {
        // 位置              // 法线
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
    
    m_vao.create();
    m_vao.bind();
    
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices, sizeof(vertices));
    
    m_ebo.create();
    m_ebo.bind();
    m_ebo.allocate(indices, sizeof(indices));
    
    // 位置属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    
    // 法线属性
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    m_vao.release();
}

void AdvancedOpenGLWidget::updateCamera() {
    m_view.setToIdentity();
    m_view.lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
}

void AdvancedOpenGLWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = true;
        m_lastMousePos = event->pos();
    }
}

void AdvancedOpenGLWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_mousePressed) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();
        
        float sensitivity = 0.1f;
        m_yaw += delta.x() * sensitivity;
        m_pitch -= delta.y() * sensitivity;
        
        // 限制俯仰角
        if (m_pitch > 89.0f) m_pitch = 89.0f;
        if (m_pitch < -89.0f) m_pitch = -89.0f;
        
        // 更新相机方向
        QVector3D front;
        front.setX(cos(qDegreesToRadians(m_yaw)) * cos(qDegreesToRadians(m_pitch)));
        front.setY(sin(qDegreesToRadians(m_pitch)));
        front.setZ(sin(qDegreesToRadians(m_yaw)) * cos(qDegreesToRadians(m_pitch)));
        m_cameraFront = front.normalized();
        
        update();
    }
}

void AdvancedOpenGLWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = false;
    }
}

void AdvancedOpenGLWidget::wheelEvent(QWheelEvent *event) {
    m_fov -= event->angleDelta().y() / 120.0f;
    
    if (m_fov < 1.0f) m_fov = 1.0f;
    if (m_fov > 45.0f) m_fov = 45.0f;
    
    resizeGL(width(), height());
    update();
}
```

---

## 4. OpenGL 着色器编程

### 4.1 着色器基础

```cpp
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>

class ShaderManager {
public:
    // ✅ 从文件加载着色器
    static QOpenGLShaderProgram* loadShaderFromFile(
        const QString &vertexPath,
        const QString &fragmentPath,
        const QString &geometryPath = QString())
    {
        QOpenGLShaderProgram *program = new QOpenGLShaderProgram();
        
        // 加载顶点着色器
        if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexPath)) {
            qDebug() << "Failed to load vertex shader:" << program->log();
            delete program;
            return nullptr;
        }
        
        // 加载片段着色器
        if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentPath)) {
            qDebug() << "Failed to load fragment shader:" << program->log();
            delete program;
            return nullptr;
        }
        
        // 加载几何着色器（可选）
        if (!geometryPath.isEmpty()) {
            if (!program->addShaderFromSourceFile(QOpenGLShader::Geometry, geometryPath)) {
                qDebug() << "Failed to load geometry shader:" << program->log();
                delete program;
                return nullptr;
            }
        }
        
        // 链接程序
        if (!program->link()) {
            qDebug() << "Failed to link shader program:" << program->log();
            delete program;
            return nullptr;
        }
        
        return program;
    }
    
    // ✅ 从源代码创建着色器
    static QOpenGLShaderProgram* createShaderProgram(
        const char *vertexSource,
        const char *fragmentSource)
    {
        QOpenGLShaderProgram *program = new QOpenGLShaderProgram();
        
        if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource)) {
            qDebug() << "Vertex shader error:" << program->log();
            delete program;
            return nullptr;
        }
        
        if (!program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource)) {
            qDebug() << "Fragment shader error:" << program->log();
            delete program;
            return nullptr;
        }
        
        if (!program->link()) {
            qDebug() << "Linking error:" << program->log();
            delete program;
            return nullptr;
        }
        
        return program;
    }
};
```

### 4.2 常用着色器示例

```glsl
// ========== 基础顶点着色器 ==========
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

// ========== Phong 光照片段着色器 ==========
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main() {
    // 环境光
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;
    
    // 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb;
    
    // 镜面反射
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoord).rgb;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}

// ========== 法线可视化几何着色器 ==========
#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vec3 Normal[];

uniform mat4 projection;
uniform float normalLength;

void main() {
    for (int i = 0; i < 3; i++) {
        // 顶点位置
        gl_Position = projection * gl_in[i].gl_Position;
        EmitVertex();
        
        // 法线终点
        vec4 normalEnd = gl_in[i].gl_Position + vec4(Normal[i] * normalLength, 0.0);
        gl_Position = projection * normalEnd;
        EmitVertex();
        
        EndPrimitive();
    }
}

// ========== 天空盒着色器 ==========
// 顶点着色器
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;  // 确保深度为 1.0
}

// 片段着色器
#version 330 core
in vec3 TexCoords;
out vec4 FragColor;

uniform samplerCube skybox;

void main() {
    FragColor = texture(skybox, TexCoords);
}

// ========== 后处理效果着色器 ==========
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float time;

// 反色效果
vec4 invert() {
    return vec4(1.0 - texture(screenTexture, TexCoords).rgb, 1.0);
}

// 灰度效果
vec4 grayscale() {
    vec4 color = texture(screenTexture, TexCoords);
    float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    return vec4(average, average, average, 1.0);
}

// 模糊效果
vec4 blur() {
    float offset = 1.0 / 300.0;
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), vec2( 0.0,    offset), vec2( offset,  offset),
        vec2(-offset,  0.0),    vec2( 0.0,    0.0),    vec2( offset,  0.0),
        vec2(-offset, -offset), vec2( 0.0,   -offset), vec2( offset, -offset)
    );
    
    float kernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16
    );
    
    vec3 color = vec3(0.0);
    for(int i = 0; i < 9; i++)
        color += texture(screenTexture, TexCoords + offsets[i]).rgb * kernel[i];
    
    return vec4(color, 1.0);
}

void main() {
    FragColor = texture(screenTexture, TexCoords);
    // FragColor = invert();
    // FragColor = grayscale();
    // FragColor = blur();
}
```



---

## 5. 纹理与材质

### 5.1 QOpenGLTexture 基础

```cpp
#include <QOpenGLTexture>
#include <QImage>

class TextureManager {
public:
    // ✅ 加载 2D 纹理
    static QOpenGLTexture* load2DTexture(const QString &path) {
        QImage image(path);
        if (image.isNull()) {
            qDebug() << "Failed to load texture:" << path;
            return nullptr;
        }
        
        // 转换为 OpenGL 格式
        image = image.convertToFormat(QImage::Format_RGBA8888).mirrored();
        
        QOpenGLTexture *texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        texture->setData(image);
        
        // 设置纹理参数
        texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
        texture->setWrapMode(QOpenGLTexture::Repeat);
        
        qDebug() << "Texture loaded:" << path;
        return texture;
    }
    
    // ✅ 加载立方体贴图（天空盒）
    static QOpenGLTexture* loadCubemap(const QStringList &faces) {
        if (faces.size() != 6) {
            qDebug() << "Cubemap requires 6 faces";
            return nullptr;
        }
        
        QOpenGLTexture *texture = new QOpenGLTexture(QOpenGLTexture::TargetCubeMap);
        texture->create();
        texture->bind();
        
        // 加载 6 个面
        QOpenGLTexture::CubeMapFace cubeFaces[] = {
            QOpenGLTexture::CubeMapPositiveX,
            QOpenGLTexture::CubeMapNegativeX,
            QOpenGLTexture::CubeMapPositiveY,
            QOpenGLTexture::CubeMapNegativeY,
            QOpenGLTexture::CubeMapPositiveZ,
            QOpenGLTexture::CubeMapNegativeZ
        };
        
        for (int i = 0; i < 6; i++) {
            QImage image(faces[i]);
            if (image.isNull()) {
                qDebug() << "Failed to load cubemap face:" << faces[i];
                delete texture;
                return nullptr;
            }
            
            image = image.convertToFormat(QImage::Format_RGBA8888);
            texture->setData(0, 0, cubeFaces[i], 
                           QOpenGLTexture::RGBA, 
                           QOpenGLTexture::UInt8, 
                           image.constBits());
        }
        
        texture->setMinificationFilter(QOpenGLTexture::Linear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
        texture->setWrapMode(QOpenGLTexture::ClampToEdge);
        
        return texture;
    }
    
    // ✅ 创建程序化纹理
    static QOpenGLTexture* createCheckerboardTexture(int size = 256) {
        QImage image(size, size, QImage::Format_RGBA8888);
        
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                bool isWhite = ((x / 32) + (y / 32)) % 2 == 0;
                QColor color = isWhite ? Qt::white : Qt::black;
                image.setPixelColor(x, y, color);
            }
        }
        
        QOpenGLTexture *texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        texture->setData(image.mirrored());
        texture->setMinificationFilter(QOpenGLTexture::Linear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
        texture->setWrapMode(QOpenGLTexture::Repeat);
        
        return texture;
    }
};

// 使用示例
class TexturedCube : public QOpenGLWidget, protected QOpenGLFunctions {
protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        
        // 加载纹理
        m_diffuseTexture = TextureManager::load2DTexture(":/textures/container_diffuse.png");
        m_specularTexture = TextureManager::load2DTexture(":/textures/container_specular.png");
        
        // 初始化着色器和几何体...
    }
    
    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_program->bind();
        
        // 绑定纹理
        glActiveTexture(GL_TEXTURE0);
        m_diffuseTexture->bind();
        m_program->setUniformValue("material.diffuse", 0);
        
        glActiveTexture(GL_TEXTURE1);
        m_specularTexture->bind();
        m_program->setUniformValue("material.specular", 1);
        
        // 绘制...
        
        m_program->release();
    }
    
private:
    QOpenGLTexture *m_diffuseTexture = nullptr;
    QOpenGLTexture *m_specularTexture = nullptr;
};
```

### 5.2 纹理过滤与环绕模式

```cpp
// 纹理过滤模式
texture->setMinificationFilter(QOpenGLTexture::Nearest);           // 最近邻
texture->setMinificationFilter(QOpenGLTexture::Linear);            // 线性
texture->setMinificationFilter(QOpenGLTexture::NearestMipMapNearest);
texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);

texture->setMagnificationFilter(QOpenGLTexture::Nearest);
texture->setMagnificationFilter(QOpenGLTexture::Linear);

// 纹理环绕模式
texture->setWrapMode(QOpenGLTexture::Repeat);          // 重复
texture->setWrapMode(QOpenGLTexture::MirroredRepeat);  // 镜像重复
texture->setWrapMode(QOpenGLTexture::ClampToEdge);     // 边缘拉伸
texture->setWrapMode(QOpenGLTexture::ClampToBorder);   // 边界颜色

// 分别设置 S、T、R 方向
texture->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::Repeat);
texture->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::ClampToEdge);
```

---

## 6. 3D 变换与相机

### 6.1 变换矩阵

```cpp
#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>

class Transform {
public:
    QVector3D position{0.0f, 0.0f, 0.0f};
    QVector3D rotation{0.0f, 0.0f, 0.0f};  // 欧拉角（度）
    QVector3D scale{1.0f, 1.0f, 1.0f};
    
    // ✅ 获取模型矩阵
    QMatrix4x4 getModelMatrix() const {
        QMatrix4x4 matrix;
        
        // 平移
        matrix.translate(position);
        
        // 旋转（按 Y-X-Z 顺序）
        matrix.rotate(rotation.y(), 0, 1, 0);
        matrix.rotate(rotation.x(), 1, 0, 0);
        matrix.rotate(rotation.z(), 0, 0, 1);
        
        // 缩放
        matrix.scale(scale);
        
        return matrix;
    }
    
    // ✅ 使用四元数旋转
    QMatrix4x4 getModelMatrixWithQuaternion(const QQuaternion &quat) const {
        QMatrix4x4 matrix;
        matrix.translate(position);
        matrix.rotate(quat);
        matrix.scale(scale);
        return matrix;
    }
    
    // ✅ 前向向量
    QVector3D forward() const {
        QMatrix4x4 rotMat;
        rotMat.rotate(rotation.y(), 0, 1, 0);
        rotMat.rotate(rotation.x(), 1, 0, 0);
        rotMat.rotate(rotation.z(), 0, 0, 1);
        return (rotMat * QVector4D(0, 0, -1, 0)).toVector3D().normalized();
    }
    
    // ✅ 右向向量
    QVector3D right() const {
        QMatrix4x4 rotMat;
        rotMat.rotate(rotation.y(), 0, 1, 0);
        rotMat.rotate(rotation.x(), 1, 0, 0);
        rotMat.rotate(rotation.z(), 0, 0, 1);
        return (rotMat * QVector4D(1, 0, 0, 0)).toVector3D().normalized();
    }
    
    // ✅ 上向向量
    QVector3D up() const {
        QMatrix4x4 rotMat;
        rotMat.rotate(rotation.y(), 0, 1, 0);
        rotMat.rotate(rotation.x(), 1, 0, 0);
        rotMat.rotate(rotation.z(), 0, 0, 1);
        return (rotMat * QVector4D(0, 1, 0, 0)).toVector3D().normalized();
    }
};
```

### 6.2 相机系统

```cpp
class Camera {
public:
    enum class ProjectionType {
        Perspective,
        Orthographic
    };
    
    Camera() = default;
    
    // ✅ 设置透视投影
    void setPerspective(float fov, float aspect, float near, float far) {
        m_projectionType = ProjectionType::Perspective;
        m_fov = fov;
        m_aspect = aspect;
        m_near = near;
        m_far = far;
        updateProjectionMatrix();
    }
    
    // ✅ 设置正交投影
    void setOrthographic(float left, float right, float bottom, float top, float near, float far) {
        m_projectionType = ProjectionType::Orthographic;
        m_projection.setToIdentity();
        m_projection.ortho(left, right, bottom, top, near, far);
    }
    
    // ✅ 获取视图矩阵
    QMatrix4x4 getViewMatrix() const {
        QMatrix4x4 view;
        view.lookAt(m_position, m_position + m_front, m_up);
        return view;
    }
    
    // ✅ 获取投影矩阵
    QMatrix4x4 getProjectionMatrix() const {
        return m_projection;
    }
    
    // ✅ 处理鼠标移动（FPS 相机）
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= m_mouseSensitivity;
        yoffset *= m_mouseSensitivity;
        
        m_yaw += xoffset;
        m_pitch += yoffset;
        
        if (constrainPitch) {
            if (m_pitch > 89.0f) m_pitch = 89.0f;
            if (m_pitch < -89.0f) m_pitch = -89.0f;
        }
        
        updateCameraVectors();
    }
    
    // ✅ 处理鼠标滚轮（缩放）
    void processMouseScroll(float yoffset) {
        m_fov -= yoffset;
        if (m_fov < 1.0f) m_fov = 1.0f;
        if (m_fov > 45.0f) m_fov = 45.0f;
        updateProjectionMatrix();
    }
    
    // ✅ 移动相机
    void moveForward(float deltaTime) {
        m_position += m_front * m_movementSpeed * deltaTime;
    }
    
    void moveBackward(float deltaTime) {
        m_position -= m_front * m_movementSpeed * deltaTime;
    }
    
    void moveLeft(float deltaTime) {
        m_position -= m_right * m_movementSpeed * deltaTime;
    }
    
    void moveRight(float deltaTime) {
        m_position += m_right * m_movementSpeed * deltaTime;
    }
    
    void moveUp(float deltaTime) {
        m_position += m_up * m_movementSpeed * deltaTime;
    }
    
    void moveDown(float deltaTime) {
        m_position -= m_up * m_movementSpeed * deltaTime;
    }
    
    // Getters
    QVector3D position() const { return m_position; }
    QVector3D front() const { return m_front; }
    QVector3D up() const { return m_up; }
    QVector3D right() const { return m_right; }
    float fov() const { return m_fov; }
    
    // Setters
    void setPosition(const QVector3D &pos) { m_position = pos; }
    void setMovementSpeed(float speed) { m_movementSpeed = speed; }
    void setMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }
    
private:
    void updateCameraVectors() {
        // 计算新的前向向量
        QVector3D front;
        front.setX(cos(qDegreesToRadians(m_yaw)) * cos(qDegreesToRadians(m_pitch)));
        front.setY(sin(qDegreesToRadians(m_pitch)));
        front.setZ(sin(qDegreesToRadians(m_yaw)) * cos(qDegreesToRadians(m_pitch)));
        m_front = front.normalized();
        
        // 重新计算右向和上向向量
        m_right = QVector3D::crossProduct(m_front, m_worldUp).normalized();
        m_up = QVector3D::crossProduct(m_right, m_front).normalized();
    }
    
    void updateProjectionMatrix() {
        if (m_projectionType == ProjectionType::Perspective) {
            m_projection.setToIdentity();
            m_projection.perspective(m_fov, m_aspect, m_near, m_far);
        }
    }
    
    // 相机属性
    QVector3D m_position{0.0f, 0.0f, 3.0f};
    QVector3D m_front{0.0f, 0.0f, -1.0f};
    QVector3D m_up{0.0f, 1.0f, 0.0f};
    QVector3D m_right{1.0f, 0.0f, 0.0f};
    QVector3D m_worldUp{0.0f, 1.0f, 0.0f};
    
    // 欧拉角
    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    
    // 相机选项
    float m_movementSpeed = 2.5f;
    float m_mouseSensitivity = 0.1f;
    float m_fov = 45.0f;
    
    // 投影参数
    ProjectionType m_projectionType = ProjectionType::Perspective;
    QMatrix4x4 m_projection;
    float m_aspect = 1.0f;
    float m_near = 0.1f;
    float m_far = 100.0f;
};

// 使用示例
class CameraWidget : public QOpenGLWidget {
protected:
    void initializeGL() override {
        // 初始化相机
        m_camera.setPerspective(45.0f, float(width()) / float(height()), 0.1f, 100.0f);
        m_camera.setPosition(QVector3D(0.0f, 0.0f, 5.0f));
    }
    
    void paintGL() override {
        m_program->bind();
        
        // 设置相机矩阵
        m_program->setUniformValue("view", m_camera.getViewMatrix());
        m_program->setUniformValue("projection", m_camera.getProjectionMatrix());
        m_program->setUniformValue("viewPos", m_camera.position());
        
        // 绘制场景...
        
        m_program->release();
    }
    
    void keyPressEvent(QKeyEvent *event) override {
        float deltaTime = 0.016f;  // 假设 60 FPS
        
        switch (event->key()) {
            case Qt::Key_W: m_camera.moveForward(deltaTime); break;
            case Qt::Key_S: m_camera.moveBackward(deltaTime); break;
            case Qt::Key_A: m_camera.moveLeft(deltaTime); break;
            case Qt::Key_D: m_camera.moveRight(deltaTime); break;
            case Qt::Key_Space: m_camera.moveUp(deltaTime); break;
            case Qt::Key_Control: m_camera.moveDown(deltaTime); break;
        }
        
        update();
    }
    
    void mouseMoveEvent(QMouseEvent *event) override {
        if (m_firstMouse) {
            m_lastMousePos = event->pos();
            m_firstMouse = false;
        }
        
        float xoffset = event->pos().x() - m_lastMousePos.x();
        float yoffset = m_lastMousePos.y() - event->pos().y();
        m_lastMousePos = event->pos();
        
        m_camera.processMouseMovement(xoffset, yoffset);
        update();
    }
    
    void wheelEvent(QWheelEvent *event) override {
        m_camera.processMouseScroll(event->angleDelta().y() / 120.0f);
        update();
    }
    
private:
    Camera m_camera;
    QPoint m_lastMousePos;
    bool m_firstMouse = true;
};
```



---

## 7. 光照与阴影

### 7.1 Phong 光照模型

```cpp
// 光源结构
struct Light {
    QVector3D position;
    QVector3D ambient{0.2f, 0.2f, 0.2f};
    QVector3D diffuse{0.5f, 0.5f, 0.5f};
    QVector3D specular{1.0f, 1.0f, 1.0f};
    
    // 衰减参数
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};

// 材质结构
struct Material {
    QVector3D ambient{1.0f, 0.5f, 0.31f};
    QVector3D diffuse{1.0f, 0.5f, 0.31f};
    QVector3D specular{0.5f, 0.5f, 0.5f};
    float shininess = 32.0f;
};

// 在着色器中设置光照
void setupLighting(QOpenGLShaderProgram *program, const Light &light, const Material &material) {
    // 光源属性
    program->setUniformValue("light.position", light.position);
    program->setUniformValue("light.ambient", light.ambient);
    program->setUniformValue("light.diffuse", light.diffuse);
    program->setUniformValue("light.specular", light.specular);
    program->setUniformValue("light.constant", light.constant);
    program->setUniformValue("light.linear", light.linear);
    program->setUniformValue("light.quadratic", light.quadratic);
    
    // 材质属性
    program->setUniformValue("material.ambient", material.ambient);
    program->setUniformValue("material.diffuse", material.diffuse);
    program->setUniformValue("material.specular", material.specular);
    program->setUniformValue("material.shininess", material.shininess);
}
```

```glsl
// Phong 光照片段着色器
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main() {
    // 环境光
    vec3 ambient = light.ambient * material.ambient;
    
    // 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
    // 镜面反射
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);
    
    // 衰减
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```

### 7.2 多光源系统

```glsl
// 多光源片段着色器
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

#define NR_POINT_LIGHTS 4

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform vec3 viewPos;

// 函数声明
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // 方向光
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    
    // 点光源
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    
    // 聚光灯
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
    
    FragColor = vec4(result, 1.0);
}

// 计算方向光
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    
    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // 合并结果
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));
    
    return (ambient + diffuse + specular);
}

// 计算点光源
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    
    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));
    
    // 合并结果
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

// 计算聚光灯
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    
    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));
    
    // 聚光灯强度
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // 合并结果
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return (ambient + diffuse + specular);
}
```

### 7.3 阴影映射

```cpp
class ShadowMapping : public QOpenGLWidget, protected QOpenGLFunctions {
protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        
        // 创建深度贴图 FBO
        const int SHADOW_WIDTH = 1024;
        const int SHADOW_HEIGHT = 1024;
        
        glGenFramebuffers(1, &m_depthMapFBO);
        
        // 创建深度纹理
        glGenTextures(1, &m_depthMap);
        glBindTexture(GL_TEXTURE_2D, m_depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        
        // 附加深度纹理到 FBO
        glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // 初始化着色器
        initShadowShaders();
    }
    
    void paintGL() override {
        // 1. 渲染深度贴图
        renderDepthMap();
        
        // 2. 正常渲染场景（使用阴影）
        renderScene();
    }
    
private:
    void renderDepthMap() {
        glViewport(0, 0, 1024, 1024);
        glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        // 配置光源空间变换矩阵
        QMatrix4x4 lightProjection;
        lightProjection.ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
        
        QMatrix4x4 lightView;
        lightView.lookAt(m_lightPos, QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
        
        QMatrix4x4 lightSpaceMatrix = lightProjection * lightView;
        
        m_depthShader->bind();
        m_depthShader->setUniformValue("lightSpaceMatrix", lightSpaceMatrix);
        
        // 渲染场景几何体...
        renderSceneGeometry();
        
        m_depthShader->release();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    void renderScene() {
        glViewport(0, 0, width(), height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_sceneShader->bind();
        
        // 设置变换矩阵
        m_sceneShader->setUniformValue("projection", m_projection);
        m_sceneShader->setUniformValue("view", m_view);
        
        // 绑定阴影贴图
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_depthMap);
        m_sceneShader->setUniformValue("shadowMap", 1);
        
        // 渲染场景...
        renderSceneGeometry();
        
        m_sceneShader->release();
    }
    
    void initShadowShaders() {
        // 深度着色器
        m_depthShader = new QOpenGLShaderProgram(this);
        m_depthShader->addShaderFromSourceCode(QOpenGLShader::Vertex, R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            
            uniform mat4 lightSpaceMatrix;
            uniform mat4 model;
            
            void main() {
                gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
            }
        )");
        m_depthShader->addShaderFromSourceCode(QOpenGLShader::Fragment, R"(
            #version 330 core
            void main() {
                // gl_FragDepth 自动写入
            }
        )");
        m_depthShader->link();
        
        // 场景着色器（带阴影）
        m_sceneShader = new QOpenGLShaderProgram(this);
        m_sceneShader->addShaderFromSourceCode(QOpenGLShader::Vertex, R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec2 aTexCoord;
            
            out vec3 FragPos;
            out vec3 Normal;
            out vec2 TexCoord;
            out vec4 FragPosLightSpace;
            
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            uniform mat4 lightSpaceMatrix;
            
            void main() {
                FragPos = vec3(model * vec4(aPos, 1.0));
                Normal = mat3(transpose(inverse(model))) * aNormal;
                TexCoord = aTexCoord;
                FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
                gl_Position = projection * view * vec4(FragPos, 1.0);
            }
        )");
        m_sceneShader->addShaderFromSourceCode(QOpenGLShader::Fragment, R"(
            #version 330 core
            in vec3 FragPos;
            in vec3 Normal;
            in vec2 TexCoord;
            in vec4 FragPosLightSpace;
            
            out vec4 FragColor;
            
            uniform sampler2D diffuseTexture;
            uniform sampler2D shadowMap;
            uniform vec3 lightPos;
            uniform vec3 viewPos;
            
            float ShadowCalculation(vec4 fragPosLightSpace) {
                // 透视除法
                vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
                // 变换到 [0,1] 范围
                projCoords = projCoords * 0.5 + 0.5;
                // 获取最近深度值
                float closestDepth = texture(shadowMap, projCoords.xy).r;
                // 获取当前片段深度
                float currentDepth = projCoords.z;
                // 检查是否在阴影中
                float bias = 0.005;
                float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
                
                // PCF 软阴影
                shadow = 0.0;
                vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
                for(int x = -1; x <= 1; ++x) {
                    for(int y = -1; y <= 1; ++y) {
                        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
                    }
                }
                shadow /= 9.0;
                
                // 超出光源视锥范围时不产生阴影
                if(projCoords.z > 1.0)
                    shadow = 0.0;
                
                return shadow;
            }
            
            void main() {
                vec3 color = texture(diffuseTexture, TexCoord).rgb;
                vec3 normal = normalize(Normal);
                vec3 lightColor = vec3(1.0);
                
                // 环境光
                vec3 ambient = 0.15 * color;
                
                // 漫反射
                vec3 lightDir = normalize(lightPos - FragPos);
                float diff = max(dot(lightDir, normal), 0.0);
                vec3 diffuse = diff * lightColor;
                
                // 镜面反射
                vec3 viewDir = normalize(viewPos - FragPos);
                vec3 reflectDir = reflect(-lightDir, normal);
                float spec = 0.0;
                vec3 halfwayDir = normalize(lightDir + viewDir);
                spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
                vec3 specular = spec * lightColor;
                
                // 计算阴影
                float shadow = ShadowCalculation(FragPosLightSpace);
                vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
                
                FragColor = vec4(lighting, 1.0);
            }
        )");
        m_sceneShader->link();
    }
    
    GLuint m_depthMapFBO;
    GLuint m_depthMap;
    QOpenGLShaderProgram *m_depthShader = nullptr;
    QOpenGLShaderProgram *m_sceneShader = nullptr;
    QVector3D m_lightPos{-2.0f, 4.0f, -1.0f};
};
```



---

## 8. QML 中的 OpenGL

### 8.1 QQuickFramebufferObject 自定义渲染

```cpp
// customopenglitem.h
#include <QQuickFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class CustomOpenGLItem : public QQuickFramebufferObject {
    Q_OBJECT
    Q_PROPERTY(float rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool animate READ animate WRITE setAnimate NOTIFY animateChanged)
    
public:
    explicit CustomOpenGLItem(QQuickItem *parent = nullptr);
    
    Renderer *createRenderer() const override;
    
    float rotation() const { return m_rotation; }
    void setRotation(float rotation) {
        if (qFuzzyCompare(m_rotation, rotation))
            return;
        m_rotation = rotation;
        emit rotationChanged();
        update();
    }
    
    QColor color() const { return m_color; }
    void setColor(const QColor &color) {
        if (m_color == color)
            return;
        m_color = color;
        emit colorChanged();
        update();
    }
    
    bool animate() const { return m_animate; }
    void setAnimate(bool animate) {
        if (m_animate == animate)
            return;
        m_animate = animate;
        emit animateChanged();
        if (m_animate)
            update();
    }
    
signals:
    void rotationChanged();
    void colorChanged();
    void animateChanged();
    
private:
    float m_rotation = 0.0f;
    QColor m_color = Qt::red;
    bool m_animate = false;
};

// 渲染器类
class CustomOpenGLRenderer : public QQuickFramebufferObject::Renderer, 
                             protected QOpenGLFunctions {
public:
    CustomOpenGLRenderer();
    ~CustomOpenGLRenderer();
    
    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
    void synchronize(QQuickFramebufferObject *item) override;
    
private:
    void initializeGL();
    void setupGeometry();
    
    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_ebo{QOpenGLBuffer::IndexBuffer};
    QOpenGLVertexArrayObject m_vao;
    
    float m_rotation = 0.0f;
    QColor m_color;
    bool m_animate = false;
    bool m_initialized = false;
    
    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
};

// customopenglitem.cpp
CustomOpenGLItem::CustomOpenGLItem(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    setMirrorVertically(true);
    
    // 动画定时器
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (m_animate) {
            m_rotation += 1.0f;
            if (m_rotation >= 360.0f)
                m_rotation -= 360.0f;
            emit rotationChanged();
            update();
        }
    });
    timer->start(16);  // ~60 FPS
}

QQuickFramebufferObject::Renderer *CustomOpenGLItem::createRenderer() const {
    return new CustomOpenGLRenderer();
}

CustomOpenGLRenderer::CustomOpenGLRenderer() {
    initializeOpenGLFunctions();
}

CustomOpenGLRenderer::~CustomOpenGLRenderer() {
    delete m_program;
}

void CustomOpenGLRenderer::render() {
    if (!m_initialized) {
        initializeGL();
        m_initialized = true;
    }
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    
    m_program->bind();
    
    // 更新模型矩阵
    QMatrix4x4 model;
    model.rotate(m_rotation, 0.0f, 1.0f, 0.0f);
    
    QMatrix4x4 mvp = m_projection * m_view * model;
    
    m_program->setUniformValue("mvp", mvp);
    m_program->setUniformValue("color", m_color);
    
    m_vao.bind();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    m_vao.release();
    
    m_program->release();
}

void CustomOpenGLRenderer::initializeGL() {
    m_program = new QOpenGLShaderProgram();
    
    const char *vertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        
        out vec3 Normal;
        out vec3 FragPos;
        
        uniform mat4 mvp;
        
        void main() {
            Normal = aNormal;
            FragPos = aPos;
            gl_Position = mvp * vec4(aPos, 1.0);
        }
    )";
    
    const char *fragmentShader = R"(
        #version 330 core
        in vec3 Normal;
        in vec3 FragPos;
        
        out vec4 FragColor;
        
        uniform vec4 color;
        
        void main() {
            vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
            float diff = max(dot(normalize(Normal), lightDir), 0.0);
            vec3 result = color.rgb * (0.3 + 0.7 * diff);
            FragColor = vec4(result, color.a);
        }
    )";
    
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);
    m_program->link();
    
    setupGeometry();
    
    // 设置相机
    m_view.setToIdentity();
    m_view.translate(0, 0, -5);
}

void CustomOpenGLRenderer::setupGeometry() {
    // 立方体顶点
    float vertices[] = {
        // 位置              // 法线
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
    
    m_vao.create();
    m_vao.bind();
    
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices, sizeof(vertices));
    
    m_ebo.create();
    m_ebo.bind();
    m_ebo.allocate(indices, sizeof(indices));
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    m_vao.release();
}

void CustomOpenGLRenderer::synchronize(QQuickFramebufferObject *item) {
    CustomOpenGLItem *glItem = static_cast<CustomOpenGLItem*>(item);
    m_rotation = glItem->rotation();
    m_color = glItem->color();
    m_animate = glItem->animate();
}

QOpenGLFramebufferObject *CustomOpenGLRenderer::createFramebufferObject(const QSize &size) {
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, float(size.width()) / float(size.height()), 0.1f, 100.0f);
    
    return new QOpenGLFramebufferObject(size, format);
}
```

### 8.2 在 QML 中使用

```qml
// Main.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MyApp 1.0  // 注册的自定义模块

Window {
    width: 1200
    height: 800
    visible: true
    title: "Qt6 OpenGL in QML"
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // 左侧：OpenGL 渲染区域
        CustomOpenGLItem {
            id: openglItem
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            rotation: rotationSlider.value
            color: Qt.hsla(hueSlider.value, satSlider.value, lightSlider.value, 1.0)
            animate: animateCheck.checked
        }
        
        // 右侧：控制面板
        ColumnLayout {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            spacing: 15
            
            GroupBox {
                title: "旋转控制"
                Layout.fillWidth: true
                
                ColumnLayout {
                    anchors.fill: parent
                    
                    Label {
                        text: "旋转角度: " + Math.round(rotationSlider.value) + "°"
                    }
                    
                    Slider {
                        id: rotationSlider
                        Layout.fillWidth: true
                        from: 0
                        to: 360
                        value: 0
                        enabled: !animateCheck.checked
                    }
                    
                    CheckBox {
                        id: animateCheck
                        text: "自动旋转"
                        checked: false
                    }
                }
            }
            
            GroupBox {
                title: "颜色控制"
                Layout.fillWidth: true
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    
                    RowLayout {
                        Label { text: "色调:" }
                        Slider {
                            id: hueSlider
                            Layout.fillWidth: true
                            from: 0
                            to: 1
                            value: 0
                        }
                        Label { text: Math.round(hueSlider.value * 360) + "°" }
                    }
                    
                    RowLayout {
                        Label { text: "饱和度:" }
                        Slider {
                            id: satSlider
                            Layout.fillWidth: true
                            from: 0
                            to: 1
                            value: 1
                        }
                        Label { text: Math.round(satSlider.value * 100) + "%" }
                    }
                    
                    RowLayout {
                        Label { text: "亮度:" }
                        Slider {
                            id: lightSlider
                            Layout.fillWidth: true
                            from: 0
                            to: 1
                            value: 0.5
                        }
                        Label { text: Math.round(lightSlider.value * 100) + "%" }
                    }
                    
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        color: openglItem.color
                        border.color: "black"
                        border.width: 1
                        
                        Label {
                            anchors.centerIn: parent
                            text: "当前颜色"
                            color: lightSlider.value > 0.5 ? "black" : "white"
                        }
                    }
                }
            }
            
            GroupBox {
                title: "预设颜色"
                Layout.fillWidth: true
                
                GridLayout {
                    anchors.fill: parent
                    columns: 3
                    rowSpacing: 5
                    columnSpacing: 5
                    
                    Repeater {
                        model: [
                            { name: "红色", color: "#FF0000" },
                            { name: "绿色", color: "#00FF00" },
                            { name: "蓝色", color: "#0000FF" },
                            { name: "黄色", color: "#FFFF00" },
                            { name: "紫色", color: "#FF00FF" },
                            { name: "青色", color: "#00FFFF" }
                        ]
                        
                        Button {
                            Layout.fillWidth: true
                            text: modelData.name
                            
                            background: Rectangle {
                                color: modelData.color
                                border.color: "black"
                                border.width: 1
                                radius: 4
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.bold: true
                                style: Text.Outline
                                styleColor: "black"
                            }
                            
                            onClicked: {
                                openglItem.color = modelData.color
                            }
                        }
                    }
                }
            }
            
            GroupBox {
                title: "性能信息"
                Layout.fillWidth: true
                
                ColumnLayout {
                    anchors.fill: parent
                    
                    Label {
                        text: "FPS: " + fpsCounter.fps
                        font.pixelSize: 14
                    }
                    
                    Label {
                        text: "帧时间: " + fpsCounter.frameTime + " ms"
                        font.pixelSize: 14
                    }
                    
                    ProgressBar {
                        Layout.fillWidth: true
                        from: 0
                        to: 60
                        value: fpsCounter.fps
                    }
                }
            }
            
            Item { Layout.fillHeight: true }
        }
    }
    
    // FPS 计数器
    QtObject {
        id: fpsCounter
        property int fps: 0
        property real frameTime: 0
        property int frameCount: 0
        property real lastTime: Date.now()
        
        function update() {
            frameCount++
            var currentTime = Date.now()
            var deltaTime = currentTime - lastTime
            
            if (deltaTime >= 1000) {
                fps = Math.round(frameCount * 1000 / deltaTime)
                frameTime = (deltaTime / frameCount).toFixed(2)
                frameCount = 0
                lastTime = currentTime
            }
        }
    }
    
    Timer {
        interval: 16
        running: true
        repeat: true
        onTriggered: fpsCounter.update()
    }
}
```

### 8.3 注册 QML 类型

```cpp
// main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "customopenglitem.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // 设置 OpenGL 格式
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);
    
    // 注册 QML 类型
    qmlRegisterType<CustomOpenGLItem>("MyApp", 1, 0, "CustomOpenGLItem");
    
    QQmlApplicationEngine engine;
    engine.loadFromModule("MyApp", "Main");
    
    if (engine.rootObjects().isEmpty())
        return -1;
    
    return app.exec();
}
```



---

## 9. Qt Quick 3D

### 9.1 Qt Quick 3D 基础

```qml
// QtQuick3DExample.qml
import QtQuick
import QtQuick3D
import QtQuick.Controls

Window {
    width: 1280
    height: 720
    visible: true
    title: "Qt Quick 3D Example"
    
    View3D {
        id: view3D
        anchors.fill: parent
        
        // 环境设置
        environment: SceneEnvironment {
            clearColor: "#222222"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }
        
        // 相机
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 200, 300)
            eulerRotation.x: -30
            
            // 相机动画
            SequentialAnimation on position.y {
                running: animateCamera.checked
                loops: Animation.Infinite
                NumberAnimation { from: 200; to: 300; duration: 2000; easing.type: Easing.InOutQuad }
                NumberAnimation { from: 300; to: 200; duration: 2000; easing.type: Easing.InOutQuad }
            }
        }
        
        // 方向光
        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            brightness: 1.0
            castsShadow: true
        }
        
        // 点光源
        PointLight {
            position: Qt.vector3d(0, 200, 0)
            brightness: 2.0
            color: "#ff8800"
            castsShadow: true
            shadowMapQuality: Light.ShadowMapQualityHigh
            
            // 光源动画
            SequentialAnimation on position.x {
                running: true
                loops: Animation.Infinite
                NumberAnimation { from: -200; to: 200; duration: 3000 }
                NumberAnimation { from: 200; to: -200; duration: 3000 }
            }
        }
        
        // 地面
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0, -50, 0)
            eulerRotation.x: -90
            scale: Qt.vector3d(10, 10, 1)
            
            materials: PrincipledMaterial {
                baseColor: "#404040"
                metalness: 0.5
                roughness: 0.5
            }
            
            castsShadows: false
            receivesShadows: true
        }
        
        // 旋转的立方体
        Model {
            id: cube
            source: "#Cube"
            position: Qt.vector3d(0, 0, 0)
            scale: Qt.vector3d(1, 1, 1)
            
            materials: PrincipledMaterial {
                baseColor: cubeColorPicker.color
                metalness: metalnessSlider.value
                roughness: roughnessSlider.value
                normalMap: Texture {
                    source: "textures/normal_map.png"
                }
            }
            
            castsShadows: true
            receivesShadows: true
            
            // 旋转动画
            NumberAnimation on eulerRotation.y {
                running: true
                loops: Animation.Infinite
                from: 0
                to: 360
                duration: 5000
            }
        }
        
        // 球体
        Model {
            source: "#Sphere"
            position: Qt.vector3d(-150, 0, 0)
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            
            materials: PrincipledMaterial {
                baseColor: "#ff0000"
                metalness: 1.0
                roughness: 0.2
            }
            
            castsShadows: true
        }
        
        // 圆柱体
        Model {
            source: "#Cylinder"
            position: Qt.vector3d(150, 0, 0)
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            
            materials: PrincipledMaterial {
                baseColor: "#00ff00"
                metalness: 0.0
                roughness: 0.8
            }
            
            castsShadows: true
        }
        
        // 自定义模型（从文件加载）
        Model {
            source: "models/custom_model.mesh"
            position: Qt.vector3d(0, 100, -100)
            
            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    source: "textures/diffuse.png"
                }
                metalnessMap: Texture {
                    source: "textures/metalness.png"
                }
                roughnessMap: Texture {
                    source: "textures/roughness.png"
                }
                normalMap: Texture {
                    source: "textures/normal.png"
                }
            }
        }
    }
    
    // 控制面板
    Column {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        spacing: 10
        
        Rectangle {
            width: 250
            height: controlColumn.height + 20
            color: "#80000000"
            radius: 10
            
            Column {
                id: controlColumn
                anchors.centerIn: parent
                width: parent.width - 20
                spacing: 10
                
                Text {
                    text: "控制面板"
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                }
                
                CheckBox {
                    id: animateCamera
                    text: "相机动画"
                    checked: false
                }
                
                Text {
                    text: "立方体颜色"
                    color: "white"
                }
                
                Rectangle {
                    id: cubeColorPicker
                    width: parent.width
                    height: 40
                    color: "#4080ff"
                    border.color: "white"
                    border.width: 2
                    radius: 5
                    
                    property color color: "#4080ff"
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            // 循环切换颜色
                            var colors = ["#4080ff", "#ff4080", "#80ff40", "#ff8040"]
                            var currentIndex = colors.indexOf(parent.color.toString())
                            parent.color = colors[(currentIndex + 1) % colors.length]
                        }
                    }
                }
                
                Text {
                    text: "金属度: " + metalnessSlider.value.toFixed(2)
                    color: "white"
                }
                
                Slider {
                    id: metalnessSlider
                    width: parent.width
                    from: 0
                    to: 1
                    value: 0.5
                }
                
                Text {
                    text: "粗糙度: " + roughnessSlider.value.toFixed(2)
                    color: "white"
                }
                
                Slider {
                    id: roughnessSlider
                    width: parent.width
                    from: 0
                    to: 1
                    value: 0.5
                }
            }
        }
    }
}
```

### 9.2 自定义 Qt Quick 3D 材质

```qml
// CustomMaterial.qml
import QtQuick
import QtQuick3D

CustomMaterial {
    property real time: 0.0
    property color baseColor: "#ffffff"
    property real waveAmplitude: 0.1
    property real waveFrequency: 5.0
    
    // 顶点着色器
    vertexShader: "
        varying vec3 vPosition;
        varying vec3 vNormal;
        varying vec2 vTexCoord;
        
        uniform float time;
        uniform float waveAmplitude;
        uniform float waveFrequency;
        
        void MAIN() {
            vPosition = VERTEX.xyz;
            vNormal = NORMAL;
            vTexCoord = UV0;
            
            // 波浪效果
            float wave = sin(VERTEX.x * waveFrequency + time) * 
                        cos(VERTEX.z * waveFrequency + time) * waveAmplitude;
            VERTEX.y += wave;
            
            // 更新法线
            vec3 tangent = vec3(1.0, 
                               cos(VERTEX.x * waveFrequency + time) * waveFrequency * waveAmplitude, 
                               0.0);
            vec3 bitangent = vec3(0.0, 
                                 cos(VERTEX.z * waveFrequency + time) * waveFrequency * waveAmplitude, 
                                 1.0);
            NORMAL = normalize(cross(tangent, bitangent));
        }
    "
    
    // 片段着色器
    fragmentShader: "
        varying vec3 vPosition;
        varying vec3 vNormal;
        varying vec2 vTexCoord;
        
        uniform vec3 baseColor;
        uniform float time;
        
        void MAIN() {
            vec3 normal = normalize(vNormal);
            vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
            
            float diff = max(dot(normal, lightDir), 0.0);
            
            // 动态颜色
            vec3 color = baseColor * (0.3 + 0.7 * diff);
            color += vec3(sin(time + vPosition.x), 
                         cos(time + vPosition.y), 
                         sin(time + vPosition.z)) * 0.1;
            
            BASE_COLOR = vec4(color, 1.0);
        }
    "
    
    // 动画
    NumberAnimation on time {
        running: true
        loops: Animation.Infinite
        from: 0
        to: Math.PI * 2
        duration: 5000
    }
}
```

---

## 10. 自定义 QML OpenGL 渲染

### 10.1 QSGRenderNode 自定义渲染节点

```cpp
// customrendernode.h
#include <QSGRenderNode>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class CustomRenderNode : public QSGRenderNode, protected QOpenGLFunctions {
public:
    CustomRenderNode();
    ~CustomRenderNode();
    
    void render(const RenderState *state) override;
    void releaseResources() override;
    StateFlags changedStates() const override;
    RenderingFlags flags() const override;
    QRectF rect() const override;
    
    void setColor(const QColor &color) { m_color = color; }
    void setRotation(float rotation) { m_rotation = rotation; }
    void setRect(const QRectF &rect) { m_rect = rect; }
    
private:
    void initialize();
    
    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_vao;
    
    QColor m_color;
    float m_rotation = 0.0f;
    QRectF m_rect;
    bool m_initialized = false;
};

// customrendernode.cpp
CustomRenderNode::CustomRenderNode() {
    initializeOpenGLFunctions();
}

CustomRenderNode::~CustomRenderNode() {
    releaseResources();
}

void CustomRenderNode::render(const RenderState *state) {
    if (!m_initialized) {
        initialize();
        m_initialized = true;
    }
    
    m_program->bind();
    
    // 设置变换矩阵
    QMatrix4x4 matrix = *state->projectionMatrix();
    matrix.translate(m_rect.center().x(), m_rect.center().y());
    matrix.rotate(m_rotation, 0, 0, 1);
    matrix.scale(m_rect.width() / 2, m_rect.height() / 2);
    
    m_program->setUniformValue("matrix", matrix);
    m_program->setUniformValue("color", m_color);
    m_program->setUniformValue("opacity", inheritedOpacity());
    
    // 绘制
    m_vao.bind();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    m_vao.release();
    
    m_program->release();
}

void CustomRenderNode::initialize() {
    m_program = new QOpenGLShaderProgram();
    
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        
        uniform mat4 matrix;
        
        void main() {
            gl_Position = matrix * vec4(aPos, 0.0, 1.0);
        }
    )");
    
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, R"(
        #version 330 core
        uniform vec4 color;
        uniform float opacity;
        
        out vec4 FragColor;
        
        void main() {
            FragColor = vec4(color.rgb, color.a * opacity);
        }
    )");
    
    m_program->link();
    
    // 矩形顶点
    float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };
    
    m_vao.create();
    m_vao.bind();
    
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices, sizeof(vertices));
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    
    m_vao.release();
}

void CustomRenderNode::releaseResources() {
    delete m_program;
    m_program = nullptr;
}

QSGRenderNode::StateFlags CustomRenderNode::changedStates() const {
    return BlendState | ScissorState | StencilState;
}

QSGRenderNode::RenderingFlags CustomRenderNode::flags() const {
    return BoundedRectRendering | DepthAwareRendering;
}

QRectF CustomRenderNode::rect() const {
    return m_rect;
}

// QML 项
class CustomRenderItem : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(float rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    
public:
    CustomRenderItem(QQuickItem *parent = nullptr) : QQuickItem(parent) {
        setFlag(ItemHasContents, true);
    }
    
    QColor color() const { return m_color; }
    void setColor(const QColor &color) {
        if (m_color != color) {
            m_color = color;
            emit colorChanged();
            update();
        }
    }
    
    float rotation() const { return m_rotation; }
    void setRotation(float rotation) {
        if (!qFuzzyCompare(m_rotation, rotation)) {
            m_rotation = rotation;
            emit rotationChanged();
            update();
        }
    }
    
signals:
    void colorChanged();
    void rotationChanged();
    
protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override {
        CustomRenderNode *node = static_cast<CustomRenderNode*>(oldNode);
        if (!node) {
            node = new CustomRenderNode();
        }
        
        node->setColor(m_color);
        node->setRotation(m_rotation);
        node->setRect(boundingRect());
        
        return node;
    }
    
private:
    QColor m_color = Qt::red;
    float m_rotation = 0.0f;
};
```



---

## 11. 帧缓冲与离屏渲染

### 11.1 QOpenGLFramebufferObject 基础

```cpp
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>

class FramebufferExample : public QOpenGLWidget, protected QOpenGLFunctions {
protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        
        // 创建帧缓冲对象
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setSamples(4);  // MSAA
        format.setTextureTarget(GL_TEXTURE_2D);
        format.setInternalTextureFormat(GL_RGBA8);
        
        m_fbo = new QOpenGLFramebufferObject(1024, 1024, format);
        
        if (!m_fbo->isValid()) {
            qDebug() << "FBO creation failed";
        }
        
        // 初始化着色器和几何体...
    }
    
    void paintGL() override {
        // 第一遍：渲染到 FBO
        m_fbo->bind();
        glViewport(0, 0, m_fbo->width(), m_fbo->height());
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 渲染场景到 FBO...
        renderScene();
        
        m_fbo->release();
        
        // 第二遍：将 FBO 纹理渲染到屏幕（带后处理）
        glViewport(0, 0, width(), height());
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_postProcessShader->bind();
        
        // 绑定 FBO 纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_fbo->texture());
        m_postProcessShader->setUniformValue("screenTexture", 0);
        m_postProcessShader->setUniformValue("time", m_time);
        
        // 渲染全屏四边形
        renderQuad();
        
        m_postProcessShader->release();
    }
    
private:
    void renderScene() {
        // 渲染 3D 场景...
    }
    
    void renderQuad() {
        // 渲染全屏四边形...
    }
    
    QOpenGLFramebufferObject *m_fbo = nullptr;
    QOpenGLShaderProgram *m_postProcessShader = nullptr;
    float m_time = 0.0f;
};
```

### 11.2 多重渲染目标（MRT）

```cpp
class MultipleRenderTargets : public QOpenGLWidget, protected QOpenGLFunctions {
protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        
        // 创建 FBO
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        
        // 创建多个颜色附件
        for (int i = 0; i < 3; i++) {
            glGenTextures(1, &m_colorTextures[i]);
            glBindTexture(GL_TEXTURE_2D, m_colorTextures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_colorTextures[i], 0);
        }
        
        // 创建深度附件
        glGenRenderbuffers(1, &m_depthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 1024);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRBO);
        
        // 指定绘制缓冲区
        GLenum attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);
        
        // 检查 FBO 完整性
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            qDebug() << "Framebuffer not complete!";
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    void paintGL() override {
        // 渲染到多个目标
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_gBufferShader->bind();
        // 渲染场景，输出到多个颜色附件
        // layout(location = 0) out vec4 gPosition;
        // layout(location = 1) out vec4 gNormal;
        // layout(location = 2) out vec4 gAlbedo;
        renderScene();
        m_gBufferShader->release();
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // 使用 G-Buffer 进行延迟光照
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_lightingShader->bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_colorTextures[0]);  // Position
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_colorTextures[1]);  // Normal
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_colorTextures[2]);  // Albedo
        
        m_lightingShader->setUniformValue("gPosition", 0);
        m_lightingShader->setUniformValue("gNormal", 1);
        m_lightingShader->setUniformValue("gAlbedo", 2);
        
        renderQuad();
        m_lightingShader->release();
    }
    
private:
    GLuint m_fbo;
    GLuint m_colorTextures[3];
    GLuint m_depthRBO;
    QOpenGLShaderProgram *m_gBufferShader = nullptr;
    QOpenGLShaderProgram *m_lightingShader = nullptr;
};
```

### 11.3 后处理效果

```glsl
// 后处理着色器集合

// ========== 模糊效果 ==========
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float blurAmount;

void main() {
    vec2 texelSize = 1.0 / textureSize(screenTexture, 0);
    vec3 result = vec3(0.0);
    
    // 高斯模糊核
    float kernel[9] = float[](
        1.0, 2.0, 1.0,
        2.0, 4.0, 2.0,
        1.0, 2.0, 1.0
    );
    
    int index = 0;
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            vec2 offset = vec2(float(x), float(y)) * texelSize * blurAmount;
            result += texture(screenTexture, TexCoords + offset).rgb * kernel[index] / 16.0;
            index++;
        }
    }
    
    FragColor = vec4(result, 1.0);
}

// ========== Bloom 效果 ==========
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;
uniform float bloomStrength;

void main() {
    vec3 hdrColor = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    
    // 添加 bloom
    hdrColor += bloomColor * bloomStrength;
    
    // 色调映射
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    
    // Gamma 校正
    result = pow(result, vec3(1.0 / 2.2));
    
    FragColor = vec4(result, 1.0);
}

// ========== 边缘检测 ==========
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main() {
    vec2 texelSize = 1.0 / textureSize(screenTexture, 0);
    
    // Sobel 算子
    float sobelX[9] = float[](
        -1.0, 0.0, 1.0,
        -2.0, 0.0, 2.0,
        -1.0, 0.0, 1.0
    );
    
    float sobelY[9] = float[](
        -1.0, -2.0, -1.0,
         0.0,  0.0,  0.0,
         1.0,  2.0,  1.0
    );
    
    float gx = 0.0;
    float gy = 0.0;
    int index = 0;
    
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            float intensity = dot(texture(screenTexture, TexCoords + offset).rgb, vec3(0.299, 0.587, 0.114));
            gx += intensity * sobelX[index];
            gy += intensity * sobelY[index];
            index++;
        }
    }
    
    float edge = sqrt(gx * gx + gy * gy);
    FragColor = vec4(vec3(edge), 1.0);
}

// ========== 色差效果 ==========
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float aberrationStrength;

void main() {
    vec2 direction = TexCoords - vec2(0.5);
    
    float r = texture(screenTexture, TexCoords + direction * aberrationStrength).r;
    float g = texture(screenTexture, TexCoords).g;
    float b = texture(screenTexture, TexCoords - direction * aberrationStrength).b;
    
    FragColor = vec4(r, g, b, 1.0);
}

// ========== 老电影效果 ==========
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float time;

float random(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;
    
    // 灰度
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    color = vec3(gray);
    
    // 噪点
    float noise = random(TexCoords + time) * 0.1;
    color += noise;
    
    // 扫描线
    float scanline = sin(TexCoords.y * 800.0) * 0.04;
    color -= scanline;
    
    // 暗角
    vec2 uv = TexCoords * (1.0 - TexCoords.yx);
    float vignette = uv.x * uv.y * 15.0;
    vignette = pow(vignette, 0.25);
    color *= vignette;
    
    FragColor = vec4(color, 1.0);
}
```

---

## 12. 性能优化与最佳实践

### 12.1 性能优化技巧

```cpp
class PerformanceOptimization {
public:
    // ✅ 1. 批量绘制
    static void batchDrawing() {
        // ❌ 错误：多次绘制调用
        for (int i = 0; i < 1000; i++) {
            glDrawArrays(GL_TRIANGLES, i * 3, 3);
        }
        
        // ✅ 正确：单次绘制调用
        glDrawArrays(GL_TRIANGLES, 0, 3000);
    }
    
    // ✅ 2. 使用实例化渲染
    static void instancedRendering() {
        // 设置实例化数据
        QOpenGLBuffer instanceVBO(QOpenGLBuffer::VertexBuffer);
        instanceVBO.create();
        instanceVBO.bind();
        
        QVector<QMatrix4x4> modelMatrices;
        for (int i = 0; i < 1000; i++) {
            QMatrix4x4 model;
            model.translate(i * 2.0f, 0, 0);
            modelMatrices.append(model);
        }
        
        instanceVBO.allocate(modelMatrices.data(), modelMatrices.size() * sizeof(QMatrix4x4));
        
        // 设置实例化属性
        for (int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(3 + i);
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), 
                                (void*)(sizeof(QVector4D) * i));
            glVertexAttribDivisor(3 + i, 1);
        }
        
        // 实例化绘制
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 1000);
    }
    
    // ✅ 3. 减少状态切换
    static void minimizeStateChanges() {
        // 按材质/纹理分组绘制
        // 1. 绘制所有使用材质 A 的对象
        // 2. 绘制所有使用材质 B 的对象
        // 而不是频繁切换材质
    }
    
    // ✅ 4. 使用 VAO
    static void useVAO() {
        // VAO 保存顶点属性配置，避免重复设置
        QOpenGLVertexArrayObject vao;
        vao.create();
        vao.bind();
        
        // 配置顶点属性...
        
        vao.release();
        
        // 绘制时只需绑定 VAO
        vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
        vao.release();
    }
    
    // ✅ 5. 纹理压缩
    static QOpenGLTexture* loadCompressedTexture(const QString &path) {
        QOpenGLTexture *texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        
        // 使用压缩格式（如 DXT/BC）
        // 减少显存占用和带宽
        
        return texture;
    }
    
    // ✅ 6. LOD（细节层次）
    static void useLOD(float distance) {
        if (distance < 10.0f) {
            // 使用高精度模型
        } else if (distance < 50.0f) {
            // 使用中精度模型
        } else {
            // 使用低精度模型
        }
    }
    
    // ✅ 7. 视锥剔除
    static bool frustumCulling(const QVector3D &position, float radius, const QMatrix4x4 &viewProj) {
        // 检查对象是否在视锥内
        QVector4D clipPos = viewProj * QVector4D(position, 1.0f);
        
        // 简化的视锥检查
        if (clipPos.x() < -clipPos.w() - radius || clipPos.x() > clipPos.w() + radius)
            return false;
        if (clipPos.y() < -clipPos.w() - radius || clipPos.y() > clipPos.w() + radius)
            return false;
        if (clipPos.z() < -clipPos.w() - radius || clipPos.z() > clipPos.w() + radius)
            return false;
        
        return true;
    }
    
    // ✅ 8. 使用 UBO（统一缓冲对象）
    static void useUniformBufferObject() {
        GLuint ubo;
        glGenBuffers(1, &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        
        // 分配缓冲区
        glBufferData(GL_UNIFORM_BUFFER, sizeof(QMatrix4x4) * 2, nullptr, GL_STATIC_DRAW);
        
        // 绑定到绑定点
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
        
        // 在着色器中：
        // layout (std140) uniform Matrices {
        //     mat4 projection;
        //     mat4 view;
        // };
    }
};
```

### 12.2 性能分析工具

```cpp
class PerformanceProfiler {
public:
    // ✅ FPS 计数器
    class FPSCounter {
    public:
        void update() {
            m_frameCount++;
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            qint64 deltaTime = currentTime - m_lastTime;
            
            if (deltaTime >= 1000) {
                m_fps = m_frameCount * 1000.0 / deltaTime;
                m_frameTime = deltaTime / double(m_frameCount);
                m_frameCount = 0;
                m_lastTime = currentTime;
                
                qDebug() << "FPS:" << m_fps << "Frame Time:" << m_frameTime << "ms";
            }
        }
        
        double fps() const { return m_fps; }
        double frameTime() const { return m_frameTime; }
        
    private:
        int m_frameCount = 0;
        qint64 m_lastTime = QDateTime::currentMSecsSinceEpoch();
        double m_fps = 0.0;
        double m_frameTime = 0.0;
    };
    
    // ✅ GPU 计时器
    class GPUTimer {
    public:
        void begin() {
            if (!m_initialized) {
                glGenQueries(1, &m_query);
                m_initialized = true;
            }
            glBeginQuery(GL_TIME_ELAPSED, m_query);
        }
        
        void end() {
            glEndQuery(GL_TIME_ELAPSED);
        }
        
        double getElapsedTime() {
            GLuint64 elapsed;
            glGetQueryObjectui64v(m_query, GL_QUERY_RESULT, &elapsed);
            return elapsed / 1000000.0;  // 转换为毫秒
        }
        
    private:
        GLuint m_query;
        bool m_initialized = false;
    };
    
    // ✅ 内存使用监控
    static void checkMemoryUsage() {
        GLint totalMemory, availableMemory;
        
        // NVIDIA
        glGetIntegerv(0x9048, &totalMemory);      // GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX
        glGetIntegerv(0x9049, &availableMemory);  // GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX
        
        qDebug() << "GPU Memory - Total:" << totalMemory / 1024 << "MB"
                 << "Available:" << availableMemory / 1024 << "MB"
                 << "Used:" << (totalMemory - availableMemory) / 1024 << "MB";
    }
};
```

### 12.3 最佳实践清单

```cpp
/*
 * Qt6 OpenGL 最佳实践
 */

// ✅ 1. 初始化
// - 在 main() 中设置 QSurfaceFormat
// - 使用核心配置文件（Core Profile）
// - 启用深度缓冲和模板缓冲
// - 根据需要启用 MSAA

// ✅ 2. 资源管理
// - 在 initializeGL() 中创建 OpenGL 资源
// - 在析构函数中调用 makeCurrent() 后释放资源
// - 使用 RAII 包装器（QOpenGLBuffer, QOpenGLTexture 等）

// ✅ 3. 着色器
// - 使用 #version 指令指定 GLSL 版本
// - 检查编译和链接错误
// - 使用 uniform 缓冲对象减少 uniform 调用
// - 避免在片段着色器中进行复杂计算

// ✅ 4. 纹理
// - 使用 mipmaps 提高性能
// - 考虑纹理压缩格式
// - 使用纹理图集减少绑定切换
// - 正确设置过滤和环绕模式

// ✅ 5. 几何体
// - 使用 VAO 管理顶点属性
// - 使用 EBO 减少顶点重复
// - 考虑实例化渲染
// - 实现 LOD 系统

// ✅ 6. 渲染
// - 最小化状态切换
// - 批量绘制相同材质的对象
// - 实现视锥剔除
// - 使用深度预处理（Z-prepass）

// ✅ 7. 性能
// - 定期分析性能瓶颈
// - 使用 GPU 分析工具
// - 监控帧时间和 FPS
// - 优化热点代码

// ✅ 8. 调试
// - 启用 OpenGL 调试上下文
// - 检查 OpenGL 错误
// - 使用 RenderDoc 等工具
// - 验证着色器输出

// ✅ 9. 跨平台
// - 测试不同 OpenGL 版本
// - 考虑 OpenGL ES 兼容性
// - 处理不同驱动的差异
// - 提供降级方案

// ✅ 10. QML 集成
// - 使用 QQuickFramebufferObject 自定义渲染
// - 在 synchronize() 中同步数据
// - 考虑使用 Qt Quick 3D
// - 注意线程安全
```

---

## 总结

本文档详细介绍了 Qt6 中的 OpenGL 编程，涵盖了以下核心内容：

1. **OpenGL 基础** - 版本对比、模块配置、渲染管线
2. **Qt OpenGL 架构** - 核心类、上下文管理
3. **QOpenGLWidget** - 基础和高级窗口部件实现
4. **着色器编程** - GLSL 语法、常用着色器示例
5. **纹理与材质** - 纹理加载、过滤、环绕模式
6. **3D 变换** - 矩阵运算、相机系统
7. **光照与阴影** - Phong 模型、多光源、阴影映射
8. **QML 集成** - QQuickFramebufferObject、自定义渲染
9. **Qt Quick 3D** - 3D 场景、材质、动画
10. **自定义渲染** - QSGRenderNode、渲染节点
11. **帧缓冲** - FBO、MRT、后处理效果
12. **性能优化** - 批量绘制、实例化、LOD、最佳实践

通过本指南，你应该能够：
- 在 Qt6 应用中集成 OpenGL 渲染
- 创建复杂的 3D 场景和效果
- 在 QML 中使用自定义 OpenGL 渲染
- 优化 OpenGL 应用的性能
- 实现高级渲染技术

**推荐学习路径：**
1. 从基础的 QOpenGLWidget 开始
2. 学习着色器编程和 GLSL
3. 掌握纹理和光照技术
4. 探索 QML 集成方案
5. 学习高级渲染技术
6. 进行性能优化

**相关资源：**
- Qt 官方文档：https://doc.qt.io/qt-6/qtopengl-index.html
- OpenGL 规范：https://www.khronos.org/opengl/
- LearnOpenGL：https://learnopengl.com/
- Qt Quick 3D：https://doc.qt.io/qt-6/qtquick3d-index.html

祝你在 Qt6 OpenGL 开发中取得成功！🚀
