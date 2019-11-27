#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include "mainthread.h"
#include "mainwindow.h"
using namespace std;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void startServer();
    void stopServer();

signals:
    void infoSender(QString info);

public slots:
    void infoReceiver(QString info);  //Receive the information emitted by threads.

private:
    void createLayout();
    QWidget *mainWidget;
    QHBoxLayout *mainLayout;
    QVBoxLayout *leftLayout;
    QVBoxLayout *rightLayout;
    QHBoxLayout *topRightLayout;
    QVBoxLayout *labelLayout;
    QVBoxLayout *editLayout;
    QHBoxLayout *buttonLayout;
    QLabel *textLabel;
    QLabel *settingLabel;
    QLabel *ipLabel;
    QLabel *portLabel;
    QLabel *dirLabel;
    QTextEdit *textEdit;
    QPushButton *startButton;
    QPushButton *stopButton;
public:
    QLineEdit *ipEdit;
    QLineEdit *portEdit;
    QLineEdit *dirEdit;
    MainThread *mainThread;
};

#endif // MAINWINDOW_H
