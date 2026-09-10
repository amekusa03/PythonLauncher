#include "mainwindow.h"
#include "addappdialog.h"
#include "stylehelper.h"
#include "i18n.h"

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
#include <QComboBox>
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
        runner->disconnect();
        if (!runner->isDetached()) {
            runner->stop();
        }
    }
}

void MainWindow::initUI() {
    resize(1020, 660);
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
    
    QHBoxLayout *titleLineLayout = new QHBoxLayout();
    titleLineLayout->setSpacing(10);

    m_lblTitle = new QLabel(this);
    QFont titleFont = m_lblTitle->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    m_lblTitle->setFont(titleFont);
    m_lblTitle->setStyleSheet("color: #2c3e50;");

    m_btnRegisterDesktop = new QPushButton(this);
    m_btnRegisterDesktop->setCursor(Qt::PointingHandCursor);
    m_btnRegisterDesktop->setStyleSheet(
        "QPushButton { background-color: #f0f2f5; color: #4a5568; border: 1px solid #dcdfe6; border-radius: 4px; padding: 2px 8px; font-size: 11px; font-weight: bold; }"
        "QPushButton:hover { background-color: #e2e8f0; color: #2d3748; border-color: #cbd5e0; }"
    );
    connect(m_btnRegisterDesktop, &QPushButton::clicked, this, &MainWindow::onRegisterDesktopClicked);

    titleLineLayout->addWidget(m_lblTitle);
    titleLineLayout->addWidget(m_btnRegisterDesktop);
    titleLineLayout->addStretch();

    m_lblSubtitle = new QLabel(this);
    m_lblSubtitle->setStyleSheet("color: #7f8c8d; font-size: 11px;");

    titleLayout->addLayout(titleLineLayout);
    titleLayout->addWidget(m_lblSubtitle);

    headerLayout->addWidget(lblIcon);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    // Language Selector
    m_comboLanguage = new QComboBox(this);
    m_comboLanguage->addItem("🇯🇵 日本語", static_cast<int>(I18n::Language::Japanese));
    m_comboLanguage->addItem("🇺🇸 English", static_cast<int>(I18n::Language::English));
    m_comboLanguage->setToolTip("Switch Language / 言語切り替え");
    m_comboLanguage->setStyleSheet(
        "QComboBox { background-color: #ffffff; border: 1px solid #dcdfe6; border-radius: 6px; padding: 4px 10px; font-size: 12px; color: #2c3e50; font-weight: bold; }"
        "QComboBox:hover { border-color: #409eff; }"
    );

    // Set initial index based on current language
    if (I18n::instance().currentLanguage() == I18n::Language::English) {
        m_comboLanguage->setCurrentIndex(1);
    } else {
        m_comboLanguage->setCurrentIndex(0);
    }
    connect(m_comboLanguage, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLanguageComboChanged);
    headerLayout->addWidget(m_comboLanguage);

    // Search filter
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setFixedWidth(180);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onFilterTextChanged);
    headerLayout->addWidget(m_searchEdit);

    // Add App Buttons
    m_btnAddSample = new QPushButton(this);
    connect(m_btnAddSample, &QPushButton::clicked, this, &MainWindow::onAddSamplesClicked);

    m_btnAddApp = new QPushButton(this);
    m_btnAddApp->setObjectName("btnPrimary");
    connect(m_btnAddApp, &QPushButton::clicked, this, &MainWindow::onAddAppClicked);

    headerLayout->addWidget(m_btnAddSample);
    headerLayout->addWidget(m_btnAddApp);

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

    // Set initial splitter proportions (80% cards, 20% log)
    splitter->setStretchFactor(0, 5);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes(QList<int>() << 500 << 140);

    mainLayout->addWidget(splitter, 1);

    // Drag & Drop notice bar
    m_lblNotice = new QLabel(this);
    m_lblNotice->setStyleSheet("color: #606266; font-size: 11px; padding: 4px; border-top: 1px solid #e4e7ed;");
    mainLayout->addWidget(m_lblNotice);

    retranslateUi();
}

void MainWindow::onLanguageComboChanged(int index) {
    I18n::Language lang = static_cast<I18n::Language>(m_comboLanguage->itemData(index).toInt());
    I18n::instance().setLanguage(lang);
    retranslateUi();
}

void MainWindow::retranslateUi() {
    setWindowTitle(TR("app_window_title"));
    m_lblTitle->setText(TR("app_title"));
    m_lblSubtitle->setText(TR("app_subtitle"));
    m_searchEdit->setPlaceholderText(TR("search_placeholder"));
    m_btnAddSample->setText(TR("btn_add_sample"));
    m_btnAddApp->setText(TR("btn_add_app"));
    m_lblNotice->setText(TR("hint_drag_drop"));

    updateRegisterDesktopButton();
    m_logViewer->retranslateUi();
    renderAppCards();
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

    if (item.keepAliveAfterExit) {
        QLabel *lblDetachedBadge = new QLabel(TR("card_badge_detached"), card);
        lblDetachedBadge->setToolTip(TR("card_tip_detached"));
        lblDetachedBadge->setStyleSheet("background-color: #fdf6ec; color: #e6a23c; font-size: 10px; font-weight: bold; border-radius: 3px; padding: 2px 6px;");
        titleLine->addWidget(lblDetachedBadge);
    }

    // Status Badge
    QLabel *lblStatus = new QLabel(card);
    lblStatus->setObjectName(QString("status_%1").arg(item.id));
    if (isRunning) {
        qint64 pid = m_runners[item.id]->currentPid();
        lblStatus->setText(TR_ARGS("card_status_running", {QString::number(pid)}));
        lblStatus->setStyleSheet("color: #67c23a; font-size: 11px; font-weight: bold;");
    } else {
        lblStatus->setText(TR("card_status_stopped"));
        lblStatus->setStyleSheet("color: #909399; font-size: 11px;");
    }
    titleLine->addWidget(lblStatus);

    titleLine->addStretch();
    infoLayout->addLayout(titleLine);

    // Description & Script Path
    QLabel *lblDesc = new QLabel(item.description.isEmpty() ? TR("card_no_desc") : item.description, card);
    lblDesc->setStyleSheet("color: #606266; font-size: 11px;");
    infoLayout->addWidget(lblDesc);

    QLabel *lblPath = new QLabel(QString("📄 %1").arg(item.scriptPath), card);
    lblPath->setStyleSheet("color: #909399; font-size: 10px;");
    infoLayout->addWidget(lblPath);

    cardLayout->addLayout(infoLayout, 1);

    // Control buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(6);

    QPushButton *btnRun = new QPushButton(TR("card_btn_run"), card);
    btnRun->setObjectName("btnRun");
    btnRun->setEnabled(!isRunning);

    QPushButton *btnStop = new QPushButton(TR("card_btn_stop"), card);
    btnStop->setObjectName("btnStop");
    btnStop->setEnabled(isRunning);

    QPushButton *btnLog = new QPushButton(TR("card_btn_log"), card);
    QPushButton *btnEdit = new QPushButton("⚙", card);
    btnEdit->setToolTip(TR("card_tip_edit"));
    QPushButton *btnDelete = new QPushButton("🗑", card);
    btnDelete->setToolTip(TR("card_tip_delete"));

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
    item1.name = TR("sample_gui_name");
    item1.description = TR("sample_gui_desc");
    item1.scriptPath = sampleGui;
    item1.interpreterPath = AppItem::autoDetectInterpreter(sampleGui);
    item1.workingDir = QFileInfo(sampleGui).absolutePath();
    item1.keepAliveAfterExit = true;

    AppItem item2;
    item2.name = TR("sample_cli_name");
    item2.description = TR("sample_cli_desc");
    item2.scriptPath = sampleCli;
    item2.interpreterPath = AppItem::autoDetectInterpreter(sampleCli);
    item2.workingDir = QFileInfo(sampleCli).absolutePath();
    item2.keepAliveAfterExit = true;

    m_appList.append(item1);
    m_appList.append(item2);

    saveAppsConfig();
    renderAppCards();

    QMessageBox::information(this, TR("sample_added_title"), TR("sample_added_text"));
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

    auto reply = QMessageBox::question(this, TR("msg_delete_confirm_title"),
        TR_ARGS("msg_delete_confirm_text", {m_appList[targetIdx].name}),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_runners.contains(appId)) {
            if (!m_runners[appId]->isDetached()) {
                m_runners[appId]->stop();
            }
            m_runners[appId]->disconnect();
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
                lblStatus->setText(TR_ARGS("card_status_running", {QString::number(pid)}));
                lblStatus->setStyleSheet("color: #67c23a; font-size: 11px; font-weight: bold;");
            } else if (state == ProcessRunner::Starting) {
                lblStatus->setText(TR("card_status_starting"));
                lblStatus->setStyleSheet("color: #e6a23c; font-size: 11px; font-weight: bold;");
            } else if (state == ProcessRunner::FailedToStart) {
                lblStatus->setText(TR("card_status_failed"));
                lblStatus->setStyleSheet("color: #f56c6c; font-size: 11px; font-weight: bold;");
            } else {
                lblStatus->setText(TR("card_status_stopped"));
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
                item.keepAliveAfterExit = true;

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

void MainWindow::updateRegisterDesktopButton() {
    QString appDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    QString desktopFilePath = QDir(appDir).filePath("python-launcher.desktop");
    bool exists = (!appDir.isEmpty() && QFile::exists(desktopFilePath));

    if (exists) {
        m_btnRegisterDesktop->setText(TR("btn_desktop_unregister"));
        m_btnRegisterDesktop->setToolTip(TR("tip_desktop_unregister"));
    } else {
        m_btnRegisterDesktop->setText(TR("btn_desktop_register"));
        m_btnRegisterDesktop->setToolTip(TR("tip_desktop_register"));
    }
}

void MainWindow::onRegisterDesktopClicked() {
    QString appDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    if (appDir.isEmpty()) {
        QMessageBox::critical(this, TR("msg_desktop_error_title"), TR("msg_desktop_dir_error"));
        return;
    }

    QDir().mkpath(appDir);
    QString desktopFilePath = QDir(appDir).filePath("python-launcher.desktop");

    if (QFile::exists(desktopFilePath)) {
        auto reply = QMessageBox::question(this, TR("msg_desktop_reg_confirm_title"),
            TR("msg_desktop_reg_confirm_text"),
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            if (QFile::remove(desktopFilePath)) {
                QMessageBox::information(this, TR("msg_desktop_unreg_success_title"), TR("msg_desktop_unreg_success_text"));
            } else {
                QMessageBox::critical(this, TR("msg_desktop_error_title"), TR("msg_desktop_unreg_failed"));
            }
            updateRegisterDesktopButton();
        }
    } else {
        QString execPath = QCoreApplication::applicationFilePath();
        QFile file(desktopFilePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, TR("msg_desktop_error_title"), TR_ARGS("msg_desktop_create_error", {file.errorString()}));
            return;
        }

        QTextStream out(&file);
        out << "[Desktop Entry]\n";
        out << "Version=1.0\n";
        out << "Type=Application\n";
        out << "Name=Python Launcher\n";
        out << "GenericName=Python App Launcher\n";
        out << "Comment=One-click Python script execution without terminal\n";
        out << "Exec=\"" << execPath << "\" %f\n";
        out << "Icon=utilities-terminal\n";
        out << "Terminal=false\n";
        out << "Categories=Development;Utility;\n";
        file.close();

        file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                            QFileDevice::ReadGroup | QFileDevice::ExeGroup |
                            QFileDevice::ReadOther | QFileDevice::ExeOther);

        QMessageBox::information(this, TR("msg_desktop_reg_success_title"), TR("msg_desktop_reg_success_text"));
        updateRegisterDesktopButton();
    }
}
