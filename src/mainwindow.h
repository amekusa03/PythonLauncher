#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QMap>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLineEdit>
#include <QSplitter>
#include <QDragEnterEvent>
#include <QDropEvent>

#include "appitem.h"
#include "processrunner.h"
#include "logviewer.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onAddAppClicked();
    void onAddSamplesClicked();
    void onEditApp(const QString& appId);
    void onDeleteApp(const QString& appId);
    void onRunApp(const QString& appId);
    void onStopApp(const QString& appId);
    void onShowLog(const QString& appId);
    void onFilterTextChanged(const QString& text);

    void onProcessStateChanged(const QString& appId, ProcessRunner::State state, qint64 pid, int exitCode);
    void onProcessOutputReceived(const QString& appId, const QString& text, bool isError);

private:
    void initUI();
    void loadAppsConfig();
    void saveAppsConfig();
    void renderAppCards();
    QWidget* createCardWidget(const AppItem& item);

    QString getConfigFilePath() const;

    QList<AppItem> m_appList;
    QMap<QString, ProcessRunner*> m_runners; // appId -> ProcessRunner

    QVBoxLayout *m_cardsLayout;
    QScrollArea *m_scrollArea;
    LogViewer *m_logViewer;
    QLineEdit *m_searchEdit;

    QMap<QString, QWidget*> m_cardWidgets; // appId -> Card Widget
};

#endif // MAINWINDOW_H
