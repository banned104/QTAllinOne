#include "opengl_item.hpp"
#include "global_macro.hpp"
#include <QQuickWindow>
#include <iostream>
#include <string>



OpenGLItem::OpenGLItem()
    : m_render( nullptr )
{
    // BasicTimer 和 普通 Timer区别?
    m_timer.start( 12, this );
    connect( this, &QQuickItem::windowChanged, this, [this]( QQuickWindow *window ){
        if ( window ) {
            connect( window, &QQuickWindow::beforeSynchronizing, this, &OpenGLItem::sync,
                    Qt::DirectConnection );
            connect( window, &QQuickWindow::sceneGraphInvalidated, this, &OpenGLItem::cleanup,
                    Qt::DirectConnection );
            // window->setClearBeforeRendering( false );
            PRINT_LOG("Window created");
        } else {
            PRINT_LOG("Window was not created");
            return;
        }
    } );
}

OpenGLItem::~OpenGLItem() {

}

void OpenGLItem::sync(){
    if ( !m_render ) {
        m_render = new MyRenderer();
        m_render->initializeGL();
        // m_render->resizeGL( window()->width(), window()->height() );
        m_render->resizeGL( this->width(), this->height() );

        PRINT_LOG( "Window height: " + std::to_string(window()->width())
                + " Window width: " + std::to_string(window()->width()) );

        PRINT_LOG( "Item height: " + std::to_string(this->width())
                  + " Item width: " + std::to_string(this->width()) );

        connect( window(), &QQuickWindow::beforeRendering, this, [this](){
            m_render->render();
        }, Qt::DirectConnection );

        connect( window(), &QQuickWindow::afterRendering, this, [this](){
            // 渲染之后调用 用于计算FPS
        }, Qt::DirectConnection );


        // Resize Event
        connect( window(), &QQuickWindow::widthChanged, this, [this](){
            m_render->resizeGL( this->width(), this->height() );
        } );
        connect( window(), &QQuickWindow::heightChanged, this, [this](){
            m_render->resizeGL( this->width(), this->height() );
        } );
    }
}

void OpenGLItem::cleanup() {
    if ( m_render ) {
        delete m_render;
        m_render = nullptr;
    }
}

void OpenGLItem::timeEvent( QTimerEvent* e ) {
    Q_UNUSED( e );
    window()->update();
}
