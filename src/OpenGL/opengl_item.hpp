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

    int fps() const { return m_fps; }
    void setFps( int f );

    QString renderType() const { return m_rendererType; }
    void setRenderType( const QString& type );

    // 依赖注入接口
    void setRenderer(std::unique_ptr<IRenderer> renderer);

    // 配置接口
    void setRenderConfig( const RenderConfig& config );

signals:
    void fpsChanged();
    void renderTypeChanged();
    void renderError( const QString& message );

public slots:
    void sync();
    void cleanup();

protected:
    void timerEvent( QTimerEvent* event ) override;
    void geometryChange( const QRectF& newGeometry, const QRectF& oldGeometry ) override;


private:
    void createRenderer();
    void initializeRenderer();
    void updateProjectMatrix();
    void handleRenderError( RenderError error, const std::string& message );
    void handleResize();

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
