#include "LogToolBar.h"

#include <QHBoxLayout>
#include <QLabel>

#include "ElaCheckBox.h"
#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include "ElaText.h"
#include "ElaTheme.h"

namespace Prism {

LogToolBar::LogToolBar(QWidget* parent)
    : QWidget(parent)
{
    initUI();
    setupConnections();
}

void LogToolBar::initUI()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(5, 2, 5, 2);
    mainLayout->setSpacing(8);

    // 过滤标签
    ElaText* filterLabel = new ElaText("过滤:", this);
    filterLabel->setTextPixelSize(12);
    mainLayout->addWidget(filterLabel);

    // 日志级别复选框
    m_infoCheck = new ElaCheckBox("INFO", this);
    m_infoCheck->setChecked(true);
    mainLayout->addWidget(m_infoCheck);

    m_successCheck = new ElaCheckBox("SUCCESS", this);
    m_successCheck->setChecked(true);
    mainLayout->addWidget(m_successCheck);

    m_warningCheck = new ElaCheckBox("WARNING", this);
    m_warningCheck->setChecked(true);
    mainLayout->addWidget(m_warningCheck);

    m_errorCheck = new ElaCheckBox("ERROR", this);
    m_errorCheck->setChecked(true);
    mainLayout->addWidget(m_errorCheck);

    m_debugCheck = new ElaCheckBox("DEBUG", this);
    m_debugCheck->setChecked(false);
    mainLayout->addWidget(m_debugCheck);

    m_stdoutCheck = new ElaCheckBox("STDOUT", this);
    m_stdoutCheck->setChecked(true);
    mainLayout->addWidget(m_stdoutCheck);

    m_stderrCheck = new ElaCheckBox("STDERR", this);
    m_stderrCheck->setChecked(true);
    mainLayout->addWidget(m_stderrCheck);

    // 分隔符
    mainLayout->addSpacing(10);

    // 搜索框
    m_searchEdit = new ElaLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索日志...");
    m_searchEdit->setFixedWidth(180);
    mainLayout->addWidget(m_searchEdit);

    // 搜索按钮
    m_searchBtn = new ElaPushButton("搜索", this);
    m_searchBtn->setFixedWidth(60);
    mainLayout->addWidget(m_searchBtn);

    // 弹性空间
    mainLayout->addStretch();

    // 导出按钮
    m_exportBtn = new ElaPushButton("导出", this);
    m_exportBtn->setFixedWidth(60);
    mainLayout->addWidget(m_exportBtn);

    // 清空按钮
    m_clearBtn = new ElaPushButton("清空", this);
    m_clearBtn->setFixedWidth(60);
    mainLayout->addWidget(m_clearBtn);

    setLayout(mainLayout);
    setFixedHeight(36);
}

void LogToolBar::setupConnections()
{
    // 过滤复选框变化
    connect(m_infoCheck, &ElaCheckBox::stateChanged, this, &LogToolBar::onFilterChanged);
    connect(m_successCheck, &ElaCheckBox::stateChanged, this, &LogToolBar::onFilterChanged);
    connect(m_warningCheck, &ElaCheckBox::stateChanged, this, &LogToolBar::onFilterChanged);
    connect(m_errorCheck, &ElaCheckBox::stateChanged, this, &LogToolBar::onFilterChanged);
    connect(m_debugCheck, &ElaCheckBox::stateChanged, this, &LogToolBar::onFilterChanged);
    connect(m_stdoutCheck, &ElaCheckBox::stateChanged, this, &LogToolBar::onFilterChanged);
    connect(m_stderrCheck, &ElaCheckBox::stateChanged, this, &LogToolBar::onFilterChanged);

    // 搜索框回车
    connect(m_searchEdit, &ElaLineEdit::returnPressed, this, &LogToolBar::onSearchTriggered);
    connect(m_searchEdit, &ElaLineEdit::textChanged, this, &LogToolBar::onSearchTextChanged);

    // 搜索按钮
    connect(m_searchBtn, &ElaPushButton::clicked, this, &LogToolBar::onSearchTriggered);

    // 导出按钮
    connect(m_exportBtn, &ElaPushButton::clicked, this, &LogToolBar::exportRequested);

    // 清空按钮
    connect(m_clearBtn, &ElaPushButton::clicked, this, &LogToolBar::clearRequested);

    // 响应主题变化
    connect(eTheme, &ElaTheme::themeModeChanged, this, &LogToolBar::onThemeChanged);

    // 初始化样式
    updateCheckBoxStyles();
}

QStringList LogToolBar::getFilterLevels() const
{
    QStringList levels;
    if (m_infoCheck->isChecked()) levels << "INFO";
    if (m_successCheck->isChecked()) levels << "SUCCESS";
    if (m_warningCheck->isChecked()) levels << "WARNING";
    if (m_errorCheck->isChecked()) levels << "ERROR";
    if (m_debugCheck->isChecked()) levels << "DEBUG";
    if (m_stdoutCheck->isChecked()) levels << "STDOUT";
    if (m_stderrCheck->isChecked()) levels << "STDERR";
    // 也添加 PROCESS 级别（进程状态变化）
    levels << "PROCESS";
    return levels;
}

QString LogToolBar::getSearchKeyword() const
{
    return m_searchEdit->text().trimmed();
}

void LogToolBar::clearSearch()
{
    m_searchEdit->clear();
}

void LogToolBar::onFilterChanged()
{
    emit filterChanged(getFilterLevels());
}

void LogToolBar::onSearchTextChanged(const QString& text)
{
    // 实时搜索（当文本清空时清除高亮）
    if (text.isEmpty()) {
        emit searchRequested(QString());
    }
}

void LogToolBar::onSearchTriggered()
{
    emit searchRequested(getSearchKeyword());
}

void LogToolBar::updateCheckBoxStyles()
{
    bool isDark = eTheme->getThemeMode() == ElaThemeType::Dark;
    QString textColor = isDark ? "#FFFFFF" : "#000000";

    // 为所有 CheckBox 设置统一的文本颜色
    QString checkBoxStyle = QString(R"(
        ElaCheckBox {
            color: %1;
        }
        ElaCheckBox::indicator {
            width: 18px;
            height: 18px;
        }
    )").arg(textColor);

    QList<ElaCheckBox*> checkBoxes = {
        m_infoCheck, m_successCheck, m_warningCheck, m_errorCheck,
        m_debugCheck, m_stdoutCheck, m_stderrCheck
    };

    for (ElaCheckBox* checkBox : checkBoxes) {
        if (checkBox) {
            checkBox->setStyleSheet(checkBoxStyle);
        }
    }
}

void LogToolBar::onThemeChanged()
{
    updateCheckBoxStyles();
}

} // namespace Prism
