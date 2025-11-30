#include "ConfigPage.h"
#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStackedWidget>
#include <QScrollArea>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "ElaTreeView.h"
#include "ElaPushButton.h"
#include "ElaMessageBar.h"
#include "ElaPlainTextEdit.h"
#include "ElaText.h"
#include "ElaRadioButton.h"
#include "ElaLineEdit.h"
#include "ElaToggleSwitch.h"
#include "ElaSpinBox.h"
#include "ElaDoubleSpinBox.h"
#include "ElaScrollArea.h"
#include "ElaScrollPageArea.h"
#include "ElaTheme.h"
#include "JsonParser.h"

#ifdef YAML_CPP_AVAILABLE
#include <yaml-cpp/yaml.h>
#endif

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
    // 右侧：配置编辑器（双模式）
    // ========================================
    QWidget* rightWidget = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(5, 5, 5, 5);

    // 标题和模式切换
    QHBoxLayout* headerLayout = new QHBoxLayout();
    ElaText* editorTitle = new ElaText("配置编辑器", this);
    editorTitle->setTextPixelSize(16);
    headerLayout->addWidget(editorTitle);
    headerLayout->addStretch();

    // 模式切换按钮（默认组件模式）
    _sourceModeBtn = new ElaRadioButton("源码模式", this);
    _formModeBtn = new ElaRadioButton("组件模式", this);
    _formModeBtn->setChecked(true);  // 默认组件模式
    headerLayout->addWidget(_sourceModeBtn);
    headerLayout->addWidget(_formModeBtn);

    rightLayout->addLayout(headerLayout);

    // ========================================
    // 编辑器堆栈（切换源码/组件模式）
    // ========================================
    _editorStack = new QStackedWidget(this);
    // 设置初始背景色
    bool isDark = eTheme->getThemeMode() == ElaThemeType::Dark;
    _editorStack->setStyleSheet(isDark
        ? "QStackedWidget { background-color: #1a1a1a; }"
        : "QStackedWidget { background-color: #f5f5f5; }");

    // 源码模式编辑器
    _configEditor = new ElaPlainTextEdit(this);
    _configEditor->setPlaceholderText("请选择左侧的配置文件进行编辑...");
    _configEditor->setReadOnly(false);
    _editorStack->addWidget(_configEditor);

    // 组件模式容器
    _formScrollArea = new QScrollArea(this);
    _formScrollArea->setWidgetResizable(true);
    _formScrollArea->setFrameShape(QFrame::NoFrame);
    _formScrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    _formContainer = new QWidget(this);
    _formContainer->setStyleSheet("background: transparent;");
    _formLayout = new QVBoxLayout(_formContainer);
    _formLayout->setContentsMargins(10, 10, 10, 10);
    _formLayout->setSpacing(15);
    _formLayout->addStretch();

    _formScrollArea->setWidget(_formContainer);
    _editorStack->addWidget(_formScrollArea);

    // 默认显示组件模式
    _editorStack->setCurrentIndex(1);

    rightLayout->addWidget(_editorStack);

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
    _mainSplitter->setStretchFactor(0, 1);
    _mainSplitter->setStretchFactor(1, 3);

    // ========================================
    // 组装到页面中央
    // ========================================
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("配置文件");
    QVBoxLayout* centerLayout = new QVBoxLayout(centralWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->addWidget(_mainSplitter);

    addCentralWidget(centralWidget, true, false, 0);
}

void ConfigPage::setupConnections()
{
    // 树形列表点击事件
    connect(_configTreeView, &ElaTreeView::clicked, this, [this](const QModelIndex& index) {
        if (!index.isValid()) return;

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

    // 模式切换
    connect(_sourceModeBtn, &ElaRadioButton::clicked, this, [this]() {
        switchEditMode(EditMode::Source);
    });
    connect(_formModeBtn, &ElaRadioButton::clicked, this, [this]() {
        switchEditMode(EditMode::Form);
    });

    // 按钮点击
    connect(_saveButton, &ElaPushButton::clicked, this, &ConfigPage::saveCurrentConfig);
    connect(_reloadButton, &ElaPushButton::clicked, this, &ConfigPage::reloadCurrentConfig);
    connect(_validateButton, &ElaPushButton::clicked, this, &ConfigPage::validateConfig);

    // 主题变化
    connect(eTheme, &ElaTheme::themeModeChanged, this, &ConfigPage::onThemeChanged);
}

void ConfigPage::switchEditMode(EditMode mode)
{
    if (_currentMode == mode) return;

    if (mode == EditMode::Form) {
        // 切换到组件模式：从源码同步到表单
        syncSourceToForm();
        _editorStack->setCurrentIndex(1);
    } else {
        // 切换到源码模式：从表单同步到源码
        syncFormToSource();
        _editorStack->setCurrentIndex(0);
    }

    _currentMode = mode;
    qDebug() << "切换编辑模式:" << (mode == EditMode::Source ? "源码" : "组件");
}

void ConfigPage::syncSourceToForm()
{
    clearFormWidgets();

    if (_currentFilePath.isEmpty()) {
        return;
    }

    if (_currentFormat == "ini") {
        buildIniForm();
    } else if (_currentFormat == "yaml") {
        buildYamlForm();
    } else if (_currentFormat == "json") {
        buildJsonForm();
    } else {
        ElaText* placeholder = new ElaText("该格式暂不支持组件模式，请使用源码模式编辑", _formContainer);
        placeholder->setTextPixelSize(14);
        placeholder->setAlignment(Qt::AlignCenter);
        _formLayout->insertWidget(0, placeholder);
    }
}

void ConfigPage::syncFormToSource()
{
    if (_formWidgetMap.isEmpty()) {
        return;
    }

    if (_currentFormat == "ini") {
        collectIniFormToSource();
    } else if (_currentFormat == "yaml") {
        collectYamlFormToSource();
    } else if (_currentFormat == "json") {
        collectJsonFormToSource();
    }
}

// ========================================
// INI 格式处理
// ========================================
QMap<QString, QMap<QString, ConfigItem>> ConfigPage::parseIniContent(const QString& content)
{
    QMap<QString, QMap<QString, ConfigItem>> result;
    QString currentSection;

    QStringList lines = content.split('\n');
    QRegularExpression sectionRegex(R"(\[(.+)\])");
    QRegularExpression keyValueRegex(R"(^([^=]+)=(.*)$)");

    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();

        if (trimmedLine.isEmpty() || trimmedLine.startsWith(';') || trimmedLine.startsWith('#')) {
            continue;
        }

        QRegularExpressionMatch sectionMatch = sectionRegex.match(trimmedLine);
        if (sectionMatch.hasMatch()) {
            currentSection = sectionMatch.captured(1);
            if (!result.contains(currentSection)) {
                result[currentSection] = QMap<QString, ConfigItem>();
            }
            continue;
        }

        QRegularExpressionMatch kvMatch = keyValueRegex.match(trimmedLine);
        if (kvMatch.hasMatch()) {
            QString key = kvMatch.captured(1).trimmed();
            QString valueStr = kvMatch.captured(2).trimmed();

            // 类型检测
            ConfigValueType type = ConfigValueType::String;

            // 检测布尔值
            QString lowerValue = valueStr.toLower();
            if (lowerValue == "true" || lowerValue == "false" ||
                lowerValue == "yes" || lowerValue == "no" ||
                lowerValue == "on" || lowerValue == "off" ||
                lowerValue == "1" || lowerValue == "0") {
                // 对于 0 和 1，进一步检查是否真的是布尔值
                if (lowerValue == "1" || lowerValue == "0") {
                    // 只有在键名包含 enabled, active, flag 等关键字时才认为是布尔
                    QString lowerKey = key.toLower();
                    if (lowerKey.contains("enable") || lowerKey.contains("active") ||
                        lowerKey.contains("flag") || lowerKey.contains("debug") ||
                        lowerKey.contains("verbose") || lowerKey.contains("use")) {
                        type = ConfigValueType::Boolean;
                    }
                } else {
                    type = ConfigValueType::Boolean;
                }
            }
            // 检测整数（如果不是布尔值）
            else {
                bool isInt = false;
                valueStr.toInt(&isInt);
                if (isInt) {
                    type = ConfigValueType::Integer;
                } else {
                    // 检测浮点数
                    bool isDouble = false;
                    valueStr.toDouble(&isDouble);
                    if (isDouble && valueStr.contains('.')) {
                        type = ConfigValueType::Double;
                    }
                }
            }

            result[currentSection][key] = ConfigItem(valueStr, type);
        }
    }

    return result;
}

void ConfigPage::buildIniForm()
{
    QString content = _configEditor->toPlainText();
    auto iniData = parseIniContent(content);

    if (iniData.isEmpty()) {
        ElaText* emptyHint = new ElaText("配置文件为空或格式不正确", _formContainer);
        emptyHint->setTextPixelSize(14);
        emptyHint->setAlignment(Qt::AlignCenter);
        _formLayout->insertWidget(0, emptyHint);
        return;
    }

    // 遍历每个 section，每个 section 创建一个卡片
    for (auto sectionIt = iniData.begin(); sectionIt != iniData.end(); ++sectionIt) {
        QString sectionName = sectionIt.key();
        const auto& keyValues = sectionIt.value();

        if (keyValues.isEmpty()) {
            continue;
        }

        // 创建分组卡片
        QWidget* card = new QWidget(_formContainer);
        card->setObjectName("ConfigCard");
        card->setStyleSheet(getCardStyleSheet());
        _formCards.append(card);  // 保存引用用于主题更新

        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(15, 12, 15, 12);
        cardLayout->setSpacing(8);

        // 分组标题
        QString displayTitle = sectionName.isEmpty() ? "General" : sectionName;
        ElaText* titleLabel = new ElaText(displayTitle, card);
        titleLabel->setTextPixelSize(15);
        titleLabel->setTextStyle(ElaTextType::Subtitle);
        cardLayout->addWidget(titleLabel);

        // 添加分隔线效果（通过间距）
        cardLayout->addSpacing(4);

        // 每个配置项一行
        for (auto kvIt = keyValues.begin(); kvIt != keyValues.end(); ++kvIt) {
            QString key = kvIt.key();
            const ConfigItem& item = kvIt.value();

            // 构建唯一路径标识
            QString path = sectionName.isEmpty() ? key : QString("%1/%2").arg(sectionName, key);

            // 单行容器
            QWidget* rowWidget = new QWidget(card);
            QHBoxLayout* rowLayout = new QHBoxLayout(rowWidget);
            rowLayout->setContentsMargins(0, 4, 0, 4);
            rowLayout->setSpacing(12);

            // 左侧：键名标签
            ElaText* keyLabel = new ElaText(key, rowWidget);
            keyLabel->setTextPixelSize(13);
            // keyLabel->setFixedWidth(120);  // 固定宽度保证对齐
            rowLayout->addWidget(keyLabel);

            // 右侧：根据类型创建不同的控件
            QWidget* valueWidget = nullptr;

            switch (item.type) {
                case ConfigValueType::Boolean: {
                    // 布尔值 -> 开关
                    ElaToggleSwitch* toggleSwitch = new ElaToggleSwitch(rowWidget);

                    // 解析布尔值
                    QString lowerValue = item.value.toLower();
                    bool isTrue = (lowerValue == "true" || lowerValue == "yes" ||
                                  lowerValue == "on" || lowerValue == "1");
                    toggleSwitch->setIsToggled(isTrue);

                    // 绑定修改信号
                    connect(toggleSwitch, &ElaToggleSwitch::toggled, this, [this]() {
                        _isModified = true;
                        _saveButton->setEnabled(true);
                        if (!_currentFilePath.isEmpty()) {
                            emit configFileModified(_currentFilePath);
                        }
                    });

                    valueWidget = toggleSwitch;
                    break;
                }

                case ConfigValueType::Integer: {
                    // 整数 -> SpinBox
                    ElaSpinBox* spinBox = new ElaSpinBox(rowWidget);
                    spinBox->setMinimum(-2147483648);
                    spinBox->setMaximum(2147483647);
                    spinBox->setValue(item.value.toInt());
                    spinBox->setFixedHeight(32);
                    spinBox->setFixedWidth(150);

                    // 绑定修改信号
                    connect(spinBox, QOverload<int>::of(&ElaSpinBox::valueChanged),
                            this, [this]() {
                        _isModified = true;
                        _saveButton->setEnabled(true);
                        if (!_currentFilePath.isEmpty()) {
                            emit configFileModified(_currentFilePath);
                        }
                    });

                    valueWidget = spinBox;
                    break;
                }

                case ConfigValueType::Double: {
                    // 浮点数 -> DoubleSpinBox
                    ElaDoubleSpinBox* doubleSpinBox = new ElaDoubleSpinBox(rowWidget);
                    doubleSpinBox->setMinimum(-1e9);
                    doubleSpinBox->setMaximum(1e9);
                    doubleSpinBox->setDecimals(6);
                    doubleSpinBox->setValue(item.value.toDouble());
                    doubleSpinBox->setFixedHeight(32);
                    doubleSpinBox->setFixedWidth(150);
                    // 绑定修改信号
                    connect(doubleSpinBox, QOverload<double>::of(&ElaDoubleSpinBox::valueChanged),
                            this, [this]() {
                        _isModified = true;
                        _saveButton->setEnabled(true);
                        if (!_currentFilePath.isEmpty()) {
                            emit configFileModified(_currentFilePath);
                        }
                    });

                    valueWidget = doubleSpinBox;
                    break;
                }

                case ConfigValueType::String:
                case ConfigValueType::Array:
                default: {
                    // 字符串/数组 -> LineEdit
                    ElaLineEdit* valueEdit = new ElaLineEdit(rowWidget);
                    valueEdit->setText(item.value);
                    valueEdit->setFixedHeight(32);
                    valueEdit->setFixedWidth(350);
                    // 绑定修改信号
                    connect(valueEdit, &ElaLineEdit::textChanged, this, [this]() {
                        _isModified = true;
                        _saveButton->setEnabled(true);
                        if (!_currentFilePath.isEmpty()) {
                            emit configFileModified(_currentFilePath);
                        }
                    });

                    valueWidget = valueEdit;
                    break;
                }
            }

            if (valueWidget) {
                rowLayout->addWidget(valueWidget, 1);  // stretch=1 让控件占满剩余空间
                _formWidgetMap[path] = valueWidget;
            }

            cardLayout->addWidget(rowWidget);
        }

        // 将卡片添加到表单布局（在 stretch 之前插入）
        _formLayout->insertWidget(_formLayout->count() - 1, card);
    }
}

void ConfigPage::collectIniFormToSource()
{
    // 从表单控件收集数据，重建 INI 内容
    QMap<QString, QMap<QString, QString>> iniData;

    for (auto it = _formWidgetMap.begin(); it != _formWidgetMap.end(); ++it) {
        QString path = it.key();
        QWidget* widget = it.value();

        // 解析路径：section/key 或 key
        QString section;
        QString key;
        int slashPos = path.indexOf('/');
        if (slashPos != -1) {
            section = path.left(slashPos);
            key = path.mid(slashPos + 1);
        } else {
            key = path;
        }

        // 根据控件类型获取值
        if (auto lineEdit = qobject_cast<ElaLineEdit*>(widget)) {
            iniData[section][key] = lineEdit->text();
        }
        else if (auto spinBox = qobject_cast<ElaSpinBox*>(widget)) {
            iniData[section][key] = QString::number(spinBox->value());
        }
        else if (auto doubleSpinBox = qobject_cast<ElaDoubleSpinBox*>(widget)) {
            iniData[section][key] = QString::number(doubleSpinBox->value(), 'f', 6).remove(QRegularExpression("0+$")).remove(QRegularExpression("\\.$"));
        }
        else if (auto toggleSwitch = qobject_cast<ElaToggleSwitch*>(widget)) {
            iniData[section][key] = toggleSwitch->getIsToggled() ? "true" : "false";
        }
    }

    // 重建 INI 文本
    QString newContent;
    for (auto sectionIt = iniData.begin(); sectionIt != iniData.end(); ++sectionIt) {
        QString section = sectionIt.key();
        const auto& keyValues = sectionIt.value();

        // 写入 section 头（空 section 不写头）
        if (!section.isEmpty()) {
            if (!newContent.isEmpty()) {
                newContent += "\n";
            }
            newContent += QString("[%1]\n").arg(section);
        }

        // 写入键值对
        for (auto kvIt = keyValues.begin(); kvIt != keyValues.end(); ++kvIt) {
            newContent += QString("%1=%2\n").arg(kvIt.key(), kvIt.value());
        }
    }

    // 更新源码编辑器（阻止信号避免循环触发）
    _configEditor->blockSignals(true);
    _configEditor->setPlainText(newContent.trimmed());
    _configEditor->blockSignals(false);
}

void ConfigPage::clearFormWidgets()
{
    QLayoutItem* item;
    while ((item = _formLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    _formWidgetMap.clear();
    _formCards.clear();
    _formLayout->addStretch();
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
        QString fileName = filePath.section('/', -1);
        QString format = detectConfigFormat(filePath);

        QList<QStandardItem*> rowItems;
        QStandardItem* nameItem = new QStandardItem(fileName);
        nameItem->setData(filePath, Qt::UserRole);
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

        // 添加错误日志
        if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
            mainWin->appendLog("ERROR", QString("无法打开配置文件: %1").arg(filePath));
        }
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

    // 如果当前是组件模式，同步到表单
    if (_currentMode == EditMode::Form) {
        syncSourceToForm();
    }

    QString fileName = filePath.section('/', -1);
    ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                           QString("已打开: %1").arg(fileName), 1500);

    // 添加成功日志
    if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
        mainWin->appendLog("INFO", QString("打开配置文件: %1 (格式: %2)")
            .arg(fileName, _currentFormat.toUpper()));
    }

    qDebug() << "打开配置文件:" << filePath << "格式:" << _currentFormat;
}

void ConfigPage::saveCurrentConfig()
{
    if (_currentFilePath.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "没有打开的配置文件", 1500);
        return;
    }

    // 如果在组件模式，先同步到源码
    if (_currentMode == EditMode::Form) {
        syncFormToSource();
    }

    QFile file(_currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                             QString("无法保存文件: %1").arg(_currentFilePath), 2000);

        // 添加错误日志
        if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
            mainWin->appendLog("ERROR", QString("无法保存配置文件: %1").arg(_currentFilePath));
        }
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << _configEditor->toPlainText();
    file.close();

    _isModified = false;
    _saveButton->setEnabled(false);

    QString fileName = _currentFilePath.section('/', -1);
    ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                           "配置文件已保存", 1500);

    // 添加成功日志
    if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
        mainWin->appendLog("SUCCESS", QString("配置文件已保存: %1").arg(fileName));
    }

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

    QString content = _configEditor->toPlainText();
    QString fileName = _currentFilePath.section('/', -1);

    if (_currentFormat == "json") {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError) {
            ElaMessageBar::error(ElaMessageBarType::BottomRight, "JSON 错误",
                                 error.errorString(), 2000);
            // 添加错误日志
            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("ERROR", QString("JSON 验证失败 (%1): %2")
                    .arg(fileName, error.errorString()));
            }
        } else {
            ElaMessageBar::success(ElaMessageBarType::BottomRight, "验证通过",
                                   "JSON 格式正确", 1500);
            // 添加成功日志
            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("SUCCESS", QString("JSON 验证通过: %1").arg(fileName));
            }
        }
    } else if (_currentFormat == "yaml") {
#ifdef YAML_CPP_AVAILABLE
        try {
            YAML::Node root = YAML::Load(content.toStdString());

            // 统计节点数量
            int nodeCount = 0;
            std::function<void(const YAML::Node&)> countNodes;
            countNodes = [&](const YAML::Node& node) {
                if (node.IsMap()) {
                    for (const auto& pair : node) {
                        nodeCount++;
                        countNodes(pair.second);
                    }
                } else if (node.IsSequence()) {
                    for (const auto& item : node) {
                        nodeCount++;
                        countNodes(item);
                    }
                } else if (node.IsScalar()) {
                    nodeCount++;
                }
            };
            countNodes(root);

            ElaMessageBar::success(ElaMessageBarType::BottomRight, "验证通过",
                                   QString("YAML 格式正确，共 %1 个节点").arg(nodeCount), 2000);
            // 添加成功日志
            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("SUCCESS", QString("YAML 验证通过: %1 (共 %2 个节点)")
                    .arg(fileName).arg(nodeCount));
            }
        } catch (const YAML::Exception& e) {
            ElaMessageBar::error(ElaMessageBarType::BottomRight, "YAML 错误",
                                 QString::fromStdString(e.what()), 2000);
            // 添加错误日志
            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("ERROR", QString("YAML 验证失败 (%1): %2")
                    .arg(fileName, QString::fromStdString(e.what())));
            }
        }
#else
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "提示",
                               "yaml-cpp 库未启用", 2000);
#endif
    } else if (_currentFormat == "ini") {
        auto result = parseIniContent(content);
        int sectionCount = result.size();
        int keyCount = 0;
        for (const auto& section : result) {
            keyCount += section.size();
        }
        ElaMessageBar::success(ElaMessageBarType::BottomRight, "验证通过",
                               QString("解析成功: %1 个分组, %2 个配置项").arg(sectionCount).arg(keyCount), 2000);
        // 添加成功日志
        if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
            mainWin->appendLog("SUCCESS", QString("INI 验证通过: %1 (%2 个分组, %3 个配置项)")
                .arg(fileName).arg(sectionCount).arg(keyCount));
        }
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
    qDebug() << "更新语法高亮器:" << format;
}

QString ConfigPage::getCardStyleSheet() const
{
    bool isDark = eTheme->getThemeMode() == ElaThemeType::Dark;

    if (isDark) {
        return R"(
            QWidget#ConfigCard {
                background-color: #2d2d2d;
                border: 1px solid #3d3d3d;
                border-radius: 8px;
            }
        )";
    } else {
        return R"(
            QWidget#ConfigCard {
                background-color: #ffffff;
                border: 1px solid #e0e0e0;
                border-radius: 8px;
            }
        )";
    }
}

void ConfigPage::onThemeChanged(ElaThemeType::ThemeMode mode)
{
    // 更新卡片样式
    QString cardStyle = getCardStyleSheet();
    for (QWidget* card : _formCards) {
        if (card) {
            card->setStyleSheet(cardStyle);
        }
    }

    // 更新 StackedWidget 背景
    bool isDark = (mode == ElaThemeType::Dark);
    QString stackStyle = isDark
        ? "QStackedWidget { background-color: #1a1a1a; }"
        : "QStackedWidget { background-color: #f5f5f5; }";
    _editorStack->setStyleSheet(stackStyle);
}

// ========================================
// YAML 格式处理
// ========================================
QMap<QString, ConfigItem> ConfigPage::parseYamlContent(const QString& content)
{
    QMap<QString, ConfigItem> result;

#ifdef YAML_CPP_AVAILABLE
    try {
        YAML::Node root = YAML::Load(content.toStdString());

        // 递归扁平化 YAML 节点
        std::function<void(const YAML::Node&, const QString&)> flattenNode;
        flattenNode = [&](const YAML::Node& node, const QString& prefix) {
            if (node.IsMap()) {
                for (const auto& pair : node) {
                    QString key = QString::fromStdString(pair.first.as<std::string>());
                    QString fullKey = prefix.isEmpty() ? key : prefix + "." + key;

                    if (pair.second.IsMap()) {
                        // 递归处理嵌套映射
                        flattenNode(pair.second, fullKey);
                    } else if (pair.second.IsSequence()) {
                        // 序列转换为字符串表示
                        QStringList items;
                        for (const auto& item : pair.second) {
                            if (item.IsScalar()) {
                                items.append(QString::fromStdString(item.as<std::string>()));
                            }
                        }
                        QString arrayStr = "[" + items.join(", ") + "]";
                        result[fullKey] = ConfigItem(arrayStr, ConfigValueType::Array);
                    } else if (pair.second.IsScalar()) {
                        // 标量值 - 检测类型
                        QString valueStr = QString::fromStdString(pair.second.as<std::string>());
                        ConfigValueType type = ConfigValueType::String;

                        // 检测布尔值
                        QString lowerValue = valueStr.toLower();
                        if (lowerValue == "true" || lowerValue == "false" ||
                            lowerValue == "yes" || lowerValue == "no" ||
                            lowerValue == "on" || lowerValue == "off") {
                            type = ConfigValueType::Boolean;
                        }
                        // 检测整数
                        else {
                            bool isInt = false;
                            valueStr.toInt(&isInt);
                            if (isInt) {
                                type = ConfigValueType::Integer;
                            } else {
                                // 检测浮点数
                                bool isDouble = false;
                                valueStr.toDouble(&isDouble);
                                if (isDouble && valueStr.contains('.')) {
                                    type = ConfigValueType::Double;
                                }
                            }
                        }

                        result[fullKey] = ConfigItem(valueStr, type);
                    }
                }
            }
        };

        flattenNode(root, QString());
    } catch (const YAML::Exception& e) {
        qDebug() << "YAML parse error:" << e.what();
    }
#else
    qDebug() << "yaml-cpp not available";
#endif

    return result;
}

void ConfigPage::buildYamlForm()
{
    QString content = _configEditor->toPlainText();
    auto yamlData = parseYamlContent(content);

    if (yamlData.isEmpty()) {
        ElaText* emptyHint = new ElaText("配置文件为空或格式不正确", _formContainer);
        emptyHint->setTextPixelSize(14);
        emptyHint->setAlignment(Qt::AlignCenter);
        _formLayout->insertWidget(0, emptyHint);
        return;
    }

    // 按照第一级键（顶层节点）分组
    QMap<QString, QMap<QString, ConfigItem>> groupedData;

    for (auto it = yamlData.begin(); it != yamlData.end(); ++it) {
        QString fullKey = it.key();
        const ConfigItem& item = it.value();

        // 分离顶层节点和子节点
        int dotPos = fullKey.indexOf('.');
        QString topLevelKey;
        QString subKey;

        if (dotPos != -1) {
            topLevelKey = fullKey.left(dotPos);
            subKey = fullKey.mid(dotPos + 1);
        } else {
            topLevelKey = "General";  // 顶层标量值放在 General 组
            subKey = fullKey;
        }

        groupedData[topLevelKey][subKey] = item;
    }

    // 遍历每个分组，每个分组创建一个卡片
    for (auto groupIt = groupedData.begin(); groupIt != groupedData.end(); ++groupIt) {
        QString groupName = groupIt.key();
        const auto& keyValues = groupIt.value();

        if (keyValues.isEmpty()) {
            continue;
        }

        // 创建分组卡片
        QWidget* card = new QWidget(_formContainer);
        card->setObjectName("ConfigCard");
        card->setStyleSheet(getCardStyleSheet());
        _formCards.append(card);

        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(15, 12, 15, 12);
        cardLayout->setSpacing(8);

        // 分组标题
        ElaText* titleLabel = new ElaText(groupName, card);
        titleLabel->setTextPixelSize(15);
        titleLabel->setTextStyle(ElaTextType::Subtitle);
        cardLayout->addWidget(titleLabel);

        // 添加分隔线效果
        cardLayout->addSpacing(4);

        // 每个配置项一行
        for (auto kvIt = keyValues.begin(); kvIt != keyValues.end(); ++kvIt) {
            QString subKey = kvIt.key();
            const ConfigItem& item = kvIt.value();

            // 构建完整路径（用于映射表）
            QString fullPath;
            if (groupName == "General") {
                fullPath = subKey;
            } else {
                fullPath = groupName + "." + subKey;
            }

            // 单行容器
            QWidget* rowWidget = new QWidget(card);
            QHBoxLayout* rowLayout = new QHBoxLayout(rowWidget);
            rowLayout->setContentsMargins(0, 4, 0, 4);
            rowLayout->setSpacing(12);

            // 左侧：键名标签（显示子键）
            ElaText* keyLabel = new ElaText(subKey, rowWidget);
            keyLabel->setTextPixelSize(13);
            // keyLabel->setFixedWidth(150);
            rowLayout->addWidget(keyLabel);

            // 右侧：根据类型创建不同的控件
            QWidget* valueWidget = nullptr;

            switch (item.type) {
                case ConfigValueType::Boolean: {
                    // 布尔值 -> 开关
                    ElaToggleSwitch* toggleSwitch = new ElaToggleSwitch(rowWidget);

                    // 解析布尔值
                    QString lowerValue = item.value.toLower();
                    bool isTrue = (lowerValue == "true" || lowerValue == "yes" ||
                                  lowerValue == "on" || lowerValue == "1");
                    toggleSwitch->setIsToggled(isTrue);

                    // 绑定修改信号
                    connect(toggleSwitch, &ElaToggleSwitch::toggled, this, [this]() {
                        _isModified = true;
                        _saveButton->setEnabled(true);
                        if (!_currentFilePath.isEmpty()) {
                            emit configFileModified(_currentFilePath);
                        }
                    });

                    valueWidget = toggleSwitch;
                    break;
                }

                case ConfigValueType::Integer: {
                    // 整数 -> SpinBox
                    ElaSpinBox* spinBox = new ElaSpinBox(rowWidget);
                    spinBox->setMinimum(-2147483648);
                    spinBox->setMaximum(2147483647);
                    spinBox->setValue(item.value.toInt());
                    spinBox->setFixedHeight(32);
                    spinBox->setFixedWidth(150);

                    // 绑定修改信号
                    connect(spinBox, QOverload<int>::of(&ElaSpinBox::valueChanged),
                            this, [this]() {
                        _isModified = true;
                        _saveButton->setEnabled(true);
                        if (!_currentFilePath.isEmpty()) {
                            emit configFileModified(_currentFilePath);
                        }
                    });

                    valueWidget = spinBox;
                    break;
                }

                case ConfigValueType::Double: {
                    // 浮点数 -> DoubleSpinBox
                    ElaDoubleSpinBox* doubleSpinBox = new ElaDoubleSpinBox(rowWidget);
                    doubleSpinBox->setMinimum(-1e9);
                    doubleSpinBox->setMaximum(1e9);
                    doubleSpinBox->setDecimals(6);
                    doubleSpinBox->setValue(item.value.toDouble());
                    doubleSpinBox->setFixedHeight(32);
                    doubleSpinBox->setFixedWidth(150);
                    // 绑定修改信号
                    connect(doubleSpinBox, QOverload<double>::of(&ElaDoubleSpinBox::valueChanged),
                            this, [this]() {
                        _isModified = true;
                        _saveButton->setEnabled(true);
                        if (!_currentFilePath.isEmpty()) {
                            emit configFileModified(_currentFilePath);
                        }
                    });

                    valueWidget = doubleSpinBox;
                    break;
                }

                case ConfigValueType::String:
                case ConfigValueType::Array:
                default: {
                    // 字符串/数组 -> LineEdit
                    ElaLineEdit* valueEdit = new ElaLineEdit(rowWidget);
                    valueEdit->setText(item.value);
                    valueEdit->setFixedHeight(32);
                    valueEdit->setFixedWidth(350);
                    // 绑定修改信号
                    connect(valueEdit, &ElaLineEdit::textChanged, this, [this]() {
                        _isModified = true;
                        _saveButton->setEnabled(true);
                        if (!_currentFilePath.isEmpty()) {
                            emit configFileModified(_currentFilePath);
                        }
                    });

                    valueWidget = valueEdit;
                    break;
                }
            }

            if (valueWidget) {
                rowLayout->addWidget(valueWidget, 1);
                _formWidgetMap[fullPath] = valueWidget;
            }

            cardLayout->addWidget(rowWidget);
        }

        // 将卡片添加到表单布局
        _formLayout->insertWidget(_formLayout->count() - 1, card);
    }
}

void ConfigPage::collectYamlFormToSource()
{
#ifdef YAML_CPP_AVAILABLE
    // 从表单控件收集数据，重建 YAML 结构
    QMap<QString, QString> flatData;

    for (auto it = _formWidgetMap.begin(); it != _formWidgetMap.end(); ++it) {
        QString path = it.key();
        QWidget* widget = it.value();

        // 根据控件类型获取值
        if (auto lineEdit = qobject_cast<ElaLineEdit*>(widget)) {
            flatData[path] = lineEdit->text();
        }
        else if (auto spinBox = qobject_cast<ElaSpinBox*>(widget)) {
            flatData[path] = QString::number(spinBox->value());
        }
        else if (auto doubleSpinBox = qobject_cast<ElaDoubleSpinBox*>(widget)) {
            flatData[path] = QString::number(doubleSpinBox->value(), 'f', 6);
        }
        else if (auto toggleSwitch = qobject_cast<ElaToggleSwitch*>(widget)) {
            flatData[path] = toggleSwitch->getIsToggled() ? "true" : "false";
        }
    }

    // 将扁平数据转换为嵌套的 YAML 结构
    YAML::Emitter out;
    out << YAML::BeginMap;

    // 按顶层节点分组
    QMap<QString, QMap<QString, QString>> groupedData;
    for (auto it = flatData.begin(); it != flatData.end(); ++it) {
        QString fullKey = it.key();
        QString value = it.value();

        int dotPos = fullKey.indexOf('.');
        if (dotPos != -1) {
            QString topKey = fullKey.left(dotPos);
            QString subKey = fullKey.mid(dotPos + 1);
            groupedData[topKey][subKey] = value;
        } else {
            // 顶层标量
            out << YAML::Key << fullKey.toStdString();

            // 处理数组格式 [item1, item2]
            if (value.startsWith('[') && value.endsWith(']')) {
                QString arrayContent = value.mid(1, value.length() - 2);
                QStringList items = arrayContent.split(',');
                out << YAML::Value << YAML::BeginSeq;
                for (const QString& item : items) {
                    out << item.trimmed().toStdString();
                }
                out << YAML::EndSeq;
            }
            // 处理布尔值
            else if (value == "true" || value == "false") {
                out << YAML::Value << (value == "true");
            }
            // 处理数字
            else {
                bool isInt = false;
                int intValue = value.toInt(&isInt);
                if (isInt) {
                    out << YAML::Value << intValue;
                } else {
                    bool isDouble = false;
                    double doubleValue = value.toDouble(&isDouble);
                    if (isDouble) {
                        out << YAML::Value << doubleValue;
                    } else {
                        out << YAML::Value << value.toStdString();
                    }
                }
            }
        }
    }

    // 处理嵌套节点
    for (auto groupIt = groupedData.begin(); groupIt != groupedData.end(); ++groupIt) {
        QString groupName = groupIt.key();
        const auto& subItems = groupIt.value();

        out << YAML::Key << groupName.toStdString();
        out << YAML::Value << YAML::BeginMap;

        for (auto subIt = subItems.begin(); subIt != subItems.end(); ++subIt) {
            QString subKey = subIt.key();
            QString value = subIt.value();

            // 进一步分解子键（支持多层嵌套）
            QStringList parts = subKey.split('.');

            if (parts.size() == 1) {
                // 单层子键
                out << YAML::Key << subKey.toStdString();

                // 处理数组格式
                if (value.startsWith('[') && value.endsWith(']')) {
                    QString arrayContent = value.mid(1, value.length() - 2);
                    QStringList items = arrayContent.split(',');
                    out << YAML::Value << YAML::BeginSeq;
                    for (const QString& item : items) {
                        out << item.trimmed().toStdString();
                    }
                    out << YAML::EndSeq;
                }
                // 处理布尔值
                else if (value == "true" || value == "false") {
                    out << YAML::Value << (value == "true");
                }
                // 处理数字
                else {
                    bool isInt = false;
                    int intValue = value.toInt(&isInt);
                    if (isInt) {
                        out << YAML::Value << intValue;
                    } else {
                        bool isDouble = false;
                        double doubleValue = value.toDouble(&isDouble);
                        if (isDouble) {
                            out << YAML::Value << doubleValue;
                        } else {
                            out << YAML::Value << value.toStdString();
                        }
                    }
                }
            } else {
                // 多层嵌套（递归处理）
                // 简化处理：直接展开第二层
                QString firstPart = parts[0];
                QString remaining = parts.mid(1).join('.');

                out << YAML::Key << firstPart.toStdString();
                out << YAML::Value << YAML::BeginMap;
                out << YAML::Key << remaining.toStdString();

                // 处理数组格式
                if (value.startsWith('[') && value.endsWith(']')) {
                    QString arrayContent = value.mid(1, value.length() - 2);
                    QStringList items = arrayContent.split(',');
                    out << YAML::Value << YAML::BeginSeq;
                    for (const QString& item : items) {
                        out << item.trimmed().toStdString();
                    }
                    out << YAML::EndSeq;
                }
                // 处理布尔值
                else if (value == "true" || value == "false") {
                    out << YAML::Value << (value == "true");
                }
                // 处理数字
                else {
                    bool isInt = false;
                    int intValue = value.toInt(&isInt);
                    if (isInt) {
                        out << YAML::Value << intValue;
                    } else {
                        bool isDouble = false;
                        double doubleValue = value.toDouble(&isDouble);
                        if (isDouble) {
                            out << YAML::Value << doubleValue;
                        } else {
                            out << YAML::Value << value.toStdString();
                        }
                    }
                }

                out << YAML::EndMap;
            }
        }

        out << YAML::EndMap;
    }

    out << YAML::EndMap;

    // 更新源码编辑器
    QString newContent = QString::fromStdString(out.c_str());
    _configEditor->blockSignals(true);
    _configEditor->setPlainText(newContent);
    _configEditor->blockSignals(false);
#else
    qDebug() << "yaml-cpp not available";
#endif
}

// ========================================
// JSON 格式处理
// ========================================
QMap<QString, ConfigItem> ConfigPage::parseJsonContent(const QString& content)
{
    QMap<QString, ConfigItem> result;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << error.errorString();
        return result;
    }

    if (!doc.isObject()) {
        qDebug() << "JSON root is not an object";
        return result;
    }

    QJsonObject root = doc.object();

    // 递归扁平化 JSON 对象
    std::function<void(const QJsonObject&, const QString&)> flattenObject;
    flattenObject = [&](const QJsonObject& obj, const QString& prefix) {
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            QString key = it.key();
            QString fullKey = prefix.isEmpty() ? key : prefix + "." + key;
            QJsonValue value = it.value();

            if (value.isObject()) {
                // 递归处理嵌套对象
                flattenObject(value.toObject(), fullKey);
            } else if (value.isArray()) {
                // 数组转换为字符串表示
                QJsonArray array = value.toArray();
                QStringList items;
                for (const QJsonValue& item : array) {
                    if (item.isString()) {
                        items.append(item.toString());
                    } else if (item.isDouble()) {
                        items.append(QString::number(item.toDouble()));
                    } else if (item.isBool()) {
                        items.append(item.toBool() ? "true" : "false");
                    }
                }
                QString arrayStr = "[" + items.join(", ") + "]";
                result[fullKey] = ConfigItem(arrayStr, ConfigValueType::Array);
            } else if (value.isBool()) {
                // 布尔值
                result[fullKey] = ConfigItem(value.toBool() ? "true" : "false", ConfigValueType::Boolean);
            } else if (value.isDouble()) {
                // 数字（整数或浮点数）
                double num = value.toDouble();
                QString numStr = QString::number(num, 'g', 15);

                // 检测是否为整数
                if (num == static_cast<int>(num)) {
                    result[fullKey] = ConfigItem(QString::number(static_cast<int>(num)), ConfigValueType::Integer);
                } else {
                    result[fullKey] = ConfigItem(numStr, ConfigValueType::Double);
                }
            } else if (value.isString()) {
                // 字符串
                result[fullKey] = ConfigItem(value.toString(), ConfigValueType::String);
            }
        }
    };

    flattenObject(root, QString());

    return result;
}

void ConfigPage::buildJsonForm()
{
    QString content = _configEditor->toPlainText();
    auto jsonData = parseJsonContent(content);

    if (jsonData.isEmpty()) {
        ElaText* emptyHint = new ElaText("配置文件为空或格式不正确", _formContainer);
        emptyHint->setTextPixelSize(14);
        emptyHint->setAlignment(Qt::AlignCenter);
        _formLayout->insertWidget(0, emptyHint);
        return;
    }

    // 按照第一级键（顶层节点）分组
    QMap<QString, QMap<QString, ConfigItem>> groupedData;

    for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
        QString fullKey = it.key();
        const ConfigItem& item = it.value();

        // 分离顶层节点和子节点
        int dotPos = fullKey.indexOf('.');
        QString topLevelKey;
        QString subKey;

        if (dotPos != -1) {
            topLevelKey = fullKey.left(dotPos);
            subKey = fullKey.mid(dotPos + 1);
        } else {
            topLevelKey = "General";  // 顶层标量值放在 General 组
            subKey = fullKey;
        }

        groupedData[topLevelKey][subKey] = item;
    }

    // 遍历每个分组，每个分组创建一个卡片
    for (auto groupIt = groupedData.begin(); groupIt != groupedData.end(); ++groupIt) {
        QString groupName = groupIt.key();
        const auto& keyValues = groupIt.value();

        if (keyValues.isEmpty()) {
            continue;
        }

        // 创建分组卡片
        QWidget* card = new QWidget(_formContainer);
        card->setObjectName("ConfigCard");
        card->setStyleSheet(getCardStyleSheet());
        _formCards.append(card);

        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(15, 12, 15, 12);
        cardLayout->setSpacing(8);

        // 分组标题
        ElaText* titleLabel = new ElaText(groupName, card);
        titleLabel->setTextPixelSize(15);
        titleLabel->setTextStyle(ElaTextType::Subtitle);
        cardLayout->addWidget(titleLabel);

        // 添加分隔线效果
        cardLayout->addSpacing(4);

        // 每个配置项一行
        for (auto kvIt = keyValues.begin(); kvIt != keyValues.end(); ++kvIt) {
            QString subKey = kvIt.key();
            const ConfigItem& item = kvIt.value();

            // 构建完整路径（用于映射表）
            QString fullPath;
            if (groupName == "General") {
                fullPath = subKey;
            } else {
                fullPath = groupName + "." + subKey;
            }

            // 单行容器
            QWidget* rowWidget = new QWidget(card);
            QHBoxLayout* rowLayout = new QHBoxLayout(rowWidget);
            rowLayout->setContentsMargins(0, 4, 0, 4);
            rowLayout->setSpacing(12);

            // 左侧：键名标签（显示子键）
            ElaText* keyLabel = new ElaText(subKey, rowWidget);
            keyLabel->setTextPixelSize(13);
            rowLayout->addWidget(keyLabel);

            // 右侧：根据类型创建不同的控件
            QWidget* valueWidget = nullptr;

            switch (item.type) {
                case ConfigValueType::Boolean: {
                    // 布尔值 -> 开关
                    ElaToggleSwitch* toggleSwitch = new ElaToggleSwitch(rowWidget);
                    toggleSwitch->setIsToggled(item.value == "true");

                    // 绑定修改信号
                    connect(toggleSwitch, &ElaToggleSwitch::toggled, this, [this]() {
                        _isModified = true;
                        _saveButton->setEnabled(true);
                        if (!_currentFilePath.isEmpty()) {
                            emit configFileModified(_currentFilePath);
                        }
                    });

                    valueWidget = toggleSwitch;
                    break;
                }

                case ConfigValueType::Integer: {
                    // 整数 -> SpinBox
                    ElaSpinBox* spinBox = new ElaSpinBox(rowWidget);
                    spinBox->setMinimum(-2147483648);
                    spinBox->setMaximum(2147483647);
                    spinBox->setValue(item.value.toInt());
                    spinBox->setFixedHeight(32);
                    spinBox->setFixedWidth(150);

                    // 绑定修改信号
                    connect(spinBox, QOverload<int>::of(&ElaSpinBox::valueChanged),
                            this, [this]() {
                        _isModified = true;
                        _saveButton->setEnabled(true);
                        if (!_currentFilePath.isEmpty()) {
                            emit configFileModified(_currentFilePath);
                        }
                    });

                    valueWidget = spinBox;
                    break;
                }

                case ConfigValueType::Double: {
                    // 浮点数 -> DoubleSpinBox
                    ElaDoubleSpinBox* doubleSpinBox = new ElaDoubleSpinBox(rowWidget);
                    doubleSpinBox->setMinimum(-1e9);
                    doubleSpinBox->setMaximum(1e9);
                    doubleSpinBox->setDecimals(6);
                    doubleSpinBox->setValue(item.value.toDouble());
                    doubleSpinBox->setFixedHeight(32);
                    doubleSpinBox->setFixedWidth(150);

                    // 绑定修改信号
                    connect(doubleSpinBox, QOverload<double>::of(&ElaDoubleSpinBox::valueChanged),
                            this, [this]() {
                        _isModified = true;
                        _saveButton->setEnabled(true);
                        if (!_currentFilePath.isEmpty()) {
                            emit configFileModified(_currentFilePath);
                        }
                    });

                    valueWidget = doubleSpinBox;
                    break;
                }

                case ConfigValueType::String:
                case ConfigValueType::Array:
                default: {
                    // 字符串/数组 -> LineEdit
                    ElaLineEdit* valueEdit = new ElaLineEdit(rowWidget);
                    valueEdit->setText(item.value);
                    valueEdit->setFixedHeight(32);
                    valueEdit->setFixedWidth(350);

                    // 绑定修改信号
                    connect(valueEdit, &ElaLineEdit::textChanged, this, [this]() {
                        _isModified = true;
                        _saveButton->setEnabled(true);
                        if (!_currentFilePath.isEmpty()) {
                            emit configFileModified(_currentFilePath);
                        }
                    });

                    valueWidget = valueEdit;
                    break;
                }
            }

            if (valueWidget) {
                rowLayout->addWidget(valueWidget);
                _formWidgetMap[fullPath] = valueWidget;
            }

            cardLayout->addWidget(rowWidget);
        }

        // 将卡片添加到表单布局
        _formLayout->insertWidget(_formLayout->count() - 1, card);
    }
}

void ConfigPage::collectJsonFormToSource()
{
    // 从表单控件收集数据，重建 JSON 结构
    QMap<QString, QString> flatData;

    for (auto it = _formWidgetMap.begin(); it != _formWidgetMap.end(); ++it) {
        QString path = it.key();
        QWidget* widget = it.value();

        // 根据控件类型获取值
        if (auto lineEdit = qobject_cast<ElaLineEdit*>(widget)) {
            flatData[path] = lineEdit->text();
        }
        else if (auto spinBox = qobject_cast<ElaSpinBox*>(widget)) {
            flatData[path] = QString::number(spinBox->value());
        }
        else if (auto doubleSpinBox = qobject_cast<ElaDoubleSpinBox*>(widget)) {
            flatData[path] = QString::number(doubleSpinBox->value());
        }
        else if (auto toggleSwitch = qobject_cast<ElaToggleSwitch*>(widget)) {
            flatData[path] = toggleSwitch->getIsToggled() ? "true" : "false";
        }
    }

    // 将扁平数据转换为嵌套的 JSON 结构
    QJsonObject root;

    // 按顶层节点分组
    QMap<QString, QMap<QString, QString>> groupedData;
    QStringList topLevelKeys;  // 顶层标量键

    for (auto it = flatData.begin(); it != flatData.end(); ++it) {
        QString fullKey = it.key();
        QString value = it.value();

        int dotPos = fullKey.indexOf('.');
        if (dotPos != -1) {
            QString topKey = fullKey.left(dotPos);
            QString subKey = fullKey.mid(dotPos + 1);
            groupedData[topKey][subKey] = value;
        } else {
            // 顶层标量
            topLevelKeys.append(fullKey);

            // 转换值
            if (value.startsWith('[') && value.endsWith(']')) {
                // 数组
                QString arrayContent = value.mid(1, value.length() - 2);
                QStringList items = arrayContent.split(',');
                QJsonArray array;
                for (const QString& item : items) {
                    QString trimmed = item.trimmed();
                    bool isNum = false;
                    double num = trimmed.toDouble(&isNum);
                    if (isNum) {
                        array.append(num);
                    } else if (trimmed == "true") {
                        array.append(true);
                    } else if (trimmed == "false") {
                        array.append(false);
                    } else {
                        array.append(trimmed);
                    }
                }
                root[fullKey] = array;
            } else if (value == "true") {
                root[fullKey] = true;
            } else if (value == "false") {
                root[fullKey] = false;
            } else {
                bool isInt = false;
                int intValue = value.toInt(&isInt);
                if (isInt) {
                    root[fullKey] = intValue;
                } else {
                    bool isDouble = false;
                    double doubleValue = value.toDouble(&isDouble);
                    if (isDouble) {
                        root[fullKey] = doubleValue;
                    } else {
                        root[fullKey] = value;
                    }
                }
            }
        }
    }

    // 处理嵌套节点
    for (auto groupIt = groupedData.begin(); groupIt != groupedData.end(); ++groupIt) {
        QString groupName = groupIt.key();
        const auto& subItems = groupIt.value();

        QJsonObject groupObject;

        for (auto subIt = subItems.begin(); subIt != subItems.end(); ++subIt) {
            QString subKey = subIt.key();
            QString value = subIt.value();

            // 进一步分解子键（支持多层嵌套）
            QStringList parts = subKey.split('.');

            if (parts.size() == 1) {
                // 单层子键
                if (value.startsWith('[') && value.endsWith(']')) {
                    // 数组
                    QString arrayContent = value.mid(1, value.length() - 2);
                    QStringList items = arrayContent.split(',');
                    QJsonArray array;
                    for (const QString& item : items) {
                        QString trimmed = item.trimmed();
                        bool isNum = false;
                        double num = trimmed.toDouble(&isNum);
                        if (isNum) {
                            array.append(num);
                        } else if (trimmed == "true") {
                            array.append(true);
                        } else if (trimmed == "false") {
                            array.append(false);
                        } else {
                            array.append(trimmed);
                        }
                    }
                    groupObject[subKey] = array;
                } else if (value == "true") {
                    groupObject[subKey] = true;
                } else if (value == "false") {
                    groupObject[subKey] = false;
                } else {
                    bool isInt = false;
                    int intValue = value.toInt(&isInt);
                    if (isInt) {
                        groupObject[subKey] = intValue;
                    } else {
                        bool isDouble = false;
                        double doubleValue = value.toDouble(&isDouble);
                        if (isDouble) {
                            groupObject[subKey] = doubleValue;
                        } else {
                            groupObject[subKey] = value;
                        }
                    }
                }
            } else {
                // 多层嵌套（简化处理：展开一层）
                QString firstPart = parts[0];
                QString remaining = parts.mid(1).join('.');

                QJsonObject nestedObj;
                if (value == "true") {
                    nestedObj[remaining] = true;
                } else if (value == "false") {
                    nestedObj[remaining] = false;
                } else {
                    bool isInt = false;
                    int intValue = value.toInt(&isInt);
                    if (isInt) {
                        nestedObj[remaining] = intValue;
                    } else {
                        bool isDouble = false;
                        double doubleValue = value.toDouble(&isDouble);
                        if (isDouble) {
                            nestedObj[remaining] = doubleValue;
                        } else {
                            nestedObj[remaining] = value;
                        }
                    }
                }
                groupObject[firstPart] = nestedObj;
            }
        }

        root[groupName] = groupObject;
    }

    // 转换为格式化的 JSON 字符串
    QJsonDocument doc(root);
    QString newContent = doc.toJson(QJsonDocument::Indented);

    // 更新源码编辑器
    _configEditor->blockSignals(true);
    _configEditor->setPlainText(newContent);
    _configEditor->blockSignals(false);
}

} // namespace Prism
