# QQuickItem C++ API 详解

> 本文档详细介绍 Qt6 中 QQuickItem 的 C++ API，包括类的使用、自定义 Item、渲染机制、事件处理等，帮助开发者在 C++ 层面深入理解和扩展 QML 可视化元素。

## 目录

1. [QQuickItem 概述](#1-quickitem-概述)
2. [基础 API](#2-基础-api)
3. [几何与布局](#3-几何与布局)
4. [变换系统](#4-变换系统)
5. [事件处理](#5-事件处理)
6. [焦点管理](#6-焦点管理)
7. [渲染与绘制](#7-渲染与绘制)
8. [自定义 QQuickItem](#8-自定义-quickitem)
9. [场景图集成](#9-场景图集成)
10. [性能优化](#10-性能优化)
11. [实战案例](#11-实战案例)

---

## 1. QQuickItem 概述

### 1.1 类层次结构

```cpp
QObject
    ↓
QQuickItem
    ↓
QQuickRectangle, QQuickText, QQuickImage, etc.
```

### 1.2 头文件和链接

```cpp
// 头文件
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickView>

// CMakeLists.txt
find_package(Qt6 REQUIRED COMPONENTS Quick)
target_link_libraries(myapp PRIVATE Qt6::Quick)

// .pro 文件
QT += quick
```

### 1.3 基本使用

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    
    // 获取根对象
    QObject *rootObject = engine.rootObjects().first();
    QQuickWindow *window = qobject_cast<QQuickWindow*>(rootObject);
    
    if (window) {
        // 获取 contentItem（根 Item）
        QQuickItem *contentItem = window->contentItem();
        
        // 查找子 Item
        QQuickItem *myItem = contentItem->findChild<QQuickItem*>("myItem");
        
        if (myItem) {
            // 操作 Item
            myItem->setWidth(200);
            myItem->setHeight(150);
            myItem->setPosition(QPointF(100, 100));
        }
    }
    
    return app.exec();
}
```

---

## 2. 基础 API

### 2.1 几何属性

```cpp
#include <QQuickItem>

class ItemGeometry : public QObject
{
    Q_OBJECT
    
public:
    void demonstrateGeometry(QQuickItem *item)
    {
        // ========== 位置 ==========
        // 获取位置
        QPointF pos = item->position();
        qreal x = item->x();
        qreal y = item->y();
        
        // 设置位置
        item->setPosition(QPointF(100, 100));
        item->setX(100);
        item->setY(100);
        
        // ========== 尺寸 ==========
        // 获取尺寸
        QSizeF size = item->size();
        qreal width = item->width();
        qreal height = item->height();
        
        // 设置尺寸
        item->setSize(QSizeF(200, 150));
        item->setWidth(200);
        item->setHeight(150);
        
        // ========== 隐式尺寸 ==========
        qreal implicitWidth = item->implicitWidth();
        qreal implicitHeight = item->implicitHeight();
        
        item->setImplicitWidth(180);
        item->setImplicitHeight(120);
        
        // ========== 边界矩形 ==========
        QRectF boundingRect = item->boundingRect();
        QRectF childrenRect = item->childrenRect();
        
        qDebug() << "位置:" << pos;
        qDebug() << "尺寸:" << size;
        qDebug() << "边界:" << boundingRect;
        qDebug() << "子项边界:" << childrenRect;
    }
};
```

### 2.2 可见性和状态

```cpp
class ItemVisibility : public QObject
{
    Q_OBJECT
    
public:
    void demonstrateVisibility(QQuickItem *item)
    {
        // ========== 可见性 ==========
        bool visible = item->isVisible();
        item->setVisible(true);
        
        // ========== 启用状态 ==========
        bool enabled = item->isEnabled();
        item->setEnabled(true);
        
        // ========== 不透明度 ==========
        qreal opacity = item->opacity();
        item->setOpacity(0.8);
        
        // ========== 裁剪 ==========
        bool clip = item->clip();
        item->setClip(true);  // 裁剪超出边界的子项
        
        // ========== Z 顺序 ==========
        qreal z = item->z();
        item->setZ(1.0);  // 提升层级
        
        // ========== 平滑处理 ==========
        bool smooth = item->smooth();
        item->setSmooth(true);  // 启用平滑缩放
        
        // ========== 抗锯齿 ==========
        bool antialiasing = item->antialiasing();
        item->setAntialiasing(true);
    }
};
```

### 2.3 父子关系

```cpp
class ItemHierarchy : public QObject
{
    Q_OBJECT
    
public:
    void demonstrateHierarchy(QQuickItem *item)
    {
        // ========== 父项 ==========
        QQuickItem *parent = item->parentItem();
        
        // 设置父项
        QQuickItem *newParent = new QQuickItem();
        item->setParentItem(newParent);
        
        // ========== 子项 ==========
        QList<QQuickItem*> children = item->childItems();
        
        qDebug() << "子项数量:" << children.count();
        
        // 遍历子项
        for (QQuickItem *child : children) {
            qDebug() << "子项:" << child->objectName();
        }
        
        // ========== 查找子项 ==========
        // 按名称查找
        QQuickItem *namedChild = item->findChild<QQuickItem*>("childName");
        
        // 查找所有子项
        QList<QQuickItem*> allChildren = item->findChildren<QQuickItem*>();
        
        // ========== 窗口 ==========
        QQuickWindow *window = item->window();
        if (window) {
            qDebug() << "所属窗口:" << window->title();
        }
    }
};
```

---

## 3. 几何与布局

### 3.1 坐标映射

```cpp
#include <QQuickItem>
#include <QPointF>
#include <QRectF>

class CoordinateMapping : public QObject
{
    Q_OBJECT
    
public:
    void demonstrateMapping(QQuickItem *item)
    {
        QPointF localPoint(50, 50);
        
        // ========== 映射到父项 ==========
        QPointF parentPoint = item->mapToItem(item->parentItem(), localPoint);
        qDebug() << "父项坐标:" << parentPoint;
        
        // ========== 映射到场景（窗口）==========
        QPointF scenePoint = item->mapToScene(localPoint);
        qDebug() << "场景坐标:" << scenePoint;
        
        // ========== 映射到全局 ==========
        QPointF globalPoint = item->mapToGlobal(localPoint);
        qDebug() << "全局坐标:" << globalPoint;
        
        // ========== 从其他项映射 ==========
        QQuickItem *otherItem = item->parentItem();
        if (otherItem) {
            QPointF fromOther = item->mapFromItem(otherItem, QPointF(0, 0));
            qDebug() << "从其他项映射:" << fromOther;
        }
        
        // ========== 矩形映射 ==========
        QRectF localRect(0, 0, 100, 100);
        QRectF mappedRect = item->mapRectToItem(item->parentItem(), localRect);
        qDebug() << "映射矩形:" << mappedRect;
        
        // ========== 从场景映射 ==========
        QPointF fromScene = item->mapFromScene(QPointF(200, 200));
        qDebug() << "从场景映射:" << fromScene;
        
        // ========== 从全局映射 ==========
        QPointF fromGlobal = item->mapFromGlobal(QPointF(500, 500));
        qDebug() << "从全局映射:" << fromGlobal;
    }
};
```

### 3.2 锚点系统（C++ 访问）

```cpp
#include <QQuickItem>
#include <QQuickAnchors>

class AnchorAccess : public QObject
{
    Q_OBJECT
    
public:
    void demonstrateAnchors(QQuickItem *item)
    {
        // 获取锚点对象
        QQuickAnchors *anchors = item->anchors();
        
        if (anchors) {
            // ========== 读取锚点 ==========
            QQuickItem *leftAnchor = anchors->left().item;
            QQuickAnchors::Anchor leftLine = anchors->left().anchorLine;
            
            // ========== 设置锚点（通过 QML 属性）==========
            // 注意：C++ 中直接设置锚点比较复杂
            // 通常通过 QML 或属性系统设置
            
            // 获取边距
            qreal leftMargin = anchors->leftMargin();
            qreal topMargin = anchors->topMargin();
            qreal margins = anchors->margins();
            
            // 设置边距
            anchors->setLeftMargin(10);
            anchors->setTopMargin(10);
            anchors->setMargins(20);  // 统一边距
            
            qDebug() << "左边距:" << leftMargin;
            qDebug() << "上边距:" << topMargin;
        }
    }
};
```

### 3.3 包含测试

```cpp
class ContainmentTest : public QObject
{
    Q_OBJECT
    
public:
    void demonstrateContainment(QQuickItem *item)
    {
        QPointF point(50, 50);
        
        // ========== 包含测试 ==========
        // 测试点是否在项内
        bool contains = item->contains(point);
        qDebug() << "包含点:" << contains;
        
        // ========== 子项包含测试 ==========
        QQuickItem *childAtPoint = item->childAt(point.x(), point.y());
        if (childAtPoint) {
            qDebug() << "该点的子项:" << childAtPoint->objectName();
        }
    }
};
```

---

## 4. 变换系统

### 4.1 基础变换

```cpp
#include <QQuickItem>
#include <QTransform>

class ItemTransform : public QObject
{
    Q_OBJECT
    
public:
    void demonstrateTransform(QQuickItem *item)
    {
        // ========== 旋转 ==========
        qreal rotation = item->rotation();
        item->setRotation(45.0);  // 旋转 45 度
        
        // ========== 缩放 ==========
        qreal scale = item->scale();
        item->setScale(1.5);  // 放大 1.5 倍
        
        // ========== 变换原点 ==========
        QPointF transformOrigin = item->transformOriginPoint();
        
        // 设置变换原点（像素坐标）
        item->setTransformOriginPoint(QPointF(50, 50));
        
        // ========== 获取变换矩阵 ==========
        QTransform transform = item->itemTransform(item->parentItem(), nullptr);
        qDebug() << "变换矩阵:" << transform;
        
        qDebug() << "旋转:" << rotation;
        qDebug() << "缩放:" << scale;
        qDebug() << "变换原点:" << transformOrigin;
    }
};
```

### 4.2 高级变换

```cpp
#include <QQuickItem>
#include <QQuickTransform>

class AdvancedTransform : public QObject
{
    Q_OBJECT
    
public:
    void demonstrateAdvancedTransform(QQuickItem *item)
    {
        // ========== 获取变换列表 ==========
        QQmlListProperty<QQuickTransform> transforms = item->transform();
        
        // 注意：QQmlListProperty 主要用于 QML
        // C++ 中通常直接操作基础变换属性
        
        // ========== 组合变换 ==========
        // 设置多个变换
        item->setRotation(30);
        item->setScale(1.2);
        item->setX(item->x() + 50);  // 平移
        
        // ========== 获取最终变换 ==========
        bool ok;
        QTransform finalTransform = item->itemTransform(nullptr, &ok);
        
        if (ok) {
            qDebug() << "最终变换矩阵:" << finalTransform;
            
            // 应用变换到点
            QPointF originalPoint(0, 0);
            QPointF transformedPoint = finalTransform.map(originalPoint);
            qDebug() << "变换后的点:" << transformedPoint;
        }
    }
};
```

---

## 5. 事件处理

### 5.1 鼠标事件

```cpp
#include <QQuickItem>
#include <QMouseEvent>

class CustomMouseItem : public QQuickItem
{
    Q_OBJECT
    
public:
    CustomMouseItem(QQuickItem *parent = nullptr)
        : QQuickItem(parent)
    {
        // 启用鼠标事件接收
        setAcceptedMouseButtons(Qt::AllButtons);
        setAcceptHoverEvents(true);
    }
    
protected:
    // ========== 鼠标按下 ==========
    void mousePressEvent(QMouseEvent *event) override
    {
        qDebug() << "鼠标按下:" << event->pos();
        qDebug() << "按钮:" << event->button();
        qDebug() << "修饰键:" << event->modifiers();
        
        // 接受事件（阻止传播）
        event->accept();
        
        // 或调用基类实现
        // QQuickItem::mousePressEvent(event);
    }
    
    // ========== 鼠标释放 ==========
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        qDebug() << "鼠标释放:" << event->pos();
        event->accept();
    }
    
    // ========== 鼠标移动 ==========
    void mouseMoveEvent(QMouseEvent *event) override
    {
        qDebug() << "鼠标移动:" << event->pos();
        
        // 可以实现拖拽
        if (event->buttons() & Qt::LeftButton) {
            // 拖拽逻辑
            QPointF delta = event->pos() - m_lastMousePos;
            setPosition(position() + delta);
        }
        
        m_lastMousePos = event->pos();
        event->accept();
    }
    
    // ========== 鼠标双击 ==========
    void mouseDoubleClickEvent(QMouseEvent *event) override
    {
        qDebug() << "鼠标双击:" << event->pos();
        event->accept();
    }
    
    // ========== 鼠标进入 ==========
    void hoverEnterEvent(QHoverEvent *event) override
    {
        qDebug() << "鼠标进入";
        Q_UNUSED(event)
    }
    
    // ========== 鼠标移动（悬停）==========
    void hoverMoveEvent(QHoverEvent *event) override
    {
        qDebug() << "鼠标悬停移动:" << event->pos();
        Q_UNUSED(event)
    }
    
    // ========== 鼠标离开 ==========
    void hoverLeaveEvent(QHoverEvent *event) override
    {
        qDebug() << "鼠标离开";
        Q_UNUSED(event)
    }
    
    // ========== 滚轮事件 ==========
    void wheelEvent(QWheelEvent *event) override
    {
        qDebug() << "滚轮:" << event->angleDelta();
        
        // 缩放示例
        qreal delta = event->angleDelta().y() / 120.0;
        setScale(scale() + delta * 0.1);
        
        event->accept();
    }
    
private:
    QPointF m_lastMousePos;
};
```

### 5.2 键盘事件

```cpp
#include <QQuickItem>
#include <QKeyEvent>

class CustomKeyItem : public QQuickItem
{
    Q_OBJECT
    
public:
    CustomKeyItem(QQuickItem *parent = nullptr)
        : QQuickItem(parent)
    {
        // 启用焦点
        setFlag(QQuickItem::ItemIsFocusScope, true);
        setFocus(true);
    }
    
protected:
    // ========== 按键按下 ==========
    void keyPressEvent(QKeyEvent *event) override
    {
        qDebug() << "按键按下:" << event->key();
        qDebug() << "文本:" << event->text();
        qDebug() << "修饰键:" << event->modifiers();
        
        // 处理特定按键
        switch (event->key()) {
            case Qt::Key_Up:
                setY(y() - 10);
                event->accept();
                break;
                
            case Qt::Key_Down:
                setY(y() + 10);
                event->accept();
                break;
                
            case Qt::Key_Left:
                setX(x() - 10);
                event->accept();
                break;
                
            case Qt::Key_Right:
                setX(x() + 10);
                event->accept();
                break;
                
            case Qt::Key_Escape:
                qDebug() << "ESC 键";
                event->accept();
                break;
                
            default:
                // 未处理的按键传递给基类
                QQuickItem::keyPressEvent(event);
                break;
        }
        
        // 检查修饰键
        if (event->modifiers() & Qt::ControlModifier) {
            qDebug() << "Ctrl 键被按下";
        }
    }
    
    // ========== 按键释放 ==========
    void keyReleaseEvent(QKeyEvent *event) override
    {
        qDebug() << "按键释放:" << event->key();
        event->accept();
    }
};
```

### 5.3 触摸事件

```cpp
#include <QQuickItem>
#include <QTouchEvent>

class CustomTouchItem : public QQuickItem
{
    Q_OBJECT
    
public:
    CustomTouchItem(QQuickItem *parent = nullptr)
        : QQuickItem(parent)
    {
        // 启用触摸事件
        setAcceptTouchEvents(true);
    }
    
protected:
    // ========== 触摸事件 ==========
    void touchEvent(QTouchEvent *event) override
    {
        qDebug() << "触摸事件类型:" << event->type();
        qDebug() << "触摸点数量:" << event->points().count();
        
        // 遍历触摸点
        for (const QEventPoint &touchPoint : event->points()) {
            qDebug() << "触摸点 ID:" << touchPoint.id();
            qDebug() << "位置:" << touchPoint.position();
            qDebug() << "状态:" << touchPoint.state();
            
            // 触摸点状态
            switch (touchPoint.state()) {
                case QEventPoint::Pressed:
                    qDebug() << "触摸开始";
                    break;
                case QEventPoint::Updated:
                    qDebug() << "触摸移动";
                    break;
                case QEventPoint::Released:
                    qDebug() << "触摸结束";
                    break;
                default:
                    break;
            }
        }
        
        // 双指缩放检测
        if (event->points().count() == 2) {
            const QEventPoint &p1 = event->points()[0];
            const QEventPoint &p2 = event->points()[1];
            
            QPointF delta = p2.position() - p1.position();
            qreal distance = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
            
            qDebug() << "双指距离:" << distance;
        }
        
        event->accept();
    }
};
```

---

## 6. 焦点管理

### 6.1 焦点 API

```cpp
#include <QQuickItem>

class FocusManagement : public QObject
{
    Q_OBJECT
    
public:
    void demonstrateFocus(QQuickItem *item)
    {
        // ========== 焦点状态 ==========
        bool hasFocus = item->hasFocus();
        bool hasActiveFocus = item->hasActiveFocus();
        
        qDebug() << "有焦点:" << hasFocus;
        qDebug() << "有激活焦点:" << hasActiveFocus;
        
        // ========== 设置焦点 ==========
        item->setFocus(true);
        
        // 强制激活焦点
        item->forceActiveFocus();
        
        // 指定焦点原因
        item->forceActiveFocus(Qt::MouseFocusReason);
        
        // ========== 焦点作用域 ==========
        bool isFocusScope = item->isFocusScope();
        item->setFlag(QQuickItem::ItemIsFocusScope, true);
        
        // ========== 焦点策略 ==========
        Qt::FocusPolicy policy = item->focusPolicy();
        item->setFocusPolicy(Qt::StrongFocus);
        
        // 焦点策略选项：
        // Qt::NoFocus        - 不接受焦点
        // Qt::TabFocus       - Tab 键焦点
        // Qt::ClickFocus     - 点击焦点
        // Qt::StrongFocus    - Tab 和点击焦点
        // Qt::WheelFocus     - 滚轮焦点
        
        // ========== 下一个/上一个焦点项 ==========
        QQuickItem *nextInFocusChain = item->nextItemInFocusChain();
        QQuickItem *prevInFocusChain = item->nextItemInFocusChain(false);
        
        qDebug() << "下一个焦点项:" << nextInFocusChain;
        qDebug() << "上一个焦点项:" << prevInFocusChain;
    }
};
```

### 6.2 焦点事件

```cpp
class CustomFocusItem : public QQuickItem
{
    Q_OBJECT
    
public:
    CustomFocusItem(QQuickItem *parent = nullptr)
        : QQuickItem(parent)
    {
        setFlag(QQuickItem::ItemIsFocusScope, true);
        setFocusPolicy(Qt::StrongFocus);
    }
    
protected:
    // ========== 焦点进入 ==========
    void focusInEvent(QFocusEvent *event) override
    {
        qDebug() << "焦点进入";
        qDebug() << "焦点原因:" << event->reason();
        
        // 焦点原因：
        // Qt::MouseFocusReason    - 鼠标点击
        // Qt::TabFocusReason      - Tab 键
        // Qt::BacktabFocusReason  - Shift+Tab
        // Qt::ActiveWindowFocusReason - 窗口激活
        // Qt::PopupFocusReason    - 弹出窗口
        // Qt::ShortcutFocusReason - 快捷键
        // Qt::MenuBarFocusReason  - 菜单栏
        // Qt::OtherFocusReason    - 其他
        
        QQuickItem::focusInEvent(event);
    }
    
    // ========== 焦点离开 ==========
    void focusOutEvent(QFocusEvent *event) override
    {
        qDebug() << "焦点离开";
        qDebug() << "焦点原因:" << event->reason();
        
        QQuickItem::focusOutEvent(event);
    }
};
```



---

## 7. 渲染与绘制

### 7.1 更新机制

```cpp
#include <QQuickItem>

class RenderUpdate : public QObject
{
    Q_OBJECT
    
public:
    void demonstrateUpdate(QQuickItem *item)
    {
        // ========== 请求更新 ==========
        // 标记项需要重绘
        item->update();
        
        // 更新特定区域
        QRectF dirtyRect(0, 0, 100, 100);
        item->update(dirtyRect);
        
        // ========== 标记脏区 ==========
        // 标记项的几何已改变
        item->polish();
        
        // ========== 窗口更新 ==========
        QQuickWindow *window = item->window();
        if (window) {
            // 请求窗口更新
            window->update();
        }
    }
};
```

### 7.2 场景图节点

```cpp
#include <QQuickItem>
#include <QSGNode>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>

class CustomRenderItem : public QQuickItem
{
    Q_OBJECT
    
public:
    CustomRenderItem(QQuickItem *parent = nullptr)
        : QQuickItem(parent)
    {
        // 启用自定义渲染
        setFlag(QQuickItem::ItemHasContents, true);
    }
    
protected:
    // ========== 更新绘制节点 ==========
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override
    {
        Q_UNUSED(data)
        
        // 创建或重用节点
        QSGGeometryNode *node = static_cast<QSGGeometryNode*>(oldNode);
        
        if (!node) {
            // 创建新节点
            node = new QSGGeometryNode();
            
            // 创建几何体（矩形）
            QSGGeometry *geometry = new QSGGeometry(
                QSGGeometry::defaultAttributes_Point2D(), 4);
            geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
            
            node->setGeometry(geometry);
            node->setFlag(QSGNode::OwnsGeometry);
            
            // 创建材质
            QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
            material->setColor(QColor(64, 128, 255));
            
            node->setMaterial(material);
            node->setFlag(QSGNode::OwnsMaterial);
        }
        
        // 更新几何体
        QSGGeometry *geometry = node->geometry();
        QSGGeometry::Point2D *vertices = geometry->vertexDataAsPoint2D();
        
        // 设置矩形顶点
        vertices[0].set(0, 0);
        vertices[1].set(width(), 0);
        vertices[2].set(0, height());
        vertices[3].set(width(), height());
        
        // 标记几何体已更新
        node->markDirty(QSGNode::DirtyGeometry);
        
        return node;
    }
};
```

### 7.3 纹理渲染

```cpp
#include <QQuickItem>
#include <QSGTexture>
#include <QSGSimpleTextureNode>
#include <QImage>

class TextureRenderItem : public QQuickItem
{
    Q_OBJECT
    
public:
    TextureRenderItem(QQuickItem *parent = nullptr)
        : QQuickItem(parent)
    {
        setFlag(QQuickItem::ItemHasContents, true);
    }
    
    void setTexture(const QImage &image)
    {
        m_image = image;
        update();
    }
    
protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override
    {
        Q_UNUSED(data)
        
        QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode*>(oldNode);
        
        if (!node) {
            node = new QSGSimpleTextureNode();
        }
        
        // 创建纹理
        if (!m_image.isNull()) {
            QSGTexture *texture = window()->createTextureFromImage(m_image);
            node->setTexture(texture);
            node->setOwnsTexture(true);
            
            // 设置纹理矩形
            node->setRect(boundingRect());
            
            // 设置纹理坐标
            node->setSourceRect(QRectF(0, 0, 1, 1));
            
            // 设置过滤模式
            node->setFiltering(QSGTexture::Linear);
        }
        
        return node;
    }
    
private:
    QImage m_image;
};
```

---

## 8. 自定义 QQuickItem

### 8.1 完整的自定义 Item

```cpp
// customitem.h
#ifndef CUSTOMITEM_H
#define CUSTOMITEM_H

#include <QQuickItem>
#include <QColor>

class CustomItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    
public:
    explicit CustomItem(QQuickItem *parent = nullptr);
    
    // 属性访问器
    QColor color() const { return m_color; }
    void setColor(const QColor &color);
    
    qreal borderWidth() const { return m_borderWidth; }
    void setBorderWidth(qreal width);
    
signals:
    void colorChanged();
    void borderWidthChanged();
    void clicked();
    
protected:
    // 事件处理
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    
    // 渲染
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;
    
private:
    QColor m_color;
    qreal m_borderWidth;
    bool m_pressed;
};

#endif // CUSTOMITEM_H
```

```cpp
// customitem.cpp
#include "customitem.h"
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>
#include <QMouseEvent>

CustomItem::CustomItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_color(Qt::blue)
    , m_borderWidth(2.0)
    , m_pressed(false)
{
    // 启用内容渲染
    setFlag(QQuickItem::ItemHasContents, true);
    
    // 接受鼠标事件
    setAcceptedMouseButtons(Qt::LeftButton);
}

void CustomItem::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged();
        update();  // 请求重绘
    }
}

void CustomItem::setBorderWidth(qreal width)
{
    if (!qFuzzyCompare(m_borderWidth, width)) {
        m_borderWidth = width;
        emit borderWidthChanged();
        update();
    }
}

void CustomItem::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    update();
    event->accept();
}

void CustomItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_pressed) {
        m_pressed = false;
        emit clicked();
        update();
    }
    event->accept();
}

void CustomItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    // 几何改变时调用
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    update();
}

QSGNode *CustomItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_UNUSED(data)
    
    QSGGeometryNode *node = static_cast<QSGGeometryNode*>(oldNode);
    
    if (!node) {
        node = new QSGGeometryNode();
        
        QSGGeometry *geometry = new QSGGeometry(
            QSGGeometry::defaultAttributes_Point2D(), 4);
        geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
        
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);
        
        QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial);
    }
    
    // 更新颜色
    QSGFlatColorMaterial *material = static_cast<QSGFlatColorMaterial*>(node->material());
    QColor color = m_pressed ? m_color.darker(120) : m_color;
    material->setColor(color);
    
    // 更新几何体
    QSGGeometry *geometry = node->geometry();
    QSGGeometry::Point2D *vertices = geometry->vertexDataAsPoint2D();
    
    qreal w = width();
    qreal h = height();
    qreal b = m_borderWidth;
    
    vertices[0].set(b, b);
    vertices[1].set(w - b, b);
    vertices[2].set(b, h - b);
    vertices[3].set(w - b, h - b);
    
    node->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
    
    return node;
}
```

### 8.2 注册到 QML

```cpp
// main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "customitem.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // ========== 注册自定义类型 ==========
    qmlRegisterType<CustomItem>("MyComponents", 1, 0, "CustomItem");
    
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    
    return app.exec();
}
```

```qml
// main.qml
import QtQuick
import MyComponents 1.0

Window {
    width: 800
    height: 600
    visible: true
    
    CustomItem {
        anchors.centerIn: parent
        width: 200
        height: 150
        color: "#4080ff"
        borderWidth: 5
        
        onClicked: {
            console.log("自定义 Item 被点击")
        }
    }
}
```

---

## 9. 场景图集成

### 9.1 场景图信号

```cpp
#include <QQuickItem>
#include <QQuickWindow>

class SceneGraphIntegration : public QObject
{
    Q_OBJECT
    
public:
    void connectSceneGraphSignals(QQuickItem *item)
    {
        QQuickWindow *window = item->window();
        if (!window) return;
        
        // ========== 场景图信号 ==========
        
        // 场景图初始化
        connect(window, &QQuickWindow::sceneGraphInitialized, this, []() {
            qDebug() << "场景图已初始化";
        });
        
        // 场景图失效
        connect(window, &QQuickWindow::sceneGraphInvalidated, this, []() {
            qDebug() << "场景图已失效";
        });
        
        // 渲染前
        connect(window, &QQuickWindow::beforeRendering, this, []() {
            qDebug() << "渲染前";
        }, Qt::DirectConnection);
        
        // 渲染后
        connect(window, &QQuickWindow::afterRendering, this, []() {
            qDebug() << "渲染后";
        }, Qt::DirectConnection);
        
        // 同步前
        connect(window, &QQuickWindow::beforeSynchronizing, this, []() {
            qDebug() << "同步前";
        }, Qt::DirectConnection);
        
        // 同步后
        connect(window, &QQuickWindow::afterSynchronizing, this, []() {
            qDebug() << "同步后";
        }, Qt::DirectConnection);
        
        // 帧交换
        connect(window, &QQuickWindow::frameSwapped, this, []() {
            qDebug() << "帧已交换";
        });
    }
};
```

### 9.2 OpenGL 集成

```cpp
#include <QQuickItem>
#include <QQuickWindow>
#include <QOpenGLFunctions>

class OpenGLIntegration : public QQuickItem, protected QOpenGLFunctions
{
    Q_OBJECT
    
public:
    OpenGLIntegration(QQuickItem *parent = nullptr)
        : QQuickItem(parent)
    {
        connect(this, &QQuickItem::windowChanged, this, &OpenGLIntegration::handleWindowChanged);
    }
    
private slots:
    void handleWindowChanged(QQuickWindow *window)
    {
        if (window) {
            // 在渲染前执行 OpenGL 代码
            connect(window, &QQuickWindow::beforeRendering, 
                    this, &OpenGLIntegration::renderOpenGL, 
                    Qt::DirectConnection);
        }
    }
    
    void renderOpenGL()
    {
        if (!m_initialized) {
            initializeOpenGLFunctions();
            m_initialized = true;
        }
        
        // 执行自定义 OpenGL 代码
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        // ... 更多 OpenGL 调用
    }
    
private:
    bool m_initialized = false;
};
```

---

## 10. 性能优化

### 10.1 缓存和层

```cpp
#include <QQuickItem>

class PerformanceOptimization : public QObject
{
    Q_OBJECT
    
public:
    void demonstrateOptimization(QQuickItem *item)
    {
        // ========== 启用层 ==========
        // 将项渲染到离屏纹理
        item->setProperty("layer.enabled", true);
        
        // 设置层的采样模式
        item->setProperty("layer.smooth", true);
        
        // 设置层的纹理大小
        item->setProperty("layer.textureSize", QSize(256, 256));
        
        // ========== 裁剪 ==========
        // 启用裁剪可以提高性能
        item->setClip(true);
        
        // ========== 平滑处理 ==========
        // 根据需要启用/禁用
        item->setSmooth(false);  // 禁用可提高性能
        
        // ========== 抗锯齿 ==========
        item->setAntialiasing(false);  // 禁用可提高性能
        
        // ========== 可见性优化 ==========
        // 不可见时不渲染
        if (!item->isVisible()) {
            // 跳过更新
            return;
        }
    }
};
```

### 10.2 批量更新

```cpp
class BatchUpdate : public QObject
{
    Q_OBJECT
    
public:
    void demonstrateBatchUpdate(QQuickItem *item)
    {
        // ========== 批量更新 ==========
        // 使用 setUpdatesEnabled 暂停更新
        item->setFlag(QQuickItem::ItemHasContents, false);
        
        // 执行多个更新
        item->setX(100);
        item->setY(100);
        item->setWidth(200);
        item->setHeight(150);
        item->setRotation(45);
        
        // 恢复更新
        item->setFlag(QQuickItem::ItemHasContents, true);
        item->update();
    }
};
```

---

## 11. 实战案例

### 11.1 自定义按钮

```cpp
// custombutton.h
#ifndef CUSTOMBUTTON_H
#define CUSTOMBUTTON_H

#include <QQuickItem>
#include <QString>
#include <QColor>

class CustomButton : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(bool pressed READ pressed NOTIFY pressedChanged)
    
public:
    explicit CustomButton(QQuickItem *parent = nullptr);
    
    QString text() const { return m_text; }
    void setText(const QString &text);
    
    QColor backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(const QColor &color);
    
    QColor textColor() const { return m_textColor; }
    void setTextColor(const QColor &color);
    
    bool pressed() const { return m_pressed; }
    
signals:
    void textChanged();
    void backgroundColorChanged();
    void textColorChanged();
    void pressedChanged();
    void clicked();
    
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;
    
private:
    QString m_text;
    QColor m_backgroundColor;
    QColor m_textColor;
    bool m_pressed;
    bool m_hovered;
};

#endif
```

```cpp
// custombutton.cpp
#include "custombutton.h"
#include <QSGSimpleRectNode>
#include <QSGSimpleTextNode>
#include <QMouseEvent>

CustomButton::CustomButton(QQuickItem *parent)
    : QQuickItem(parent)
    , m_text("Button")
    , m_backgroundColor(QColor(64, 128, 255))
    , m_textColor(Qt::white)
    , m_pressed(false)
    , m_hovered(false)
{
    setFlag(QQuickItem::ItemHasContents, true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
    
    // 设置默认大小
    setImplicitWidth(120);
    setImplicitHeight(40);
}

void CustomButton::setText(const QString &text)
{
    if (m_text != text) {
        m_text = text;
        emit textChanged();
        update();
    }
}

void CustomButton::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        emit backgroundColorChanged();
        update();
    }
}

void CustomButton::setTextColor(const QColor &color)
{
    if (m_textColor != color) {
        m_textColor = color;
        emit textColorChanged();
        update();
    }
}

void CustomButton::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    emit pressedChanged();
    update();
    event->accept();
}

void CustomButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_pressed) {
        m_pressed = false;
        emit pressedChanged();
        emit clicked();
        update();
    }
    event->accept();
}

void CustomButton::hoverEnterEvent(QHoverEvent *event)
{
    Q_UNUSED(event)
    m_hovered = true;
    update();
}

void CustomButton::hoverLeaveEvent(QHoverEvent *event)
{
    Q_UNUSED(event)
    m_hovered = false;
    update();
}

QSGNode *CustomButton::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_UNUSED(data)
    
    QSGSimpleRectNode *rectNode = static_cast<QSGSimpleRectNode*>(oldNode);
    
    if (!rectNode) {
        rectNode = new QSGSimpleRectNode();
    }
    
    // 计算颜色（根据状态）
    QColor bgColor = m_backgroundColor;
    if (m_pressed) {
        bgColor = bgColor.darker(120);
    } else if (m_hovered) {
        bgColor = bgColor.lighter(110);
    }
    
    // 设置矩形
    rectNode->setRect(boundingRect());
    rectNode->setColor(bgColor);
    
    // 注意：这里简化了文本渲染
    // 实际应用中应该使用 QSGSimpleTextNode 或 QQuickText
    
    return rectNode;
}
```

### 11.2 使用示例

```cpp
// main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "custombutton.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // 注册自定义按钮
    qmlRegisterType<CustomButton>("MyComponents", 1, 0, "CustomButton");
    
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    
    return app.exec();
}
```

```qml
// main.qml
import QtQuick
import MyComponents 1.0

Window {
    width: 800
    height: 600
    visible: true
    title: "自定义按钮示例"
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        CustomButton {
            text: "点击我"
            backgroundColor: "#4080ff"
            textColor: "white"
            
            onClicked: {
                console.log("按钮被点击")
            }
        }
        
        CustomButton {
            text: "红色按钮"
            backgroundColor: "#ff6b6b"
            textColor: "white"
        }
        
        CustomButton {
            text: "绿色按钮"
            backgroundColor: "#4ecdc4"
            textColor: "white"
        }
    }
}
```

---

## 总结

### 核心知识点

1. **基础 API**
   - 几何属性（位置、尺寸）
   - 可见性和状态
   - 父子关系

2. **几何与布局**
   - 坐标映射
   - 锚点系统
   - 包含测试

3. **变换系统**
   - 基础变换（旋转、缩放）
   - 变换矩阵
   - 变换原点

4. **事件处理**
   - 鼠标事件
   - 键盘事件
   - 触摸事件

5. **焦点管理**
   - 焦点状态
   - 焦点作用域
   - 焦点事件

6. **渲染与绘制**
   - 更新机制
   - 场景图节点
   - 自定义渲染

7. **自定义 Item**
   - 属性定义
   - 事件重写
   - 场景图集成

### 最佳实践

✅ **推荐做法**
- 使用 Q_PROPERTY 暴露属性到 QML
- 正确处理事件并调用 accept()
- 在 updatePaintNode 中高效更新场景图
- 使用信号通知属性变化
- 合理使用层和缓存优化性能

❌ **避免做法**
- 不要在渲染线程执行耗时操作
- 避免频繁创建/销毁场景图节点
- 不要忽略几何变化通知
- 避免在事件处理中执行复杂逻辑

### 参考资源

- **官方文档**: https://doc.qt.io/qt-6/qquickitem.html
- **场景图**: https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html
- **自定义 Item**: https://doc.qt.io/qt-6/qtquick-scenegraph-customgeometry-example.html

---

**祝你在 QQuickItem C++ 开发中取得成功！** 🚀
