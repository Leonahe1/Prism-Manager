#include "AppSettings.h"
#include <QSettings>
#include <QDebug>

namespace Prism {

AppSettings& AppSettings::instance()
{
    static AppSettings instance;
    return instance;
}

AppSettings::AppSettings(QObject* parent)
    : QObject(parent)
{
    // 构造时自动加载设置
    load();
}

// ========================================
// 外观设置
// ========================================

int AppSettings::themeMode() const
{
    return _themeMode;
}

void AppSettings::setThemeMode(int mode)
{
    if (_themeMode != mode) {
        _themeMode = mode;
        emit themeChanged(mode);
        emit settingsChanged();
    }
}

QString AppSettings::language() const
{
    return _language;
}

void AppSettings::setLanguage(const QString& lang)
{
    if (_language != lang) {
        _language = lang;
        emit settingsChanged();
    }
}

// ========================================
// 编辑器设置
// ========================================

QString AppSettings::editorFont() const
{
    return _editorFont;
}

void AppSettings::setEditorFont(const QString& font)
{
    if (_editorFont != font) {
        _editorFont = font;
        emit editorSettingsChanged();
        emit settingsChanged();
    }
}

int AppSettings::editorFontSize() const
{
    return _editorFontSize;
}

void AppSettings::setEditorFontSize(int size)
{
    if (_editorFontSize != size) {
        _editorFontSize = qBound(8, size, 24);
        emit editorSettingsChanged();
        emit settingsChanged();
    }
}

int AppSettings::tabWidth() const
{
    return _tabWidth;
}

void AppSettings::setTabWidth(int width)
{
    if (_tabWidth != width) {
        _tabWidth = qBound(2, width, 8);
        emit editorSettingsChanged();
        emit settingsChanged();
    }
}

bool AppSettings::showLineNumbers() const
{
    return _showLineNumbers;
}

void AppSettings::setShowLineNumbers(bool show)
{
    if (_showLineNumbers != show) {
        _showLineNumbers = show;
        emit editorSettingsChanged();
        emit settingsChanged();
    }
}

// ========================================
// 日志设置
// ========================================

int AppSettings::maxLogLines() const
{
    return _maxLogLines;
}

void AppSettings::setMaxLogLines(int lines)
{
    if (_maxLogLines != lines) {
        _maxLogLines = qBound(1000, lines, 100000);
        emit logSettingsChanged();
        emit settingsChanged();
    }
}

QStringList AppSettings::defaultLogLevels() const
{
    return _defaultLogLevels;
}

void AppSettings::setDefaultLogLevels(const QStringList& levels)
{
    if (_defaultLogLevels != levels) {
        _defaultLogLevels = levels;
        emit logSettingsChanged();
        emit settingsChanged();
    }
}

QString AppSettings::timestampFormat() const
{
    return _timestampFormat;
}

void AppSettings::setTimestampFormat(const QString& format)
{
    if (_timestampFormat != format) {
        _timestampFormat = format;
        emit logSettingsChanged();
        emit settingsChanged();
    }
}

// ========================================
// 通用设置
// ========================================

bool AppSettings::restoreLastProject() const
{
    return _restoreLastProject;
}

void AppSettings::setRestoreLastProject(bool restore)
{
    if (_restoreLastProject != restore) {
        _restoreLastProject = restore;
        emit settingsChanged();
    }
}

int AppSettings::recentProjectCount() const
{
    return _recentProjectCount;
}

void AppSettings::setRecentProjectCount(int count)
{
    if (_recentProjectCount != count) {
        _recentProjectCount = qBound(3, count, 10);
        emit settingsChanged();
    }
}

bool AppSettings::rememberWindowGeometry() const
{
    return _rememberWindowGeometry;
}

void AppSettings::setRememberWindowGeometry(bool remember)
{
    if (_rememberWindowGeometry != remember) {
        _rememberWindowGeometry = remember;
        emit settingsChanged();
    }
}

int AppSettings::autoSaveInterval() const
{
    return _autoSaveInterval;
}

void AppSettings::setAutoSaveInterval(int minutes)
{
    if (_autoSaveInterval != minutes) {
        _autoSaveInterval = qBound(0, minutes, 60);
        emit settingsChanged();
    }
}

// ========================================
// 操作方法
// ========================================

void AppSettings::load()
{
    QSettings settings("GTTC", "Prism");

    // 外观设置
    settings.beginGroup("Appearance");
    _themeMode = settings.value("themeMode", 2).toInt();
    _language = settings.value("language", "zh_CN").toString();
    settings.endGroup();

    // 编辑器设置
    settings.beginGroup("Editor");
    _editorFont = settings.value("font", "Consolas").toString();
    _editorFontSize = settings.value("fontSize", 12).toInt();
    _tabWidth = settings.value("tabWidth", 4).toInt();
    _showLineNumbers = settings.value("showLineNumbers", true).toBool();
    settings.endGroup();

    // 日志设置
    settings.beginGroup("Logging");
    _maxLogLines = settings.value("maxLines", 10000).toInt();
    QString levelsStr = settings.value("defaultLevels", "INFO,SUCCESS,WARNING,ERROR,STDOUT,STDERR").toString();
    _defaultLogLevels = levelsStr.split(',', Qt::SkipEmptyParts);
    _timestampFormat = settings.value("timestampFormat", "HH:mm:ss").toString();
    settings.endGroup();

    // 通用设置
    settings.beginGroup("General");
    _restoreLastProject = settings.value("restoreLastProject", true).toBool();
    _recentProjectCount = settings.value("recentProjectCount", 5).toInt();
    _rememberWindowGeometry = settings.value("rememberWindowGeometry", true).toBool();
    _autoSaveInterval = settings.value("autoSaveInterval", 0).toInt();
    settings.endGroup();

    qDebug() << "AppSettings: 已加载设置";
}

void AppSettings::save()
{
    QSettings settings("GTTC", "Prism");

    // 外观设置
    settings.beginGroup("Appearance");
    settings.setValue("themeMode", _themeMode);
    settings.setValue("language", _language);
    settings.endGroup();

    // 编辑器设置
    settings.beginGroup("Editor");
    settings.setValue("font", _editorFont);
    settings.setValue("fontSize", _editorFontSize);
    settings.setValue("tabWidth", _tabWidth);
    settings.setValue("showLineNumbers", _showLineNumbers);
    settings.endGroup();

    // 日志设置
    settings.beginGroup("Logging");
    settings.setValue("maxLines", _maxLogLines);
    settings.setValue("defaultLevels", _defaultLogLevels.join(','));
    settings.setValue("timestampFormat", _timestampFormat);
    settings.endGroup();

    // 通用设置
    settings.beginGroup("General");
    settings.setValue("restoreLastProject", _restoreLastProject);
    settings.setValue("recentProjectCount", _recentProjectCount);
    settings.setValue("rememberWindowGeometry", _rememberWindowGeometry);
    settings.setValue("autoSaveInterval", _autoSaveInterval);
    settings.endGroup();

    qDebug() << "AppSettings: 已保存设置";
}

void AppSettings::reset()
{
    // 外观设置
    _themeMode = 2;  // 跟随系统
    _language = "zh_CN";

    // 编辑器设置
    _editorFont = "Consolas";
    _editorFontSize = 12;
    _tabWidth = 4;
    _showLineNumbers = true;

    // 日志设置
    _maxLogLines = 10000;
    _defaultLogLevels = QStringList{ "INFO", "SUCCESS", "WARNING", "ERROR", "STDOUT", "STDERR" };
    _timestampFormat = "HH:mm:ss";

    // 通用设置
    _restoreLastProject = true;
    _recentProjectCount = 5;
    _rememberWindowGeometry = true;
    _autoSaveInterval = 0;

    emit themeChanged(_themeMode);
    emit editorSettingsChanged();
    emit logSettingsChanged();
    emit settingsChanged();

    qDebug() << "AppSettings: 已重置为默认值";
}

} // namespace Prism
