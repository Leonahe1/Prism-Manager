#ifndef CREATESNAPSHOTDIALOG_H
#define CREATESNAPSHOTDIALOG_H

#include "ElaContentDialog.h"
#include <QStringList>

class ElaLineEdit;
class ElaPlainTextEdit;
class ElaCheckBox;
class QVBoxLayout;

/**
 * @brief 创建配置快照对话框
 *
 * 功能：
 * - 输入快照名称（支持默认名称和自定义）
 * - 输入快照描述（可选）
 * - 选择要包含的配置文件（支持全选/取消全选）
 */
class CreateSnapshotDialog : public ElaContentDialog {
    Q_OBJECT

public:
    explicit CreateSnapshotDialog(const QString& projectPath,
                                 const QStringList& configFiles,
                                 QWidget* parent = nullptr);
    ~CreateSnapshotDialog() override;

    /**
     * @brief 获取快照名称
     * @return 快照名称（如果为空则使用默认名称）
     */
    QString getSnapshotName() const;

    /**
     * @brief 获取快照描述
     * @return 快照描述
     */
    QString getSnapshotDescription() const;

    /**
     * @brief 获取选中的配置文件列表
     * @return 配置文件相对路径列表
     */
    QStringList getSelectedFiles() const;

private slots:
    void onSelectAllChanged(int state);
    void onFileCheckBoxChanged(int state);

private:
    void setupUI();
    void updateSelectAllState();

    QString _projectPath;
    QStringList _configFiles;

    ElaLineEdit* _nameEdit;
    ElaPlainTextEdit* _descriptionEdit;
    ElaCheckBox* _selectAllCheckBox;
    QVBoxLayout* _filesLayout;
    QList<ElaCheckBox*> _fileCheckBoxes;
};

#endif // CREATESNAPSHOTDIALOG_H
