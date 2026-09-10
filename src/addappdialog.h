#ifndef ADDAPPDIALOG_H
#define ADDAPPDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QToolButton>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QFormLayout>
#include "appitem.h"

class AddAppDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddAppDialog(QWidget *parent = nullptr);
    explicit AddAppDialog(const AppItem& item, QWidget *parent = nullptr);

    AppItem getAppItem() const;

private slots:
    void browseScript();
    void browseInterpreter();
    void browseWorkingDir();
    void browseIcon();
    void autoDetectInterpreter();
    void validateAndAccept();

private:
    void initUI();
    void retranslateUi();
    void populateFromItem(const AppItem& item);

    QLabel *m_lblHeader;
    QLabel *m_lblName;
    QLabel *m_lblDescription;
    QLabel *m_lblScriptPath;
    QLabel *m_lblInterpreterPath;
    QLabel *m_lblWorkingDir;
    QLabel *m_lblArguments;
    QLabel *m_lblIconPath;
    QLabel *m_lblMode;

    QLineEdit *m_editName;
    QLineEdit *m_editDescription;
    QLineEdit *m_editScriptPath;
    QLineEdit *m_editInterpreterPath;
    QLineEdit *m_editWorkingDir;
    QLineEdit *m_editArguments;
    QLineEdit *m_editIconPath;
    QCheckBox *m_checkKeepAlive;

    QToolButton *m_btnBrowseScript;
    QToolButton *m_btnBrowseInterp;
    QToolButton *m_btnBrowseWork;
    QToolButton *m_btnBrowseIcon;
    QPushButton *m_btnAutoDetect;

    QPushButton *m_btnCancel;
    QPushButton *m_btnOk;

    AppItem m_currentItem;
    bool m_isEditMode;
};

#endif // ADDAPPDIALOG_H
