#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileDialog>
#include <QHostAddress>
#include <QMainWindow>
#include <QMessageBox>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_buttonConnect_clicked();
    void readData();
    void on_buttonSend_clicked();
    bool checkServerCredentials(QString ip, QString port);
    void clearConnection();
    void sendData(QString type, QByteArray data, QString fileName = nullptr);
    void on_buttonPicture_clicked();

private:
    Ui::MainWindow* ui;
    QTcpSocket socket;
};
#endif // MAINWINDOW_H
