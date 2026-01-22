#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QString>
#include <QStringList>

namespace Prism {

/**
 * @brief 应用设置管理单例类
 *
 * 统一管理所有应用配置项，使用 QSettings 持久化存储。
 * 配置分为四大类：外观、编辑器、日志、通用设置。
 */
class AppSettings : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static AppSettings& instance();

    // ========================================
    // 外观设置 (Appearance)
    // ========================================

    /**
     * @brief 主题模式
     * @return 0=浅色, 1=深色, 2=跟随系统
     */
    int themeMode() const;
    void setThemeMode(int mode);

    /**
     * @brief 界面语言
     * @return 语言代码，如 "zh_CN"
     */
    QString language() const;
    void setLanguage(const QString& lang);

    // ========================================
    // 编辑器设置 (Editor)
    // ========================================

    /**
     * @brief 代码字体
     */
    QString editorFont() const;
    void setEditorFont(const QString& font);

    /**
     * @brief 字体大小
     */
    int editorFontSize() const;
    void setEditorFontSize(int size);

    /**
     * @brief Tab 宽度
     */
    int tabWidth() const;
    void setTabWidth(int width);

    /**
     * @brief 是否显示行号
     */
    bool showLineNumbers() const;
    void setShowLineNumbers(bool show);

    // ========================================
    // 日志设置 (Logging)
    // ========================================

    /**
     * @brief 最大日志行数
     */
    int maxLogLines() const;
    void setMaxLogLines(int lines);

    /**
     * @brief 默认显示的日志级别
     * @return 级别列表，如 ["INFO", "SUCCESS", "WARNING", "ERROR"]
     */
    QStringList defaultLogLevels() const;
    void setDefaultLogLevels(const QStringList& levels);

    /**
     * @brief 日志时间戳格式
     */
    QString timestampFormat() const;
    void setTimestampFormat(const QString& format);

    // ========================================
    // 通用设置 (General)
    // ========================================

    /**
     * @brief 启动时是否恢复上次项目
     */
    bool restoreLastProject() const;
    void setRestoreLastProject(bool restore);

    /**
     * @brief 首页最近项目显示数量
     */
    int recentProjectCount() const;
    void setRecentProjectCount(int count);

    /**
     * @brief 是否记住窗口位置和大小
     */
    bool rememberWindowGeometry() const;
    void setRememberWindowGeometry(bool remember);

    /**
     * @brief 自动保存间隔（分钟），0 表示禁用
     */
    int autoSaveInterval() const;
    void setAutoSaveInterval(int minutes);

    // ========================================
    // 操作方法
    // ========================================

    /**
     * @brief 从 QSettings 加载所有设置
     */
    void load();

    /**
     * @brief 保存所有设置到 QSettings
     */
    void save();

    /**
     * @brief 重置所有设置为默认值
     */
    void reset();

signals:
    /**
     * @brief 任意设置项变更时发出
     */
    void settingsChanged();

    /**
     * @brief 主题设置变更时发出
     * @param mode 新的主题模式
     */
    void themeChanged(int mode);

    /**
     * @brief 编辑器设置变更时发出
     */
    void editorSettingsChanged();

    /**
     * @brief 日志设置变更时发出
     */
    void logSettingsChanged();

private:
    explicit AppSettings(QObject* parent = nullptr);
    ~AppSettings() override = default;

    // 禁止拷贝和赋值
    AppSettings(const AppSettings&) = delete;
    AppSettings& operator=(const AppSettings&) = delete;

    // ========================================
    // 成员变量
    // ========================================

    // 外观设置
    int _themeMode{ 2 };           // 默认跟随系统
    QString _language{ "zh_CN" };

    // 编辑器设置
    QString _editorFont{ "Consolas" };
    int _editorFontSize{ 12 };
    int _tabWidth{ 4 };
    bool _showLineNumbers{ true };

    // 日志设置
    int _maxLogLines{ 10000 };
    QStringList _defaultLogLevels{ "INFO", "SUCCESS", "WARNING", "ERROR", "STDOUT", "STDERR" };
    QString _timestampFormat{ "HH:mm:ss" };

    // 通用设置
    bool _restoreLastProject{ true };
    int _recentProjectCount{ 5 };
    bool _rememberWindowGeometry{ true };
    int _autoSaveInterval{ 0 };
};

} // namespace Prism

#endif // APPSETTINGS_H
