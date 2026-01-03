#include "SnapshotPage.h"
#include "CreateSnapshotDialog.h"
#include "MainWindow.h"
#include "ElaScrollPageArea.h"
#include "ElaText.h"
#include "ElaPushButton.h"
#include "ElaIconButton.h"
#include "ElaMessageBar.h"
#include "ElaContentDialog.h"
#include "ElaLineEdit.h"
#include "ElaPlainTextEdit.h"
#include "ElaTheme.h"
#include "ElaScrollArea.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QDateTime>
#include <QDebug>
#include <QScrollArea>

// ========== SnapshotCard 实现 ==========

SnapshotCard::SnapshotCard(const SnapshotMetadata& metadata,
                          const QString& projectPath,
                          QWidget* parent)
    : QWidget(parent)
    , _metadata(metadata)
    , _projectPath(projectPath)
{
    setupUI();

    // 监听主题变化
    connect(ElaTheme::getInstance(), &ElaTheme::themeModeChanged, this, &SnapshotCard::updateTheme);
    updateTheme();
}

void SnapshotCard::setupUI() {
    // 设置卡片的尺寸策略 - 水平扩展，垂直根据内容自适应
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(0);

    // 创建卡片容器
    _cardWidget = new QWidget(this);
    _cardWidget->setObjectName("SnapshotCard");
    _cardWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QVBoxLayout* cardLayout = new QVBoxLayout(_cardWidget);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    cardLayout->setSpacing(8);

    // ========== 标题行 ==========
    QHBoxLayout* titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(10);

    // 快照名称
    _nameLabel = new ElaText(_metadata.name, _cardWidget);
    _nameLabel->setTextPixelSize(16);
    titleLayout->addWidget(_nameLabel);

    // 激活状态标识
    if (_metadata.isActive) {
        ElaText* activeLabel = new ElaText("🟢 当前激活", _cardWidget);
        activeLabel->setTextPixelSize(12);
        titleLayout->addWidget(activeLabel);
    }

    titleLayout->addStretch();

    // 操作按钮
    _detailsButton = new ElaIconButton(ElaIconType::Eye, _cardWidget);
    _detailsButton->setFixedSize(28, 28);
    _detailsButton->setToolTip("查看详情");
    connect(_detailsButton, &ElaIconButton::clicked, this, [this]() {
        emit viewDetailsRequested(_metadata.id);
    });
    titleLayout->addWidget(_detailsButton);

    _renameButton = new ElaIconButton(ElaIconType::Pencil, _cardWidget);
    _renameButton->setFixedSize(28, 28);
    _renameButton->setToolTip("重命名");
    connect(_renameButton, &ElaIconButton::clicked, this, [this]() {
        emit renameRequested(_metadata.id);
    });
    titleLayout->addWidget(_renameButton);

    _deleteButton = new ElaIconButton(ElaIconType::Trash, _cardWidget);
    _deleteButton->setFixedSize(28, 28);
    _deleteButton->setToolTip("删除");
    connect(_deleteButton, &ElaIconButton::clicked, this, [this]() {
        emit deleteRequested(_metadata.id);
    });
    titleLayout->addWidget(_deleteButton);

    cardLayout->addLayout(titleLayout);

    // ========== 信息行 ==========
    _timeLabel = new ElaText(QString("创建时间: %1").arg(_metadata.createTime.toString("yyyy-MM-dd HH:mm:ss")), _cardWidget);
    _timeLabel->setTextPixelSize(13);
    cardLayout->addWidget(_timeLabel);

    _filesLabel = new ElaText(QString("包含文件: %1 个").arg(_metadata.files.size()), _cardWidget);
    _filesLabel->setTextPixelSize(13);
    cardLayout->addWidget(_filesLabel);

    // 描述
    if (!_metadata.description.isEmpty()) {
        _descLabel = new ElaText(_metadata.description, _cardWidget);
        _descLabel->setTextPixelSize(13);
        _descLabel->setWordWrap(true);
        cardLayout->addWidget(_descLabel);
    }

    // ========== 应用按钮 ==========
    if (!_metadata.isActive) {
        _applyButton = new ElaPushButton("应用此快照", _cardWidget);
        _applyButton->setFixedHeight(32);
        connect(_applyButton, &ElaPushButton::clicked, this, [this]() {
            emit applyRequested(_metadata.id);
        });
        cardLayout->addWidget(_applyButton);
    }

    mainLayout->addWidget(_cardWidget);
}

void SnapshotCard::updateTheme() {
    bool isDark = ElaTheme::getInstance()->getThemeMode() == ElaThemeType::Dark;

    QString cardStyle = QString(
        "QWidget#SnapshotCard {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 8px;"
        "}"
    ).arg(isDark ? "#2D2D2D" : "#FFFFFF",
          isDark ? "#404040" : "#E0E0E0");

    _cardWidget->setStyleSheet(cardStyle);
}

// ========== SnapshotPage 实现 ==========

SnapshotPage::SnapshotPage(QWidget* parent)
    : BasePage(parent)
{
    setWindowTitle("配置快照管理");
    initUI();
}

SnapshotPage::~SnapshotPage() {
}

void SnapshotPage::initUI() {
    // 创建中央容器
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);

    // ========== 标题栏区域 ==========
    ElaScrollPageArea* headerArea = new ElaScrollPageArea(centralWidget);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerArea);
    headerLayout->setContentsMargins(15, 10, 15, 10);

    ElaText* titleText = new ElaText("配置快照管理", headerArea);
    titleText->setTextPixelSize(18);
    headerLayout->addWidget(titleText);

    headerLayout->addStretch();

    // 刷新按钮
    _refreshButton = new ElaIconButton(ElaIconType::ArrowsRotate, headerArea);
    _refreshButton->setFixedSize(32, 32);
    _refreshButton->setToolTip("刷新");
    connect(_refreshButton, &ElaIconButton::clicked, this, &SnapshotPage::onRefresh);
    headerLayout->addWidget(_refreshButton);

    // 创建快照按钮
    _createButton = new ElaPushButton("+ 创建快照", headerArea);
    _createButton->setFixedSize(110, 32);
    connect(_createButton, &ElaPushButton::clicked, this, &SnapshotPage::onCreateSnapshot);
    headerLayout->addWidget(_createButton);

    mainLayout->addWidget(headerArea);

    // ========== 快照列表滚动区域 (使用 ElaScrollArea) ==========
    _scrollArea = new ElaScrollArea(centralWidget);
    _scrollArea->setWidgetResizable(true);
    _scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _scrollArea->setIsGrabGesture(true);

    // 滚动区域内的容器
    _scrollContainer = new QWidget(_scrollArea);
    _scrollContainer->setStyleSheet("background: transparent;");
    _snapshotsLayout = new QVBoxLayout(_scrollContainer);
    _snapshotsLayout->setContentsMargins(5, 5, 5, 5);
    _snapshotsLayout->setSpacing(10);
    _snapshotsLayout->addStretch();

    _scrollArea->setWidget(_scrollContainer);
    mainLayout->addWidget(_scrollArea, 1);

    // ========== 空状态提示 ==========
    _emptyStateText = new ElaText("暂无快照，点击上方按钮创建第一个快照", centralWidget);
    _emptyStateText->setTextPixelSize(14);
    _emptyStateText->setAlignment(Qt::AlignCenter);
    _emptyStateText->setVisible(false);
    mainLayout->addWidget(_emptyStateText, 1, Qt::AlignCenter);

    centralWidget->setWindowTitle("配置快照");
    // 使用 ElaScrollPage 的 addCentralWidget 方法添加内容
    addCentralWidget(centralWidget, true, false, 0);
}

void SnapshotPage::setProject(const QString& projectPath, const QStringList& configFiles) {
    _projectPath = projectPath;
    _configFiles = configFiles;

    refreshSnapshots();
}

void SnapshotPage::refreshSnapshots() {
    if (_projectPath.isEmpty()) {
        showEmptyState();
        return;
    }

    // 清除旧的卡片
    clearSnapshotCards();

    // 获取快照列表
    QList<SnapshotMetadata> snapshots = SnapshotManager::instance().getSnapshots(_projectPath);

    if (snapshots.isEmpty()) {
        showEmptyState();
        return;
    }

    // 隐藏空状态提示，显示卡片列表
    _emptyStateText->setVisible(false);
    _scrollArea->setVisible(true);

    // 创建快照卡片
    for (const SnapshotMetadata& metadata : snapshots) {
        SnapshotCard* card = new SnapshotCard(metadata, _projectPath, _scrollContainer);

        connect(card, &SnapshotCard::applyRequested, this, &SnapshotPage::onApplySnapshot);
        connect(card, &SnapshotCard::renameRequested, this, &SnapshotPage::onRenameSnapshot);
        connect(card, &SnapshotCard::deleteRequested, this, &SnapshotPage::onDeleteSnapshot);
        connect(card, &SnapshotCard::viewDetailsRequested, this, &SnapshotPage::onViewDetails);

        _snapshotCards.append(card);
        _snapshotsLayout->insertWidget(_snapshotsLayout->count() - 1, card);
    }

    qDebug() << "[SnapshotPage] 刷新快照列表，共" << snapshots.size() << "个快照";
}

void SnapshotPage::onCreateSnapshot() {
    if (_projectPath.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "提示", "请先选择项目", 2000);
        return;
    }

    if (_configFiles.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "提示", "当前项目没有配置文件", 2000);
        return;
    }

    // 检查快照数量限制
    if (SnapshotManager::instance().getSnapshotCount(_projectPath) >= 20) {
        ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                            "快照数量已达上限（20个），请删除旧快照后再创建", 3000);
        return;
    }

    // 显示创建快照对话框
    CreateSnapshotDialog* dialog = new CreateSnapshotDialog(_projectPath, _configFiles, this);

    connect(dialog, &ElaContentDialog::rightButtonClicked, this, [this, dialog]() {
        QString name = dialog->getSnapshotName();
        QString description = dialog->getSnapshotDescription();
        QStringList selectedFiles = dialog->getSelectedFiles();

        if (selectedFiles.isEmpty()) {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "提示", "请至少选择一个配置文件", 2000);
            return;
        }

        // 创建快照
        QString errorMsg;
        QString snapshotId = SnapshotManager::instance().createSnapshot(
            _projectPath, name, description, selectedFiles, errorMsg);

        if (snapshotId.isEmpty()) {
            ElaMessageBar::error(ElaMessageBarType::BottomRight, "创建失败", errorMsg, 3000);

            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("ERROR", QString("创建快照失败: %1").arg(errorMsg));
            }
        } else {
            ElaMessageBar::success(ElaMessageBarType::BottomRight, "创建成功",
                                  QString("快照 \"%1\" 已创建").arg(name), 2000);

            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("SUCCESS", QString("创建快照: %1 (包含 %2 个文件)")
                                  .arg(name).arg(selectedFiles.size()));
            }

            // 刷新列表
            refreshSnapshots();
        }

        dialog->close();
    });

    dialog->show();
}

void SnapshotPage::onApplySnapshot(const QString& snapshotId) {
    // 显示确认对话框
    auto* confirmDialog = new ElaContentDialog(this);
    confirmDialog->setWindowTitle("应用快照");
    confirmDialog->setFixedHeight(150);
    QWidget* contentWidget = new QWidget(confirmDialog);
    QVBoxLayout* layout = new QVBoxLayout(contentWidget);

    ElaText* warningText = new ElaText("⚠️ 应用快照会覆盖当前配置文件", confirmDialog);
    warningText->setTextPixelSize(14);
    layout->addWidget(warningText);

    ElaText* hintText = new ElaText("请确保当前配置已保存，应用快照后将无法撤销", confirmDialog);
    hintText->setTextPixelSize(12);
    layout->addWidget(hintText);

    confirmDialog->setCentralWidget(contentWidget);
    confirmDialog->setLeftButtonText("取消");
    confirmDialog->setMiddleButtonText("");
    confirmDialog->setRightButtonText("应用");

    QList<ElaPushButton*> buttons = confirmDialog->findChildren<ElaPushButton*>();
    if (buttons.size() >= 3) {
        // 隐藏中间按钮
        buttons[1]->setVisible(false);
    }
    connect(confirmDialog, &ElaContentDialog::rightButtonClicked, this, [this, snapshotId, confirmDialog]() {
        QString errorMsg;
        if (SnapshotManager::instance().applySnapshot(_projectPath, snapshotId, errorMsg)) {
            ElaMessageBar::success(ElaMessageBarType::BottomRight, "应用成功",
                                  "配置已恢复到快照状态", 2000);

            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("SUCCESS", QString("应用快照: %1").arg(snapshotId));
                // 通知主窗口重新加载配置
                mainWin->reloadCurrentProject();
            }

            refreshSnapshots();
        } else {
            ElaMessageBar::error(ElaMessageBarType::BottomRight, "应用失败", errorMsg, 3000);

            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("ERROR", QString("应用快照失败: %1").arg(errorMsg));
            }
        }

        confirmDialog->close();
    });

    confirmDialog->show();
}

void SnapshotPage::onRenameSnapshot(const QString& snapshotId) {
    // 获取当前快照元数据
    SnapshotMetadata metadata;
    for (SnapshotCard* card : _snapshotCards) {
        // 通过卡片找到对应的元数据（简化处理，实际应该从 SnapshotManager 获取）
        QList<SnapshotMetadata> snapshots = SnapshotManager::instance().getSnapshots(_projectPath);
        for (const SnapshotMetadata& snap : snapshots) {
            if (snap.id == snapshotId) {
                metadata = snap;
                break;
            }
        }
    }

    // 显示重命名对话框
    auto* renameDialog = new ElaContentDialog(this);
    renameDialog->setWindowTitle("重命名快照");
    QList<ElaPushButton *> buttons = renameDialog->findChildren<ElaPushButton*>();
    if (buttons.size() >= 3)
    {
        buttons[1]->setVisible(false);
    }
    QWidget* contentWidget = new QWidget(renameDialog);
    QVBoxLayout* layout = new QVBoxLayout(contentWidget);

    ElaText* nameLabel = new ElaText("快照名称:", renameDialog);
    nameLabel->setTextPixelSize(14);
    layout->addWidget(nameLabel);

    ElaLineEdit* nameEdit = new ElaLineEdit(renameDialog);
    nameEdit->setText(metadata.name);
    nameEdit->setFixedHeight(35);
    layout->addWidget(nameEdit);

    ElaText* descLabel = new ElaText("快照描述:", renameDialog);
    descLabel->setTextPixelSize(14);
    layout->addWidget(descLabel);

    ElaPlainTextEdit* descEdit = new ElaPlainTextEdit(renameDialog);
    descEdit->setPlainText(metadata.description);
    descEdit->setFixedHeight(80);
    layout->addWidget(descEdit);

    renameDialog->setCentralWidget(contentWidget);
    renameDialog->setLeftButtonText("取消");
    renameDialog->setMiddleButtonText("");
    renameDialog->setRightButtonText("确定");

    connect(renameDialog, &ElaContentDialog::rightButtonClicked, this, [this, snapshotId, nameEdit, descEdit, renameDialog]() {
        QString newName = nameEdit->text().trimmed();
        QString newDesc = descEdit->toPlainText().trimmed();

        if (newName.isEmpty()) {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "提示", "快照名称不能为空", 2000);
            return;
        }

        if (SnapshotManager::instance().renameSnapshot(_projectPath, snapshotId, newName, newDesc)) {
            ElaMessageBar::success(ElaMessageBarType::BottomRight, "重命名成功", "", 2000);

            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("SUCCESS", QString("重命名快照: %1").arg(newName));
            }

            refreshSnapshots();
        } else {
            ElaMessageBar::error(ElaMessageBarType::BottomRight, "重命名失败", "", 2000);
        }

        renameDialog->close();
    });

    renameDialog->show();
}

void SnapshotPage::onDeleteSnapshot(const QString& snapshotId) {
    // 显示确认对话框
    auto* confirmDialog = new ElaContentDialog(this);
    confirmDialog->setWindowTitle("删除快照");

    QWidget* contentWidget = new QWidget(confirmDialog);
    QVBoxLayout* layout = new QVBoxLayout(contentWidget);

    ElaText* warningText = new ElaText("⚠️ 确定要删除此快照吗？", confirmDialog);
    warningText->setTextPixelSize(14);
    layout->addWidget(warningText);

    ElaText* hintText = new ElaText("删除后将无法恢复", confirmDialog);
    hintText->setTextPixelSize(12);
    layout->addWidget(hintText);

    confirmDialog->setCentralWidget(contentWidget);
    confirmDialog->setLeftButtonText("取消");
    confirmDialog->setMiddleButtonText("");
    confirmDialog->setRightButtonText("删除");

    connect(confirmDialog, &ElaContentDialog::rightButtonClicked, this, [this, snapshotId, confirmDialog]() {
        if (SnapshotManager::instance().deleteSnapshot(_projectPath, snapshotId)) {
            ElaMessageBar::success(ElaMessageBarType::BottomRight, "删除成功", "", 2000);

            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("SUCCESS", QString("删除快照: %1").arg(snapshotId));
            }

            refreshSnapshots();
        } else {
            ElaMessageBar::error(ElaMessageBarType::BottomRight, "删除失败", "", 2000);
        }

        confirmDialog->close();
    });

    confirmDialog->show();
}

void SnapshotPage::onViewDetails(const QString& snapshotId) {
    // 获取快照元数据
    QList<SnapshotMetadata> snapshots = SnapshotManager::instance().getSnapshots(_projectPath);
    SnapshotMetadata metadata;

    for (const SnapshotMetadata& snap : snapshots) {
        if (snap.id == snapshotId) {
            metadata = snap;
            break;
        }
    }

    if (metadata.id.isEmpty()) {
        return;
    }

    // 显示详情对话框
    auto* detailsDialog = new ElaContentDialog(this);
    detailsDialog->setWindowTitle("快照详情");
    detailsDialog->setMinimumHeight(400);
    QList<ElaPushButton *> buttons = detailsDialog->findChildren<ElaPushButton*>();
    if (buttons.size() >= 3)
    {
        buttons[0]->setVisible(false);
        buttons[1]->setVisible(false);
    }
    QWidget* contentWidget = new QWidget(detailsDialog);
    QVBoxLayout* layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(12);

    // 快照名称
    ElaText* nameText = new ElaText(QString("快照名称: %1").arg(metadata.name), contentWidget);
    nameText->setTextPixelSize(15);
    layout->addWidget(nameText);

    // 创建时间
    ElaText* timeText = new ElaText(QString("创建时间: %1").arg(metadata.createTime.toString("yyyy-MM-dd HH:mm:ss")), contentWidget);
    timeText->setTextPixelSize(13);
    layout->addWidget(timeText);

    // 快照ID
    ElaText* idText = new ElaText(QString("快照ID: %1").arg(metadata.id), contentWidget);
    idText->setTextPixelSize(13);
    layout->addWidget(idText);

    // 描述
    if (!metadata.description.isEmpty()) {
        ElaText* descText = new ElaText(QString("描述: %1").arg(metadata.description), contentWidget);
        descText->setTextPixelSize(13);
        descText->setWordWrap(true);
        layout->addWidget(descText);
    }

    // 分隔
    layout->addSpacing(5);

    // 包含的文件列表
    ElaText* filesTitle = new ElaText(QString("包含文件 (%1 个):").arg(metadata.files.size()), contentWidget);
    filesTitle->setTextPixelSize(14);
    layout->addWidget(filesTitle);

    // 文件列表使用滚动区域（防止文件太多）
    ElaScrollArea* fileScrollArea = new ElaScrollArea(contentWidget);
    fileScrollArea->setWidgetResizable(true);
    fileScrollArea->setMaximumHeight(150);
    fileScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget* fileListWidget = new QWidget(fileScrollArea);
    fileListWidget->setStyleSheet("background: transparent;");
    QVBoxLayout* fileListLayout = new QVBoxLayout(fileListWidget);
    fileListLayout->setContentsMargins(5, 5, 5, 5);
    fileListLayout->setSpacing(4);

    for (const QString& file : metadata.files) {
        ElaText* fileText = new ElaText(QString("• %1").arg(file), fileListWidget);
        fileText->setTextPixelSize(12);
        fileListLayout->addWidget(fileText);
    }
    fileListLayout->addStretch();

    fileScrollArea->setWidget(fileListWidget);
    layout->addWidget(fileScrollArea);

    detailsDialog->setCentralWidget(contentWidget);
    detailsDialog->setLeftButtonText("");
    detailsDialog->setMiddleButtonText("");
    detailsDialog->setRightButtonText("关闭");

    detailsDialog->show();
}

void SnapshotPage::onRefresh() {
    refreshSnapshots();
    ElaMessageBar::success(ElaMessageBarType::BottomRight, "刷新成功", "", 1000);
}

void SnapshotPage::clearSnapshotCards() {
    for (SnapshotCard* card : _snapshotCards) {
        _snapshotsLayout->removeWidget(card);
        card->deleteLater();
    }
    _snapshotCards.clear();
}

void SnapshotPage::showEmptyState() {
    _scrollArea->setVisible(false);
    _emptyStateText->setVisible(true);
}
