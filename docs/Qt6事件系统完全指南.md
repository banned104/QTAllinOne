# Qt6 事件系统完全指南

> 本文档深入剖析 Qt6 的事件系统，从 C++ 底层的事件循环（app.exec()）到 QML 的事件处理，涵盖事件分发、过滤、自定义事件等核心机制。

## 目录

1. [事件系统概述](#1-事件系统概述)
2. [事件循环机制](#2-事件循环机制)
3. [事件类型与层次结构](#3-事件类型与层次结构)
4. [事件分发机制](#4-事件分发机制)
5. [事件过滤器](#5-事件过滤器)
6. [自定义事件](#6-自定义事件)
7. [QML 事件处理](#7-qml-事件处理)
8. [鼠标事件详解](#8-鼠标事件详解)
9. [键盘事件详解](#9-键盘事件详解)
10. [触摸与手势事件](#10-触摸与手势事件)
11. [定时器事件](#11-定时器事件)
12. [事件优先级与队列](#12-事件优先级与队列)

---

## 1. 事件系统概述

### 1.1 什么是事件？

事件（Event）是 Qt 中用于通知对象发生了某些事情的机制。事件可以来自：
- **系统**：鼠标点击、键盘按键、窗口调整大小
- **应用程序**：定时器触发、自定义事件
- **其他对象**：信号槽机制

### 1.2 事件 vs 信号槽

```cpp
// === 信号槽（Signal-Slot）===
// 特点：
// - 高层抽象，类型安全
// - 松耦合，发送者和接收者不需要知道对方
// - 可以跨线程
// - 自动连接管理

QPushButton *button = new QPushButton("Click");
QObject::connect(button, &QPushButton::clicked, []() {
    qDebug() << "Button clicked via signal";
});

// === 事件（Event）===
// 特点：
// - 底层机制，更灵活
// - 可以被过滤、拦截、修改
// - 有明确的传播路径
// - 可以被重写处理

class MyButton : public QPushButton {
protected:
    void mousePressEvent(QMouseEvent *event) override {
        qDebug() << "Button pressed via event";
        QPushButton::mousePressEvent(event);  // 调用基类处理
    }
};
```

### 1.3 事件系统架构

```
┌─────────────────────────────────────────────────────┐
│                   应用程序层                          │
│  QML / Widgets / 自定义类                            │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│                   事件分发层                          │
│  QCoreApplication::notify()                         │
│  QObject::event()                                   │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│                   事件队列                            │
│  QEventLoop                                         │
│  事件优先级管理                                       │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│                   事件源                              │
│  操作系统 / 定时器 / 网络 / 自定义                    │
└─────────────────────────────────────────────────────┘
```

---

## 2. 事件循环机制

### 2.1 app.exec() 深度解析

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>

int main(int argc, char *argv[])
{
    // 1. 创建应用程序对象
    QGuiApplication app(argc, argv);
    
    qDebug() << "Before exec()";
    
    // 2. 进入事件循环
    // ✅ app.exec() 做了什么？
    int result = app.exec();
    // - 创建主事件循环（QEventLoop）
    // - 开始处理事件队列
    // - 阻塞在这里，直到 quit() 被调用
    // - 返回退出码
    
    qDebug() << "After exec()";  // 只有退出时才会执行
    
    return result;
}
```

### 2.2 事件循环的工作原理

```cpp
// Qt 事件循环的伪代码实现
int QCoreApplication::exec()
{
    QEventLoop eventLoop;
    
    // 进入事件循环
    while (!exitRequested) {
        // 1. 等待事件（阻塞或超时）
        waitForEvents(timeout);
        
        // 2. 处理所有待处理的事件
        while (hasEvents()) {
            QEvent *event = getNextEvent();
            
            // 3. 分发事件到目标对象
            notify(event->receiver(), event);
            
            // 4. 删除已处理的事件
            delete event;
        }
        
        // 5. 处理延迟删除的对象
        processDeferredDeletes();
        
        // 6. 发送空闲事件（如果没有其他事件）
        sendPostedEvents();
    }
    
    return exitCode;
}
```

### 2.3 嵌套事件循环

```cpp
#include <QEventLoop>
#include <QTimer>
#include <QPushButton>

class MyWidget : public QWidget {
public:
    void doSomethingBlocking() {
        qDebug() << "Starting blocking operation...";
        
        // ✅ 创建嵌套事件循环
        QEventLoop loop;
        
        // 设置超时
        QTimer::singleShot(3000, &loop, &QEventLoop::quit);
        
        // 或等待某个信号
        QPushButton *button = new QPushButton("Continue");
        connect(button, &QPushButton::clicked, &loop, &QEventLoop::quit);
        button->show();
        
        // 进入嵌套循环（阻塞在这里）
        loop.exec();
        
        qDebug() << "Blocking operation finished";
    }
};

// 使用示例
void example() {
    MyWidget widget;
    widget.doSomethingBlocking();  // 阻塞直到超时或按钮点击
    qDebug() << "Continue execution";
}
```

### 2.4 事件循环的控制

```cpp
#include <QCoreApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // ✅ 方式 1：使用 quit() 退出
    QTimer::singleShot(5000, &app, &QCoreApplication::quit);
    
    // ✅ 方式 2：使用 exit(code) 退出并返回退出码
    QTimer::singleShot(5000, []() {
        QCoreApplication::exit(0);  // 退出码 0
    });
    
    // ✅ 方式 3：处理特定数量的事件后退出
    QTimer::singleShot(0, []() {
        // 处理所有待处理的事件，然后返回
        QCoreApplication::processEvents();
        
        // 处理事件最多 100ms
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    });
    
    return app.exec();
}
```

### 2.5 多线程中的事件循环

```cpp
#include <QThread>
#include <QTimer>

class WorkerThread : public QThread {
    Q_OBJECT
    
protected:
    void run() override {
        qDebug() << "Worker thread started:" << QThread::currentThread();
        
        // ✅ 在工作线程中创建事件循环
        QTimer *timer = new QTimer;
        connect(timer, &QTimer::timeout, []() {
            qDebug() << "Timer in worker thread";
        });
        timer->start(1000);
        
        // 进入事件循环（必须调用 exec()）
        exec();  // QThread::exec()
        
        qDebug() << "Worker thread finished";
        delete timer;
    }
    
public slots:
    void stop() {
        qDebug() << "Stopping worker thread";
        quit();  // 退出事件循环
    }
};

// 使用示例
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    WorkerThread *worker = new WorkerThread;
    worker->start();
    
    // 5 秒后停止工作线程
    QTimer::singleShot(5000, worker, &WorkerThread::stop);
    
    QObject::connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    
    return app.exec();
}
```


---

## 3. 事件类型与层次结构

### 3.1 QEvent 类层次结构

```cpp
// QEvent 是所有事件的基类
class QEvent {
public:
    enum Type {
        None = 0,                   // 无效事件
        
        // === 窗口事件 ===
        Close = 19,                 // 窗口关闭
        Show = 17,                  // 窗口显示
        Hide = 18,                  // 窗口隐藏
        Resize = 14,                // 窗口调整大小
        Move = 13,                  // 窗口移动
        Paint = 12,                 // 绘制事件
        
        // === 鼠标事件 ===
        MouseButtonPress = 2,       // 鼠标按下
        MouseButtonRelease = 3,     // 鼠标释放
        MouseButtonDblClick = 4,    // 鼠标双击
        MouseMove = 5,              // 鼠标移动
        Wheel = 31,                 // 鼠标滚轮
        Enter = 10,                 // 鼠标进入
        Leave = 11,                 // 鼠标离开
        
        // === 键盘事件 ===
        KeyPress = 6,               // 键盘按下
        KeyRelease = 7,             // 键盘释放
        ShortcutOverride = 51,      // 快捷键覆盖
        
        // === 焦点事件 ===
        FocusIn = 8,                // 获得焦点
        FocusOut = 9,               // 失去焦点
        FocusAboutToChange = 23,    // 焦点即将改变
        
        // === 触摸事件 ===
        TouchBegin = 194,           // 触摸开始
        TouchUpdate = 195,          // 触摸更新
        TouchEnd = 196,             // 触摸结束
        TouchCancel = 209,          // 触摸取消
        
        // === 手势事件 ===
        Gesture = 198,              // 手势
        GestureOverride = 202,      // 手势覆盖
        
        // === 定时器事件 ===
        Timer = 1,                  // 定时器
        
        // === 拖放事件 ===
        DragEnter = 60,             // 拖动进入
        DragMove = 61,              // 拖动移动
        DragLeave = 62,             // 拖动离开
        Drop = 63,                  // 放下
        
        // === 其他常用事件 ===
        ContextMenu = 82,           // 上下文菜单
        ToolTip = 110,              // 工具提示
        WhatsThis = 111,            // What's This 帮助
        StatusTip = 112,            // 状态提示
        
        // === 自定义事件 ===
        User = 1000,                // 用户自定义事件起始值
        MaxUser = 65535             // 用户自定义事件最大值
    };
    
    Type type() const;              // 获取事件类型
    bool isAccepted() const;        // 事件是否被接受
    void accept();                  // 接受事件
    void ignore();                  // 忽略事件
};
```

### 3.2 常用事件类

```cpp
// === 鼠标事件 ===
class QMouseEvent : public QInputEvent {
public:
    QPoint pos() const;             // 相对于接收者的位置
    QPoint globalPos() const;       // 屏幕绝对位置
    Qt::MouseButton button() const; // 触发事件的按钮
    Qt::MouseButtons buttons() const; // 当前按下的所有按钮
    Qt::KeyboardModifiers modifiers() const; // 键盘修饰键
};

// === 键盘事件 ===
class QKeyEvent : public QInputEvent {
public:
    int key() const;                // 按键代码（Qt::Key_xxx）
    QString text() const;           // 按键文本
    Qt::KeyboardModifiers modifiers() const; // 修饰键
    bool isAutoRepeat() const;      // 是否是自动重复
    int count() const;              // 重复次数
};

// === 调整大小事件 ===
class QResizeEvent : public QEvent {
public:
    QSize size() const;             // 新尺寸
    QSize oldSize() const;          // 旧尺寸
};

// === 绘制事件 ===
class QPaintEvent : public QEvent {
public:
    QRect rect() const;             // 需要重绘的区域
    QRegion region() const;         // 需要重绘的区域（更精确）
};

// === 定时器事件 ===
class QTimerEvent : public QEvent {
public:
    int timerId() const;            // 定时器 ID
};

// === 关闭事件 ===
class QCloseEvent : public QEvent {
public:
    // 可以通过 ignore() 阻止窗口关闭
};
```

### 3.3 事件类型判断

```cpp
void MyWidget::event(QEvent *event)
{
    // ✅ 方式 1：使用 type()
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        qDebug() << "Mouse pressed at:" << mouseEvent->pos();
    }
    
    // ✅ 方式 2：使用 switch
    switch (event->type()) {
    case QEvent::KeyPress: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        qDebug() << "Key pressed:" << keyEvent->key();
        break;
    }
    case QEvent::Resize: {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event);
        qDebug() << "Resized to:" << resizeEvent->size();
        break;
    }
    default:
        break;
    }
    
    // ✅ 方式 3：使用 dynamic_cast（更安全但慢）
    if (QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
        qDebug() << "Mouse event:" << mouseEvent->pos();
    }
    
    return QWidget::event(event);
}
```

---

## 4. 事件分发机制

### 4.1 事件分发流程

```
事件产生
    ↓
QCoreApplication::notify()
    ↓
QObject::event()
    ↓
特定事件处理器（如 mousePressEvent()）
    ↓
事件被接受或传播给父对象
```

### 4.2 重写 event() 函数

```cpp
#include <QWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>

class MyWidget : public QWidget {
protected:
    // ✅ 重写 event() - 所有事件的入口
    bool event(QEvent *event) override {
        qDebug() << "Event type:" << event->type();
        
        // 在事件到达特定处理器之前拦截
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            qDebug() << "Intercepted mouse press at:" << mouseEvent->pos();
            
            // ✅ 返回 true 表示事件已处理，不再传递
            // return true;
            
            // ✅ 返回 false 或调用基类，继续传递
        }
        
        // 调用基类处理其他事件
        return QWidget::event(event);
    }
    
    // ✅ 特定事件处理器
    void mousePressEvent(QMouseEvent *event) override {
        qDebug() << "Mouse press handler:" << event->pos();
        
        // 处理事件
        if (event->button() == Qt::LeftButton) {
            qDebug() << "Left button pressed";
            event->accept();  // 接受事件
        } else {
            event->ignore();  // 忽略事件，传递给父对象
        }
        
        // 或调用基类
        // QWidget::mousePressEvent(event);
    }
};
```

### 4.3 事件传播

```cpp
#include <QWidget>
#include <QMouseEvent>

class ParentWidget : public QWidget {
protected:
    void mousePressEvent(QMouseEvent *event) override {
        qDebug() << "Parent received mouse press";
        event->accept();  // 接受事件，停止传播
    }
};

class ChildWidget : public QWidget {
protected:
    void mousePressEvent(QMouseEvent *event) override {
        qDebug() << "Child received mouse press";
        
        // ✅ 忽略事件，传播给父对象
        event->ignore();
        
        // 或者不调用基类的处理器
        // QWidget::mousePressEvent(event);
    }
};

// 使用示例
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ParentWidget *parent = new ParentWidget;
    parent->resize(400, 300);
    
    ChildWidget *child = new ChildWidget(parent);
    child->setGeometry(50, 50, 200, 100);
    
    parent->show();
    
    // 点击 child：
    // 输出：Child received mouse press
    //      Parent received mouse press
    
    return app.exec();
}
```

### 4.4 重写 notify() 函数

```cpp
#include <QApplication>
#include <QEvent>
#include <QDebug>

class MyApplication : public QApplication {
public:
    using QApplication::QApplication;
    
    // ✅ 重写 notify() - 全局事件拦截
    bool notify(QObject *receiver, QEvent *event) override {
        // 在事件到达任何对象之前拦截
        
        // 记录所有鼠标事件
        if (event->type() == QEvent::MouseButtonPress) {
            qDebug() << "Global: Mouse press on" << receiver->objectName();
        }
        
        // 拦截特定对象的事件
        if (receiver->objectName() == "criticalWidget") {
            if (event->type() == QEvent::Close) {
                qDebug() << "Preventing close of critical widget";
                return true;  // 阻止事件
            }
        }
        
        // 调用基类处理
        return QApplication::notify(receiver, event);
    }
};

int main(int argc, char *argv[])
{
    MyApplication app(argc, argv);
    
    // 应用程序代码...
    
    return app.exec();
}
```


---

## 5. 事件过滤器

### 5.1 事件过滤器基础

```cpp
#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>

class EventFilter : public QObject {
    Q_OBJECT
    
protected:
    // ✅ 实现事件过滤器
    bool eventFilter(QObject *watched, QEvent *event) override {
        // 过滤特定对象的特定事件
        if (watched->objectName() == "myLineEdit") {
            if (event->type() == QEvent::KeyPress) {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
                
                // 只允许数字输入
                if (keyEvent->key() >= Qt::Key_0 && keyEvent->key() <= Qt::Key_9) {
                    return false;  // 允许事件继续
                } else {
                    qDebug() << "Blocked non-numeric key";
                    return true;   // 阻止事件
                }
            }
        }
        
        // 调用基类处理其他事件
        return QObject::eventFilter(watched, event);
    }
};

// 使用示例
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QLineEdit *lineEdit = new QLineEdit;
    lineEdit->setObjectName("myLineEdit");
    
    // ✅ 安装事件过滤器
    EventFilter *filter = new EventFilter;
    lineEdit->installEventFilter(filter);
    
    lineEdit->show();
    
    return app.exec();
}
```

### 5.2 多个事件过滤器

```cpp
class Filter1 : public QObject {
protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        qDebug() << "Filter1:" << event->type();
        return false;  // 继续传递
    }
};

class Filter2 : public QObject {
protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        qDebug() << "Filter2:" << event->type();
        return false;  // 继续传递
    }
};

// 使用示例
QWidget *widget = new QWidget;

Filter1 *filter1 = new Filter1;
Filter2 *filter2 = new Filter2;

// ✅ 安装多个过滤器（后安装的先执行）
widget->installEventFilter(filter1);
widget->installEventFilter(filter2);

// 事件处理顺序：
// 1. filter2->eventFilter()
// 2. filter1->eventFilter()
// 3. widget->event()

// ✅ 移除事件过滤器
widget->removeEventFilter(filter2);
```

### 5.3 全局事件过滤器

```cpp
#include <QApplication>

class GlobalEventFilter : public QObject {
protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        // 监控所有对象的所有事件
        
        // 记录所有鼠标点击
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            qDebug() << "Global: Mouse clicked on" 
                     << watched->metaObject()->className()
                     << "at" << mouseEvent->globalPos();
        }
        
        // 记录所有键盘输入
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            qDebug() << "Global: Key pressed:" << keyEvent->text();
        }
        
        return false;  // 不阻止事件
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // ✅ 安装全局事件过滤器
    GlobalEventFilter *globalFilter = new GlobalEventFilter;
    app.installEventFilter(globalFilter);
    
    // 现在所有对象的所有事件都会经过这个过滤器
    
    return app.exec();
}
```

### 5.4 实战：实现输入验证

```cpp
#include <QLineEdit>
#include <QRegularExpression>

class InputValidator : public QObject {
    Q_OBJECT
    
public:
    enum ValidationType {
        Numeric,        // 只允许数字
        Alpha,          // 只允许字母
        AlphaNumeric,   // 字母和数字
        Email,          // 邮箱格式
        Phone           // 电话号码
    };
    
    InputValidator(ValidationType type, QObject *parent = nullptr)
        : QObject(parent), m_type(type) {}
    
protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            QString text = keyEvent->text();
            
            if (text.isEmpty()) {
                return false;  // 允许控制键（如 Backspace）
            }
            
            switch (m_type) {
            case Numeric:
                if (!text.contains(QRegularExpression("^[0-9]$"))) {
                    return true;  // 阻止非数字
                }
                break;
                
            case Alpha:
                if (!text.contains(QRegularExpression("^[a-zA-Z]$"))) {
                    return true;  // 阻止非字母
                }
                break;
                
            case AlphaNumeric:
                if (!text.contains(QRegularExpression("^[a-zA-Z0-9]$"))) {
                    return true;
                }
                break;
                
            default:
                break;
            }
        }
        
        return QObject::eventFilter(watched, event);
    }
    
private:
    ValidationType m_type;
};

// 使用示例
QLineEdit *phoneEdit = new QLineEdit;
phoneEdit->setPlaceholderText("Phone number");
phoneEdit->installEventFilter(new InputValidator(InputValidator::Numeric, phoneEdit));

QLineEdit *nameEdit = new QLineEdit;
nameEdit->setPlaceholderText("Name");
nameEdit->installEventFilter(new InputValidator(InputValidator::Alpha, nameEdit));
```

---

## 6. 自定义事件

### 6.1 创建自定义事件

```cpp
#include <QEvent>
#include <QCoreApplication>

// ✅ 步骤 1：定义自定义事件类型
const QEvent::Type MyCustomEventType = 
    static_cast<QEvent::Type>(QEvent::User + 1);

// ✅ 步骤 2：创建自定义事件类
class MyCustomEvent : public QEvent {
public:
    MyCustomEvent(const QString &message, int value)
        : QEvent(MyCustomEventType)
        , m_message(message)
        , m_value(value)
    {}
    
    QString message() const { return m_message; }
    int value() const { return m_value; }
    
private:
    QString m_message;
    int m_value;
};

// ✅ 步骤 3：处理自定义事件
class MyObject : public QObject {
    Q_OBJECT
    
protected:
    bool event(QEvent *event) override {
        if (event->type() == MyCustomEventType) {
            MyCustomEvent *customEvent = static_cast<MyCustomEvent*>(event);
            qDebug() << "Received custom event:"
                     << customEvent->message()
                     << customEvent->value();
            return true;  // 事件已处理
        }
        
        return QObject::event(event);
    }
};

// ✅ 步骤 4：发送自定义事件
void sendCustomEvent(QObject *receiver)
{
    MyCustomEvent *event = new MyCustomEvent("Hello", 42);
    
    // 方式 1：立即发送（同步）
    QCoreApplication::sendEvent(receiver, event);
    delete event;  // sendEvent 不会删除事件
    
    // 方式 2：投递到事件队列（异步）
    MyCustomEvent *event2 = new MyCustomEvent("World", 100);
    QCoreApplication::postEvent(receiver, event2);
    // postEvent 会自动删除事件，不要手动 delete
}
```

### 6.2 自定义事件的优先级

```cpp
#include <QCoreApplication>

// ✅ postEvent 支持优先级
void sendPriorityEvents(QObject *receiver)
{
    // 高优先级事件
    QCoreApplication::postEvent(receiver, 
        new MyCustomEvent("High Priority", 1),
        Qt::HighEventPriority);
    
    // 正常优先级事件（默认）
    QCoreApplication::postEvent(receiver,
        new MyCustomEvent("Normal Priority", 2));
    
    // 低优先级事件
    QCoreApplication::postEvent(receiver,
        new MyCustomEvent("Low Priority", 3),
        Qt::LowEventPriority);
    
    // 处理顺序：High → Normal → Low
}
```

### 6.3 实战：实现进度通知系统

```cpp
#include <QObject>
#include <QEvent>
#include <QCoreApplication>
#include <QThread>

// 进度事件
class ProgressEvent : public QEvent {
public:
    static const QEvent::Type EventType = 
        static_cast<QEvent::Type>(QEvent::User + 100);
    
    ProgressEvent(int current, int total, const QString &message)
        : QEvent(EventType)
        , m_current(current)
        , m_total(total)
        , m_message(message)
    {}
    
    int current() const { return m_current; }
    int total() const { return m_total; }
    QString message() const { return m_message; }
    int percentage() const { return (m_current * 100) / m_total; }
    
private:
    int m_current;
    int m_total;
    QString m_message;
};

// 工作线程
class Worker : public QObject {
    Q_OBJECT
    
public:
    Worker(QObject *receiver) : m_receiver(receiver) {}
    
public slots:
    void doWork() {
        int total = 100;
        
        for (int i = 0; i <= total; ++i) {
            // 模拟工作
            QThread::msleep(50);
            
            // ✅ 发送进度事件到主线程
            ProgressEvent *event = new ProgressEvent(
                i, total, QString("Processing item %1").arg(i)
            );
            QCoreApplication::postEvent(m_receiver, event);
        }
        
        emit finished();
    }
    
signals:
    void finished();
    
private:
    QObject *m_receiver;
};

// 主窗口
class MainWindow : public QWidget {
    Q_OBJECT
    
protected:
    bool event(QEvent *event) override {
        if (event->type() == ProgressEvent::EventType) {
            ProgressEvent *progressEvent = static_cast<ProgressEvent*>(event);
            
            // 更新 UI
            qDebug() << progressEvent->percentage() << "%"
                     << progressEvent->message();
            
            // 更新进度条
            // progressBar->setValue(progressEvent->percentage());
            
            return true;
        }
        
        return QWidget::event(event);
    }
};

// 使用示例
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    MainWindow window;
    window.show();
    
    QThread *thread = new QThread;
    Worker *worker = new Worker(&window);
    worker->moveToThread(thread);
    
    QObject::connect(thread, &QThread::started, worker, &Worker::doWork);
    QObject::connect(worker, &Worker::finished, thread, &QThread::quit);
    QObject::connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    thread->start();
    
    return app.exec();
}
```


---

## 7. QML 事件处理

### 7.1 QML 中的鼠标事件

```qml
import QtQuick

Rectangle {
    width: 400
    height: 300
    color: "lightblue"
    
    // ✅ MouseArea - QML 中处理鼠标事件的主要方式
    MouseArea {
        anchors.fill: parent
        
        // 基本鼠标事件
        onClicked: function(mouse) {
            console.log("Clicked at:", mouse.x, mouse.y)
            console.log("Button:", mouse.button)
            console.log("Modifiers:", mouse.modifiers)
            
            // mouse 对象属性：
            // - x, y: 相对于 MouseArea 的位置
            // - button: Qt.LeftButton, Qt.RightButton, Qt.MiddleButton
            // - buttons: 当前按下的所有按钮
            // - modifiers: Qt.ShiftModifier, Qt.ControlModifier, Qt.AltModifier
            // - accepted: 是否接受事件
        }
        
        onPressed: function(mouse) {
            console.log("Pressed")
        }
        
        onReleased: function(mouse) {
            console.log("Released")
        }
        
        onDoubleClicked: function(mouse) {
            console.log("Double clicked")
        }
        
        onPressAndHold: function(mouse) {
            console.log("Press and hold")
        }
        
        // 鼠标移动事件
        onPositionChanged: function(mouse) {
            console.log("Mouse moved to:", mouse.x, mouse.y)
        }
        
        // 鼠标进入/离开
        onEntered: {
            console.log("Mouse entered")
            parent.color = "lightgreen"
        }
        
        onExited: {
            console.log("Mouse exited")
            parent.color = "lightblue"
        }
        
        // 鼠标滚轮
        onWheel: function(wheel) {
            console.log("Wheel delta:", wheel.angleDelta.y)
            // wheel.angleDelta.x: 水平滚动
            // wheel.angleDelta.y: 垂直滚动
        }
        
        // 配置选项
        acceptedButtons: Qt.LeftButton | Qt.RightButton  // 接受的按钮
        hoverEnabled: true                               // 启用悬停事件
        cursorShape: Qt.PointingHandCursor              // 鼠标光标形状
        preventStealing: false                          // 防止事件被窃取
        propagateComposedEvents: false                  // 是否传播组合事件
    }
}
```

### 7.2 QML 中的键盘事件

```qml
import QtQuick

Rectangle {
    width: 400
    height: 300
    focus: true  // ✅ 必须设置 focus 才能接收键盘事件
    
    // ✅ Keys 附加属性
    Keys.onPressed: function(event) {
        console.log("Key pressed:", event.key)
        console.log("Text:", event.text)
        console.log("Modifiers:", event.modifiers)
        console.log("Auto repeat:", event.isAutoRepeat)
        
        // 处理特定按键
        if (event.key === Qt.Key_Escape) {
            console.log("Escape pressed")
            event.accepted = true  // 阻止事件传播
        }
        
        // 处理修饰键组合
        if (event.key === Qt.Key_S && event.modifiers & Qt.ControlModifier) {
            console.log("Ctrl+S pressed")
            event.accepted = true
        }
    }
    
    Keys.onReleased: function(event) {
        console.log("Key released:", event.key)
    }
    
    // ✅ 特定按键的快捷处理
    Keys.onReturnPressed: {
        console.log("Return/Enter pressed")
    }
    
    Keys.onEscapePressed: {
        console.log("Escape pressed")
    }
    
    Keys.onBackPressed: {
        console.log("Back pressed (Android)")
    }
    
    Keys.onMenuPressed: {
        console.log("Menu pressed (Android)")
    }
    
    // ✅ 方向键
    Keys.onUpPressed: {
        console.log("Up arrow")
    }
    
    Keys.onDownPressed: {
        console.log("Down arrow")
    }
    
    Keys.onLeftPressed: {
        console.log("Left arrow")
    }
    
    Keys.onRightPressed: {
        console.log("Right arrow")
    }
    
    // ✅ 数字键
    Keys.onDigit0Pressed: { console.log("0") }
    Keys.onDigit1Pressed: { console.log("1") }
    // ... Keys.onDigit9Pressed
    
    Text {
        anchors.centerIn: parent
        text: "Press any key\n(Click here first to get focus)"
        
        MouseArea {
            anchors.fill: parent
            onClicked: parent.parent.forceActiveFocus()
        }
    }
}
```

### 7.3 QML 焦点管理

```qml
import QtQuick
import QtQuick.Controls

Item {
    width: 400
    height: 300
    
    Column {
        spacing: 10
        
        // ✅ 焦点属性
        TextField {
            id: field1
            placeholderText: "Field 1"
            focus: true  // 初始焦点
            
            // 焦点事件
            onActiveFocusChanged: {
                if (activeFocus) {
                    console.log("Field 1 got focus")
                } else {
                    console.log("Field 1 lost focus")
                }
            }
            
            Keys.onTabPressed: {
                field2.forceActiveFocus()  // 手动设置焦点
            }
        }
        
        TextField {
            id: field2
            placeholderText: "Field 2"
            
            Keys.onTabPressed: {
                field3.forceActiveFocus()
            }
        }
        
        TextField {
            id: field3
            placeholderText: "Field 3"
            
            Keys.onTabPressed: {
                field1.forceActiveFocus()
            }
        }
        
        Button {
            text: "Focus Field 1"
            onClicked: field1.forceActiveFocus()
        }
    }
    
    // ✅ FocusScope - 焦点作用域
    FocusScope {
        width: 200
        height: 200
        
        Rectangle {
            anchors.fill: parent
            color: parent.activeFocus ? "lightgreen" : "lightgray"
            focus: true
            
            Text {
                anchors.centerIn: parent
                text: "Focus Scope"
            }
            
            Keys.onPressed: function(event) {
                console.log("FocusScope received key:", event.key)
            }
        }
    }
}
```

### 7.4 QML 触摸事件

```qml
import QtQuick

Rectangle {
    width: 400
    height: 300
    
    // ✅ MultiPointTouchArea - 多点触摸
    MultiPointTouchArea {
        anchors.fill: parent
        
        // 最小/最大触摸点数
        minimumTouchPoints: 1
        maximumTouchPoints: 5
        
        // 触摸点列表
        touchPoints: [
            TouchPoint { id: point1 },
            TouchPoint { id: point2 },
            TouchPoint { id: point3 }
        ]
        
        onPressed: function(touchPoints) {
            console.log("Touch pressed, count:", touchPoints.length)
            for (var i = 0; i < touchPoints.length; i++) {
                console.log("Point", i, "at:", touchPoints[i].x, touchPoints[i].y)
            }
        }
        
        onUpdated: function(touchPoints) {
            console.log("Touch updated")
        }
        
        onReleased: function(touchPoints) {
            console.log("Touch released")
        }
        
        onCanceled: function(touchPoints) {
            console.log("Touch canceled")
        }
        
        onGestureStarted: function(gesture) {
            console.log("Gesture started")
        }
    }
    
    // ✅ 显示触摸点
    Repeater {
        model: 5
        
        Rectangle {
            width: 50
            height: 50
            radius: 25
            color: "red"
            opacity: 0.5
            
            x: parent.touchPoints[index] ? parent.touchPoints[index].x - 25 : -100
            y: parent.touchPoints[index] ? parent.touchPoints[index].y - 25 : -100
            
            visible: parent.touchPoints[index] && parent.touchPoints[index].pressed
        }
    }
}
```

### 7.5 QML 手势识别

```qml
import QtQuick

Rectangle {
    width: 400
    height: 300
    color: "lightblue"
    
    // ✅ PinchArea - 捏合手势
    PinchArea {
        anchors.fill: parent
        
        onPinchStarted: function(pinch) {
            console.log("Pinch started")
            console.log("Center:", pinch.center)
            console.log("Scale:", pinch.scale)
            console.log("Angle:", pinch.angle)
        }
        
        onPinchUpdated: function(pinch) {
            // 缩放
            targetRect.scale = pinch.scale
            
            // 旋转
            targetRect.rotation = pinch.angle
            
            // 移动
            targetRect.x = pinch.center.x - targetRect.width / 2
            targetRect.y = pinch.center.y - targetRect.height / 2
        }
        
        onPinchFinished: function(pinch) {
            console.log("Pinch finished")
            console.log("Final scale:", pinch.scale)
        }
        
        Rectangle {
            id: targetRect
            width: 100
            height: 100
            color: "red"
            anchors.centerIn: parent
        }
    }
    
    // ✅ 拖动手势
    Rectangle {
        id: draggableRect
        width: 100
        height: 100
        color: "green"
        
        Drag.active: dragArea.drag.active
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
        
        MouseArea {
            id: dragArea
            anchors.fill: parent
            
            drag.target: parent
            drag.axis: Drag.XAndYAxis
            drag.minimumX: 0
            drag.maximumX: parent.parent.width - parent.width
            drag.minimumY: 0
            drag.maximumY: parent.parent.height - parent.height
            
            onPressed: {
                console.log("Drag started")
            }
            
            onReleased: {
                console.log("Drag ended")
            }
        }
    }
}
```

### 7.6 QML 与 C++ 事件交互

```cpp
// C++ 端
#include <QObject>
#include <QEvent>
#include <QQmlEngine>

class EventBridge : public QObject {
    Q_OBJECT
    QML_ELEMENT
    
public:
    explicit EventBridge(QObject *parent = nullptr) : QObject(parent) {}
    
    // ✅ 从 C++ 发送事件到 QML
    Q_INVOKABLE void triggerCustomEvent(const QString &message) {
        emit customEventTriggered(message);
    }
    
signals:
    void customEventTriggered(const QString &message);
    
protected:
    // ✅ 接收来自 QML 的事件
    bool event(QEvent *event) override {
        if (event->type() == QEvent::User + 1) {
            qDebug() << "Received custom event from QML";
            return true;
        }
        return QObject::event(event);
    }
};
```

```qml
// QML 端
import QtQuick

Item {
    EventBridge {
        id: bridge
        
        // ✅ 接收来自 C++ 的事件
        onCustomEventTriggered: function(message) {
            console.log("Received from C++:", message)
        }
    }
    
    Button {
        text: "Trigger C++ Event"
        onClicked: {
            // ✅ 触发 C++ 端的事件
            bridge.triggerCustomEvent("Hello from QML")
        }
    }
}
```


---

## 8. 鼠标事件详解

### 8.1 鼠标事件的完整生命周期

```cpp
class MouseEventWidget : public QWidget {
protected:
    // ✅ 鼠标事件的完整流程
    void enterEvent(QEnterEvent *event) override {
        qDebug() << "1. Mouse entered widget";
        QWidget::enterEvent(event);
    }
    
    void mousePressEvent(QMouseEvent *event) override {
        qDebug() << "2. Mouse button pressed";
        qDebug() << "   Position:" << event->pos();
        qDebug() << "   Global position:" << event->globalPos();
        qDebug() << "   Button:" << event->button();
        qDebug() << "   Buttons:" << event->buttons();
        qDebug() << "   Modifiers:" << event->modifiers();
        
        QWidget::mousePressEvent(event);
    }
    
    void mouseMoveEvent(QMouseEvent *event) override {
        qDebug() << "3. Mouse moved to:" << event->pos();
        // 注意：默认只在按下按钮时触发
        // 需要 setMouseTracking(true) 才能始终触发
        
        QWidget::mouseMoveEvent(event);
    }
    
    void mouseReleaseEvent(QMouseEvent *event) override {
        qDebug() << "4. Mouse button released";
        QWidget::mouseReleaseEvent(event);
    }
    
    void mouseDoubleClickEvent(QMouseEvent *event) override {
        qDebug() << "5. Mouse double clicked";
        QWidget::mouseDoubleClickEvent(event);
    }
    
    void leaveEvent(QEvent *event) override {
        qDebug() << "6. Mouse left widget";
        QWidget::leaveEvent(event);
    }
    
    void wheelEvent(QWheelEvent *event) override {
        qDebug() << "7. Mouse wheel scrolled";
        qDebug() << "   Angle delta:" << event->angleDelta();
        qDebug() << "   Pixel delta:" << event->pixelDelta();
        
        QWidget::wheelEvent(event);
    }
};
```

### 8.2 鼠标追踪

```cpp
class MouseTrackingWidget : public QWidget {
public:
    MouseTrackingWidget() {
        // ✅ 启用鼠标追踪（即使没有按下按钮也触发 mouseMoveEvent）
        setMouseTracking(true);
    }
    
protected:
    void mouseMoveEvent(QMouseEvent *event) override {
        // 现在即使没有按下按钮也会触发
        qDebug() << "Mouse at:" << event->pos();
        
        // 更新光标下的信息
        updateCursorInfo(event->pos());
        
        QWidget::mouseMoveEvent(event);
    }
    
    void updateCursorInfo(const QPoint &pos) {
        // 显示坐标、颜色等信息
    }
};
```

### 8.3 鼠标光标

```cpp
#include <QCursor>

class CursorWidget : public QWidget {
public:
    CursorWidget() {
        // ✅ 设置光标形状
        setCursor(Qt::PointingHandCursor);
        
        // 可用的光标形状：
        // - Qt::ArrowCursor: 标准箭头
        // - Qt::UpArrowCursor: 向上箭头
        // - Qt::CrossCursor: 十字
        // - Qt::WaitCursor: 等待（沙漏/旋转）
        // - Qt::IBeamCursor: I 型（文本）
        // - Qt::SizeVerCursor: 垂直调整大小
        // - Qt::SizeHorCursor: 水平调整大小
        // - Qt::SizeBDiagCursor: 对角线调整大小
        // - Qt::SizeFDiagCursor: 反对角线调整大小
        // - Qt::SizeAllCursor: 全方向调整大小
        // - Qt::BlankCursor: 隐藏光标
        // - Qt::SplitVCursor: 垂直分割
        // - Qt::SplitHCursor: 水平分割
        // - Qt::PointingHandCursor: 手型
        // - Qt::ForbiddenCursor: 禁止
        // - Qt::WhatsThisCursor: What's This
        // - Qt::BusyCursor: 忙碌
        // - Qt::OpenHandCursor: 打开的手
        // - Qt::ClosedHandCursor: 关闭的手
        // - Qt::DragCopyCursor: 拖动复制
        // - Qt::DragMoveCursor: 拖动移动
        // - Qt::DragLinkCursor: 拖动链接
    }
    
protected:
    void mouseMoveEvent(QMouseEvent *event) override {
        // ✅ 根据位置动态改变光标
        if (event->pos().x() < width() / 2) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::CrossCursor);
        }
        
        QWidget::mouseMoveEvent(event);
    }
    
    void enterEvent(QEnterEvent *event) override {
        // ✅ 进入时改变光标
        setCursor(Qt::PointingHandCursor);
        QWidget::enterEvent(event);
    }
    
    void leaveEvent(QEvent *event) override {
        // ✅ 离开时恢复光标
        unsetCursor();  // 恢复父窗口的光标
        QWidget::leaveEvent(event);
    }
};

// ✅ 自定义光标
void setCustomCursor() {
    QPixmap pixmap(":/cursor.png");
    QCursor cursor(pixmap, 0, 0);  // 热点在 (0, 0)
    widget->setCursor(cursor);
}

// ✅ 全局光标
void setGlobalCursor() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    // 执行耗时操作...
    
    QApplication::restoreOverrideCursor();
}
```

### 8.4 鼠标事件的坐标系统

```cpp
void mousePressEvent(QMouseEvent *event) override {
    // ✅ 相对于接收者的位置
    QPoint localPos = event->pos();
    qDebug() << "Local position:" << localPos;
    
    // ✅ 屏幕绝对位置
    QPoint globalPos = event->globalPos();
    qDebug() << "Global position:" << globalPos;
    
    // ✅ 相对于窗口的位置
    QPoint windowPos = event->windowPos().toPoint();
    qDebug() << "Window position:" << windowPos;
    
    // ✅ 转换坐标
    QPoint parentPos = mapToParent(localPos);
    QPoint globalFromLocal = mapToGlobal(localPos);
    QPoint localFromGlobal = mapFromGlobal(globalPos);
    
    // ✅ 相对于其他控件的位置
    QWidget *otherWidget = findChild<QWidget*>("other");
    if (otherWidget) {
        QPoint otherPos = otherWidget->mapFrom(this, localPos);
        qDebug() << "Position in other widget:" << otherPos;
    }
}
```

---

## 9. 键盘事件详解

### 9.1 键盘事件处理

```cpp
class KeyboardWidget : public QWidget {
protected:
    void keyPressEvent(QKeyEvent *event) override {
        qDebug() << "Key pressed";
        qDebug() << "   Key:" << event->key();
        qDebug() << "   Text:" << event->text();
        qDebug() << "   Modifiers:" << event->modifiers();
        qDebug() << "   Native scan code:" << event->nativeScanCode();
        qDebug() << "   Auto repeat:" << event->isAutoRepeat();
        qDebug() << "   Count:" << event->count();
        
        // ✅ 处理特定按键
        switch (event->key()) {
        case Qt::Key_Escape:
            qDebug() << "Escape pressed";
            close();
            break;
            
        case Qt::Key_Return:
        case Qt::Key_Enter:
            qDebug() << "Enter pressed";
            break;
            
        case Qt::Key_Space:
            qDebug() << "Space pressed";
            break;
            
        case Qt::Key_Tab:
            qDebug() << "Tab pressed";
            break;
            
        case Qt::Key_Backspace:
            qDebug() << "Backspace pressed";
            break;
            
        case Qt::Key_Delete:
            qDebug() << "Delete pressed";
            break;
            
        default:
            break;
        }
        
        // ✅ 处理修饰键组合
        if (event->modifiers() & Qt::ControlModifier) {
            if (event->key() == Qt::Key_C) {
                qDebug() << "Ctrl+C pressed";
                event->accept();
                return;
            }
            if (event->key() == Qt::Key_V) {
                qDebug() << "Ctrl+V pressed";
                event->accept();
                return;
            }
        }
        
        if (event->modifiers() & Qt::ShiftModifier) {
            qDebug() << "Shift is pressed";
        }
        
        if (event->modifiers() & Qt::AltModifier) {
            qDebug() << "Alt is pressed";
        }
        
        QWidget::keyPressEvent(event);
    }
    
    void keyReleaseEvent(QKeyEvent *event) override {
        qDebug() << "Key released:" << event->key();
        QWidget::keyReleaseEvent(event);
    }
};
```

### 9.2 快捷键

```cpp
#include <QShortcut>
#include <QKeySequence>

class ShortcutWidget : public QWidget {
public:
    ShortcutWidget() {
        // ✅ 方式 1：使用 QShortcut
        QShortcut *saveShortcut = new QShortcut(QKeySequence::Save, this);
        connect(saveShortcut, &QShortcut::activated, this, &ShortcutWidget::save);
        
        // ✅ 自定义快捷键
        QShortcut *customShortcut = new QShortcut(
            QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), this);
        connect(customShortcut, &QShortcut::activated, []() {
            qDebug() << "Ctrl+Shift+S pressed";
        });
        
        // ✅ 标准快捷键
        new QShortcut(QKeySequence::New, this, SLOT(newFile()));
        new QShortcut(QKeySequence::Open, this, SLOT(openFile()));
        new QShortcut(QKeySequence::Save, this, SLOT(saveFile()));
        new QShortcut(QKeySequence::SaveAs, this, SLOT(saveFileAs()));
        new QShortcut(QKeySequence::Close, this, SLOT(closeFile()));
        new QShortcut(QKeySequence::Quit, this, SLOT(quit()));
        
        new QShortcut(QKeySequence::Undo, this, SLOT(undo()));
        new QShortcut(QKeySequence::Redo, this, SLOT(redo()));
        new QShortcut(QKeySequence::Cut, this, SLOT(cut()));
        new QShortcut(QKeySequence::Copy, this, SLOT(copy()));
        new QShortcut(QKeySequence::Paste, this, SLOT(paste()));
        new QShortcut(QKeySequence::Delete, this, SLOT(deleteSelected()));
        new QShortcut(QKeySequence::SelectAll, this, SLOT(selectAll()));
        
        new QShortcut(QKeySequence::Find, this, SLOT(find()));
        new QShortcut(QKeySequence::FindNext, this, SLOT(findNext()));
        new QShortcut(QKeySequence::FindPrevious, this, SLOT(findPrevious()));
        new QShortcut(QKeySequence::Replace, this, SLOT(replace()));
        
        new QShortcut(QKeySequence::Print, this, SLOT(print()));
        new QShortcut(QKeySequence::Help, this, SLOT(help()));
        new QShortcut(QKeySequence::Preferences, this, SLOT(preferences()));
    }
    
private slots:
    void save() {
        qDebug() << "Save triggered";
    }
    
    void newFile() { qDebug() << "New file"; }
    void openFile() { qDebug() << "Open file"; }
    void saveFile() { qDebug() << "Save file"; }
    void saveFileAs() { qDebug() << "Save file as"; }
    void closeFile() { qDebug() << "Close file"; }
    void quit() { qDebug() << "Quit"; }
    
    void undo() { qDebug() << "Undo"; }
    void redo() { qDebug() << "Redo"; }
    void cut() { qDebug() << "Cut"; }
    void copy() { qDebug() << "Copy"; }
    void paste() { qDebug() << "Paste"; }
    void deleteSelected() { qDebug() << "Delete"; }
    void selectAll() { qDebug() << "Select all"; }
    
    void find() { qDebug() << "Find"; }
    void findNext() { qDebug() << "Find next"; }
    void findPrevious() { qDebug() << "Find previous"; }
    void replace() { qDebug() << "Replace"; }
    
    void print() { qDebug() << "Print"; }
    void help() { qDebug() << "Help"; }
    void preferences() { qDebug() << "Preferences"; }
};
```

### 9.3 输入法事件

```cpp
class InputMethodWidget : public QWidget {
protected:
    // ✅ 输入法事件
    void inputMethodEvent(QInputMethodEvent *event) override {
        qDebug() << "Input method event";
        qDebug() << "   Commit string:" << event->commitString();
        qDebug() << "   Preedit string:" << event->preeditString();
        
        // 处理输入法输入
        QString text = event->commitString();
        if (!text.isEmpty()) {
            // 插入文本
            insertText(text);
        }
        
        QWidget::inputMethodEvent(event);
    }
    
    // ✅ 输入法查询
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override {
        switch (query) {
        case Qt::ImCursorRectangle:
            // 返回光标矩形
            return QRect(cursorPos(), QSize(1, fontMetrics().height()));
            
        case Qt::ImFont:
            // 返回字体
            return font();
            
        case Qt::ImCursorPosition:
            // 返回光标位置
            return cursorPosition();
            
        case Qt::ImSurroundingText:
            // 返回周围文本
            return surroundingText();
            
        case Qt::ImCurrentSelection:
            // 返回当前选择
            return selectedText();
            
        default:
            break;
        }
        
        return QWidget::inputMethodQuery(query);
    }
    
private:
    void insertText(const QString &text) {
        // 实现文本插入
    }
    
    QPoint cursorPos() const {
        // 返回光标位置
        return QPoint(0, 0);
    }
    
    int cursorPosition() const {
        return 0;
    }
    
    QString surroundingText() const {
        return QString();
    }
    
    QString selectedText() const {
        return QString();
    }
};
```


---

## 10. 触摸与手势事件

### 10.1 触摸事件处理

```cpp
#include <QTouchEvent>

class TouchWidget : public QWidget {
public:
    TouchWidget() {
        // ✅ 启用触摸事件
        setAttribute(Qt::WA_AcceptTouchEvents);
    }
    
protected:
    bool event(QEvent *event) override {
        switch (event->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
        case QEvent::TouchCancel:
            return handleTouchEvent(static_cast<QTouchEvent*>(event));
        default:
            break;
        }
        
        return QWidget::event(event);
    }
    
    bool handleTouchEvent(QTouchEvent *event) {
        qDebug() << "Touch event:" << event->type();
        qDebug() << "   Touch points:" << event->points().size();
        
        // ✅ 遍历所有触摸点
        for (const QEventPoint &touchPoint : event->points()) {
            qDebug() << "   Point ID:" << touchPoint.id();
            qDebug() << "   State:" << touchPoint.state();
            qDebug() << "   Position:" << touchPoint.position();
            qDebug() << "   Global position:" << touchPoint.globalPosition();
            qDebug() << "   Pressure:" << touchPoint.pressure();
            qDebug() << "   Ellipse diameters:" << touchPoint.ellipseDiameters();
            
            // 触摸点状态：
            // - QEventPoint::Pressed: 按下
            // - QEventPoint::Updated: 移动
            // - QEventPoint::Stationary: 静止
            // - QEventPoint::Released: 释放
            
            switch (touchPoint.state()) {
            case QEventPoint::Pressed:
                qDebug() << "   Touch point pressed";
                break;
            case QEventPoint::Updated:
                qDebug() << "   Touch point moved";
                break;
            case QEventPoint::Released:
                qDebug() << "   Touch point released";
                break;
            default:
                break;
            }
        }
        
        event->accept();
        return true;
    }
};
```

### 10.2 手势识别

```cpp
#include <QGesture>
#include <QGestureEvent>

class GestureWidget : public QWidget {
public:
    GestureWidget() {
        // ✅ 启用手势识别
        grabGesture(Qt::PinchGesture);
        grabGesture(Qt::PanGesture);
        grabGesture(Qt::SwipeGesture);
        grabGesture(Qt::TapGesture);
        grabGesture(Qt::TapAndHoldGesture);
    }
    
protected:
    bool event(QEvent *event) override {
        if (event->type() == QEvent::Gesture) {
            return gestureEvent(static_cast<QGestureEvent*>(event));
        }
        return QWidget::event(event);
    }
    
    bool gestureEvent(QGestureEvent *event) {
        qDebug() << "Gesture event";
        
        // ✅ 捏合手势
        if (QGesture *pinch = event->gesture(Qt::PinchGesture)) {
            handlePinchGesture(static_cast<QPinchGesture*>(pinch));
        }
        
        // ✅ 平移手势
        if (QGesture *pan = event->gesture(Qt::PanGesture)) {
            handlePanGesture(static_cast<QPanGesture*>(pan));
        }
        
        // ✅ 滑动手势
        if (QGesture *swipe = event->gesture(Qt::SwipeGesture)) {
            handleSwipeGesture(static_cast<QSwipeGesture*>(swipe));
        }
        
        // ✅ 点击手势
        if (QGesture *tap = event->gesture(Qt::TapGesture)) {
            handleTapGesture(static_cast<QTapGesture*>(tap));
        }
        
        // ✅ 长按手势
        if (QGesture *tapAndHold = event->gesture(Qt::TapAndHoldGesture)) {
            handleTapAndHoldGesture(static_cast<QTapAndHoldGesture*>(tapAndHold));
        }
        
        return true;
    }
    
    void handlePinchGesture(QPinchGesture *gesture) {
        qDebug() << "Pinch gesture";
        qDebug() << "   State:" << gesture->state();
        qDebug() << "   Scale factor:" << gesture->scaleFactor();
        qDebug() << "   Total scale factor:" << gesture->totalScaleFactor();
        qDebug() << "   Rotation angle:" << gesture->rotationAngle();
        qDebug() << "   Total rotation angle:" << gesture->totalRotationAngle();
        qDebug() << "   Center point:" << gesture->centerPoint();
        
        if (gesture->state() == Qt::GestureFinished) {
            // 应用缩放和旋转
            qreal scale = gesture->totalScaleFactor();
            qreal rotation = gesture->totalRotationAngle();
            
            qDebug() << "Final scale:" << scale;
            qDebug() << "Final rotation:" << rotation;
        }
    }
    
    void handlePanGesture(QPanGesture *gesture) {
        qDebug() << "Pan gesture";
        qDebug() << "   Delta:" << gesture->delta();
        qDebug() << "   Offset:" << gesture->offset();
        qDebug() << "   Acceleration:" << gesture->acceleration();
        
        if (gesture->state() == Qt::GestureUpdated) {
            // 移动对象
            QPointF delta = gesture->delta();
            qDebug() << "Moving by:" << delta;
        }
    }
    
    void handleSwipeGesture(QSwipeGesture *gesture) {
        qDebug() << "Swipe gesture";
        qDebug() << "   Horizontal direction:" << gesture->horizontalDirection();
        qDebug() << "   Vertical direction:" << gesture->verticalDirection();
        
        if (gesture->state() == Qt::GestureFinished) {
            if (gesture->horizontalDirection() == QSwipeGesture::Left) {
                qDebug() << "Swiped left";
            } else if (gesture->horizontalDirection() == QSwipeGesture::Right) {
                qDebug() << "Swiped right";
            }
            
            if (gesture->verticalDirection() == QSwipeGesture::Up) {
                qDebug() << "Swiped up";
            } else if (gesture->verticalDirection() == QSwipeGesture::Down) {
                qDebug() << "Swiped down";
            }
        }
    }
    
    void handleTapGesture(QTapGesture *gesture) {
        qDebug() << "Tap gesture";
        qDebug() << "   Position:" << gesture->position();
        
        if (gesture->state() == Qt::GestureFinished) {
            qDebug() << "Tapped at:" << gesture->position();
        }
    }
    
    void handleTapAndHoldGesture(QTapAndHoldGesture *gesture) {
        qDebug() << "Tap and hold gesture";
        qDebug() << "   Position:" << gesture->position();
        
        if (gesture->state() == Qt::GestureFinished) {
            qDebug() << "Long press at:" << gesture->position();
        }
    }
};
```

---

## 11. 定时器事件

### 11.1 QTimer 使用

```cpp
#include <QTimer>

class TimerExample : public QObject {
    Q_OBJECT
    
public:
    TimerExample() {
        // ✅ 方式 1：单次定时器
        QTimer::singleShot(1000, this, &TimerExample::onTimeout);
        
        // ✅ 方式 2：重复定时器
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &TimerExample::onTimeout);
        timer->start(1000);  // 每秒触发一次
        
        // ✅ 方式 3：精确定时器
        QTimer *preciseTimer = new QTimer(this);
        preciseTimer->setTimerType(Qt::PreciseTimer);  // 精确定时
        // - Qt::PreciseTimer: 精确到毫秒（默认）
        // - Qt::CoarseTimer: 粗略定时（5% 误差）
        // - Qt::VeryCoarseTimer: 非常粗略（最多 1 秒误差）
        preciseTimer->start(100);
        
        // ✅ 方式 4：使用 Lambda
        QTimer *lambdaTimer = new QTimer(this);
        connect(lambdaTimer, &QTimer::timeout, []() {
            qDebug() << "Lambda timer triggered";
        });
        lambdaTimer->start(2000);
    }
    
private slots:
    void onTimeout() {
        qDebug() << "Timer timeout";
    }
};
```

### 11.2 基于事件的定时器

```cpp
class TimerEventWidget : public QWidget {
public:
    TimerEventWidget() {
        // ✅ 启动定时器（返回定时器 ID）
        m_timerId = startTimer(1000);  // 1 秒
        
        // ✅ 启动多个定时器
        m_fastTimerId = startTimer(100);   // 100 毫秒
        m_slowTimerId = startTimer(5000);  // 5 秒
    }
    
    ~TimerEventWidget() {
        // ✅ 停止定时器
        killTimer(m_timerId);
        killTimer(m_fastTimerId);
        killTimer(m_slowTimerId);
    }
    
protected:
    void timerEvent(QTimerEvent *event) override {
        // ✅ 根据定时器 ID 处理不同的定时器
        if (event->timerId() == m_timerId) {
            qDebug() << "Main timer";
        } else if (event->timerId() == m_fastTimerId) {
            qDebug() << "Fast timer";
        } else if (event->timerId() == m_slowTimerId) {
            qDebug() << "Slow timer";
        }
        
        QWidget::timerEvent(event);
    }
    
private:
    int m_timerId;
    int m_fastTimerId;
    int m_slowTimerId;
};
```

---

## 12. 事件优先级与队列

### 12.1 事件优先级

```cpp
#include <QCoreApplication>

// Qt 事件优先级（从高到低）：
// 1. Qt::HighEventPriority (1)
// 2. Qt::NormalEventPriority (0) - 默认
// 3. Qt::LowEventPriority (-1)

void postPriorityEvents(QObject *receiver) {
    // ✅ 高优先级事件（先处理）
    QCoreApplication::postEvent(receiver, 
        new QEvent(QEvent::User),
        Qt::HighEventPriority);
    
    // ✅ 正常优先级事件
    QCoreApplication::postEvent(receiver,
        new QEvent(QEvent::User + 1));
    
    // ✅ 低优先级事件（最后处理）
    QCoreApplication::postEvent(receiver,
        new QEvent(QEvent::User + 2),
        Qt::LowEventPriority);
}
```

### 12.2 事件队列管理

```cpp
#include <QCoreApplication>

class EventQueueExample {
public:
    static void demonstrateEventQueue() {
        // ✅ 发送事件（同步，立即处理）
        QEvent event(QEvent::User);
        QCoreApplication::sendEvent(receiver, &event);
        // 事件立即被处理，函数返回后事件已处理完毕
        
        // ✅ 投递事件（异步，加入队列）
        QCoreApplication::postEvent(receiver, new QEvent(QEvent::User));
        // 事件被加入队列，稍后在事件循环中处理
        // 注意：postEvent 会自动删除事件，不要手动 delete
        
        // ✅ 发送已投递的事件
        QCoreApplication::sendPostedEvents();
        // 立即处理所有已投递的事件
        
        // ✅ 发送特定类型的已投递事件
        QCoreApplication::sendPostedEvents(receiver, QEvent::User);
        // 只处理特定接收者的特定类型事件
        
        // ✅ 移除已投递的事件
        QCoreApplication::removePostedEvents(receiver);
        // 移除所有发送给 receiver 的事件
        
        QCoreApplication::removePostedEvents(receiver, QEvent::User);
        // 只移除特定类型的事件
    }
};
```

### 12.3 事件压缩

```cpp
// Qt 会自动压缩某些类型的事件，避免队列过载

class EventCompressionExample : public QWidget {
protected:
    void resizeEvent(QResizeEvent *event) override {
        // ✅ Resize 事件会被压缩
        // 如果队列中已有 Resize 事件，新的会替换旧的
        // 这样可以避免处理大量中间状态的 resize
        
        qDebug() << "Resize to:" << event->size();
        QWidget::resizeEvent(event);
    }
    
    void paintEvent(QPaintEvent *event) override {
        // ✅ Paint 事件也会被压缩
        // 多个 paint 请求会被合并成一个
        
        qDebug() << "Paint region:" << event->region();
        QWidget::paintEvent(event);
    }
    
    void mouseMoveEvent(QMouseEvent *event) override {
        // ✅ MouseMove 事件会被压缩
        // 如果处理速度跟不上，中间的移动事件会被跳过
        
        qDebug() << "Mouse at:" << event->pos();
        QWidget::mouseMoveEvent(event);
    }
};

// 可以被压缩的事件类型：
// - QEvent::Resize
// - QEvent::Paint
// - QEvent::MouseMove
// - QEvent::LayoutRequest
// - QEvent::UpdateRequest
```

---

## 13. 最佳实践与性能优化

### 13.1 事件处理最佳实践

```cpp
class BestPracticesWidget : public QWidget {
protected:
    // ✅ 1. 总是调用基类实现（除非你知道自己在做什么）
    void mousePressEvent(QMouseEvent *event) override {
        // 你的处理逻辑
        handleMousePress(event);
        
        // 调用基类
        QWidget::mousePressEvent(event);
    }
    
    // ✅ 2. 正确使用 accept() 和 ignore()
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Escape) {
            // 处理了事件，接受它
            event->accept();
            close();
            return;  // 不调用基类
        }
        
        // 没有处理，传递给父对象
        event->ignore();
        // 或调用基类
        QWidget::keyPressEvent(event);
    }
    
    // ✅ 3. 避免在事件处理器中做耗时操作
    void paintEvent(QPaintEvent *event) override {
        // ❌ 不要在 paintEvent 中做耗时操作
        // heavyCalculation();  // 错误！
        
        // ✅ 使用缓存
        if (m_cachedPixmap.isNull()) {
            m_cachedPixmap = generatePixmap();
        }
        
        QPainter painter(this);
        painter.drawPixmap(0, 0, m_cachedPixmap);
    }
    
    // ✅ 4. 使用事件过滤器而不是子类化
    bool eventFilter(QObject *watched, QEvent *event) override {
        // 比子类化更灵活
        if (watched == someWidget && event->type() == QEvent::KeyPress) {
            // 处理事件
            return true;
        }
        return QWidget::eventFilter(watched, event);
    }
    
private:
    QPixmap m_cachedPixmap;
    
    void handleMousePress(QMouseEvent *event) {
        // 实际的处理逻辑
    }
    
    QPixmap generatePixmap() {
        // 生成缓存的 pixmap
        return QPixmap();
    }
};
```

### 13.2 性能优化技巧

```cpp
// ✅ 1. 减少不必要的事件
class OptimizedWidget : public QWidget {
public:
    OptimizedWidget() {
        // 如果不需要鼠标追踪，不要启用
        // setMouseTracking(true);  // 只在需要时启用
        
        // 如果不需要触摸事件，不要启用
        // setAttribute(Qt::WA_AcceptTouchEvents);
    }
    
protected:
    // ✅ 2. 使用事件压缩
    void mouseMoveEvent(QMouseEvent *event) override {
        // 如果处理很慢，考虑跳过一些事件
        static QTime lastUpdate;
        if (lastUpdate.elapsed() < 16) {  // 限制为 60 FPS
            return;
        }
        lastUpdate.start();
        
        // 处理事件
        updateMousePosition(event->pos());
    }
    
    // ✅ 3. 批量处理事件
    void timerEvent(QTimerEvent *event) override {
        // 批量处理累积的数据
        processPendingData();
    }
    
private:
    void updateMousePosition(const QPoint &pos) {
        // 更新位置
    }
    
    void processPendingData() {
        // 批量处理
    }
};

// ✅ 4. 使用 QCoreApplication::processEvents() 保持响应
void longRunningOperation() {
    for (int i = 0; i < 1000000; ++i) {
        // 执行操作
        doSomething(i);
        
        // 每 1000 次迭代处理一次事件
        if (i % 1000 == 0) {
            QCoreApplication::processEvents();
        }
    }
}
```

---

## 14. 快速参考

### 14.1 常用事件类型

| 事件类型 | 说明 | 处理函数 |
|---------|------|---------|
| `MouseButtonPress` | 鼠标按下 | `mousePressEvent()` |
| `MouseButtonRelease` | 鼠标释放 | `mouseReleaseEvent()` |
| `MouseMove` | 鼠标移动 | `mouseMoveEvent()` |
| `KeyPress` | 键盘按下 | `keyPressEvent()` |
| `KeyRelease` | 键盘释放 | `keyReleaseEvent()` |
| `Paint` | 绘制 | `paintEvent()` |
| `Resize` | 调整大小 | `resizeEvent()` |
| `Close` | 关闭 | `closeEvent()` |
| `Timer` | 定时器 | `timerEvent()` |
| `FocusIn` | 获得焦点 | `focusInEvent()` |
| `FocusOut` | 失去焦点 | `focusOutEvent()` |

### 14.2 事件处理流程

```
1. 事件产生（系统/应用程序）
   ↓
2. QCoreApplication::notify()
   ↓
3. 事件过滤器（如果有）
   ↓
4. QObject::event()
   ↓
5. 特定事件处理器（如 mousePressEvent()）
   ↓
6. 事件被接受或传播给父对象
```

### 14.3 事件 vs 信号槽

| 特性 | 事件 | 信号槽 |
|------|------|--------|
| **抽象层次** | 底层 | 高层 |
| **类型安全** | 需要转换 | 类型安全 |
| **传播** | 可以传播给父对象 | 不传播 |
| **过滤** | 可以过滤 | 不能过滤 |
| **跨线程** | 需要手动处理 | 自动处理 |
| **性能** | 更快 | 略慢 |
| **使用场景** | 底层输入、系统事件 | 对象间通信 |

---

## 结语

Qt 的事件系统是整个框架的核心机制之一，理解事件系统对于开发高质量的 Qt 应用至关重要。

**关键要点：**

1. ✅ **事件循环**：`app.exec()` 启动事件循环，处理所有事件
2. ✅ **事件分发**：事件从 `notify()` → `event()` → 特定处理器
3. ✅ **事件过滤**：使用 `eventFilter()` 拦截和修改事件
4. ✅ **自定义事件**：继承 `QEvent` 创建自定义事件类型
5. ✅ **QML 事件**：通过 MouseArea、Keys 等处理事件
6. ✅ **性能优化**：避免耗时操作、使用事件压缩、批量处理

**最佳实践：**
- 总是调用基类的事件处理器
- 正确使用 `accept()` 和 `ignore()`
- 避免在事件处理器中做耗时操作
- 优先使用信号槽，必要时才使用事件
- 使用事件过滤器而不是过度子类化

---

*文档版本：1.0*  
*最后更新：2025-11-04*  
*适用于：Qt 6.x*  
*作者：Kiro AI Assistant*
