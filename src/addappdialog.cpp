#include "addappdialog.h"
#include "i18n.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

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

AddAppDialog::AddAppDialog(QWidget *parent)
    : QDialog(parent), m_isEditMode(false) {
    initUI();
}

AddAppDialog::AddAppDialog(const AppItem& item, QWidget *parent)
    : QDialog(parent), m_currentItem(item), m_isEditMode(true) {
    initUI();
    populateFromItem(item);
}

void AddAppDialog::initUI() {
    setMinimumWidth(560);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    m_lblHeader = new QLabel(this);
    QFont headerFont = m_lblHeader->font();
    headerFont.setPointSize(12);
    headerFont.setBold(true);
    m_lblHeader->setFont(headerFont);
    mainLayout->addWidget(m_lblHeader);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(12);

    m_lblName = new QLabel(this);
    m_editName = new QLineEdit(this);
    formLayout->addRow(m_lblName, m_editName);

    m_lblDescription = new QLabel(this);
    m_editDescription = new QLineEdit(this);
    formLayout->addRow(m_lblDescription, m_editDescription);

    // Script path picker
    m_lblScriptPath = new QLabel(this);
    QHBoxLayout *scriptLayout = new QHBoxLayout();
    m_editScriptPath = new QLineEdit(this);
    m_btnBrowseScript = new QToolButton(this);
    connect(m_btnBrowseScript, &QToolButton::clicked, this, &AddAppDialog::browseScript);
    scriptLayout->addWidget(m_editScriptPath);
    scriptLayout->addWidget(m_btnBrowseScript);
    formLayout->addRow(m_lblScriptPath, scriptLayout);

    // Interpreter path picker
    m_lblInterpreterPath = new QLabel(this);
    QHBoxLayout *interpLayout = new QHBoxLayout();
    m_editInterpreterPath = new QLineEdit(this);
    m_btnBrowseInterp = new QToolButton(this);
    connect(m_btnBrowseInterp, &QToolButton::clicked, this, &AddAppDialog::browseInterpreter);
    m_btnAutoDetect = new QPushButton(this);
    connect(m_btnAutoDetect, &QPushButton::clicked, this, &AddAppDialog::autoDetectInterpreter);
    interpLayout->addWidget(m_editInterpreterPath);
    interpLayout->addWidget(m_btnBrowseInterp);
    interpLayout->addWidget(m_btnAutoDetect);
    formLayout->addRow(m_lblInterpreterPath, interpLayout);

    // Working directory
    m_lblWorkingDir = new QLabel(this);
    QHBoxLayout *workLayout = new QHBoxLayout();
    m_editWorkingDir = new QLineEdit(this);
    m_btnBrowseWork = new QToolButton(this);
    connect(m_btnBrowseWork, &QToolButton::clicked, this, &AddAppDialog::browseWorkingDir);
    workLayout->addWidget(m_editWorkingDir);
    workLayout->addWidget(m_btnBrowseWork);
    formLayout->addRow(m_lblWorkingDir, workLayout);

    // Arguments
    m_lblArguments = new QLabel(this);
    m_editArguments = new QLineEdit(this);
    formLayout->addRow(m_lblArguments, m_editArguments);

    // Icon path
    m_lblIconPath = new QLabel(this);
    QHBoxLayout *iconLayout = new QHBoxLayout();
    m_editIconPath = new QLineEdit(this);
    m_btnBrowseIcon = new QToolButton(this);
    connect(m_btnBrowseIcon, &QToolButton::clicked, this, &AddAppDialog::browseIcon);
    iconLayout->addWidget(m_editIconPath);
    iconLayout->addWidget(m_btnBrowseIcon);
    formLayout->addRow(m_lblIconPath, iconLayout);

    m_lblMode = new QLabel(this);
    m_checkKeepAlive = new QCheckBox(this);
    m_checkKeepAlive->setChecked(true);
    formLayout->addRow(m_lblMode, m_checkKeepAlive);

    mainLayout->addLayout(formLayout);

    // Dialog buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_btnCancel = new QPushButton(this);
    m_btnOk = new QPushButton(this);
    m_btnOk->setDefault(true);
    m_btnOk->setStyleSheet("background-color: #27ae60; color: white; font-weight: bold; padding: 6px 16px; border-radius: 4px;");

    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_btnOk, &QPushButton::clicked, this, &AddAppDialog::validateAndAccept);

    btnLayout->addWidget(m_btnCancel);
    btnLayout->addWidget(m_btnOk);
    mainLayout->addLayout(btnLayout);

    retranslateUi();
}

void AddAppDialog::retranslateUi() {
    setWindowTitle(m_isEditMode ? TR("dlg_title_edit") : TR("dlg_title_add"));
    m_lblHeader->setText(m_isEditMode ? TR("dlg_header_edit") : TR("dlg_header_add"));

    m_lblName->setText(TR("dlg_lbl_name"));
    m_editName->setPlaceholderText(TR("dlg_ph_name"));

    m_lblDescription->setText(TR("dlg_lbl_desc"));
    m_editDescription->setPlaceholderText(TR("dlg_ph_desc"));

    m_lblScriptPath->setText(TR("dlg_lbl_script"));
    m_editScriptPath->setPlaceholderText(TR("dlg_ph_script"));
    m_btnBrowseScript->setText(TR("dlg_btn_browse"));

    m_lblInterpreterPath->setText(TR("dlg_lbl_interp"));
    m_editInterpreterPath->setPlaceholderText(TR("dlg_ph_interp"));
    m_btnBrowseInterp->setText(TR("dlg_btn_browse"));
    m_btnAutoDetect->setText(TR("dlg_btn_autodetect"));

    m_lblWorkingDir->setText(TR("dlg_lbl_workdir"));
    m_editWorkingDir->setPlaceholderText(TR("dlg_ph_workdir"));
    m_btnBrowseWork->setText(TR("dlg_btn_browse"));

    m_lblArguments->setText(TR("dlg_lbl_args"));
    m_editArguments->setPlaceholderText(TR("dlg_ph_args"));

    m_lblIconPath->setText(TR("dlg_lbl_icon"));
    m_editIconPath->setPlaceholderText(TR("dlg_ph_icon"));
    m_btnBrowseIcon->setText(TR("dlg_btn_browse"));

    m_lblMode->setText(TR("dlg_lbl_mode"));
    m_checkKeepAlive->setText(TR("dlg_chk_keepalive"));

    m_btnCancel->setText(TR("dlg_btn_cancel"));
    m_btnOk->setText(m_isEditMode ? TR("dlg_btn_save") : TR("dlg_btn_register"));
}

void AddAppDialog::populateFromItem(const AppItem& item) {
    m_editName->setText(item.name);
    m_editDescription->setText(item.description);
    m_editScriptPath->setText(item.scriptPath);
    m_editInterpreterPath->setText(item.interpreterPath);
    m_editWorkingDir->setText(item.workingDir);
    m_editArguments->setText(item.arguments);
    m_editIconPath->setText(item.iconPath);
    m_checkKeepAlive->setChecked(item.keepAliveAfterExit);
}

void AddAppDialog::browseScript() {
    QString file = QFileDialog::getOpenFileName(this, TR("dlg_browse_script_title"), m_editScriptPath->text(), TR("dlg_browse_script_filter"));
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
    QString file = QFileDialog::getOpenFileName(this, TR("dlg_browse_interp_title"), m_editInterpreterPath->text(), TR("dlg_browse_interp_filter"));
    if (!file.isEmpty()) {
        m_editInterpreterPath->setText(file);
    }
}

void AddAppDialog::browseWorkingDir() {
    QString dir = QFileDialog::getExistingDirectory(this, TR("dlg_browse_workdir_title"), m_editWorkingDir->text());
    if (!dir.isEmpty()) {
        m_editWorkingDir->setText(dir);
    }
}

void AddAppDialog::browseIcon() {
    QString file = QFileDialog::getOpenFileName(this, TR("dlg_browse_icon_title"), m_editIconPath->text(), TR("dlg_browse_icon_filter"));
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
        QMessageBox::warning(this, TR("dlg_val_error_title"), TR("dlg_val_name_empty"));
        m_editName->setFocus();
        return;
    }
    if (m_editScriptPath->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, TR("dlg_val_error_title"), TR("dlg_val_script_empty"));
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
    m_currentItem.keepAliveAfterExit = m_checkKeepAlive->isChecked();

    accept();
}

AppItem AddAppDialog::getAppItem() const {
    return m_currentItem;
}
