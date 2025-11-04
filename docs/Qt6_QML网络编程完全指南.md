# Qt6 QML 网络编程完全指南

> 本文档详细介绍 Qt6 中的网络编程，涵盖 C++ 底层网络 API、QML 网络组件、HTTP/HTTPS 请求、WebSocket、TCP/UDP 通信、文件下载上传等核心技术。

## 目录

1. [网络编程概述](#1-网络编程概述)
2. [QNetworkAccessManager 详解](#2-qnetworkaccessmanager-详解)
3. [HTTP/HTTPS 请求](#3-httphttps-请求)
4. [RESTful API 交互](#4-restful-api-交互)
5. [文件下载与上传](#5-文件下载与上传)
6. [WebSocket 通信](#6-websocket-通信)
7. [TCP/UDP 套接字](#7-tcpudp-套接字)
8. [QML 网络组件](#8-qml-网络组件)
9. [网络状态监控](#9-网络状态监控)
10. [SSL/TLS 安全通信](#10-ssltls-安全通信)
11. [网络缓存与代理](#11-网络缓存与代理)
12. [异步网络编程](#12-异步网络编程)

---

## 1. 网络编程概述

### 1.1 Qt 网络模块架构

```
┌─────────────────────────────────────────────────────┐
│                   应用层                              │
│  QML / C++ 应用代码                                  │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│                   高层 API                            │
│  QNetworkAccessManager (HTTP/FTP)                   │
│  QWebSocket (WebSocket)                             │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│                   中层 API                            │
│  QNetworkRequest / QNetworkReply                    │
│  QSslSocket (SSL/TLS)                               │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│                   底层 API                            │
│  QTcpSocket / QUdpSocket                            │
│  QAbstractSocket                                    │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│                   操作系统                            │
│  Socket API / 网络驱动                               │
└─────────────────────────────────────────────────────┘
```

### 1.2 Qt 网络模块组件

```cpp
// Qt 网络模块需要在 CMakeLists.txt 中添加
find_package(Qt6 REQUIRED COMPONENTS Network)
target_link_libraries(myapp PRIVATE Qt6::Network)

// 或在 .pro 文件中
QT += network

// 主要类：
#include <QNetworkAccessManager>    // HTTP/HTTPS 请求管理
#include <QNetworkRequest>          // 网络请求
#include <QNetworkReply>            // 网络响应
#include <QTcpSocket>               // TCP 套接字
#include <QUdpSocket>               // UDP 套接字
#include <QWebSocket>               // WebSocket
#include <QSslSocket>               // SSL/TLS 套接字
#include <QNetworkProxy>            // 代理设置
#include <QNetworkCookieJar>        // Cookie 管理
#include <QNetworkDiskCache>        // 磁盘缓存
```

### 1.3 网络编程基本流程

```cpp
// HTTP 请求的基本流程
QNetworkAccessManager *manager = new QNetworkAccessManager(this);

// 1. 创建请求
QNetworkRequest request(QUrl("https://api.example.com/data"));

// 2. 发送请求
QNetworkReply *reply = manager->get(request);

// 3. 处理响应
connect(reply, &QNetworkReply::finished, [reply]() {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "Success:" << data;
    } else {
        qDebug() << "Error:" << reply->errorString();
    }
    reply->deleteLater();
});
```

---

## 2. QNetworkAccessManager 详解

### 2.1 QNetworkAccessManager 基础

```cpp
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

class NetworkManager : public QObject {
    Q_OBJECT
    
public:
    NetworkManager(QObject *parent = nullptr) 
        : QObject(parent)
        , m_manager(new QNetworkAccessManager(this))
    {
        // ✅ 配置网络管理器
        
        // 设置重定向策略
        m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        // - NoLessSafeRedirectPolicy: 不允许从 HTTPS 重定向到 HTTP
        // - ManualRedirectPolicy: 手动处理重定向
        // - SameOriginRedirectPolicy: 只允许同源重定向
        
        // 设置严格传输安全（HSTS）
        m_manager->setStrictTransportSecurityEnabled(true);
        
        // 连接信号
        connect(m_manager, &QNetworkAccessManager::finished,
                this, &NetworkManager::onRequestFinished);
        
        connect(m_manager, &QNetworkAccessManager::sslErrors,
                this, &NetworkManager::onSslErrors);
    }
    
    // ✅ GET 请求
    void get(const QString &url) {
        QNetworkRequest request(QUrl(url));
        setCommonHeaders(request);
        
        QNetworkReply *reply = m_manager->get(request);
        connectReplySignals(reply);
    }
    
    // ✅ POST 请求
    void post(const QString &url, const QByteArray &data) {
        QNetworkRequest request(QUrl(url));
        setCommonHeaders(request);
        request.setHeader(QNetworkRequest::ContentTypeHeader, 
                         "application/json");
        
        QNetworkReply *reply = m_manager->post(request, data);
        connectReplySignals(reply);
    }
    
    // ✅ PUT 请求
    void put(const QString &url, const QByteArray &data) {
        QNetworkRequest request(QUrl(url));
        setCommonHeaders(request);
        
        QNetworkReply *reply = m_manager->put(request, data);
        connectReplySignals(reply);
    }
    
    // ✅ DELETE 请求
    void deleteResource(const QString &url) {
        QNetworkRequest request(QUrl(url));
        setCommonHeaders(request);
        
        QNetworkReply *reply = m_manager->deleteResource(request);
        connectReplySignals(reply);
    }
    
    // ✅ HEAD 请求（只获取头部）
    void head(const QString &url) {
        QNetworkRequest request(QUrl(url));
        QNetworkReply *reply = m_manager->head(request);
        connectReplySignals(reply);
    }
    
private:
    void setCommonHeaders(QNetworkRequest &request) {
        // 设置通用请求头
        request.setHeader(QNetworkRequest::UserAgentHeader, 
                         "MyApp/1.0");
        request.setRawHeader("Accept", "application/json");
        request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8");
    }
    
    void connectReplySignals(QNetworkReply *reply) {
        // 连接进度信号
        connect(reply, &QNetworkReply::downloadProgress,
                this, &NetworkManager::onDownloadProgress);
        
        connect(reply, &QNetworkReply::uploadProgress,
                this, &NetworkManager::onUploadProgress);
        
        // 错误处理
        connect(reply, &QNetworkReply::errorOccurred,
                this, &NetworkManager::onError);
    }
    
private slots:
    void onRequestFinished(QNetworkReply *reply) {
        qDebug() << "Request finished:" << reply->url();
        
        if (reply->error() == QNetworkReply::NoError) {
            // 读取响应数据
            QByteArray data = reply->readAll();
            
            // 获取响应头
            QVariant statusCode = reply->attribute(
                QNetworkRequest::HttpStatusCodeAttribute);
            qDebug() << "Status code:" << statusCode.toInt();
            
            // 处理数据
            emit dataReceived(data);
        } else {
            qDebug() << "Error:" << reply->errorString();
            emit errorOccurred(reply->errorString());
        }
        
        reply->deleteLater();
    }
    
    void onDownloadProgress(qint64 received, qint64 total) {
        if (total > 0) {
            int percentage = (received * 100) / total;
            qDebug() << "Download progress:" << percentage << "%";
            emit downloadProgress(percentage);
        }
    }
    
    void onUploadProgress(qint64 sent, qint64 total) {
        if (total > 0) {
            int percentage = (sent * 100) / total;
            qDebug() << "Upload progress:" << percentage << "%";
            emit uploadProgress(percentage);
        }
    }
    
    void onError(QNetworkReply::NetworkError error) {
        qDebug() << "Network error:" << error;
    }
    
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors) {
        qDebug() << "SSL errors:";
        for (const QSslError &error : errors) {
            qDebug() << "  -" << error.errorString();
        }
        
        // ⚠️ 仅用于开发/测试，生产环境不要忽略 SSL 错误
        // reply->ignoreSslErrors();
    }
    
signals:
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);
    void downloadProgress(int percentage);
    void uploadProgress(int percentage);
    
private:
    QNetworkAccessManager *m_manager;
};
```

### 2.2 QNetworkRequest 详解

```cpp
#include <QNetworkRequest>

void configureRequest() {
    QNetworkRequest request(QUrl("https://api.example.com/data"));
    
    // === 设置请求头 ===
    
    // ✅ 标准请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, 
                     "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, 1024);
    request.setHeader(QNetworkRequest::UserAgentHeader, 
                     "MyApp/1.0");
    
    // ✅ 自定义请求头
    request.setRawHeader("Authorization", "Bearer token123");
    request.setRawHeader("X-Custom-Header", "custom-value");
    request.setRawHeader("Accept-Encoding", "gzip, deflate");
    
    // === 设置请求属性 ===
    
    // ✅ 缓存控制
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                        QNetworkRequest::PreferCache);
    // - AlwaysNetwork: 总是从网络获取
    // - PreferNetwork: 优先网络
    // - PreferCache: 优先缓存
    // - AlwaysCache: 总是从缓存
    
    // ✅ 重定向策略
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                        QNetworkRequest::NoLessSafeRedirectPolicy);
    
    // ✅ 最大重定向次数
    request.setMaximumRedirectsAllowed(5);
    
    // ✅ HTTP/2 支持
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
    
    // ✅ 超时设置（Qt 5.15+）
    request.setTransferTimeout(30000);  // 30 秒
    
    // === SSL 配置 ===
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    request.setSslConfiguration(sslConfig);
    
    // === 优先级 ===
    request.setPriority(QNetworkRequest::HighPriority);
    // - HighPriority: 高优先级
    // - NormalPriority: 正常优先级（默认）
    // - LowPriority: 低优先级
}
```


### 2.3 QNetworkReply 详解

```cpp
void handleNetworkReply(QNetworkReply *reply) {
    // === 检查错误 ===
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error:" << reply->error();
        qDebug() << "Error string:" << reply->errorString();
        
        // 常见错误类型：
        // - ConnectionRefusedError: 连接被拒绝
        // - RemoteHostClosedError: 远程主机关闭连接
        // - HostNotFoundError: 主机未找到
        // - TimeoutError: 超时
        // - OperationCanceledError: 操作被取消
        // - SslHandshakeFailedError: SSL 握手失败
        // - ContentNotFoundError: 404 Not Found
        // - AuthenticationRequiredError: 需要认证
        // - ContentAccessDenied: 403 Forbidden
        // - ProtocolUnknownError: 未知协议
        
        reply->deleteLater();
        return;
    }
    
    // === 获取响应状态 ===
    QVariant statusCode = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute);
    QVariant statusReason = reply->attribute(
        QNetworkRequest::HttpReasonPhraseAttribute);
    
    qDebug() << "Status:" << statusCode.toInt() << statusReason.toString();
    
    // === 获取响应头 ===
    
    // 标准头部
    QVariant contentType = reply->header(QNetworkRequest::ContentTypeHeader);
    QVariant contentLength = reply->header(QNetworkRequest::ContentLengthHeader);
    QVariant lastModified = reply->header(QNetworkRequest::LastModifiedHeader);
    
    qDebug() << "Content-Type:" << contentType.toString();
    qDebug() << "Content-Length:" << contentLength.toLongLong();
    
    // 自定义头部
    QByteArray customHeader = reply->rawHeader("X-Custom-Header");
    qDebug() << "Custom header:" << customHeader;
    
    // 所有头部
    QList<QByteArray> headerList = reply->rawHeaderList();
    for (const QByteArray &header : headerList) {
        qDebug() << header << ":" << reply->rawHeader(header);
    }
    
    // === 读取响应数据 ===
    
    // 方式 1：一次性读取所有数据
    QByteArray data = reply->readAll();
    
    // 方式 2：分块读取
    while (!reply->atEnd()) {
        QByteArray chunk = reply->read(1024);  // 每次读取 1KB
        processChunk(chunk);
    }
    
    // 方式 3：读取指定大小
    QByteArray buffer = reply->read(4096);
    
    // === 获取重定向 URL ===
    QUrl redirectUrl = reply->attribute(
        QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!redirectUrl.isEmpty()) {
        qDebug() << "Redirected to:" << redirectUrl;
    }
    
    // === 检查是否完成 ===
    if (reply->isFinished()) {
        qDebug() << "Reply finished";
    }
    
    // === 检查是否正在运行 ===
    if (reply->isRunning()) {
        qDebug() << "Reply still running";
    }
    
    // === 中止请求 ===
    // reply->abort();
    
    // === 清理 ===
    reply->deleteLater();
}

void processChunk(const QByteArray &chunk) {
    // 处理数据块
}
```

---

## 3. HTTP/HTTPS 请求

### 3.1 GET 请求详解

```cpp
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

class HttpClient : public QObject {
    Q_OBJECT
    
public:
    HttpClient(QObject *parent = nullptr) 
        : QObject(parent)
        , m_manager(new QNetworkAccessManager(this))
    {}
    
    // ✅ 简单 GET 请求
    void simpleGet(const QString &url) {
        QNetworkRequest request(QUrl(url));
        QNetworkReply *reply = m_manager->get(request);
        
        connect(reply, &QNetworkReply::finished, [reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                qDebug() << "Response:" << data;
            }
            reply->deleteLater();
        });
    }
    
    // ✅ 带查询参数的 GET 请求
    void getWithParams(const QString &baseUrl, const QMap<QString, QString> &params) {
        QUrl url(baseUrl);
        QUrlQuery query;
        
        for (auto it = params.begin(); it != params.end(); ++it) {
            query.addQueryItem(it.key(), it.value());
        }
        
        url.setQuery(query);
        
        qDebug() << "Request URL:" << url.toString();
        // 例如：https://api.example.com/search?q=test&page=1
        
        QNetworkRequest request(url);
        QNetworkReply *reply = m_manager->get(request);
        
        connect(reply, &QNetworkReply::finished, [reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                
                // 解析 JSON 响应
                QJsonDocument doc = QJsonDocument::fromJson(data);
                if (doc.isObject()) {
                    QJsonObject obj = doc.object();
                    qDebug() << "JSON response:" << obj;
                }
            }
            reply->deleteLater();
        });
    }
    
    // ✅ 带认证的 GET 请求
    void getWithAuth(const QString &url, const QString &token) {
        QNetworkRequest request(QUrl(url));
        
        // Bearer Token 认证
        request.setRawHeader("Authorization", 
                            QString("Bearer %1").arg(token).toUtf8());
        
        QNetworkReply *reply = m_manager->get(request);
        
        connect(reply, &QNetworkReply::finished, [reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                qDebug() << "Authenticated response:" << data;
            } else if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
                qDebug() << "Authentication failed";
            }
            reply->deleteLater();
        });
    }
    
    // ✅ Basic 认证
    void getWithBasicAuth(const QString &url, 
                         const QString &username, 
                         const QString &password) {
        QNetworkRequest request(QUrl(url));
        
        // Basic 认证
        QString credentials = QString("%1:%2").arg(username, password);
        QByteArray encodedCredentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", 
                            "Basic " + encodedCredentials);
        
        QNetworkReply *reply = m_manager->get(request);
        
        connect(reply, &QNetworkReply::finished, [reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                qDebug() << "Response:" << data;
            }
            reply->deleteLater();
        });
    }
    
private:
    QNetworkAccessManager *m_manager;
};
```

### 3.2 POST 请求详解

```cpp
class HttpPostClient : public QObject {
    Q_OBJECT
    
public:
    HttpPostClient(QObject *parent = nullptr)
        : QObject(parent)
        , m_manager(new QNetworkAccessManager(this))
    {}
    
    // ✅ POST JSON 数据
    void postJson(const QString &url, const QJsonObject &jsonData) {
        QNetworkRequest request(QUrl(url));
        request.setHeader(QNetworkRequest::ContentTypeHeader, 
                         "application/json");
        
        // 将 JSON 对象转换为字节数组
        QJsonDocument doc(jsonData);
        QByteArray data = doc.toJson();
        
        qDebug() << "Posting JSON:" << data;
        
        QNetworkReply *reply = m_manager->post(request, data);
        
        connect(reply, &QNetworkReply::finished, [reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray response = reply->readAll();
                qDebug() << "Response:" << response;
                
                // 解析响应
                QJsonDocument responseDoc = QJsonDocument::fromJson(response);
                if (responseDoc.isObject()) {
                    QJsonObject responseObj = responseDoc.object();
                    qDebug() << "Response JSON:" << responseObj;
                }
            } else {
                qDebug() << "Error:" << reply->errorString();
            }
            reply->deleteLater();
        });
    }
    
    // ✅ POST 表单数据（application/x-www-form-urlencoded）
    void postForm(const QString &url, const QMap<QString, QString> &formData) {
        QNetworkRequest request(QUrl(url));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/x-www-form-urlencoded");
        
        // 构建表单数据
        QUrlQuery params;
        for (auto it = formData.begin(); it != formData.end(); ++it) {
            params.addQueryItem(it.key(), it.value());
        }
        
        QByteArray data = params.toString(QUrl::FullyEncoded).toUtf8();
        
        qDebug() << "Posting form:" << data;
        
        QNetworkReply *reply = m_manager->post(request, data);
        
        connect(reply, &QNetworkReply::finished, [reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray response = reply->readAll();
                qDebug() << "Response:" << response;
            }
            reply->deleteLater();
        });
    }
    
    // ✅ POST 多部分表单数据（multipart/form-data）
    void postMultipart(const QString &url, 
                      const QMap<QString, QString> &fields,
                      const QString &filePath) {
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        
        // 添加文本字段
        for (auto it = fields.begin(); it != fields.end(); ++it) {
            QHttpPart textPart;
            textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                              QVariant(QString("form-data; name=\"%1\"").arg(it.key())));
            textPart.setBody(it.value().toUtf8());
            multiPart->append(textPart);
        }
        
        // 添加文件
        QFile *file = new QFile(filePath);
        if (file->open(QIODevice::ReadOnly)) {
            QHttpPart filePart;
            filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                              QVariant("application/octet-stream"));
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                              QVariant(QString("form-data; name=\"file\"; filename=\"%1\"")
                                      .arg(QFileInfo(filePath).fileName())));
            filePart.setBodyDevice(file);
            file->setParent(multiPart);  // 文件会随 multiPart 一起删除
            
            multiPart->append(filePart);
        }
        
        QNetworkRequest request(QUrl(url));
        QNetworkReply *reply = m_manager->post(request, multiPart);
        multiPart->setParent(reply);  // multiPart 会随 reply 一起删除
        
        connect(reply, &QNetworkReply::finished, [reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray response = reply->readAll();
                qDebug() << "Upload response:" << response;
            } else {
                qDebug() << "Upload error:" << reply->errorString();
            }
            reply->deleteLater();
        });
        
        // 监控上传进度
        connect(reply, &QNetworkReply::uploadProgress,
                [](qint64 sent, qint64 total) {
            if (total > 0) {
                int percentage = (sent * 100) / total;
                qDebug() << "Upload progress:" << percentage << "%";
            }
        });
    }
    
private:
    QNetworkAccessManager *m_manager;
};
```

### 3.3 PUT 和 DELETE 请求

```cpp
class HttpCrudClient : public QObject {
    Q_OBJECT
    
public:
    HttpCrudClient(QObject *parent = nullptr)
        : QObject(parent)
        , m_manager(new QNetworkAccessManager(this))
    {}
    
    // ✅ PUT 请求（更新资源）
    void updateResource(const QString &url, const QJsonObject &data) {
        QNetworkRequest request(QUrl(url));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");
        
        QJsonDocument doc(data);
        QByteArray jsonData = doc.toJson();
        
        QNetworkReply *reply = m_manager->put(request, jsonData);
        
        connect(reply, &QNetworkReply::finished, [reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                qDebug() << "Resource updated successfully";
                QByteArray response = reply->readAll();
                qDebug() << "Response:" << response;
            } else {
                qDebug() << "Update failed:" << reply->errorString();
            }
            reply->deleteLater();
        });
    }
    
    // ✅ PATCH 请求（部分更新）
    void patchResource(const QString &url, const QJsonObject &data) {
        QNetworkRequest request(QUrl(url));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");
        
        QJsonDocument doc(data);
        QByteArray jsonData = doc.toJson();
        
        // Qt 没有直接的 patch() 方法，使用 sendCustomRequest
        QNetworkReply *reply = m_manager->sendCustomRequest(
            request, "PATCH", jsonData);
        
        connect(reply, &QNetworkReply::finished, [reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                qDebug() << "Resource patched successfully";
            }
            reply->deleteLater();
        });
    }
    
    // ✅ DELETE 请求
    void deleteResource(const QString &url) {
        QNetworkRequest request(QUrl(url));
        
        QNetworkReply *reply = m_manager->deleteResource(request);
        
        connect(reply, &QNetworkReply::finished, [reply]() {
            QVariant statusCode = reply->attribute(
                QNetworkRequest::HttpStatusCodeAttribute);
            
            if (reply->error() == QNetworkReply::NoError) {
                qDebug() << "Resource deleted successfully";
                qDebug() << "Status code:" << statusCode.toInt();
            } else {
                qDebug() << "Delete failed:" << reply->errorString();
            }
            reply->deleteLater();
        });
    }
    
private:
    QNetworkAccessManager *m_manager;
};
```


---

## 4. RESTful API 交互

### 4.1 完整的 REST API 客户端

```cpp
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class RestApiClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString baseUrl READ baseUrl WRITE setBaseUrl NOTIFY baseUrlChanged)
    Q_PROPERTY(QString authToken READ authToken WRITE setAuthToken NOTIFY authTokenChanged)
    
public:
    explicit RestApiClient(QObject *parent = nullptr)
        : QObject(parent)
        , m_manager(new QNetworkAccessManager(this))
        , m_baseUrl("https://api.example.com")
    {}
    
    QString baseUrl() const { return m_baseUrl; }
    void setBaseUrl(const QString &url) {
        if (m_baseUrl != url) {
            m_baseUrl = url;
            emit baseUrlChanged();
        }
    }
    
    QString authToken() const { return m_authToken; }
    void setAuthToken(const QString &token) {
        if (m_authToken != token) {
            m_authToken = token;
            emit authTokenChanged();
        }
    }
    
    // ✅ GET 请求
    Q_INVOKABLE void get(const QString &endpoint) {
        QUrl url(m_baseUrl + endpoint);
        QNetworkRequest request = createRequest(url);
        
        QNetworkReply *reply = m_manager->get(request);
        handleReply(reply, "GET", endpoint);
    }
    
    // ✅ POST 请求
    Q_INVOKABLE void post(const QString &endpoint, const QVariantMap &data) {
        QUrl url(m_baseUrl + endpoint);
        QNetworkRequest request = createRequest(url);
        
        QJsonObject jsonObj = QJsonObject::fromVariantMap(data);
        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson();
        
        QNetworkReply *reply = m_manager->post(request, jsonData);
        handleReply(reply, "POST", endpoint);
    }
    
    // ✅ PUT 请求
    Q_INVOKABLE void put(const QString &endpoint, const QVariantMap &data) {
        QUrl url(m_baseUrl + endpoint);
        QNetworkRequest request = createRequest(url);
        
        QJsonObject jsonObj = QJsonObject::fromVariantMap(data);
        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson();
        
        QNetworkReply *reply = m_manager->put(request, jsonData);
        handleReply(reply, "PUT", endpoint);
    }
    
    // ✅ DELETE 请求
    Q_INVOKABLE void deleteResource(const QString &endpoint) {
        QUrl url(m_baseUrl + endpoint);
        QNetworkRequest request = createRequest(url);
        
        QNetworkReply *reply = m_manager->deleteResource(request);
        handleReply(reply, "DELETE", endpoint);
    }
    
signals:
    void requestStarted(const QString &method, const QString &endpoint);
    void requestFinished(const QString &method, const QString &endpoint, 
                        const QVariantMap &data);
    void requestError(const QString &method, const QString &endpoint, 
                     const QString &error);
    void baseUrlChanged();
    void authTokenChanged();
    
private:
    QNetworkRequest createRequest(const QUrl &url) {
        QNetworkRequest request(url);
        
        // 设置通用头部
        request.setHeader(QNetworkRequest::ContentTypeHeader, 
                         "application/json");
        request.setHeader(QNetworkRequest::UserAgentHeader,
                         "MyApp/1.0");
        request.setRawHeader("Accept", "application/json");
        
        // 添加认证令牌
        if (!m_authToken.isEmpty()) {
            request.setRawHeader("Authorization",
                                QString("Bearer %1").arg(m_authToken).toUtf8());
        }
        
        return request;
    }
    
    void handleReply(QNetworkReply *reply, 
                    const QString &method, 
                    const QString &endpoint) {
        emit requestStarted(method, endpoint);
        
        connect(reply, &QNetworkReply::finished, [this, reply, method, endpoint]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray responseData = reply->readAll();
                
                // 解析 JSON 响应
                QJsonDocument doc = QJsonDocument::fromJson(responseData);
                QVariantMap data;
                
                if (doc.isObject()) {
                    data = doc.object().toVariantMap();
                } else if (doc.isArray()) {
                    data["items"] = doc.array().toVariantList();
                }
                
                emit requestFinished(method, endpoint, data);
            } else {
                QString errorString = reply->errorString();
                
                // 尝试解析错误响应
                QByteArray errorData = reply->readAll();
                if (!errorData.isEmpty()) {
                    QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
                    if (errorDoc.isObject()) {
                        QJsonObject errorObj = errorDoc.object();
                        if (errorObj.contains("message")) {
                            errorString = errorObj["message"].toString();
                        }
                    }
                }
                
                emit requestError(method, endpoint, errorString);
            }
            
            reply->deleteLater();
        });
    }
    
private:
    QNetworkAccessManager *m_manager;
    QString m_baseUrl;
    QString m_authToken;
};
```

### 4.2 在 QML 中使用 REST API

```qml
// RestApiClient 需要在 main.cpp 中注册
// qmlRegisterType<RestApiClient>("MyApp", 1, 0, "RestApiClient");

import QtQuick
import QtQuick.Controls
import MyApp 1.0

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    title: "REST API Demo"
    
    RestApiClient {
        id: api
        baseUrl: "https://jsonplaceholder.typicode.com"
        
        onRequestStarted: function(method, endpoint) {
            console.log("Request started:", method, endpoint)
            statusText.text = "Loading..."
            busyIndicator.running = true
        }
        
        onRequestFinished: function(method, endpoint, data) {
            console.log("Request finished:", method, endpoint)
            console.log("Data:", JSON.stringify(data, null, 2))
            
            statusText.text = "Success"
            busyIndicator.running = false
            
            // 显示数据
            resultText.text = JSON.stringify(data, null, 2)
        }
        
        onRequestError: function(method, endpoint, error) {
            console.error("Request error:", method, endpoint, error)
            statusText.text = "Error: " + error
            busyIndicator.running = false
        }
    }
    
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10
        
        Row {
            spacing: 10
            
            Button {
                text: "GET /posts"
                onClicked: api.get("/posts")
            }
            
            Button {
                text: "GET /posts/1"
                onClicked: api.get("/posts/1")
            }
            
            Button {
                text: "POST /posts"
                onClicked: {
                    api.post("/posts", {
                        title: "New Post",
                        body: "This is a new post",
                        userId: 1
                    })
                }
            }
            
            Button {
                text: "PUT /posts/1"
                onClicked: {
                    api.put("/posts/1", {
                        id: 1,
                        title: "Updated Post",
                        body: "This post has been updated",
                        userId: 1
                    })
                }
            }
            
            Button {
                text: "DELETE /posts/1"
                onClicked: api.deleteResource("/posts/1")
            }
        }
        
        Row {
            spacing: 10
            
            Text {
                id: statusText
                text: "Ready"
            }
            
            BusyIndicator {
                id: busyIndicator
                running: false
                width: 20
                height: 20
            }
        }
        
        ScrollView {
            width: parent.width
            height: parent.height - 100
            
            TextArea {
                id: resultText
                readOnly: true
                wrapMode: TextArea.Wrap
                font.family: "Courier"
            }
        }
    }
}
```

### 4.3 带分页的 API 请求

```cpp
class PaginatedApiClient : public QObject {
    Q_OBJECT
    
public:
    explicit PaginatedApiClient(QObject *parent = nullptr)
        : QObject(parent)
        , m_manager(new QNetworkAccessManager(this))
    {}
    
    // ✅ 获取分页数据
    Q_INVOKABLE void fetchPage(const QString &endpoint, 
                               int page, 
                               int pageSize) {
        QUrl url(m_baseUrl + endpoint);
        QUrlQuery query;
        query.addQueryItem("page", QString::number(page));
        query.addQueryItem("per_page", QString::number(pageSize));
        url.setQuery(query);
        
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                         "application/json");
        
        QNetworkReply *reply = m_manager->get(request);
        
        connect(reply, &QNetworkReply::finished, [this, reply, page]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(data);
                
                if (doc.isArray()) {
                    QJsonArray items = doc.array();
                    
                    // 从响应头获取分页信息
                    int totalCount = reply->rawHeader("X-Total-Count").toInt();
                    int totalPages = reply->rawHeader("X-Total-Pages").toInt();
                    
                    emit pageReceived(page, items.toVariantList(), 
                                    totalCount, totalPages);
                }
            } else {
                emit errorOccurred(reply->errorString());
            }
            reply->deleteLater();
        });
    }
    
    // ✅ 获取所有页面
    Q_INVOKABLE void fetchAllPages(const QString &endpoint, int pageSize) {
        m_allItems.clear();
        m_currentPage = 1;
        m_pageSize = pageSize;
        m_endpoint = endpoint;
        
        fetchNextPage();
    }
    
signals:
    void pageReceived(int page, const QVariantList &items, 
                     int totalCount, int totalPages);
    void allPagesReceived(const QVariantList &allItems);
    void errorOccurred(const QString &error);
    
private slots:
    void fetchNextPage() {
        QUrl url(m_baseUrl + m_endpoint);
        QUrlQuery query;
        query.addQueryItem("page", QString::number(m_currentPage));
        query.addQueryItem("per_page", QString::number(m_pageSize));
        url.setQuery(query);
        
        QNetworkRequest request(url);
        QNetworkReply *reply = m_manager->get(request);
        
        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(data);
                
                if (doc.isArray()) {
                    QJsonArray items = doc.array();
                    
                    // 添加到总列表
                    for (const QJsonValue &item : items) {
                        m_allItems.append(item.toVariant());
                    }
                    
                    // 检查是否还有更多页面
                    int totalPages = reply->rawHeader("X-Total-Pages").toInt();
                    
                    if (m_currentPage < totalPages) {
                        m_currentPage++;
                        fetchNextPage();  // 递归获取下一页
                    } else {
                        // 所有页面已获取
                        emit allPagesReceived(m_allItems);
                    }
                }
            } else {
                emit errorOccurred(reply->errorString());
            }
            reply->deleteLater();
        });
    }
    
private:
    QNetworkAccessManager *m_manager;
    QString m_baseUrl = "https://api.example.com";
    QString m_endpoint;
    int m_currentPage = 1;
    int m_pageSize = 20;
    QVariantList m_allItems;
};
```


---

## 5. 文件下载与上传

### 5.1 文件下载

```cpp
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>

class FileDownloader : public QObject {
    Q_OBJECT
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    
public:
    explicit FileDownloader(QObject *parent = nullptr)
        : QObject(parent)
        , m_manager(new QNetworkAccessManager(this))
        , m_progress(0)
    {}
    
    int progress() const { return m_progress; }
    QString status() const { return m_status; }
    
    // ✅ 下载文件
    Q_INVOKABLE void download(const QString &url, const QString &savePath) {
        m_savePath = savePath;
        setStatus("Starting download...");
        setProgress(0);
        
        QNetworkRequest request(QUrl(url));
        
        // 设置请求头
        request.setRawHeader("User-Agent", "MyApp/1.0");
        
        // 支持断点续传
        QFile file(savePath);
        if (file.exists()) {
            qint64 fileSize = file.size();
            request.setRawHeader("Range", 
                                QString("bytes=%1-").arg(fileSize).toUtf8());
            qDebug() << "Resuming download from byte:" << fileSize;
        }
        
        QNetworkReply *reply = m_manager->get(request);
        
        // 连接信号
        connect(reply, &QNetworkReply::downloadProgress,
                this, &FileDownloader::onDownloadProgress);
        
        connect(reply, &QNetworkReply::finished,
                this, &FileDownloader::onDownloadFinished);
        
        connect(reply, &QNetworkReply::readyRead,
                this, &FileDownloader::onReadyRead);
        
        connect(reply, &QNetworkReply::errorOccurred,
                this, &FileDownloader::onError);
        
        // 保存 reply 指针
        m_currentReply = reply;
        
        // 打开文件准备写入
        m_file = new QFile(savePath, this);
        if (file.exists()) {
            m_file->open(QIODevice::Append);  // 追加模式（断点续传）
        } else {
            m_file->open(QIODevice::WriteOnly);
        }
    }
    
    // ✅ 取消下载
    Q_INVOKABLE void cancel() {
        if (m_currentReply) {
            m_currentReply->abort();
            setStatus("Download cancelled");
        }
    }
    
    // ✅ 暂停下载
    Q_INVOKABLE void pause() {
        if (m_currentReply) {
            m_currentReply->abort();
            setStatus("Download paused");
        }
    }
    
signals:
    void progressChanged();
    void statusChanged();
    void downloadFinished(bool success, const QString &filePath);
    void downloadError(const QString &error);
    
private slots:
    void onDownloadProgress(qint64 received, qint64 total) {
        if (total > 0) {
            int percentage = (received * 100) / total;
            setProgress(percentage);
            
            // 计算下载速度
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            if (m_lastTime > 0) {
                qint64 timeDiff = currentTime - m_lastTime;
                if (timeDiff > 0) {
                    qint64 bytesDiff = received - m_lastReceived;
                    double speed = (bytesDiff * 1000.0) / timeDiff;  // bytes/s
                    
                    QString speedStr;
                    if (speed > 1024 * 1024) {
                        speedStr = QString::number(speed / (1024 * 1024), 'f', 2) + " MB/s";
                    } else if (speed > 1024) {
                        speedStr = QString::number(speed / 1024, 'f', 2) + " KB/s";
                    } else {
                        speedStr = QString::number(speed, 'f', 2) + " B/s";
                    }
                    
                    setStatus(QString("Downloading... %1% (%2)")
                             .arg(percentage).arg(speedStr));
                }
            }
            
            m_lastTime = currentTime;
            m_lastReceived = received;
        }
    }
    
    void onReadyRead() {
        if (m_file && m_currentReply) {
            // 将数据写入文件
            m_file->write(m_currentReply->readAll());
        }
    }
    
    void onDownloadFinished() {
        if (m_file) {
            m_file->close();
            m_file->deleteLater();
            m_file = nullptr;
        }
        
        if (m_currentReply->error() == QNetworkReply::NoError) {
            setStatus("Download completed");
            setProgress(100);
            emit downloadFinished(true, m_savePath);
        } else if (m_currentReply->error() != QNetworkReply::OperationCanceledError) {
            setStatus("Download failed");
            emit downloadFinished(false, QString());
        }
        
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    
    void onError(QNetworkReply::NetworkError error) {
        QString errorString = m_currentReply->errorString();
        qDebug() << "Download error:" << error << errorString;
        
        if (error != QNetworkReply::OperationCanceledError) {
            setStatus("Error: " + errorString);
            emit downloadError(errorString);
        }
    }
    
private:
    void setProgress(int progress) {
        if (m_progress != progress) {
            m_progress = progress;
            emit progressChanged();
        }
    }
    
    void setStatus(const QString &status) {
        if (m_status != status) {
            m_status = status;
            emit statusChanged();
        }
    }
    
private:
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_currentReply = nullptr;
    QFile *m_file = nullptr;
    QString m_savePath;
    int m_progress;
    QString m_status;
    qint64 m_lastTime = 0;
    qint64 m_lastReceived = 0;
};
```

### 5.2 在 QML 中使用文件下载器

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import MyApp 1.0

Window {
    width: 600
    height: 400
    visible: true
    title: "File Downloader"
    
    FileDownloader {
        id: downloader
        
        onDownloadFinished: function(success, filePath) {
            if (success) {
                console.log("Download completed:", filePath)
                resultDialog.text = "File downloaded successfully to:\n" + filePath
                resultDialog.open()
            } else {
                console.log("Download failed")
            }
        }
        
        onDownloadError: function(error) {
            console.error("Download error:", error)
            errorDialog.text = "Download error:\n" + error
            errorDialog.open()
        }
    }
    
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20
        
        TextField {
            id: urlField
            width: parent.width
            placeholderText: "Enter download URL..."
            text: "https://speed.hetzner.de/100MB.bin"
        }
        
        Row {
            spacing: 10
            
            Button {
                text: "Choose Save Location"
                onClicked: saveDialog.open()
            }
            
            Text {
                id: savePathText
                text: "No location selected"
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        
        Row {
            spacing: 10
            
            Button {
                text: "Download"
                enabled: savePathText.text !== "No location selected"
                onClicked: {
                    downloader.download(urlField.text, savePathText.text)
                }
            }
            
            Button {
                text: "Pause"
                onClicked: downloader.pause()
            }
            
            Button {
                text: "Cancel"
                onClicked: downloader.cancel()
            }
        }
        
        Column {
            width: parent.width
            spacing: 10
            
            Text {
                text: "Progress: " + downloader.progress + "%"
                font.pixelSize: 16
            }
            
            ProgressBar {
                width: parent.width
                from: 0
                to: 100
                value: downloader.progress
            }
            
            Text {
                text: "Status: " + downloader.status
                font.pixelSize: 14
                color: "#666"
            }
        }
    }
    
    FileDialog {
        id: saveDialog
        fileMode: FileDialog.SaveFile
        defaultSuffix: "bin"
        onAccepted: {
            var path = selectedFile.toString()
            // 移除 file:// 前缀
            path = path.replace(/^file:\/\/\//, "")
            savePathText.text = path
        }
    }
    
    Dialog {
        id: resultDialog
        title: "Success"
        property alias text: resultText.text
        standardButtons: Dialog.Ok
        
        Text {
            id: resultText
        }
    }
    
    Dialog {
        id: errorDialog
        title: "Error"
        property alias text: errorText.text
        standardButtons: Dialog.Ok
        
        Text {
            id: errorText
            color: "red"
        }
    }
}
```

### 5.3 文件上传

```cpp
class FileUploader : public QObject {
    Q_OBJECT
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    
public:
    explicit FileUploader(QObject *parent = nullptr)
        : QObject(parent)
        , m_manager(new QNetworkAccessManager(this))
        , m_progress(0)
    {}
    
    int progress() const { return m_progress; }
    QString status() const { return m_status; }
    
    // ✅ 上传文件
    Q_INVOKABLE void upload(const QString &url, 
                           const QString &filePath,
                           const QString &fieldName = "file") {
        QFile *file = new QFile(filePath);
        if (!file->open(QIODevice::ReadOnly)) {
            setStatus("Failed to open file");
            emit uploadError("Cannot open file: " + filePath);
            delete file;
            return;
        }
        
        setStatus("Preparing upload...");
        setProgress(0);
        
        // 创建多部分表单
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        
        // 添加文件部分
        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                          QVariant("application/octet-stream"));
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                          QVariant(QString("form-data; name=\"%1\"; filename=\"%2\"")
                                  .arg(fieldName)
                                  .arg(QFileInfo(filePath).fileName())));
        filePart.setBodyDevice(file);
        file->setParent(multiPart);  // 文件会随 multiPart 一起删除
        
        multiPart->append(filePart);
        
        // 创建请求
        QNetworkRequest request(QUrl(url));
        request.setRawHeader("User-Agent", "MyApp/1.0");
        
        // 发送请求
        QNetworkReply *reply = m_manager->post(request, multiPart);
        multiPart->setParent(reply);  // multiPart 会随 reply 一起删除
        
        // 连接信号
        connect(reply, &QNetworkReply::uploadProgress,
                this, &FileUploader::onUploadProgress);
        
        connect(reply, &QNetworkReply::finished,
                this, &FileUploader::onUploadFinished);
        
        connect(reply, &QNetworkReply::errorOccurred,
                this, &FileUploader::onError);
        
        m_currentReply = reply;
    }
    
    // ✅ 取消上传
    Q_INVOKABLE void cancel() {
        if (m_currentReply) {
            m_currentReply->abort();
            setStatus("Upload cancelled");
        }
    }
    
signals:
    void progressChanged();
    void statusChanged();
    void uploadFinished(bool success, const QString &response);
    void uploadError(const QString &error);
    
private slots:
    void onUploadProgress(qint64 sent, qint64 total) {
        if (total > 0) {
            int percentage = (sent * 100) / total;
            setProgress(percentage);
            setStatus(QString("Uploading... %1%").arg(percentage));
        }
    }
    
    void onUploadFinished() {
        if (m_currentReply->error() == QNetworkReply::NoError) {
            QByteArray response = m_currentReply->readAll();
            setStatus("Upload completed");
            setProgress(100);
            emit uploadFinished(true, QString::fromUtf8(response));
        } else if (m_currentReply->error() != QNetworkReply::OperationCanceledError) {
            setStatus("Upload failed");
            emit uploadFinished(false, QString());
        }
        
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    
    void onError(QNetworkReply::NetworkError error) {
        if (error != QNetworkReply::OperationCanceledError) {
            QString errorString = m_currentReply->errorString();
            setStatus("Error: " + errorString);
            emit uploadError(errorString);
        }
    }
    
private:
    void setProgress(int progress) {
        if (m_progress != progress) {
            m_progress = progress;
            emit progressChanged();
        }
    }
    
    void setStatus(const QString &status) {
        if (m_status != status) {
            m_status = status;
            emit statusChanged();
        }
    }
    
private:
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_currentReply = nullptr;
    int m_progress;
    QString m_status;
};
```


---

## 6. WebSocket 通信

### 6.1 WebSocket 客户端

```cpp
#include <QWebSocket>
#include <QUrl>

class WebSocketClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    
public:
    explicit WebSocketClient(QObject *parent = nullptr)
        : QObject(parent)
        , m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    {
        // 连接信号
        connect(m_webSocket, &QWebSocket::connected,
                this, &WebSocketClient::onConnected);
        
        connect(m_webSocket, &QWebSocket::disconnected,
                this, &WebSocketClient::onDisconnected);
        
        connect(m_webSocket, &QWebSocket::textMessageReceived,
                this, &WebSocketClient::onTextMessageReceived);
        
        connect(m_webSocket, &QWebSocket::binaryMessageReceived,
                this, &WebSocketClient::onBinaryMessageReceived);
        
        connect(m_webSocket, &QWebSocket::errorOccurred,
                this, &WebSocketClient::onError);
        
        connect(m_webSocket, &QWebSocket::stateChanged,
                this, &WebSocketClient::onStateChanged);
    }
    
    bool connected() const { return m_connected; }
    QString url() const { return m_url; }
    
    void setUrl(const QString &url) {
        if (m_url != url) {
            m_url = url;
            emit urlChanged();
        }
    }
    
    // ✅ 连接到 WebSocket 服务器
    Q_INVOKABLE void connectToServer(const QString &url) {
        if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
            qDebug() << "Already connected";
            return;
        }
        
        setUrl(url);
        qDebug() << "Connecting to:" << url;
        m_webSocket->open(QUrl(url));
    }
    
    // ✅ 断开连接
    Q_INVOKABLE void disconnectFromServer() {
        if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
            m_webSocket->close();
        }
    }
    
    // ✅ 发送文本消息
    Q_INVOKABLE void sendText(const QString &message) {
        if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
            qDebug() << "Sending text:" << message;
            m_webSocket->sendTextMessage(message);
        } else {
            qDebug() << "Not connected, cannot send message";
        }
    }
    
    // ✅ 发送 JSON 消息
    Q_INVOKABLE void sendJson(const QVariantMap &data) {
        QJsonObject jsonObj = QJsonObject::fromVariantMap(data);
        QJsonDocument doc(jsonObj);
        QString jsonString = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        sendText(jsonString);
    }
    
    // ✅ 发送二进制消息
    Q_INVOKABLE void sendBinary(const QByteArray &data) {
        if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
            m_webSocket->sendBinaryMessage(data);
        }
    }
    
    // ✅ Ping 服务器
    Q_INVOKABLE void ping() {
        if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
            m_webSocket->ping();
        }
    }
    
signals:
    void connectedChanged();
    void urlChanged();
    void connected();
    void disconnected();
    void textMessageReceived(const QString &message);
    void jsonMessageReceived(const QVariantMap &data);
    void binaryMessageReceived(const QByteArray &data);
    void errorOccurred(const QString &error);
    void stateChanged(const QString &state);
    
private slots:
    void onConnected() {
        qDebug() << "WebSocket connected";
        m_connected = true;
        emit connectedChanged();
        emit connected();
    }
    
    void onDisconnected() {
        qDebug() << "WebSocket disconnected";
        m_connected = false;
        emit connectedChanged();
        emit disconnected();
    }
    
    void onTextMessageReceived(const QString &message) {
        qDebug() << "Text message received:" << message;
        emit textMessageReceived(message);
        
        // 尝试解析为 JSON
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (doc.isObject()) {
            QVariantMap data = doc.object().toVariantMap();
            emit jsonMessageReceived(data);
        }
    }
    
    void onBinaryMessageReceived(const QByteArray &data) {
        qDebug() << "Binary message received, size:" << data.size();
        emit binaryMessageReceived(data);
    }
    
    void onError(QAbstractSocket::SocketError error) {
        QString errorString = m_webSocket->errorString();
        qDebug() << "WebSocket error:" << error << errorString;
        emit errorOccurred(errorString);
    }
    
    void onStateChanged(QAbstractSocket::SocketState state) {
        QString stateString;
        switch (state) {
        case QAbstractSocket::UnconnectedState:
            stateString = "Unconnected";
            break;
        case QAbstractSocket::HostLookupState:
            stateString = "Host Lookup";
            break;
        case QAbstractSocket::ConnectingState:
            stateString = "Connecting";
            break;
        case QAbstractSocket::ConnectedState:
            stateString = "Connected";
            break;
        case QAbstractSocket::ClosingState:
            stateString = "Closing";
            break;
        default:
            stateString = "Unknown";
            break;
        }
        
        qDebug() << "WebSocket state changed:" << stateString;
        emit stateChanged(stateString);
    }
    
private:
    QWebSocket *m_webSocket;
    bool m_connected = false;
    QString m_url;
};
```

### 6.2 在 QML 中使用 WebSocket

```qml
import QtQuick
import QtQuick.Controls
import MyApp 1.0

Window {
    width: 800
    height: 600
    visible: true
    title: "WebSocket Client"
    
    WebSocketClient {
        id: wsClient
        
        onConnected: {
            console.log("Connected to server")
            statusText.text = "Connected"
            statusText.color = "green"
            connectButton.text = "Disconnect"
        }
        
        onDisconnected: {
            console.log("Disconnected from server")
            statusText.text = "Disconnected"
            statusText.color = "red"
            connectButton.text = "Connect"
        }
        
        onTextMessageReceived: function(message) {
            console.log("Received:", message)
            messageModel.append({
                type: "received",
                content: message,
                time: Qt.formatTime(new Date(), "hh:mm:ss")
            })
        }
        
        onJsonMessageReceived: function(data) {
            console.log("Received JSON:", JSON.stringify(data))
            // 处理 JSON 数据
        }
        
        onErrorOccurred: function(error) {
            console.error("Error:", error)
            errorDialog.text = error
            errorDialog.open()
        }
        
        onStateChanged: function(state) {
            stateText.text = "State: " + state
        }
    }
    
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10
        
        // 连接控制
        Row {
            spacing: 10
            
            TextField {
                id: urlField
                width: 400
                placeholderText: "WebSocket URL..."
                text: "ws://echo.websocket.org"
            }
            
            Button {
                id: connectButton
                text: "Connect"
                onClicked: {
                    if (wsClient.connected) {
                        wsClient.disconnectFromServer()
                    } else {
                        wsClient.connectToServer(urlField.text)
                    }
                }
            }
            
            Text {
                id: statusText
                text: "Disconnected"
                color: "red"
                anchors.verticalCenter: parent.verticalCenter
            }
            
            Text {
                id: stateText
                text: "State: Unconnected"
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        
        // 消息输入
        Row {
            spacing: 10
            width: parent.width
            
            TextField {
                id: messageField
                width: parent.width - sendButton.width - 20
                placeholderText: "Enter message..."
                onAccepted: sendButton.clicked()
            }
            
            Button {
                id: sendButton
                text: "Send"
                enabled: wsClient.connected
                onClicked: {
                    if (messageField.text.length > 0) {
                        wsClient.sendText(messageField.text)
                        messageModel.append({
                            type: "sent",
                            content: messageField.text,
                            time: Qt.formatTime(new Date(), "hh:mm:ss")
                        })
                        messageField.text = ""
                    }
                }
            }
        }
        
        // 快捷操作
        Row {
            spacing: 10
            
            Button {
                text: "Send JSON"
                enabled: wsClient.connected
                onClicked: {
                    wsClient.sendJson({
                        type: "message",
                        content: "Hello from QML",
                        timestamp: Date.now()
                    })
                }
            }
            
            Button {
                text: "Ping"
                enabled: wsClient.connected
                onClicked: wsClient.ping()
            }
            
            Button {
                text: "Clear"
                onClicked: messageModel.clear()
            }
        }
        
        // 消息列表
        Rectangle {
            width: parent.width
            height: parent.height - 150
            border.color: "#ccc"
            border.width: 1
            
            ListView {
                id: messageListView
                anchors.fill: parent
                anchors.margins: 5
                clip: true
                
                model: ListModel {
                    id: messageModel
                }
                
                delegate: Rectangle {
                    width: messageListView.width
                    height: messageText.height + 10
                    color: model.type === "sent" ? "#e3f2fd" : "#f5f5f5"
                    
                    Row {
                        anchors.fill: parent
                        anchors.margins: 5
                        spacing: 10
                        
                        Text {
                            text: model.time
                            color: "#666"
                            font.pixelSize: 12
                        }
                        
                        Text {
                            text: model.type === "sent" ? "→" : "←"
                            font.bold: true
                            color: model.type === "sent" ? "#2196f3" : "#4caf50"
                        }
                        
                        Text {
                            id: messageText
                            text: model.content
                            wrapMode: Text.WordWrap
                            width: parent.width - 100
                        }
                    }
                }
                
                onCountChanged: {
                    positionViewAtEnd()
                }
            }
        }
    }
    
    Dialog {
        id: errorDialog
        title: "Error"
        property alias text: errorText.text
        standardButtons: Dialog.Ok
        
        Text {
            id: errorText
            color: "red"
        }
    }
}
```

### 6.3 WebSocket 心跳保持

```cpp
class WebSocketClientWithHeartbeat : public WebSocketClient {
    Q_OBJECT
    
public:
    explicit WebSocketClientWithHeartbeat(QObject *parent = nullptr)
        : WebSocketClient(parent)
        , m_heartbeatTimer(new QTimer(this))
    {
        // 设置心跳定时器
        m_heartbeatTimer->setInterval(30000);  // 30 秒
        connect(m_heartbeatTimer, &QTimer::timeout,
                this, &WebSocketClientWithHeartbeat::sendHeartbeat);
        
        // 连接时启动心跳
        connect(this, &WebSocketClient::connected, [this]() {
            m_heartbeatTimer->start();
        });
        
        // 断开时停止心跳
        connect(this, &WebSocketClient::disconnected, [this]() {
            m_heartbeatTimer->stop();
        });
    }
    
    // ✅ 设置心跳间隔
    Q_INVOKABLE void setHeartbeatInterval(int milliseconds) {
        m_heartbeatTimer->setInterval(milliseconds);
    }
    
private slots:
    void sendHeartbeat() {
        qDebug() << "Sending heartbeat";
        
        // 方式 1：发送 ping 帧
        ping();
        
        // 方式 2：发送自定义心跳消息
        sendJson({
            {"type", "heartbeat"},
            {"timestamp", QDateTime::currentMSecsSinceEpoch()}
        });
    }
    
private:
    QTimer *m_heartbeatTimer;
};
```


---

## 7. TCP/UDP 套接字

### 7.1 TCP 客户端

```cpp
#include <QTcpSocket>
#include <QHostAddress>

class TcpClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    
public:
    explicit TcpClient(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QTcpSocket(this))
    {
        // 连接信号
        connect(m_socket, &QTcpSocket::connected,
                this, &TcpClient::onConnected);
        
        connect(m_socket, &QTcpSocket::disconnected,
                this, &TcpClient::onDisconnected);
        
        connect(m_socket, &QTcpSocket::readyRead,
                this, &TcpClient::onReadyRead);
        
        connect(m_socket, &QTcpSocket::errorOccurred,
                this, &TcpClient::onError);
        
        connect(m_socket, &QTcpSocket::stateChanged,
                this, &TcpClient::onStateChanged);
    }
    
    bool connected() const { return m_connected; }
    
    // ✅ 连接到服务器
    Q_INVOKABLE void connectToHost(const QString &host, int port) {
        qDebug() << "Connecting to" << host << ":" << port;
        m_socket->connectToHost(host, port);
    }
    
    // ✅ 断开连接
    Q_INVOKABLE void disconnectFromHost() {
        m_socket->disconnectFromHost();
    }
    
    // ✅ 发送数据
    Q_INVOKABLE void sendData(const QString &data) {
        if (m_socket->state() == QAbstractSocket::ConnectedState) {
            QByteArray bytes = data.toUtf8();
            qint64 written = m_socket->write(bytes);
            m_socket->flush();
            
            qDebug() << "Sent" << written << "bytes";
        } else {
            qDebug() << "Not connected";
        }
    }
    
    // ✅ 发送二进制数据
    Q_INVOKABLE void sendBinary(const QByteArray &data) {
        if (m_socket->state() == QAbstractSocket::ConnectedState) {
            m_socket->write(data);
            m_socket->flush();
        }
    }
    
signals:
    void connectedChanged();
    void connected();
    void disconnected();
    void dataReceived(const QString &data);
    void binaryDataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);
    void stateChanged(const QString &state);
    
private slots:
    void onConnected() {
        qDebug() << "TCP connected";
        m_connected = true;
        emit connectedChanged();
        emit connected();
    }
    
    void onDisconnected() {
        qDebug() << "TCP disconnected";
        m_connected = false;
        emit connectedChanged();
        emit disconnected();
    }
    
    void onReadyRead() {
        // 读取所有可用数据
        QByteArray data = m_socket->readAll();
        qDebug() << "Received" << data.size() << "bytes";
        
        // 发送文本数据信号
        emit dataReceived(QString::fromUtf8(data));
        
        // 发送二进制数据信号
        emit binaryDataReceived(data);
    }
    
    void onError(QAbstractSocket::SocketError error) {
        QString errorString = m_socket->errorString();
        qDebug() << "TCP error:" << error << errorString;
        emit errorOccurred(errorString);
    }
    
    void onStateChanged(QAbstractSocket::SocketState state) {
        QString stateString;
        switch (state) {
        case QAbstractSocket::UnconnectedState:
            stateString = "Unconnected";
            break;
        case QAbstractSocket::HostLookupState:
            stateString = "Host Lookup";
            break;
        case QAbstractSocket::ConnectingState:
            stateString = "Connecting";
            break;
        case QAbstractSocket::ConnectedState:
            stateString = "Connected";
            break;
        case QAbstractSocket::ClosingState:
            stateString = "Closing";
            break;
        default:
            stateString = "Unknown";
            break;
        }
        
        qDebug() << "TCP state:" << stateString;
        emit stateChanged(stateString);
    }
    
private:
    QTcpSocket *m_socket;
    bool m_connected = false;
};
```

### 7.2 TCP 服务器

```cpp
#include <QTcpServer>
#include <QTcpSocket>

class TcpServer : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool listening READ listening NOTIFY listeningChanged)
    Q_PROPERTY(int port READ port NOTIFY portChanged)
    
public:
    explicit TcpServer(QObject *parent = nullptr)
        : QObject(parent)
        , m_server(new QTcpServer(this))
    {
        connect(m_server, &QTcpServer::newConnection,
                this, &TcpServer::onNewConnection);
    }
    
    bool listening() const { return m_server->isListening(); }
    int port() const { return m_server->serverPort(); }
    
    // ✅ 启动服务器
    Q_INVOKABLE bool start(int port) {
        if (m_server->listen(QHostAddress::Any, port)) {
            qDebug() << "Server started on port" << m_server->serverPort();
            emit listeningChanged();
            emit portChanged();
            return true;
        } else {
            qDebug() << "Failed to start server:" << m_server->errorString();
            emit errorOccurred(m_server->errorString());
            return false;
        }
    }
    
    // ✅ 停止服务器
    Q_INVOKABLE void stop() {
        // 断开所有客户端
        for (QTcpSocket *client : m_clients) {
            client->disconnectFromHost();
            client->deleteLater();
        }
        m_clients.clear();
        
        m_server->close();
        qDebug() << "Server stopped";
        emit listeningChanged();
    }
    
    // ✅ 向所有客户端广播消息
    Q_INVOKABLE void broadcast(const QString &message) {
        QByteArray data = message.toUtf8();
        for (QTcpSocket *client : m_clients) {
            if (client->state() == QAbstractSocket::ConnectedState) {
                client->write(data);
                client->flush();
            }
        }
        qDebug() << "Broadcast to" << m_clients.size() << "clients";
    }
    
    // ✅ 获取连接的客户端数量
    Q_INVOKABLE int clientCount() const {
        return m_clients.size();
    }
    
signals:
    void listeningChanged();
    void portChanged();
    void clientConnected(const QString &address);
    void clientDisconnected(const QString &address);
    void dataReceived(const QString &address, const QString &data);
    void errorOccurred(const QString &error);
    
private slots:
    void onNewConnection() {
        while (m_server->hasPendingConnections()) {
            QTcpSocket *client = m_server->nextPendingConnection();
            m_clients.append(client);
            
            QString address = client->peerAddress().toString();
            qDebug() << "New client connected:" << address;
            emit clientConnected(address);
            
            // 连接客户端信号
            connect(client, &QTcpSocket::readyRead, [this, client]() {
                QByteArray data = client->readAll();
                QString address = client->peerAddress().toString();
                emit dataReceived(address, QString::fromUtf8(data));
            });
            
            connect(client, &QTcpSocket::disconnected, [this, client]() {
                QString address = client->peerAddress().toString();
                qDebug() << "Client disconnected:" << address;
                emit clientDisconnected(address);
                
                m_clients.removeOne(client);
                client->deleteLater();
            });
        }
    }
    
private:
    QTcpServer *m_server;
    QList<QTcpSocket*> m_clients;
};
```

### 7.3 UDP 套接字

```cpp
#include <QUdpSocket>

class UdpSocket : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool bound READ bound NOTIFY boundChanged)
    
public:
    explicit UdpSocket(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QUdpSocket(this))
    {
        connect(m_socket, &QUdpSocket::readyRead,
                this, &UdpSocket::onReadyRead);
        
        connect(m_socket, &QUdpSocket::errorOccurred,
                this, &UdpSocket::onError);
    }
    
    bool bound() const { return m_socket->state() == QAbstractSocket::BoundState; }
    
    // ✅ 绑定端口（接收数据）
    Q_INVOKABLE bool bind(int port) {
        if (m_socket->bind(QHostAddress::Any, port)) {
            qDebug() << "UDP socket bound to port" << port;
            emit boundChanged();
            return true;
        } else {
            qDebug() << "Failed to bind:" << m_socket->errorString();
            emit errorOccurred(m_socket->errorString());
            return false;
        }
    }
    
    // ✅ 关闭套接字
    Q_INVOKABLE void close() {
        m_socket->close();
        emit boundChanged();
    }
    
    // ✅ 发送数据报
    Q_INVOKABLE void sendDatagram(const QString &data, 
                                  const QString &host, 
                                  int port) {
        QByteArray datagram = data.toUtf8();
        qint64 sent = m_socket->writeDatagram(datagram, 
                                              QHostAddress(host), 
                                              port);
        
        if (sent == -1) {
            qDebug() << "Failed to send datagram:" << m_socket->errorString();
        } else {
            qDebug() << "Sent" << sent << "bytes to" << host << ":" << port;
        }
    }
    
    // ✅ 加入多播组
    Q_INVOKABLE bool joinMulticastGroup(const QString &groupAddress) {
        QHostAddress group(groupAddress);
        if (m_socket->joinMulticastGroup(group)) {
            qDebug() << "Joined multicast group:" << groupAddress;
            return true;
        } else {
            qDebug() << "Failed to join multicast group:" 
                     << m_socket->errorString();
            return false;
        }
    }
    
    // ✅ 离开多播组
    Q_INVOKABLE bool leaveMulticastGroup(const QString &groupAddress) {
        QHostAddress group(groupAddress);
        if (m_socket->leaveMulticastGroup(group)) {
            qDebug() << "Left multicast group:" << groupAddress;
            return true;
        } else {
            qDebug() << "Failed to leave multicast group:" 
                     << m_socket->errorString();
            return false;
        }
    }
    
signals:
    void boundChanged();
    void datagramReceived(const QString &data, 
                         const QString &sender, 
                         int senderPort);
    void errorOccurred(const QString &error);
    
private slots:
    void onReadyRead() {
        while (m_socket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(m_socket->pendingDatagramSize());
            
            QHostAddress sender;
            quint16 senderPort;
            
            m_socket->readDatagram(datagram.data(), 
                                  datagram.size(),
                                  &sender, 
                                  &senderPort);
            
            qDebug() << "Received datagram from" 
                     << sender.toString() << ":" << senderPort;
            
            emit datagramReceived(QString::fromUtf8(datagram),
                                sender.toString(),
                                senderPort);
        }
    }
    
    void onError(QAbstractSocket::SocketError error) {
        qDebug() << "UDP error:" << error << m_socket->errorString();
        emit errorOccurred(m_socket->errorString());
    }
    
private:
    QUdpSocket *m_socket;
};
```


---

## 8. QML 网络组件

### 8.1 XMLHttpRequest 在 QML 中的使用

```qml
import QtQuick

Item {
    // ✅ 使用 XMLHttpRequest 发送 HTTP 请求
    function makeRequest(url, method, data, callback) {
        var xhr = new XMLHttpRequest()
        
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    console.log("Success:", xhr.responseText)
                    if (callback) {
                        callback(null, xhr.responseText)
                    }
                } else {
                    console.error("Error:", xhr.status, xhr.statusText)
                    if (callback) {
                        callback(xhr.statusText, null)
                    }
                }
            }
        }
        
        xhr.open(method, url, true)  // true = 异步
        
        // 设置请求头
        xhr.setRequestHeader("Content-Type", "application/json")
        xhr.setRequestHeader("Accept", "application/json")
        
        // 发送请求
        if (data) {
            xhr.send(JSON.stringify(data))
        } else {
            xhr.send()
        }
    }
    
    // ✅ GET 请求
    function get(url, callback) {
        makeRequest(url, "GET", null, callback)
    }
    
    // ✅ POST 请求
    function post(url, data, callback) {
        makeRequest(url, "POST", data, callback)
    }
    
    // ✅ PUT 请求
    function put(url, data, callback) {
        makeRequest(url, "PUT", data, callback)
    }
    
    // ✅ DELETE 请求
    function deleteRequest(url, callback) {
        makeRequest(url, "DELETE", null, callback)
    }
    
    Component.onCompleted: {
        // 使用示例
        get("https://jsonplaceholder.typicode.com/posts/1", function(error, response) {
            if (error) {
                console.error("Error:", error)
            } else {
                var data = JSON.parse(response)
                console.log("Post title:", data.title)
            }
        })
        
        // POST 示例
        post("https://jsonplaceholder.typicode.com/posts", {
            title: "New Post",
            body: "This is a new post",
            userId: 1
        }, function(error, response) {
            if (error) {
                console.error("Error:", error)
            } else {
                var data = JSON.parse(response)
                console.log("Created post ID:", data.id)
            }
        })
    }
}
```

### 8.2 创建可复用的 HTTP 客户端组件

```qml
// HttpClient.qml
import QtQuick

QtObject {
    id: root
    
    property string baseUrl: ""
    property var headers: ({})
    property int timeout: 30000
    
    signal requestStarted(string method, string url)
    signal requestFinished(string method, string url, var data)
    signal requestError(string method, string url, string error)
    
    function request(method, endpoint, data, callback) {
        var url = baseUrl + endpoint
        var xhr = new XMLHttpRequest()
        
        // 超时处理
        var timeoutId = setTimeout(function() {
            xhr.abort()
            var errorMsg = "Request timeout"
            console.error(errorMsg)
            requestError(method, url, errorMsg)
            if (callback) callback(errorMsg, null)
        }, timeout)
        
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                clearTimeout(timeoutId)
                
                if (xhr.status >= 200 && xhr.status < 300) {
                    var response = xhr.responseText
                    var responseData = null
                    
                    try {
                        responseData = JSON.parse(response)
                    } catch (e) {
                        responseData = response
                    }
                    
                    console.log("Request succeeded:", method, url)
                    requestFinished(method, url, responseData)
                    if (callback) callback(null, responseData)
                } else {
                    var errorMsg = xhr.status + " " + xhr.statusText
                    console.error("Request failed:", method, url, errorMsg)
                    requestError(method, url, errorMsg)
                    if (callback) callback(errorMsg, null)
                }
            }
        }
        
        console.log("Starting request:", method, url)
        requestStarted(method, url)
        
        xhr.open(method, url, true)
        
        // 设置通用头部
        for (var key in headers) {
            xhr.setRequestHeader(key, headers[key])
        }
        
        // 发送请求
        if (data) {
            xhr.setRequestHeader("Content-Type", "application/json")
            xhr.send(JSON.stringify(data))
        } else {
            xhr.send()
        }
    }
    
    function get(endpoint, callback) {
        request("GET", endpoint, null, callback)
    }
    
    function post(endpoint, data, callback) {
        request("POST", endpoint, data, callback)
    }
    
    function put(endpoint, data, callback) {
        request("PUT", endpoint, data, callback)
    }
    
    function deleteResource(endpoint, callback) {
        request("DELETE", endpoint, null, callback)
    }
    
    function setTimeout(callback, delay) {
        // 使用 Timer 实现 setTimeout
        var timer = Qt.createQmlObject(
            'import QtQuick 2.0; Timer {}',
            root
        )
        timer.interval = delay
        timer.repeat = false
        timer.triggered.connect(function() {
            callback()
            timer.destroy()
        })
        timer.start()
        
        return timer
    }
    
    function clearTimeout(timer) {
        if (timer) {
            timer.stop()
            timer.destroy()
        }
    }
}
```

```qml
// 使用 HttpClient
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    
    HttpClient {
        id: api
        baseUrl: "https://jsonplaceholder.typicode.com"
        headers: {
            "Accept": "application/json",
            "User-Agent": "MyQMLApp/1.0"
        }
        timeout: 10000
        
        onRequestStarted: function(method, url) {
            console.log("Request started:", method, url)
            statusText.text = "Loading..."
        }
        
        onRequestFinished: function(method, url, data) {
            console.log("Request finished:", method, url)
            statusText.text = "Success"
            resultText.text = JSON.stringify(data, null, 2)
        }
        
        onRequestError: function(method, url, error) {
            console.error("Request error:", method, url, error)
            statusText.text = "Error: " + error
        }
    }
    
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10
        
        Row {
            spacing: 10
            
            Button {
                text: "GET /posts"
                onClicked: {
                    api.get("/posts", function(error, data) {
                        if (!error) {
                            console.log("Received", data.length, "posts")
                        }
                    })
                }
            }
            
            Button {
                text: "POST /posts"
                onClicked: {
                    api.post("/posts", {
                        title: "New Post",
                        body: "Content",
                        userId: 1
                    }, function(error, data) {
                        if (!error) {
                            console.log("Created post:", data.id)
                        }
                    })
                }
            }
        }
        
        Text {
            id: statusText
            text: "Ready"
        }
        
        ScrollView {
            width: parent.width
            height: parent.height - 100
            
            TextArea {
                id: resultText
                readOnly: true
                wrapMode: TextArea.Wrap
                font.family: "Courier"
            }
        }
    }
}
```

---

## 9. 网络状态监控

### 9.1 网络配置监控

```cpp
#include <QNetworkConfigurationManager>
#include <QNetworkConfiguration>

class NetworkMonitor : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool online READ online NOTIFY onlineChanged)
    Q_PROPERTY(QString connectionType READ connectionType NOTIFY connectionTypeChanged)
    
public:
    explicit NetworkMonitor(QObject *parent = nullptr)
        : QObject(parent)
        , m_manager(new QNetworkConfigurationManager(this))
    {
        // 监控网络状态变化
        connect(m_manager, &QNetworkConfigurationManager::onlineStateChanged,
                this, &NetworkMonitor::onOnlineStateChanged);
        
        connect(m_manager, &QNetworkConfigurationManager::configurationChanged,
                this, &NetworkMonitor::onConfigurationChanged);
        
        updateStatus();
    }
    
    bool online() const { return m_online; }
    QString connectionType() const { return m_connectionType; }
    
signals:
    void onlineChanged();
    void connectionTypeChanged();
    void networkStatusChanged(bool online, const QString &type);
    
private slots:
    void onOnlineStateChanged(bool online) {
        qDebug() << "Online state changed:" << online;
        updateStatus();
    }
    
    void onConfigurationChanged(const QNetworkConfiguration &config) {
        qDebug() << "Configuration changed:" << config.name();
        updateStatus();
    }
    
private:
    void updateStatus() {
        bool wasOnline = m_online;
        QString oldType = m_connectionType;
        
        m_online = m_manager->isOnline();
        
        // 获取当前活动配置
        QNetworkConfiguration config = m_manager->defaultConfiguration();
        
        if (config.isValid()) {
            switch (config.bearerType()) {
            case QNetworkConfiguration::BearerEthernet:
                m_connectionType = "Ethernet";
                break;
            case QNetworkConfiguration::BearerWLAN:
                m_connectionType = "WiFi";
                break;
            case QNetworkConfiguration::Bearer2G:
            case QNetworkConfiguration::Bearer3G:
            case QNetworkConfiguration::Bearer4G:
            case QNetworkConfiguration::BearerLTE:
                m_connectionType = "Mobile";
                break;
            case QNetworkConfiguration::BearerBluetooth:
                m_connectionType = "Bluetooth";
                break;
            default:
                m_connectionType = "Unknown";
                break;
            }
        } else {
            m_connectionType = "None";
        }
        
        if (wasOnline != m_online) {
            emit onlineChanged();
        }
        
        if (oldType != m_connectionType) {
            emit connectionTypeChanged();
        }
        
        emit networkStatusChanged(m_online, m_connectionType);
    }
    
private:
    QNetworkConfigurationManager *m_manager;
    bool m_online = false;
    QString m_connectionType;
};
```

### 9.2 在 QML 中使用网络监控

```qml
import QtQuick
import QtQuick.Controls
import MyApp 1.0

ApplicationWindow {
    width: 400
    height: 300
    visible: true
    title: "Network Monitor"
    
    NetworkMonitor {
        id: networkMonitor
        
        onNetworkStatusChanged: function(online, type) {
            console.log("Network status:", online ? "Online" : "Offline")
            console.log("Connection type:", type)
            
            if (online) {
                statusText.text = "Online (" + type + ")"
                statusText.color = "green"
                statusIcon.text = "✓"
            } else {
                statusText.text = "Offline"
                statusText.color = "red"
                statusIcon.text = "✗"
            }
        }
    }
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            
            Text {
                id: statusIcon
                font.pixelSize: 48
                color: networkMonitor.online ? "green" : "red"
            }
            
            Column {
                anchors.verticalCenter: parent.verticalCenter
                
                Text {
                    id: statusText
                    font.pixelSize: 24
                    font.bold: true
                }
                
                Text {
                    text: "Connection: " + networkMonitor.connectionType
                    font.pixelSize: 14
                    color: "#666"
                }
            }
        }
        
        Button {
            text: "Test Connection"
            anchors.horizontalCenter: parent.horizontalCenter
            enabled: networkMonitor.online
            onClicked: {
                // 测试网络连接
                testConnection()
            }
        }
    }
    
    function testConnection() {
        var xhr = new XMLHttpRequest()
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    console.log("Connection test: Success")
                } else {
                    console.log("Connection test: Failed")
                }
            }
        }
        xhr.open("GET", "https://www.google.com", true)
        xhr.send()
    }
}
```

---

## 10. 最佳实践与性能优化

### 10.1 网络请求最佳实践

```cpp
class NetworkBestPractices {
public:
    // ✅ 1. 复用 QNetworkAccessManager
    static QNetworkAccessManager* sharedManager() {
        static QNetworkAccessManager *manager = new QNetworkAccessManager();
        return manager;
    }
    
    // ✅ 2. 设置合理的超时
    static void setRequestTimeout(QNetworkRequest &request, int milliseconds) {
        request.setTransferTimeout(milliseconds);
    }
    
    // ✅ 3. 使用连接池
    static void configureConnectionPool(QNetworkAccessManager *manager) {
        // Qt 会自动管理连接池，但可以配置
        // 默认每个主机最多 6 个并发连接
    }
    
    // ✅ 4. 启用 HTTP/2
    static void enableHttp2(QNetworkRequest &request) {
        request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
    }
    
    // ✅ 5. 压缩请求
    static void enableCompression(QNetworkRequest &request) {
        request.setRawHeader("Accept-Encoding", "gzip, deflate");
    }
    
    // ✅ 6. 缓存策略
    static void setCachePolicy(QNetworkRequest &request, bool preferCache) {
        if (preferCache) {
            request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                                QNetworkRequest::PreferCache);
        } else {
            request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                                QNetworkRequest::PreferNetwork);
        }
    }
};
```

### 10.2 错误处理与重试机制

```cpp
class NetworkRequestWithRetry : public QObject {
    Q_OBJECT
    
public:
    explicit NetworkRequestWithRetry(QObject *parent = nullptr)
        : QObject(parent)
        , m_manager(new QNetworkAccessManager(this))
        , m_maxRetries(3)
        , m_retryDelay(1000)
    {}
    
    void get(const QString &url) {
        m_url = url;
        m_retryCount = 0;
        sendRequest();
    }
    
signals:
    void finished(const QByteArray &data);
    void error(const QString &errorString);
    
private:
    void sendRequest() {
        QNetworkRequest request(QUrl(m_url));
        QNetworkReply *reply = m_manager->get(request);
        
        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                emit finished(data);
            } else {
                handleError(reply);
            }
            reply->deleteLater();
        });
    }
    
    void handleError(QNetworkReply *reply) {
        QNetworkReply::NetworkError error = reply->error();
        
        // 判断是否应该重试
        bool shouldRetry = false;
        
        switch (error) {
        case QNetworkReply::TimeoutError:
        case QNetworkReply::TemporaryNetworkFailureError:
        case QNetworkReply::NetworkSessionFailedError:
        case QNetworkReply::ServiceUnavailableError:
            shouldRetry = true;
            break;
        default:
            break;
        }
        
        if (shouldRetry && m_retryCount < m_maxRetries) {
            m_retryCount++;
            qDebug() << "Retrying request, attempt" << m_retryCount;
            
            // 延迟重试
            QTimer::singleShot(m_retryDelay, this, &NetworkRequestWithRetry::sendRequest);
        } else {
            emit error(reply->errorString());
        }
    }
    
private:
    QNetworkAccessManager *m_manager;
    QString m_url;
    int m_maxRetries;
    int m_retryDelay;
    int m_retryCount = 0;
};
```

---

## 11. 快速参考

### 11.1 常用网络类

| 类 | 用途 | 示例 |
|---|------|------|
| `QNetworkAccessManager` | HTTP/HTTPS 请求管理 | `manager->get(request)` |
| `QNetworkRequest` | 网络请求配置 | `request.setUrl(url)` |
| `QNetworkReply` | 网络响应处理 | `reply->readAll()` |
| `QWebSocket` | WebSocket 通信 | `socket->sendTextMessage()` |
| `QTcpSocket` | TCP 客户端 | `socket->connectToHost()` |
| `QTcpServer` | TCP 服务器 | `server->listen()` |
| `QUdpSocket` | UDP 通信 | `socket->writeDatagram()` |

### 11.2 HTTP 方法对比

| 方法 | 用途 | 幂等性 | 安全性 |
|------|------|--------|--------|
| GET | 获取资源 | ✓ | ✓ |
| POST | 创建资源 | ✗ | ✗ |
| PUT | 更新资源 | ✓ | ✗ |
| PATCH | 部分更新 | ✗ | ✗ |
| DELETE | 删除资源 | ✓ | ✗ |
| HEAD | 获取头部 | ✓ | ✓ |

---

## 结语

Qt 的网络编程模块提供了完整的网络通信解决方案，从高层的 HTTP API 到底层的 Socket 编程，满足各种应用场景的需求。

**关键要点：**

1. ✅ **QNetworkAccessManager**：复用实例，避免频繁创建
2. ✅ **异步编程**：使用信号槽处理网络响应，保持 UI 响应
3. ✅ **错误处理**：完善的错误处理和重试机制
4. ✅ **安全通信**：使用 HTTPS 和 SSL/TLS 保护数据
5. ✅ **性能优化**：启用 HTTP/2、压缩、缓存
6. ✅ **网络监控**：监控网络状态变化，提供离线支持

**最佳实践：**
- 复用 QNetworkAccessManager 实例
- 设置合理的超时时间
- 实现重试机制处理临时故障
- 使用 HTTPS 保护敏感数据
- 监控网络状态，提供离线功能
- 在 QML 中使用 C++ 网络类获得更好的性能

---

*文档版本：1.0*  
*最后更新：2025-11-04*  
*适用于：Qt 6.x*  
*作者：Kiro AI Assistant*
