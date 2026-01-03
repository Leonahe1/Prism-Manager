#include "CreateSnapshotDialog.h"
#include "core/SnapshotManager.h"
#include "ElaLineEdit.h"
#include "ElaPlainTextEdit.h"
#include "ElaCheckBox.h"
#include "ElaText.h"
#include "ElaScrollArea.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QDir>

CreateSnapshotDialog::CreateSnapshotDialog(const QString& projectPath,
                                         const QStringList& configFiles,
                                         QWidget* parent)
    : ElaContentDialog(parent)
    , _projectPath(projectPath)
    , _configFiles(configFiles)
{
    setupUI();
}

CreateSnapshotDialog::~CreateSnapshotDialog() {
}

void CreateSnapshotDialog::setupUI() {
    setWindowTitle("创建配置快照");
    setWindowModality(Qt::ApplicationModal);

    // 创建主布局
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ========== 快照名称 ==========
    ElaText* nameLabel = new ElaText("快照名称:", this);
    nameLabel->setTextPixelSize(14);
    mainLayout->addWidget(nameLabel);

    _nameEdit = new ElaLineEdit(this);
    _nameEdit->setPlaceholderText(SnapshotManager::instance().generateDefaultSnapshotName(_projectPath));
    _nameEdit->setFixedHeight(35);
    mainLayout->addWidget(_nameEdit);

    // ========== 快照描述 ==========
    ElaText* descLabel = new ElaText("快照描述 (可选):", this);
    descLabel->setTextPixelSize(14);
    mainLayout->addWidget(descLabel);

    _descriptionEdit = new ElaPlainTextEdit(this);
    _descriptionEdit->setPlaceholderText("输入快照描述，例如：用于雷达算法测试的基准配置");
    _descriptionEdit->setFixedHeight(80);
    mainLayout->addWidget(_descriptionEdit);

    // ========== 包含文件 ==========
    ElaText* filesLabel = new ElaText("包含文件:", this);
    filesLabel->setTextPixelSize(14);
    mainLayout->addWidget(filesLabel);

    // 全选复选框
    _selectAllCheckBox = new ElaCheckBox("全选", this);
    _selectAllCheckBox->setChecked(true);
    connect(_selectAllCheckBox, &ElaCheckBox::stateChanged, this, &CreateSnapshotDialog::onSelectAllChanged);
    mainLayout->addWidget(_selectAllCheckBox);

    // 文件列表（使用滚动区域）
    QWidget* filesWidget = new QWidget(this);
    filesWidget->setStyleSheet("background: transparent;");
    _filesLayout = new QVBoxLayout(filesWidget);
    _filesLayout->setSpacing(8);
    _filesLayout->setContentsMargins(10, 5, 10, 5);

    // 添加文件复选框
    for (const QString& filePath : _configFiles) {
        // 显示相对路径
        QString relativePath = QDir(_projectPath).relativeFilePath(filePath);

        ElaCheckBox* checkBox = new ElaCheckBox(relativePath, filesWidget);
        checkBox->setChecked(true);
        checkBox->setProperty("filePath", filePath);  // 存储完整路径
        connect(checkBox, &ElaCheckBox::stateChanged, this, &CreateSnapshotDialog::onFileCheckBoxChanged);

        _fileCheckBoxes.append(checkBox);
        _filesLayout->addWidget(checkBox);
    }

    // 滚动区域
    ElaScrollArea* scrollArea = new ElaScrollArea(this);
    scrollArea->setWidget(filesWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFixedHeight(200);
    mainLayout->addWidget(scrollArea);

    // ========== 提示信息 ==========
    ElaText* hintText = new ElaText(QString("当前项目已有 %1 个快照，最多可创建 20 个快照")
                                   .arg(SnapshotManager::instance().getSnapshotCount(_projectPath)), this);
    hintText->setTextPixelSize(12);
    mainLayout->addWidget(hintText);

    // 设置中央部件
    setCentralWidget(centralWidget);

    // 设置对话框按钮
    setLeftButtonText("取消");
    setMiddleButtonText("");  // 隐藏中间按钮
    setRightButtonText("创建快照");

    // 设置对话框大小
    setFixedSize(500, 600);
}

QString CreateSnapshotDialog::getSnapshotName() const {
    QString name = _nameEdit->text().trimmed();
    if (name.isEmpty()) {
        return SnapshotManager::instance().generateDefaultSnapshotName(_projectPath);
    }
    return name;
}

QString CreateSnapshotDialog::getSnapshotDescription() const {
    return _descriptionEdit->toPlainText().trimmed();
}

QStringList CreateSnapshotDialog::getSelectedFiles() const {
    QStringList selectedFiles;

    for (ElaCheckBox* checkBox : _fileCheckBoxes) {
        if (checkBox->isChecked()) {
            QString filePath = checkBox->property("filePath").toString();
            // 返回相对路径
            QString relativePath = QDir(_projectPath).relativeFilePath(filePath);
            selectedFiles.append(relativePath);
        }
    }

    return selectedFiles;
}

void CreateSnapshotDialog::onSelectAllChanged(int state) {
    bool checked = (state == Qt::Checked);

    // 阻止信号，避免触发 onFileCheckBoxChanged
    for (ElaCheckBox* checkBox : _fileCheckBoxes) {
        checkBox->blockSignals(true);
        checkBox->setChecked(checked);
        checkBox->blockSignals(false);
    }
}

void CreateSnapshotDialog::onFileCheckBoxChanged(int state) {
    Q_UNUSED(state);
    updateSelectAllState();
}

void CreateSnapshotDialog::updateSelectAllState() {
    int checkedCount = 0;
    int totalCount = _fileCheckBoxes.size();

    for (ElaCheckBox* checkBox : _fileCheckBoxes) {
        if (checkBox->isChecked()) {
            checkedCount++;
        }
    }

    // 更新全选复选框状态
    _selectAllCheckBox->blockSignals(true);
    if (checkedCount == 0) {
        _selectAllCheckBox->setCheckState(Qt::Unchecked);
    } else if (checkedCount == totalCount) {
        _selectAllCheckBox->setCheckState(Qt::Checked);
    } else {
        _selectAllCheckBox->setCheckState(Qt::PartiallyChecked);
    }
    _selectAllCheckBox->blockSignals(false);
}
