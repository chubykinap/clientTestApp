#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      socket(this)
{
    ui->setupUi(this);

    ui->textInfo->setReadOnly(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_buttonConnect_clicked()
{
    clearConnection();
    QString serverIp = ui->textIpAddress->toPlainText();
    QString serverPort = ui->textPort->toPlainText();
    if (!checkServerCredentials(serverIp, serverPort)){
        QMessageBox::warning(this, "Error", "Wrong server credentials!\r\nPlease enter correct data.");
        return;
    }

    socket.connectToHost(QHostAddress(serverIp), serverPort.toInt());
    if (!socket.waitForConnected(2000)){
        QMessageBox::warning(this, "Error",
                             QString("Cannot connect to " + serverIp + ":%1").arg(serverPort));
    }
    else {
        socket.write("Hello, server!");
        ui->textInfo->append(QString("Connected to " + serverIp + ":%1").arg(serverPort));
        connect(&socket, &QTcpSocket::readyRead, this, &MainWindow::readData);
    }
}

bool MainWindow::checkServerCredentials(QString ip, QString port){
    QRegExp addressExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    if (ip == "" || !addressExp.exactMatch(ip)) {
        return false;
    }
    QRegExp reg("\\d*");
    if(port == NULL || !reg.exactMatch(port)) {
        return false;
    }
    return true;
}

void MainWindow::readData(){
    ui->textInfo->append(socket.readAll());
}

void MainWindow::on_buttonSend_clicked()
{
    socket.write(ui->textMessage->toPlainText().toUtf8());
    ui->textMessage->setText("");
}

void MainWindow::clearConnection(){
    disconnect(&socket, &QTcpSocket::readyRead, this, &MainWindow::readData);
    socket.close();
}
