#ifndef APPITEM_H
#define APPITEM_H

#include <QString>
#include <QJsonObject>

class AppItem {
public:
    AppItem();

    QString id;
    QString name;
    QString description;
    QString scriptPath;
    QString interpreterPath;
    QString workingDir;
    QString arguments;
    QString iconPath;
    bool keepAliveAfterExit = true;

    QJsonObject toJson() const;
    static AppItem fromJson(const QJsonObject& json);

    static QString autoDetectInterpreter(const QString& scriptPath);
};

#endif // APPITEM_H
