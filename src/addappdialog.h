#ifndef ADDAPPDIALOG_H
#define ADDAPPDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QToolButton>
#include <QPushButton>
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
    void populateFromItem(const AppItem& item);

    QLineEdit *m_editName;
    QLineEdit *m_editDescription;
    QLineEdit *m_editScriptPath;
    QLineEdit *m_editInterpreterPath;
    QLineEdit *m_editWorkingDir;
    QLineEdit *m_editArguments;
    QLineEdit *m_editIconPath;

    QPushButton *m_btnAutoDetect;

    AppItem m_currentItem;
    bool m_isEditMode;
};

#endif // ADDAPPDIALOG_H
