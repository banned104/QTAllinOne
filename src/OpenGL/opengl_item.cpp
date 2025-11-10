#include "opengl_item.hpp"
#include "global_macro.hpp"
#include "render_factory.hpp"
#include <QQuickWindow>
#include <string>



OpenGLItem::OpenGLItem()
    : m_fps(0)
    , m_lastTime( QTime::currentTime() )
    , m_rendererType( "triangle" )
    , m_frameNumer(0)
    , m_rendererInitialized( false )
{
    m_config = RenderConfig::createTriangleConfig();

    m_timer.start(12, this);

    // 设置标志 让 QQuickItem 接收几何变化的通知
    setFlag( ItemHasContents, true );

    // 连接窗口信号
    connect( this, &QQuickItem::windowChanged, this, [this](QQuickWindow *window) {
        if ( window ) {
            connect( window, &QQuickWindow::beforeRendering, this, &OpenGLItem::sync,
                    Qt::DirectConnection );
            connect( window, &QQuickWindow::sceneGraphInvalidated, this, &OpenGLItem::cleanup,
                    Qt::DirectConnection );
        }
    } );
}

OpenGLItem::~OpenGLItem() {
    cleanup();
}

void OpenGLItem::setFps( int fps ) {
    if ( fps == m_fps ) return;
    m_fps = fps;
    emit fpsChanged();
}

void OpenGLItem::setRenderType( const QString& type ) {
    if ( type == m_rendererType ) return;

    m_rendererType = type;

    if ( m_renderer ) {
        m_renderer->cleanup();
        m_renderer.reset();
    }

    createRenderer();

    emit renderTypeChanged();
}

void OpenGLItem::setRenderer( std::unique_ptr<IRenderer> renderer ) {
    if ( m_renderer ) {
        m_renderer->cleanup();
    }

    m_renderer = std::move( renderer );
    m_rendererInitialized = false;
}

void OpenGLItem::setRenderConfig( const RenderConfig& config ) {
    m_config = config;
    // 如果渲染器已经初始化, 修改了Config之后要重新初始化
    if ( m_rendererInitialized && m_renderer ) {
        m_renderer->cleanup();
        initializeRenderer();
    }
}

void OpenGLItem::sync(){
    if ( !m_renderer ) {
        createRenderer();
    }

    if ( m_renderer && !m_rendererInitialized ) {
        initializeRenderer();

        if ( m_rendererInitialized && window() ) {
            // 初始化视口
            m_renderer->resize( static_cast<int>(width()), static_cast<int>(height()) );
            updateProjectMatrix();
        }

        // 连接渲染信号
        connect( window(), &QQuickWindow::beforeSynchronizing, this, [this](){
            if ( m_renderer && m_rendererInitialized ) {
                // 创建上下文
                RenderContext context(
                    QSize( static_cast<int>(width()),static_cast<int>(height()) ),
                    m_projectionMatrix,
                    0.0f
                );
                context = context.withFrameNumber(m_frameNumer++);

                // 执行渲染
                m_renderer->render(context);
            }
        }, Qt::DirectConnection );

        // 连接FPS计算
        connect( window(), &QQuickWindow::afterRendering, this, [this](){
            static int frame = 0;
            frame++;
            if ( m_lastTime.msecsTo(QTime::currentTime()) > 1000 ) {
                setFps( frame );
                m_lastTime = QTime::currentTime();
                frame = 0;
            }
        }, Qt::DirectConnection );

    }
}

void OpenGLItem::cleanup() {
    if ( m_renderer ) {
        m_renderer->cleanup();
        m_renderer.reset();
    }
    m_rendererInitialized = false;
}

void OpenGLItem::timerEvent( QTimerEvent* e ) {
    Q_UNUSED( e );
    if ( window() ) {
        window()->update();
    }
}

void OpenGLItem::createRenderer() {
    m_renderer = RenderFactory::create( m_rendererType.toStdString() );

    if ( !m_renderer ) {
        qWarning() << "Failed to create: " << m_rendererType;
    }
}

void OpenGLItem::initializeRenderer() {
    if ( !m_renderer ) { return; }

    // 添加错误回调
    m_renderer->setErrorCallback( [this] ( RenderError error, const std::string& msg ){
        handleRenderError(error, msg);
    } );

    // 初始化渲染器
    if ( !m_renderer->initialize(m_config) ) {
        qWarning() << "Failed to initialize renderer";
        m_renderer.reset();
        m_rendererInitialized = false;
    } else {
        m_rendererInitialized = true;
    }
}

void OpenGLItem::updateProjectMatrix() {
    int w = static_cast<int>( width() );
    int h = static_cast<int>( height() );

    if ( w <=0 || h <= 0 ) return;

    qreal aspect = qreal(w)/qreal(h);
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(30.0f, aspect, 3.0f, 10.0f );

}

void OpenGLItem::handleRenderError( RenderError error, const std::string& msg ) {
    QString errorMsg = QString::fromStdString(msg);
    qWarning() << "Render Error: " << errorMsg;
    emit renderError(errorMsg);
}

void OpenGLItem::geometryChange( const QRectF& newGeometry, const QRectF& oldGeometry ) {
    QQuickItem::geometryChange( newGeometry, oldGeometry );

    // 只有尺寸变化时才会处理
    if ( newGeometry.size() != oldGeometry.size() ) {
        handleResize();
    }
}

void OpenGLItem::handleResize()
{
    if (m_renderer && m_rendererInitialized) {
        int w = static_cast<int>(width());
        int h = static_cast<int>(height());

        // 避免无效尺寸
        if (w > 0 && h > 0) {
            m_renderer->resize(w, h);
            updateProjectMatrix();

            // 触发重绘
            if (window()) {
                window()->update();
            }
        }
    }
}



