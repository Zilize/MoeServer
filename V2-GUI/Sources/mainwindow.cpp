#include "mainwindow.h"
#include "mainthread.h"
#include <QApplication>
#include <QPushButton>
#include <iostream>
#include <unistd.h>
#include <QLayout>
#include <QTextEdit>
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    createLayout();
}

MainWindow::~MainWindow()
{
}

void MainWindow::startServer()
{
    this->startButton->setEnabled(false);
    this->stopButton->setEnabled(true);

    QString ipQString = QString(ipEdit->text());
    QString portQString = QString(portEdit->text());
    QString dirQString = QString(dirEdit->text());
    mainThread = new MainThread(ipQString, portQString, dirQString);
    QObject::connect(mainThread, &MainThread::threadInfoSender, this, &MainWindow::infoReceiver);
    mainThread->start();

    emit infoSender("[STATUS] START SERVING!\n");
}

void MainWindow::stopServer()
{
    this->startButton->setEnabled(true);
    this->stopButton->setEnabled(false);
    mainThread->stop_proxy();
    mainThread->terminate();
    emit infoSender("[STATUS] STOP SERVING!\n");
}


void MainWindow::infoReceiver(QString info)
{
    this->textEdit->append(info);
}

void MainWindow::createLayout()
{
    setWindowTitle(tr("WEB SERVER"));

    mainWidget = new QWidget;
    this->setCentralWidget(mainWidget);
    mainLayout = new QHBoxLayout();
    mainWidget->setLayout(mainLayout);

    leftLayout = new QVBoxLayout;
    rightLayout = new QVBoxLayout;
    topRightLayout = new QHBoxLayout;
    labelLayout = new QVBoxLayout;
    editLayout = new QVBoxLayout;
    buttonLayout = new QHBoxLayout;

    textLabel = new QLabel(tr("STATUS of SERVER:"));
    textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    textEdit->setMinimumWidth(490);
    textEdit->setMinimumHeight(320);
    leftLayout->addWidget(textLabel);
    leftLayout->addWidget(textEdit);

    settingLabel = new QLabel(tr("SETTING:"));
    settingLabel->setMinimumWidth(250);
    ipLabel = new QLabel(tr("IP"));
    portLabel = new QLabel(tr("PORT"));
    dirLabel = new QLabel(tr("DIR"));
    labelLayout->addWidget(ipLabel);
    labelLayout->addWidget(portLabel);
    labelLayout->addWidget(dirLabel);
    ipEdit = new QLineEdit("0.0.0.0");
    portEdit = new QLineEdit("9000");
    dirEdit = new QLineEdit("/home/wwwroot/workspace");
    editLayout->addWidget(ipEdit);
    editLayout->addWidget(portEdit);
    editLayout->addWidget(dirEdit);
    ipLabel->setBuddy(ipEdit);
    portLabel->setBuddy(portEdit);
    dirLabel->setBuddy(dirEdit);
    topRightLayout->addLayout(labelLayout);
    topRightLayout->addLayout(editLayout);
    startButton = new QPushButton(tr("START"));
    stopButton = new QPushButton(tr("STOP"));
    stopButton->setEnabled(false);
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    rightLayout->addWidget(settingLabel);
    rightLayout->addLayout(topRightLayout);
    rightLayout->addLayout(buttonLayout);
    rightLayout->addStretch();
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    QObject::connect(startButton, &QPushButton::clicked, this, &MainWindow::startServer);
    QObject::connect(stopButton, &QPushButton::clicked, this, &MainWindow::stopServer);
    QObject::connect(this, &MainWindow::infoSender, this, &MainWindow::infoReceiver);
}
