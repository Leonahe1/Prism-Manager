#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include "BasePage.h"
#include "ElaDef.h"
#include <QMap>
#include <QString>

class ElaCentralStackedWidget;
class ElaWidget;
class ElaScrollPageArea;
// 前向声明
class ElaTreeView;
class ElaPlainTextEdit;
class QStandardItemModel;
class QSplitter;
class QPushButton;
class QStackedWidget;
class QScrollArea;
class ElaRadioButton;

namespace Prism {

class ConfigParser;

/**
 * @brief 配置值类型枚举
 */
enum class ConfigValueType {
    String,     // 字符串
    Integer,    // 整数
    Double,     // 浮点数
    Boolean,    // 布尔值
    Array       // 数组
};

/**
 * @brief 配置项数据结构
 */
struct ConfigItem {
    QString value;              // 值（字符串表示）
    ConfigValueType type;       // 类型

    ConfigItem() : type(ConfigValueType::String) {}
    ConfigItem(const QString& val, ConfigValueType t) : value(val), type(t) {}
};

/**
 * @brief 编辑模式枚举
 */
enum class EditMode {
    Source,     // 源码模式
    Form        // 组件模式
};

/**
 * @brief 配置文件管理页面
 *
 * 职责：
 * - 左侧：配置文件树形列表（支持多格式：INI/JSON/YAML）
 * - 右侧：配置编辑器（双模式：源码/组件）
 * - 底部：保存/重载/验证按钮
 *
 * 设计要点：
 * - 支持不同格式的配置文件切换
 * - 源码模式：直接编辑原始文本
 * - 组件模式：根据配置内容生成表单控件
 */
class ConfigPage : public BasePage
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit ConfigPage(QWidget* parent = nullptr);
    ~ConfigPage() override;

    void setProjectName(const QString& projectName);
    void loadConfigFiles(const QStringList& configFiles);
    void openConfigFile(const QString& filePath);

public slots:
    void saveCurrentConfig();
    void reloadCurrentConfig();
    void validateConfig();
    void addConfigFile();

    /**
     * @brief 切换编辑模式
     * @param mode 目标模式
     */
    void switchEditMode(EditMode mode);

    /**
     * @brief 主题变化响应
     */
    void onThemeChanged(ElaThemeType::ThemeMode mode);

signals:
    void configFileModified(const QString& filePath);
    void configFileSaved(const QString& filePath);

private:
    // ========================================
    // 初始化方法
    // ========================================
    void initUI();
    void setupConnections();

    // ========================================
    // 辅助方法
    // ========================================
    QString detectConfigFormat(const QString& filePath);
    void updateSyntaxHighlighter(const QString& format);

    /**
     * @brief 从源码同步到组件模式
     */
    void syncSourceToForm();

    /**
     * @brief 从组件模式同步到源码
     */
    void syncFormToSource();

    /**
     * @brief 解析 INI 格式配置
     * @return 分组的键值对映射，包含类型信息
     */
    QMap<QString, QMap<QString, ConfigItem>> parseIniContent(const QString& content);

    /**
     * @brief 构建 INI 格式的表单界面
     */
    void buildIniForm();

    /**
     * @brief 从 INI 表单收集数据并同步到源码编辑器
     */
    void collectIniFormToSource();

    /**
     * @brief 解析 YAML 格式配置
     * @return 扁平化的键值对映射，包含类型信息 (例如: "server.host" -> ConfigItem{"localhost", String})
     */
    QMap<QString, ConfigItem> parseYamlContent(const QString& content);

    /**
     * @brief 构建 YAML 格式的表单界面
     */
    void buildYamlForm();

    /**
     * @brief 从 YAML 表单收集数据并同步到源码编辑器
     */
    void collectYamlFormToSource();

    /**
     * @brief 解析 JSON 格式配置
     * @return 扁平化的键值对映射，包含类型信息 (例如: "server.host" -> ConfigItem{"localhost", String})
     */
    QMap<QString, ConfigItem> parseJsonContent(const QString& content);

    /**
     * @brief 构建 JSON 格式的表单界面
     */
    void buildJsonForm();

    /**
     * @brief 从 JSON 表单收集数据并同步到源码编辑器
     */
    void collectJsonFormToSource();

    /**
     * @brief 清空表单控件
     */
    void clearFormWidgets();

    /**
     * @brief 获取卡片样式表（根据当前主题）
     */
    QString getCardStyleSheet() const;

private:
    // ========================================
    // UI 组件
    // ========================================
    QSplitter* _mainSplitter{ nullptr };
    ElaTreeView* _configTreeView{ nullptr };
    QStandardItemModel* _treeModel{ nullptr };

    // 模式切换
    ElaRadioButton* _sourceModeBtn{ nullptr };
    ElaRadioButton* _formModeBtn{ nullptr };
    QStackedWidget* _editorStack{ nullptr };

    // 源码模式
    ElaPlainTextEdit* _configEditor{ nullptr };

    // 组件模式
    QScrollArea* _formScrollArea{ nullptr };
    QWidget* _formContainer{ nullptr };
    QVBoxLayout* _formLayout{ nullptr };

    // 按钮
    QPushButton* _saveButton{ nullptr };
    QPushButton* _reloadButton{ nullptr };
    QPushButton* _validateButton{ nullptr };
    QPushButton* _addFileButton{ nullptr };

    // ========================================
    // 数据管理
    // ========================================
    QString _currentProjectName;
    QString _currentFilePath;
    QString _currentFormat;
    QMap<QString, QString> _filePathToFormat;
    bool _isModified{ false };
    EditMode _currentMode{ EditMode::Form };  // 默认组件模式

    // 组件模式数据：path -> widget（支持嵌套路径如 "server.host"）
    QMap<QString, QWidget*> _formWidgetMap;

    // 卡片列表（用于主题更新）
    QList<QWidget*> _formCards;
};

} // namespace Prism

#endif // CONFIGPAGE_H
