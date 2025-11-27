#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include "BasePage.h"
#include <QMap>
#include <QString>

// 前向声明
class ElaTreeView;
class ElaPlainTextEdit;
class QStandardItemModel;
class QSplitter;
class QPushButton;

namespace Prism {

class ConfigParser;

/**
 * @brief 配置文件管理页面
 *
 * 职责：
 * - 左侧：配置文件树形列表（支持多格式：INI/JSON/YAML）
 * - 右侧：配置编辑器（文本编辑 + 语法高亮）
 * - 底部：保存/重载/验证按钮
 *
 * 设计要点：
 * - 支持不同格式的配置文件切换
 * - 使用 ConfigParser 工厂模式解析不同格式
 * - 实时保存提示
 */
class ConfigPage : public BasePage
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit ConfigPage(QWidget* parent = nullptr);
    ~ConfigPage() override;

    /**
     * @brief 设置当前项目名称
     * @param projectName 项目名称（用于从 ProjectManager 获取数据）
     */
    void setProjectName(const QString& projectName);

    /**
     * @brief 加载项目的配置文件列表
     * @param configFiles 配置文件路径列表
     */
    void loadConfigFiles(const QStringList& configFiles);

    /**
     * @brief 打开指定配置文件
     * @param filePath 配置文件路径
     */
    void openConfigFile(const QString& filePath);

public slots:
    /**
     * @brief 保存当前编辑的配置文件
     */
    void saveCurrentConfig();

    /**
     * @brief 重新加载配置文件
     */
    void reloadCurrentConfig();

    /**
     * @brief 验证配置文件格式
     */
    void validateConfig();

    /**
     * @brief 添加新配置文件到项目
     */
    void addConfigFile();

signals:
    /**
     * @brief 配置文件修改信号
     * @param filePath 文件路径
     */
    void configFileModified(const QString& filePath);

    /**
     * @brief 配置文件保存信号
     * @param filePath 文件路径
     */
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
    /**
     * @brief 根据文件扩展名推断配置格式
     * @param filePath 文件路径
     * @return 配置格式 (ini/json/yaml/unknown)
     */
    QString detectConfigFormat(const QString& filePath);

    /**
     * @brief 更新编辑器的语法高亮
     * @param format 配置格式
     */
    void updateSyntaxHighlighter(const QString& format);

private:
    // ========================================
    // UI 组件
    // ========================================
    QSplitter* _mainSplitter{ nullptr };           // 主分割器
    ElaTreeView* _configTreeView{ nullptr };       // 配置文件树
    QStandardItemModel* _treeModel{ nullptr };     // 树模型
    ElaPlainTextEdit* _configEditor{ nullptr };    // 配置编辑器

    QPushButton* _saveButton{ nullptr };           // 保存按钮
    QPushButton* _reloadButton{ nullptr };         // 重载按钮
    QPushButton* _validateButton{ nullptr };       // 验证按钮
    QPushButton* _addFileButton{ nullptr };        // 添加文件按钮

    // ========================================
    // 数据管理
    // ========================================
    QString _currentProjectName;                   // 当前项目名称
    QString _currentFilePath;                      // 当前编辑的文件路径
    QString _currentFormat;                        // 当前文件格式 (ini/json/yaml)
    QMap<QString, QString> _filePathToFormat;      // 文件路径 → 格式映射
    bool _isModified{ false };                     // 是否已修改

    // TODO: 后续添加语法高亮器
    // QSyntaxHighlighter* _highlighter{ nullptr };
};

} // namespace Prism

#endif // CONFIGPAGE_H
