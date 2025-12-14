#ifndef ADDCONFIGITEMDIALOG_H
#define ADDCONFIGITEMDIALOG_H

#include "ElaDialog.h"
#include "ConfigPage.h"
#include <QString>
#include <QStringList>

class ElaLineEdit;
class ElaComboBox;
class ElaPushButton;
class ElaText;
class ElaSpinBox;
class ElaDoubleSpinBox;
class ElaToggleSwitch;
class QVBoxLayout;
class QHBoxLayout;
class QWidget;

namespace Prism {

/**
 * @brief 添加新配置项对话框
 *
 * 用于在组件模式下添加新的配置项
 * 支持设置 Section、键名、类型和默认值
 */
class AddConfigItemDialog : public ElaDialog
{
    Q_OBJECT

public:
    explicit AddConfigItemDialog(QWidget* parent = nullptr);
    ~AddConfigItemDialog() override;

    /**
     * @brief 设置可用的 Section 列表
     * @param sections Section 名称列表
     */
    void setSections(const QStringList& sections);

    /**
     * @brief 获取用户输入的完整键名（包含 Section 前缀）
     * @return 完整键名（如 "UI.timeout"）
     */
    QString getKey() const;

    /**
     * @brief 获取用户选择的类型
     * @return 配置值类型
     */
    ConfigValueType getType() const;

    /**
     * @brief 获取用户输入的默认值
     * @return 默认值
     */
    QString getDefaultValue() const;

private slots:
    void onSectionChanged(int index);
    void onTypeChanged(int index);
    void onConfirmClicked();
    void onCancelClicked();

private:
    void initUI();
    void setupConnections();
    bool validateInput();

private:
    // UI 组件
    ElaComboBox* _sectionComboBox{ nullptr };     // Section 选择
    ElaLineEdit* _customSectionInput{ nullptr };  // 自定义 Section 输入框
    ElaLineEdit* _keyInput{ nullptr };
    ElaComboBox* _typeComboBox{ nullptr };

    // 默认值输入控件（根据类型动态切换）
    QWidget* _valueContainer{ nullptr };
    QHBoxLayout* _valueLayout{ nullptr };
    ElaLineEdit* _valueInput{ nullptr };          // 字符串/数组
    ElaSpinBox* _intSpinBox{ nullptr };           // 整数
    ElaDoubleSpinBox* _doubleSpinBox{ nullptr };  // 浮点数
    ElaToggleSwitch* _boolToggle{ nullptr };      // 布尔值

    ElaPushButton* _confirmButton{ nullptr };
    ElaPushButton* _cancelButton{ nullptr };

    // 数据
    QString _section;                              // 选择的 Section
    QString _key;
    ConfigValueType _type{ ConfigValueType::String };
    QString _defaultValue;
};

} // namespace Prism

#endif // ADDCONFIGITEMDIALOG_H
