#include "appitem.h"
#include <QUuid>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

AppItem::AppItem() {
    id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QJsonObject AppItem::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["name"] = name;
    json["description"] = description;
    json["scriptPath"] = scriptPath;
    json["interpreterPath"] = interpreterPath;
    json["workingDir"] = workingDir;
    json["arguments"] = arguments;
    json["iconPath"] = iconPath;
    json["keepAliveAfterExit"] = keepAliveAfterExit;
    return json;
}

AppItem AppItem::fromJson(const QJsonObject& json) {
    AppItem item;
    if (json.contains("id") && !json["id"].toString().isEmpty()) {
        item.id = json["id"].toString();
    }
    item.name = json["name"].toString();
    item.description = json["description"].toString();
    item.scriptPath = json["scriptPath"].toString();
    item.interpreterPath = json["interpreterPath"].toString();
    item.workingDir = json["workingDir"].toString();
    item.arguments = json["arguments"].toString();
    item.iconPath = json["iconPath"].toString();
    item.keepAliveAfterExit = json.contains("keepAliveAfterExit") ? json["keepAliveAfterExit"].toBool(true) : true;
    return item;
}

QString AppItem::autoDetectInterpreter(const QString& scriptPath) {
    if (!scriptPath.isEmpty()) {
        QFileInfo scriptInfo(scriptPath);
        QDir dir = scriptInfo.dir();

        // 1. Check venv or .venv in script directory
        QString venvPath = dir.filePath("venv/bin/python");
        if (QFileInfo::exists(venvPath)) return venvPath;

        QString dotVenvPath = dir.filePath(".venv/bin/python");
        if (QFileInfo::exists(dotVenvPath)) return dotVenvPath;

        // 2. Check parent directory venv
        dir.cdUp();
        venvPath = dir.filePath("venv/bin/python");
        if (QFileInfo::exists(venvPath)) return venvPath;
        dotVenvPath = dir.filePath(".venv/bin/python");
        if (QFileInfo::exists(dotVenvPath)) return dotVenvPath;
    }

    // 3. Fallback to system python3
    if (QFileInfo::exists("/usr/bin/python3")) {
        return "/usr/bin/python3";
    }

    return "python3";
}
