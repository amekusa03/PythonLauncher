#include "logviewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QScrollBar>

LogViewer::LogViewer(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // Control bar
    QHBoxLayout *controlLayout = new QHBoxLayout();
    QLabel *lblTitle = new QLabel("📄 実行ログ モニター (ターミナル出力)", this);
    QFont font = lblTitle->font();
    font.setBold(true);
    lblTitle->setFont(font);

    m_comboApps = new QComboBox(this);
    m_comboApps->addItem("すべてのログ (全体)", "ALL");
    connect(m_comboApps, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogViewer::onAppSelectorChanged);

    m_chkAutoScroll = new QCheckBox("自動スクロール", this);
    m_chkAutoScroll->setChecked(true);

    m_btnClear = new QPushButton("ログ消去", this);
    m_btnSave = new QPushButton("ログ保存...", this);

    connect(m_btnClear, &QPushButton::clicked, this, &LogViewer::clearCurrentLog);
    connect(m_btnSave, &QPushButton::clicked, this, &LogViewer::saveLogToFile);

    controlLayout->addWidget(lblTitle);
    controlLayout->addSpacing(15);
    controlLayout->addWidget(new QLabel("表示対象:", this));
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
        "  font-size: 10pt;"
        "  border: 1px solid #3c3c3c;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "}"
    );

    mainLayout->addWidget(m_textConsole, 1);
    m_currentAppId = "ALL";
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
    QString color = isError ? "#f92672" : (text.startsWith("[システム]") ? "#66d9ef" : "#a6e22e");

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
    QString fileName = QFileDialog::getSaveFileName(this, "ログを保存", "python_app.log", "Log Files (*.log *.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "保存失敗", "ログファイルを書き込めませんでした。");
        return;
    }

    QTextStream out(&file);
    out << m_textConsole->toPlainText();
    file.close();
    QMessageBox::information(this, "保存完了", "ログを保存しました: " + fileName);
}

QString LogViewer::escapeHtml(const QString& plain) {
    QString escaped = plain;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    return escaped;
}
