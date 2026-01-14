#pragma once

#include <QWidget>
#include <QStringList>

class ElaComboBox;
class ElaLineEdit;
class ElaPushButton;
class ElaCheckBox;

namespace Prism {

/**
 * @brief 日志工具栏组件
 *
 * 提供日志过滤、搜索和导出功能
 */
class LogToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit LogToolBar(QWidget* parent = nullptr);

    /**
     * @brief 获取当前过滤的日志级别
     */
    QStringList getFilterLevels() const;

    /**
     * @brief 获取当前搜索关键字
     */
    QString getSearchKeyword() const;

    /**
     * @brief 清空搜索框
     */
    void clearSearch();

signals:
    /**
     * @brief 过滤条件改变信号
     * @param levels 选中的日志级别列表
     */
    void filterChanged(const QStringList& levels);

    /**
     * @brief 搜索请求信号
     * @param keyword 搜索关键字
     */
    void searchRequested(const QString& keyword);

    /**
     * @brief 导出请求信号
     */
    void exportRequested();

    /**
     * @brief 清空日志请求信号
     */
    void clearRequested();

private slots:
    void onFilterChanged();
    void onSearchTextChanged(const QString& text);
    void onSearchTriggered();

private:
    void initUI();
    void setupConnections();

    // 过滤复选框
    ElaCheckBox* m_infoCheck{ nullptr };
    ElaCheckBox* m_successCheck{ nullptr };
    ElaCheckBox* m_warningCheck{ nullptr };
    ElaCheckBox* m_errorCheck{ nullptr };
    ElaCheckBox* m_debugCheck{ nullptr };
    ElaCheckBox* m_stdoutCheck{ nullptr };
    ElaCheckBox* m_stderrCheck{ nullptr };

    // 搜索框
    ElaLineEdit* m_searchEdit{ nullptr };

    // 按钮
    ElaPushButton* m_searchBtn{ nullptr };
    ElaPushButton* m_exportBtn{ nullptr };
    ElaPushButton* m_clearBtn{ nullptr };
};

} // namespace Prism
