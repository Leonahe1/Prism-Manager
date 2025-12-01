#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include "BasePage.h"
#include "ElaDef.h"
#include "core/ParserFactory.h"
#include <QMap>
#include <QString>
#include <QVariant>

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
    void setProjectRootPath(const QString& rootPath);
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
    void configFileAdded(const QString& filePath);  // 新增：通知添加了新配置文件

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
     * @brief 从 QVariant 推断并创建 ConfigItem
     * @param variant QVariant 值
     * @return ConfigItem 包含值和类型
     */
    ConfigItem variantToConfigItem(const QVariant& variant);

    /**
     * @brief 从源码同步到组件模式
     */
    void syncSourceToForm();

    /**
     * @brief 从组件模式同步到源码
     */
    void syncFormToSource();

    /**
     * @brief 使用 ParserFactory 解析配置文件
     * @param filePath 文件路径
     * @return 解析后的配置项映射
     */
    QMap<QString, ConfigItem> parseConfigFile(const QString& filePath);

    /**
     * @brief 构建表单界面（统一方法）
     * @param configData 配置数据
     */
    void buildForm(const QMap<QString, ConfigItem>& configData);

    /**
     * @brief 从表单收集数据并保存到文件
     */
    void collectFormAndSave();

    /**
     * @brief 清空表单控件
     */
    void clearFormWidgets();

    /**
     * @brief 获取卡片样式表（根据当前主题）
     */
    QString getCardStyleSheet() const;

    /**
     * @brief 获取分割器与滚动条在不同主题下的样式表
     * @return 根据当前主题获取的样式表
     */
    QString getSplitterAndScrollBarStyleSheet() const;

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
    QString _currentProjectRootPath;  // 新增：项目根路径
    QStringList _currentConfigFiles;  // 新增：当前配置文件列表
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
