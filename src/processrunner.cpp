#include "processrunner.h"
#include "i18n.h"
#include <QProcessEnvironment>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

ProcessRunner::ProcessRunner(QObject *parent)
    : QObject(parent), m_process(new QProcess(this)), m_state(NotRunning), m_isDetached(false), m_detachedPid(0) {

    connect(m_process, &QProcess::readyReadStandardOutput, this, &ProcessRunner::onReadyReadStdout);
    connect(m_process, &QProcess::readyReadStandardError, this, &ProcessRunner::onReadyReadStderr);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ProcessRunner::onFinished);
    connect(m_process, &QProcess::errorOccurred, this, &ProcessRunner::onErrorOccurred);
    connect(m_process, &QProcess::stateChanged, this, &ProcessRunner::onProcessStateChanged);
}

ProcessRunner::~ProcessRunner() {
    if (!m_isDetached && m_process && m_process->state() != QProcess::NotRunning) {
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
    m_isDetached = item.keepAliveAfterExit;
    m_detachedPid = 0;

    // Set working directory
    QString workDir = item.workingDir;
    if (workDir.isEmpty() && !item.scriptPath.isEmpty()) {
        workDir = QFileInfo(item.scriptPath).absolutePath();
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
        QStringList userArgs = QProcess::splitCommand(item.arguments);
        args.append(userArgs);
    }

    setState(Starting);

    if (m_isDetached) {
        QString baseName = QFileInfo(item.scriptPath).completeBaseName();
        if (baseName.isEmpty()) baseName = "python_app";
        QString logPath = (workDir.isEmpty() ? QDir::currentPath() : workDir) + "/" + baseName + ".log";

        emit outputReceived(m_appId, TR_ARGS("proc_detached_start", {program, args.join(" ")}), false);
        emit outputReceived(m_appId, TR_ARGS("proc_detached_log_dest", {logPath}), false);
        emit outputReceived(m_appId, TR("proc_detached_bg_notice"), false);

        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString displayEnv = env.value("DISPLAY", ":0");
        QString waylandEnv = env.value("WAYLAND_DISPLAY", "");
        QString xauthEnv = env.value("XAUTHORITY", "");

        QStringList formattedArgs;
        for (const QString& arg : args) {
            QString escaped = arg;
            escaped.replace("'", "'\\''");
            formattedArgs << QString("'%1'").arg(escaped);
        }

        QString shellCmd = QString("export DISPLAY=\"%1\"; export WAYLAND_DISPLAY=\"%2\"; export XAUTHORITY=\"%3\"; export PYTHONUNBUFFERED=1; exec setsid nohup '%4' %5 > '%6' 2>&1 < /dev/null")
                            .arg(displayEnv, waylandEnv, xauthEnv, program, formattedArgs.join(" "), logPath);

        qint64 pid = 0;
        bool ok = QProcess::startDetached("/bin/sh", QStringList() << "-c" << shellCmd, workDir, &pid);

        if (ok && pid > 0) {
            m_detachedPid = pid;
            setState(Running);
            emit outputReceived(m_appId, TR_ARGS("proc_detached_success", {QString::number(pid)}), false);
            return true;
        } else {
            setState(FailedToStart, -1);
            emit outputReceived(m_appId, TR("proc_detached_failed"), true);
            return false;
        }
    } else {
        // Configure environment for unbuffered Python output
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PYTHONUNBUFFERED", "1");
        m_process->setProcessEnvironment(env);

        if (!workDir.isEmpty()) {
            m_process->setWorkingDirectory(workDir);
        }

        emit outputReceived(m_appId, TR_ARGS("proc_normal_start", {program, args.join(" ")}), false);

        m_process->start(program, args);
        return true;
    }
}

void ProcessRunner::stop() {
    if (m_isDetached) {
        if (m_detachedPid > 0) {
            emit outputReceived(m_appId, TR_ARGS("proc_send_sigterm", {QString::number(m_detachedPid)}), false);
            QProcess::execute("kill", QStringList() << QString::number(m_detachedPid));
            m_detachedPid = 0;
            setState(Finished, 0);
        }
        return;
    }

    if (m_process && m_process->state() != QProcess::NotRunning) {
        emit outputReceived(m_appId, TR("proc_req_stop"), false);
        m_process->terminate();
    }
}

void ProcessRunner::killProcess() {
    if (m_isDetached) {
        if (m_detachedPid > 0) {
            emit outputReceived(m_appId, TR_ARGS("proc_send_sigkill", {QString::number(m_detachedPid)}), true);
            QProcess::execute("kill", QStringList() << "-9" << QString::number(m_detachedPid));
            m_detachedPid = 0;
            setState(Finished, -1);
        }
        return;
    }

    if (m_process && m_process->state() != QProcess::NotRunning) {
        emit outputReceived(m_appId, TR("proc_force_killed"), true);
        m_process->kill();
    }
}

qint64 ProcessRunner::currentPid() const {
    if (m_isDetached) {
        return m_detachedPid;
    }
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
    if (!m_isDetached) {
        setState(Finished, exitCode);
        emit outputReceived(m_appId, TR_ARGS("proc_finished", {QString::number(exitCode)}), false);
    }
}

void ProcessRunner::onErrorOccurred(QProcess::ProcessError error) {
    if (!m_isDetached && error == QProcess::FailedToStart) {
        setState(FailedToStart, -1);
        emit outputReceived(m_appId, TR("proc_error_start"), true);
    }
}

void ProcessRunner::onProcessStateChanged(QProcess::ProcessState newState) {
    if (!m_isDetached && newState == QProcess::Running) {
        setState(Running);
        emit outputReceived(m_appId, TR_ARGS("proc_running", {QString::number(currentPid())}), false);
    }
}
