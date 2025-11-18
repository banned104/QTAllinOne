#pragma once

#include <QObject>
// #include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QPainter>
#include <QtQml/qqmlregistration.h>

class CppPainter : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT     // 自动注册为QML组件

public:
    explicit CppPainter( QQuickPaintedItem* parent = nullptr )
        :QQuickPaintedItem(parent)
    {
        qDebug() << "Create Painter";
    }

signals:
private:
    QPoint trianglePos = {0,0};
    void paint(QPainter* painter) override {
        qDebug() << "Painting";
        painter->drawText( QPoint(50, 50), "haha" );
    }
};
