#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(this)
{
    ui->setupUi(this);

    ui->textInfo->setReadOnly(true);
    ui->buttonSend->setEnabled(false);
    ui->buttonPicture->setEnabled(false);
}

MainWindow::~MainWindow()
{
    clearConnection();
    delete ui;
}

void MainWindow::on_buttonConnect_clicked()
{
    clearConnection();
    QString serverIp = ui->textIpAddress->toPlainText();
    QString serverPort = ui->textPort->toPlainText();
    if (!checkServerCredentials(serverIp, serverPort)) {
        QMessageBox::warning(this, "Error", "Wrong server credentials!\r\nPlease enter correct data.");
        ui->buttonSend->setEnabled(false);
        ui->buttonPicture->setEnabled(false);
        return;
    }

    socket.connectToHost(QHostAddress(serverIp), serverPort.toInt());
    if (!socket.waitForConnected(2000)) {
        QMessageBox::warning(this, "Error",
            QString("Cannot connect to " + serverIp + ":%1").arg(serverPort));
        ui->buttonSend->setEnabled(false);
        ui->buttonPicture->setEnabled(false);
    } else {
        sendData("message", "Hello, server!");
        ui->textInfo->append(QString("Connected to " + serverIp + ":%1").arg(serverPort));
        connect(&socket, &QTcpSocket::readyRead, this, &MainWindow::readData);
        ui->buttonSend->setEnabled(true);
        ui->buttonPicture->setEnabled(true);
    }
}

bool MainWindow::checkServerCredentials(QString ip, QString port)
{
    QRegExp addressExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    if (ip == "" || !addressExp.exactMatch(ip)) {
        return false;
    }
    bool parsed;
    int intPort = port.toInt(&parsed);
    if (!parsed || intPort < 1 || intPort > 65535) {
        return false;
    }
    return true;
}

void MainWindow::readData()
{
    ui->textInfo->append(socket.readAll());
}

void MainWindow::on_buttonSend_clicked()
{
    if (!checkConnection())
        return;

    if (ui->textMessage->toPlainText() == "") {
        return;
    }
    sendData("message", ui->textMessage->toPlainText().toUtf8());
    ui->textMessage->setText("");
}

void MainWindow::on_buttonPicture_clicked()
{
    if (!checkConnection())
        return;
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select image to send"),
        "home/",
        tr("Images ( *.jpg *.png *.bmp"));
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "Error opening file",
            "Please select file to transfer.");
    } else {
        QFile image(fileName);
        if (image.exists() && image.open(QIODevice::ReadOnly)) {
            sendData("image", image.readAll(), fileName);
            ui->textInfo->append("Sent file " + fileName.split("/").last());
        } else {
            QMessageBox::warning(this, "Error opening file",
                "Could not open the selected file.");
        }
        image.close();
    }
}

void MainWindow::sendData(QString type, QByteArray data, QString filename)
{
    QByteArray header;
    if (type == "message") {
        header.prepend(QString("type:message,filesize:%1").arg(data.size()).toUtf8());
    } else {
        header.prepend(QString("type:image,filesize:%1,file:%2")
                           .arg(data.size())
                           .arg(filename.split("/").last())
                           .toUtf8());
    }

    header.resize(128);
    QByteArray message = data.prepend(header);
    socket.write(message);
}

void MainWindow::clearConnection()
{
    disconnect(&socket, &QTcpSocket::readyRead, this, &MainWindow::readData);
    socket.close();
}

bool MainWindow::checkConnection()
{
    if (!socket.waitForConnected(500)) {
        QMessageBox::warning(this, "Error", "Lost connection to server.");
        ui->buttonSend->setEnabled(false);
        ui->buttonPicture->setEnabled(false);
        return false;
    }
    return true;
}
