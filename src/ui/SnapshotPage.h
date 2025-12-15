#ifndef SNAPSHOTPAGE_H
#define SNAPSHOTPAGE_H

#include "BasePage.h"
#include "core/SnapshotManager.h"
#include <QWidget>

class ElaScrollPageArea;
class ElaText;
class ElaPushButton;
class ElaIconButton;
class QVBoxLayout;

/**
 * @brief 快照卡片组件
 *
 * 显示单个快照的信息和操作按钮
 */
class SnapshotCard : public QWidget {
    Q_OBJECT

public:
    explicit SnapshotCard(const SnapshotMetadata& metadata,
                         const QString& projectPath,
                         QWidget* parent = nullptr);

signals:
    void applyRequested(const QString& snapshotId);
    void renameRequested(const QString& snapshotId);
    void deleteRequested(const QString& snapshotId);
    void viewDetailsRequested(const QString& snapshotId);

private:
    void setupUI();
    void updateTheme();

    SnapshotMetadata _metadata;
    QString _projectPath;

    QWidget* _cardWidget;
    ElaText* _nameLabel;
    ElaText* _timeLabel;
    ElaText* _filesLabel;
    ElaText* _descLabel;
    ElaPushButton* _applyButton;
    ElaIconButton* _renameButton;
    ElaIconButton* _deleteButton;
    ElaIconButton* _detailsButton;
};

/**
 * @brief 配置快照管理页面
 *
 * 功能：
 * - 显示所有快照列表
 * - 创建新快照
 * - 应用快照（一键切换配置）
 * - 重命名/删除快照
 * - 查看快照详情
 */
class SnapshotPage : public BasePage {
    Q_OBJECT

public:
    explicit SnapshotPage(QWidget* parent = nullptr);
    ~SnapshotPage() override;

    /**
     * @brief 设置当前项目路径
     * @param projectPath 项目根目录路径
     * @param configFiles 项目的配置文件列表
     */
    void setProject(const QString& projectPath, const QStringList& configFiles);

    /**
     * @brief 刷新快照列表
     */
    void refreshSnapshots();

private slots:
    void onCreateSnapshot();
    void onApplySnapshot(const QString& snapshotId);
    void onRenameSnapshot(const QString& snapshotId);
    void onDeleteSnapshot(const QString& snapshotId);
    void onViewDetails(const QString& snapshotId);
    void onRefresh();

private:
    void initUI();
    void clearSnapshotCards();
    void showEmptyState();

    QString _projectPath;
    QStringList _configFiles;

    ElaScrollPageArea* _scrollArea;
    QVBoxLayout* _snapshotsLayout;
    ElaPushButton* _createButton;
    ElaIconButton* _refreshButton;
    ElaText* _emptyStateText;

    QList<SnapshotCard*> _snapshotCards;
};

#endif // SNAPSHOTPAGE_H
