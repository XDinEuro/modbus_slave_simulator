#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

class QTcpServerInterface;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionStartButton_triggered();

    void on_actionStopButton_triggered();

    void on_applyButton_clicked();

    void on_processData(const QByteArray& data);

    void on_pushButton_start_clicked();

private:
    QTcpServerInterface* qTcpServerInterface;
    Ui::MainWindow *ui;
    bool m_isRunning;
};

#endif // MAINWINDOW_H
