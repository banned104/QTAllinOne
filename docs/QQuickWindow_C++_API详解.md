# QQuickWindow C++ API 详解

> 本文档详细介绍 Qt6 中 QQuickWindow 的 C++ API，包括窗口管理、渲染控制、场景图、事件处理等，帮助开发者在 C++ 层面深入理解和控制 QML 窗口系统。

## 目录

1. [QQuickWindow 概述](#1-quickwindow-概述)
2. [窗口创建与管理](#2-窗口创建与管理)
3. [窗口属性与状态](#3-窗口属性与状态)
4. [渲染与场景图](#4-渲染与场景图)
5. [OpenGL 集成](#5-opengl-集成)
6. [事件处理](#6-事件处理)
7. [窗口内容管理](#7-窗口内容管理)
8. [多窗口管理](#8-多窗口管理)
9. [性能优化](#9-性能优化)
10. [高级特性](#10-高级特性)
11. [实战案例](#11-实战案例)

---

## 1. QQuickWindow 概述

### 1.1 类层次结构

```cpp
QObject
    ↓
QSurface
    ↓
QWindow
    ↓
QQuickWindow
```

### 1.2 头文件和链接

```cpp
// 头文件
#include <QQuickWindow>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlApplicationEngine>

// CMakeLists.txt
find_package(Qt6 REQUIRED COMPONENTS Quick)
target_link_libraries(myapp PRIVATE Qt6::Quick)

// .pro 文件
QT += quick
```

### 1.3 基本使用

```cpp
#include <QGuiApplication>
#include <QQuickWindow>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // ========== 方法 1: 使用 QQmlApplicationEngine ==========
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    
    // 获取窗口
    QObject *rootObject = engine.rootObjects().first();
    QQuickWindow *window = qobject_cast<QQuickWindow*>(rootObject);
    
    if (window) {
        // 操作窗口
        window->setTitle("My Application");
        window->setWidth(1280);
        window->setHeight(720);
    }
    
    return app.exec();
}
```

```cpp
#include <QGuiApplication>
#include <QQuickView>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // ========== 方法 2: 使用 QQuickView ==========
    QQuickView view;
    view.setSource(QUrl(QStringLiteral("qrc:/main.qml")));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
    
    // QQuickView 继承自 QQuickWindow
    view.setTitle("My Application");
    
    return app.exec();
}
```

---

## 2. 窗口创建与管理

### 2.1 创建窗口

```cpp
#include <QQuickWindow>
#include <QQuickItem>
#include <QQmlComponent>
#include <QQmlEngine>

class WindowCreation
{
public:
    // ========== 创建空窗口 ==========
    static QQuickWindow* createEmptyWindow()
    {
        QQuickWindow *window = new QQuickWindow();
        window->setTitle("Empty Window");
        window->setWidth(800);
        window->setHeight(600);
        window->setColor(QColor(240, 240, 240));
        
        return window;
    }
    
    // ========== 从 QML 创建窗口 ==========
    static QQuickWindow* createFromQml(QQmlEngine *engine, const QString &qmlFile)
    {
        QQmlComponent component(engine, QUrl(qmlFile));
        
        if (component.isError()) {
            qDebug() << "Error loading QML:" << component.errors();
            return nullptr;
        }
        
        QObject *object = component.create();
        QQuickWindow *window = qobject_cast<QQuickWindow*>(object);
        
        if (!window) {
            qDebug() << "Root object is not a Window";
            delete object;
            return nullptr;
        }
        
        return window;
    }
    
    // ========== 动态创建窗口 ==========
    static QQuickWindow* createDynamicWindow(QQmlEngine *engine)
    {
        QString qmlCode = R"(
            import QtQuick
            import QtQuick.Controls
            
            Window {
                width: 640
                height: 480
                visible: true
                title: "Dynamic Window"
                
                Rectangle {
                    anchors.fill: parent
                    color: "#f0f0f0"
                    
                    Text {
                        anchors.centerIn: parent
                        text: "Dynamically Created Window"
                        font.pixelSize: 20
                    }
                }
            }
        )";
        
        QQmlComponent component(engine);
        component.setData(qmlCode.toUtf8(), QUrl());
        
        QObject *object = component.create();
        return qobject_cast<QQuickWindow*>(object);
    }
};
```

### 2.2 显示和隐藏

```cpp
#include <QQuickWindow>

class WindowDisplay
{
public:
    static void demonstrateDisplay(QQuickWindow *window)
    {
        // ========== 显示窗口 ==========
        window->show();              // 显示窗口
        window->showNormal();        // 正常显示
        window->showMinimized();     // 最小化显示
        window->showMaximized();     // 最大化显示
        window->showFullScreen();    // 全屏显示
        
        // ========== 隐藏窗口 ==========
        window->hide();              // 隐藏窗口
        
        // ========== 提升和降低 ==========
        window->raise();             // 提升到最前
        window->lower();             // 降低到最后
        
        // ========== 激活窗口 ==========
        window->requestActivate();   // 请求激活
        
        // ========== 关闭窗口 ==========
        window->close();             // 关闭窗口
        
        // ========== 检查状态 ==========
        bool visible = window->isVisible();
        bool active = window->isActive();
        bool exposed = window->isExposed();
        
        qDebug() << "可见:" << visible;
        qDebug() << "激活:" << active;
        qDebug() << "暴露:" << exposed;
    }
};
```

---

## 3. 窗口属性与状态

### 3.1 几何属性

```cpp
#include <QQuickWindow>
#include <QRect>
#include <QPoint>
#include <QSize>

class WindowGeometry
{
public:
    static void demonstrateGeometry(QQuickWindow *window)
    {
        // ========== 位置 ==========
        QPoint pos = window->position();
        int x = window->x();
        int y = window->y();
        
        window->setPosition(QPoint(100, 100));
        window->setX(100);
        window->setY(100);
        
        // ========== 尺寸 ==========
        QSize size = window->size();
        int width = window->width();
        int height = window->height();
        
        window->resize(QSize(1280, 720));
        window->setWidth(1280);
        window->setHeight(720);
        
        // ========== 几何矩形 ==========
        QRect geometry = window->geometry();
        window->setGeometry(QRect(100, 100, 1280, 720));
        
        // ========== 最小/最大尺寸 ==========
        QSize minSize = window->minimumSize();
        QSize maxSize = window->maximumSize();
        
        window->setMinimumSize(QSize(800, 600));
        window->setMaximumSize(QSize(1920, 1080));
        
        window->setMinimumWidth(800);
        window->setMinimumHeight(600);
        window->setMaximumWidth(1920);
        window->setMaximumHeight(1080);
        
        // ========== 帧边距 ==========
        QMargins frameMargins = window->frameMargins();
        qDebug() << "帧边距:" << frameMargins;
        
        // ========== 内容项尺寸 ==========
        QQuickItem *contentItem = window->contentItem();
        if (contentItem) {
            qreal contentWidth = contentItem->width();
            qreal contentHeight = contentItem->height();
            qDebug() << "内容尺寸:" << contentWidth << "x" << contentHeight;
        }
    }
};
```

### 3.2 窗口标题和图标

```cpp
#include <QQuickWindow>
#include <QIcon>

class WindowTitleIcon
{
public:
    static void demonstrateTitleIcon(QQuickWindow *window)
    {
        // ========== 标题 ==========
        QString title = window->title();
        window->setTitle("My Application");
        
        // 动态标题
        int documentCount = 5;
        window->setTitle(QString("My App - %1 documents").arg(documentCount));
        
        // ========== 图标 ==========
        QIcon icon(":/icons/app_icon.png");
        window->setIcon(icon);
        
        // 从资源加载
        window->setIcon(QIcon(":/icons/app_icon.png"));
        
        // 多尺寸图标
        QIcon multiSizeIcon;
        multiSizeIcon.addFile(":/icons/icon_16.png", QSize(16, 16));
        multiSizeIcon.addFile(":/icons/icon_32.png", QSize(32, 32));
        multiSizeIcon.addFile(":/icons/icon_64.png", QSize(64, 64));
        window->setIcon(multiSizeIcon);
    }
};
```

### 3.3 窗口标志和状态

```cpp
#include <QQuickWindow>

class WindowFlagsState
{
public:
    static void demonstrateFlagsState(QQuickWindow *window)
    {
        // ========== 窗口标志 ==========
        Qt::WindowFlags flags = window->flags();
        
        // 设置窗口标志
        window->setFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        
        // 常用标志组合
        // 无边框窗口
        window->setFlags(Qt::Window | Qt::FramelessWindowHint);
        
        // 置顶窗口
        window->setFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        
        // 工具窗口
        window->setFlags(Qt::Tool);
        
        // 对话框
        window->setFlags(Qt::Dialog);
        
        // ========== 窗口状态 ==========
        Qt::WindowStates state = window->windowStates();
        
        // 设置窗口状态
        window->setWindowStates(Qt::WindowNoState);      // 正常
        window->setWindowStates(Qt::WindowMinimized);    // 最小化
        window->setWindowStates(Qt::WindowMaximized);    // 最大化
        window->setWindowStates(Qt::WindowFullScreen);   // 全屏
        
        // 检查状态
        bool isMinimized = (state & Qt::WindowMinimized);
        bool isMaximized = (state & Qt::WindowMaximized);
        bool isFullScreen = (state & Qt::WindowFullScreen);
        
        qDebug() << "最小化:" << isMinimized;
        qDebug() << "最大化:" << isMaximized;
        qDebug() << "全屏:" << isFullScreen;
        
        // ========== 模态 ==========
        Qt::WindowModality modality = window->modality();
        
        window->setModality(Qt::NonModal);           // 非模态
        window->setModality(Qt::WindowModal);        // 窗口模态
        window->setModality(Qt::ApplicationModal);   // 应用程序模态
        
        // ========== 不透明度 ==========
        qreal opacity = window->opacity();
        window->setOpacity(0.9);  // 90% 不透明
    }
};
```

### 3.4 窗口颜色和格式

```cpp
#include <QQuickWindow>
#include <QColor>
#include <QSurfaceFormat>

class WindowColorFormat
{
public:
    static void demonstrateColorFormat(QQuickWindow *window)
    {
        // ========== 背景颜色 ==========
        QColor color = window->color();
        window->setColor(QColor(240, 240, 240));
        window->setColor(Qt::white);
        window->setColor("#f0f0f0");
        
        // 透明窗口
        window->setColor(Qt::transparent);
        
        // ========== 表面格式 ==========
        QSurfaceFormat format = window->format();
        
        // 设置格式（通常在创建前设置）
        QSurfaceFormat newFormat;
        newFormat.setVersion(3, 3);
        newFormat.setProfile(QSurfaceFormat::CoreProfile);
        newFormat.setDepthBufferSize(24);
        newFormat.setStencilBufferSize(8);
        newFormat.setSamples(4);  // 4x MSAA
        
        window->setFormat(newFormat);
        
        // ========== 渲染目标 ==========
        // 获取渲染目标类型（只读）
        QQuickWindow::RenderTarget renderTarget = window->renderTarget();
        
        qDebug() << "渲染目标:" << renderTarget;
    }
    
    // ========== 设置默认格式（应用程序启动时）==========
    static void setupDefaultFormat()
    {
        QSurfaceFormat format;
        format.setVersion(3, 3);
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setDepthBufferSize(24);
        format.setStencilBufferSize(8);
        format.setSamples(4);
        format.setSwapInterval(1);  // 垂直同步
        
        QSurfaceFormat::setDefaultFormat(format);
    }
};
```

---

## 4. 渲染与场景图

### 4.1 场景图信号

```cpp
#include <QQuickWindow>
#include <QObject>

class SceneGraphSignals : public QObject
{
    Q_OBJECT
    
public:
    void connectSignals(QQuickWindow *window)
    {
        // ========== 场景图生命周期 ==========
        
        // 场景图初始化（在渲染线程）
        connect(window, &QQuickWindow::sceneGraphInitialized, this, []() {
            qDebug() << "场景图已初始化";
            qDebug() << "线程:" << QThread::currentThread();
        }, Qt::DirectConnection);
        
        // 场景图失效（在渲染线程）
        connect(window, &QQuickWindow::sceneGraphInvalidated, this, []() {
            qDebug() << "场景图已失效";
        }, Qt::DirectConnection);
        
        // 场景图错误
        connect(window, &QQuickWindow::sceneGraphError, 
                this, [](QQuickWindow::SceneGraphError error, const QString &message) {
            qDebug() << "场景图错误:" << error << message;
        });
        
        // ========== 渲染循环 ==========
        
        // 同步前（在渲染线程）
        connect(window, &QQuickWindow::beforeSynchronizing, this, []() {
            qDebug() << "同步前";
        }, Qt::DirectConnection);
        
        // 同步后（在渲染线程）
        connect(window, &QQuickWindow::afterSynchronizing, this, []() {
            qDebug() << "同步后";
        }, Qt::DirectConnection);
        
        // 渲染前（在渲染线程）
        connect(window, &QQuickWindow::beforeRendering, this, []() {
            qDebug() << "渲染前";
        }, Qt::DirectConnection);
        
        // 渲染后（在渲染线程）
        connect(window, &QQuickWindow::afterRendering, this, []() {
            qDebug() << "渲染后";
        }, Qt::DirectConnection);
        
        // 帧交换（在渲染线程）
        connect(window, &QQuickWindow::frameSwapped, this, []() {
            qDebug() << "帧已交换";
        }, Qt::DirectConnection);
        
        // ========== 渲染目标 ==========
        
        // 渲染目标改变
        connect(window, &QQuickWindow::beforeRenderPassRecording, this, []() {
            qDebug() << "渲染通道记录前";
        }, Qt::DirectConnection);
        
        connect(window, &QQuickWindow::afterRenderPassRecording, this, []() {
            qDebug() << "渲染通道记录后";
        }, Qt::DirectConnection);
    }
};
```

### 4.2 渲染控制

```cpp
#include <QQuickWindow>

class RenderControl
{
public:
    static void demonstrateRenderControl(QQuickWindow *window)
    {
        // ========== 持久化设置 ==========
        
        // 持久化场景图
        window->setPersistentSceneGraph(true);
        bool persistent = window->isPersistentSceneGraph();
        
        // 持久化 OpenGL 上下文
        window->setPersistentGraphics(true);
        bool persistentGL = window->isPersistentGraphics();
        
        // ========== 渲染目标 ==========
        
        // 设置渲染目标
        window->setRenderTarget(QQuickRenderTarget());
        
        // ========== 图形 API ==========
        
        // 获取图形 API
        QSGRendererInterface *renderer = window->rendererInterface();
        if (renderer) {
            QSGRendererInterface::GraphicsApi api = renderer->graphicsApi();
            
            switch (api) {
                case QSGRendererInterface::OpenGL:
                    qDebug() << "使用 OpenGL";
                    break;
                case QSGRendererInterface::Vulkan:
                    qDebug() << "使用 Vulkan";
                    break;
                case QSGRendererInterface::Direct3D11:
                    qDebug() << "使用 Direct3D 11";
                    break;
                case QSGRendererInterface::Metal:
                    qDebug() << "使用 Metal";
                    break;
                default:
                    qDebug() << "未知 API";
                    break;
            }
        }
        
        // ========== 更新控制 ==========
        
        // 请求更新
        window->update();
        
        // 调度更新
        window->scheduleRenderJob([]() {
            qDebug() << "渲染任务执行";
        }, QQuickWindow::BeforeSynchronizingStage);
        
        // 渲染阶段：
        // - BeforeSynchronizingStage
        // - AfterSynchronizingStage
        // - BeforeRenderingStage
        // - AfterRenderingStage
        // - AfterSwapStage
        // - NoStage
    }
};
```

### 4.3 场景图纹理

```cpp
#include <QQuickWindow>
#include <QImage>
#include <QSGTexture>

class SceneGraphTexture
{
public:
    static void demonstrateTexture(QQuickWindow *window)
    {
        // ========== 从图像创建纹理 ==========
        QImage image(":/images/texture.png");
        
        if (!image.isNull()) {
            // 创建纹理
            QSGTexture *texture = window->createTextureFromImage(image);
            
            if (texture) {
                qDebug() << "纹理已创建";
                qDebug() << "纹理 ID:" << texture->textureId();
                qDebug() << "纹理尺寸:" << texture->textureSize();
                
                // 使用纹理...
                
                // 删除纹理
                delete texture;
            }
        }
        
        // ========== 创建纹理（带选项）==========
        QQuickWindow::CreateTextureOptions options;
        options |= QQuickWindow::TextureHasAlphaChannel;
        options |= QQuickWindow::TextureCanUseAtlas;
        
        QSGTexture *texture2 = window->createTextureFromImage(image, options);
        
        // ========== 从 ID 创建纹理 ==========
        uint textureId = 123;  // OpenGL 纹理 ID
        QSize size(256, 256);
        QQuickWindow::CreateTextureOptions opts;
        
        QSGTexture *texture3 = window->createTextureFromNativeObject(
            QQuickWindow::NativeObjectTexture,
            &textureId,
            0,
            size,
            opts
        );
    }
};
```

---

## 5. OpenGL 集成

### 5.1 OpenGL 上下文

```cpp
#include <QQuickWindow>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

class OpenGLIntegration : public QObject
{
    Q_OBJECT
    
public:
    void setupOpenGL(QQuickWindow *window)
    {
        // ========== 获取 OpenGL 上下文 ==========
        QOpenGLContext *context = window->openglContext();
        
        if (context) {
            qDebug() << "OpenGL 版本:" << context->format().version();
            qDebug() << "OpenGL 配置:" << context->format().profile();
            
            // 使上下文当前
            context->makeCurrent(window);
            
            // 获取 OpenGL 函数
            QOpenGLFunctions *gl = context->functions();
            
            // 使用 OpenGL
            const GLubyte *vendor = gl->glGetString(GL_VENDOR);
            const GLubyte *renderer = gl->glGetString(GL_RENDERER);
            const GLubyte *version = gl->glGetString(GL_VERSION);
            
            qDebug() << "供应商:" << (const char*)vendor;
            qDebug() << "渲染器:" << (const char*)renderer;
            qDebug() << "版本:" << (const char*)version;
            
            // 完成后释放上下文
            context->doneCurrent();
        }
        
        // ========== 连接渲染信号 ==========
        connect(window, &QQuickWindow::beforeRendering, 
                this, &OpenGLIntegration::renderOpenGL, 
                Qt::DirectConnection);
    }
    
private slots:
    void renderOpenGL()
    {
        QQuickWindow *window = qobject_cast<QQuickWindow*>(sender());
        if (!window) return;
        
        QOpenGLContext *context = window->openglContext();
        if (!context) return;
        
        QOpenGLFunctions *gl = context->functions();
        
        // 执行自定义 OpenGL 渲染
        gl->glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 更多 OpenGL 调用...
        
        // 重置 OpenGL 状态（重要！）
        window->resetOpenGLState();
    }
};
```

### 5.2 自定义 OpenGL 渲染

```cpp
#include <QQuickWindow>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class CustomOpenGLRenderer : public QObject
{
    Q_OBJECT
    
public:
    CustomOpenGLRenderer(QQuickWindow *window)
        : m_window(window)
    {
        connect(window, &QQuickWindow::beforeRendering, 
                this, &CustomOpenGLRenderer::render, 
                Qt::DirectConnection);
        
        connect(window, &QQuickWindow::sceneGraphInvalidated, 
                this, &CustomOpenGLRenderer::cleanup, 
                Qt::DirectConnection);
    }
    
private slots:
    void render()
    {
        if (!m_initialized) {
            initialize();
            m_initialized = true;
        }
        
        QOpenGLFunctions *gl = m_window->openglContext()->functions();
        
        // 保存 Qt Quick 的 OpenGL 状态
        gl->glDisable(GL_DEPTH_TEST);
        gl->glDisable(GL_CULL_FACE);
        
        // 使用着色器程序
        m_program->bind();
        
        // 设置 uniform
        m_program->setUniformValue("matrix", m_matrix);
        
        // 绑定 VAO 并绘制
        m_vao.bind();
        gl->glDrawArrays(GL_TRIANGLES, 0, 3);
        m_vao.release();
        
        m_program->release();
        
        // 恢复 Qt Quick 的状态
        m_window->resetOpenGLState();
    }
    
    void cleanup()
    {
        delete m_program;
        m_program = nullptr;
        
        m_vbo.destroy();
        m_vao.destroy();
        
        m_initialized = false;
    }
    
    void initialize()
    {
        QOpenGLFunctions *gl = m_window->openglContext()->functions();
        
        // 创建着色器程序
        m_program = new QOpenGLShaderProgram();
        
        const char *vertexShader = R"(
            #version 330 core
            layout (location = 0) in vec3 position;
            uniform mat4 matrix;
            void main() {
                gl_Position = matrix * vec4(position, 1.0);
            }
        )";
        
        const char *fragmentShader = R"(
            #version 330 core
            out vec4 fragColor;
            void main() {
                fragColor = vec4(1.0, 0.5, 0.2, 1.0);
            }
        )";
        
        m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader);
        m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);
        m_program->link();
        
        // 创建几何体
        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };
        
        m_vao.create();
        m_vao.bind();
        
        m_vbo.create();
        m_vbo.bind();
        m_vbo.allocate(vertices, sizeof(vertices));
        
        gl->glEnableVertexAttribArray(0);
        gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        
        m_vao.release();
        m_vbo.release();
    }
    
private:
    QQuickWindow *m_window;
    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_vao;
    QMatrix4x4 m_matrix;
    bool m_initialized = false;
};
```


---

## 6. �¼�����

QQuickWindow �̳��� QWindow��֧�����б�׼�����¼���

### 6.1 �ؼ��¼���������

```cpp
// ��д�¼���������
class CustomQuickWindow : public QQuickWindow
{
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void touchEvent(QTouchEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};
```

---

## 7. �������ݹ���

### 7.1 ���������

```cpp
// ��ȡ��������
QQuickItem *contentItem = window->contentItem();

// ��������
QQuickItem *item = contentItem->findChild<QQuickItem*>("itemName");

// ������������
QList<QQuickItem*> children = contentItem->childItems();
```

---

## 8. �ര�ڹ���

### 8.1 �����������

```cpp
QList<QQuickWindow*> windows;

for (int i = 0; i < 3; i++) {
    QQuickWindow *window = new QQuickWindow();
    window->setTitle(QString("Window %1").arg(i + 1));
    window->setGeometry(100 + i * 50, 100 + i * 50, 800, 600);
    window->show();
    windows.append(window);
}
```

---

## 9. �����Ż�

### 9.1 �ؼ��Ż�����

- ʹ�� setPersistentSceneGraph(true) ���ֳ���ͼ
- ʹ�� setPersistentGraphics(true) ����ͼ��������
- ����ʹ�� update() �����ǳ���ˢ��
- ����Ⱦ�߳�ʹ�� Qt::DirectConnection

---

## 10. �߼�����

### 10.1 ��ͼ

```cpp
QImage screenshot = window->grabWindow();
screenshot.save("screenshot.png");
```

### 10.2 ��Ļ����

```cpp
QScreen *screen = window->screen();
qDebug() << "��Ļ����:" << screen->name();
qDebug() << "��Ļ�ߴ�:" << screen->size();
qDebug() << "DPI:" << screen->logicalDotsPerInch();
```

---

## 11. ʵս����

### 11.1 ����Ӧ�ó���

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // ����Ĭ�ϸ�ʽ
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
    
    QQmlApplicationEngine engine;
    engine.load(QUrl("qrc:/main.qml"));
    
    QQuickWindow *window = qobject_cast<QQuickWindow*>(
        engine.rootObjects().first());
    
    if (window) {
        // ���ô���
        window->setTitle("My Application");
        window->setMinimumSize(QSize(800, 600));
        
        // �����ź�
        QObject::connect(window, &QQuickWindow::closing, []() {
            qDebug() << "���ڹر�";
        });
    }
    
    return app.exec();
}
```

---

## �ܽ�

### ����֪ʶ��

1. **���ڴ���** - QQmlApplicationEngine, QQuickView
2. **��������** - ���Ρ����⡢ͼ�ꡢ��־
3. **��Ⱦ����** - ����ͼ�źš���Ⱦѭ��
4. **OpenGL ����** - �Զ�����Ⱦ�������Ĺ���
5. **�¼�����** - ���̡���ꡢ�����¼�
6. **�����Ż�** - �־û������¿���

### ���ʵ��

 **�Ƽ�**
- ��Ӧ������ʱ���� QSurfaceFormat
- ʹ�� Qt::DirectConnection ������Ⱦ�ź�
- ���� resetOpenGLState() �ָ�״̬
- ����ʹ�ó־û�ѡ��

 **����**
- ����Ⱦ�߳�ִ�к�ʱ����
- ���ǻָ� OpenGL ״̬
- Ƶ������/���ٴ���
- �����̰߳�ȫ

### �ο���Դ

- **�ٷ��ĵ�**: https://doc.qt.io/qt-6/qquickwindow.html
- **����ͼ**: https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html

---

**ף���� QQuickWindow C++ ������ȡ�óɹ���** 
