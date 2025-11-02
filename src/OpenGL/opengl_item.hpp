#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include "render.hpp"
#include <QTime>
#include <QQuickItem>
#include <QBasicTimer>

class MyRenderer : public Render
{
public:
    MyRenderer(){}
    ~MyRenderer(){}

    void render() {
        // rgba
        glClearColor( 0.1,0.1, 0.5, 1.0 );
        // 当前缓冲区颜色设置为刚才设置的清除颜色
        glClear( GL_COLOR_BUFFER_BIT );
    }
};

class OpenGLItem : public QQuickItem
{
    Q_OBJECT

public:
    OpenGLItem();
    ~OpenGLItem();

public slots:
    void sync();
    void cleanup();
protected:
    void timeEvent( QTimerEvent* e );
private:
    QBasicTimer m_timer;
    Render *m_render;

};

#endif
