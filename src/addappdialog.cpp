#include "addappdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

static QString suggestAppName(const QString& scriptPath) {
    QFileInfo info(scriptPath);
    QString baseName = info.completeBaseName();
    QString baseLower = baseName.toLower();

    // "main.py" や "app.py" など汎用的な名前の場合は、親フォルダ名をアプリ名として推測する
    if (baseLower == "main" || baseLower == "app" || baseLower == "run" ||
        baseLower == "index" || baseLower == "__main__" || baseLower == "script") {
        QString dirName = info.dir().dirName();
        if (!dirName.isEmpty() && dirName != "/") {
            return dirName;
        }
    }
    return baseName;
}

AddAppDialog::AddAppDialog(QWidget *parent)
    : QDialog(parent), m_isEditMode(false) {
    initUI();
    setWindowTitle("Pythonアプリの新規登録");
}

AddAppDialog::AddAppDialog(const AppItem& item, QWidget *parent)
    : QDialog(parent), m_currentItem(item), m_isEditMode(true) {
    initUI();
    setWindowTitle("Pythonアプリ設定の編集");
    populateFromItem(item);
}

void AddAppDialog::initUI() {
    setMinimumWidth(550);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *headerLabel = new QLabel(m_isEditMode ? "Pythonアプリの登録情報を編集します" : "新しいPythonアプリを登録します", this);
    QFont headerFont = headerLabel->font();
    headerFont.setPointSize(12);
    headerFont.setBold(true);
    headerLabel->setFont(headerFont);
    mainLayout->addWidget(headerLabel);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(12);

    m_editName = new QLineEdit(this);
    m_editName->setPlaceholderText("例: 画像一括リサイズツール");
    formLayout->addRow("アプリ名 (*):", m_editName);

    m_editDescription = new QLineEdit(this);
    m_editDescription->setPlaceholderText("例: ドラッグした画像を全自動で変換するスクリプト");
    formLayout->addRow("説明:", m_editDescription);

    // Script path picker
    QHBoxLayout *scriptLayout = new QHBoxLayout();
    m_editScriptPath = new QLineEdit(this);
    m_editScriptPath->setPlaceholderText("例: /home/user/scripts/main.py");
    QToolButton *btnScript = new QToolButton(this);
    btnScript->setText("参照...");
    connect(btnScript, &QToolButton::clicked, this, &AddAppDialog::browseScript);
    scriptLayout->addWidget(m_editScriptPath);
    scriptLayout->addWidget(btnScript);
    formLayout->addRow("スクリプトパス (*):", scriptLayout);

    // Interpreter path picker
    QHBoxLayout *interpLayout = new QHBoxLayout();
    m_editInterpreterPath = new QLineEdit(this);
    m_editInterpreterPath->setPlaceholderText("例: /usr/bin/python3 または venv/bin/python");
    QToolButton *btnInterp = new QToolButton(this);
    btnInterp->setText("参照...");
    connect(btnInterp, &QToolButton::clicked, this, &AddAppDialog::browseInterpreter);
    m_btnAutoDetect = new QPushButton("自動検出", this);
    connect(m_btnAutoDetect, &QPushButton::clicked, this, &AddAppDialog::autoDetectInterpreter);
    interpLayout->addWidget(m_editInterpreterPath);
    interpLayout->addWidget(btnInterp);
    interpLayout->addWidget(m_btnAutoDetect);
    formLayout->addRow("Python環境 (Interp):", interpLayout);

    // Working directory
    QHBoxLayout *workLayout = new QHBoxLayout();
    m_editWorkingDir = new QLineEdit(this);
    m_editWorkingDir->setPlaceholderText("空欄の場合はスクリプトの場所");
    QToolButton *btnWork = new QToolButton(this);
    btnWork->setText("参照...");
    connect(btnWork, &QToolButton::clicked, this, &AddAppDialog::browseWorkingDir);
    workLayout->addWidget(m_editWorkingDir);
    workLayout->addWidget(btnWork);
    formLayout->addRow("作業ディレクトリ:", workLayout);

    // Arguments
    m_editArguments = new QLineEdit(this);
    m_editArguments->setPlaceholderText("例: --config settings.json --verbose");
    formLayout->addRow("実行引数 (任意):", m_editArguments);

    // Icon path
    QHBoxLayout *iconLayout = new QHBoxLayout();
    m_editIconPath = new QLineEdit(this);
    m_editIconPath->setPlaceholderText("任意 (.png, .jpg, .ico, .svg)");
    QToolButton *btnIcon = new QToolButton(this);
    btnIcon->setText("参照...");
    connect(btnIcon, &QToolButton::clicked, this, &AddAppDialog::browseIcon);
    iconLayout->addWidget(m_editIconPath);
    iconLayout->addWidget(btnIcon);
    formLayout->addRow("アプリアイコン:", iconLayout);

    mainLayout->addLayout(formLayout);

    // Dialog buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton *btnCancel = new QPushButton("キャンセル", this);
    QPushButton *btnOk = new QPushButton(m_isEditMode ? "保存" : "登録", this);
    btnOk->setDefault(true);
    btnOk->setStyleSheet("background-color: #27ae60; color: white; font-weight: bold; padding: 6px 16px; border-radius: 4px;");

    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnOk, &QPushButton::clicked, this, &AddAppDialog::validateAndAccept);

    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnOk);
    mainLayout->addLayout(btnLayout);
}

void AddAppDialog::populateFromItem(const AppItem& item) {
    m_editName->setText(item.name);
    m_editDescription->setText(item.description);
    m_editScriptPath->setText(item.scriptPath);
    m_editInterpreterPath->setText(item.interpreterPath);
    m_editWorkingDir->setText(item.workingDir);
    m_editArguments->setText(item.arguments);
    m_editIconPath->setText(item.iconPath);
}

void AddAppDialog::browseScript() {
    QString file = QFileDialog::getOpenFileName(this, "Pythonスクリプトを選択", m_editScriptPath->text(), "Python Files (*.py *.pyw);;All Files (*)");
    if (!file.isEmpty()) {
        m_editScriptPath->setText(file);
        if (m_editName->text().trimmed().isEmpty()) {
            m_editName->setText(suggestAppName(file));
        }
        if (m_editWorkingDir->text().trimmed().isEmpty()) {
            QFileInfo info(file);
            m_editWorkingDir->setText(info.absolutePath());
        }
        if (m_editInterpreterPath->text().trimmed().isEmpty()) {
            autoDetectInterpreter();
        }
    }
}

void AddAppDialog::browseInterpreter() {
    QString file = QFileDialog::getOpenFileName(this, "Python実行ファイルを選択", m_editInterpreterPath->text(), "Executable (python python3);;All Files (*)");
    if (!file.isEmpty()) {
        m_editInterpreterPath->setText(file);
    }
}

void AddAppDialog::browseWorkingDir() {
    QString dir = QFileDialog::getExistingDirectory(this, "作業ディレクトリを選択", m_editWorkingDir->text());
    if (!dir.isEmpty()) {
        m_editWorkingDir->setText(dir);
    }
}

void AddAppDialog::browseIcon() {
    QString file = QFileDialog::getOpenFileName(this, "アイコン画像を選択", m_editIconPath->text(), "Images (*.png *.jpg *.jpeg *.ico *.svg);;All Files (*)");
    if (!file.isEmpty()) {
        m_editIconPath->setText(file);
    }
}

void AddAppDialog::autoDetectInterpreter() {
    QString detected = AppItem::autoDetectInterpreter(m_editScriptPath->text());
    m_editInterpreterPath->setText(detected);
}

void AddAppDialog::validateAndAccept() {
    if (m_editName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "入力エラー", "アプリ名を入力してください。");
        m_editName->setFocus();
        return;
    }
    if (m_editScriptPath->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "入力エラー", "Pythonスクリプトのパスを指定してください。");
        m_editScriptPath->setFocus();
        return;
    }

    if (m_editInterpreterPath->text().trimmed().isEmpty()) {
        autoDetectInterpreter();
    }

    m_currentItem.name = m_editName->text().trimmed();
    m_currentItem.description = m_editDescription->text().trimmed();
    m_currentItem.scriptPath = m_editScriptPath->text().trimmed();
    m_currentItem.interpreterPath = m_editInterpreterPath->text().trimmed();
    m_currentItem.workingDir = m_editWorkingDir->text().trimmed();
    m_currentItem.arguments = m_editArguments->text().trimmed();
    m_currentItem.iconPath = m_editIconPath->text().trimmed();

    accept();
}

AppItem AddAppDialog::getAppItem() const {
    return m_currentItem;
}
