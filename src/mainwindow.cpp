#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QTcpServerInterface.h"
#include <QStandardItemModel>
#include <QMessageBox>
 #include <QtCore/qmath.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->actionStartButton->setEnabled(true);
    ui->actionStopButton->setEnabled(false);
    m_isRunning = false;

}

MainWindow::~MainWindow()
{
    delete ui;
    delete qTcpServerInterface;
}

void MainWindow::on_actionStartButton_triggered()
{
//    workerThread = new QThread();
    qTcpServerInterface = new QTcpServerInterface(ui);
    qTcpServerInterface->open(ui->port_lineEdit->text());
//    qTcpServerInterface->moveToThread(workerThread);
    // something to be connected ?
    connect(qTcpServerInterface, &QTcpServerInterface::processData, this, &MainWindow::on_processData);
    ui->actionStartButton->setEnabled(false);
    ui->actionStopButton->setEnabled(true);
}

void MainWindow::on_actionStopButton_triggered()
{
    ui->actionStartButton->setEnabled(true);
    ui->actionStopButton->setEnabled(false);
    // disconnect first
    disconnect(qTcpServerInterface, &QTcpServerInterface::processData, this, &MainWindow::on_processData);
    qTcpServerInterface->close();
    qTcpServerInterface->deleteLater();
//    workerThread->quit();
//    workerThread->wait();

}

void MainWindow::on_applyButton_clicked()
{
    int startAddress = ui->startAddress_edit->text().toInt();
    int endAddress = ui->endAddress_edit->text().toInt();
    int colCount = ui->colCount_edit->text().toInt();
    int rowCount = (endAddress - startAddress)/colCount;
    int defaultValue = ui->defaultValue_comboBox->currentText().toInt();
    QStandardItemModel* dataModel = new QStandardItemModel(rowCount, colCount*2);
    QList<QString> headerLabels;
    for(int i = 0; i < colCount; i++) {
        headerLabels.append("address");
        headerLabels.append("value");
    }
    dataModel->setHorizontalHeaderLabels(headerLabels);

    QStandardItem *aItem;
    for(int i = 0; i < rowCount; i++) {
        for(int j = 0; j < colCount; j++) {
            aItem = new QStandardItem(QString::number(startAddress + j*rowCount + i));
            dataModel->setItem(i, 2*j, aItem);
            aItem = new QStandardItem(QString::number(defaultValue));
            dataModel->setItem(i, 2*j+1, aItem);
        }
    }
    ui->colis_tableView->setModel(dataModel);
}

void MainWindow::on_processData(const QByteArray &data)
{
    QByteArray replyData1;
    for(int i = 0; i < 5; i++) {
        replyData1.append(data.at(i)); // copy MBAP
    }
    QByteArray replyData2;
    int slaveID = data.at(6);
    int funcCode = data.at(7);
    if(slaveID != ui->slaveID_edit->text().toInt()) {
        QMessageBox::warning(this, "Info", "requested slave ID is different from simulator's");
        return;
    }
    if(funcCode == 1) {
    int address = (uint8_t)data.at(8) * 256 + (uint8_t)data.at(9);
    if(address < ui->startAddress_edit->text().toInt() || address > ui->endAddress_edit->text().toInt()) {
        QMessageBox::warning(this, "Info","requested address out of range");
        return;
    }
    int dataLen = (uint8_t)data.at(10) * 256 + (uint8_t)data.at(11);
    qInfo() << "dataLen : " << dataLen;
    int offset = address - ui->startAddress_edit->text().toInt();
    QStandardItemModel* model = (QStandardItemModel *)ui->colis_tableView->model();
    if(model == nullptr){
        QMessageBox::warning(this, "Info","data table not set up yet");
        return;
    }
    // get row and column
    int row = offset%(model->rowCount());
    int col = offset/model->rowCount();
    QString data2ReadStr;
    for(int i = 0; i < dataLen; i++) {
        int curRow = (row + i)%model->rowCount();
        int curCol = col + (row + i)/model->rowCount();
        if(model->item(curRow, 2*curCol + 1)->text().toInt() == 1) {
            data2ReadStr.push_back("1");
        }
        else {
            data2ReadStr.push_back("0");
        }
    }
    for(int i = 0 ; i < (dataLen/8 + 1) * 8 - dataLen; i++) data2ReadStr.push_back("0");
    QString dataHexStr;
    QString tmpStr;
    for(int i = 0; i < data2ReadStr.size(); i++) {
        if(i%4==0) {
            int tmp = 0;
            if(data2ReadStr[i+3] == "1") tmp += qPow(2, 3);
            if(data2ReadStr[i+2] == "1") tmp += qPow(2, 2);
            if(data2ReadStr[i+1] == "1") tmp += qPow(2, 1);
            if(data2ReadStr[i]   == "1") tmp += qPow(2, 0);
            if(tmp < 10) tmpStr.push_front(QString::number(tmp));
            else if (tmp == 10) tmpStr.push_front("a");
            else if (tmp == 11) tmpStr.push_front("b");
            else if (tmp == 12) tmpStr.push_front("c");
            else if (tmp == 13) tmpStr.push_front("d");
            else if (tmp == 14) tmpStr.push_front("e");
            else if (tmp == 15) tmpStr.push_front("f");
            if(tmpStr.size()==2) {
                dataHexStr.push_back(tmpStr);
                tmpStr.clear();
            }
        }
    }
    QByteArray dataHex = QByteArray::fromHex(dataHexStr.toLatin1());
    qInfo() << "dataHexstr : " << data2ReadStr;
    qInfo() << "dataHex : " << dataHexStr;

    replyData2.append(3+dataHex.size());
    replyData2.append(slaveID);
    replyData2.append(funcCode);
    replyData2.append(dataHex.size());
    replyData2.append(dataHex);

    replyData1.append(replyData2);
    emit qTcpServerInterface->requestSend(replyData1);
    return;
    }

    if(funcCode == 5) {
        int address = (uint8_t)data.at(8) * 256 + (uint8_t)data.at(9);
        if(address < ui->startAddress_edit->text().toInt() || address > ui->endAddress_edit->text().toInt()) {
            QMessageBox::warning(this, "warn","requested address out of range");
            return;
        }
        int offset = address - ui->startAddress_edit->text().toInt();
        QStandardItemModel* model = (QStandardItemModel *)ui->colis_tableView->model();
        // get row and column
        int row = offset%(model->rowCount());
        int col = offset/model->rowCount();
        int value = (uint8_t)data.at(10)>0?1:0;
        model->item(row, 2*col + 1)->setText(QString::number(value));
        emit qTcpServerInterface->requestSend(data); // reply the same
        return;
    }
}

void MainWindow::on_pushButton_start_clicked()
{
    if(m_isRunning == false) {
        qTcpServerInterface = new QTcpServerInterface(ui);
        qTcpServerInterface->open(ui->port_lineEdit->text());
        connect(qTcpServerInterface, &QTcpServerInterface::processData, this, &MainWindow::on_processData);
        ui->pushButton_start->setText("stop");
        m_isRunning = true;
    }
    else {
        ui->pushButton_start->setText("start");
        // disconnect first
        disconnect(qTcpServerInterface, &QTcpServerInterface::processData, this, &MainWindow::on_processData);
        qTcpServerInterface->close();
        qTcpServerInterface->deleteLater();
        m_isRunning = false;
    }

}
