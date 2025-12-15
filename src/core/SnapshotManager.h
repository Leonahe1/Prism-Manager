#ifndef SNAPSHOTMANAGER_H
#define SNAPSHOTMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QJsonObject>

/**
 * @brief 快照元数据结构
 */
struct SnapshotMetadata {
    QString id;              // 唯一标识（时间戳格式）
    QString name;            // 快照名称
    QString description;     // 快照描述
    QDateTime createTime;    // 创建时间
    QStringList files;       // 包含的文件相对路径列表
    bool isActive;           // 是否为当前激活的快照

    // 转换为 JSON
    QJsonObject toJson() const;
    // 从 JSON 加载
    static SnapshotMetadata fromJson(const QJsonObject& json);
};

/**
 * @brief 配置快照管理器（单例模式）
 *
 * 功能：
 * - 创建项目配置快照
 * - 应用快照（一键切换配置）
 * - 管理快照（查看、重命名、删除）
 * - 快照数量限制（每个项目最多20个）
 */
class SnapshotManager : public QObject {
    Q_OBJECT

public:
    static SnapshotManager& instance();

    /**
     * @brief 创建配置快照
     * @param projectPath 项目根目录路径
     * @param name 快照名称（为空则使用默认名称）
     * @param description 快照描述
     * @param files 要包含的文件相对路径列表
     * @param errorMsg 错误信息输出
     * @return 成功返回快照ID，失败返回空字符串
     */
    QString createSnapshot(const QString& projectPath,
                          const QString& name,
                          const QString& description,
                          const QStringList& files,
                          QString& errorMsg);

    /**
     * @brief 获取项目的所有快照
     * @param projectPath 项目根目录路径
     * @return 快照元数据列表（按创建时间倒序）
     */
    QList<SnapshotMetadata> getSnapshots(const QString& projectPath);

    /**
     * @brief 应用快照（恢复配置）
     * @param projectPath 项目根目录路径
     * @param snapshotId 快照ID
     * @param errorMsg 错误信息输出
     * @return 是否成功
     */
    bool applySnapshot(const QString& projectPath,
                      const QString& snapshotId,
                      QString& errorMsg);

    /**
     * @brief 删除快照
     * @param projectPath 项目根目录路径
     * @param snapshotId 快照ID
     * @return 是否成功
     */
    bool deleteSnapshot(const QString& projectPath,
                       const QString& snapshotId);

    /**
     * @brief 重命名快照
     * @param projectPath 项目根目录路径
     * @param snapshotId 快照ID
     * @param newName 新名称
     * @param newDescription 新描述（可选）
     * @return 是否成功
     */
    bool renameSnapshot(const QString& projectPath,
                       const QString& snapshotId,
                       const QString& newName,
                       const QString& newDescription = QString());

    /**
     * @brief 获取当前激活的快照ID
     * @param projectPath 项目根目录路径
     * @return 快照ID，如果没有则返回空字符串
     */
    QString getActiveSnapshotId(const QString& projectPath);

    /**
     * @brief 设置当前激活的快照
     * @param projectPath 项目根目录路径
     * @param snapshotId 快照ID
     */
    void setActiveSnapshot(const QString& projectPath, const QString& snapshotId);

    /**
     * @brief 获取快照数量
     * @param projectPath 项目根目录路径
     * @return 快照数量
     */
    int getSnapshotCount(const QString& projectPath);

    /**
     * @brief 生成默认快照名称
     * @param projectPath 项目根目录路径
     * @return 默认名称（如 "配置快照 #1"）
     */
    QString generateDefaultSnapshotName(const QString& projectPath);

    /**
     * @brief 验证配置文件格式是否正确
     * @param filePath 文件路径
     * @param errorMsg 错误信息输出
     * @return 是否有效
     */
    bool validateConfigFile(const QString& filePath, QString& errorMsg);

signals:
    void snapshotCreated(const QString& projectPath, const QString& snapshotId);
    void snapshotApplied(const QString& projectPath, const QString& snapshotId);
    void snapshotDeleted(const QString& projectPath, const QString& snapshotId);
    void snapshotRenamed(const QString& projectPath, const QString& snapshotId);

private:
    SnapshotManager() = default;
    ~SnapshotManager() = default;
    SnapshotManager(const SnapshotManager&) = delete;
    SnapshotManager& operator=(const SnapshotManager&) = delete;

    // 获取快照目录路径
    QString getSnapshotsDir(const QString& projectPath) const;

    // 获取快照存储路径
    QString getSnapshotPath(const QString& projectPath, const QString& snapshotId) const;

    // 获取快照元数据文件路径
    QString getMetadataPath(const QString& projectPath, const QString& snapshotId) const;

    // 获取激活快照记录文件路径
    QString getActiveSnapshotFilePath(const QString& projectPath) const;

    // 生成快照ID（基于时间戳）
    QString generateSnapshotId() const;

    // 保存快照元数据
    bool saveMetadata(const QString& projectPath, const SnapshotMetadata& metadata);

    // 加载快照元数据
    SnapshotMetadata loadMetadata(const QString& projectPath, const QString& snapshotId);

    // 复制文件
    bool copyFile(const QString& srcPath, const QString& dstPath);

    // 确保目录存在
    bool ensureDirectoryExists(const QString& dirPath);

    static constexpr int MAX_SNAPSHOTS_PER_PROJECT = 20;  // 每个项目最多20个快照
};

#endif // SNAPSHOTMANAGER_H
