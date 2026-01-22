#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include "BasePage.h"
#include <QList>

class ElaComboBox;
class ElaSpinBox;
class ElaToggleSwitch;
class ElaCheckBox;
class QWidget;

namespace Prism {

/**
 * @brief 设置页面
 *
 * 提供应用程序的配置界面，包含四大配置分类：
 * - 外观设置：主题、语言
 * - 编辑器设置：字体、大小、Tab宽度、行号
 * - 日志设置：最大行数、时间戳格式、默认级别
 * - 通用设置：启动行为、窗口记忆、自动保存
 */
class SettingsPage : public BasePage
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit SettingsPage(QWidget* parent = nullptr);
    ~SettingsPage() override;

private slots:
    /**
     * @brief 应用设置变更
     */
    void onApplySettings();

    /**
     * @brief 重置为默认值
     */
    void onResetSettings();

    /**
     * @brief 主题模式变更
     */
    void onThemeModeChanged(int index);

private:
    void initUI();
    void setupConnections();
    void loadCurrentSettings();

    /**
     * @brief 创建设置分组卡片
     * @param title 分组标题
     * @return 卡片容器
     */
    QWidget* createSettingsCard(const QString& title);

    /**
     * @brief 创建设置项行（标签 + 控件）
     * @param label 标签文本
     * @param widget 控件
     * @param parent 父容器
     * @return 行容器
     */
    QWidget* createSettingRow(const QString& label, QWidget* widget, QWidget* parent);

    /**
     * @brief 获取卡片样式表
     */
    QString getCardStyleSheet() const;

    /**
     * @brief 响应主题变化
     */
    void onThemeChanged();

    // ========================================
    // UI 控件
    // ========================================

    // 外观设置
    ElaComboBox* _themeModeCombo{ nullptr };
    ElaComboBox* _languageCombo{ nullptr };

    // 编辑器设置
    ElaComboBox* _editorFontCombo{ nullptr };
    ElaSpinBox* _fontSizeSpinBox{ nullptr };
    ElaSpinBox* _tabWidthSpinBox{ nullptr };
    ElaToggleSwitch* _showLineNumbersSwitch{ nullptr };

    // 日志设置
    ElaSpinBox* _maxLogLinesSpinBox{ nullptr };
    ElaComboBox* _timestampFormatCombo{ nullptr };
    QList<ElaCheckBox*> _logLevelCheckBoxes;

    // 通用设置
    ElaToggleSwitch* _restoreLastProjectSwitch{ nullptr };
    ElaSpinBox* _recentProjectCountSpinBox{ nullptr };
    ElaToggleSwitch* _rememberWindowGeometrySwitch{ nullptr };
    ElaSpinBox* _autoSaveIntervalSpinBox{ nullptr };

    // 卡片列表（用于主题更新）
    QList<QWidget*> _cards;
};

} // namespace Prism

#endif // SETTINGSPAGE_H
