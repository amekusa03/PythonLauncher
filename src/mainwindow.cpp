#include "mainwindow.h"
#include "addappdialog.h"
#include "stylehelper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QMimeData>
#include <QMessageBox>
#include <QFileInfo>
#include <QApplication>
#include <QDebug>

static QString suggestAppName(const QString& scriptPath) {
    QFileInfo info(scriptPath);
    QString baseName = info.completeBaseName();
    QString baseLower = baseName.toLower();

    if (baseLower == "main" || baseLower == "app" || baseLower == "run" ||
        baseLower == "index" || baseLower == "__main__" || baseLower == "script") {
        QString dirName = info.dir().dirName();
        if (!dirName.isEmpty() && dirName != "/") {
            return dirName;
        }
    }
    return baseName;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    initUI();
    loadAppsConfig();
    renderAppCards();
    setAcceptDrops(true);
}

MainWindow::~MainWindow() {
    saveAppsConfig();
    for (auto runner : m_runners) {
        runner->stop();
    }
}

void MainWindow::initUI() {
    setWindowTitle("Python App Launcher");
    resize(1000, 650);
    setStyleSheet(StyleHelper::getAppStyleSheet());

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    // Top Header
    QHBoxLayout *headerLayout = new QHBoxLayout();

    QLabel *lblIcon = new QLabel("🐍", this);
    QFont iconFont = lblIcon->font();
    iconFont.setPointSize(24);
    lblIcon->setFont(iconFont);

    QVBoxLayout *titleLayout = new QVBoxLayout();
    QLabel *lblTitle = new QLabel("Python アプリ ランチャー", this);
    QFont titleFont = lblTitle->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    lblTitle->setFont(titleFont);
    lblTitle->setStyleSheet("color: #2c3e50;");

    QLabel *lblSubtitle = new QLabel("コマンドライン入力不要・ワンクリックでPythonスクリプトを安全起動", this);
    lblSubtitle->setStyleSheet("color: #7f8c8d; font-size: 11px;");

    titleLayout->addWidget(lblTitle);
    titleLayout->addWidget(lblSubtitle);

    headerLayout->addWidget(lblIcon);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    // Search filter
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍 アプリ名で検索...");
    m_searchEdit->setFixedWidth(200);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onFilterTextChanged);
    headerLayout->addWidget(m_searchEdit);

    // Add App Buttons
    QPushButton *btnAddSample = new QPushButton("サンプルアプリ登録", this);
    connect(btnAddSample, &QPushButton::clicked, this, &MainWindow::onAddSamplesClicked);

    QPushButton *btnAddApp = new QPushButton("＋ 新規アプリ追加", this);
    btnAddApp->setObjectName("btnPrimary");
    connect(btnAddApp, &QPushButton::clicked, this, &MainWindow::onAddAppClicked);

    headerLayout->addWidget(btnAddSample);
    headerLayout->addWidget(btnAddApp);

    mainLayout->addLayout(headerLayout);

    // Splitter: Cards on Top/Left, LogViewer on Bottom/Right
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);

    // App List Container
    QWidget *cardsContainer = new QWidget(this);
    QVBoxLayout *containerLayout = new QVBoxLayout(cardsContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    
    QWidget *scrollContent = new QWidget(this);
    m_cardsLayout = new QVBoxLayout(scrollContent);
    m_cardsLayout->setContentsMargins(5, 5, 5, 5);
    m_cardsLayout->setSpacing(10);
    m_cardsLayout->addStretch();

    m_scrollArea->setWidget(scrollContent);
    containerLayout->addWidget(m_scrollArea);

    // Log viewer
    m_logViewer = new LogViewer(this);

    splitter->addWidget(cardsContainer);
    splitter->addWidget(m_logViewer);

    // Set initial splitter proportions (60% cards, 40% log)
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(splitter, 1);

    // Drag & Drop notice bar
    QLabel *lblNotice = new QLabel("💡 ヒント: .py ファイルをこのウィンドウ内にドラッグ＆ドロップして簡単に登録できます。", this);
    lblNotice->setStyleSheet("color: #606266; font-size: 11px; padding: 4px; border-top: 1px solid #e4e7ed;");
    mainLayout->addWidget(lblNotice);
}

QString MainWindow::getConfigFilePath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.filePath("apps.json");
}

void MainWindow::loadAppsConfig() {
    QString path = getConfigFilePath();
    QFile file(path);
    if (!file.exists()) {
        // Load default samples on first start
        onAddSamplesClicked();
        return;
    }

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray()) {
            QJsonArray arr = doc.array();
            m_appList.clear();
            for (const QJsonValue& val : arr) {
                if (val.isObject()) {
                    AppItem item = AppItem::fromJson(val.toObject());
                    m_appList.append(item);
                }
            }
        }
    }
}

void MainWindow::saveAppsConfig() {
    QJsonArray arr;
    for (const AppItem& item : m_appList) {
        arr.append(item.toJson());
    }
    QJsonDocument doc(arr);

    QFile file(getConfigFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void MainWindow::renderAppCards() {
    // Clear existing layout items
    QLayoutItem *child;
    while ((child = m_cardsLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
    m_cardWidgets.clear();

    QString filter = m_searchEdit ? m_searchEdit->text().trimmed().toLower() : "";

    for (const AppItem& item : m_appList) {
        if (!filter.isEmpty()) {
            if (!item.name.toLower().contains(filter) && !item.description.toLower().contains(filter) && !item.scriptPath.toLower().contains(filter)) {
                continue;
            }
        }
        QWidget *card = createCardWidget(item);
        m_cardsLayout->addWidget(card);
        m_cardWidgets[item.id] = card;

        m_logViewer->registerApp(item.id, item.name);
    }

    m_cardsLayout->addStretch();
}

QWidget* MainWindow::createCardWidget(const AppItem& item) {
    QFrame *card = new QFrame(this);
    card->setObjectName("AppCard");

    bool isRunning = m_runners.contains(item.id) && m_runners[item.id]->currentState() == ProcessRunner::Running;
    card->setStyleSheet(StyleHelper::getCardStyleSheet(isRunning));

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(15, 12, 15, 12);
    cardLayout->setSpacing(15);

    // App Icon / Badge
    QLabel *lblIcon = new QLabel(card);
    lblIcon->setFixedSize(48, 48);
    lblIcon->setAlignment(Qt::AlignCenter);

    if (!item.iconPath.isEmpty() && QFile::exists(item.iconPath)) {
        QPixmap pix(item.iconPath);
        lblIcon->setPixmap(pix.scaled(44, 44, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        lblIcon->setText("🐍");
        QFont iconFont = lblIcon->font();
        iconFont.setPointSize(22);
        lblIcon->setFont(iconFont);
        lblIcon->setStyleSheet("background-color: #eef5fe; border-radius: 8px;");
    }
    cardLayout->addWidget(lblIcon);

    // App Text details
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);

    QHBoxLayout *titleLine = new QHBoxLayout();
    QLabel *lblName = new QLabel(item.name, card);
    QFont fontName = lblName->font();
    fontName.setPointSize(12);
    fontName.setBold(true);
    lblName->setFont(fontName);
    titleLine->addWidget(lblName);

    // Interpreter badge
    QString interpName = QFileInfo(item.interpreterPath).fileName();
    if (interpName.isEmpty()) interpName = "python3";
    QLabel *lblInterpBadge = new QLabel(interpName, card);
    lblInterpBadge->setStyleSheet("background-color: #e4e7ed; color: #409eff; font-size: 10px; font-weight: bold; border-radius: 3px; padding: 2px 6px;");
    titleLine->addWidget(lblInterpBadge);

    // Status Badge
    QLabel *lblStatus = new QLabel(card);
    lblStatus->setObjectName(QString("status_%1").arg(item.id));
    if (isRunning) {
        qint64 pid = m_runners[item.id]->currentPid();
        lblStatus->setText(QString("🟢 実行中 (PID: %1)").arg(pid));
        lblStatus->setStyleSheet("color: #67c23a; font-size: 11px; font-weight: bold;");
    } else {
        lblStatus->setText("⚪ 停止中");
        lblStatus->setStyleSheet("color: #909399; font-size: 11px;");
    }
    titleLine->addWidget(lblStatus);

    titleLine->addStretch();
    infoLayout->addLayout(titleLine);

    // Description & Script Path
    QLabel *lblDesc = new QLabel(item.description.isEmpty() ? "（説明なし）" : item.description, card);
    lblDesc->setStyleSheet("color: #606266; font-size: 11px;");
    infoLayout->addWidget(lblDesc);

    QLabel *lblPath = new QLabel(QString("📄 %1").arg(item.scriptPath), card);
    lblPath->setStyleSheet("color: #909399; font-size: 10px;");
    infoLayout->addWidget(lblPath);

    cardLayout->addLayout(infoLayout, 1);

    // Control buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(6);

    QPushButton *btnRun = new QPushButton("▶ 起動", card);
    btnRun->setObjectName("btnRun");
    btnRun->setEnabled(!isRunning);

    QPushButton *btnStop = new QPushButton("■ 停止", card);
    btnStop->setObjectName("btnStop");
    btnStop->setEnabled(isRunning);

    QPushButton *btnLog = new QPushButton("📄 ログ", card);
    QPushButton *btnEdit = new QPushButton("⚙", card);
    btnEdit->setToolTip("編集");
    QPushButton *btnDelete = new QPushButton("🗑", card);
    btnDelete->setToolTip("削除");

    QString appId = item.id;
    connect(btnRun, &QPushButton::clicked, this, [this, appId]() { onRunApp(appId); });
    connect(btnStop, &QPushButton::clicked, this, [this, appId]() { onStopApp(appId); });
    connect(btnLog, &QPushButton::clicked, this, [this, appId]() { onShowLog(appId); });
    connect(btnEdit, &QPushButton::clicked, this, [this, appId]() { onEditApp(appId); });
    connect(btnDelete, &QPushButton::clicked, this, [this, appId]() { onDeleteApp(appId); });

    btnLayout->addWidget(btnRun);
    btnLayout->addWidget(btnStop);
    btnLayout->addWidget(btnLog);
    btnLayout->addWidget(btnEdit);
    btnLayout->addWidget(btnDelete);

    cardLayout->addLayout(btnLayout);

    return card;
}

void MainWindow::onAddAppClicked() {
    AddAppDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        AppItem newItem = dlg.getAppItem();
        m_appList.append(newItem);
        saveAppsConfig();
        renderAppCards();
    }
}

void MainWindow::onAddSamplesClicked() {
    QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);

    QString candidate1 = dir.absoluteFilePath("../sample_apps/demo_gui.py");
    QString candidate2 = dir.absoluteFilePath("sample_apps/demo_gui.py");
    QString candidate3 = QDir::current().absoluteFilePath("sample_apps/demo_gui.py");

    QString sampleGui = candidate1;
    if (!QFile::exists(sampleGui) && QFile::exists(candidate2)) sampleGui = candidate2;
    if (!QFile::exists(sampleGui) && QFile::exists(candidate3)) sampleGui = candidate3;

    QString cli1 = dir.absoluteFilePath("../sample_apps/demo_cli_tool.py");
    QString cli2 = dir.absoluteFilePath("sample_apps/demo_cli_tool.py");
    QString cli3 = QDir::current().absoluteFilePath("sample_apps/demo_cli_tool.py");

    QString sampleCli = cli1;
    if (!QFile::exists(sampleCli) && QFile::exists(cli2)) sampleCli = cli2;
    if (!QFile::exists(sampleCli) && QFile::exists(cli3)) sampleCli = cli3;

    AppItem item1;
    item1.name = "サンプル GUI アプリ (Tkinter)";
    item1.description = "ワンクリックで動くTkinterメッセージボックス付きサンプル画面";
    item1.scriptPath = sampleGui;
    item1.interpreterPath = AppItem::autoDetectInterpreter(sampleGui);
    item1.workingDir = QFileInfo(sampleGui).absolutePath();

    AppItem item2;
    item2.name = "サンプル CLI ツール (リアルタイム出力)";
    item2.description = "ログ画面にリアルタイム進捗とエラーを出力する処理サンプル";
    item2.scriptPath = sampleCli;
    item2.interpreterPath = AppItem::autoDetectInterpreter(sampleCli);
    item2.workingDir = QFileInfo(sampleCli).absolutePath();

    m_appList.append(item1);
    m_appList.append(item2);

    saveAppsConfig();
    renderAppCards();

    QMessageBox::information(this, "サンプル登録", "動作テスト用のサンプルPythonアプリを2件追加しました！");
}

void MainWindow::onEditApp(const QString& appId) {
    int targetIdx = -1;
    for (int i = 0; i < m_appList.size(); ++i) {
        if (m_appList[i].id == appId) {
            targetIdx = i;
            break;
        }
    }
    if (targetIdx < 0) return;

    AddAppDialog dlg(m_appList[targetIdx], this);
    if (dlg.exec() == QDialog::Accepted) {
        m_appList[targetIdx] = dlg.getAppItem();
        saveAppsConfig();
        renderAppCards();
    }
}

void MainWindow::onDeleteApp(const QString& appId) {
    int targetIdx = -1;
    for (int i = 0; i < m_appList.size(); ++i) {
        if (m_appList[i].id == appId) {
            targetIdx = i;
            break;
        }
    }
    if (targetIdx < 0) return;

    auto reply = QMessageBox::question(this, "登録削除",
        QString("「%1」の登録を一覧から削除しますか？\n（スクリプトファイル自体は削除されません）").arg(m_appList[targetIdx].name),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_runners.contains(appId)) {
            m_runners[appId]->stop();
            m_runners[appId]->deleteLater();
            m_runners.remove(appId);
        }
        m_logViewer->unregisterApp(appId);
        m_appList.removeAt(targetIdx);
        saveAppsConfig();
        renderAppCards();
    }
}

void MainWindow::onRunApp(const QString& appId) {
    const AppItem *targetItem = nullptr;
    for (const AppItem& item : m_appList) {
        if (item.id == appId) {
            targetItem = &item;
            break;
        }
    }
    if (!targetItem) return;

    if (!m_runners.contains(appId)) {
        ProcessRunner *runner = new ProcessRunner(this);
        connect(runner, &ProcessRunner::stateChanged, this, &MainWindow::onProcessStateChanged);
        connect(runner, &ProcessRunner::outputReceived, this, &MainWindow::onProcessOutputReceived);
        m_runners[appId] = runner;
    }

    m_logViewer->selectAppTab(appId);
    m_runners[appId]->start(*targetItem);
}

void MainWindow::onStopApp(const QString& appId) {
    if (m_runners.contains(appId)) {
        m_runners[appId]->stop();
    }
}

void MainWindow::onShowLog(const QString& appId) {
    m_logViewer->selectAppTab(appId);
}

void MainWindow::onFilterTextChanged(const QString& text) {
    Q_UNUSED(text);
    renderAppCards();
}

void MainWindow::onProcessStateChanged(const QString& appId, ProcessRunner::State state, qint64 pid, int exitCode) {
    Q_UNUSED(exitCode);
    if (m_cardWidgets.contains(appId)) {
        QWidget *card = m_cardWidgets[appId];
        QLabel *lblStatus = card->findChild<QLabel*>(QString("status_%1").arg(appId));
        QPushButton *btnRun = card->findChild<QPushButton*>("btnRun");
        QPushButton *btnStop = card->findChild<QPushButton*>("btnStop");

        bool isRunning = (state == ProcessRunner::Running);
        card->setStyleSheet(StyleHelper::getCardStyleSheet(isRunning));

        if (lblStatus) {
            if (state == ProcessRunner::Running) {
                lblStatus->setText(QString("🟢 実行中 (PID: %1)").arg(pid));
                lblStatus->setStyleSheet("color: #67c23a; font-size: 11px; font-weight: bold;");
            } else if (state == ProcessRunner::Starting) {
                lblStatus->setText("🟡 起動処理中...");
                lblStatus->setStyleSheet("color: #e6a23c; font-size: 11px; font-weight: bold;");
            } else if (state == ProcessRunner::FailedToStart) {
                lblStatus->setText("🔴 起動失敗");
                lblStatus->setStyleSheet("color: #f56c6c; font-size: 11px; font-weight: bold;");
            } else {
                lblStatus->setText("⚪ 停止中");
                lblStatus->setStyleSheet("color: #909399; font-size: 11px;");
            }
        }

        if (btnRun) btnRun->setEnabled(!isRunning && state != ProcessRunner::Starting);
        if (btnStop) btnStop->setEnabled(isRunning || state == ProcessRunner::Starting);
    }
}

void MainWindow::onProcessOutputReceived(const QString& appId, const QString& text, bool isError) {
    QString appName = "PythonApp";
    for (const AppItem& item : m_appList) {
        if (item.id == appId) {
            appName = item.name;
            break;
        }
    }
    m_logViewer->appendLog(appId, appName, text, isError);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        for (const QUrl &url : event->mimeData()->urls()) {
            QString path = url.toLocalFile();
            if (path.endsWith(".py", Qt::CaseInsensitive) || path.endsWith(".pyw", Qt::CaseInsensitive)) {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        for (const QUrl &url : event->mimeData()->urls()) {
            QString path = url.toLocalFile();
            if (path.endsWith(".py", Qt::CaseInsensitive) || path.endsWith(".pyw", Qt::CaseInsensitive)) {
                AppItem item;
                QFileInfo info(path);
                item.name = suggestAppName(path);
                item.scriptPath = path;
                item.workingDir = info.absolutePath();
                item.interpreterPath = AppItem::autoDetectInterpreter(path);

                AddAppDialog dlg(item, this);
                if (dlg.exec() == QDialog::Accepted) {
                    m_appList.append(dlg.getAppItem());
                    saveAppsConfig();
                    renderAppCards();
                }
                break;
            }
        }
    }
}
