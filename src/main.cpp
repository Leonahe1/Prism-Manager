#include "ui/MainWindow.h"
#include <QApplication>
#include <QDebug>
#include "ElaApplication.h"

/**
 * @brief Prism 主入口
 *
 * 初始化流程：
 * 1. 启用高分屏支持（必须在 QApplication 创建之前）
 * 2. 创建 QApplication
 * 3. 初始化 ElaApplication
 * 4. 设置应用信息
 * 5. 显示主窗口
 */
int main(int argc, char* argv[]) {
    // ========================================
    // 高分屏支持（必须在 QApplication 创建前配置）
    // ========================================
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#else
    // Qt 5.14 以下版本，手动设置缩放比例
    qputenv("QT_SCALE_FACTOR", "1.5");
#endif
#endif

    // ========================================
    // 创建应用程序实例
    // ========================================
    QApplication app(argc, argv);

    // 关键：初始化 ElaApplication 单例（必须在 QApplication 创建后立即调用）
    eApp->init();
    // ========================================
    // 设置应用程序信息
    // ========================================
    app.setOrganizationName("PrismTeam");
    app.setOrganizationDomain("prism-manager.com");
    app.setApplicationName("Prism");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("Prism - Configuration Integration Environment");

    // ========================================
    // 日志输出
    // ========================================
    qDebug() << "========================================";
    qDebug() << "Prism 管理器启动中...";
    qDebug() << "版本:" << app.applicationVersion();
    qDebug() << "Qt 版本:" << qVersion();
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

    qDebug() << "Prism 管理器退出，返回码:" << ret;
    return ret;
}
