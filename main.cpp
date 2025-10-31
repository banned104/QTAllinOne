#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QDebug>  // 添加调试输出支持
// #include <HuskarUI/husapp.h>
#include "HuskarUI/include/husapp.h"
#include "iostream"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    /*! Set OpenGL, optional */
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QQuickWindow::setDefaultAlphaBuffer(true);

    QQmlApplicationEngine engine;
    HusApp::initialize(&engine);

    // 获取应用程序目录，用于定位 HuskarUI 插件
    QString appDir = QCoreApplication::applicationDirPath();
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

    return app.exec();
}
