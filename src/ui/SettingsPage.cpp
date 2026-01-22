#include "SettingsPage.h"
#include "core/AppSettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFontDatabase>
#include <QDebug>

#include "ElaText.h"
#include "ElaComboBox.h"
#include "ElaSpinBox.h"
#include "ElaToggleSwitch.h"
#include "ElaCheckBox.h"
#include "ElaPushButton.h"
#include "ElaScrollArea.h"
#include "ElaTheme.h"
#include "ElaMessageBar.h"

namespace Prism {

SettingsPage::SettingsPage(QWidget* parent)
    : BasePage(parent)
{
    setWindowTitle("设置");
    setTitleVisible(false);
    setContentsMargins(2, 2, 0, 0);

    initUI();
    setupConnections();
    loadCurrentSettings();

    qDebug() << "SettingsPage 初始化完成";
}

SettingsPage::~SettingsPage()
{
}

void SettingsPage::initUI()
{
    // 主容器
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("设置");
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // 标题
    ElaText* titleText = new ElaText("设置", this);
    titleText->setTextPixelSize(28);
    mainLayout->addWidget(titleText);

    // 滚动区域
    ElaScrollArea* scrollArea = new ElaScrollArea(centralWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget* scrollContent = new QWidget(scrollArea);
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 15, 0);
    scrollLayout->setSpacing(15);

    // ========================================
    // 外观设置
    // ========================================
    QWidget* appearanceCard = createSettingsCard("外观设置");
    QVBoxLayout* appearanceLayout = qobject_cast<QVBoxLayout*>(appearanceCard->layout());

    // 主题模式
    _themeModeCombo = new ElaComboBox(appearanceCard);
    _themeModeCombo->addItem("浅色模式");
    _themeModeCombo->addItem("深色模式");
    _themeModeCombo->addItem("跟随系统");
    _themeModeCombo->setFixedWidth(200);
    appearanceLayout->addWidget(createSettingRow("主题模式", _themeModeCombo, appearanceCard));

    // 界面语言
    _languageCombo = new ElaComboBox(appearanceCard);
    _languageCombo->addItem("简体中文");
    _languageCombo->setFixedWidth(200);
    _languageCombo->setEnabled(false);  // 暂时禁用，只支持中文
    appearanceLayout->addWidget(createSettingRow("界面语言", _languageCombo, appearanceCard));

    scrollLayout->addWidget(appearanceCard);

    // ========================================
    // 编辑器设置
    // ========================================
    QWidget* editorCard = createSettingsCard("编辑器设置");
    QVBoxLayout* editorLayout = qobject_cast<QVBoxLayout*>(editorCard->layout());

    // 代码字体
    _editorFontCombo = new ElaComboBox(editorCard);
    QFontDatabase fontDb;
    QStringList monoFonts = { "Consolas", "Courier New", "Monaco", "Source Code Pro",
                              "Fira Code", "JetBrains Mono", "Cascadia Code" };
    for (const QString& font : monoFonts) {
        if (fontDb.families().contains(font)) {
            _editorFontCombo->addItem(font);
        }
    }
    if (_editorFontCombo->count() == 0) {
        _editorFontCombo->addItem("Consolas");
    }
    _editorFontCombo->setFixedWidth(200);
    editorLayout->addWidget(createSettingRow("代码字体", _editorFontCombo, editorCard));

    // 字体大小
    _fontSizeSpinBox = new ElaSpinBox(editorCard);
    _fontSizeSpinBox->setRange(8, 24);
    _fontSizeSpinBox->setValue(12);
    _fontSizeSpinBox->setFixedWidth(100);
    editorLayout->addWidget(createSettingRow("字体大小", _fontSizeSpinBox, editorCard));

    // Tab 宽度
    _tabWidthSpinBox = new ElaSpinBox(editorCard);
    _tabWidthSpinBox->setRange(2, 8);
    _tabWidthSpinBox->setValue(4);
    _tabWidthSpinBox->setFixedWidth(100);
    editorLayout->addWidget(createSettingRow("Tab 宽度", _tabWidthSpinBox, editorCard));

    // 显示行号
    _showLineNumbersSwitch = new ElaToggleSwitch(editorCard);
    _showLineNumbersSwitch->setIsToggled(true);
    editorLayout->addWidget(createSettingRow("显示行号", _showLineNumbersSwitch, editorCard));

    scrollLayout->addWidget(editorCard);

    // ========================================
    // 日志设置
    // ========================================
    QWidget* loggingCard = createSettingsCard("日志设置");
    QVBoxLayout* loggingLayout = qobject_cast<QVBoxLayout*>(loggingCard->layout());

    // 最大日志行数
    _maxLogLinesSpinBox = new ElaSpinBox(loggingCard);
    _maxLogLinesSpinBox->setRange(1000, 100000);
    _maxLogLinesSpinBox->setSingleStep(1000);
    _maxLogLinesSpinBox->setValue(10000);
    _maxLogLinesSpinBox->setFixedWidth(120);
    loggingLayout->addWidget(createSettingRow("最大日志行数", _maxLogLinesSpinBox, loggingCard));

    // 时间戳格式
    _timestampFormatCombo = new ElaComboBox(loggingCard);
    _timestampFormatCombo->addItem("HH:mm:ss");
    _timestampFormatCombo->addItem("HH:mm:ss.zzz");
    _timestampFormatCombo->addItem("yyyy-MM-dd HH:mm:ss");
    _timestampFormatCombo->setFixedWidth(200);
    loggingLayout->addWidget(createSettingRow("时间戳格式", _timestampFormatCombo, loggingCard));

    // 默认显示级别
    QWidget* logLevelsRow = new QWidget(loggingCard);
    QHBoxLayout* logLevelsLayout = new QHBoxLayout(logLevelsRow);
    logLevelsLayout->setContentsMargins(0, 8, 0, 8);
    logLevelsLayout->setSpacing(8);

    ElaText* logLevelsLabel = new ElaText("默认显示级别", logLevelsRow);
    logLevelsLabel->setTextPixelSize(14);
    logLevelsLabel->setFixedWidth(120);
    logLevelsLayout->addWidget(logLevelsLabel);

    QStringList levels = { "INFO", "SUCCESS", "WARNING", "ERROR", "DEBUG", "STDOUT", "STDERR" };
    for (const QString& level : levels) {
        ElaCheckBox* checkBox = new ElaCheckBox(level, logLevelsRow);
        checkBox->setChecked(level != "DEBUG");  // 默认不选 DEBUG
        _logLevelCheckBoxes.append(checkBox);
        logLevelsLayout->addWidget(checkBox);
    }
    logLevelsLayout->addStretch();

    loggingLayout->addWidget(logLevelsRow);

    scrollLayout->addWidget(loggingCard);

    // ========================================
    // 通用设置
    // ========================================
    QWidget* generalCard = createSettingsCard("通用设置");
    QVBoxLayout* generalLayout = qobject_cast<QVBoxLayout*>(generalCard->layout());

    // 启动时打开上次项目
    _restoreLastProjectSwitch = new ElaToggleSwitch(generalCard);
    _restoreLastProjectSwitch->setIsToggled(true);
    generalLayout->addWidget(createSettingRow("启动时打开上次项目", _restoreLastProjectSwitch, generalCard));

    // 最近项目数量
    _recentProjectCountSpinBox = new ElaSpinBox(generalCard);
    _recentProjectCountSpinBox->setRange(3, 10);
    _recentProjectCountSpinBox->setValue(5);
    _recentProjectCountSpinBox->setFixedWidth(100);
    generalLayout->addWidget(createSettingRow("最近项目数量", _recentProjectCountSpinBox, generalCard));

    // 记住窗口位置
    _rememberWindowGeometrySwitch = new ElaToggleSwitch(generalCard);
    _rememberWindowGeometrySwitch->setIsToggled(true);
    generalLayout->addWidget(createSettingRow("记住窗口位置", _rememberWindowGeometrySwitch, generalCard));

    // 自动保存间隔
    _autoSaveIntervalSpinBox = new ElaSpinBox(generalCard);
    _autoSaveIntervalSpinBox->setRange(0, 60);
    _autoSaveIntervalSpinBox->setValue(0);
    _autoSaveIntervalSpinBox->setSpecialValueText("禁用");
    _autoSaveIntervalSpinBox->setSuffix(" 分钟");
    _autoSaveIntervalSpinBox->setFixedWidth(120);
    generalLayout->addWidget(createSettingRow("自动保存间隔", _autoSaveIntervalSpinBox, generalCard));

    scrollLayout->addWidget(generalCard);

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);

    // ========================================
    // 底部按钮
    // ========================================
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    ElaPushButton* resetButton = new ElaPushButton("重置为默认", centralWidget);
    resetButton->setFixedHeight(36);
    connect(resetButton, &ElaPushButton::clicked, this, &SettingsPage::onResetSettings);
    buttonLayout->addWidget(resetButton);

    buttonLayout->addStretch();

    ElaPushButton* applyButton = new ElaPushButton("应用", centralWidget);
    applyButton->setFixedHeight(36);
    applyButton->setFixedWidth(100);
    connect(applyButton, &ElaPushButton::clicked, this, &SettingsPage::onApplySettings);
    buttonLayout->addWidget(applyButton);

    mainLayout->addLayout(buttonLayout);

    addCentralWidget(centralWidget, true, false, 0);
}

void SettingsPage::setupConnections()
{
    // 主题模式即时生效
    connect(_themeModeCombo, QOverload<int>::of(&ElaComboBox::currentIndexChanged),
            this, &SettingsPage::onThemeModeChanged);

    // 响应系统主题变化
    connect(eTheme, &ElaTheme::themeModeChanged, this, [this]() {
        onThemeChanged();
    });
}

void SettingsPage::loadCurrentSettings()
{
    AppSettings& settings = AppSettings::instance();

    // 外观设置
    _themeModeCombo->setCurrentIndex(settings.themeMode());
    // 语言暂时固定

    // 编辑器设置
    int fontIndex = _editorFontCombo->findText(settings.editorFont());
    if (fontIndex >= 0) {
        _editorFontCombo->setCurrentIndex(fontIndex);
    }
    _fontSizeSpinBox->setValue(settings.editorFontSize());
    _tabWidthSpinBox->setValue(settings.tabWidth());
    _showLineNumbersSwitch->setIsToggled(settings.showLineNumbers());

    // 日志设置
    _maxLogLinesSpinBox->setValue(settings.maxLogLines());
    int formatIndex = _timestampFormatCombo->findText(settings.timestampFormat());
    if (formatIndex >= 0) {
        _timestampFormatCombo->setCurrentIndex(formatIndex);
    }

    QStringList enabledLevels = settings.defaultLogLevels();
    for (ElaCheckBox* checkBox : _logLevelCheckBoxes) {
        checkBox->setChecked(enabledLevels.contains(checkBox->text()));
    }

    // 通用设置
    _restoreLastProjectSwitch->setIsToggled(settings.restoreLastProject());
    _recentProjectCountSpinBox->setValue(settings.recentProjectCount());
    _rememberWindowGeometrySwitch->setIsToggled(settings.rememberWindowGeometry());
    _autoSaveIntervalSpinBox->setValue(settings.autoSaveInterval());
}

void SettingsPage::onApplySettings()
{
    AppSettings& settings = AppSettings::instance();

    // 外观设置
    settings.setThemeMode(_themeModeCombo->currentIndex());

    // 编辑器设置
    settings.setEditorFont(_editorFontCombo->currentText());
    settings.setEditorFontSize(_fontSizeSpinBox->value());
    settings.setTabWidth(_tabWidthSpinBox->value());
    settings.setShowLineNumbers(_showLineNumbersSwitch->getIsToggled());

    // 日志设置
    settings.setMaxLogLines(_maxLogLinesSpinBox->value());
    settings.setTimestampFormat(_timestampFormatCombo->currentText());

    QStringList enabledLevels;
    for (ElaCheckBox* checkBox : _logLevelCheckBoxes) {
        if (checkBox->isChecked()) {
            enabledLevels.append(checkBox->text());
        }
    }
    settings.setDefaultLogLevels(enabledLevels);

    // 通用设置
    settings.setRestoreLastProject(_restoreLastProjectSwitch->getIsToggled());
    settings.setRecentProjectCount(_recentProjectCountSpinBox->value());
    settings.setRememberWindowGeometry(_rememberWindowGeometrySwitch->getIsToggled());
    settings.setAutoSaveInterval(_autoSaveIntervalSpinBox->value());

    // 保存到持久化存储
    settings.save();

    ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                           "设置已保存", 1500);

    qDebug() << "SettingsPage: 已应用并保存设置";
}

void SettingsPage::onResetSettings()
{
    AppSettings& settings = AppSettings::instance();
    settings.reset();

    // 重新加载 UI
    loadCurrentSettings();

    ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                           "已重置为默认设置", 1500);

    qDebug() << "SettingsPage: 已重置为默认设置";
}

void SettingsPage::onThemeModeChanged(int index)
{
    // 主题模式即时生效
    ElaThemeType::ThemeMode mode;
    switch (index) {
        case 0:
            mode = ElaThemeType::Light;
            break;
        case 1:
            mode = ElaThemeType::Dark;
            break;
        case 2:
        default:
            // 跟随系统：这里简单处理，实际应该检测系统主题
            mode = ElaThemeType::Light;
            break;
    }

    eTheme->setThemeMode(mode);
}

QWidget* SettingsPage::createSettingsCard(const QString& title)
{
    QWidget* card = new QWidget(this);
    card->setObjectName("SettingsCard");
    card->setStyleSheet(getCardStyleSheet());
    _cards.append(card);

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(8);

    // 标题
    ElaText* titleLabel = new ElaText(title, card);
    titleLabel->setTextPixelSize(16);
    titleLabel->setTextStyle(ElaTextType::Subtitle);
    layout->addWidget(titleLabel);

    // 分隔线效果
    layout->addSpacing(8);

    return card;
}

QWidget* SettingsPage::createSettingRow(const QString& label, QWidget* widget, QWidget* parent)
{
    QWidget* row = new QWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 8, 0, 8);
    layout->setSpacing(20);

    ElaText* labelWidget = new ElaText(label, row);
    labelWidget->setTextPixelSize(14);
    labelWidget->setFixedWidth(140);
    layout->addWidget(labelWidget);

    layout->addWidget(widget);
    layout->addStretch();

    return row;
}

QString SettingsPage::getCardStyleSheet() const
{
    bool isDark = eTheme->getThemeMode() == ElaThemeType::Dark;

    if (isDark) {
        return R"(
            QWidget#SettingsCard {
                background-color: #2d2d2d;
                border: 1px solid #3d3d3d;
                border-radius: 8px;
            }
        )";
    } else {
        return R"(
            QWidget#SettingsCard {
                background-color: #ffffff;
                border: 1px solid #e0e0e0;
                border-radius: 8px;
            }
        )";
    }
}

void SettingsPage::onThemeChanged()
{
    // 更新所有卡片样式
    QString cardStyle = getCardStyleSheet();
    for (QWidget* card : _cards) {
        if (card) {
            card->setStyleSheet(cardStyle);
        }
    }
}

} // namespace Prism
