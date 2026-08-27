#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    
    QApplication::setApplicationName("PythonLauncher");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("CustomTools");

    MainWindow w;
    w.show();

    return a.exec();
}
