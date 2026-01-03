#include "SnapshotManager.h"
#include "ParserFactory.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTextStream>
#include <QDebug>

// ========== SnapshotMetadata 实现 ==========

QJsonObject SnapshotMetadata::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["name"] = name;
    json["description"] = description;
    json["createTime"] = createTime.toString(Qt::ISODate);
    json["isActive"] = isActive;

    QJsonArray filesArray;
    for (const QString& file : files) {
        filesArray.append(file);
    }
    json["files"] = filesArray;

    return json;
}

SnapshotMetadata SnapshotMetadata::fromJson(const QJsonObject& json) {
    SnapshotMetadata metadata;
    metadata.id = json["id"].toString();
    metadata.name = json["name"].toString();
    metadata.description = json["description"].toString();
    metadata.createTime = QDateTime::fromString(json["createTime"].toString(), Qt::ISODate);
    metadata.isActive = json["isActive"].toBool();

    QJsonArray filesArray = json["files"].toArray();
    for (const QJsonValue& value : filesArray) {
        metadata.files.append(value.toString());
    }

    return metadata;
}

// ========== SnapshotManager 实现 ==========

SnapshotManager& SnapshotManager::instance() {
    static SnapshotManager instance;
    return instance;
}

QString SnapshotManager::createSnapshot(const QString& projectPath,
                                       const QString& name,
                                       const QString& description,
                                       const QStringList& files,
                                       QString& errorMsg) {
    // 1. 检查快照数量限制
    if (getSnapshotCount(projectPath) >= MAX_SNAPSHOTS_PER_PROJECT) {
        errorMsg = QString("快照数量已达上限（%1个），请删除旧快照后再创建").arg(MAX_SNAPSHOTS_PER_PROJECT);
        return QString();
    }

    // 2. 验证所有文件格式
    for (const QString& relativeFilePath : files) {
        QString fullPath = QDir(projectPath).filePath(relativeFilePath);
        QString validateError;
        if (!validateConfigFile(fullPath, validateError)) {
            errorMsg = QString("文件 %1 格式验证失败: %2").arg(relativeFilePath, validateError);
            return QString();
        }
    }

    // 3. 生成快照ID和名称
    QString snapshotId = generateSnapshotId();
    QString snapshotName = name.isEmpty() ? generateDefaultSnapshotName(projectPath) : name;

    // 4. 创建快照目录
    QString snapshotDir = getSnapshotPath(projectPath, snapshotId);
    if (!ensureDirectoryExists(snapshotDir)) {
        errorMsg = "创建快照目录失败";
        return QString();
    }

    // 5. 复制文件到快照目录
    for (const QString& relativeFilePath : files) {
        QString srcPath = QDir(projectPath).filePath(relativeFilePath);
        QString dstPath = QDir(snapshotDir).filePath(relativeFilePath);

        // 确保目标文件的父目录存在
        QFileInfo dstFileInfo(dstPath);
        if (!ensureDirectoryExists(dstFileInfo.absolutePath())) {
            errorMsg = QString("创建目录失败: %1").arg(dstFileInfo.absolutePath());
            return QString();
        }

        if (!copyFile(srcPath, dstPath)) {
            errorMsg = QString("复制文件失败: %1").arg(relativeFilePath);
            return QString();
        }
    }

    // 6. 创建元数据
    SnapshotMetadata metadata;
    metadata.id = snapshotId;
    metadata.name = snapshotName;
    metadata.description = description;
    metadata.createTime = QDateTime::currentDateTime();
    metadata.files = files;
    metadata.isActive = false;

    // 7. 保存元数据
    if (!saveMetadata(projectPath, metadata)) {
        errorMsg = "保存快照元数据失败";
        return QString();
    }

    qDebug() << "[SnapshotManager] 快照创建成功:" << snapshotName << "ID:" << snapshotId;
    emit snapshotCreated(projectPath, snapshotId);

    return snapshotId;
}

QList<SnapshotMetadata> SnapshotManager::getSnapshots(const QString& projectPath) {
    QList<SnapshotMetadata> snapshots;

    QString snapshotsDir = getSnapshotsDir(projectPath);
    QDir dir(snapshotsDir);

    if (!dir.exists()) {
        return snapshots;
    }

    // 获取所有快照目录
    QStringList snapshotDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString& snapshotId : snapshotDirs) {
        SnapshotMetadata metadata = loadMetadata(projectPath, snapshotId);
        if (!metadata.id.isEmpty()) {
            snapshots.append(metadata);
        }
    }

    // 按创建时间倒序排序
    std::sort(snapshots.begin(), snapshots.end(), [](const SnapshotMetadata& a, const SnapshotMetadata& b) {
        return a.createTime > b.createTime;
    });

    return snapshots;
}

bool SnapshotManager::applySnapshot(const QString& projectPath,
                                   const QString& snapshotId,
                                   QString& errorMsg) {
    // 1. 加载快照元数据
    SnapshotMetadata metadata = loadMetadata(projectPath, snapshotId);
    if (metadata.id.isEmpty()) {
        errorMsg = "快照不存在或元数据损坏";
        return false;
    }

    // 2. 复制快照文件到项目目录
    QString snapshotDir = getSnapshotPath(projectPath, snapshotId);

    for (const QString& relativeFilePath : metadata.files) {
        QString srcPath = QDir(snapshotDir).filePath(relativeFilePath);
        QString dstPath = QDir(projectPath).filePath(relativeFilePath);

        // 确保目标文件的父目录存在
        QFileInfo dstFileInfo(dstPath);
        if (!ensureDirectoryExists(dstFileInfo.absolutePath())) {
            errorMsg = QString("创建目录失败: %1").arg(dstFileInfo.absolutePath());
            return false;
        }

        // 删除旧文件（如果存在）
        if (QFile::exists(dstPath)) {
            QFile::remove(dstPath);
        }

        if (!copyFile(srcPath, dstPath)) {
            errorMsg = QString("恢复文件失败: %1").arg(relativeFilePath);
            return false;
        }
    }

    // 3. 设置为当前激活的快照
    setActiveSnapshot(projectPath, snapshotId);

    qDebug() << "[SnapshotManager] 快照应用成功:" << metadata.name;
    emit snapshotApplied(projectPath, snapshotId);

    return true;
}

bool SnapshotManager::deleteSnapshot(const QString& projectPath, const QString& snapshotId) {
    QString snapshotDir = getSnapshotPath(projectPath, snapshotId);

    QDir dir(snapshotDir);
    if (!dir.exists()) {
        qWarning() << "[SnapshotManager] 快照目录不存在:" << snapshotDir;
        return false;
    }

    // 递归删除快照目录
    if (!dir.removeRecursively()) {
        qWarning() << "[SnapshotManager] 删除快照目录失败:" << snapshotDir;
        return false;
    }

    // 如果删除的是当前激活的快照，清除激活状态
    if (getActiveSnapshotId(projectPath) == snapshotId) {
        setActiveSnapshot(projectPath, QString());
    }

    qDebug() << "[SnapshotManager] 快照删除成功:" << snapshotId;
    emit snapshotDeleted(projectPath, snapshotId);

    return true;
}

bool SnapshotManager::renameSnapshot(const QString& projectPath,
                                    const QString& snapshotId,
                                    const QString& newName,
                                    const QString& newDescription) {
    // 加载元数据
    SnapshotMetadata metadata = loadMetadata(projectPath, snapshotId);
    if (metadata.id.isEmpty()) {
        qWarning() << "[SnapshotManager] 快照不存在:" << snapshotId;
        return false;
    }

    // 更新名称和描述
    metadata.name = newName;
    if (!newDescription.isNull()) {
        metadata.description = newDescription;
    }

    // 保存元数据
    if (!saveMetadata(projectPath, metadata)) {
        qWarning() << "[SnapshotManager] 保存元数据失败";
        return false;
    }

    qDebug() << "[SnapshotManager] 快照重命名成功:" << newName;
    emit snapshotRenamed(projectPath, snapshotId);

    return true;
}

QString SnapshotManager::getActiveSnapshotId(const QString& projectPath) {
    QString filePath = getActiveSnapshotFilePath(projectPath);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }

    QTextStream in(&file);
    QString snapshotId = in.readLine().trimmed();
    file.close();

    return snapshotId;
}

void SnapshotManager::setActiveSnapshot(const QString& projectPath, const QString& snapshotId) {
    QString filePath = getActiveSnapshotFilePath(projectPath);

    // 确保目录存在
    QFileInfo fileInfo(filePath);
    ensureDirectoryExists(fileInfo.absolutePath());

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "[SnapshotManager] 无法写入激活快照文件:" << filePath;
        return;
    }

    QTextStream out(&file);
    out << snapshotId;
    file.close();

    qDebug() << "[SnapshotManager] 设置激活快照:" << snapshotId;
}

int SnapshotManager::getSnapshotCount(const QString& projectPath) {
    return getSnapshots(projectPath).size();
}

QString SnapshotManager::generateDefaultSnapshotName(const QString& projectPath) {
    int count = getSnapshotCount(projectPath);
    return QString("配置快照 #%1").arg(count + 1);
}

bool SnapshotManager::validateConfigFile(const QString& filePath, QString& errorMsg) {
    QFileInfo fileInfo(filePath);

    // 检查文件是否存在
    if (!fileInfo.exists()) {
        errorMsg = "文件不存在";
        return false;
    }

    // 检查文件是否可读
    if (!fileInfo.isReadable()) {
        errorMsg = "文件不可读";
        return false;
    }

    // 根据文件扩展名验证格式
    QString suffix = fileInfo.suffix().toLower();

    try {
        if (suffix == "json") {
            // 验证 JSON 格式
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly)) {
                errorMsg = "无法打开文件";
                return false;
            }

            QByteArray data = file.readAll();
            file.close();

            QJsonParseError parseError;
            QJsonDocument::fromJson(data, &parseError);

            if (parseError.error != QJsonParseError::NoError) {
                errorMsg = QString("JSON 解析错误: %1").arg(parseError.errorString());
                return false;
            }

        } else if (suffix == "yaml" || suffix == "yml") {
            // 验证 YAML 格式（使用 ParserFactory）
            auto parser = Prism::ParserFactory::instance().createParser(Prism::ConfigFormat::YAML);
            QVariantMap data = parser->parse(filePath);

            if (data.isEmpty() && QFileInfo(filePath).size() > 0) {
                errorMsg = "YAML 解析失败或文件为空";
                return false;
            }

        } else if (suffix == "ini" || suffix == "cfg" || suffix == "conf") {
            // 验证 INI 格式
            auto parser = Prism::ParserFactory::instance().createParser(Prism::ConfigFormat::INI);
            QVariantMap data = parser->parse(filePath);

            if (data.isEmpty() && QFileInfo(filePath).size() > 0) {
                errorMsg = "INI 解析失败或文件为空";
                return false;
            }

        } else {
            // 未知格式，仅检查文件是否可读
            errorMsg = QString("未知文件格式: %1").arg(suffix);
            return false;
        }

    } catch (const std::exception& e) {
        errorMsg = QString("验证异常: %1").arg(e.what());
        return false;
    } catch (...) {
        errorMsg = "未知验证错误";
        return false;
    }

    return true;
}

// ========== 私有方法 ==========

QString SnapshotManager::getSnapshotsDir(const QString& projectPath) const {
    return QDir(projectPath).filePath(".prism/snapshots");
}

QString SnapshotManager::getSnapshotPath(const QString& projectPath, const QString& snapshotId) const {
    return QDir(getSnapshotsDir(projectPath)).filePath(snapshotId);
}

QString SnapshotManager::getMetadataPath(const QString& projectPath, const QString& snapshotId) const {
    return QDir(getSnapshotPath(projectPath, snapshotId)).filePath("metadata.json");
}

QString SnapshotManager::getActiveSnapshotFilePath(const QString& projectPath) const {
    return QDir(projectPath).filePath(".prism/active_snapshot.txt");
}

QString SnapshotManager::generateSnapshotId() const {
    return QString("snapshot_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
}

bool SnapshotManager::saveMetadata(const QString& projectPath, const SnapshotMetadata& metadata) {
    QString metadataPath = getMetadataPath(projectPath, metadata.id);

    QFile file(metadataPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "[SnapshotManager] 无法写入元数据文件:" << metadataPath;
        return false;
    }

    QJsonDocument doc(metadata.toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

SnapshotMetadata SnapshotManager::loadMetadata(const QString& projectPath, const QString& snapshotId) {
    QString metadataPath = getMetadataPath(projectPath, snapshotId);

    QFile file(metadataPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[SnapshotManager] 无法读取元数据文件:" << metadataPath;
        return SnapshotMetadata();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[SnapshotManager] 元数据解析失败:" << parseError.errorString();
        return SnapshotMetadata();
    }

    return SnapshotMetadata::fromJson(doc.object());
}

bool SnapshotManager::copyFile(const QString& srcPath, const QString& dstPath) {
    // 如果目标文件已存在，先删除
    if (QFile::exists(dstPath)) {
        QFile::remove(dstPath);
    }

    if (!QFile::copy(srcPath, dstPath)) {
        qWarning() << "[SnapshotManager] 复制文件失败:" << srcPath << "->" << dstPath;
        return false;
    }

    return true;
}

bool SnapshotManager::ensureDirectoryExists(const QString& dirPath) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "[SnapshotManager] 创建目录失败:" << dirPath;
            return false;
        }
    }
    return true;
}
