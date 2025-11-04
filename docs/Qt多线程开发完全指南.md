# Qt 多线程开发完全指南

> 本文档详细介绍 Qt 中的多线程编程，包括 std::thread、QThread、QML WorkerScript、线程池、信号槽跨线程通信等核心技术。

## 目录

1. [多线程基础概念](#1-多线程基础概念)
2. [std::thread 在 Qt 中的使用](#2-stdthread-在-qt-中的使用)
3. [QThread 详解](#3-qthread-详解)
4. [QML WorkerScript](#4-qml-workerscript)
5. [QThreadPool 线程池](#5-qthreadpool-线程池)
6. [QtConcurrent 高级并发](#6-qtconcurrent-高级并发)
7. [信号槽跨线程通信](#7-信号槽跨线程通信)
8. [线程同步机制](#8-线程同步机制)
9. [QML 与 C++ 线程交互](#9-qml-与-c-线程交互)
10. [最佳实践与常见陷阱](#10-最佳实践与常见陷阱)

---

## 1. 多线程基础概念

### 1.1 为什么需要多线程？

**主要场景：**
- 🔄 **耗时操作**：文件 I/O、网络请求、数据库查询
- 🎨 **保持 UI 响应**：避免阻塞主线程（GUI 线程）
- ⚡ **并行计算**：充分利用多核 CPU
- 🔔 **后台任务**：日志记录、数据同步、监控

### 1.2 Qt 中的线程模型

```
主线程（GUI 线程）
├── 事件循环（QEventLoop）
├── UI 更新（必须在主线程）
└── 信号槽处理

工作线程
├── 独立的事件循环（可选）
├── 耗时任务处理
└── 通过信号槽与主线程通信
```

### 1.3 Qt 多线程方案对比

| 方案 | 适用场景 | 优点 | 缺点 |
|------|---------|------|------|
| **std::thread** | 简单任务，不需要 Qt 特性 | 标准 C++，轻量 | 无事件循环，无信号槽 |
| **QThread** | 需要事件循环和信号槽 | Qt 集成好，功能完整 | 相对复杂 |
| **QThreadPool** | 大量短期任务 | 自动管理，高效 | 不适合长期任务 |
| **QtConcurrent** | 并行算法（map/reduce） | 简单易用，高层抽象 | 灵活性较低 |
| **WorkerScript** | QML 中的轻量计算 | QML 原生，简单 | 功能受限，性能一般 |


---

## 2. std::thread 在 Qt 中的使用

### 2.1 基本用法

```cpp
#include <QCoreApplication>
#include <QDebug>
#include <thread>
#include <chrono>

// 简单的线程函数
void simpleTask() {
    qDebug() << "Thread ID:" << std::this_thread::get_id();
    qDebug() << "Doing work...";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    qDebug() << "Work done!";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 创建并启动线程
    std::thread worker(simpleTask);
    
    // 等待线程完成
    worker.join();
    
    return app.exec();
}
```

### 2.2 使用 Lambda 表达式

```cpp
#include <QCoreApplication>
#include <QDebug>
#include <thread>
#include <atomic>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    std::atomic<int> counter{0};
    
    // 使用 lambda 创建线程
    std::thread worker([&counter]() {
        for (int i = 0; i < 10; ++i) {
            counter++;
            qDebug() << "Counter:" << counter.load();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    // 分离线程（不等待完成）
    worker.detach();
    
    return app.exec();
}
```

### 2.3 传递参数

```cpp
#include <thread>
#include <QString>
#include <QDebug>

// 按值传递
void processData(int id, QString data) {
    qDebug() << "Thread" << id << "processing:" << data;
}

// 按引用传递（需要 std::ref）
void incrementCounter(int& counter) {
    counter++;
    qDebug() << "Counter incremented to:" << counter;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 按值传递参数
    std::thread t1(processData, 1, QString("Hello"));
    
    // 按引用传递参数
    int counter = 0;
    std::thread t2(incrementCounter, std::ref(counter));
    
    t1.join();
    t2.join();
    
    qDebug() << "Final counter:" << counter;
    
    return app.exec();
}
```


### 2.4 std::thread 的局限性

```cpp
#include <QObject>
#include <thread>

class Worker : public QObject {
    Q_OBJECT
signals:
    void resultReady(int result);
    
public:
    void doWork() {
        // ❌ 问题：std::thread 中无法直接使用信号槽
        // emit resultReady(42);  // 这不会正常工作！
        
        // ✅ 解决方案：使用 QMetaObject::invokeMethod
        QMetaObject::invokeMethod(this, [this]() {
            emit resultReady(42);
        }, Qt::QueuedConnection);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    Worker worker;
    
    // std::thread 中使用 Qt 对象需要小心
    std::thread t([&worker]() {
        worker.doWork();
    });
    
    t.join();
    
    return app.exec();
}
```

### 2.5 RAII 线程管理

```cpp
#include <thread>
#include <memory>

// 线程 RAII 包装器
class ThreadGuard {
public:
    explicit ThreadGuard(std::thread&& t) : thread_(std::move(t)) {}
    
    ~ThreadGuard() {
        if (thread_.joinable()) {
            thread_.join();  // 自动等待线程完成
        }
    }
    
    // 禁止拷贝
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
    
private:
    std::thread thread_;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    {
        // 使用 RAII 管理线程生命周期
        ThreadGuard guard(std::thread([]() {
            qDebug() << "Thread running...";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }));
        
        // 离开作用域时自动 join
    }
    
    qDebug() << "Thread completed";
    
    return app.exec();
}
```

### 2.6 何时使用 std::thread？

**✅ 适合使用：**
- 简单的后台任务，不需要 Qt 特性
- 需要与标准 C++ 代码兼容
- 轻量级任务，不需要事件循环

**❌ 不适合使用：**
- 需要使用 Qt 信号槽
- 需要事件循环处理
- 需要与 QML 交互
- 需要定时器等 Qt 特性


---

## 3. QThread 详解

### 3.1 QThread 基本用法（推荐方式）

```cpp
#include <QThread>
#include <QObject>
#include <QDebug>

// ✅ 推荐：将工作对象移动到线程
class Worker : public QObject {
    Q_OBJECT
    
public:
    explicit Worker(QObject *parent = nullptr) : QObject(parent) {}
    
public slots:
    // 工作函数（在工作线程中执行）
    void doWork(const QString &parameter) {
        qDebug() << "Worker thread:" << QThread::currentThread();
        qDebug() << "Processing:" << parameter;
        
        // 模拟耗时操作
        QThread::sleep(2);
        
        // 发送结果
        emit resultReady("Result: " + parameter);
    }
    
signals:
    void resultReady(const QString &result);
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "Main thread:" << QThread::currentThread();
    
    // 创建线程
    QThread *thread = new QThread;
    
    // 创建工作对象
    Worker *worker = new Worker;
    
    // ✅ 关键：将工作对象移动到线程
    worker->moveToThread(thread);
    
    // 连接信号槽
    QObject::connect(thread, &QThread::started, 
                     worker, [worker]() { worker->doWork("Hello"); });
    QObject::connect(worker, &Worker::resultReady, 
                     [](const QString &result) {
        qDebug() << "Result received:" << result;
    });
    
    // 清理：线程结束时删除对象
    QObject::connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    // 启动线程
    thread->start();
    
    return app.exec();
}
```

### 3.2 QThread 完整示例（带停止机制）

```cpp
#include <QThread>
#include <QObject>
#include <QDebug>
#include <QTimer>

class Worker : public QObject {
    Q_OBJECT
    
public:
    explicit Worker(QObject *parent = nullptr) 
        : QObject(parent), m_running(false) {}
    
public slots:
    void startWork() {
        m_running = true;
        qDebug() << "Work started in thread:" << QThread::currentThread();
        
        // 使用定时器进行周期性任务
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Worker::doPeriodicWork);
        timer->start(1000);  // 每秒执行一次
    }
    
    void stopWork() {
        m_running = false;
        qDebug() << "Work stopped";
        emit finished();
    }
    
private slots:
    void doPeriodicWork() {
        if (!m_running) return;
        
        qDebug() << "Doing periodic work...";
        emit progress(QDateTime::currentDateTime().toString());
    }
    
signals:
    void progress(const QString &message);
    void finished();
    
private:
    bool m_running;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QThread *thread = new QThread;
    Worker *worker = new Worker;
    
    worker->moveToThread(thread);
    
    // 线程启动时开始工作
    QObject::connect(thread, &QThread::started, worker, &Worker::startWork);
    
    // 接收进度更新
    QObject::connect(worker, &Worker::progress, [](const QString &msg) {
        qDebug() << "Progress:" << msg;
    });
    
    // 工作完成时退出线程
    QObject::connect(worker, &Worker::finished, thread, &QThread::quit);
    
    // 清理
    QObject::connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    // 启动线程
    thread->start();
    
    // 5 秒后停止工作
    QTimer::singleShot(5000, worker, &Worker::stopWork);
    
    return app.exec();
}
```


### 3.3 继承 QThread（不推荐，但常见）

```cpp
#include <QThread>
#include <QDebug>

// ❌ 不推荐：继承 QThread 并重写 run()
class WorkerThread : public QThread {
    Q_OBJECT
    
protected:
    // 重写 run() 方法
    void run() override {
        qDebug() << "Thread started:" << currentThread();
        
        // 执行耗时任务
        for (int i = 0; i < 5; ++i) {
            qDebug() << "Working..." << i;
            sleep(1);  // QThread::sleep()
            
            // 发送进度信号
            emit progress(i * 20);
        }
        
        qDebug() << "Thread finished";
        emit resultReady("Done");
    }
    
signals:
    void progress(int percentage);
    void resultReady(const QString &result);
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    WorkerThread *thread = new WorkerThread;
    
    // 连接信号
    QObject::connect(thread, &WorkerThread::progress, [](int p) {
        qDebug() << "Progress:" << p << "%";
    });
    
    QObject::connect(thread, &WorkerThread::resultReady, [](const QString &r) {
        qDebug() << "Result:" << r;
    });
    
    // 线程完成后自动删除
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    // 启动线程
    thread->start();
    
    return app.exec();
}
```

**为什么不推荐继承 QThread？**
1. ❌ `run()` 中的代码没有事件循环（除非手动调用 `exec()`）
2. ❌ 无法使用槽函数（槽函数在主线程执行）
3. ❌ 违反单一职责原则（线程管理 + 业务逻辑混合）
4. ✅ 推荐使用 `moveToThread()` 方式

### 3.4 QThread 事件循环

```cpp
#include <QThread>
#include <QObject>
#include <QTimer>
#include <QDebug>

class Worker : public QObject {
    Q_OBJECT
    
public slots:
    void doWork() {
        qDebug() << "Work started";
        
        // 创建定时器（需要事件循环）
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, []() {
            qDebug() << "Timer tick:" << QTime::currentTime().toString();
        });
        timer->start(1000);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QThread *thread = new QThread;
    Worker *worker = new Worker;
    
    worker->moveToThread(thread);
    
    QObject::connect(thread, &QThread::started, worker, &Worker::doWork);
    
    // ✅ 线程会自动运行事件循环（QThread::exec()）
    thread->start();
    
    return app.exec();
}
```


### 3.5 QThread 高级特性

#### 3.5.1 线程优先级
```cpp
QThread *thread = new QThread;

// 设置线程优先级
thread->setPriority(QThread::HighPriority);

// 优先级选项：
// - IdlePriority        // 最低优先级
// - LowestPriority
// - LowPriority
// - NormalPriority      // 默认
// - HighPriority
// - HighestPriority
// - TimeCriticalPriority // 最高优先级
// - InheritPriority     // 继承父线程优先级

thread->start();
```

#### 3.5.2 等待线程完成
```cpp
QThread *thread = new QThread;
Worker *worker = new Worker;
worker->moveToThread(thread);

thread->start();

// 等待线程完成（阻塞当前线程）
if (thread->wait(5000)) {  // 等待最多 5 秒
    qDebug() << "Thread finished successfully";
} else {
    qDebug() << "Thread timeout";
    thread->terminate();  // 强制终止（危险！）
}
```

#### 3.5.3 线程本地存储
```cpp
#include <QThreadStorage>

// 线程本地存储（每个线程有独立的副本）
QThreadStorage<int> threadLocalCounter;

void incrementCounter() {
    if (!threadLocalCounter.hasLocalData()) {
        threadLocalCounter.setLocalData(0);
    }
    
    int value = threadLocalCounter.localData();
    threadLocalCounter.setLocalData(value + 1);
    
    qDebug() << "Thread" << QThread::currentThread() 
             << "counter:" << threadLocalCounter.localData();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 在多个线程中调用，每个线程有独立的计数器
    QThread *t1 = QThread::create([]() {
        for (int i = 0; i < 5; ++i) incrementCounter();
    });
    
    QThread *t2 = QThread::create([]() {
        for (int i = 0; i < 3; ++i) incrementCounter();
    });
    
    t1->start();
    t2->start();
    
    t1->wait();
    t2->wait();
    
    return 0;
}
```

### 3.6 QThread 实战：文件下载器

```cpp
#include <QThread>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

class FileDownloader : public QObject {
    Q_OBJECT
    
public:
    explicit FileDownloader(QObject *parent = nullptr) 
        : QObject(parent), m_manager(new QNetworkAccessManager(this)) {
        
        connect(m_manager, &QNetworkAccessManager::finished,
                this, &FileDownloader::onDownloadFinished);
    }
    
public slots:
    void download(const QUrl &url, const QString &savePath) {
        qDebug() << "Downloading from:" << url.toString();
        m_savePath = savePath;
        
        QNetworkRequest request(url);
        QNetworkReply *reply = m_manager->get(request);
        
        // 监控下载进度
        connect(reply, &QNetworkReply::downloadProgress,
                this, &FileDownloader::onDownloadProgress);
    }
    
private slots:
    void onDownloadProgress(qint64 received, qint64 total) {
        if (total > 0) {
            int percentage = (received * 100) / total;
            emit progress(percentage);
        }
    }
    
    void onDownloadFinished(QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QFile file(m_savePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
                emit finished(true, "Download completed");
            } else {
                emit finished(false, "Failed to save file");
            }
        } else {
            emit finished(false, reply->errorString());
        }
        
        reply->deleteLater();
    }
    
signals:
    void progress(int percentage);
    void finished(bool success, const QString &message);
    
private:
    QNetworkAccessManager *m_manager;
    QString m_savePath;
};

// 使用示例
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QThread *thread = new QThread;
    FileDownloader *downloader = new FileDownloader;
    
    downloader->moveToThread(thread);
    
    QObject::connect(thread, &QThread::started, [downloader]() {
        downloader->download(
            QUrl("https://example.com/file.zip"),
            "downloaded_file.zip"
        );
    });
    
    QObject::connect(downloader, &FileDownloader::progress, [](int p) {
        qDebug() << "Download progress:" << p << "%";
    });
    
    QObject::connect(downloader, &FileDownloader::finished, 
                     [thread](bool success, const QString &msg) {
        qDebug() << "Download" << (success ? "succeeded" : "failed") << ":" << msg;
        thread->quit();
    });
    
    QObject::connect(thread, &QThread::finished, downloader, &QObject::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    thread->start();
    
    return app.exec();
}
```


---

## 4. QML WorkerScript

### 4.1 WorkerScript 基本用法

WorkerScript 允许在 QML 中运行 JavaScript 代码在独立线程中，避免阻塞 UI。

#### Main.qml
```qml
import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: "WorkerScript Demo"
    
    // ✅ 创建 WorkerScript
    WorkerScript {
        id: worker
        source: "worker.js"  // JavaScript 文件路径
        
        // 接收来自 Worker 的消息
        onMessage: function(messageObject) {
            console.log("Received from worker:", messageObject.result)
            resultText.text = messageObject.result
            progressBar.value = messageObject.progress || 0
        }
    }
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        Button {
            text: "Start Heavy Calculation"
            onClicked: {
                // 发送消息给 Worker
                worker.sendMessage({
                    action: "calculate",
                    data: 1000000
                })
            }
        }
        
        ProgressBar {
            id: progressBar
            width: 300
            from: 0
            to: 100
        }
        
        Text {
            id: resultText
            text: "Result will appear here"
        }
    }
}
```

#### worker.js
```javascript
// ✅ WorkerScript 的 JavaScript 文件

// 接收来自主线程的消息
WorkerScript.onMessage = function(message) {
    console.log("Worker received:", message.action)
    
    if (message.action === "calculate") {
        var result = 0
        var total = message.data
        
        // 模拟耗时计算
        for (var i = 0; i < total; i++) {
            result += Math.sqrt(i)
            
            // 定期报告进度
            if (i % 10000 === 0) {
                WorkerScript.sendMessage({
                    progress: (i / total) * 100,
                    result: "Processing... " + i
                })
            }
        }
        
        // 发送最终结果回主线程
        WorkerScript.sendMessage({
            progress: 100,
            result: "Calculation complete! Result: " + result.toFixed(2)
        })
    }
}
```

### 4.2 WorkerScript 数据处理示例

#### Main.qml
```qml
import QtQuick
import QtQuick.Controls

Window {
    width: 800
    height: 600
    visible: true
    title: "Data Processing with WorkerScript"
    
    WorkerScript {
        id: dataProcessor
        source: "dataprocessor.js"
        
        onMessage: function(msg) {
            if (msg.type === "progress") {
                progressBar.value = msg.value
                statusText.text = msg.message
            } else if (msg.type === "result") {
                resultModel.clear()
                for (var i = 0; i < msg.data.length; i++) {
                    resultModel.append(msg.data[i])
                }
                statusText.text = "Processing complete!"
            }
        }
    }
    
    ListModel {
        id: resultModel
    }
    
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10
        
        Button {
            text: "Process Large Dataset"
            onClicked: {
                // 生成大量数据
                var largeData = []
                for (var i = 0; i < 10000; i++) {
                    largeData.push({
                        id: i,
                        value: Math.random() * 100
                    })
                }
                
                // 发送给 Worker 处理
                dataProcessor.sendMessage({
                    action: "process",
                    data: largeData
                })
            }
        }
        
        ProgressBar {
            id: progressBar
            width: parent.width
            from: 0
            to: 100
        }
        
        Text {
            id: statusText
            text: "Ready"
        }
        
        ListView {
            width: parent.width
            height: 400
            model: resultModel
            
            delegate: Rectangle {
                width: parent.width
                height: 30
                color: index % 2 ? "#f0f0f0" : "white"
                
                Text {
                    anchors.centerIn: parent
                    text: "ID: " + model.id + ", Value: " + model.value.toFixed(2)
                }
            }
        }
    }
}
```

#### dataprocessor.js
```javascript
WorkerScript.onMessage = function(message) {
    if (message.action === "process") {
        var data = message.data
        var processed = []
        var total = data.length
        
        // 处理数据
        for (var i = 0; i < total; i++) {
            // 复杂的数据处理
            var item = data[i]
            var processedValue = Math.sqrt(item.value) * 2 + Math.sin(item.id)
            
            processed.push({
                id: item.id,
                value: processedValue
            })
            
            // 报告进度
            if (i % 100 === 0) {
                WorkerScript.sendMessage({
                    type: "progress",
                    value: (i / total) * 100,
                    message: "Processing item " + i + " of " + total
                })
            }
        }
        
        // 发送结果（只发送前 100 项，避免消息过大）
        WorkerScript.sendMessage({
            type: "result",
            data: processed.slice(0, 100)
        })
    }
}
```


### 4.3 WorkerScript 限制与注意事项

**✅ 可以使用：**
- 基本 JavaScript 语法
- 数学运算
- 字符串处理
- 数组和对象操作
- `console.log()` 调试

**❌ 不能使用：**
- QML 对象和属性（如 `parent`、`anchors`）
- Qt API（如 `Qt.createComponent()`）
- DOM API（如 `document`、`window`）
- 访问 QML 上下文
- 导入 QML 模块

**数据传递限制：**
```javascript
// ✅ 可以传递的数据类型
WorkerScript.sendMessage({
    number: 42,
    string: "hello",
    boolean: true,
    array: [1, 2, 3],
    object: { key: "value" }
})

// ❌ 不能传递的数据类型
WorkerScript.sendMessage({
    qmlObject: someQMLItem,      // ❌ QML 对象
    function: function() {},      // ❌ 函数
    date: new Date()             // ❌ Date 对象（会转为字符串）
})
```

### 4.4 WorkerScript vs C++ 线程

| 特性 | WorkerScript | C++ QThread |
|------|--------------|-------------|
| **性能** | 较慢（JavaScript） | 快（原生代码） |
| **易用性** | 简单，纯 QML | 需要 C++ 知识 |
| **功能** | 受限（JavaScript） | 完整（C++ API） |
| **适用场景** | 轻量计算、数据处理 | 重量级任务、系统调用 |
| **调试** | 容易 | 相对复杂 |

**何时使用 WorkerScript？**
- ✅ 纯 QML 项目，不想引入 C++
- ✅ 简单的数据处理和计算
- ✅ 快速原型开发
- ❌ 需要高性能计算
- ❌ 需要访问系统 API
- ❌ 需要复杂的线程同步

### 4.5 WorkerScript 实战：图片处理

#### Main.qml
```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Window {
    width: 800
    height: 600
    visible: true
    title: "Image Processing"
    
    WorkerScript {
        id: imageProcessor
        source: "imageprocessor.js"
        
        onMessage: function(msg) {
            if (msg.type === "processed") {
                // 创建新的图片
                var canvas = canvasItem.getContext("2d")
                var imageData = canvas.createImageData(msg.width, msg.height)
                
                for (var i = 0; i < msg.pixels.length; i++) {
                    imageData.data[i] = msg.pixels[i]
                }
                
                canvas.putImageData(imageData, 0, 0)
                statusText.text = "Processing complete!"
            }
        }
    }
    
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10
        
        Row {
            spacing: 10
            
            Button {
                text: "Apply Grayscale"
                onClicked: processImage("grayscale")
            }
            
            Button {
                text: "Apply Blur"
                onClicked: processImage("blur")
            }
            
            Button {
                text: "Invert Colors"
                onClicked: processImage("invert")
            }
        }
        
        Text {
            id: statusText
            text: "Ready"
        }
        
        Canvas {
            id: canvasItem
            width: 400
            height: 300
        }
    }
    
    function processImage(filter) {
        statusText.text = "Processing..."
        
        var ctx = canvasItem.getContext("2d")
        var imageData = ctx.getImageData(0, 0, canvasItem.width, canvasItem.height)
        
        imageProcessor.sendMessage({
            action: filter,
            pixels: Array.from(imageData.data),
            width: canvasItem.width,
            height: canvasItem.height
        })
    }
}
```

#### imageprocessor.js
```javascript
WorkerScript.onMessage = function(message) {
    var pixels = message.pixels
    var width = message.width
    var height = message.height
    
    if (message.action === "grayscale") {
        // 灰度处理
        for (var i = 0; i < pixels.length; i += 4) {
            var gray = pixels[i] * 0.299 + pixels[i+1] * 0.587 + pixels[i+2] * 0.114
            pixels[i] = gray
            pixels[i+1] = gray
            pixels[i+2] = gray
        }
    } else if (message.action === "invert") {
        // 反色处理
        for (var i = 0; i < pixels.length; i += 4) {
            pixels[i] = 255 - pixels[i]
            pixels[i+1] = 255 - pixels[i+1]
            pixels[i+2] = 255 - pixels[i+2]
        }
    } else if (message.action === "blur") {
        // 简单模糊（3x3 平均）
        var newPixels = pixels.slice()
        for (var y = 1; y < height - 1; y++) {
            for (var x = 1; x < width - 1; x++) {
                var idx = (y * width + x) * 4
                var sum = [0, 0, 0]
                
                // 3x3 邻域
                for (var dy = -1; dy <= 1; dy++) {
                    for (var dx = -1; dx <= 1; dx++) {
                        var nidx = ((y + dy) * width + (x + dx)) * 4
                        sum[0] += pixels[nidx]
                        sum[1] += pixels[nidx + 1]
                        sum[2] += pixels[nidx + 2]
                    }
                }
                
                newPixels[idx] = sum[0] / 9
                newPixels[idx + 1] = sum[1] / 9
                newPixels[idx + 2] = sum[2] / 9
            }
        }
        pixels = newPixels
    }
    
    WorkerScript.sendMessage({
        type: "processed",
        pixels: pixels,
        width: width,
        height: height
    })
}
```


---

## 5. QThreadPool 线程池

### 5.1 QThreadPool 基本概念

线程池维护一组可重用的线程，避免频繁创建和销毁线程的开销。

```cpp
#include <QThreadPool>
#include <QRunnable>
#include <QDebug>

// ✅ 创建可运行任务（继承 QRunnable）
class Task : public QRunnable {
public:
    Task(int id) : m_id(id) {}
    
    void run() override {
        qDebug() << "Task" << m_id << "running in thread:" 
                 << QThread::currentThread();
        
        // 模拟耗时操作
        QThread::sleep(1);
        
        qDebug() << "Task" << m_id << "completed";
    }
    
private:
    int m_id;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 获取全局线程池
    QThreadPool *pool = QThreadPool::globalInstance();
    
    qDebug() << "Thread pool max threads:" << pool->maxThreadCount();
    qDebug() << "Active threads:" << pool->activeThreadCount();
    
    // 提交多个任务
    for (int i = 0; i < 10; ++i) {
        Task *task = new Task(i);
        pool->start(task);  // 线程池会自动管理任务执行
    }
    
    // 等待所有任务完成
    pool->waitForDone();
    
    qDebug() << "All tasks completed";
    
    return 0;
}
```

### 5.2 QThreadPool 配置

```cpp
#include <QThreadPool>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QThreadPool *pool = QThreadPool::globalInstance();
    
    // 设置最大线程数
    pool->setMaxThreadCount(8);  // 默认为 CPU 核心数
    
    // 设置线程过期时间（毫秒）
    pool->setExpiryTimeout(30000);  // 30 秒后回收空闲线程
    
    // 获取配置信息
    qDebug() << "Max threads:" << pool->maxThreadCount();
    qDebug() << "Ideal thread count:" << QThread::idealThreadCount();  // CPU 核心数
    qDebug() << "Active threads:" << pool->activeThreadCount();
    
    // 创建自定义线程池
    QThreadPool *customPool = new QThreadPool;
    customPool->setMaxThreadCount(4);
    
    return app.exec();
}
```

### 5.3 带返回值的任务（使用信号槽）

```cpp
#include <QThreadPool>
#include <QRunnable>
#include <QObject>

// ✅ 使用 QObject 发送信号
class CalculationTask : public QObject, public QRunnable {
    Q_OBJECT
    
public:
    CalculationTask(int input) : m_input(input) {
        setAutoDelete(true);  // 任务完成后自动删除
    }
    
    void run() override {
        qDebug() << "Calculating for input:" << m_input;
        
        // 模拟复杂计算
        int result = 0;
        for (int i = 0; i < m_input; ++i) {
            result += i;
        }
        
        QThread::sleep(1);
        
        // 发送结果信号
        emit resultReady(m_input, result);
    }
    
signals:
    void resultReady(int input, int result);
    
private:
    int m_input;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QThreadPool *pool = QThreadPool::globalInstance();
    
    for (int i = 1; i <= 5; ++i) {
        CalculationTask *task = new CalculationTask(i * 1000);
        
        // 连接信号（在任务启动前）
        QObject::connect(task, &CalculationTask::resultReady,
                        [](int input, int result) {
            qDebug() << "Result for" << input << "is" << result;
        });
        
        pool->start(task);
    }
    
    pool->waitForDone();
    
    return 0;
}
```


### 5.4 任务优先级

```cpp
#include <QThreadPool>
#include <QRunnable>

class PriorityTask : public QRunnable {
public:
    PriorityTask(int id, int priority) : m_id(id) {
        // 设置任务优先级（默认为 0）
        // 优先级越高，越早执行
        setAutoDelete(true);
    }
    
    void run() override {
        qDebug() << "Task" << m_id << "executing";
        QThread::sleep(1);
    }
    
private:
    int m_id;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QThreadPool *pool = QThreadPool::globalInstance();
    pool->setMaxThreadCount(2);  // 限制并发数，观察优先级效果
    
    // 提交不同优先级的任务
    for (int i = 0; i < 10; ++i) {
        PriorityTask *task = new PriorityTask(i, i);
        pool->start(task, i);  // 第二个参数是优先级
    }
    
    pool->waitForDone();
    
    return 0;
}
```

### 5.5 取消任务

```cpp
#include <QThreadPool>
#include <QRunnable>
#include <QAtomicInt>

class CancellableTask : public QRunnable {
public:
    CancellableTask(QAtomicInt *cancelFlag) 
        : m_cancelFlag(cancelFlag) {
        setAutoDelete(true);
    }
    
    void run() override {
        for (int i = 0; i < 100; ++i) {
            // 检查取消标志
            if (m_cancelFlag->load()) {
                qDebug() << "Task cancelled at step" << i;
                return;
            }
            
            // 执行工作
            QThread::msleep(50);
            qDebug() << "Step" << i;
        }
        
        qDebug() << "Task completed";
    }
    
private:
    QAtomicInt *m_cancelFlag;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QAtomicInt cancelFlag(0);
    
    QThreadPool *pool = QThreadPool::globalInstance();
    
    // 启动任务
    CancellableTask *task = new CancellableTask(&cancelFlag);
    pool->start(task);
    
    // 2 秒后取消任务
    QTimer::singleShot(2000, [&cancelFlag]() {
        qDebug() << "Cancelling task...";
        cancelFlag.store(1);
    });
    
    pool->waitForDone();
    
    return 0;
}
```

### 5.6 QThreadPool 实战：批量图片处理

```cpp
#include <QThreadPool>
#include <QRunnable>
#include <QImage>
#include <QDir>
#include <QDebug>

class ImageProcessTask : public QObject, public QRunnable {
    Q_OBJECT
    
public:
    ImageProcessTask(const QString &inputPath, const QString &outputPath)
        : m_inputPath(inputPath), m_outputPath(outputPath) {
        setAutoDelete(true);
    }
    
    void run() override {
        QImage image(m_inputPath);
        
        if (image.isNull()) {
            emit error(m_inputPath, "Failed to load image");
            return;
        }
        
        // 图片处理：转为灰度
        QImage processed = image.convertToFormat(QImage::Format_Grayscale8);
        
        // 缩放
        processed = processed.scaled(800, 600, Qt::KeepAspectRatio, 
                                     Qt::SmoothTransformation);
        
        // 保存
        if (processed.save(m_outputPath)) {
            emit finished(m_inputPath, m_outputPath);
        } else {
            emit error(m_inputPath, "Failed to save image");
        }
    }
    
signals:
    void finished(const QString &input, const QString &output);
    void error(const QString &input, const QString &message);
    
private:
    QString m_inputPath;
    QString m_outputPath;
};

class ImageBatchProcessor : public QObject {
    Q_OBJECT
    
public:
    void processDirectory(const QString &inputDir, const QString &outputDir) {
        QDir inDir(inputDir);
        QDir outDir(outputDir);
        
        if (!outDir.exists()) {
            outDir.mkpath(".");
        }
        
        // 获取所有图片文件
        QStringList filters;
        filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp";
        QStringList files = inDir.entryList(filters, QDir::Files);
        
        m_totalFiles = files.size();
        m_processedFiles = 0;
        
        qDebug() << "Processing" << m_totalFiles << "images...";
        
        QThreadPool *pool = QThreadPool::globalInstance();
        
        for (const QString &file : files) {
            QString inputPath = inDir.filePath(file);
            QString outputPath = outDir.filePath("processed_" + file);
            
            ImageProcessTask *task = new ImageProcessTask(inputPath, outputPath);
            
            connect(task, &ImageProcessTask::finished,
                    this, &ImageBatchProcessor::onTaskFinished);
            connect(task, &ImageProcessTask::error,
                    this, &ImageBatchProcessor::onTaskError);
            
            pool->start(task);
        }
    }
    
private slots:
    void onTaskFinished(const QString &input, const QString &output) {
        m_processedFiles++;
        qDebug() << "Processed:" << input << "→" << output
                 << "(" << m_processedFiles << "/" << m_totalFiles << ")";
        
        if (m_processedFiles == m_totalFiles) {
            qDebug() << "All images processed!";
            emit allFinished();
        }
    }
    
    void onTaskError(const QString &input, const QString &message) {
        qWarning() << "Error processing" << input << ":" << message;
        m_processedFiles++;
    }
    
signals:
    void allFinished();
    
private:
    int m_totalFiles = 0;
    int m_processedFiles = 0;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    ImageBatchProcessor processor;
    
    QObject::connect(&processor, &ImageBatchProcessor::allFinished, []() {
        qDebug() << "Batch processing complete!";
        QCoreApplication::quit();
    });
    
    processor.processDirectory("./input_images", "./output_images");
    
    return app.exec();
}

#include "main.moc"  // 包含 moc 生成的代码
```


---

## 6. QtConcurrent 高级并发

### 6.1 QtConcurrent::run() - 简单异步执行

```cpp
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QDebug>

// 简单函数
int heavyCalculation(int input) {
    qDebug() << "Calculating in thread:" << QThread::currentThread();
    QThread::sleep(2);
    return input * input;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // ✅ 在线程池中异步执行函数
    QFuture<int> future = QtConcurrent::run(heavyCalculation, 42);
    
    // 方式 1：阻塞等待结果
    int result = future.result();  // 阻塞直到完成
    qDebug() << "Result:" << result;
    
    // 方式 2：使用 QFutureWatcher 非阻塞监听
    QFuture<int> future2 = QtConcurrent::run(heavyCalculation, 100);
    QFutureWatcher<int> *watcher = new QFutureWatcher<int>;
    
    QObject::connect(watcher, &QFutureWatcher<int>::finished, [watcher]() {
        qDebug() << "Async result:" << watcher->result();
        watcher->deleteLater();
        QCoreApplication::quit();
    });
    
    watcher->setFuture(future2);
    
    return app.exec();
}
```

### 6.2 QtConcurrent::mapped() - 并行映射

```cpp
#include <QtConcurrent>
#include <QVector>
#include <QDebug>

// 映射函数：对每个元素进行处理
int square(int x) {
    qDebug() << "Processing" << x << "in thread:" << QThread::currentThread();
    return x * x;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 输入数据
    QVector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // ✅ 并行映射：对每个元素应用 square 函数
    QFuture<int> future = QtConcurrent::mapped(input, square);
    
    // 等待完成并获取结果
    QVector<int> results = future.results();
    
    qDebug() << "Input:" << input;
    qDebug() << "Results:" << results;
    
    return 0;
}
```

### 6.3 QtConcurrent::mappedReduced() - 映射并归约

```cpp
#include <QtConcurrent>
#include <QVector>
#include <QDebug>

// 映射函数
int square(int x) {
    return x * x;
}

// 归约函数：将结果累加
void sum(int &result, const int &intermediate) {
    result += intermediate;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QVector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // ✅ 并行映射并归约：计算平方和
    QFuture<int> future = QtConcurrent::mappedReduced(
        input,
        square,      // 映射函数
        sum,         // 归约函数
        QtConcurrent::SequentialReduce  // 归约选项
    );
    
    int result = future.result();
    
    qDebug() << "Sum of squares:" << result;  // 1+4+9+16+...+100 = 385
    
    return 0;
}
```

### 6.4 QtConcurrent::filtered() - 并行过滤

```cpp
#include <QtConcurrent>
#include <QVector>
#include <QDebug>

// 过滤函数：保留偶数
bool isEven(int x) {
    return x % 2 == 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QVector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // ✅ 并行过滤
    QFuture<int> future = QtConcurrent::filtered(input, isEven);
    
    QVector<int> results = future.results();
    
    qDebug() << "Input:" << input;
    qDebug() << "Even numbers:" << results;  // [2, 4, 6, 8, 10]
    
    return 0;
}
```

### 6.5 QtConcurrent::filteredReduced() - 过滤并归约

```cpp
#include <QtConcurrent>
#include <QVector>
#include <QString>
#include <QDebug>

struct Person {
    QString name;
    int age;
};

// 过滤：年龄 >= 18
bool isAdult(const Person &p) {
    return p.age >= 18;
}

// 归约：收集名字
void collectNames(QStringList &result, const Person &p) {
    result.append(p.name);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QVector<Person> people = {
        {"Alice", 25},
        {"Bob", 17},
        {"Charlie", 30},
        {"David", 16},
        {"Eve", 22}
    };
    
    // ✅ 过滤成年人并收集名字
    QFuture<QStringList> future = QtConcurrent::filteredReduced(
        people,
        isAdult,
        collectNames
    );
    
    QStringList adults = future.result();
    
    qDebug() << "Adults:" << adults;  // ["Alice", "Charlie", "Eve"]
    
    return 0;
}
```


### 7.4 线程安全的信号发送

```cpp
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QDebug>

class DataProducer : public QObject {
    Q_OBJECT
    
public:
    void produceData() {
        // 在任意线程中调用都是安全的
        for (int i = 0; i < 10; ++i) {
            QThread::msleep(100);
            
            // ✅ 信号发送是线程安全的
            emit dataReady(i);
        }
    }
    
signals:
    void dataReady(int value);
};

class DataConsumer : public QObject {
    Q_OBJECT
    
public slots:
    void onDataReady(int value) {
        // ✅ 槽函数在接收者线程中执行
        QMutexLocker locker(&m_mutex);
        m_data.append(value);
        qDebug() << "Consumed:" << value << "in thread:" << QThread::currentThread();
    }
    
private:
    QMutex m_mutex;
    QVector<int> m_data;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    DataProducer producer;
    DataConsumer consumer;
    
    // 连接信号槽（跨线程安全）
    QObject::connect(&producer, &DataProducer::dataReady,
                     &consumer, &DataConsumer::onDataReady);
    
    // 在工作线程中生产数据
    QThread *thread = QThread::create([&producer]() {
        producer.produceData();
    });
    
    thread->start();
    thread->wait();
    
    delete thread;
    
    return 0;
}
```

---

## 8. 线程同步机制

### 8.1 QMutex - 互斥锁

```cpp
#include <QMutex>
#include <QThread>
#include <QDebug>

class Counter {
public:
    void increment() {
        // ✅ 手动加锁
        m_mutex.lock();
        m_value++;
        qDebug() << "Counter:" << m_value;
        m_mutex.unlock();
    }
    
    void incrementSafe() {
        // ✅ 使用 QMutexLocker（RAII，自动解锁）
        QMutexLocker locker(&m_mutex);
        m_value++;
        qDebug() << "Counter:" << m_value;
        // locker 析构时自动解锁
    }
    
    int value() const {
        QMutexLocker locker(&m_mutex);
        return m_value;
    }
    
private:
    mutable QMutex m_mutex;  // mutable 允许在 const 函数中使用
    int m_value = 0;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    Counter counter;
    
    // 创建多个线程同时访问计数器
    QVector<QThread*> threads;
    for (int i = 0; i < 5; ++i) {
        QThread *thread = QThread::create([&counter]() {
            for (int j = 0; j < 100; ++j) {
                counter.incrementSafe();
            }
        });
        threads.append(thread);
        thread->start();
    }
    
    // 等待所有线程完成
    for (QThread *thread : threads) {
        thread->wait();
        delete thread;
    }
    
    qDebug() << "Final value:" << counter.value();  // 应该是 500
    
    return 0;
}
```

### 8.2 QReadWriteLock - 读写锁

```cpp
#include <QReadWriteLock>
#include <QThread>
#include <QVector>
#include <QDebug>

class SharedData {
public:
    // 读操作（多个线程可以同时读）
    int read(int index) const {
        QReadLocker locker(&m_lock);
        if (index >= 0 && index < m_data.size()) {
            return m_data[index];
        }
        return -1;
    }
    
    // 写操作（独占访问）
    void write(int index, int value) {
        QWriteLocker locker(&m_lock);
        if (index >= 0 && index < m_data.size()) {
            m_data[index] = value;
        }
    }
    
    void append(int value) {
        QWriteLocker locker(&m_lock);
        m_data.append(value);
    }
    
    int size() const {
        QReadLocker locker(&m_lock);
        return m_data.size();
    }
    
private:
    mutable QReadWriteLock m_lock;
    QVector<int> m_data;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    SharedData data;
    
    // 初始化数据
    for (int i = 0; i < 100; ++i) {
        data.append(i);
    }
    
    // 创建多个读线程
    QVector<QThread*> readers;
    for (int i = 0; i < 5; ++i) {
        QThread *thread = QThread::create([&data, i]() {
            for (int j = 0; j < 1000; ++j) {
                int value = data.read(j % data.size());
                if (j % 100 == 0) {
                    qDebug() << "Reader" << i << "read:" << value;
                }
            }
        });
        readers.append(thread);
        thread->start();
    }
    
    // 创建写线程
    QThread *writer = QThread::create([&data]() {
        for (int i = 0; i < 100; ++i) {
            data.write(i, i * 2);
            QThread::msleep(10);
        }
    });
    writer->start();
    
    // 等待完成
    for (QThread *thread : readers) {
        thread->wait();
        delete thread;
    }
    writer->wait();
    delete writer;
    
    return 0;
}
```


### 8.3 QSemaphore - 信号量

```cpp
#include <QSemaphore>
#include <QThread>
#include <QDebug>

// 生产者-消费者模式
const int BufferSize = 10;
QSemaphore freeSpace(BufferSize);  // 可用空间
QSemaphore usedSpace(0);           // 已用空间
QVector<int> buffer(BufferSize);
int writePos = 0;
int readPos = 0;

void producer() {
    for (int i = 0; i < 100; ++i) {
        freeSpace.acquire();  // 等待可用空间
        
        buffer[writePos] = i;
        writePos = (writePos + 1) % BufferSize;
        
        qDebug() << "Produced:" << i;
        
        usedSpace.release();  // 增加已用空间
        
        QThread::msleep(50);
    }
}

void consumer() {
    for (int i = 0; i < 100; ++i) {
        usedSpace.acquire();  // 等待数据
        
        int value = buffer[readPos];
        readPos = (readPos + 1) % BufferSize;
        
        qDebug() << "Consumed:" << value;
        
        freeSpace.release();  // 增加可用空间
        
        QThread::msleep(100);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QThread *producerThread = QThread::create(producer);
    QThread *consumerThread = QThread::create(consumer);
    
    producerThread->start();
    consumerThread->start();
    
    producerThread->wait();
    consumerThread->wait();
    
    delete producerThread;
    delete consumerThread;
    
    return 0;
}
```

### 8.4 QWaitCondition - 条件变量

```cpp
#include <QWaitCondition>
#include <QMutex>
#include <QThread>
#include <QQueue>
#include <QDebug>

class TaskQueue {
public:
    void enqueue(int task) {
        QMutexLocker locker(&m_mutex);
        m_queue.enqueue(task);
        qDebug() << "Enqueued task:" << task;
        
        // ✅ 唤醒一个等待的线程
        m_condition.wakeOne();
    }
    
    int dequeue() {
        QMutexLocker locker(&m_mutex);
        
        // ✅ 等待直到队列非空
        while (m_queue.isEmpty()) {
            m_condition.wait(&m_mutex);
        }
        
        int task = m_queue.dequeue();
        qDebug() << "Dequeued task:" << task;
        return task;
    }
    
    void stop() {
        QMutexLocker locker(&m_mutex);
        m_stopped = true;
        
        // ✅ 唤醒所有等待的线程
        m_condition.wakeAll();
    }
    
    bool isStopped() const {
        QMutexLocker locker(&m_mutex);
        return m_stopped;
    }
    
private:
    mutable QMutex m_mutex;
    QWaitCondition m_condition;
    QQueue<int> m_queue;
    bool m_stopped = false;
};

void producer(TaskQueue *queue) {
    for (int i = 0; i < 20; ++i) {
        queue->enqueue(i);
        QThread::msleep(100);
    }
    queue->stop();
}

void consumer(TaskQueue *queue, int id) {
    while (!queue->isStopped()) {
        int task = queue->dequeue();
        qDebug() << "Consumer" << id << "processing task:" << task;
        QThread::msleep(200);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    TaskQueue queue;
    
    QThread *producerThread = QThread::create([&queue]() { producer(&queue); });
    QThread *consumer1 = QThread::create([&queue]() { consumer(&queue, 1); });
    QThread *consumer2 = QThread::create([&queue]() { consumer(&queue, 2); });
    
    producerThread->start();
    consumer1->start();
    consumer2->start();
    
    producerThread->wait();
    consumer1->wait();
    consumer2->wait();
    
    delete producerThread;
    delete consumer1;
    delete consumer2;
    
    return 0;
}
```

### 8.5 QAtomicInt/QAtomicPointer - 原子操作

```cpp
#include <QAtomicInt>
#include <QAtomicPointer>
#include <QThread>
#include <QDebug>

class AtomicCounter {
public:
    void increment() {
        // ✅ 原子递增（无需锁）
        m_counter.fetchAndAddRelaxed(1);
    }
    
    int value() const {
        return m_counter.loadRelaxed();
    }
    
    // 比较并交换（CAS）
    bool compareAndSwap(int expected, int newValue) {
        return m_counter.testAndSetRelaxed(expected, newValue);
    }
    
private:
    QAtomicInt m_counter{0};
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    AtomicCounter counter;
    
    // 创建多个线程同时递增
    QVector<QThread*> threads;
    for (int i = 0; i < 10; ++i) {
        QThread *thread = QThread::create([&counter]() {
            for (int j = 0; j < 1000; ++j) {
                counter.increment();
            }
        });
        threads.append(thread);
        thread->start();
    }
    
    // 等待完成
    for (QThread *thread : threads) {
        thread->wait();
        delete thread;
    }
    
    qDebug() << "Final value:" << counter.value();  // 应该是 10000
    
    return 0;
}
```

### 8.6 线程同步对比

| 机制 | 用途 | 性能 | 复杂度 |
|------|------|------|--------|
| **QMutex** | 互斥访问共享资源 | 中等 | 简单 |
| **QReadWriteLock** | 读多写少场景 | 读操作快 | 中等 |
| **QSemaphore** | 资源计数、生产者消费者 | 中等 | 中等 |
| **QWaitCondition** | 复杂的等待条件 | 高效 | 复杂 |
| **QAtomicInt** | 简单的原子操作 | 最快 | 简单 |


---

## 9. QML 与 C++ 线程交互

### 9.1 从 C++ 后台线程更新 QML

```cpp
// backend.h
#include <QObject>
#include <QThread>

class Backend : public QObject {
    Q_OBJECT
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    
public:
    explicit Backend(QObject *parent = nullptr);
    
    int progress() const { return m_progress; }
    QString status() const { return m_status; }
    
    Q_INVOKABLE void startWork();
    Q_INVOKABLE void stopWork();
    
signals:
    void progressChanged();
    void statusChanged();
    void workFinished(const QString &result);
    
private:
    void setProgress(int value);
    void setStatus(const QString &status);
    
    int m_progress = 0;
    QString m_status = "Ready";
    QThread *m_workerThread = nullptr;
};

// backend.cpp
#include "backend.h"
#include <QDebug>

class Worker : public QObject {
    Q_OBJECT
    
public slots:
    void doWork() {
        for (int i = 0; i <= 100; i += 10) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                emit finished("Cancelled");
                return;
            }
            
            QThread::sleep(1);
            emit progress(i);
            emit status(QString("Processing... %1%").arg(i));
        }
        
        emit finished("Work completed successfully!");
    }
    
signals:
    void progress(int value);
    void status(const QString &text);
    void finished(const QString &result);
};

Backend::Backend(QObject *parent) : QObject(parent) {}

void Backend::startWork() {
    if (m_workerThread) return;
    
    m_workerThread = new QThread;
    Worker *worker = new Worker;
    
    worker->moveToThread(m_workerThread);
    
    // ✅ 连接信号到主线程（自动使用 QueuedConnection）
    connect(m_workerThread, &QThread::started, worker, &Worker::doWork);
    
    connect(worker, &Worker::progress, this, [this](int value) {
        setProgress(value);
    });
    
    connect(worker, &Worker::status, this, [this](const QString &text) {
        setStatus(text);
    });
    
    connect(worker, &Worker::finished, this, [this](const QString &result) {
        emit workFinished(result);
        m_workerThread->quit();
    });
    
    connect(m_workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished, this, [this]() {
        m_workerThread = nullptr;
    });
    
    m_workerThread->start();
}

void Backend::stopWork() {
    if (m_workerThread) {
        m_workerThread->requestInterruption();
    }
}

void Backend::setProgress(int value) {
    if (m_progress != value) {
        m_progress = value;
        emit progressChanged();
    }
}

void Backend::setStatus(const QString &status) {
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

#include "backend.moc"
```

```qml
// Main.qml
import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: "C++ Thread Demo"
    
    // ✅ C++ 后端对象（在 main.cpp 中注册）
    // Backend {
    //     id: backend
    // }
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        Text {
            text: backend.status
            font.pixelSize: 16
        }
        
        ProgressBar {
            width: 400
            from: 0
            to: 100
            value: backend.progress
        }
        
        Row {
            spacing: 10
            
            Button {
                text: "Start Work"
                onClicked: backend.startWork()
            }
            
            Button {
                text: "Stop Work"
                onClicked: backend.stopWork()
            }
        }
    }
    
    Connections {
        target: backend
        
        function onWorkFinished(result) {
            console.log("Work finished:", result)
        }
    }
}
```

```cpp
// main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "backend.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    
    // ✅ 注册 C++ 对象到 QML
    Backend backend;
    engine.rootContext()->setContextProperty("backend", &backend);
    
    engine.loadFromModule("MyApp", "Main");
    
    return app.exec();
}
```


### 9.2 使用 QML 单例访问 C++ 线程

```cpp
// datamanager.h
#include <QObject>
#include <QQmlEngine>

class DataManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
public:
    static DataManager *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    
    Q_INVOKABLE void loadData(const QString &url);
    Q_INVOKABLE void saveData(const QString &data);
    
signals:
    void dataLoaded(const QString &data);
    void dataSaved(bool success);
    void error(const QString &message);
    
private:
    explicit DataManager(QObject *parent = nullptr);
};

// datamanager.cpp
#include "datamanager.h"
#include <QtConcurrent>
#include <QNetworkAccessManager>
#include <QNetworkReply>

DataManager *DataManager::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine) {
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return new DataManager();
}

DataManager::DataManager(QObject *parent) : QObject(parent) {}

void DataManager::loadData(const QString &url) {
    // ✅ 在线程池中异步加载
    QtConcurrent::run([this, url]() {
        // 模拟网络请求
        QThread::sleep(2);
        
        QString data = "Data from " + url;
        
        // ✅ 使用 QMetaObject::invokeMethod 在主线程发送信号
        QMetaObject::invokeMethod(this, [this, data]() {
            emit dataLoaded(data);
        }, Qt::QueuedConnection);
    });
}

void DataManager::saveData(const QString &data) {
    QtConcurrent::run([this, data]() {
        // 模拟保存操作
        QThread::sleep(1);
        
        bool success = !data.isEmpty();
        
        QMetaObject::invokeMethod(this, [this, success]() {
            emit dataSaved(success);
        }, Qt::QueuedConnection);
    });
}
```

```qml
// Main.qml
import QtQuick
import QtQuick.Controls
import MyApp  // 导入包含 DataManager 的模块

Window {
    width: 640
    height: 480
    visible: true
    title: "Singleton Thread Demo"
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        Button {
            text: "Load Data"
            onClicked: {
                // ✅ 使用单例
                DataManager.loadData("https://example.com/data")
            }
        }
        
        Button {
            text: "Save Data"
            onClicked: {
                DataManager.saveData("Some data to save")
            }
        }
        
        Text {
            id: resultText
            text: "Result will appear here"
        }
    }
    
    Connections {
        target: DataManager
        
        function onDataLoaded(data) {
            console.log("Data loaded:", data)
            resultText.text = data
        }
        
        function onDataSaved(success) {
            console.log("Data saved:", success)
            resultText.text = success ? "Saved successfully" : "Save failed"
        }
        
        function onError(message) {
            console.error("Error:", message)
            resultText.text = "Error: " + message
        }
    }
}
```

### 9.3 QML 调用 C++ 异步方法（使用 QFuture）

```cpp
// asyncapi.h
#include <QObject>
#include <QFuture>
#include <QQmlEngine>

class AsyncAPI : public QObject {
    Q_OBJECT
    QML_ELEMENT
    
public:
    explicit AsyncAPI(QObject *parent = nullptr);
    
    // ✅ 返回 QFuture 的异步方法
    Q_INVOKABLE QFuture<QString> fetchData(const QString &url);
    Q_INVOKABLE QFuture<int> calculate(int a, int b);
    
private:
    QString doFetch(const QString &url);
    int doCalculate(int a, int b);
};

// asyncapi.cpp
#include "asyncapi.h"
#include <QtConcurrent>
#include <QThread>

AsyncAPI::AsyncAPI(QObject *parent) : QObject(parent) {}

QFuture<QString> AsyncAPI::fetchData(const QString &url) {
    // ✅ 在线程池中异步执行
    return QtConcurrent::run([this, url]() {
        return doFetch(url);
    });
}

QString AsyncAPI::doFetch(const QString &url) {
    qDebug() << "Fetching from:" << url;
    QThread::sleep(2);  // 模拟网络延迟
    return "Data from " + url;
}

QFuture<int> AsyncAPI::calculate(int a, int b) {
    return QtConcurrent::run([this, a, b]() {
        return doCalculate(a, b);
    });
}

int AsyncAPI::doCalculate(int a, int b) {
    qDebug() << "Calculating:" << a << "+" << b;
    QThread::sleep(1);
    return a + b;
}
```

```qml
// Main.qml
import QtQuick
import QtQuick.Controls
import MyApp

Window {
    width: 640
    height: 480
    visible: true
    title: "Async API Demo"
    
    AsyncAPI {
        id: api
    }
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        Button {
            text: "Fetch Data"
            onClicked: {
                // ✅ 调用异步方法
                var future = api.fetchData("https://example.com")
                
                // 监听完成（需要轮询或使用 QFutureWatcher）
                // 注意：QML 中直接使用 QFuture 有限制
                // 推荐使用信号槽方式
            }
        }
        
        Text {
            id: resultText
            text: "Click button to fetch data"
        }
    }
}
```

**注意**：QML 中直接使用 QFuture 有限制，推荐使用信号槽方式（如 9.1 示例）。


---

## 10. 最佳实践与常见陷阱

### 10.1 最佳实践

#### ✅ 1. 使用 moveToThread() 而不是继承 QThread
```cpp
// ✅ 推荐
Worker *worker = new Worker;
worker->moveToThread(thread);

// ❌ 不推荐
class WorkerThread : public QThread {
    void run() override { /* ... */ }
};
```

#### ✅ 2. 始终使用 QMutexLocker（RAII）
```cpp
// ✅ 推荐：自动解锁
void safeMethod() {
    QMutexLocker locker(&m_mutex);
    // 操作共享数据
    // locker 析构时自动解锁
}

// ❌ 不推荐：手动解锁容易出错
void unsafeMethod() {
    m_mutex.lock();
    // 如果这里抛出异常，锁永远不会释放！
    m_mutex.unlock();
}
```

#### ✅ 3. 正确清理线程
```cpp
// ✅ 推荐：自动清理
QObject::connect(thread, &QThread::finished, worker, &QObject::deleteLater);
QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);

// ❌ 不推荐：手动删除可能导致崩溃
delete worker;  // 如果线程还在运行，会崩溃
delete thread;
```

#### ✅ 4. 避免在析构函数中等待线程
```cpp
class MyClass {
public:
    ~MyClass() {
        // ❌ 不推荐：可能导致死锁
        if (m_thread) {
            m_thread->quit();
            m_thread->wait();  // 危险！
        }
    }
    
    // ✅ 推荐：提供显式的停止方法
    void stop() {
        if (m_thread) {
            m_thread->quit();
            m_thread->wait(5000);  // 超时保护
        }
    }
    
private:
    QThread *m_thread = nullptr;
};
```

#### ✅ 5. 使用信号槽进行跨线程通信
```cpp
// ✅ 推荐：线程安全
emit dataReady(data);

// ❌ 不推荐：直接调用可能不安全
receiver->processData(data);  // 在哪个线程执行？
```

### 10.2 常见陷阱

#### ❌ 陷阱 1：在非主线程更新 UI
```cpp
// ❌ 错误：在工作线程中直接更新 UI
void Worker::run() {
    // 这会导致崩溃或未定义行为！
    label->setText("Updated");
}

// ✅ 正确：通过信号槽更新 UI
void Worker::run() {
    emit textChanged("Updated");
}

// 在主线程中连接
connect(worker, &Worker::textChanged, label, &QLabel::setText);
```

#### ❌ 陷阱 2：忘记启动事件循环
```cpp
class WorkerThread : public QThread {
    void run() override {
        // ❌ 没有事件循环，定时器和信号槽不工作
        QTimer *timer = new QTimer;
        connect(timer, &QTimer::timeout, []() {
            qDebug() << "This will never be called!";
        });
        timer->start(1000);
        
        // ✅ 需要启动事件循环
        exec();  // 或者使用 moveToThread() 方式
    }
};
```

#### ❌ 陷阱 3：数据竞争
```cpp
class Counter {
public:
    // ❌ 不安全：多线程访问
    void increment() {
        m_value++;  // 非原子操作！
    }
    
    // ✅ 安全：使用互斥锁
    void incrementSafe() {
        QMutexLocker locker(&m_mutex);
        m_value++;
    }
    
    // ✅ 安全：使用原子操作
    void incrementAtomic() {
        m_atomicValue.fetchAndAddRelaxed(1);
    }
    
private:
    int m_value = 0;
    QMutex m_mutex;
    QAtomicInt m_atomicValue{0};
};
```

#### ❌ 陷阱 4：死锁
```cpp
// ❌ 死锁示例
QMutex mutex1, mutex2;

void thread1() {
    mutex1.lock();
    QThread::msleep(10);
    mutex2.lock();  // 等待 thread2 释放 mutex2
    // ...
    mutex2.unlock();
    mutex1.unlock();
}

void thread2() {
    mutex2.lock();
    QThread::msleep(10);
    mutex1.lock();  // 等待 thread1 释放 mutex1 → 死锁！
    // ...
    mutex1.unlock();
    mutex2.unlock();
}

// ✅ 解决方案：始终以相同顺序获取锁
void thread1Safe() {
    mutex1.lock();
    mutex2.lock();
    // ...
    mutex2.unlock();
    mutex1.unlock();
}

void thread2Safe() {
    mutex1.lock();  // 与 thread1 相同的顺序
    mutex2.lock();
    // ...
    mutex2.unlock();
    mutex1.unlock();
}
```

#### ❌ 陷阱 5：对象生命周期问题
```cpp
// ❌ 危险：对象可能在线程运行时被删除
void startWork() {
    Worker worker;  // 栈对象
    QThread thread;
    worker.moveToThread(&thread);
    thread.start();
    // worker 在函数结束时被销毁，但线程可能还在运行！
}

// ✅ 正确：使用堆对象并自动清理
void startWorkSafe() {
    Worker *worker = new Worker;
    QThread *thread = new QThread;
    worker->moveToThread(thread);
    
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    thread->start();
}
```


### 10.3 性能优化建议

#### 1. 选择合适的线程数量
```cpp
// ✅ 使用 CPU 核心数
int threadCount = QThread::idealThreadCount();
QThreadPool::globalInstance()->setMaxThreadCount(threadCount);

// ❌ 避免创建过多线程
// 线程过多会导致上下文切换开销
```

#### 2. 避免频繁创建销毁线程
```cpp
// ❌ 不推荐：每次都创建新线程
void processData(const Data &data) {
    QThread *thread = new QThread;
    // ... 处理数据
    delete thread;
}

// ✅ 推荐：使用线程池
void processDataOptimized(const Data &data) {
    QtConcurrent::run([data]() {
        // 处理数据
    });
}
```

#### 3. 减少锁的粒度
```cpp
// ❌ 锁的范围太大
void processLarge() {
    QMutexLocker locker(&m_mutex);
    
    // 大量计算（不需要保护）
    int result = heavyCalculation();
    
    // 只有这里需要保护
    m_data.append(result);
}

// ✅ 缩小锁的范围
void processOptimized() {
    // 在锁外进行计算
    int result = heavyCalculation();
    
    // 只在必要时加锁
    {
        QMutexLocker locker(&m_mutex);
        m_data.append(result);
    }
}
```

#### 4. 使用无锁数据结构
```cpp
// ✅ 对于简单的计数器，使用原子操作
QAtomicInt counter{0};
counter.fetchAndAddRelaxed(1);  // 比 QMutex 快得多

// ✅ 对于单生产者单消费者，考虑无锁队列
// Qt 没有内置，但可以使用第三方库如 Boost.Lockfree
```

### 10.4 调试技巧

#### 1. 启用线程调试
```cpp
// 在 main() 中启用
qputenv("QT_LOGGING_RULES", "qt.core.thread=true");

// 或在代码中
QLoggingCategory::setFilterRules("qt.core.thread=true");
```

#### 2. 检测数据竞争
```bash
# 使用 Thread Sanitizer（需要编译器支持）
# CMakeLists.txt
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")

# 运行程序
./myapp
```

#### 3. 打印线程信息
```cpp
void debugThreadInfo() {
    qDebug() << "Current thread:" << QThread::currentThread();
    qDebug() << "Thread ID:" << QThread::currentThreadId();
    qDebug() << "Is main thread:" 
             << (QThread::currentThread() == QCoreApplication::instance()->thread());
}
```

#### 4. 使用 QElapsedTimer 测量性能
```cpp
#include <QElapsedTimer>

void measurePerformance() {
    QElapsedTimer timer;
    timer.start();
    
    // 执行操作
    heavyCalculation();
    
    qDebug() << "Elapsed time:" << timer.elapsed() << "ms";
}
```

### 10.5 线程安全的单例模式

```cpp
// ✅ Qt 6 推荐：使用 Q_GLOBAL_STATIC
class Singleton {
public:
    static Singleton *instance() {
        static Singleton instance;  // C++11 保证线程安全
        return &instance;
    }
    
private:
    Singleton() = default;
    ~Singleton() = default;
    
    // 禁止拷贝
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

// ✅ 或使用 QMutex（Qt 5 兼容）
class SingletonOld {
public:
    static SingletonOld *instance() {
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        
        if (!s_instance) {
            s_instance = new SingletonOld;
        }
        return s_instance;
    }
    
private:
    SingletonOld() = default;
    static SingletonOld *s_instance;
};

SingletonOld *SingletonOld::s_instance = nullptr;
```

### 10.6 线程池任务优先级管理

```cpp
class PriorityTaskManager {
public:
    enum Priority {
        Low = 0,
        Normal = 5,
        High = 10,
        Critical = 20
    };
    
    void submitTask(QRunnable *task, Priority priority) {
        QThreadPool::globalInstance()->start(task, priority);
    }
    
    // 提交高优先级任务
    void submitCriticalTask(QRunnable *task) {
        submitTask(task, Critical);
    }
};
```

### 10.7 优雅地停止线程

```cpp
class StoppableWorker : public QObject {
    Q_OBJECT
    
public:
    void stop() {
        m_stopRequested.store(true);
    }
    
public slots:
    void doWork() {
        while (!m_stopRequested.load()) {
            // 执行工作
            processItem();
            
            // 定期检查停止标志
            if (m_stopRequested.load()) {
                break;
            }
        }
        
        emit finished();
    }
    
signals:
    void finished();
    
private:
    QAtomicInt m_stopRequested{0};
    
    void processItem() {
        // 处理单个项目
        QThread::msleep(100);
    }
};

// 使用示例
void stopWorkerGracefully() {
    worker->stop();  // 设置停止标志
    thread->quit();  // 退出事件循环
    thread->wait(5000);  // 等待最多 5 秒
    
    if (thread->isRunning()) {
        qWarning() << "Thread did not stop gracefully, terminating...";
        thread->terminate();  // 最后手段（危险！）
    }
}
```


---

## 11. 实战案例

### 11.1 案例 1：多线程文件搜索

```cpp
// filesearcher.h
#include <QObject>
#include <QThread>
#include <QDir>
#include <QAtomicInt>

class FileSearcher : public QObject {
    Q_OBJECT
    
public:
    explicit FileSearcher(QObject *parent = nullptr);
    
    Q_INVOKABLE void search(const QString &directory, 
                           const QString &pattern,
                           int maxThreads = 4);
    Q_INVOKABLE void cancel();
    
signals:
    void fileFound(const QString &path);
    void progress(int filesScanned, int filesFound);
    void finished(int totalFiles, int matchedFiles);
    
private:
    void searchDirectory(const QString &dir, const QRegularExpression &regex);
    
    QAtomicInt m_cancelled{0};
    QAtomicInt m_filesScanned{0};
    QAtomicInt m_filesFound{0};
};

// filesearcher.cpp
#include "filesearcher.h"
#include <QtConcurrent>
#include <QDirIterator>
#include <QRegularExpression>

FileSearcher::FileSearcher(QObject *parent) : QObject(parent) {}

void FileSearcher::search(const QString &directory, 
                         const QString &pattern,
                         int maxThreads) {
    m_cancelled.store(0);
    m_filesScanned.store(0);
    m_filesFound.store(0);
    
    QRegularExpression regex(pattern, QRegularExpression::CaseInsensitiveOption);
    
    // 获取所有子目录
    QDir dir(directory);
    QStringList subdirs;
    subdirs.append(directory);
    
    QDirIterator it(directory, QDir::Dirs | QDir::NoDotAndDotDot, 
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        subdirs.append(it.next());
    }
    
    qDebug() << "Searching in" << subdirs.size() << "directories...";
    
    // 设置线程池大小
    QThreadPool pool;
    pool.setMaxThreadCount(maxThreads);
    
    // 为每个目录创建搜索任务
    QVector<QFuture<void>> futures;
    for (const QString &subdir : subdirs) {
        QFuture<void> future = QtConcurrent::run(&pool, [this, subdir, regex]() {
            searchDirectory(subdir, regex);
        });
        futures.append(future);
    }
    
    // 等待所有任务完成
    for (const QFuture<void> &future : futures) {
        future.waitForFinished();
    }
    
    emit finished(m_filesScanned.load(), m_filesFound.load());
}

void FileSearcher::searchDirectory(const QString &dir, 
                                  const QRegularExpression &regex) {
    if (m_cancelled.load()) return;
    
    QDirIterator it(dir, QDir::Files, QDirIterator::NoIteratorFlags);
    
    while (it.hasNext()) {
        if (m_cancelled.load()) return;
        
        QString filePath = it.next();
        QString fileName = it.fileName();
        
        m_filesScanned.fetchAndAddRelaxed(1);
        
        // 检查文件名是否匹配
        if (regex.match(fileName).hasMatch()) {
            m_filesFound.fetchAndAddRelaxed(1);
            
            // 在主线程发送信号
            QMetaObject::invokeMethod(this, [this, filePath]() {
                emit fileFound(filePath);
            }, Qt::QueuedConnection);
        }
        
        // 定期报告进度
        if (m_filesScanned.load() % 100 == 0) {
            QMetaObject::invokeMethod(this, [this]() {
                emit progress(m_filesScanned.load(), m_filesFound.load());
            }, Qt::QueuedConnection);
        }
    }
}

void FileSearcher::cancel() {
    m_cancelled.store(1);
}
```

```qml
// Main.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Window {
    width: 800
    height: 600
    visible: true
    title: "File Searcher"
    
    FileSearcher {
        id: searcher
        
        onFileFound: function(path) {
            resultModel.append({filePath: path})
        }
        
        onProgress: function(scanned, found) {
            statusText.text = "Scanned: " + scanned + ", Found: " + found
        }
        
        onFinished: function(total, matched) {
            statusText.text = "Search complete! Scanned: " + total + ", Found: " + matched
            searchButton.enabled = true
        }
    }
    
    ListModel {
        id: resultModel
    }
    
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10
        
        Row {
            spacing: 10
            
            TextField {
                id: directoryField
                width: 400
                placeholderText: "Directory path..."
                text: "C:/Users"
            }
            
            Button {
                text: "Browse"
                onClicked: folderDialog.open()
            }
        }
        
        Row {
            spacing: 10
            
            TextField {
                id: patternField
                width: 400
                placeholderText: "Search pattern (regex)..."
                text: ".*\\.txt$"
            }
            
            Button {
                id: searchButton
                text: "Search"
                onClicked: {
                    resultModel.clear()
                    searchButton.enabled = false
                    searcher.search(directoryField.text, patternField.text, 4)
                }
            }
            
            Button {
                text: "Cancel"
                onClicked: searcher.cancel()
            }
        }
        
        Text {
            id: statusText
            text: "Ready"
        }
        
        ListView {
            width: parent.width
            height: 400
            model: resultModel
            clip: true
            
            delegate: Rectangle {
                width: parent.width
                height: 30
                color: index % 2 ? "#f0f0f0" : "white"
                
                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: model.filePath
                }
            }
        }
    }
    
    FolderDialog {
        id: folderDialog
        onAccepted: {
            directoryField.text = selectedFolder.toString().replace("file:///", "")
        }
    }
}
```


### 11.2 案例 2：实时数据处理管道

```cpp
// dataprocessor.h
#include <QObject>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

// 数据项
struct DataItem {
    int id;
    QByteArray rawData;
    QDateTime timestamp;
};

// 处理阶段 1：数据采集
class DataCollector : public QObject {
    Q_OBJECT
public:
    void start();
    void stop();
    
signals:
    void dataCollected(const DataItem &item);
    
private:
    QAtomicInt m_running{0};
};

// 处理阶段 2：数据处理
class DataProcessor : public QObject {
    Q_OBJECT
public slots:
    void processData(const DataItem &item);
    
signals:
    void dataProcessed(const DataItem &item, const QString &result);
};

// 处理阶段 3：数据存储
class DataStorage : public QObject {
    Q_OBJECT
public slots:
    void storeData(const DataItem &item, const QString &result);
    
signals:
    void dataStored(int id);
};

// 管道管理器
class ProcessingPipeline : public QObject {
    Q_OBJECT
    Q_PROPERTY(int itemsProcessed READ itemsProcessed NOTIFY itemsProcessedChanged)
    
public:
    explicit ProcessingPipeline(QObject *parent = nullptr);
    ~ProcessingPipeline();
    
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    
    int itemsProcessed() const { return m_itemsProcessed; }
    
signals:
    void itemsProcessedChanged();
    void pipelineStatus(const QString &status);
    
private:
    DataCollector *m_collector;
    DataProcessor *m_processor;
    DataStorage *m_storage;
    
    QThread *m_collectorThread;
    QThread *m_processorThread;
    QThread *m_storageThread;
    
    int m_itemsProcessed = 0;
};

// dataprocessor.cpp
#include "dataprocessor.h"
#include <QTimer>

// === DataCollector 实现 ===
void DataCollector::start() {
    m_running.store(1);
    
    // 模拟数据采集
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (!m_running.load()) {
            sender()->deleteLater();
            return;
        }
        
        static int id = 0;
        DataItem item;
        item.id = id++;
        item.rawData = QByteArray::number(qrand());
        item.timestamp = QDateTime::currentDateTime();
        
        emit dataCollected(item);
    });
    
    timer->start(100);  // 每 100ms 采集一次
}

void DataCollector::stop() {
    m_running.store(0);
}

// === DataProcessor 实现 ===
void DataProcessor::processData(const DataItem &item) {
    // 模拟耗时处理
    QThread::msleep(50);
    
    QString result = QString("Processed: %1 bytes at %2")
                        .arg(item.rawData.size())
                        .arg(item.timestamp.toString());
    
    emit dataProcessed(item, result);
}

// === DataStorage 实现 ===
void DataStorage::storeData(const DataItem &item, const QString &result) {
    // 模拟存储操作
    QThread::msleep(20);
    
    qDebug() << "Stored item" << item.id << ":" << result;
    
    emit dataStored(item.id);
}

// === ProcessingPipeline 实现 ===
ProcessingPipeline::ProcessingPipeline(QObject *parent) 
    : QObject(parent) {
    
    // 创建工作对象
    m_collector = new DataCollector;
    m_processor = new DataProcessor;
    m_storage = new DataStorage;
    
    // 创建线程
    m_collectorThread = new QThread;
    m_processorThread = new QThread;
    m_storageThread = new QThread;
    
    // 移动到线程
    m_collector->moveToThread(m_collectorThread);
    m_processor->moveToThread(m_processorThread);
    m_storage->moveToThread(m_storageThread);
    
    // 连接管道
    connect(m_collector, &DataCollector::dataCollected,
            m_processor, &DataProcessor::processData);
    
    connect(m_processor, &DataProcessor::dataProcessed,
            m_storage, &DataStorage::storeData);
    
    connect(m_storage, &DataStorage::dataStored, this, [this](int id) {
        m_itemsProcessed++;
        emit itemsProcessedChanged();
        
        if (m_itemsProcessed % 10 == 0) {
            emit pipelineStatus(QString("Processed %1 items").arg(m_itemsProcessed));
        }
    });
    
    // 启动线程
    m_collectorThread->start();
    m_processorThread->start();
    m_storageThread->start();
}

ProcessingPipeline::~ProcessingPipeline() {
    stop();
    
    m_collectorThread->quit();
    m_processorThread->quit();
    m_storageThread->quit();
    
    m_collectorThread->wait();
    m_processorThread->wait();
    m_storageThread->wait();
    
    delete m_collector;
    delete m_processor;
    delete m_storage;
    
    delete m_collectorThread;
    delete m_processorThread;
    delete m_storageThread;
}

void ProcessingPipeline::start() {
    m_itemsProcessed = 0;
    emit itemsProcessedChanged();
    emit pipelineStatus("Pipeline started");
    
    QMetaObject::invokeMethod(m_collector, &DataCollector::start);
}

void ProcessingPipeline::stop() {
    emit pipelineStatus("Pipeline stopping...");
    QMetaObject::invokeMethod(m_collector, &DataCollector::stop);
}
```

```qml
// Main.qml
import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: "Processing Pipeline Demo"
    
    ProcessingPipeline {
        id: pipeline
        
        onPipelineStatus: function(status) {
            statusText.text = status
        }
    }
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        Text {
            text: "Items Processed: " + pipeline.itemsProcessed
            font.pixelSize: 24
        }
        
        Text {
            id: statusText
            text: "Ready"
            font.pixelSize: 16
        }
        
        Row {
            spacing: 10
            
            Button {
                text: "Start Pipeline"
                onClicked: pipeline.start()
            }
            
            Button {
                text: "Stop Pipeline"
                onClicked: pipeline.stop()
            }
        }
    }
}
```


---

## 12. 快速参考

### 12.1 线程创建对比

| 方法 | 代码示例 | 适用场景 |
|------|---------|---------|
| **std::thread** | `std::thread t(func);` | 简单任务，无 Qt 依赖 |
| **QThread + moveToThread** | `worker->moveToThread(thread);` | 需要事件循环和信号槽 |
| **继承 QThread** | `class T : public QThread` | 不推荐（遗留代码） |
| **QThreadPool** | `pool->start(runnable);` | 大量短期任务 |
| **QtConcurrent::run** | `QtConcurrent::run(func);` | 简单异步执行 |
| **WorkerScript** | `WorkerScript { source: "..." }` | QML 中的轻量计算 |

### 12.2 同步机制对比

| 机制 | 用途 | 性能 | 示例 |
|------|------|------|------|
| **QMutex** | 互斥访问 | 中等 | `QMutexLocker locker(&mutex);` |
| **QReadWriteLock** | 读多写少 | 读快 | `QReadLocker locker(&lock);` |
| **QSemaphore** | 资源计数 | 中等 | `semaphore.acquire();` |
| **QWaitCondition** | 条件等待 | 高效 | `condition.wait(&mutex);` |
| **QAtomicInt** | 原子操作 | 最快 | `atomic.fetchAndAddRelaxed(1);` |

### 12.3 信号槽连接类型

```cpp
// Qt::AutoConnection（默认）
connect(sender, &S::signal, receiver, &R::slot);

// Qt::DirectConnection（同步，立即调用）
connect(sender, &S::signal, receiver, &R::slot, Qt::DirectConnection);

// Qt::QueuedConnection（异步，跨线程安全）
connect(sender, &S::signal, receiver, &R::slot, Qt::QueuedConnection);

// Qt::BlockingQueuedConnection（阻塞等待）
connect(sender, &S::signal, receiver, &R::slot, Qt::BlockingQueuedConnection);
```

### 12.4 常用线程函数

```cpp
// 获取当前线程
QThread *thread = QThread::currentThread();

// 获取线程 ID
Qt::HANDLE id = QThread::currentThreadId();

// 获取 CPU 核心数
int cores = QThread::idealThreadCount();

// 线程休眠
QThread::sleep(1);      // 秒
QThread::msleep(100);   // 毫秒
QThread::usleep(1000);  // 微秒

// 让出 CPU 时间片
QThread::yieldCurrentThread();
```

### 12.5 线程安全检查清单

- [ ] UI 更新只在主线程进行
- [ ] 共享数据使用互斥锁保护
- [ ] 信号槽使用 QueuedConnection 跨线程
- [ ] 避免在析构函数中等待线程
- [ ] 使用 RAII（QMutexLocker）管理锁
- [ ] 检查对象生命周期（使用 deleteLater）
- [ ] 避免死锁（统一加锁顺序）
- [ ] 使用原子操作代替简单的计数器
- [ ] 测试多线程场景（Thread Sanitizer）
- [ ] 文档化线程安全性

---

## 13. 参考资源

### 13.1 官方文档

- [QThread](https://doc.qt.io/qt-6/qthread.html)
- [QtConcurrent](https://doc.qt.io/qt-6/qtconcurrent-index.html)
- [QThreadPool](https://doc.qt.io/qt-6/qthreadpool.html)
- [Thread Support in Qt](https://doc.qt.io/qt-6/threads.html)
- [WorkerScript QML Type](https://doc.qt.io/qt-6/qml-qtqml-workerscript-workerscript.html)

### 13.2 推荐阅读

- **Qt 官方博客**：[Threading Basics](https://www.qt.io/blog/2010/06/17/youre-doing-it-wrong)
- **C++ Concurrency in Action** by Anthony Williams
- **Effective Modern C++** by Scott Meyers（第 7 章：并发 API）

### 13.3 调试工具

- **Helgrind**：检测数据竞争和死锁（Valgrind 工具集）
- **Thread Sanitizer**：Clang/GCC 的线程安全检测
- **Qt Creator Debugger**：可视化线程状态
- **GDB**：`info threads` 查看所有线程

### 13.4 性能分析工具

- **Valgrind Callgrind**：性能分析
- **perf**（Linux）：系统级性能分析
- **Visual Studio Profiler**（Windows）
- **Instruments**（macOS）

---

## 结语

Qt 提供了丰富的多线程编程工具，从底层的 QThread 到高层的 QtConcurrent，从 C++ 到 QML，覆盖了各种应用场景。

**关键要点：**

1. ✅ **选择合适的工具**：根据任务特点选择 QThread、QThreadPool 或 QtConcurrent
2. ✅ **使用 moveToThread()**：而不是继承 QThread
3. ✅ **信号槽跨线程**：利用 Qt 的自动线程安全机制
4. ✅ **保护共享数据**：使用互斥锁或原子操作
5. ✅ **避免常见陷阱**：UI 线程、死锁、对象生命周期
6. ✅ **性能优化**：合理的线程数、减少锁粒度、使用线程池
7. ✅ **充分测试**：使用 Thread Sanitizer 等工具检测问题

**记住：**
> "Premature optimization is the root of all evil." - Donald Knuth

先确保代码正确，再考虑性能优化。多线程增加了复杂性，只在真正需要时使用。

---

*文档版本：1.0*  
*最后更新：2025-11-04*  
*适用于：Qt 6.x*  
*作者：Kiro AI Assistant*
