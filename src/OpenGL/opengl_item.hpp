#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include "triangle_render.hpp"
#include <QTime>
#include <QQuickItem>
#include <QBasicTimer>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class OpenGLItem : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(int fps READ fps WRITE setFps NOTIFY fpsChanged FINAL)
    Q_PROPERTY(QString renderType READ renderType WRITE setRenderType NOTIFY renderTypeChanged FINAL)

public:
    OpenGLItem();
    ~OpenGLItem() override;

    // int fps()

private:
    void createRenderer();
    void initializeRenderer();
    void updateProjectMatrix();
    void handleRenderError( RenderError error, const std::string& message );

    std::unique_ptr<IRenderer> m_renderer;
    RenderConfig m_config;
    QMatrix4x4 m_projectionMatrix;

    int m_fps;
    QTime m_lastTime;
    QBasicTimer m_timer;
    QString m_rendererType;
    quint64 m_frameNumer;

    bool m_rendererInitialized;
};

#endif
