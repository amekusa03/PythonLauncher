#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QString>

class StyleHelper {
public:
    static QString getAppStyleSheet() {
        return R"(
            QMainWindow {
                background-color: #f5f7fa;
            }
            QWidget {
                font-family: 'Segoe UI', 'Helvetica Neue', 'Hiragino Sans', 'Meiryo', sans-serif;
            }
            QGroupBox {
                border: 1px solid #e1e8ed;
                border-radius: 8px;
                margin-top: 12px;
                background-color: #ffffff;
                font-weight: bold;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                padding: 2px 8px;
                color: #2c3e50;
            }
            QPushButton {
                background-color: #ffffff;
                border: 1px solid #dcdfe6;
                border-radius: 6px;
                padding: 6px 14px;
                font-size: 13px;
                color: #606266;
            }
            QPushButton:hover {
                background-color: #ecf5ff;
                color: #409eff;
                border-color: #c6e2ff;
            }
            QPushButton:pressed {
                background-color: #d9ecff;
                border-color: #409eff;
            }
            QPushButton#btnPrimary {
                background-color: #409eff;
                color: white;
                border: none;
                font-weight: bold;
            }
            QPushButton#btnPrimary:hover {
                background-color: #66b1ff;
            }
            QPushButton#btnRun {
                background-color: #67c23a;
                color: white;
                border: none;
                font-weight: bold;
            }
            QPushButton#btnRun:hover {
                background-color: #85ce61;
            }
            QPushButton#btnStop {
                background-color: #f56c6c;
                color: white;
                border: none;
                font-weight: bold;
            }
            QPushButton#btnStop:hover {
                background-color: #f78989;
            }
            QSplitter::handle {
                background-color: #dcdfe6;
            }
            QScrollArea {
                border: none;
                background-color: transparent;
            }
        )";
    }

    static QString getCardStyleSheet(bool isRunning) {
        if (isRunning) {
            return R"(
                QFrame#AppCard {
                    background-color: #ffffff;
                    border: 2px solid #67c23a;
                    border-radius: 8px;
                }
            )";
        } else {
            return R"(
                QFrame#AppCard {
                    background-color: #ffffff;
                    border: 1px solid #e4e7ed;
                    border-radius: 8px;
                }
                QFrame#AppCard:hover {
                    border: 1px solid #b3d8ff;
                    background-color: #fafbfc;
                }
            )";
        }
    }
};

#endif // STYLEHELPER_H
