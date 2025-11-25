#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariantMap>
#include <memory>

namespace Prism {

class ProcessRunner;

/**
 * @brief 项目数据结构
 */
struct Project {
    QString name;
    QString configPath;
    QVariantMap configData;
    std::shared_ptr<ProcessRunner> processRunner;
    bool isRunning = false;

    Project() = default;
    Project(const QString& n, const QString& path)
        : name(n), configPath(path), isRunning(false) {}
};

/**
 * @brief 项目管理器
 * 负责管理多个项目的配置和进程
 * 设计为支持未来的 IDataSource 抽象（局域网同步等）
 */
class ProjectManager : public QObject {
    Q_OBJECT

public:
    explicit ProjectManager(QObject* parent = nullptr);
    ~ProjectManager() override;

    /**
     * @brief 创建新项目
     * @param name 项目名称
     * @param configPath 配置文件路径
     * @return 是否成功
     */
    bool createProject(const QString& name, const QString& configPath);

    /**
     * @brief 加载项目配置
     * @param projectName 项目名称
     * @return 是否成功
     */
    bool loadProject(const QString& projectName);

    /**
     * @brief 删除项目
     * @param projectName 项目名称
     * @return 是否成功
     */
    bool removeProject(const QString& projectName);

    /**
     * @brief 获取项目
     * @param projectName 项目名称
     * @return 项目指针（如果不存在返回 nullptr）
     */
    Project* getProject(const QString& projectName);

    /**
     * @brief 获取所有项目名称
     * @return 项目名称列表
     */
    QStringList getAllProjectNames() const;

    /**
     * @brief 运行项目进程
     * @param projectName 项目名称
     * @param program 程序路径
     * @param arguments 命令行参数
     * @return 是否成功启动
     */
    bool runProject(const QString& projectName, const QString& program, const QStringList& arguments);

    /**
     * @brief 停止项目进程
     * @param projectName 项目名称
     * @param forceKill 是否强制杀死
     */
    void stopProject(const QString& projectName, bool forceKill = false);

    /**
     * @brief 获取项目的进程运行器
     * @param projectName 项目名称
     * @return 进程运行器指针
     */
    std::shared_ptr<ProcessRunner> getProcessRunner(const QString& projectName);

signals:
    /**
     * @brief 项目创建信号
     * @param projectName 项目名称
     */
    void projectCreated(const QString& projectName);

    /**
     * @brief 项目加载信号
     * @param projectName 项目名称
     */
    void projectLoaded(const QString& projectName);

    /**
     * @brief 项目删除信号
     * @param projectName 项目名称
     */
    void projectRemoved(const QString& projectName);

    /**
     * @brief 项目运行状态变化信号
     * @param projectName 项目名称
     * @param isRunning 是否正在运行
     */
    void projectStateChanged(const QString& projectName, bool isRunning);

private:
    QMap<QString, std::shared_ptr<Project>> m_projects;
};

} // namespace Prism
