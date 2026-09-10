#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QMap>
#include <QDateTime>

class LogViewer : public QWidget {
    Q_OBJECT
public:
    explicit LogViewer(QWidget *parent = nullptr);

    void appendLog(const QString& appId, const QString& appName, const QString& text, bool isError);
    void registerApp(const QString& appId, const QString& appName);
    void unregisterApp(const QString& appId);
    void selectAppTab(const QString& appId);
    void retranslateUi();

private slots:
    void onAppSelectorChanged(int index);
    void clearCurrentLog();
    void saveLogToFile();

private:
    struct AppLogBuffer {
        QString appName;
        QString htmlContent;
    };

    QLabel *m_lblTitle;
    QLabel *m_lblTarget;
    QComboBox *m_comboApps;
    QTextEdit *m_textConsole;
    QCheckBox *m_chkAutoScroll;
    QPushButton *m_btnClear;
    QPushButton *m_btnSave;

    QMap<QString, AppLogBuffer> m_logs; // appId -> AppLogBuffer
    QString m_currentAppId;

    void updateConsoleDisplay();
    QString escapeHtml(const QString& plain);
};

#endif // LOGVIEWER_H
