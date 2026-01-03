#include "ProcessEditDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QUuid>

#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include "ElaText.h"
#include "ElaMessageBar.h"
#include "ElaToggleSwitch.h"

namespace Prism {

ProcessEditDialog::ProcessEditDialog(QWidget* parent, bool isEdit)
    : ElaDialog(parent)
    , _isEditMode(isEdit)
{
    setWindowTitle(isEdit ? "编辑进程" : "添加进程");
    setWindowModality(Qt::WindowModal);
    resize(520, 360);

    setupUI();
    setupConnections();
}

ProcessEditDialog::~ProcessEditDialog() = default;

void ProcessEditDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 进程名称
    QHBoxLayout* nameLayout = new QHBoxLayout();
    ElaText* nameLabel = new ElaText("进程名称:", this);
    nameLabel->setTextPixelSize(14);
    nameLabel->setFixedWidth(70);
    _nameEdit = new ElaLineEdit(this);
    _nameEdit->setPlaceholderText("输入进程名称，用于标识");
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(_nameEdit);
    mainLayout->addLayout(nameLayout);

    // 程序路径
    QHBoxLayout* programLayout = new QHBoxLayout();
    ElaText* programLabel = new ElaText("程序路径:", this);
    programLabel->setTextPixelSize(14);
    programLabel->setFixedWidth(70);
    _programEdit = new ElaLineEdit(this);
    _programEdit->setPlaceholderText("选择或输入可执行文件路径");
    _browseButton = new ElaPushButton("浏览", this);
    _browseButton->setFixedWidth(60);
    programLayout->addWidget(programLabel);
    programLayout->addWidget(_programEdit);
    programLayout->addWidget(_browseButton);
    mainLayout->addLayout(programLayout);

    // 命令参数
    QHBoxLayout* argsLayout = new QHBoxLayout();
    ElaText* argsLabel = new ElaText("命令参数:", this);
    argsLabel->setTextPixelSize(14);
    argsLabel->setFixedWidth(70);
    _argumentsEdit = new ElaLineEdit(this);
    _argumentsEdit->setPlaceholderText("命令行参数（可选）");
    argsLayout->addWidget(argsLabel);
    argsLayout->addWidget(_argumentsEdit);
    mainLayout->addLayout(argsLayout);

    // 工作目录
    QHBoxLayout* workDirLayout = new QHBoxLayout();
    ElaText* workDirLabel = new ElaText("工作目录:", this);
    workDirLabel->setTextPixelSize(14);
    workDirLabel->setFixedWidth(70);
    _workDirEdit = new ElaLineEdit(this);
    _workDirEdit->setPlaceholderText("工作目录（可选，默认为程序所在目录）");
    _browseWorkDirButton = new ElaPushButton("浏览", this);
    _browseWorkDirButton->setFixedWidth(60);
    workDirLayout->addWidget(workDirLabel);
    workDirLayout->addWidget(_workDirEdit);
    workDirLayout->addWidget(_browseWorkDirButton);
    mainLayout->addLayout(workDirLayout);

    // 显示控制台开关
    QHBoxLayout* consoleLayout = new QHBoxLayout();
    ElaText* consoleLabel = new ElaText("显示控制台:", this);
    consoleLabel->setTextPixelSize(14);
    consoleLabel->setFixedWidth(70);
    _showConsoleSwitch = new ElaToggleSwitch(this);
    ElaText* consoleHint = new ElaText("开启后程序将显示独立的控制台窗口，但日志不会在此处显示", this);
    consoleHint->setTextPixelSize(11);
    consoleHint->setStyleSheet("color: #888888;");
    consoleLayout->addWidget(consoleLabel);
    consoleLayout->addWidget(_showConsoleSwitch);
    consoleLayout->addWidget(consoleHint, 1);
    mainLayout->addLayout(consoleLayout);

    // 弹性空间
    mainLayout->addStretch();

    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    _cancelButton = new ElaPushButton("取消", this);
    _cancelButton->setFixedWidth(80);
    _confirmButton = new ElaPushButton(_isEditMode ? "保存" : "添加", this);
    _confirmButton->setFixedWidth(80);
    buttonLayout->addWidget(_cancelButton);
    buttonLayout->addWidget(_confirmButton);
    mainLayout->addLayout(buttonLayout);
}

void ProcessEditDialog::setupConnections()
{
    connect(_browseButton, &ElaPushButton::clicked, this, &ProcessEditDialog::onBrowseClicked);
    connect(_browseWorkDirButton, &ElaPushButton::clicked, this, &ProcessEditDialog::onBrowseWorkDirClicked);
    connect(_confirmButton, &ElaPushButton::clicked, this, &ProcessEditDialog::onConfirmClicked);
    connect(_cancelButton, &ElaPushButton::clicked, this, &ProcessEditDialog::onCancelClicked);
}

void ProcessEditDialog::setConfig(const ProcessConfig& config)
{
    _configId = config.id;
    _nameEdit->setText(config.name);
    _programEdit->setText(config.program);
    _argumentsEdit->setText(config.arguments);
    _workDirEdit->setText(config.workingDirectory);
    _showConsoleSwitch->setIsToggled(config.showConsole);
}

ProcessConfig ProcessEditDialog::getConfig() const
{
    ProcessConfig config;
    config.id = _configId.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : _configId;
    config.name = _nameEdit->text().trimmed();
    config.program = _programEdit->text().trimmed();
    config.arguments = _argumentsEdit->text().trimmed();
    config.workingDirectory = _workDirEdit->text().trimmed();
    config.showConsole = _showConsoleSwitch->getIsToggled();
    return config;
}

void ProcessEditDialog::onBrowseClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择可执行程序",
        QString(),
#ifdef Q_OS_WIN
        "可执行文件 (*.exe *.bat *.cmd);;所有文件 (*.*)"
#else
        "可执行文件 (*);;所有文件 (*.*)"
#endif
    );

    if (!fileName.isEmpty()) {
        _programEdit->setText(fileName);

        // 如果工作目录为空，自动填充程序所在目录
        if (_workDirEdit->text().trimmed().isEmpty()) {
            QFileInfo fileInfo(fileName);
            _workDirEdit->setText(fileInfo.absolutePath());
        }

        // 如果名称为空，自动填充程序名
        if (_nameEdit->text().trimmed().isEmpty()) {
            QFileInfo fileInfo(fileName);
            _nameEdit->setText(fileInfo.baseName());
        }
    }
}

void ProcessEditDialog::onBrowseWorkDirClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "选择工作目录",
        _workDirEdit->text()
    );

    if (!dir.isEmpty()) {
        _workDirEdit->setText(dir);
    }
}

void ProcessEditDialog::onConfirmClicked()
{
    if (validateInput()) {
        accept();
    }
}

void ProcessEditDialog::onCancelClicked()
{
    reject();
}

bool ProcessEditDialog::validateInput()
{
    if (_nameEdit->text().trimmed().isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "请输入进程名称", 1500, this);
        _nameEdit->setFocus();
        return false;
    }

    if (_programEdit->text().trimmed().isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "请选择或输入程序路径", 1500, this);
        _programEdit->setFocus();
        return false;
    }

    return true;
}

} // namespace Prism
