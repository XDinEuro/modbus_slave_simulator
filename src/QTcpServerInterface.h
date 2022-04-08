#ifndef QTCPSERVERINTERFACE_H
#define QTCPSERVERINTERFACE_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include "mainwindow.h"

class QTcpServerInterface : public QObject
{
    Q_OBJECT;

public:
    QTcpServerInterface(Ui::MainWindow* m_ui);
    virtual ~QTcpServerInterface();

    bool open(const QString& port);
    bool close();

signals:
    void requestSend(const QByteArray& data, bool verbose = true);
    void processData(const QByteArray& data);

public slots:
    void onConnectionEstablish();
    void onReceiveData();
    void onSendData(const QByteArray& data, bool verbose);
    void onDisconnection();

protected:
    QTcpServer* m_pTcpServer;
    QTcpSocket* m_pCurrentTcpSocket;
    Ui::MainWindow* m_ui;
};

#endif  // QTCPSERVERINTERFACE_H
