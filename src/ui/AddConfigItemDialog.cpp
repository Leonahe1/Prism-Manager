#include "AddConfigItemDialog.h"
#include "ElaLineEdit.h"
#include "ElaComboBox.h"
#include "ElaPushButton.h"
#include "ElaText.h"
#include "ElaMessageBar.h"
#include "ElaSpinBox.h"
#include "ElaDoubleSpinBox.h"
#include "ElaToggleSwitch.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QRegularExpression>

namespace Prism {

AddConfigItemDialog::AddConfigItemDialog(QWidget* parent)
    : ElaDialog(parent)
{
    setWindowTitle("添加新配置项");
    setFixedSize(520, 380);  // 增加高度以容纳 Section 选择
    setIsFixedSize(true);

    initUI();
    setupConnections();

    // 居中显示
    moveToCenter();
}

AddConfigItemDialog::~AddConfigItemDialog()
{
}

void AddConfigItemDialog::initUI()
{
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(25, 15, 25, 20);
    mainLayout->setSpacing(18);

    // 标题说明
    ElaText* hintText = new ElaText("请输入新配置项的信息", centralWidget);
    hintText->setTextPixelSize(13);
    mainLayout->addWidget(hintText);

    mainLayout->addSpacing(5);

    // 表单布局
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setHorizontalSpacing(15);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    // Section 选择容器
    QWidget* sectionContainer = new QWidget(centralWidget);
    QHBoxLayout* sectionLayout = new QHBoxLayout(sectionContainer);
    sectionLayout->setContentsMargins(0, 0, 0, 0);
    sectionLayout->setSpacing(8);

    _sectionComboBox = new ElaComboBox(sectionContainer);
    _sectionComboBox->addItem("+ 新建分组...");     // 索引 0: 新建分组选项
    _sectionComboBox->setCurrentIndex(0);
    _sectionComboBox->setFixedHeight(35);
    _sectionComboBox->setFixedWidth(200);

    // 自定义 Section 输入框（默认隐藏）
    _customSectionInput = new ElaLineEdit(sectionContainer);
    _customSectionInput->setPlaceholderText("输入新分组名称");
    _customSectionInput->setFixedHeight(35);
    _customSectionInput->setFixedWidth(108);
    _customSectionInput->setVisible(false);

    sectionLayout->addWidget(_sectionComboBox);
    sectionLayout->addWidget(_customSectionInput);
    sectionLayout->addStretch();

    ElaText* sectionLabel = new ElaText("分组:", centralWidget);
    sectionLabel->setTextPixelSize(13);
    sectionLabel->setFixedWidth(60);
    formLayout->addRow(sectionLabel, sectionContainer);

    // 键名输入
    _keyInput = new ElaLineEdit(centralWidget);
    _keyInput->setPlaceholderText("例如: timeout 或 host");
    _keyInput->setFixedHeight(35);
    _keyInput->setFixedWidth(320);

    ElaText* keyLabel = new ElaText("键名:", centralWidget);
    keyLabel->setTextPixelSize(13);
    keyLabel->setFixedWidth(60);
    formLayout->addRow(keyLabel, _keyInput);

    // 类型选择
    _typeComboBox = new ElaComboBox(centralWidget);
    _typeComboBox->addItem("字符串 (String)");
    _typeComboBox->addItem("整数 (Integer)");
    _typeComboBox->addItem("浮点数 (Double)");
    _typeComboBox->addItem("布尔值 (Boolean)");
    _typeComboBox->addItem("数组 (Array)");
    _typeComboBox->setCurrentIndex(0);
    _typeComboBox->setFixedHeight(35);
    _typeComboBox->setFixedWidth(320);  // 改为固定宽度，确保对齐

    ElaText* typeLabel = new ElaText("类型:", centralWidget);
    typeLabel->setTextPixelSize(13);
    typeLabel->setFixedWidth(60);
    formLayout->addRow(typeLabel, _typeComboBox);

    // 默认值容器（用于动态切换控件）
    _valueContainer = new QWidget(centralWidget);
    _valueLayout = new QHBoxLayout(_valueContainer);
    _valueLayout->setContentsMargins(0, 0, 0, 0);
    _valueLayout->setSpacing(0);

    // 创建所有可能的输入控件
    // 字符串输入框
    _valueInput = new ElaLineEdit(_valueContainer);
    _valueInput->setPlaceholderText("请输入默认值");
    _valueInput->setFixedHeight(35);
    _valueInput->setFixedWidth(320);

    // 整数输入框
    _intSpinBox = new ElaSpinBox(_valueContainer);
    _intSpinBox->setMinimum(-2147483648);
    _intSpinBox->setMaximum(2147483647);
    _intSpinBox->setValue(0);
    _intSpinBox->setFixedHeight(35);
    _intSpinBox->setFixedWidth(320);
    _intSpinBox->setVisible(false);

    // 浮点数输入框
    _doubleSpinBox = new ElaDoubleSpinBox(_valueContainer);
    _doubleSpinBox->setMinimum(-1e10);
    _doubleSpinBox->setMaximum(1e10);
    _doubleSpinBox->setDecimals(6);
    _doubleSpinBox->setValue(0.0);
    _doubleSpinBox->setSingleStep(0.01);
    _doubleSpinBox->setDecimals(2);
    _doubleSpinBox->setFixedHeight(35);
    _doubleSpinBox->setFixedWidth(320);
    _doubleSpinBox->setVisible(false);

    // 布尔值开关
    _boolToggle = new ElaToggleSwitch(_valueContainer);
    _boolToggle->setIsToggled(false);
    _boolToggle->setVisible(false);

    // 添加到布局
    _valueLayout->addWidget(_valueInput);
    _valueLayout->addWidget(_intSpinBox);
    _valueLayout->addWidget(_doubleSpinBox);
    _valueLayout->addWidget(_boolToggle);
    _valueLayout->addStretch();

    ElaText* valueLabel = new ElaText("默认值:", centralWidget);
    valueLabel->setTextPixelSize(13);
    valueLabel->setFixedWidth(60);
    formLayout->addRow(valueLabel, _valueContainer);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    // 底部按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    buttonLayout->addStretch();

    _cancelButton = new ElaPushButton("取消", centralWidget);
    _cancelButton->setFixedSize(100, 35);
    buttonLayout->addWidget(_cancelButton);

    _confirmButton = new ElaPushButton("确定", centralWidget);
    _confirmButton->setFixedSize(100, 35);
    buttonLayout->addWidget(_confirmButton);

    mainLayout->addLayout(buttonLayout);

    // 设置中央控件
    QVBoxLayout* dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(0, 10, 0, 0);  // 顶部留出标题栏空间
    dialogLayout->addWidget(centralWidget);
    setLayout(dialogLayout);
}

void AddConfigItemDialog::setupConnections()
{
    // Section 选择改变时显示/隐藏自定义输入框
    connect(_sectionComboBox, QOverload<int>::of(&ElaComboBox::currentIndexChanged),
            this, &AddConfigItemDialog::onSectionChanged);

    // 类型改变时更新默认值提示
    connect(_typeComboBox, QOverload<int>::of(&ElaComboBox::currentIndexChanged),
            this, &AddConfigItemDialog::onTypeChanged);

    // 确定按钮
    connect(_confirmButton, &ElaPushButton::clicked,
            this, &AddConfigItemDialog::onConfirmClicked);

    // 取消按钮
    connect(_cancelButton, &ElaPushButton::clicked,
            this, &AddConfigItemDialog::onCancelClicked);
}

void AddConfigItemDialog::onSectionChanged(int index)
{
    // 索引 0 是 "+ 新建分组..." 选项
    bool isCustomSection = (index == 0);
    _customSectionInput->setVisible(isCustomSection);

    if (isCustomSection) {
        _customSectionInput->setFocus();
        _customSectionInput->clear();
    }
}

void AddConfigItemDialog::onTypeChanged(int index)
{
    // 隐藏所有控件
    _valueInput->setVisible(false);
    _intSpinBox->setVisible(false);
    _doubleSpinBox->setVisible(false);
    _boolToggle->setVisible(false);

    // 根据类型显示对应的控件
    switch (index) {
        case 0: // String
            _valueInput->setVisible(true);
            _valueInput->setPlaceholderText("例如: localhost");
            break;
        case 1: // Integer
            _intSpinBox->setVisible(true);
            _intSpinBox->setValue(0);
            break;
        case 2: // Double
            _doubleSpinBox->setVisible(true);
            _doubleSpinBox->setValue(0.0);
            break;
        case 3: // Boolean
            _boolToggle->setVisible(true);
            _boolToggle->setIsToggled(false);
            break;
        case 4: // Array
            _valueInput->setVisible(true);
            _valueInput->setPlaceholderText("例如: [item1, item2, item3]");
            break;
    }
}

void AddConfigItemDialog::onConfirmClicked()
{
    if (!validateInput()) {
        return;
    }

    // 保存 Section
    int sectionIndex = _sectionComboBox->currentIndex();
    if (sectionIndex == 0) {
        // 索引 0: "+ 新建分组..." - 使用自定义输入
        _section = _customSectionInput->text().trimmed();
    } else {
        // 其他: 已有的 Section
        _section = _sectionComboBox->currentText();
    }

    // 保存键名
    _key = _keyInput->text().trimmed();

    // 根据下拉框索引确定类型
    int typeIndex = _typeComboBox->currentIndex();
    switch (typeIndex) {
        case 0:
            _type = ConfigValueType::String;
            break;
        case 1:
            _type = ConfigValueType::Integer;
            break;
        case 2:
            _type = ConfigValueType::Double;
            break;
        case 3:
            _type = ConfigValueType::Boolean;
            break;
        case 4:
            _type = ConfigValueType::Array;
            break;
        default:
            _type = ConfigValueType::String;
    }

    // 默认值会在 getDefaultValue() 中从对应控件获取

    accept();
}

void AddConfigItemDialog::onCancelClicked()
{
    reject();
}

bool AddConfigItemDialog::validateInput()
{
    // 验证自定义 Section（如果选择了"新建分组"）
    int sectionIndex = _sectionComboBox->currentIndex();
    if (sectionIndex == 0) {
        QString customSection = _customSectionInput->text().trimmed();
        if (customSection.isEmpty()) {
            ElaMessageBar::error(ElaMessageBarType::Top, "错误", "请输入新分组名称", 2000, this);
            return false;
        }
        // 验证 Section 名称格式（允许字母、数字、下划线、中文）
        // 使用 QString 构造正则表达式，避免编码问题
        QRegularExpression sectionRegex(QString::fromUtf8("^[a-zA-Z0-9_\u4e00-\u9fa5]+$"));
        if (!sectionRegex.match(customSection).hasMatch()) {
            ElaMessageBar::error(ElaMessageBarType::Top, "错误",
                                "分组名称格式不正确，只能包含字母、数字、下划线和中文",
                                2000, this);
            return false;
        }
    }

    QString key = _keyInput->text().trimmed();

    // 验证键名
    if (key.isEmpty()) {
        ElaMessageBar::error(ElaMessageBarType::Top, "错误", "键名不能为空", 2000, this);
        return false;
    }

    // 验证键名格式（允许字母、数字、点、下划线、中括号、中文）
    QRegularExpression keyRegex(QString::fromUtf8("^[a-zA-Z0-9_.\u4e00-\u9fa5\\[\\]]+$"));
    if (!keyRegex.match(key).hasMatch()) {
        ElaMessageBar::error(ElaMessageBarType::Top, "错误",
                            "键名格式不正确，只能包含字母、数字、点、下划线、中括号和中文",
                            2000, this);
        return false;
    }

    // 根据类型验证默认值
    int typeIndex = _typeComboBox->currentIndex();

    switch (typeIndex) {
        case 0: { // String
            // 字符串无需特殊验证
            break;
        }
        case 1: { // Integer
            // 整数输入框已自动限制，无需额外验证
            break;
        }
        case 2: { // Double
            // 浮点数输入框已自动限制，无需额外验证
            break;
        }
        case 3: { // Boolean
            // 开关控件无需验证
            break;
        }
        case 4: { // Array
            QString value = _valueInput->text().trimmed();
            if (!value.isEmpty()) {
                // 验证数组格式：必须以 [ 开头，] 结尾
                if (!value.startsWith('[') || !value.endsWith(']')) {
                    ElaMessageBar::error(ElaMessageBarType::Top, "错误",
                                        "数组格式不正确，必须以 [ 开头，] 结尾",
                                        2000, this);
                    return false;
                }

                // 验证括号匹配
                int bracketCount = 0;
                for (QChar c : value) {
                    if (c == '[') bracketCount++;
                    else if (c == ']') bracketCount--;
                    if (bracketCount < 0) {
                        ElaMessageBar::error(ElaMessageBarType::Top, "错误",
                                            "数组格式不正确，括号不匹配",
                                            2000, this);
                        return false;
                    }
                }
                if (bracketCount != 0) {
                    ElaMessageBar::error(ElaMessageBarType::Top, "错误",
                                        "数组格式不正确，括号不匹配",
                                        2000, this);
                    return false;
                }
            }
            break;
        }
    }

    return true;
}

QString AddConfigItemDialog::getKey() const
{
    // 返回完整路径：Section.Key
    if (_section.isEmpty()) {
        return _key;  // 理论上不应该发生，因为必须选择 Section
    }
    return _section + "." + _key;
}

ConfigValueType AddConfigItemDialog::getType() const
{
    return _type;
}

QString AddConfigItemDialog::getDefaultValue() const
{
    // 根据类型从对应的控件获取值
    switch (_type) {
        case ConfigValueType::String:
            return _valueInput->text().trimmed();
        case ConfigValueType::Integer:
            return QString::number(_intSpinBox->value());
        case ConfigValueType::Double:
            return QString::number(_doubleSpinBox->value());
        case ConfigValueType::Boolean:
            return _boolToggle->getIsToggled() ? "true" : "false";
        case ConfigValueType::Array: {
            QString value = _valueInput->text().trimmed();
            return value.isEmpty() ? "[]" : value;
        }
        default:
            return "";
    }
}

void AddConfigItemDialog::setSections(const QStringList& sections)
{
    // 清空现有选项（保留第一个 "+ 新建分组..."）
    while (_sectionComboBox->count() > 1) {
        _sectionComboBox->removeItem(1);
    }

    // 添加传入的 sections（在 "+ 新建分组..." 之后）
    for (const QString& section : sections) {
        if (!section.isEmpty() && section != "General") {
            _sectionComboBox->addItem(section);
        }
    }

    // 如果有已存在的 section，默认选择第一个已存在的（而不是"新建分组"）
    if (_sectionComboBox->count() > 1) {
        _sectionComboBox->setCurrentIndex(1);
        _customSectionInput->setVisible(false);
    } else {
        // 没有已存在的 section，显示自定义输入框
        _customSectionInput->setVisible(true);
    }
}

} // namespace Prism
