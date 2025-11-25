#include "ProjectManager.h"
#include "ProcessRunner.h"
#include "ParserFactory.h"
#include <QDebug>
#include <QFileInfo>

namespace Prism {

ProjectManager::ProjectManager(QObject* parent)
    : QObject(parent)
{
}

ProjectManager::~ProjectManager() {
    // 停止所有运行中的进程
    for (auto it = m_projects.begin(); it != m_projects.end(); ++it) {
        if (it.value()->isRunning && it.value()->processRunner) {
            it.value()->processRunner->stop(true);
        }
    }
}

bool ProjectManager::createProject(const QString& name, const QString& configPath) {
    if (m_projects.contains(name)) {
        qWarning() << "[ProjectManager] Project already exists:" << name;
        return false;
    }

    QFileInfo fileInfo(configPath);
    if (!fileInfo.exists()) {
        qWarning() << "[ProjectManager] Config file not found:" << configPath;
        return false;
    }

    auto project = std::make_shared<Project>(name, configPath);
    m_projects[name] = project;

    qDebug() << "[ProjectManager] Created project:" << name << "with config:" << configPath;
    emit projectCreated(name);

    return true;
}

bool ProjectManager::loadProject(const QString& projectName) {
    auto project = getProject(projectName);
    if (!project) {
        qWarning() << "[ProjectManager] Project not found:" << projectName;
        return false;
    }

    // 使用 ParserFactory 解析配置文件
    ParserFactory& factory = ParserFactory::instance();
    project->configData = factory.parse(project->configPath);

    if (project->configData.isEmpty()) {
        qWarning() << "[ProjectManager] Failed to load config:" << factory.getLastError();
        return false;
    }

    qDebug() << "[ProjectManager] Loaded project:" << projectName
             << "with" << project->configData.size() << "config entries";
    emit projectLoaded(projectName);

    return true;
}

bool ProjectManager::removeProject(const QString& projectName) {
    if (!m_projects.contains(projectName)) {
        return false;
    }

    // 停止进程
    stopProject(projectName, true);

    m_projects.remove(projectName);
    qDebug() << "[ProjectManager] Removed project:" << projectName;
    emit projectRemoved(projectName);

    return true;
}

Project* ProjectManager::getProject(const QString& projectName) {
    auto it = m_projects.find(projectName);
    return (it != m_projects.end()) ? it.value().get() : nullptr;
}

QStringList ProjectManager::getAllProjectNames() const {
    return m_projects.keys();
}

bool ProjectManager::runProject(const QString& projectName, const QString& program, const QStringList& arguments) {
    auto project = getProject(projectName);
    if (!project) {
        qWarning() << "[ProjectManager] Project not found:" << projectName;
        return false;
    }

    if (project->isRunning) {
        qWarning() << "[ProjectManager] Project is already running:" << projectName;
        return false;
    }

    // 创建进程运行器
    if (!project->processRunner) {
        project->processRunner = std::make_shared<ProcessRunner>();

        // 连接信号
        connect(project->processRunner.get(), &ProcessRunner::started, this, [this, projectName]() {
            auto proj = getProject(projectName);
            if (proj) {
                proj->isRunning = true;
                emit projectStateChanged(projectName, true);
            }
        });

        connect(project->processRunner.get(), &ProcessRunner::finished, this,
                [this, projectName](int exitCode, QProcess::ExitStatus status) {
            Q_UNUSED(exitCode);
            Q_UNUSED(status);
            auto proj = getProject(projectName);
            if (proj) {
                proj->isRunning = false;
                emit projectStateChanged(projectName, false);
            }
        });
    }

    // 启动进程
    bool success = project->processRunner->start(program, arguments);
    if (success) {
        qDebug() << "[ProjectManager] Started project:" << projectName;
    }

    return success;
}

void ProjectManager::stopProject(const QString& projectName, bool forceKill) {
    auto project = getProject(projectName);
    if (!project || !project->processRunner) {
        return;
    }

    project->processRunner->stop(forceKill);
    project->isRunning = false;
    emit projectStateChanged(projectName, false);

    qDebug() << "[ProjectManager] Stopped project:" << projectName;
}

std::shared_ptr<ProcessRunner> ProjectManager::getProcessRunner(const QString& projectName) {
    auto project = getProject(projectName);
    return project ? project->processRunner : nullptr;
}

} // namespace Prism
