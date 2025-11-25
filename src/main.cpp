#include "ui/MainWindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDebug>

#include "ElaWidgetTools/ElaApplication.h"

// TODO: 当集成 ElaWidgetTools 后，使用 ElaApplication
// #include "ElaApplication.h"

/**
 * @brief Prism 主入口
 *
 * 初始化流程：
 * 1. 启用高分屏支持
 * 2. 创建 QApplication（或 ElaApplication）
 * 3. 设置应用信息
 * 4. 加载主题和样式
 * 5. 显示主窗口
 */
int main(int argc, char* argv[]) {
    // ========================================
    // 高分屏支持（Qt 5.14+）
    // ========================================
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // ========================================
    // 创建应用程序实例
    // ========================================
    // TODO: 替换为 ElaApplication
    // ElaApplication app(argc, argv);
    QApplication app(argc, argv);
    //eApp->init();
    // ========================================
    // 设置应用程序信息
    // ========================================
    app.setOrganizationName("PrismTeam");
    app.setOrganizationDomain("prism-manager.com");
    app.setApplicationName("Prism");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("Prism - Configuration Integration Environment");

    // ========================================
    // 设置样式（可选：Fusion 风格作为后备）
    // ========================================
    // ElaWidgetTools 会自动应用 Fluent Design
    // 这里先使用 Fusion 作为占位符
    app.setStyle(QStyleFactory::create("Fusion"));

    // 设置调色板（深色主题）
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);

    // ========================================
    // 日志输出
    // ========================================
    qDebug() << "========================================";
    qDebug() << "Prism Manager Starting...";
    qDebug() << "Version:" << app.applicationVersion();
    qDebug() << "Qt Version:" << qVersion();
    qDebug() << "========================================";

    // ========================================
    // 创建并显示主窗口
    // ========================================
    Prism::MainWindow mainWindow;
    mainWindow.show();

    // ========================================
    // 进入事件循环
    // ========================================
    int ret = app.exec();

    qDebug() << "Prism Manager Exiting with code:" << ret;
    return ret;
}
