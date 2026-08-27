#include "processrunner.h"
#include <QProcessEnvironment>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

ProcessRunner::ProcessRunner(QObject *parent)
    : QObject(parent), m_process(new QProcess(this)), m_state(NotRunning) {

    connect(m_process, &QProcess::readyReadStandardOutput, this, &ProcessRunner::onReadyReadStdout);
    connect(m_process, &QProcess::readyReadStandardError, this, &ProcessRunner::onReadyReadStderr);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ProcessRunner::onFinished);
    connect(m_process, &QProcess::errorOccurred, this, &ProcessRunner::onErrorOccurred);
    connect(m_process, &QProcess::stateChanged, this, &ProcessRunner::onProcessStateChanged);
}

ProcessRunner::~ProcessRunner() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        if (!m_process->waitForFinished(1000)) {
            m_process->kill();
        }
    }
}

bool ProcessRunner::start(const AppItem& item) {
    if (m_state == Running || m_state == Starting) {
        return false;
    }

    m_appId = item.id;

    // Configure environment for unbuffered Python output
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONUNBUFFERED", "1");
    m_process->setProcessEnvironment(env);

    // Set working directory
    QString workDir = item.workingDir;
    if (workDir.isEmpty() && !item.scriptPath.isEmpty()) {
        workDir = QFileInfo(item.scriptPath).absolutePath();
    }
    if (!workDir.isEmpty()) {
        m_process->setWorkingDirectory(workDir);
    }

    // Prepare program & arguments
    QString program = item.interpreterPath;
    if (program.isEmpty()) {
        program = AppItem::autoDetectInterpreter(item.scriptPath);
    }

    QStringList args;
    if (!item.scriptPath.isEmpty()) {
        args << item.scriptPath;
    }

    if (!item.arguments.trimmed().isEmpty()) {
        // Split extra arguments respecting quotes
        QStringList userArgs = QProcess::splitCommand(item.arguments);
        args.append(userArgs);
    }

    setState(Starting);
    emit outputReceived(m_appId, QString("[システム] 起動中: %1 %2\n").arg(program, args.join(" ")), false);

    m_process->start(program, args);
    return true;
}

void ProcessRunner::stop() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        emit outputReceived(m_appId, "[システム] プロセス停止を要求しました...\n", false);
        m_process->terminate();
    }
}

void ProcessRunner::killProcess() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        emit outputReceived(m_appId, "[システム] プロセスを強制終了(SIGKILL)しました。\n", true);
        m_process->kill();
    }
}

qint64 ProcessRunner::currentPid() const {
    return m_process ? m_process->processId() : 0;
}

void ProcessRunner::setState(State state, int exitCode) {
    m_state = state;
    emit stateChanged(m_appId, m_state, currentPid(), exitCode);
}

void ProcessRunner::onReadyReadStdout() {
    if (!m_process) return;
    QString out = QString::fromUtf8(m_process->readAllStandardOutput());
    if (!out.isEmpty()) {
        emit outputReceived(m_appId, out, false);
    }
}

void ProcessRunner::onReadyReadStderr() {
    if (!m_process) return;
    QString err = QString::fromUtf8(m_process->readAllStandardError());
    if (!err.isEmpty()) {
        emit outputReceived(m_appId, err, true);
    }
}

void ProcessRunner::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitStatus);
    setState(Finished, exitCode);
    emit outputReceived(m_appId, QString("[システム] プロセスが終了しました (終了コード: %1)\n").arg(exitCode), false);
}

void ProcessRunner::onErrorOccurred(QProcess::ProcessError error) {
    if (error == QProcess::FailedToStart) {
        setState(FailedToStart, -1);
        emit outputReceived(m_appId, "[エラー] プロセスの起動に失敗しました。Pythonのパスまたはスクリプトパスを確認してください。\n", true);
    }
}

void ProcessRunner::onProcessStateChanged(QProcess::ProcessState newState) {
    if (newState == QProcess::Running) {
        setState(Running);
        emit outputReceived(m_appId, QString("[システム] 起動完了 (PID: %1)\n").arg(currentPid()), false);
    }
}
