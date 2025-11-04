# Qt6 数据库编程完全指南

> 本文档详细介绍 Qt6 中的数据库编程，涵盖 SQLite、MySQL、PostgreSQL 等主流数据库的连接、操作、事务处理、QML 集成等核心技术。

## 目录

1. [数据库编程概述](#1-数据库编程概述)
2. [Qt SQL 模块架构](#2-qt-sql-模块架构)
3. [SQLite 数据库](#3-sqlite-数据库)
4. [MySQL 数据库](#4-mysql-数据库)
5. [PostgreSQL 数据库](#5-postgresql-数据库)
6. [数据库连接管理](#6-数据库连接管理)
7. [SQL 查询与执行](#7-sql-查询与执行)
8. [预处理语句与参数绑定](#8-预处理语句与参数绑定)
9. [事务处理](#9-事务处理)
10. [QSqlTableModel 与 QSqlQueryModel](#10-qsqltablemodel-与-qsqlquerymodel)
11. [QML 数据库集成](#11-qml-数据库集成)
12. [数据库性能优化](#12-数据库性能优化)

---

## 1. 数据库编程概述

### 1.1 Qt 支持的数据库

Qt SQL 模块支持多种数据库系统：

| 数据库 | 驱动名称 | 特点 | 适用场景 |
|--------|---------|------|---------|
| **SQLite** | QSQLITE | 轻量级、无服务器、嵌入式 | 移动应用、桌面应用、原型开发 |
| **MySQL** | QMYSQL | 开源、高性能、广泛使用 | Web 应用、企业应用 |
| **PostgreSQL** | QPSQL | 功能强大、标准兼容、开源 | 复杂查询、数据分析 |
| **Oracle** | QOCI | 企业级、高可靠性 | 大型企业应用 |
| **SQL Server** | QODBC | 微软数据库 | Windows 企业应用 |
| **DB2** | QDB2 | IBM 数据库 | 大型机应用 |

### 1.2 Qt SQL 模块配置

```cmake
# CMakeLists.txt
find_package(Qt6 REQUIRED COMPONENTS Sql)
target_link_libraries(myapp PRIVATE Qt6::Sql)

# 或在 .pro 文件中
QT += sql
```

```cpp
// 包含必要的头文件
#include <QSqlDatabase>      // 数据库连接
#include <QSqlQuery>         // SQL 查询
#include <QSqlError>         // 错误处理
#include <QSqlRecord>        // 记录
#include <QSqlField>         // 字段
#include <QSqlDriver>        // 驱动
#include <QSqlTableModel>    // 表模型
#include <QSqlQueryModel>    // 查询模型
#include <QSqlRelationalTableModel>  // 关联表模型
```

### 1.3 数据库编程基本流程

```cpp
// 1. 添加数据库连接
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
db.setDatabaseName("mydb.db");

// 2. 打开数据库
if (!db.open()) {
    qDebug() << "Failed to open database:" << db.lastError().text();
    return;
}

// 3. 执行 SQL 查询
QSqlQuery query;
query.exec("CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)");

// 4. 插入数据
query.prepare("INSERT INTO users (name, age) VALUES (?, ?)");
query.addBindValue("Alice");
query.addBindValue(25);
query.exec();

// 5. 查询数据
query.exec("SELECT * FROM users");
while (query.next()) {
    int id = query.value(0).toInt();
    QString name = query.value(1).toString();
    int age = query.value(2).toInt();
    qDebug() << id << name << age;
}

// 6. 关闭数据库
db.close();
```

---

## 2. Qt SQL 模块架构

### 2.1 核心类层次结构

```
QSqlDatabase (数据库连接)
    ↓
QSqlDriver (数据库驱动)
    ↓
QSqlQuery (SQL 查询执行)
    ↓
QSqlResult (查询结果)
    ↓
QSqlRecord (记录)
    ↓
QSqlField (字段)

数据模型层：
QAbstractItemModel
    ↓
QSqlQueryModel (只读查询模型)
    ↓
QSqlTableModel (可编辑表模型)
    ↓
QSqlRelationalTableModel (关联表模型)
```

### 2.2 数据库驱动检查

```cpp
#include <QSqlDatabase>
#include <QDebug>

void checkAvailableDrivers() {
    qDebug() << "Available SQL drivers:";
    QStringList drivers = QSqlDatabase::drivers();
    
    for (const QString &driver : drivers) {
        qDebug() << "  -" << driver;
    }
    
    // 检查特定驱动是否可用
    if (QSqlDatabase::isDriverAvailable("QSQLITE")) {
        qDebug() << "SQLite driver is available";
    }
    
    if (QSqlDatabase::isDriverAvailable("QMYSQL")) {
        qDebug() << "MySQL driver is available";
    }
    
    if (QSqlDatabase::isDriverAvailable("QPSQL")) {
        qDebug() << "PostgreSQL driver is available";
    }
}
```

---

## 3. SQLite 数据库

### 3.1 SQLite 基础操作

```cpp
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class SQLiteDatabase {
public:
    // ✅ 创建并打开 SQLite 数据库
    static bool createDatabase(const QString &dbPath) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(dbPath);
        
        if (!db.open()) {
            qDebug() << "Failed to open database:" << db.lastError().text();
            return false;
        }
        
        qDebug() << "Database opened successfully:" << dbPath;
        return true;
    }
    
    // ✅ 创建表
    static bool createTables() {
        QSqlQuery query;
        
        // 创建用户表
        QString createUsersTable = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL UNIQUE,
                email TEXT NOT NULL,
                age INTEGER,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        )";
        
        if (!query.exec(createUsersTable)) {
            qDebug() << "Failed to create users table:" << query.lastError().text();
            return false;
        }
        
        // 创建文章表
        QString createPostsTable = R"(
            CREATE TABLE IF NOT EXISTS posts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                title TEXT NOT NULL,
                content TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            )
        )";
        
        if (!query.exec(createPostsTable)) {
            qDebug() << "Failed to create posts table:" << query.lastError().text();
            return false;
        }
        
        // 创建索引
        query.exec("CREATE INDEX IF NOT EXISTS idx_posts_user_id ON posts(user_id)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_users_username ON users(username)");
        
        qDebug() << "Tables created successfully";
        return true;
    }
    
    // ✅ 插入数据
    static bool insertUser(const QString &username, const QString &email, int age) {
        QSqlQuery query;
        query.prepare("INSERT INTO users (username, email, age) VALUES (?, ?, ?)");
        query.addBindValue(username);
        query.addBindValue(email);
        query.addBindValue(age);
        
        if (!query.exec()) {
            qDebug() << "Failed to insert user:" << query.lastError().text();
            return false;
        }
        
        qDebug() << "User inserted, ID:" << query.lastInsertId().toInt();
        return true;
    }
    
    // ✅ 查询数据
    static void queryUsers() {
        QSqlQuery query("SELECT id, username, email, age, created_at FROM users");
        
        qDebug() << "Users:";
        while (query.next()) {
            int id = query.value(0).toInt();
            QString username = query.value(1).toString();
            QString email = query.value(2).toString();
            int age = query.value(3).toInt();
            QString createdAt = query.value(4).toString();
            
            qDebug() << QString("  ID: %1, Username: %2, Email: %3, Age: %4, Created: %5")
                        .arg(id).arg(username).arg(email).arg(age).arg(createdAt);
        }
    }
    
    // ✅ 更新数据
    static bool updateUser(int userId, const QString &email, int age) {
        QSqlQuery query;
        query.prepare("UPDATE users SET email = ?, age = ? WHERE id = ?");
        query.addBindValue(email);
        query.addBindValue(age);
        query.addBindValue(userId);
        
        if (!query.exec()) {
            qDebug() << "Failed to update user:" << query.lastError().text();
            return false;
        }
        
        qDebug() << "User updated, rows affected:" << query.numRowsAffected();
        return true;
    }
    
    // ✅ 删除数据
    static bool deleteUser(int userId) {
        QSqlQuery query;
        query.prepare("DELETE FROM users WHERE id = ?");
        query.addBindValue(userId);
        
        if (!query.exec()) {
            qDebug() << "Failed to delete user:" << query.lastError().text();
            return false;
        }
        
        qDebug() << "User deleted, rows affected:" << query.numRowsAffected();
        return true;
    }
    
    // ✅ 关联查询
    static void queryUserPosts(int userId) {
        QSqlQuery query;
        query.prepare(R"(
            SELECT u.username, p.title, p.content, p.created_at
            FROM users u
            INNER JOIN posts p ON u.id = p.user_id
            WHERE u.id = ?
            ORDER BY p.created_at DESC
        )");
        query.addBindValue(userId);
        
        if (!query.exec()) {
            qDebug() << "Failed to query user posts:" << query.lastError().text();
            return;
        }
        
        qDebug() << "Posts by user" << userId << ":";
        while (query.next()) {
            QString username = query.value(0).toString();
            QString title = query.value(1).toString();
            QString content = query.value(2).toString();
            QString createdAt = query.value(3).toString();
            
            qDebug() << QString("  [%1] %2: %3 (%4)")
                        .arg(username).arg(title).arg(content).arg(createdAt);
        }
    }
};

// 使用示例
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // 创建数据库
    if (!SQLiteDatabase::createDatabase("myapp.db")) {
        return -1;
    }
    
    // 创建表
    SQLiteDatabase::createTables();
    
    // 插入数据
    SQLiteDatabase::insertUser("alice", "alice@example.com", 25);
    SQLiteDatabase::insertUser("bob", "bob@example.com", 30);
    
    // 查询数据
    SQLiteDatabase::queryUsers();
    
    // 更新数据
    SQLiteDatabase::updateUser(1, "alice.new@example.com", 26);
    
    // 删除数据
    // SQLiteDatabase::deleteUser(2);
    
    return 0;
}
```

### 3.2 SQLite 特有功能

```cpp
class SQLiteAdvanced {
public:
    // ✅ 启用外键约束（SQLite 默认关闭）
    static void enableForeignKeys() {
        QSqlQuery query;
        query.exec("PRAGMA foreign_keys = ON");
        qDebug() << "Foreign keys enabled";
    }
    
    // ✅ 设置 WAL 模式（Write-Ahead Logging，提高并发性能）
    static void enableWAL() {
        QSqlQuery query;
        query.exec("PRAGMA journal_mode = WAL");
        qDebug() << "WAL mode enabled";
    }
    
    // ✅ 优化数据库
    static void optimizeDatabase() {
        QSqlQuery query;
        
        // 分析数据库，更新统计信息
        query.exec("ANALYZE");
        
        // 清理未使用的空间
        query.exec("VACUUM");
        
        qDebug() << "Database optimized";
    }
    
    // ✅ 获取数据库信息
    static void getDatabaseInfo() {
        QSqlQuery query;
        
        // 获取所有表
        query.exec("SELECT name FROM sqlite_master WHERE type='table'");
        qDebug() << "Tables:";
        while (query.next()) {
            qDebug() << "  -" << query.value(0).toString();
        }
        
        // 获取表结构
        query.exec("PRAGMA table_info(users)");
        qDebug() << "Users table structure:";
        while (query.next()) {
            QString name = query.value(1).toString();
            QString type = query.value(2).toString();
            bool notNull = query.value(3).toBool();
            QString defaultValue = query.value(4).toString();
            bool primaryKey = query.value(5).toBool();
            
            qDebug() << QString("  %1 %2 %3 %4 %5")
                        .arg(name)
                        .arg(type)
                        .arg(notNull ? "NOT NULL" : "")
                        .arg(defaultValue.isEmpty() ? "" : "DEFAULT " + defaultValue)
                        .arg(primaryKey ? "PRIMARY KEY" : "");
        }
    }
    
    // ✅ 全文搜索（FTS5）
    static void createFTSTable() {
        QSqlQuery query;
        
        // 创建 FTS5 虚拟表
        QString createFTS = R"(
            CREATE VIRTUAL TABLE IF NOT EXISTS posts_fts 
            USING fts5(title, content, tokenize='porter')
        )";
        
        if (!query.exec(createFTS)) {
            qDebug() << "Failed to create FTS table:" << query.lastError().text();
            return;
        }
        
        qDebug() << "FTS table created";
    }
    
    static void searchPosts(const QString &keyword) {
        QSqlQuery query;
        query.prepare("SELECT title, content FROM posts_fts WHERE posts_fts MATCH ?");
        query.addBindValue(keyword);
        
        if (!query.exec()) {
            qDebug() << "Search failed:" << query.lastError().text();
            return;
        }
        
        qDebug() << "Search results for:" << keyword;
        while (query.next()) {
            qDebug() << "  -" << query.value(0).toString();
        }
    }
};
```


---

## 4. MySQL 数据库

### 4.1 MySQL 连接配置

```cpp
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class MySQLDatabase {
public:
    // ✅ 连接到 MySQL 数据库
    static bool connectToMySQL(const QString &host,
                              int port,
                              const QString &database,
                              const QString &username,
                              const QString &password) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
        
        db.setHostName(host);
        db.setPort(port);
        db.setDatabaseName(database);
        db.setUserName(username);
        db.setPassword(password);
        
        // 设置连接选项
        db.setConnectOptions("MYSQL_OPT_RECONNECT=1");  // 自动重连
        
        if (!db.open()) {
            qDebug() << "Failed to connect to MySQL:" << db.lastError().text();
            return false;
        }
        
        qDebug() << "Connected to MySQL successfully";
        qDebug() << "  Host:" << host;
        qDebug() << "  Database:" << database;
        qDebug() << "  Port:" << port;
        
        return true;
    }
    
    // ✅ 创建数据库（需要有权限）
    static bool createDatabase(const QString &dbName) {
        QSqlQuery query;
        QString sql = QString("CREATE DATABASE IF NOT EXISTS %1 "
                            "CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci")
                            .arg(dbName);
        
        if (!query.exec(sql)) {
            qDebug() << "Failed to create database:" << query.lastError().text();
            return false;
        }
        
        qDebug() << "Database created:" << dbName;
        return true;
    }
    
    // ✅ 创建表（MySQL 特定语法）
    static bool createTables() {
        QSqlQuery query;
        
        // 创建用户表
        QString createUsersTable = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INT AUTO_INCREMENT PRIMARY KEY,
                username VARCHAR(50) NOT NULL UNIQUE,
                email VARCHAR(100) NOT NULL,
                password_hash VARCHAR(255) NOT NULL,
                age INT,
                status ENUM('active', 'inactive', 'banned') DEFAULT 'active',
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
                INDEX idx_username (username),
                INDEX idx_email (email)
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
        )";
        
        if (!query.exec(createUsersTable)) {
            qDebug() << "Failed to create users table:" << query.lastError().text();
            return false;
        }
        
        // 创建文章表
        QString createPostsTable = R"(
            CREATE TABLE IF NOT EXISTS posts (
                id INT AUTO_INCREMENT PRIMARY KEY,
                user_id INT NOT NULL,
                title VARCHAR(200) NOT NULL,
                content TEXT,
                view_count INT DEFAULT 0,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
                INDEX idx_user_id (user_id),
                INDEX idx_created_at (created_at),
                FULLTEXT INDEX idx_title_content (title, content)
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
        )";
        
        if (!query.exec(createPostsTable)) {
            qDebug() << "Failed to create posts table:" << query.lastError().text();
            return false;
        }
        
        qDebug() << "Tables created successfully";
        return true;
    }
    
    // ✅ 批量插入（使用事务提高性能）
    static bool batchInsertUsers(const QList<QVariantMap> &users) {
        QSqlDatabase db = QSqlDatabase::database();
        
        if (!db.transaction()) {
            qDebug() << "Failed to start transaction";
            return false;
        }
        
        QSqlQuery query;
        query.prepare("INSERT INTO users (username, email, password_hash, age) "
                     "VALUES (?, ?, ?, ?)");
        
        for (const QVariantMap &user : users) {
            query.addBindValue(user["username"]);
            query.addBindValue(user["email"]);
            query.addBindValue(user["password_hash"]);
            query.addBindValue(user["age"]);
            
            if (!query.exec()) {
                qDebug() << "Failed to insert user:" << query.lastError().text();
                db.rollback();
                return false;
            }
        }
        
        if (!db.commit()) {
            qDebug() << "Failed to commit transaction";
            return false;
        }
        
        qDebug() << "Batch insert completed:" << users.size() << "users";
        return true;
    }
    
    // ✅ 全文搜索（MySQL FULLTEXT）
    static void fullTextSearch(const QString &keyword) {
        QSqlQuery query;
        query.prepare(R"(
            SELECT id, title, content, 
                   MATCH(title, content) AGAINST(? IN NATURAL LANGUAGE MODE) AS relevance
            FROM posts
            WHERE MATCH(title, content) AGAINST(? IN NATURAL LANGUAGE MODE)
            ORDER BY relevance DESC
            LIMIT 10
        )");
        query.addBindValue(keyword);
        query.addBindValue(keyword);
        
        if (!query.exec()) {
            qDebug() << "Search failed:" << query.lastError().text();
            return;
        }
        
        qDebug() << "Search results for:" << keyword;
        while (query.next()) {
            int id = query.value(0).toInt();
            QString title = query.value(1).toString();
            double relevance = query.value(3).toDouble();
            
            qDebug() << QString("  [%1] %2 (relevance: %3)")
                        .arg(id).arg(title).arg(relevance);
        }
    }
    
    // ✅ 获取数据库统计信息
    static void getDatabaseStats() {
        QSqlQuery query;
        
        // 获取表大小
        query.exec(R"(
            SELECT 
                table_name,
                ROUND(((data_length + index_length) / 1024 / 1024), 2) AS size_mb,
                table_rows
            FROM information_schema.TABLES
            WHERE table_schema = DATABASE()
            ORDER BY (data_length + index_length) DESC
        )");
        
        qDebug() << "Database statistics:";
        while (query.next()) {
            QString tableName = query.value(0).toString();
            double sizeMB = query.value(1).toDouble();
            int rows = query.value(2).toInt();
            
            qDebug() << QString("  %1: %2 MB, %3 rows")
                        .arg(tableName).arg(sizeMB).arg(rows);
        }
    }
    
    // ✅ 使用存储过程
    static void callStoredProcedure(int userId) {
        QSqlQuery query;
        query.prepare("CALL get_user_posts(?)");
        query.addBindValue(userId);
        
        if (!query.exec()) {
            qDebug() << "Failed to call stored procedure:" << query.lastError().text();
            return;
        }
        
        qDebug() << "Stored procedure results:";
        while (query.next()) {
            qDebug() << "  -" << query.value(0).toString();
        }
    }
};

// 使用示例
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // 连接到 MySQL
    if (!MySQLDatabase::connectToMySQL(
            "localhost",      // host
            3306,            // port
            "myapp",         // database
            "root",          // username
            "password"       // password
        )) {
        return -1;
    }
    
    // 创建表
    MySQLDatabase::createTables();
    
    // 批量插入
    QList<QVariantMap> users;
    users.append({
        {"username", "alice"},
        {"email", "alice@example.com"},
        {"password_hash", "hash123"},
        {"age", 25}
    });
    users.append({
        {"username", "bob"},
        {"email", "bob@example.com"},
        {"password_hash", "hash456"},
        {"age", 30}
    });
    
    MySQLDatabase::batchInsertUsers(users);
    
    // 全文搜索
    MySQLDatabase::fullTextSearch("keyword");
    
    // 获取统计信息
    MySQLDatabase::getDatabaseStats();
    
    return 0;
}
```

### 4.2 MySQL 连接池

```cpp
#include <QSqlDatabase>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>

class MySQLConnectionPool {
public:
    static MySQLConnectionPool& instance() {
        static MySQLConnectionPool pool;
        return pool;
    }
    
    // ✅ 初始化连接池
    bool initialize(const QString &host,
                   int port,
                   const QString &database,
                   const QString &username,
                   const QString &password,
                   int poolSize = 5) {
        QMutexLocker locker(&m_mutex);
        
        m_host = host;
        m_port = port;
        m_database = database;
        m_username = username;
        m_password = password;
        
        // 创建连接
        for (int i = 0; i < poolSize; ++i) {
            QString connectionName = QString("mysql_conn_%1").arg(i);
            QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connectionName);
            
            db.setHostName(m_host);
            db.setPort(m_port);
            db.setDatabaseName(m_database);
            db.setUserName(m_username);
            db.setPassword(m_password);
            
            if (!db.open()) {
                qDebug() << "Failed to open connection:" << db.lastError().text();
                return false;
            }
            
            m_availableConnections.enqueue(connectionName);
        }
        
        qDebug() << "Connection pool initialized with" << poolSize << "connections";
        return true;
    }
    
    // ✅ 获取连接
    QString getConnection() {
        QMutexLocker locker(&m_mutex);
        
        while (m_availableConnections.isEmpty()) {
            qDebug() << "Waiting for available connection...";
            m_condition.wait(&m_mutex);
        }
        
        QString connectionName = m_availableConnections.dequeue();
        m_usedConnections.insert(connectionName);
        
        qDebug() << "Connection acquired:" << connectionName;
        return connectionName;
    }
    
    // ✅ 释放连接
    void releaseConnection(const QString &connectionName) {
        QMutexLocker locker(&m_mutex);
        
        if (m_usedConnections.contains(connectionName)) {
            m_usedConnections.remove(connectionName);
            m_availableConnections.enqueue(connectionName);
            m_condition.wakeOne();
            
            qDebug() << "Connection released:" << connectionName;
        }
    }
    
    // ✅ 关闭所有连接
    void closeAll() {
        QMutexLocker locker(&m_mutex);
        
        // 关闭所有连接
        for (const QString &connectionName : m_usedConnections) {
            QSqlDatabase::database(connectionName).close();
            QSqlDatabase::removeDatabase(connectionName);
        }
        
        while (!m_availableConnections.isEmpty()) {
            QString connectionName = m_availableConnections.dequeue();
            QSqlDatabase::database(connectionName).close();
            QSqlDatabase::removeDatabase(connectionName);
        }
        
        qDebug() << "All connections closed";
    }
    
private:
    MySQLConnectionPool() = default;
    ~MySQLConnectionPool() { closeAll(); }
    
    MySQLConnectionPool(const MySQLConnectionPool&) = delete;
    MySQLConnectionPool& operator=(const MySQLConnectionPool&) = delete;
    
    QString m_host;
    int m_port;
    QString m_database;
    QString m_username;
    QString m_password;
    
    QQueue<QString> m_availableConnections;
    QSet<QString> m_usedConnections;
    QMutex m_mutex;
    QWaitCondition m_condition;
};

// 使用示例
void useConnectionPool() {
    // 初始化连接池
    MySQLConnectionPool::instance().initialize(
        "localhost", 3306, "myapp", "root", "password", 5
    );
    
    // 获取连接
    QString conn = MySQLConnectionPool::instance().getConnection();
    
    // 使用连接
    QSqlDatabase db = QSqlDatabase::database(conn);
    QSqlQuery query(db);
    query.exec("SELECT * FROM users");
    
    // 释放连接
    MySQLConnectionPool::instance().releaseConnection(conn);
}
```


---

## 5. PostgreSQL 数据库

### 5.1 PostgreSQL 连接配置

```cpp
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class PostgreSQLDatabase {
public:
    // ✅ 连接到 PostgreSQL 数据库
    static bool connectToPostgreSQL(const QString &host,
                                   int port,
                                   const QString &database,
                                   const QString &username,
                                   const QString &password) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
        
        db.setHostName(host);
        db.setPort(port);
        db.setDatabaseName(database);
        db.setUserName(username);
        db.setPassword(password);
        
        // 设置连接选项
        db.setConnectOptions("connect_timeout=10");
        
        if (!db.open()) {
            qDebug() << "Failed to connect to PostgreSQL:" << db.lastError().text();
            return false;
        }
        
        qDebug() << "Connected to PostgreSQL successfully";
        qDebug() << "  Host:" << host;
        qDebug() << "  Database:" << database;
        qDebug() << "  Port:" << port;
        
        // 获取 PostgreSQL 版本
        QSqlQuery query;
        query.exec("SELECT version()");
        if (query.next()) {
            qDebug() << "  Version:" << query.value(0).toString();
        }
        
        return true;
    }
    
    // ✅ 创建表（PostgreSQL 特定语法）
    static bool createTables() {
        QSqlQuery query;
        
        // 创建用户表
        QString createUsersTable = R"(
            CREATE TABLE IF NOT EXISTS users (
                id SERIAL PRIMARY KEY,
                username VARCHAR(50) NOT NULL UNIQUE,
                email VARCHAR(100) NOT NULL,
                password_hash VARCHAR(255) NOT NULL,
                age INTEGER CHECK (age >= 0 AND age <= 150),
                status VARCHAR(20) DEFAULT 'active' 
                    CHECK (status IN ('active', 'inactive', 'banned')),
                metadata JSONB,
                created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
            )
        )";
        
        if (!query.exec(createUsersTable)) {
            qDebug() << "Failed to create users table:" << query.lastError().text();
            return false;
        }
        
        // 创建索引
        query.exec("CREATE INDEX IF NOT EXISTS idx_users_username ON users(username)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_users_email ON users(email)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_users_metadata ON users USING GIN(metadata)");
        
        // 创建文章表
        QString createPostsTable = R"(
            CREATE TABLE IF NOT EXISTS posts (
                id SERIAL PRIMARY KEY,
                user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
                title VARCHAR(200) NOT NULL,
                content TEXT,
                tags TEXT[],
                view_count INTEGER DEFAULT 0,
                search_vector tsvector,
                created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
            )
        )";
        
        if (!query.exec(createPostsTable)) {
            qDebug() << "Failed to create posts table:" << query.lastError().text();
            return false;
        }
        
        // 创建全文搜索索引
        query.exec("CREATE INDEX IF NOT EXISTS idx_posts_search ON posts USING GIN(search_vector)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_posts_tags ON posts USING GIN(tags)");
        
        // 创建触发器自动更新 updated_at
        QString createTrigger = R"(
            CREATE OR REPLACE FUNCTION update_updated_at_column()
            RETURNS TRIGGER AS $$
            BEGIN
                NEW.updated_at = CURRENT_TIMESTAMP;
                RETURN NEW;
            END;
            $$ language 'plpgsql';
            
            DROP TRIGGER IF EXISTS update_users_updated_at ON users;
            CREATE TRIGGER update_users_updated_at 
                BEFORE UPDATE ON users 
                FOR EACH ROW 
                EXECUTE FUNCTION update_updated_at_column();
            
            DROP TRIGGER IF EXISTS update_posts_updated_at ON posts;
            CREATE TRIGGER update_posts_updated_at 
                BEFORE UPDATE ON posts 
                FOR EACH ROW 
                EXECUTE FUNCTION update_updated_at_column();
        )";
        
        if (!query.exec(createTrigger)) {
            qDebug() << "Failed to create trigger:" << query.lastError().text();
        }
        
        qDebug() << "Tables created successfully";
        return true;
    }
    
    // ✅ 使用 JSONB 类型
    static bool insertUserWithMetadata(const QString &username,
                                      const QString &email,
                                      const QJsonObject &metadata) {
        QSqlQuery query;
        query.prepare(R"(
            INSERT INTO users (username, email, password_hash, metadata)
            VALUES (?, ?, ?, ?::jsonb)
        )");
        query.addBindValue(username);
        query.addBindValue(email);
        query.addBindValue("hash123");
        query.addBindValue(QJsonDocument(metadata).toJson(QJsonDocument::Compact));
        
        if (!query.exec()) {
            qDebug() << "Failed to insert user:" << query.lastError().text();
            return false;
        }
        
        qDebug() << "User inserted with metadata";
        return true;
    }
    
    // ✅ 查询 JSONB 数据
    static void queryUsersByMetadata(const QString &key, const QString &value) {
        QSqlQuery query;
        query.prepare("SELECT username, metadata FROM users WHERE metadata->? = ?");
        query.addBindValue(key);
        query.addBindValue(QJsonDocument(QJsonArray{value}).toJson());
        
        if (!query.exec()) {
            qDebug() << "Query failed:" << query.lastError().text();
            return;
        }
        
        qDebug() << "Users with" << key << "=" << value << ":";
        while (query.next()) {
            QString username = query.value(0).toString();
            QJsonDocument metadata = QJsonDocument::fromJson(query.value(1).toByteArray());
            qDebug() << "  -" << username << ":" << metadata.toJson(QJsonDocument::Compact);
        }
    }
    
    // ✅ 使用数组类型
    static bool insertPostWithTags(int userId,
                                  const QString &title,
                                  const QString &content,
                                  const QStringList &tags) {
        QSqlQuery query;
        query.prepare(R"(
            INSERT INTO posts (user_id, title, content, tags)
            VALUES (?, ?, ?, ?)
        )");
        query.addBindValue(userId);
        query.addBindValue(title);
        query.addBindValue(content);
        
        // 将 QStringList 转换为 PostgreSQL 数组格式
        QString tagsArray = "{" + tags.join(",") + "}";
        query.addBindValue(tagsArray);
        
        if (!query.exec()) {
            qDebug() << "Failed to insert post:" << query.lastError().text();
            return false;
        }
        
        qDebug() << "Post inserted with tags";
        return true;
    }
    
    // ✅ 全文搜索（PostgreSQL tsvector）
    static void fullTextSearch(const QString &keyword) {
        QSqlQuery query;
        query.prepare(R"(
            SELECT id, title, content,
                   ts_rank(search_vector, plainto_tsquery('english', ?)) AS rank
            FROM posts
            WHERE search_vector @@ plainto_tsquery('english', ?)
            ORDER BY rank DESC
            LIMIT 10
        )");
        query.addBindValue(keyword);
        query.addBindValue(keyword);
        
        if (!query.exec()) {
            qDebug() << "Search failed:" << query.lastError().text();
            return;
        }
        
        qDebug() << "Search results for:" << keyword;
        while (query.next()) {
            int id = query.value(0).toInt();
            QString title = query.value(1).toString();
            double rank = query.value(3).toDouble();
            
            qDebug() << QString("  [%1] %2 (rank: %3)")
                        .arg(id).arg(title).arg(rank);
        }
    }
    
    // ✅ 使用 CTE（Common Table Expressions）
    static void queryWithCTE() {
        QSqlQuery query;
        query.exec(R"(
            WITH user_stats AS (
                SELECT 
                    u.id,
                    u.username,
                    COUNT(p.id) AS post_count,
                    SUM(p.view_count) AS total_views
                FROM users u
                LEFT JOIN posts p ON u.id = p.user_id
                GROUP BY u.id, u.username
            )
            SELECT 
                username,
                post_count,
                total_views,
                CASE 
                    WHEN post_count > 10 THEN 'active'
                    WHEN post_count > 5 THEN 'moderate'
                    ELSE 'inactive'
                END AS activity_level
            FROM user_stats
            ORDER BY total_views DESC
        )");
        
        qDebug() << "User statistics:";
        while (query.next()) {
            QString username = query.value(0).toString();
            int postCount = query.value(1).toInt();
            int totalViews = query.value(2).toInt();
            QString activityLevel = query.value(3).toString();
            
            qDebug() << QString("  %1: %2 posts, %3 views (%4)")
                        .arg(username).arg(postCount).arg(totalViews).arg(activityLevel);
        }
    }
    
    // ✅ 使用窗口函数
    static void queryWithWindowFunction() {
        QSqlQuery query;
        query.exec(R"(
            SELECT 
                username,
                created_at,
                ROW_NUMBER() OVER (ORDER BY created_at) AS row_num,
                RANK() OVER (ORDER BY age DESC) AS age_rank,
                DENSE_RANK() OVER (ORDER BY age DESC) AS age_dense_rank
            FROM users
            ORDER BY created_at
        )");
        
        qDebug() << "Users with window functions:";
        while (query.next()) {
            QString username = query.value(0).toString();
            int rowNum = query.value(2).toInt();
            int ageRank = query.value(3).toInt();
            
            qDebug() << QString("  #%1: %2 (age rank: %3)")
                        .arg(rowNum).arg(username).arg(ageRank);
        }
    }
    
    // ✅ 使用 LISTEN/NOTIFY（异步通知）
    static void setupNotification() {
        QSqlDatabase db = QSqlDatabase::database();
        QSqlDriver *driver = db.driver();
        
        if (driver->hasFeature(QSqlDriver::EventNotifications)) {
            driver->subscribeToNotification("user_changes");
            
            QObject::connect(driver, &QSqlDriver::notification,
                           [](const QString &name, QSqlDriver::NotificationSource source, const QVariant &payload) {
                qDebug() << "Notification received:";
                qDebug() << "  Channel:" << name;
                qDebug() << "  Payload:" << payload.toString();
            });
            
            qDebug() << "Subscribed to notifications";
        }
    }
    
    static void sendNotification(const QString &message) {
        QSqlQuery query;
        query.prepare("NOTIFY user_changes, ?");
        query.addBindValue(message);
        query.exec();
    }
};

// 使用示例
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // 连接到 PostgreSQL
    if (!PostgreSQLDatabase::connectToPostgreSQL(
            "localhost",      // host
            5432,            // port
            "myapp",         // database
            "postgres",      // username
            "password"       // password
        )) {
        return -1;
    }
    
    // 创建表
    PostgreSQLDatabase::createTables();
    
    // 使用 JSONB
    QJsonObject metadata;
    metadata["role"] = "admin";
    metadata["preferences"] = QJsonObject{{"theme", "dark"}};
    PostgreSQLDatabase::insertUserWithMetadata("alice", "alice@example.com", metadata);
    
    // 使用数组
    PostgreSQLDatabase::insertPostWithTags(1, "My Post", "Content", {"tech", "programming", "qt"});
    
    // 全文搜索
    PostgreSQLDatabase::fullTextSearch("programming");
    
    // CTE 查询
    PostgreSQLDatabase::queryWithCTE();
    
    // 窗口函数
    PostgreSQLDatabase::queryWithWindowFunction();
    
    return 0;
}
```


---

## 6. 数据库连接管理

### 6.1 多数据库连接

```cpp
#include <QSqlDatabase>
#include <QDebug>

class DatabaseManager {
public:
    // ✅ 添加多个数据库连接
    static bool setupDatabases() {
        // 默认连接（SQLite）
        QSqlDatabase defaultDb = QSqlDatabase::addDatabase("QSQLITE");
        defaultDb.setDatabaseName("local.db");
        
        if (!defaultDb.open()) {
            qDebug() << "Failed to open default database";
            return false;
        }
        
        // 命名连接（MySQL）
        QSqlDatabase mysqlDb = QSqlDatabase::addDatabase("QMYSQL", "mysql_connection");
        mysqlDb.setHostName("localhost");
        mysqlDb.setDatabaseName("myapp");
        mysqlDb.setUserName("root");
        mysqlDb.setPassword("password");
        
        if (!mysqlDb.open()) {
            qDebug() << "Failed to open MySQL database";
            return false;
        }
        
        // 命名连接（PostgreSQL）
        QSqlDatabase pgDb = QSqlDatabase::addDatabase("QPSQL", "pg_connection");
        pgDb.setHostName("localhost");
        pgDb.setDatabaseName("myapp");
        pgDb.setUserName("postgres");
        pgDb.setPassword("password");
        
        if (!pgDb.open()) {
            qDebug() << "Failed to open PostgreSQL database";
            return false;
        }
        
        qDebug() << "All databases connected successfully";
        return true;
    }
    
    // ✅ 使用特定连接
    static void useSpecificConnection() {
        // 使用默认连接
        QSqlQuery defaultQuery;
        defaultQuery.exec("SELECT * FROM local_table");
        
        // 使用 MySQL 连接
        QSqlQuery mysqlQuery(QSqlDatabase::database("mysql_connection"));
        mysqlQuery.exec("SELECT * FROM mysql_table");
        
        // 使用 PostgreSQL 连接
        QSqlQuery pgQuery(QSqlDatabase::database("pg_connection"));
        pgQuery.exec("SELECT * FROM pg_table");
    }
    
    // ✅ 获取所有连接
    static void listConnections() {
        QStringList connections = QSqlDatabase::connectionNames();
        
        qDebug() << "Active database connections:";
        for (const QString &name : connections) {
            QSqlDatabase db = QSqlDatabase::database(name);
            qDebug() << "  -" << name << ":" << db.driverName() 
                     << "(" << db.databaseName() << ")";
        }
    }
    
    // ✅ 关闭特定连接
    static void closeConnection(const QString &connectionName) {
        if (QSqlDatabase::contains(connectionName)) {
            QSqlDatabase db = QSqlDatabase::database(connectionName);
            db.close();
            QSqlDatabase::removeDatabase(connectionName);
            qDebug() << "Connection closed:" << connectionName;
        }
    }
    
    // ✅ 关闭所有连接
    static void closeAllConnections() {
        QStringList connections = QSqlDatabase::connectionNames();
        
        for (const QString &name : connections) {
            QSqlDatabase db = QSqlDatabase::database(name);
            db.close();
        }
        
        // 移除所有连接
        for (const QString &name : connections) {
            QSqlDatabase::removeDatabase(name);
        }
        
        qDebug() << "All connections closed";
    }
};
```

### 6.2 连接状态检查

```cpp
class ConnectionChecker {
public:
    // ✅ 检查连接是否有效
    static bool isConnectionValid(const QString &connectionName = QString()) {
        QSqlDatabase db = connectionName.isEmpty() 
            ? QSqlDatabase::database()
            : QSqlDatabase::database(connectionName);
        
        if (!db.isValid()) {
            qDebug() << "Database connection is not valid";
            return false;
        }
        
        if (!db.isOpen()) {
            qDebug() << "Database is not open";
            return false;
        }
        
        // 执行简单查询测试连接
        QSqlQuery query(db);
        if (!query.exec("SELECT 1")) {
            qDebug() << "Connection test failed:" << query.lastError().text();
            return false;
        }
        
        return true;
    }
    
    // ✅ 自动重连
    static bool reconnect(const QString &connectionName = QString()) {
        QSqlDatabase db = connectionName.isEmpty()
            ? QSqlDatabase::database()
            : QSqlDatabase::database(connectionName);
        
        if (db.isOpen()) {
            db.close();
        }
        
        if (!db.open()) {
            qDebug() << "Reconnection failed:" << db.lastError().text();
            return false;
        }
        
        qDebug() << "Reconnected successfully";
        return true;
    }
    
    // ✅ 获取连接信息
    static void printConnectionInfo(const QString &connectionName = QString()) {
        QSqlDatabase db = connectionName.isEmpty()
            ? QSqlDatabase::database()
            : QSqlDatabase::database(connectionName);
        
        qDebug() << "Connection Information:";
        qDebug() << "  Connection Name:" << db.connectionName();
        qDebug() << "  Driver:" << db.driverName();
        qDebug() << "  Database:" << db.databaseName();
        qDebug() << "  Host:" << db.hostName();
        qDebug() << "  Port:" << db.port();
        qDebug() << "  User:" << db.userName();
        qDebug() << "  Is Open:" << db.isOpen();
        qDebug() << "  Is Valid:" << db.isValid();
        qDebug() << "  Tables:" << db.tables();
    }
};
```

---

## 7. SQL 查询与执行

### 7.1 基本查询操作

```cpp
class QueryOperations {
public:
    // ✅ SELECT 查询
    static void selectQuery() {
        QSqlQuery query;
        
        // 简单查询
        if (!query.exec("SELECT id, username, email FROM users")) {
            qDebug() << "Query failed:" << query.lastError().text();
            return;
        }
        
        // 遍历结果
        while (query.next()) {
            int id = query.value(0).toInt();
            QString username = query.value(1).toString();
            QString email = query.value(2).toString();
            
            qDebug() << id << username << email;
        }
        
        // 或使用字段名
        query.exec("SELECT id, username, email FROM users");
        while (query.next()) {
            int id = query.value("id").toInt();
            QString username = query.value("username").toString();
            QString email = query.value("email").toString();
            
            qDebug() << id << username << email;
        }
    }
    
    // ✅ INSERT 查询
    static int insertQuery(const QString &username, const QString &email) {
        QSqlQuery query;
        query.prepare("INSERT INTO users (username, email) VALUES (?, ?)");
        query.addBindValue(username);
        query.addBindValue(email);
        
        if (!query.exec()) {
            qDebug() << "Insert failed:" << query.lastError().text();
            return -1;
        }
        
        // 获取插入的 ID
        int id = query.lastInsertId().toInt();
        qDebug() << "Inserted user with ID:" << id;
        
        return id;
    }
    
    // ✅ UPDATE 查询
    static bool updateQuery(int userId, const QString &email) {
        QSqlQuery query;
        query.prepare("UPDATE users SET email = ? WHERE id = ?");
        query.addBindValue(email);
        query.addBindValue(userId);
        
        if (!query.exec()) {
            qDebug() << "Update failed:" << query.lastError().text();
            return false;
        }
        
        int rowsAffected = query.numRowsAffected();
        qDebug() << "Updated" << rowsAffected << "rows";
        
        return rowsAffected > 0;
    }
    
    // ✅ DELETE 查询
    static bool deleteQuery(int userId) {
        QSqlQuery query;
        query.prepare("DELETE FROM users WHERE id = ?");
        query.addBindValue(userId);
        
        if (!query.exec()) {
            qDebug() << "Delete failed:" << query.lastError().text();
            return false;
        }
        
        int rowsAffected = query.numRowsAffected();
        qDebug() << "Deleted" << rowsAffected << "rows";
        
        return rowsAffected > 0;
    }
    
    // ✅ 获取查询元数据
    static void queryMetadata() {
        QSqlQuery query("SELECT * FROM users LIMIT 1");
        
        if (!query.exec()) {
            return;
        }
        
        QSqlRecord record = query.record();
        
        qDebug() << "Query metadata:";
        qDebug() << "  Field count:" << record.count();
        
        for (int i = 0; i < record.count(); ++i) {
            QSqlField field = record.field(i);
            qDebug() << QString("  Field %1: %2 (%3)")
                        .arg(i)
                        .arg(field.name())
                        .arg(QMetaType(field.metaType()).name());
        }
    }
    
    // ✅ 检查查询结果
    static void checkQueryResult() {
        QSqlQuery query("SELECT * FROM users");
        
        // 检查查询是否成功
        if (!query.isActive()) {
            qDebug() << "Query is not active";
            return;
        }
        
        // 检查是否有结果
        if (!query.isSelect()) {
            qDebug() << "Query is not a SELECT statement";
            return;
        }
        
        // 获取结果集大小（不是所有驱动都支持）
        int size = query.size();
        if (size >= 0) {
            qDebug() << "Result set size:" << size;
        } else {
            qDebug() << "Size not available, counting manually...";
            int count = 0;
            while (query.next()) {
                count++;
            }
            qDebug() << "Result count:" << count;
        }
    }
};
```

---

## 8. 预处理语句与参数绑定

### 8.1 参数绑定方式

```cpp
class PreparedStatements {
public:
    // ✅ 位置绑定（使用 ?）
    static void positionalBinding() {
        QSqlQuery query;
        query.prepare("INSERT INTO users (username, email, age) VALUES (?, ?, ?)");
        
        // 按顺序绑定参数
        query.addBindValue("alice");
        query.addBindValue("alice@example.com");
        query.addBindValue(25);
        
        if (!query.exec()) {
            qDebug() << "Insert failed:" << query.lastError().text();
        }
    }
    
    // ✅ 命名绑定（使用 :name）
    static void namedBinding() {
        QSqlQuery query;
        query.prepare("INSERT INTO users (username, email, age) VALUES (:username, :email, :age)");
        
        // 使用名称绑定参数
        query.bindValue(":username", "bob");
        query.bindValue(":email", "bob@example.com");
        query.bindValue(":age", 30);
        
        if (!query.exec()) {
            qDebug() << "Insert failed:" << query.lastError().text();
        }
    }
    
    // ✅ 批量插入
    static void batchInsert() {
        QSqlQuery query;
        query.prepare("INSERT INTO users (username, email, age) VALUES (?, ?, ?)");
        
        // 准备批量数据
        QVariantList usernames;
        usernames << "user1" << "user2" << "user3";
        
        QVariantList emails;
        emails << "user1@example.com" << "user2@example.com" << "user3@example.com";
        
        QVariantList ages;
        ages << 20 << 25 << 30;
        
        // 绑定批量数据
        query.addBindValue(usernames);
        query.addBindValue(emails);
        query.addBindValue(ages);
        
        // 执行批量插入
        if (!query.execBatch()) {
            qDebug() << "Batch insert failed:" << query.lastError().text();
        } else {
            qDebug() << "Batch insert successful";
        }
    }
    
    // ✅ NULL 值处理
    static void handleNullValues() {
        QSqlQuery query;
        query.prepare("INSERT INTO users (username, email, age) VALUES (?, ?, ?)");
        
        query.addBindValue("charlie");
        query.addBindValue("charlie@example.com");
        query.addBindValue(QVariant(QMetaType::fromType<int>()));  // NULL
        
        if (!query.exec()) {
            qDebug() << "Insert failed:" << query.lastError().text();
        }
    }
    
    // ✅ 二进制数据（BLOB）
    static void insertBinaryData(const QByteArray &data) {
        QSqlQuery query;
        query.prepare("INSERT INTO files (filename, data) VALUES (?, ?)");
        query.addBindValue("image.png");
        query.addBindValue(data);
        
        if (!query.exec()) {
            qDebug() << "Insert binary data failed:" << query.lastError().text();
        }
    }
    
    static QByteArray readBinaryData(int fileId) {
        QSqlQuery query;
        query.prepare("SELECT data FROM files WHERE id = ?");
        query.addBindValue(fileId);
        
        if (query.exec() && query.next()) {
            return query.value(0).toByteArray();
        }
        
        return QByteArray();
    }
};
```


---

## 9. 事务处理

### 9.1 事务基础

```cpp
class TransactionManager {
public:
    // ✅ 基本事务操作
    static bool basicTransaction() {
        QSqlDatabase db = QSqlDatabase::database();
        
        // 开始事务
        if (!db.transaction()) {
            qDebug() << "Failed to start transaction";
            return false;
        }
        
        QSqlQuery query;
        
        // 执行多个操作
        query.prepare("INSERT INTO users (username, email) VALUES (?, ?)");
        query.addBindValue("alice");
        query.addBindValue("alice@example.com");
        
        if (!query.exec()) {
            qDebug() << "Insert failed, rolling back";
            db.rollback();
            return false;
        }
        
        int userId = query.lastInsertId().toInt();
        
        query.prepare("INSERT INTO posts (user_id, title) VALUES (?, ?)");
        query.addBindValue(userId);
        query.addBindValue("First Post");
        
        if (!query.exec()) {
            qDebug() << "Insert post failed, rolling back";
            db.rollback();
            return false;
        }
        
        // 提交事务
        if (!db.commit()) {
            qDebug() << "Failed to commit transaction";
            return false;
        }
        
        qDebug() << "Transaction committed successfully";
        return true;
    }
    
    // ✅ 转账示例（经典事务场景）
    static bool transfer(int fromUserId, int toUserId, double amount) {
        QSqlDatabase db = QSqlDatabase::database();
        
        if (!db.transaction()) {
            qDebug() << "Failed to start transaction";
            return false;
        }
        
        QSqlQuery query;
        
        // 检查余额
        query.prepare("SELECT balance FROM accounts WHERE user_id = ?");
        query.addBindValue(fromUserId);
        
        if (!query.exec() || !query.next()) {
            db.rollback();
            return false;
        }
        
        double balance = query.value(0).toDouble();
        if (balance < amount) {
            qDebug() << "Insufficient balance";
            db.rollback();
            return false;
        }
        
        // 扣款
        query.prepare("UPDATE accounts SET balance = balance - ? WHERE user_id = ?");
        query.addBindValue(amount);
        query.addBindValue(fromUserId);
        
        if (!query.exec()) {
            qDebug() << "Debit failed";
            db.rollback();
            return false;
        }
        
        // 入账
        query.prepare("UPDATE accounts SET balance = balance + ? WHERE user_id = ?");
        query.addBindValue(amount);
        query.addBindValue(toUserId);
        
        if (!query.exec()) {
            qDebug() << "Credit failed";
            db.rollback();
            return false;
        }
        
        // 记录交易
        query.prepare("INSERT INTO transactions (from_user, to_user, amount) VALUES (?, ?, ?)");
        query.addBindValue(fromUserId);
        query.addBindValue(toUserId);
        query.addBindValue(amount);
        
        if (!query.exec()) {
            qDebug() << "Transaction log failed";
            db.rollback();
            return false;
        }
        
        // 提交事务
        if (!db.commit()) {
            qDebug() << "Failed to commit";
            return false;
        }
        
        qDebug() << "Transfer completed successfully";
        return true;
    }
    
    // ✅ 保存点（Savepoint）
    static bool useSavepoint() {
        QSqlDatabase db = QSqlDatabase::database();
        
        if (!db.transaction()) {
            return false;
        }
        
        QSqlQuery query;
        
        // 第一个操作
        query.exec("INSERT INTO users (username) VALUES ('user1')");
        
        // 创建保存点
        query.exec("SAVEPOINT sp1");
        
        // 第二个操作
        query.exec("INSERT INTO users (username) VALUES ('user2')");
        
        // 回滚到保存点（撤销 user2，保留 user1）
        query.exec("ROLLBACK TO SAVEPOINT sp1");
        
        // 提交事务
        db.commit();
        
        return true;
    }
    
    // ✅ 事务隔离级别
    static void setIsolationLevel() {
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query;
        
        // MySQL
        query.exec("SET TRANSACTION ISOLATION LEVEL READ COMMITTED");
        
        // PostgreSQL
        query.exec("SET TRANSACTION ISOLATION LEVEL SERIALIZABLE");
        
        // SQLite（默认 SERIALIZABLE）
        query.exec("PRAGMA read_uncommitted = 0");
    }
};
```

---

## 10. QSqlTableModel 与 QSqlQueryModel

### 10.1 QSqlTableModel 使用

```cpp
#include <QSqlTableModel>
#include <QTableView>

class TableModelExample : public QObject {
    Q_OBJECT
    
public:
    TableModelExample(QObject *parent = nullptr) : QObject(parent) {
        setupModel();
    }
    
    void setupModel() {
        // ✅ 创建表模型
        m_model = new QSqlTableModel(this);
        m_model->setTable("users");
        
        // 设置编辑策略
        m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
        // - OnFieldChange: 字段改变时立即提交
        // - OnRowChange: 行改变时提交
        // - OnManualSubmit: 手动提交
        
        // 选择数据
        m_model->select();
        
        // 设置列标题
        m_model->setHeaderData(0, Qt::Horizontal, "ID");
        m_model->setHeaderData(1, Qt::Horizontal, "Username");
        m_model->setHeaderData(2, Qt::Horizontal, "Email");
        m_model->setHeaderData(3, Qt::Horizontal, "Age");
        
        // 设置过滤
        m_model->setFilter("age >= 18");
        
        // 设置排序
        m_model->setSort(1, Qt::AscendingOrder);  // 按 username 升序
        
        qDebug() << "Model row count:" << m_model->rowCount();
    }
    
    // ✅ 插入行
    void insertRow() {
        int row = m_model->rowCount();
        m_model->insertRow(row);
        
        m_model->setData(m_model->index(row, 1), "newuser");
        m_model->setData(m_model->index(row, 2), "newuser@example.com");
        m_model->setData(m_model->index(row, 3), 25);
        
        // 提交更改
        if (m_model->submitAll()) {
            qDebug() << "Row inserted successfully";
        } else {
            qDebug() << "Insert failed:" << m_model->lastError().text();
            m_model->revertAll();
        }
    }
    
    // ✅ 更新行
    void updateRow(int row, const QString &email) {
        m_model->setData(m_model->index(row, 2), email);
        
        if (m_model->submitAll()) {
            qDebug() << "Row updated successfully";
        } else {
            qDebug() << "Update failed:" << m_model->lastError().text();
            m_model->revertAll();
        }
    }
    
    // ✅ 删除行
    void deleteRow(int row) {
        m_model->removeRow(row);
        
        if (m_model->submitAll()) {
            qDebug() << "Row deleted successfully";
        } else {
            qDebug() << "Delete failed:" << m_model->lastError().text();
            m_model->revertAll();
        }
    }
    
    // ✅ 获取数据
    void getData(int row, int column) {
        QModelIndex index = m_model->index(row, column);
        QVariant data = m_model->data(index);
        qDebug() << "Data at" << row << "," << column << ":" << data;
    }
    
    // ✅ 刷新数据
    void refresh() {
        m_model->select();
        qDebug() << "Model refreshed";
    }
    
    QSqlTableModel* model() { return m_model; }
    
private:
    QSqlTableModel *m_model;
};
```

### 10.2 QSqlQueryModel 使用

```cpp
#include <QSqlQueryModel>

class QueryModelExample : public QObject {
    Q_OBJECT
    
public:
    QueryModelExample(QObject *parent = nullptr) : QObject(parent) {
        setupModel();
    }
    
    void setupModel() {
        // ✅ 创建查询模型（只读）
        m_model = new QSqlQueryModel(this);
        
        // 设置查询
        m_model->setQuery(R"(
            SELECT 
                u.id,
                u.username,
                u.email,
                COUNT(p.id) AS post_count
            FROM users u
            LEFT JOIN posts p ON u.id = p.user_id
            GROUP BY u.id, u.username, u.email
            ORDER BY post_count DESC
        )");
        
        if (m_model->lastError().isValid()) {
            qDebug() << "Query failed:" << m_model->lastError().text();
            return;
        }
        
        // 设置列标题
        m_model->setHeaderData(0, Qt::Horizontal, "ID");
        m_model->setHeaderData(1, Qt::Horizontal, "Username");
        m_model->setHeaderData(2, Qt::Horizontal, "Email");
        m_model->setHeaderData(3, Qt::Horizontal, "Posts");
        
        qDebug() << "Query model row count:" << m_model->rowCount();
    }
    
    // ✅ 获取数据
    void printData() {
        for (int row = 0; row < m_model->rowCount(); ++row) {
            QString username = m_model->record(row).value("username").toString();
            int postCount = m_model->record(row).value("post_count").toInt();
            
            qDebug() << username << "has" << postCount << "posts";
        }
    }
    
    QSqlQueryModel* model() { return m_model; }
    
private:
    QSqlQueryModel *m_model;
};
```

---

## 11. QML 数据库集成

### 11.1 创建数据库管理类

```cpp
// databasemanager.h
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantList>
#include <QVariantMap>

class DatabaseManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    
    bool connected() const { return m_connected; }
    
    // ✅ 初始化数据库
    Q_INVOKABLE bool initialize(const QString &dbPath);
    
    // ✅ 执行查询
    Q_INVOKABLE QVariantList query(const QString &sql);
    
    // ✅ 执行更新
    Q_INVOKABLE bool execute(const QString &sql);
    
    // ✅ 插入数据
    Q_INVOKABLE int insert(const QString &table, const QVariantMap &data);
    
    // ✅ 更新数据
    Q_INVOKABLE bool update(const QString &table, 
                           const QVariantMap &data, 
                           const QString &where);
    
    // ✅ 删除数据
    Q_INVOKABLE bool remove(const QString &table, const QString &where);
    
    // ✅ 查询单条记录
    Q_INVOKABLE QVariantMap queryOne(const QString &sql);
    
signals:
    void connectedChanged();
    void error(const QString &message);
    
private:
    bool m_connected = false;
};

// databasemanager.cpp
DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {}

bool DatabaseManager::initialize(const QString &dbPath) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    
    if (!db.open()) {
        emit error("Failed to open database: " + db.lastError().text());
        return false;
    }
    
    m_connected = true;
    emit connectedChanged();
    
    return true;
}

QVariantList DatabaseManager::query(const QString &sql) {
    QVariantList results;
    QSqlQuery query(sql);
    
    if (!query.exec()) {
        emit error("Query failed: " + query.lastError().text());
        return results;
    }
    
    while (query.next()) {
        QVariantMap row;
        QSqlRecord record = query.record();
        
        for (int i = 0; i < record.count(); ++i) {
            row[record.fieldName(i)] = query.value(i);
        }
        
        results.append(row);
    }
    
    return results;
}

bool DatabaseManager::execute(const QString &sql) {
    QSqlQuery query;
    
    if (!query.exec(sql)) {
        emit error("Execute failed: " + query.lastError().text());
        return false;
    }
    
    return true;
}

int DatabaseManager::insert(const QString &table, const QVariantMap &data) {
    QStringList keys;
    QStringList placeholders;
    QVariantList values;
    
    for (auto it = data.begin(); it != data.end(); ++it) {
        keys.append(it.key());
        placeholders.append("?");
        values.append(it.value());
    }
    
    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                    .arg(table)
                    .arg(keys.join(", "))
                    .arg(placeholders.join(", "));
    
    QSqlQuery query;
    query.prepare(sql);
    
    for (const QVariant &value : values) {
        query.addBindValue(value);
    }
    
    if (!query.exec()) {
        emit error("Insert failed: " + query.lastError().text());
        return -1;
    }
    
    return query.lastInsertId().toInt();
}

bool DatabaseManager::update(const QString &table, 
                            const QVariantMap &data, 
                            const QString &where) {
    QStringList sets;
    QVariantList values;
    
    for (auto it = data.begin(); it != data.end(); ++it) {
        sets.append(it.key() + " = ?");
        values.append(it.value());
    }
    
    QString sql = QString("UPDATE %1 SET %2 WHERE %3")
                    .arg(table)
                    .arg(sets.join(", "))
                    .arg(where);
    
    QSqlQuery query;
    query.prepare(sql);
    
    for (const QVariant &value : values) {
        query.addBindValue(value);
    }
    
    if (!query.exec()) {
        emit error("Update failed: " + query.lastError().text());
        return false;
    }
    
    return true;
}

bool DatabaseManager::remove(const QString &table, const QString &where) {
    QString sql = QString("DELETE FROM %1 WHERE %2").arg(table, where);
    
    QSqlQuery query;
    if (!query.exec(sql)) {
        emit error("Delete failed: " + query.lastError().text());
        return false;
    }
    
    return true;
}

QVariantMap DatabaseManager::queryOne(const QString &sql) {
    QVariantMap result;
    QSqlQuery query(sql);
    
    if (!query.exec()) {
        emit error("Query failed: " + query.lastError().text());
        return result;
    }
    
    if (query.next()) {
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            result[record.fieldName(i)] = query.value(i);
        }
    }
    
    return result;
}
```

### 11.2 在 QML 中使用数据库

```qml
// Main.qml
import QtQuick
import QtQuick.Controls
import MyApp 1.0  // 假设 DatabaseManager 已注册

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    title: "Database Demo"
    
    DatabaseManager {
        id: db
        
        Component.onCompleted: {
            // 初始化数据库
            if (db.initialize("myapp.db")) {
                console.log("Database initialized")
                
                // 创建表
                db.execute(`
                    CREATE TABLE IF NOT EXISTS users (
                        id INTEGER PRIMARY KEY AUTOINCREMENT,
                        username TEXT NOT NULL,
                        email TEXT NOT NULL,
                        age INTEGER
                    )
                `)
                
                // 加载数据
                loadUsers()
            }
        }
        
        onError: function(message) {
            console.error("Database error:", message)
            errorDialog.text = message
            errorDialog.open()
        }
    }
    
    ListModel {
        id: userModel
    }
    
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10
        
        // 输入表单
        Row {
            spacing: 10
            
            TextField {
                id: usernameField
                placeholderText: "Username"
                width: 150
            }
            
            TextField {
                id: emailField
                placeholderText: "Email"
                width: 200
            }
            
            TextField {
                id: ageField
                placeholderText: "Age"
                width: 80
            }
            
            Button {
                text: "Add User"
                onClicked: addUser()
            }
        }
        
        // 用户列表
        ListView {
            width: parent.width
            height: parent.height - 100
            model: userModel
            
            delegate: Rectangle {
                width: parent.width
                height: 50
                color: index % 2 ? "#f0f0f0" : "white"
                
                Row {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 20
                    
                    Text {
                        text: model.id
                        width: 50
                    }
                    
                    Text {
                        text: model.username
                        width: 150
                    }
                    
                    Text {
                        text: model.email
                        width: 200
                    }
                    
                    Text {
                        text: model.age
                        width: 50
                    }
                    
                    Button {
                        text: "Delete"
                        onClicked: deleteUser(model.id)
                    }
                }
            }
        }
    }
    
    function loadUsers() {
        userModel.clear()
        
        var users = db.query("SELECT * FROM users ORDER BY id DESC")
        
        for (var i = 0; i < users.length; i++) {
            userModel.append(users[i])
        }
        
        console.log("Loaded", users.length, "users")
    }
    
    function addUser() {
        if (usernameField.text === "" || emailField.text === "") {
            return
        }
        
        var data = {
            username: usernameField.text,
            email: emailField.text,
            age: parseInt(ageField.text) || 0
        }
        
        var id = db.insert("users", data)
        
        if (id > 0) {
            console.log("User added with ID:", id)
            usernameField.text = ""
            emailField.text = ""
            ageField.text = ""
            loadUsers()
        }
    }
    
    function deleteUser(userId) {
        if (db.remove("users", "id = " + userId)) {
            console.log("User deleted:", userId)
            loadUsers()
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


---

## 12. 数据库性能优化

### 12.1 索引优化

```cpp
class IndexOptimization {
public:
    // ✅ 创建索引
    static void createIndexes() {
        QSqlQuery query;
        
        // 单列索引
        query.exec("CREATE INDEX IF NOT EXISTS idx_users_username ON users(username)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_users_email ON users(email)");
        
        // 复合索引
        query.exec("CREATE INDEX IF NOT EXISTS idx_posts_user_date ON posts(user_id, created_at)");
        
        // 唯一索引
        query.exec("CREATE UNIQUE INDEX IF NOT EXISTS idx_users_email_unique ON users(email)");
        
        // 部分索引（PostgreSQL）
        query.exec("CREATE INDEX IF NOT EXISTS idx_active_users ON users(username) WHERE status = 'active'");
        
        qDebug() << "Indexes created";
    }
    
    // ✅ 分析查询性能
    static void analyzeQuery(const QString &sql) {
        QSqlQuery query;
        
        // SQLite
        query.exec("EXPLAIN QUERY PLAN " + sql);
        
        // MySQL
        // query.exec("EXPLAIN " + sql);
        
        // PostgreSQL
        // query.exec("EXPLAIN ANALYZE " + sql);
        
        qDebug() << "Query plan:";
        while (query.next()) {
            qDebug() << "  " << query.value(0).toString();
        }
    }
};
```

### 12.2 批量操作优化

```cpp
class BatchOptimization {
public:
    // ✅ 批量插入（使用事务）
    static void batchInsertWithTransaction(const QList<QVariantMap> &records) {
        QSqlDatabase db = QSqlDatabase::database();
        
        // 开始事务
        db.transaction();
        
        QSqlQuery query;
        query.prepare("INSERT INTO users (username, email, age) VALUES (?, ?, ?)");
        
        for (const QVariantMap &record : records) {
            query.addBindValue(record["username"]);
            query.addBindValue(record["email"]);
            query.addBindValue(record["age"]);
            
            if (!query.exec()) {
                qDebug() << "Insert failed:" << query.lastError().text();
                db.rollback();
                return;
            }
        }
        
        // 提交事务
        db.commit();
        
        qDebug() << "Batch insert completed:" << records.size() << "records";
    }
    
    // ✅ 使用 execBatch（更快）
    static void batchInsertOptimized(const QList<QVariantMap> &records) {
        QSqlDatabase db = QSqlDatabase::database();
        db.transaction();
        
        QSqlQuery query;
        query.prepare("INSERT INTO users (username, email, age) VALUES (?, ?, ?)");
        
        QVariantList usernames, emails, ages;
        
        for (const QVariantMap &record : records) {
            usernames << record["username"];
            emails << record["email"];
            ages << record["age"];
        }
        
        query.addBindValue(usernames);
        query.addBindValue(emails);
        query.addBindValue(ages);
        
        if (!query.execBatch()) {
            qDebug() << "Batch insert failed:" << query.lastError().text();
            db.rollback();
            return;
        }
        
        db.commit();
        qDebug() << "Optimized batch insert completed";
    }
};
```

### 12.3 连接优化

```cpp
class ConnectionOptimization {
public:
    // ✅ 使用连接池（见前面的 MySQLConnectionPool）
    
    // ✅ 启用 SQLite 性能优化
    static void optimizeSQLite() {
        QSqlQuery query;
        
        // 使用 WAL 模式
        query.exec("PRAGMA journal_mode = WAL");
        
        // 增加缓存大小（单位：页，默认 2000）
        query.exec("PRAGMA cache_size = 10000");
        
        // 同步模式（NORMAL 比 FULL 快，但略有风险）
        query.exec("PRAGMA synchronous = NORMAL");
        
        // 临时文件存储在内存中
        query.exec("PRAGMA temp_store = MEMORY");
        
        // 启用内存映射 I/O
        query.exec("PRAGMA mmap_size = 268435456");  // 256MB
        
        qDebug() << "SQLite optimized";
    }
    
    // ✅ MySQL 连接优化
    static void optimizeMySQL() {
        QSqlDatabase db = QSqlDatabase::database();
        
        // 设置连接选项
        db.setConnectOptions(
            "MYSQL_OPT_RECONNECT=1;"
            "MYSQL_OPT_CONNECT_TIMEOUT=10;"
            "CLIENT_COMPRESS=1"
        );
    }
};
```

### 12.4 查询优化技巧

```cpp
class QueryOptimization {
public:
    // ✅ 使用 LIMIT 限制结果集
    static void useLimitOffset() {
        QSqlQuery query;
        
        // 分页查询
        int page = 1;
        int pageSize = 20;
        int offset = (page - 1) * pageSize;
        
        query.prepare("SELECT * FROM users ORDER BY id LIMIT ? OFFSET ?");
        query.addBindValue(pageSize);
        query.addBindValue(offset);
        query.exec();
    }
    
    // ✅ 只查询需要的列
    static void selectSpecificColumns() {
        // ❌ 不好：查询所有列
        // SELECT * FROM users
        
        // ✅ 好：只查询需要的列
        QSqlQuery query("SELECT id, username, email FROM users");
    }
    
    // ✅ 使用 EXISTS 代替 IN（大数据集）
    static void useExists() {
        // ❌ 不好：IN 子查询
        // SELECT * FROM users WHERE id IN (SELECT user_id FROM posts)
        
        // ✅ 好：EXISTS
        QSqlQuery query(R"(
            SELECT * FROM users u
            WHERE EXISTS (SELECT 1 FROM posts p WHERE p.user_id = u.id)
        )");
    }
    
    // ✅ 避免在 WHERE 子句中使用函数
    static void avoidFunctionsInWhere() {
        // ❌ 不好：无法使用索引
        // SELECT * FROM users WHERE LOWER(username) = 'alice'
        
        // ✅ 好：可以使用索引
        QSqlQuery query("SELECT * FROM users WHERE username = 'alice'");
    }
};
```

---

## 13. 快速参考

### 13.1 数据库驱动对比

| 特性 | SQLite | MySQL | PostgreSQL |
|------|--------|-------|------------|
| **类型** | 嵌入式 | 客户端-服务器 | 客户端-服务器 |
| **并发** | 读并发好，写串行 | 高并发 | 高并发 |
| **事务** | 支持 | 支持 | 支持（更强） |
| **全文搜索** | FTS5 | FULLTEXT | tsvector |
| **JSON** | JSON1 扩展 | JSON 类型 | JSONB 类型 |
| **数组** | 不支持 | 不支持 | 支持 |
| **触发器** | 支持 | 支持 | 支持 |
| **存储过程** | 不支持 | 支持 | 支持 |
| **适用场景** | 移动/桌面应用 | Web 应用 | 复杂查询/分析 |

### 13.2 常用 SQL 操作

```sql
-- 创建表
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL,
    email TEXT UNIQUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 插入数据
INSERT INTO users (username, email) VALUES ('alice', 'alice@example.com');

-- 查询数据
SELECT * FROM users WHERE age >= 18 ORDER BY created_at DESC LIMIT 10;

-- 更新数据
UPDATE users SET email = 'newemail@example.com' WHERE id = 1;

-- 删除数据
DELETE FROM users WHERE id = 1;

-- 创建索引
CREATE INDEX idx_username ON users(username);

-- 事务
BEGIN TRANSACTION;
INSERT INTO users (username) VALUES ('bob');
COMMIT;
-- 或 ROLLBACK;
```

### 13.3 Qt SQL 类速查

```cpp
// 数据库连接
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
db.setDatabaseName("mydb.db");
db.open();

// 执行查询
QSqlQuery query;
query.exec("SELECT * FROM users");
while (query.next()) {
    QString name = query.value(0).toString();
}

// 预处理语句
query.prepare("INSERT INTO users (name) VALUES (?)");
query.addBindValue("Alice");
query.exec();

// 事务
db.transaction();
// ... 执行操作
db.commit();  // 或 db.rollback();

// 表模型
QSqlTableModel *model = new QSqlTableModel();
model->setTable("users");
model->select();

// 查询模型
QSqlQueryModel *model = new QSqlQueryModel();
model->setQuery("SELECT * FROM users");
```

---

## 14. 最佳实践

### 14.1 安全性

```cpp
// ✅ 1. 始终使用预处理语句防止 SQL 注入
void safeQuery(const QString &username) {
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE username = ?");
    query.addBindValue(username);  // 安全
    query.exec();
}

void unsafeQuery(const QString &username) {
    // ❌ 危险：SQL 注入风险
    QSqlQuery query;
    query.exec("SELECT * FROM users WHERE username = '" + username + "'");
}

// ✅ 2. 不要在代码中硬编码密码
// 使用配置文件或环境变量

// ✅ 3. 加密敏感数据
// 密码应该使用哈希（如 bcrypt、Argon2）

// ✅ 4. 限制数据库用户权限
// 应用程序使用的数据库用户应该只有必要的权限
```

### 14.2 错误处理

```cpp
// ✅ 始终检查错误
QSqlQuery query;
if (!query.exec("SELECT * FROM users")) {
    qDebug() << "Query failed:" << query.lastError().text();
    qDebug() << "Database error:" << query.lastError().databaseText();
    qDebug() << "Driver error:" << query.lastError().driverText();
    return;
}

// ✅ 检查数据库连接
QSqlDatabase db = QSqlDatabase::database();
if (!db.isValid()) {
    qDebug() << "Database is not valid";
    return;
}

if (!db.isOpen()) {
    qDebug() << "Database is not open";
    return;
}
```

### 14.3 性能建议

1. **使用事务**：批量操作时使用事务可以大幅提升性能
2. **创建索引**：为经常查询的列创建索引
3. **限制结果集**：使用 LIMIT 限制返回的行数
4. **只查询需要的列**：避免 SELECT *
5. **使用连接池**：复用数据库连接
6. **启用 WAL 模式**：SQLite 使用 WAL 模式提高并发性能
7. **批量操作**：使用 execBatch() 进行批量插入
8. **预处理语句**：重复执行的查询使用预处理语句

---

## 结语

Qt 的 SQL 模块提供了强大而灵活的数据库编程接口，支持多种主流数据库系统。

**关键要点：**

1. ✅ **选择合适的数据库**：SQLite 适合嵌入式，MySQL/PostgreSQL 适合服务器
2. ✅ **使用预处理语句**：防止 SQL 注入，提高性能
3. ✅ **事务处理**：保证数据一致性，提高批量操作性能
4. ✅ **错误处理**：始终检查操作结果和错误信息
5. ✅ **性能优化**：创建索引、使用连接池、批量操作
6. ✅ **QML 集成**：通过 C++ 类暴露数据库功能给 QML

**最佳实践：**
- 使用预处理语句防止 SQL 注入
- 批量操作使用事务
- 为常用查询创建索引
- 复用数据库连接
- 完善的错误处理
- 敏感数据加密存储

---

*文档版本：1.0*  
*最后更新：2025-11-04*  
*适用于：Qt 6.x*  
*作者：Kiro AI Assistant*
