#include "ConfigPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "ElaTreeView.h"
#include "ElaPlainTextEdit.h"
#include "ElaPushButton.h"
#include "ElaMessageBar.h"
#include "ElaText.h"

namespace Prism {

ConfigPage::ConfigPage(QWidget* parent)
    : BasePage(parent)
{
    setWindowTitle("配置文件管理");
    initUI();
    setupConnections();

    qDebug() << "ConfigPage 初始化完成";
}

ConfigPage::~ConfigPage()
{
}

void ConfigPage::initUI()
{
    // ========================================
    // 创建主分割器（左右布局）
    // ========================================
    _mainSplitter = new QSplitter(Qt::Horizontal, this);

    // ========================================
    // 左侧：配置文件树形列表
    // ========================================
    QWidget* leftWidget = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(5, 5, 5, 5);

    ElaText* treeTitle = new ElaText("配置文件列表", this);
    treeTitle->setTextPixelSize(16);
    leftLayout->addWidget(treeTitle);

    _configTreeView = new ElaTreeView(this);
    _treeModel = new QStandardItemModel(this);
    _treeModel->setHorizontalHeaderLabels({"文件名", "格式"});
    _configTreeView->setModel(_treeModel);
    _configTreeView->setHeaderHidden(false);
    leftLayout->addWidget(_configTreeView);

    // 添加文件按钮
    _addFileButton = new ElaPushButton("添加配置文件", this);
    leftLayout->addWidget(_addFileButton);

    _mainSplitter->addWidget(leftWidget);

    // ========================================
    // 右侧：配置编辑器
    // ========================================
    QWidget* rightWidget = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(5, 5, 5, 5);

    ElaText* editorTitle = new ElaText("配置编辑器", this);
    editorTitle->setTextPixelSize(16);
    rightLayout->addWidget(editorTitle);

    _configEditor = new ElaPlainTextEdit(this);
    _configEditor->setPlaceholderText("请选择左侧的配置文件进行编辑...");
    _configEditor->setReadOnly(false);
    rightLayout->addWidget(_configEditor);

    // 底部按钮栏
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _validateButton = new ElaPushButton("验证格式", this);
    buttonLayout->addWidget(_validateButton);

    _reloadButton = new ElaPushButton("重新加载", this);
    buttonLayout->addWidget(_reloadButton);

    _saveButton = new ElaPushButton("保存", this);
    _saveButton->setEnabled(false);
    buttonLayout->addWidget(_saveButton);

    rightLayout->addLayout(buttonLayout);

    _mainSplitter->addWidget(rightWidget);

    // 设置分割器比例
    _mainSplitter->setStretchFactor(0, 1); // 左侧占 1/4
    _mainSplitter->setStretchFactor(1, 3); // 右侧占 3/4

    // ========================================
    // 组装到页面中央
    // ========================================
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* centerLayout = new QVBoxLayout(centralWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->addWidget(_mainSplitter);

    addCentralWidget(centralWidget);
}

void ConfigPage::setupConnections()
{
    // 树形列表点击事件
    connect(_configTreeView, &ElaTreeView::clicked, this, [this](const QModelIndex& index) {
        if (!index.isValid()) return;

        // 获取文件路径（假设存储在第一列的 UserRole 中）
        QString filePath = _treeModel->itemFromIndex(index.siblingAtColumn(0))->data(Qt::UserRole).toString();
        if (!filePath.isEmpty()) {
            openConfigFile(filePath);
        }
    });

    // 添加文件按钮
    connect(_addFileButton, &ElaPushButton::clicked, this, &ConfigPage::addConfigFile);

    // 编辑器内容变化
    connect(_configEditor, &ElaPlainTextEdit::textChanged, this, [this]() {
        if (!_currentFilePath.isEmpty()) {
            _isModified = true;
            _saveButton->setEnabled(true);
            emit configFileModified(_currentFilePath);
        }
    });

    // 按钮点击
    connect(_saveButton, &ElaPushButton::clicked, this, &ConfigPage::saveCurrentConfig);
    connect(_reloadButton, &ElaPushButton::clicked, this, &ConfigPage::reloadCurrentConfig);
    connect(_validateButton, &ElaPushButton::clicked, this, &ConfigPage::validateConfig);
}

void ConfigPage::setProjectName(const QString& projectName)
{
    _currentProjectName = projectName;
    qDebug() << "ConfigPage 设置项目名称:" << projectName;
}

void ConfigPage::loadConfigFiles(const QStringList& configFiles)
{
    _treeModel->removeRows(0, _treeModel->rowCount());

    for (const QString& filePath : configFiles) {
        QString fileName = filePath.section('/', -1); // 提取文件名
        QString format = detectConfigFormat(filePath);

        QList<QStandardItem*> rowItems;
        QStandardItem* nameItem = new QStandardItem(fileName);
        nameItem->setData(filePath, Qt::UserRole); // 存储完整路径
        nameItem->setEditable(false);

        QStandardItem* formatItem = new QStandardItem(format.toUpper());
        formatItem->setEditable(false);

        rowItems << nameItem << formatItem;
        _treeModel->appendRow(rowItems);

        _filePathToFormat[filePath] = format;
    }

    qDebug() << "加载配置文件列表，共" << configFiles.size() << "个文件";
}

void ConfigPage::openConfigFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                             QString("无法打开文件: %1").arg(filePath), 2000);
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString content = in.readAll();
    file.close();

    _configEditor->setPlainText(content);
    _currentFilePath = filePath;
    _currentFormat = detectConfigFormat(filePath);
    _isModified = false;
    _saveButton->setEnabled(false);

    // TODO: 更新语法高亮
    // updateSyntaxHighlighter(_currentFormat);

    ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                           QString("已打开: %1").arg(filePath.section('/', -1)), 1500);

    qDebug() << "打开配置文件:" << filePath << "格式:" << _currentFormat;
}

void ConfigPage::saveCurrentConfig()
{
    if (_currentFilePath.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "没有打开的配置文件", 1500);
        return;
    }

    QFile file(_currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                             QString("无法保存文件: %1").arg(_currentFilePath), 2000);
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << _configEditor->toPlainText();
    file.close();

    _isModified = false;
    _saveButton->setEnabled(false);

    ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                           "配置文件已保存", 1500);

    emit configFileSaved(_currentFilePath);

    qDebug() << "保存配置文件:" << _currentFilePath;
}

void ConfigPage::reloadCurrentConfig()
{
    if (_currentFilePath.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "没有打开的配置文件", 1500);
        return;
    }

    if (_isModified) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "重新加载确认",
                                      "当前文件已修改，重新加载将丢失未保存的更改。\n确定要继续吗？",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }

    openConfigFile(_currentFilePath);
}

void ConfigPage::validateConfig()
{
    if (_currentFilePath.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "没有打开的配置文件", 1500);
        return;
    }

    // TODO: 使用 ConfigParser 工厂模式验证格式
    // 这里先做简单的格式检测
    QString content = _configEditor->toPlainText();

    if (_currentFormat == "json") {
        // TODO: JSON 验证
        ElaMessageBar::information(ElaMessageBarType::BottomRight, "提示",
                                   "JSON 格式验证功能即将推出", 2000);
    } else if (_currentFormat == "yaml") {
        // TODO: YAML 验证
        ElaMessageBar::information(ElaMessageBarType::BottomRight, "提示",
                                   "YAML 格式验证功能即将推出", 2000);
    } else if (_currentFormat == "ini") {
        // TODO: INI 验证
        ElaMessageBar::information(ElaMessageBarType::BottomRight, "提示",
                                   "INI 格式验证功能即将推出", 2000);
    } else {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "未知的配置格式", 1500);
    }
}

void ConfigPage::addConfigFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择配置文件",
        QDir::homePath(),
        "配置文件 (*.ini *.json *.yaml *.yml *.conf);;所有文件 (*.*)"
    );

    if (!filePath.isEmpty()) {
        // TODO: 将文件添加到当前项目
        ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                               QString("已添加: %1").arg(filePath.section('/', -1)), 1500);

        qDebug() << "添加配置文件:" << filePath;
    }
}

QString ConfigPage::detectConfigFormat(const QString& filePath)
{
    QString suffix = filePath.section('.', -1).toLower();

    if (suffix == "ini" || suffix == "conf") {
        return "ini";
    } else if (suffix == "json") {
        return "json";
    } else if (suffix == "yaml" || suffix == "yml") {
        return "yaml";
    } else {
        return "unknown";
    }
}

void ConfigPage::updateSyntaxHighlighter(const QString& format)
{
    // TODO: 根据格式设置语法高亮器
    // if (_highlighter) {
    //     delete _highlighter;
    // }
    // if (format == "json") {
    //     _highlighter = new JsonHighlighter(_configEditor->document());
    // } else if (format == "yaml") {
    //     _highlighter = new YamlHighlighter(_configEditor->document());
    // }
    qDebug() << "更新语法高亮器:" << format;
}

} // namespace Prism
