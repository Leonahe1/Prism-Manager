#ifndef PROCESSEDITDIALOG_H
#define PROCESSEDITDIALOG_H

#include "ElaDialog.h"
#include <QString>

class ElaLineEdit;
class ElaPushButton;
class ElaToggleSwitch;

namespace Prism {

/**
 * @brief 进程配置数据结构
 */
struct ProcessConfig {
    QString id;               // 唯一标识符
    QString name;             // 进程名称（用于显示）
    QString program;          // 程序路径
    QString arguments;        // 命令参数
    QString workingDirectory; // 工作目录（可选）
    bool showConsole{ false };// 是否显示控制台窗口

    ProcessConfig() = default;
    ProcessConfig(const QString& id, const QString& name,
                  const QString& program, const QString& args = QString())
        : id(id), name(name), program(program), arguments(args), showConsole(false) {}
};

/**
 * @brief 进程编辑对话框
 *
 * 用于添加或编辑进程配置
 */
class ProcessEditDialog : public ElaDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     * @param isEdit 是否为编辑模式（false 为添加模式）
     */
    explicit ProcessEditDialog(QWidget* parent = nullptr, bool isEdit = false);
    ~ProcessEditDialog() override;

    /**
     * @brief 设置进程配置（编辑模式）
     */
    void setConfig(const ProcessConfig& config);

    /**
     * @brief 获取进程配置
     */
    ProcessConfig getConfig() const;

private slots:
    void onBrowseClicked();
    void onBrowseWorkDirClicked();
    void onConfirmClicked();
    void onCancelClicked();

private:
    void setupUI();
    void setupConnections();
    bool validateInput();

    bool _isEditMode{ false };
    QString _configId;

    // UI 组件
    ElaLineEdit* _nameEdit{ nullptr };
    ElaLineEdit* _programEdit{ nullptr };
    ElaPushButton* _browseButton{ nullptr };
    ElaLineEdit* _argumentsEdit{ nullptr };
    ElaLineEdit* _workDirEdit{ nullptr };
    ElaPushButton* _browseWorkDirButton{ nullptr };
    ElaToggleSwitch* _showConsoleSwitch{ nullptr };
    ElaPushButton* _confirmButton{ nullptr };
    ElaPushButton* _cancelButton{ nullptr };
};

} // namespace Prism

#endif // PROCESSEDITDIALOG_H
