#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(this)
{
    ui->setupUi(this);
    ui->textIpAddress->setText("127.0.1.1");
    ui->textPort->setText("8080");
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
    // socket.write(ui->textMessage->toPlainText().toUtf8());
    sendData("message", ui->textMessage->toPlainText().toUtf8());
    ui->textMessage->setText("");
}

void MainWindow::on_buttonPicture_clicked()
{
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
            // QByteArray data = image.readAll();
            // socket.write(data);
            sendData("image", image.readAll(), fileName);
        } else {
            QMessageBox::warning(this, "Error opening file",
                "Could not open the selected file.");
        }
        image.close();
    }
}

void MainWindow::sendData(QString type, QByteArray data, QString filename)
{
    QDataStream stream(&socket);
    QByteArray header;
    stream.setVersion(QDataStream::Qt_5_9);
    if (type == "message") {
        header.prepend(QString("type:message,filesize:%1").arg(data.size()).toUtf8());
    } else {
        header.prepend(QString("type:image,filesize:%1,file:%2")
                           .arg(data.size())
                           .arg(filename)
                           .toUtf8());
    }

    header.resize(128);
    QByteArray message = data.prepend(header);

    stream << message;
}

void MainWindow::clearConnection()
{
    disconnect(&socket, &QTcpSocket::readyRead, this, &MainWindow::readData);
    socket.close();
}
