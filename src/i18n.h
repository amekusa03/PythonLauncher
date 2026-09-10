#ifndef I18N_H
#define I18N_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QStringList>

class I18n : public QObject {
    Q_OBJECT
public:
    enum class Language {
        Japanese = 0,
        English = 1
    };
    Q_ENUM(Language)

    static I18n& instance();

    Language currentLanguage() const { return m_currentLang; }
    void setLanguage(Language lang);

    QString t(const QString& key, const QStringList& args = {}) const;

    static QString trStr(const QString& key, const QStringList& args = {}) {
        return instance().t(key, args);
    }

signals:
    void languageChanged(I18n::Language lang);

private:
    explicit I18n(QObject *parent = nullptr);
    void initDictionary();

    Language m_currentLang;
    QMap<QString, QString> m_dictJa;
    QMap<QString, QString> m_dictEn;
};

inline QString TR(const QString& key) {
    return I18n::trStr(key);
}

inline QString TR_ARGS(const QString& key, const QStringList& args) {
    return I18n::trStr(key, args);
}

#endif // I18N_H
