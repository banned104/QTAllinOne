#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include "triangle_render.hpp"
#include <QTime>
#include <QQuickItem>
#include <QBasicTimer>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class MyRenderer : public Render
{
public:
    MyRenderer(){}
    ~MyRenderer(){}

    void render() {
        // rgba
        glClearColor( 0.0, 0.0, 0.5, 1.0 );
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
    MyRenderer *m_render;

    QOpenGLShaderProgram* m_program;
    QOpenGLBuffer* m_vbo;
    QMatrix4x4 m_projection;

};

#endif
