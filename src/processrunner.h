#ifndef PROCESSRUNNER_H
#define PROCESSRUNNER_H

#include <QObject>
#include <QProcess>
#include "appitem.h"

class ProcessRunner : public QObject {
    Q_OBJECT
public:
    enum State {
        NotRunning,
        Starting,
        Running,
        Finished,
        FailedToStart
    };
    Q_ENUM(State)

    explicit ProcessRunner(QObject *parent = nullptr);
    ~ProcessRunner();

    bool start(const AppItem& item);
    void stop();
    void killProcess();

    State currentState() const { return m_state; }
    qint64 currentPid() const;
    QString appId() const { return m_appId; }

signals:
    void stateChanged(const QString& appId, ProcessRunner::State state, qint64 pid, int exitCode);
    void outputReceived(const QString& appId, const QString& text, bool isError);

private slots:
    void onReadyReadStdout();
    void onReadyReadStderr();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onErrorOccurred(QProcess::ProcessError error);
    void onProcessStateChanged(QProcess::ProcessState newState);

private:
    QProcess *m_process;
    QString m_appId;
    State m_state;
    void setState(State state, int exitCode = 0);
};

#endif // PROCESSRUNNER_H
