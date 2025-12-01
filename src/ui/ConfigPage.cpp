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
#include <QHeaderView>
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
    ConfigPage::ConfigPage(QWidget *parent)
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
        // 主题适配
        _mainSplitter->setStyleSheet(getSplitterAndScrollBarStyleSheet());
        // ========================================
        // 左侧：配置文件树形列表
        // ========================================
        QWidget *leftWidget = new QWidget(this);
        QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
        leftLayout->setContentsMargins(5, 5, 5, 5);

        ElaText *treeTitle = new ElaText("配置文件列表", this);
        treeTitle->setTextPixelSize(16);
        leftLayout->addWidget(treeTitle);

        _configTreeView = new ElaTreeView(this);
        _treeModel = new QStandardItemModel(this);
        _treeModel->setHorizontalHeaderLabels({"文件名", "格式"});
        _configTreeView->setModel(_treeModel);
        _configTreeView->setHeaderHidden(false);
        _configTreeView->header()->resizeSection(0, 190);
        _configTreeView->header()->resizeSection(1, QHeaderView::ResizeToContents);
        leftLayout->addWidget(_configTreeView);

        // 添加文件按钮
        _addFileButton = new ElaPushButton("添加配置文件", this);
        leftLayout->addWidget(_addFileButton);

        _mainSplitter->addWidget(leftWidget);

        // ========================================
        // 右侧：配置编辑器（双模式）
        // ========================================
        QWidget *rightWidget = new QWidget(this);
        QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
        rightLayout->setContentsMargins(5, 5, 5, 5);

        // 标题和模式切换
        QHBoxLayout *headerLayout = new QHBoxLayout();
        ElaText *editorTitle = new ElaText("配置编辑器", this);
        editorTitle->setTextPixelSize(16);
        headerLayout->addWidget(editorTitle);
        headerLayout->addStretch();

        // 模式切换按钮（默认组件模式）
        _sourceModeBtn = new ElaRadioButton("源码模式", this);
        _formModeBtn = new ElaRadioButton("组件模式", this);
        _formModeBtn->setChecked(true); // 默认组件模式
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

        //_formScrollArea->setStyleSheet(getSplitterAndScrollBarStyleSheet());

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
        QHBoxLayout *buttonLayout = new QHBoxLayout();
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
        _mainSplitter->setStretchFactor(0, 3);
        _mainSplitter->setStretchFactor(1, 7);

        // ========================================
        // 组装到页面中央
        // ========================================
        QWidget *centralWidget = new QWidget(this);
        centralWidget->setWindowTitle("配置文件");
        QVBoxLayout *centerLayout = new QVBoxLayout(centralWidget);
        centerLayout->setContentsMargins(0, 0, 0, 0);
        centerLayout->addWidget(_mainSplitter);

        addCentralWidget(centralWidget, true, false, 0);
    }

    void ConfigPage::setupConnections()
    {
        // 树形列表点击事件
        connect(_configTreeView, &ElaTreeView::clicked, this, [this](const QModelIndex &index) {
            if (!index.isValid()) return;

            QString filePath = _treeModel->itemFromIndex(index.siblingAtColumn(0))->data(Qt::UserRole).toString();
            if (!filePath.isEmpty())
            {
                openConfigFile(filePath);
            }
        });

        // 添加文件按钮
        connect(_addFileButton, &ElaPushButton::clicked, this, &ConfigPage::addConfigFile);

        // 编辑器内容变化
        connect(_configEditor, &ElaPlainTextEdit::textChanged, this, [this]() {
            if (!_currentFilePath.isEmpty())
            {
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

        if (mode == EditMode::Form)
        {
            // 切换到组件模式：从源码同步到表单
            syncSourceToForm();
            _editorStack->setCurrentIndex(1);
        }
        else
        {
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

        // 使用 ParserFactory 解析配置文件
        auto configData = parseConfigFile(_currentFilePath);

        if (configData.isEmpty()) {
            ElaText *placeholder = new ElaText("无法解析配置文件或文件为空", _formContainer);
            placeholder->setTextPixelSize(14);
            placeholder->setAlignment(Qt::AlignCenter);
            _formLayout->insertWidget(0, placeholder);
            return;
        }

        // 构建表单界面
        buildForm(configData);
    }

    void ConfigPage::syncFormToSource()
    {
        if (_formWidgetMap.isEmpty()) {
            return;
        }

        // 使用统一的保存方法
        collectFormAndSave();
    }

    void ConfigPage::clearFormWidgets()
    {
        QLayoutItem *item;
        while ((item = _formLayout->takeAt(0)) != nullptr)
        {
            if (item->widget())
            {
                item->widget()->deleteLater();
            }
            delete item;
        }

        _formWidgetMap.clear();
        _formCards.clear();
        _formLayout->addStretch();
    }

    void ConfigPage::setProjectName(const QString &projectName)
    {
        _currentProjectName = projectName;
        qDebug() << "ConfigPage 设置项目名称:" << projectName;
    }

    void ConfigPage::setProjectRootPath(const QString &rootPath)
    {
        _currentProjectRootPath = rootPath;
        qDebug() << "ConfigPage 设置项目根路径:" << rootPath;
    }

    void ConfigPage::loadConfigFiles(const QStringList &configFiles)
    {
        _currentConfigFiles = configFiles;  // 存储配置文件列表
        _treeModel->removeRows(0, _treeModel->rowCount());

        for (const QString &filePath: configFiles)
        {
            QString fileName = filePath.section('/', -1);
            QString format = detectConfigFormat(filePath);

            QList<QStandardItem *> rowItems;
            QStandardItem *nameItem = new QStandardItem(fileName);
            nameItem->setData(filePath, Qt::UserRole);
            nameItem->setEditable(false);

            QStandardItem *formatItem = new QStandardItem(format.toUpper());
            formatItem->setEditable(false);

            rowItems << nameItem << formatItem;
            _treeModel->appendRow(rowItems);
            _filePathToFormat[filePath] = format;
        }

        qDebug() << "加载配置文件列表，共" << configFiles.size() << "个文件";
    }

    void ConfigPage::openConfigFile(const QString &filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                                 QString("无法打开文件: %1").arg(filePath), 2000);

            // 添加错误日志
            if (auto *mainWin = qobject_cast<Prism::MainWindow *>(window()))
            {
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
        if (_currentMode == EditMode::Form)
        {
            syncSourceToForm();
        }

        QString fileName = filePath.section('/', -1);
        ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                               QString("已打开: %1").arg(fileName), 1500);

        // 添加成功日志
        if (auto *mainWin = qobject_cast<Prism::MainWindow *>(window()))
        {
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

        // 如果在组件模式，使用 collectFormAndSave
        if (_currentMode == EditMode::Form) {
            collectFormAndSave();
            return;
        }

        // 源码模式：直接保存编辑器内容
        QFile file(_currentFilePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                                 QString("无法保存文件: %1").arg(_currentFilePath), 2000);

            if (auto *mainWin = qobject_cast<Prism::MainWindow *>(window())) {
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

        if (auto *mainWin = qobject_cast<Prism::MainWindow *>(window())) {
            mainWin->appendLog("SUCCESS", QString("配置文件已保存: %1").arg(fileName));
        }

        emit configFileSaved(_currentFilePath);

        qDebug() << "保存配置文件:" << _currentFilePath;
    }

    void ConfigPage::reloadCurrentConfig()
    {
        if (_currentFilePath.isEmpty())
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                                   "没有打开的配置文件", 1500);
            return;
        }

        if (_isModified)
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "重新加载确认",
                                          "当前文件已修改，重新加载将丢失未保存的更改。\n确定要继续吗？",
                                          QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No)
            {
                return;
            }
        }

        openConfigFile(_currentFilePath);
    }

    void ConfigPage::validateConfig()
    {
        if (_currentFilePath.isEmpty())
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                                   "没有打开的配置文件", 1500);
            return;
        }

        QString content = _configEditor->toPlainText();
        QString fileName = _currentFilePath.section('/', -1);

        if (_currentFormat == "json")
        {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &error);
            if (error.error != QJsonParseError::NoError)
            {
                ElaMessageBar::error(ElaMessageBarType::BottomRight, "JSON 错误",
                                     error.errorString(), 2000);
                // 添加错误日志
                if (auto *mainWin = qobject_cast<Prism::MainWindow *>(window()))
                {
                    mainWin->appendLog("ERROR", QString("JSON 验证失败 (%1): %2")
                                       .arg(fileName, error.errorString()));
                }
            }
            else
            {
                ElaMessageBar::success(ElaMessageBarType::BottomRight, "验证通过",
                                       "JSON 格式正确", 1500);
                // 添加成功日志
                if (auto *mainWin = qobject_cast<Prism::MainWindow *>(window()))
                {
                    mainWin->appendLog("SUCCESS", QString("JSON 验证通过: %1").arg(fileName));
                }
            }
        }
        else if (_currentFormat == "yaml")
        {
#ifdef YAML_CPP_AVAILABLE
            try
            {
                YAML::Node root = YAML::Load(content.toStdString());

                // 统计节点数量
                int nodeCount = 0;
                std::function<void(const YAML::Node &)> countNodes;
                countNodes = [&](const YAML::Node &node) {
                    if (node.IsMap())
                    {
                        for (const auto &pair: node)
                        {
                            nodeCount++;
                            countNodes(pair.second);
                        }
                    }
                    else if (node.IsSequence())
                    {
                        for (const auto &item: node)
                        {
                            nodeCount++;
                            countNodes(item);
                        }
                    }
                    else if (node.IsScalar())
                    {
                        nodeCount++;
                    }
                };
                countNodes(root);

                ElaMessageBar::success(ElaMessageBarType::BottomRight, "验证通过",
                                       QString("YAML 格式正确，共 %1 个节点").arg(nodeCount), 2000);
                // 添加成功日志
                if (auto *mainWin = qobject_cast<Prism::MainWindow *>(window()))
                {
                    mainWin->appendLog("SUCCESS", QString("YAML 验证通过: %1 (共 %2 个节点)")
                                       .arg(fileName).arg(nodeCount));
                }
            }
            catch (const YAML::Exception &e)
            {
                ElaMessageBar::error(ElaMessageBarType::BottomRight, "YAML 错误",
                                     QString::fromStdString(e.what()), 2000);
                // 添加错误日志
                if (auto *mainWin = qobject_cast<Prism::MainWindow *>(window()))
                {
                    mainWin->appendLog("ERROR", QString("YAML 验证失败 (%1): %2")
                                       .arg(fileName, QString::fromStdString(e.what())));
                }
            }
#else
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "提示",
                                   "yaml-cpp 库未启用", 2000);
#endif
        }
        else if (_currentFormat == "ini")
        {
            auto result = parseConfigFile(_currentFilePath);
            ElaMessageBar::success(ElaMessageBarType::BottomRight, "验证通过",
                                   QString("解析成功: %1 个配置项").arg(result.size()), 2000);
            // 添加成功日志
            if (auto *mainWin = qobject_cast<Prism::MainWindow *>(window()))
            {
                mainWin->appendLog("SUCCESS", QString("INI 验证通过: %1 (%2 个配置项)")
                                   .arg(fileName).arg(result.size()));
            }
        }
        else
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                                   "未知的配置格式", 1500);
        }
    }

    void ConfigPage::addConfigFile()
    {
        // 确定默认打开路径：优先使用项目的 config 目录
        QString defaultPath;
        if (!_currentProjectRootPath.isEmpty()) {
            QDir configDir(QDir(_currentProjectRootPath).absoluteFilePath("config"));
            if (configDir.exists()) {
                defaultPath = configDir.absolutePath();
            } else {
                defaultPath = _currentProjectRootPath;
            }
        } else {
            defaultPath = QDir::homePath();
        }

        QString filePath = QFileDialog::getOpenFileName(
            this,
            "选择配置文件",
            defaultPath,
            "配置文件 (*.ini *.json *.yaml *.yml *.conf);;所有文件 (*.*)"
        );

        if (!filePath.isEmpty())
        {
            // 检查文件是否已存在
            if (_currentConfigFiles.contains(filePath)) {
                ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                                       "此文件已在列表中", 1500);
                return;
            }

            // 添加到配置文件列表
            _currentConfigFiles.append(filePath);

            // 刷新树视图
            loadConfigFiles(_currentConfigFiles);

            // 发射信号通知 MainWindow
            emit configFileAdded(filePath);

            QString fileName = filePath.section('/', -1);
            ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                                   QString("已添加配置文件: %1").arg(fileName), 1500);

            // 添加日志
            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("SUCCESS", QString("添加配置文件: %1").arg(filePath));
            }

            qDebug() << "添加配置文件:" << filePath;
        }
    }

    QString ConfigPage::detectConfigFormat(const QString &filePath)
    {
        QString suffix = filePath.section('.', -1).toLower();

        if (suffix == "ini" || suffix == "conf")
        {
            return "ini";
        }
        else if (suffix == "json")
        {
            return "json";
        }
        else if (suffix == "yaml" || suffix == "yml")
        {
            return "yaml";
        }
        else
        {
            return "unknown";
        }
    }

    void ConfigPage::updateSyntaxHighlighter(const QString &format)
    {
        qDebug() << "更新语法高亮器:" << format;
    }

    ConfigItem ConfigPage::variantToConfigItem(const QVariant& variant)
    {
        ConfigItem item;

        switch (variant.type()) {
            case QVariant::Bool:
                item.type = ConfigValueType::Boolean;
                item.value = variant.toBool() ? "true" : "false";
                break;

            case QVariant::Int:
            case QVariant::LongLong:
            case QVariant::UInt:
            case QVariant::ULongLong:
                item.type = ConfigValueType::Integer;
                item.value = QString::number(variant.toLongLong());
                break;

            case QVariant::Double:
                item.type = ConfigValueType::Double;
                item.value = QString::number(variant.toDouble(), 'f', 6)
                    .remove(QRegularExpression("0+$"))
                    .remove(QRegularExpression("\\.$"));
                break;

            case QVariant::List:
            case QVariant::StringList:
                item.type = ConfigValueType::Array;
                if (variant.type() == QVariant::StringList) {
                    QStringList list = variant.toStringList();
                    item.value = "[" + list.join(", ") + "]";
                } else {
                    QVariantList list = variant.toList();
                    QStringList strList;
                    for (const auto& v : list) {
                        strList.append(v.toString());
                    }
                    item.value = "[" + strList.join(", ") + "]";
                }
                break;

            case QVariant::String:
            default:
                {
                    QString str = variant.toString();
                    QString lowerStr = str.toLower();

                    // 尝试检测布尔值
                    if (lowerStr == "true" || lowerStr == "false" ||
                        lowerStr == "yes" || lowerStr == "no" ||
                        lowerStr == "on" || lowerStr == "off") {
                        item.type = ConfigValueType::Boolean;
                        item.value = str;
                    }
                    // 尝试检测整数
                    else {
                        bool isInt = false;
                        str.toInt(&isInt);
                        if (isInt) {
                            item.type = ConfigValueType::Integer;
                            item.value = str;
                        } else {
                            // 尝试检测浮点数
                            bool isDouble = false;
                            str.toDouble(&isDouble);
                            if (isDouble && str.contains('.')) {
                                item.type = ConfigValueType::Double;
                                item.value = str;
                            } else {
                                // 默认为字符串
                                item.type = ConfigValueType::String;
                                item.value = str;
                            }
                        }
                    }
                }
                break;
        }

        return item;
    }

    QMap<QString, ConfigItem> ConfigPage::parseConfigFile(const QString& filePath)
    {
        QMap<QString, ConfigItem> result;

        // 使用 ParserFactory 解析配置文件
        ParserFactory& factory = ParserFactory::instance();
        QVariantMap variantMap = factory.parse(filePath);

        if (variantMap.isEmpty()) {
            QString error = factory.getLastError();
            if (!error.isEmpty()) {
                qWarning() << "解析配置文件失败:" << error;
                if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                    mainWin->appendLog("ERROR", QString("解析失败: %1").arg(error));
                }
            }
            return result;
        }

        // 将 QVariantMap 转换为 ConfigItem
        for (auto it = variantMap.constBegin(); it != variantMap.constEnd(); ++it) {
            result[it.key()] = variantToConfigItem(it.value());
        }

        qDebug() << "解析配置文件成功:" << filePath << "共" << result.size() << "个配置项";
        return result;
    }

    void ConfigPage::buildForm(const QMap<QString, ConfigItem>& configData)
    {
        if (configData.isEmpty()) {
            ElaText *emptyHint = new ElaText("配置文件为空或格式不正确", _formContainer);
            emptyHint->setTextPixelSize(14);
            emptyHint->setAlignment(Qt::AlignCenter);
            _formLayout->insertWidget(0, emptyHint);
            return;
        }

        // 按照第一级键（顶层节点）分组
        QMap<QString, QMap<QString, ConfigItem>> groupedData;

        for (auto it = configData.begin(); it != configData.end(); ++it) {
            QString fullKey = it.key();
            const ConfigItem &item = it.value();

            // 分离顶层节点和子节点
            int dotPos = fullKey.indexOf('.');
            QString topLevelKey;
            QString subKey;

            if (dotPos != -1) {
                topLevelKey = fullKey.left(dotPos);
                subKey = fullKey.mid(dotPos + 1);
            } else {
                topLevelKey = "General"; // 顶层标量值放在 General 组
                subKey = fullKey;
            }

            groupedData[topLevelKey][subKey] = item;
        }

        // 遍历每个分组，每个分组创建一个卡片
        for (auto groupIt = groupedData.begin(); groupIt != groupedData.end(); ++groupIt) {
            QString groupName = groupIt.key();
            const auto &keyValues = groupIt.value();

            if (keyValues.isEmpty()) {
                continue;
            }

            // 创建分组卡片
            QWidget *card = new QWidget(_formContainer);
            card->setObjectName("ConfigCard");
            card->setStyleSheet(getCardStyleSheet());
            _formCards.append(card);

            QVBoxLayout *cardLayout = new QVBoxLayout(card);
            cardLayout->setContentsMargins(15, 12, 15, 12);
            cardLayout->setSpacing(8);

            // 分组标题
            ElaText *titleLabel = new ElaText(groupName, card);
            titleLabel->setTextPixelSize(15);
            titleLabel->setTextStyle(ElaTextType::Subtitle);
            cardLayout->addWidget(titleLabel);

            // 添加分隔线效果
            cardLayout->addSpacing(4);

            // 每个配置项一行
            for (auto kvIt = keyValues.begin(); kvIt != keyValues.end(); ++kvIt) {
                QString subKey = kvIt.key();
                const ConfigItem &item = kvIt.value();

                // 构建完整路径（用于映射表）
                QString fullPath;
                if (groupName == "General") {
                    fullPath = subKey;
                } else {
                    fullPath = groupName + "." + subKey;
                }

                // 单行容器
                QWidget *rowWidget = new QWidget(card);
                QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
                rowLayout->setContentsMargins(0, 4, 0, 4);
                rowLayout->setSpacing(12);

                // 左侧：键名标签（显示子键）
                ElaText *keyLabel = new ElaText(subKey, rowWidget);
                keyLabel->setTextPixelSize(13);
                rowLayout->addWidget(keyLabel);
                rowLayout->addStretch();

                // 右侧：根据类型创建不同的控件
                QWidget *valueWidget = nullptr;

                switch (item.type) {
                    case ConfigValueType::Boolean: {
                        ElaToggleSwitch *toggleSwitch = new ElaToggleSwitch(rowWidget);
                        QString lowerValue = item.value.toLower();
                        bool isTrue = (lowerValue == "true" || lowerValue == "yes" ||
                                       lowerValue == "on" || lowerValue == "1");
                        toggleSwitch->setIsToggled(isTrue);

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
                        ElaSpinBox *spinBox = new ElaSpinBox(rowWidget);
                        spinBox->setMinimum(-2147483648);
                        spinBox->setMaximum(2147483647);
                        spinBox->setValue(item.value.toInt());
                        spinBox->setFixedHeight(32);
                        spinBox->setFixedWidth(150);

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
                        ElaDoubleSpinBox *doubleSpinBox = new ElaDoubleSpinBox(rowWidget);
                        doubleSpinBox->setMinimum(-1e10);
                        doubleSpinBox->setMaximum(1e10);
                        doubleSpinBox->setDecimals(6);
                        doubleSpinBox->setValue(item.value.toDouble());
                        doubleSpinBox->setFixedHeight(32);
                        doubleSpinBox->setFixedWidth(150);

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
                        ElaLineEdit *lineEdit = new ElaLineEdit(rowWidget);
                        lineEdit->setText(item.value);
                        lineEdit->setFixedHeight(32);
                        lineEdit->setFixedWidth(350);

                        connect(lineEdit, &ElaLineEdit::textChanged, this, [this]() {
                            _isModified = true;
                            _saveButton->setEnabled(true);
                            if (!_currentFilePath.isEmpty()) {
                                emit configFileModified(_currentFilePath);
                            }
                        });

                        valueWidget = lineEdit;
                        break;
                    }
                }

                // 添加控件到布局
                rowLayout->addWidget(valueWidget);

                // 添加行到卡片
                cardLayout->addWidget(rowWidget);

                // 保存控件映射
                _formWidgetMap[fullPath] = valueWidget;
            }

            // 添加卡片到表单布局
            _formLayout->insertWidget(_formLayout->count() - 1, card);
        }

        qDebug() << "构建表单界面成功，共" << groupedData.size() << "个分组," << configData.size() << "个配置项";
    }

    void ConfigPage::collectFormAndSave()
    {
        if (_currentFilePath.isEmpty()) {
            qWarning() << "没有打开的配置文件，无法保存";
            return;
        }

        // 从表单控件收集数据
        QVariantMap configData;

        for (auto it = _formWidgetMap.begin(); it != _formWidgetMap.end(); ++it) {
            QString path = it.key();
            QWidget *widget = it.value();

            QVariant value;

            // 根据控件类型获取值
            if (auto lineEdit = qobject_cast<ElaLineEdit*>(widget)) {
                value = lineEdit->text();
            }
            else if (auto spinBox = qobject_cast<ElaSpinBox*>(widget)) {
                value = spinBox->value();
            }
            else if (auto doubleSpinBox = qobject_cast<ElaDoubleSpinBox*>(widget)) {
                value = doubleSpinBox->value();
            }
            else if (auto toggleSwitch = qobject_cast<ElaToggleSwitch*>(widget)) {
                value = toggleSwitch->getIsToggled();
            }

            configData[path] = value;
        }

        // 使用 ParserFactory 保存配置
        ParserFactory& factory = ParserFactory::instance();
        ConfigFormat format = factory.detectFormat(_currentFilePath);

        bool success = factory.save(_currentFilePath, configData, format);

        if (success) {
            // 重新读取文件内容到源码编辑器
            QFile file(_currentFilePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                in.setCodec("UTF-8");
                _configEditor->setPlainText(in.readAll());
                file.close();
            }

            _isModified = false;
            _saveButton->setEnabled(false);

            QString fileName = _currentFilePath.section('/', -1);
            ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                                   "配置文件已保存", 1500);

            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("SUCCESS", QString("配置文件已保存: %1").arg(fileName));
            }

            emit configFileSaved(_currentFilePath);
        } else {
            QString error = factory.getLastError();
            ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                                 QString("保存失败: %1").arg(error), 2000);

            if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
                mainWin->appendLog("ERROR", QString("保存失败: %1").arg(error));
            }
        }

        qDebug() << "从表单保存配置文件:" << _currentFilePath << "结果:" << (success ? "成功" : "失败");
    }

    QString ConfigPage::getCardStyleSheet() const
    {
        bool isDark = eTheme->getThemeMode() == ElaThemeType::Dark;

        if (isDark)
        {
            return R"(
            QWidget#ConfigCard {
                background-color: #2d2d2d;
                border: 1px solid #3d3d3d;
                border-radius: 8px;
            }
        )";
        }
        else
        {
            return R"(
            QWidget#ConfigCard {
                background-color: #ffffff;
                border: 1px solid #e0e0e0;
                border-radius: 8px;
            }
        )";
        }
    }

    QString ConfigPage::getSplitterAndScrollBarStyleSheet() const
    {
        bool isDark = eTheme->getThemeMode() == ElaThemeType::Dark;

        if (!isDark)
        {
            return R"(
            /* QSplitter整体背景（浅色基调） */
                QSplitter {
                    background-color: #f8f8f8;
                }

                /* 左右分割条（垂直handle） */
                QSplitter::handle:vertical {
                    width: 5px;                /* 分割条宽度 */
                    background-color: #e0e0e0; /* 分割条底色（浅灰） */
                    border-radius: 2px;        /* 圆角优化 */
                    margin: 0 1px;             /* 与上下边缘间距，避免贴边 */
                }

                /* hover交互效果 */
                QSplitter::handle:vertical:hover {
                    background-color: #c0c0c0; /* hover时加深颜色 */
                    width: 6px;                /*  hover时加宽，增强反馈 */
                }
                /* 当前QScrollArea的垂直滚动条 */
                QScrollBar:vertical {
                    width: 8px;
                    background: #f5f5f5;
                }
                QScrollBar::handle:vertical {
                    background: #dcdcdc;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical:hover {
                    background: #b9b9b9;
                }
                QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
                    background: #eeeeee;
                }
                QScrollArea { background: transparent; border: none; }
            )";
        }
        else
        {
            return R"(
                 /* QSplitter整体背景（深色基调） */
                 QSplitter {
                    background-color: #222222;
                 }

                 /* 左右分割条（垂直handle） */
                 QSplitter::handle:vertical {
                    width: 4px;                /* 分割条宽度（深色下略窄更精致） */
                    background-color: #444444; /* 分割条底色（深灰） */
                    border-radius: 2px;        /* 圆角优化 */
                margin: 0 1px;             /* 与上下边缘间距 */
                }

                /* hover交互效果 */
                QSplitter::handle:vertical:hover {
                    background-color: #666666; /* hover时提亮颜色 */
                    width: 5px;                /*  hover时加宽 */
                }
                /* 当前QScrollArea的垂直滚动条（深色模式） */
                QScrollBar:vertical {
                    width: 8px;
                    background: #2c2c2c;  /* 滚动条背景（深灰） */
                    margin: 0px 0px 0px 0px;
                }
                QScrollBar::handle:vertical {
                    background: #4a4a4a;  /* 滑块颜色（中灰） */
                    border-radius: 4px;
                    min-height: 30px;     /* 滑块最小高度 */
                }
                QScrollBar::handle:vertical:hover {
                    background: #6a6a6a;  /* hover时高亮（浅灰） */
                }
                QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
                    background: #363636;  /* 空白区域颜色（比背景稍浅） */
                }
                /* 隐藏箭头按钮（深色模式下更简洁） */
                QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {
                    background-color: transparent;
                }
                QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                    background-color: transparent;
                }
                QScrollArea { background: transparent; border: none; }
            )";
        }
    }

    void ConfigPage::onThemeChanged(ElaThemeType::ThemeMode mode)
    {
        // 更新卡片样式
        QString cardStyle = getCardStyleSheet();
        for (QWidget *card: _formCards)
        {
            if (card)
            {
                card->setStyleSheet(cardStyle);
            }
        }

        // 更新 StackedWidget 背景
        bool isDark = (mode == ElaThemeType::Dark);
        QString stackStyle = isDark
                                 ? "QStackedWidget { background-color: #1a1a1a; }"
                                 : "QStackedWidget { background-color: #f5f5f5; }";
        _editorStack->setStyleSheet(stackStyle);

        // 更新分割器
        _mainSplitter->setStyleSheet(getSplitterAndScrollBarStyleSheet());
    }
} // namespace Prism
