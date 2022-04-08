#include "QTcpServerInterface.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>

QTcpServerInterface::QTcpServerInterface(Ui::MainWindow* ui)
    : m_pTcpServer(nullptr)
    , m_pCurrentTcpSocket(nullptr)
    , m_ui(ui)
{
}

QTcpServerInterface::~QTcpServerInterface() {}

bool QTcpServerInterface::open(const QString& port)
{
    m_pTcpServer = new QTcpServer(this);
    if (m_pTcpServer == nullptr || m_pTcpServer->listen(QHostAddress::Any, port.toInt()) == false) {
        qInfo() << "Can not listen on host: " << port;
        if (m_pTcpServer != nullptr) {
            delete m_pTcpServer;
            m_pTcpServer = nullptr;
        }
        return false;
    }
    connect(m_pTcpServer, &QTcpServer::newConnection, this, &QTcpServerInterface::onConnectionEstablish);
    return true;
}

bool QTcpServerInterface::close()
{
    if (m_pTcpServer != nullptr) {
        if (m_pCurrentTcpSocket != nullptr) {
            disconnect(m_pCurrentTcpSocket, SIGNAL(readyRead()), this, SLOT(onReceiveData()));
            disconnect(this,
                       SIGNAL(requestSend(const QByteArray&, bool)),
                       this,
                       SLOT(onSendData(const QByteArray&, bool)));
            disconnect(m_pCurrentTcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnection()));
        }
        m_pTcpServer->close();
        delete m_pTcpServer;
        m_pTcpServer = nullptr;
    }
    return true;
}

void QTcpServerInterface::onConnectionEstablish()
{
    QTcpSocket* pTcpSocket    = m_pTcpServer->nextPendingConnection();
    QString strConnectionInfo = pTcpSocket->peerAddress().toString() + ":" + QString("%1").arg(pTcpSocket->peerPort());
    if (m_pCurrentTcpSocket == nullptr) {
        m_pCurrentTcpSocket = pTcpSocket;
        qInfo() << "new client connected:" << strConnectionInfo;
        connect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(onReceiveData()));
        connect(this, SIGNAL(requestSend(const QByteArray&, bool)), this, SLOT(onSendData(const QByteArray&, bool)));
        connect(pTcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnection()));
    }
    else {
        // refuse new client
        qInfo() << "client exists already" << strConnectionInfo;
        pTcpSocket->write("client exists already\n");
        pTcpSocket->close();
    }
}

void QTcpServerInterface::onReceiveData()
{
    m_pCurrentTcpSocket = dynamic_cast<QTcpSocket*>(sender());
    if (m_pCurrentTcpSocket != nullptr) {
        auto data         = m_pCurrentTcpSocket->readAll();
        QString logString = QString::fromLocal8Bit("[QTcpServer revceive] ") + data.toHex();
        m_ui->log_browser->append(logString);
        emit processData(data);
    }
}

void QTcpServerInterface::onSendData(const QByteArray& data, bool verbose)
{
    if (m_pCurrentTcpSocket != nullptr) {
        m_pCurrentTcpSocket->write(data);
        if (verbose) {
            QString logString = QString::fromLocal8Bit("[QTcpServer send] ") + data.toHex();
            m_ui->log_browser->append(logString);
        }
    }
}

void QTcpServerInterface::onDisconnection()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    QString clientInfo =
        clientSocket->peerAddress().toString() + ":" + QString::fromStdString(std::to_string(clientSocket->peerPort()));
    qInfo() << QString::fromLocal8Bit("client disconnected") << clientInfo;
    if (!clientSocket) {
        qInfo() << "Invalid socket!";
        return;
    }
    disconnect(m_pCurrentTcpSocket, SIGNAL(readyRead()), this, SLOT(onReceiveData()));
    disconnect(this, SIGNAL(requestSend(const QByteArray&, bool)), this, SLOT(onSendData(const QByteArray&, bool)));
    disconnect(m_pCurrentTcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnection()));
    m_pCurrentTcpSocket->close();
    m_pCurrentTcpSocket = nullptr;
}
