#include "logviewer.h"
#include "i18n.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QScrollBar>

LogViewer::LogViewer(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 6, 8, 6);
    mainLayout->setSpacing(5);

    // Control bar
    QHBoxLayout *controlLayout = new QHBoxLayout();
    m_lblTitle = new QLabel(this);
    QFont font = m_lblTitle->font();
    font.setBold(true);
    m_lblTitle->setFont(font);

    m_comboApps = new QComboBox(this);
    m_comboApps->addItem("", "ALL");
    connect(m_comboApps, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogViewer::onAppSelectorChanged);

    m_lblTarget = new QLabel(this);
    m_chkAutoScroll = new QCheckBox(this);
    m_chkAutoScroll->setChecked(true);

    m_btnClear = new QPushButton(this);
    m_btnSave = new QPushButton(this);

    connect(m_btnClear, &QPushButton::clicked, this, &LogViewer::clearCurrentLog);
    connect(m_btnSave, &QPushButton::clicked, this, &LogViewer::saveLogToFile);

    controlLayout->addWidget(m_lblTitle);
    controlLayout->addSpacing(15);
    controlLayout->addWidget(m_lblTarget);
    controlLayout->addWidget(m_comboApps, 1);
    controlLayout->addWidget(m_chkAutoScroll);
    controlLayout->addWidget(m_btnClear);
    controlLayout->addWidget(m_btnSave);

    mainLayout->addLayout(controlLayout);

    // Console text edit
    m_textConsole = new QTextEdit(this);
    m_textConsole->setReadOnly(true);
    m_textConsole->setStyleSheet(
        "QTextEdit {"
        "  background-color: #1e1e1e;"
        "  color: #d4d4d4;"
        "  font-family: 'Consolas', 'DejaVu Sans Mono', 'Monospace', monospace;"
        "  font-size: 9pt;"
        "  border: 1px solid #3c3c3c;"
        "  border-radius: 4px;"
        "  padding: 4px 6px;"
        "}"
    );

    mainLayout->addWidget(m_textConsole, 1);
    m_currentAppId = "ALL";

    retranslateUi();
}

void LogViewer::retranslateUi() {
    m_lblTitle->setText(TR("log_title"));
    m_lblTarget->setText(TR("log_target"));
    m_chkAutoScroll->setText(TR("log_autoscroll"));
    m_btnClear->setText(TR("log_clear"));
    m_btnSave->setText(TR("log_save"));

    int allIdx = m_comboApps->findData("ALL");
    if (allIdx >= 0) {
        m_comboApps->setItemText(allIdx, TR("log_all"));
    }
}

void LogViewer::registerApp(const QString& appId, const QString& appName) {
    if (!m_logs.contains(appId)) {
        m_logs[appId] = { appName, "" };
        m_comboApps->addItem(QString("📱 %1").arg(appName), appId);
    } else {
        m_logs[appId].appName = appName;
        int idx = m_comboApps->findData(appId);
        if (idx >= 0) {
            m_comboApps->setItemText(idx, QString("📱 %1").arg(appName));
        }
    }
}

void LogViewer::unregisterApp(const QString& appId) {
    m_logs.remove(appId);
    int idx = m_comboApps->findData(appId);
    if (idx >= 0) {
        m_comboApps->removeItem(idx);
    }
}

void LogViewer::selectAppTab(const QString& appId) {
    int idx = m_comboApps->findData(appId);
    if (idx >= 0) {
        m_comboApps->setCurrentIndex(idx);
    }
}

void LogViewer::appendLog(const QString& appId, const QString& appName, const QString& text, bool isError) {
    registerApp(appId, appName);

    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString color = isError ? "#f92672" : (text.startsWith("[システム]") || text.startsWith("[System]") ? "#66d9ef" : "#a6e22e");

    QString escaped = escapeHtml(text);
    escaped.replace("\n", "<br>");

    QString lineHtml = QString("<span style='color: #75715e;'>[%1]</span> <span style='color: %2;'>%3</span>")
                           .arg(timeStr, color, escaped);

    // Append to specific app buffer
    m_logs[appId].htmlContent.append(lineHtml);

    // Append to ALL buffer
    if (!m_logs.contains("ALL_GLOBAL")) {
        m_logs["ALL_GLOBAL"] = { "ALL", "" };
    }
    m_logs["ALL_GLOBAL"].htmlContent.append(
        QString("<span style='color: #fd971f;'>[%1]</span> %2").arg(escapeHtml(appName), lineHtml)
    );

    // If currently displaying this app or ALL
    if (m_currentAppId == appId || m_currentAppId == "ALL") {
        updateConsoleDisplay();
    }
}

void LogViewer::onAppSelectorChanged(int index) {
    if (index < 0) return;
    m_currentAppId = m_comboApps->itemData(index).toString();
    updateConsoleDisplay();
}

void LogViewer::updateConsoleDisplay() {
    if (m_currentAppId == "ALL") {
        m_textConsole->setHtml(m_logs.value("ALL_GLOBAL").htmlContent);
    } else {
        m_textConsole->setHtml(m_logs.value(m_currentAppId).htmlContent);
    }

    if (m_chkAutoScroll->isChecked()) {
        QScrollBar *sb = m_textConsole->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
}

void LogViewer::clearCurrentLog() {
    if (m_currentAppId == "ALL") {
        for (auto it = m_logs.begin(); it != m_logs.end(); ++it) {
            it.value().htmlContent.clear();
        }
    } else {
        m_logs[m_currentAppId].htmlContent.clear();
    }
    m_textConsole->clear();
}

void LogViewer::saveLogToFile() {
    QString fileName = QFileDialog::getSaveFileName(this, TR("log_save_dialog_title"), "python_app.log", TR("log_save_filter"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, TR("log_save_error_title"), TR("log_save_error_text"));
        return;
    }

    QTextStream out(&file);
    out << m_textConsole->toPlainText();
    file.close();
    QMessageBox::information(this, TR("log_save_success_title"), TR_ARGS("log_save_success_text", {fileName}));
}

QString LogViewer::escapeHtml(const QString& plain) {
    QString escaped = plain;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    return escaped;
}
