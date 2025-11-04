#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QDebug>  // 添加调试输出支持
// #include <HuskarUI/husapp.h>
#include "HuskarUI/include/husapp.h"
#include "iostream"
#include "src/OpenGL/opengl_item.hpp"

// 测试各种设计模式
#include "src/CPP/VirtualFunctionTest.hpp"
#include "src/CPP/FactoryTest.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    // 自动创建的QQuickWindow类
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QQuickWindow::setDefaultAlphaBuffer(true);
    // 和Plugin一样, 在main.qml之前使用了, 所以需要在loadModule之前导入 OpenGLItem!!!
    qmlRegisterType<OpenGLItem>( "lib.OpenGLItem", 1, 0, "OpenGLItem" );
    QQmlApplicationEngine engine;
    HusApp::initialize(&engine);

    // 获取应用程序目录，用于定位 HuskarUI 插件
    QString appDir = QCoreApplication::applicationDirPath();    // 也可以使用子类: QGuiApplication::applicationDirPath();

    std::cout << "Application Dir: " << appDir.toStdString() << std::endl;

    // 关键修复：必须在 loadFromModule 之前添加 Import Path
    // 添加 HuskarUI 插件路径
    engine.addImportPath(appDir + "/HuskarUI/plugins");
    engine.addImportPath(appDir);
    
    // 调试输出：查看所有 Import Path
    qDebug() << "QML Import Paths:" << engine.importPathList();

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    
    // 加载主 QML 文件（必须在 addImportPath 之后）
    engine.loadFromModule("QMLSQLite", "Main");

    // 设计模式测试
    useStrategyTest();
    FactoryRender::createRenderer(_OPENGL_RENDER);

    return app.exec();
}
