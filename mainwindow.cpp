
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
    restoreWindowState();
    ui->setupUi(this);
    initUI();
    findLocalIPs();
    loadSettings();


    // buttons
    connect(ui->button_TcpClient, SIGNAL(clicked()), this, SLOT(onTcpClientButtonClicked()));
    connect(ui->button_TcpServer, SIGNAL(clicked()), this, SLOT(onTcpServerButtonClicked()));
    connect(ui->button_Refresh, SIGNAL(clicked()), this, SLOT(onRefreshButtonClicked()));
    connect(ui->ledOnButton, SIGNAL(clicked()), this, SLOT(onONButtonClicked()));
    connect(ui->ledOffButton, SIGNAL(clicked()), this, SLOT(onOFFButtonClicked()));
    connect(ui->exitButton, SIGNAL(clicked()), this, SLOT(onExitButtonClicked()));
}

/******************************************************************************
 ******************************************************************************
 **
 ** TCP Client
 **
 ******************************************************************************
 ******************************************************************************/

/***********************************
 *
 * TCP client start button clicked
 *
 ***********************************/
void MainWindow::onTcpClientButtonClicked()
{
    disconnect(ui->button_TcpClient, SIGNAL(clicked()), this, SLOT(onTcpClientButtonClicked()));

    if (setupConnection(TCPCLIENT))
    {
        ui->statusBar->showMessage(messageTCP + "Connecting to " + tcpClientTargetAddr.toString() + ": " + QString::number(tcpClientTargetPort), 0);
        ui->lineEdit_TcpClientTargetIP->setDisabled(true);
        ui->lineEdit_TcpClientTargetPort->setDisabled(true);
        ui->button_TcpClient->setText("Stop");

        connect(ui->button_TcpClient, SIGNAL(clicked()), this, SLOT(onTcpClientStopButtonClicked()));
        connect(mytcpclient, SIGNAL(myClientConnected(QString, quint16)), this, SLOT(onTcpClientNewConnection(QString, quint16)));
        connect(mytcpclient, SIGNAL(connectionFailed()), this, SLOT(onTcpClientTimeOut()));
    }

    saveSettings();
}

/***********************************
 *
 * TCP client has a new connection
 *
 ***********************************/
void MainWindow::onTcpClientNewConnection(const QString &from, quint16 port)
{
    disconnect(mytcpclient, SIGNAL(myClientConnected(QString, quint16)), this, SLOT(onTcpClientNewConnection(QString, quint16)));
    disconnect(mytcpclient, SIGNAL(connectionFailed()), this, SLOT(onTcpClientTimeOut()));
    disconnect(ui->button_TcpClient, SIGNAL(clicked()), this, SLOT(onTcpClientStopButtonClicked()));
    connect(mytcpclient, SIGNAL(myClientDisconnected()), this, SLOT(onTcpClientDisconnected()));

    ui->button_TcpClient->setDisabled(false);
    ui->button_TcpClient->setText("Disconnect");

    ui->button_TcpClientSend->setDisabled(false);
    ui->lineEdit_TcpClientSend->setDisabled(false);
    ui->textBrowser_TcpClientMessage->setDisabled(false);

    ui->statusBar->showMessage(messageTCP + "Connected to " + from + ": " + QString::number(port), 0);
    connect(ui->button_TcpClient, SIGNAL(clicked()), this, SLOT(onTcpClientDisconnectButtonClicked()));

    connect(mytcpclient, SIGNAL(newMessage(QString, QString)), this, SLOT(onTcpClientAppendMessage(QString, QString)));
    connect(ui->button_TcpClientSend, SIGNAL(clicked()), this, SLOT(onTcpClientSendMessage()));
    connect(ui->lineEdit_TcpClientSend, SIGNAL(returnPressed()), this, SLOT(onTcpClientSendMessage()));
}

/***********************************
 *
 * TCP client stop button clicked
 *
 ***********************************/
void MainWindow::onTcpClientStopButtonClicked()
{
    disconnect(ui->button_TcpClient, SIGNAL(clicked()), this, SLOT(onTcpClientStopButtonClicked()));

    ui->statusBar->showMessage(messageTCP + "Stopped", 2000);
    disconnect(mytcpclient, SIGNAL(myClientConnected(QString, quint16)), this, SLOT(onTcpClientNewConnection(QString, quint16)));
    disconnect(mytcpclient, SIGNAL(connectionFailed()), this, SLOT(onTcpClientTimeOut()));
    ui->button_TcpClient->setText("Connect");
    mytcpclient->abortConnection();
    ui->lineEdit_TcpClientTargetIP->setDisabled(false);
    ui->lineEdit_TcpClientTargetPort->setDisabled(false);

    ui->button_TcpClientSend->setDisabled(true);
    ui->lineEdit_TcpClientSend->setDisabled(true);
    ui->textBrowser_TcpClientMessage->setDisabled(true);

    connect(ui->button_TcpClient, SIGNAL(clicked()), this, SLOT(onTcpClientButtonClicked()));
}

/***********************************
 *
 * TCP client connection time out
 *
 ***********************************/
void MainWindow::onTcpClientTimeOut()
{
    ui->statusBar->showMessage(messageTCP + "Connection time out", 2000);
    disconnect(ui->button_TcpClient, SIGNAL(clicked()), this, SLOT(onTcpClientStopButtonClicked()));
    disconnect(mytcpclient, SIGNAL(myClientConnected(QString, quint16)), this, SLOT(onTcpClientNewConnection(QString, quint16)));
    disconnect(mytcpclient, SIGNAL(connectionFailed()), this, SLOT(onTcpClientTimeOut()));

    ui->button_TcpClient->setText("Connect");
    ui->lineEdit_TcpClientTargetIP->setDisabled(false);
    ui->lineEdit_TcpClientTargetPort->setDisabled(false);

    mytcpclient->closeClient();
    connect(ui->button_TcpClient, SIGNAL(clicked()), this, SLOT(onTcpClientButtonClicked()));
}

/***********************************
 *
 * TCP client diconnect button clicked
 *
 ***********************************/
void MainWindow::onTcpClientDisconnectButtonClicked()
{
    mytcpclient->disconnectCurrentConnection();
}

/***********************************
 *
 * TCP client disconnected
 *
 ***********************************/
void MainWindow::onTcpClientDisconnected()
{
    ui->statusBar->showMessage(messageTCP + "Disconnected from server", 2000);
    disconnect(mytcpclient, SIGNAL(myClientDisconnected()), this, SLOT(onTcpClientDisconnected()));
    disconnect(mytcpclient, SIGNAL(newMessage(QString, QString)), this, SLOT(onTcpClientAppendMessage(QString, QString)));
    disconnect(ui->button_TcpClientSend, SIGNAL(clicked()), this, SLOT(onTcpClientSendMessage()));
    disconnect(ui->lineEdit_TcpClientSend, SIGNAL(returnPressed()), this, SLOT(onTcpClientSendMessage()));
    disconnect(ui->button_TcpClient, SIGNAL(clicked()), this, SLOT(onTcpClientDisconnectButtonClicked()));
    ui->button_TcpClient->setText("Connect");

    ui->button_TcpClientSend->setDisabled(true);
    ui->lineEdit_TcpClientSend->setDisabled(true);
    ui->textBrowser_TcpClientMessage->setDisabled(true);

    ui->button_TcpClient->setDisabled(false);
    ui->lineEdit_TcpClientTargetIP->setDisabled(false);
    ui->lineEdit_TcpClientTargetPort->setDisabled(false);

    mytcpclient->closeClient();
    mytcpclient->close();

    connect(ui->button_TcpClient, SIGNAL(clicked()), this, SLOT(onTcpClientButtonClicked()));
}

/***********************************
 *
 * TCP client append a message
 * to message browser
 *
 ***********************************/
void MainWindow::onTcpClientAppendMessage(const QString &from, const QString &message)
{
    if (from.isEmpty() || message.isEmpty())
    {
        return;
    }

    QTextCursor cursor(ui->textBrowser_TcpClientMessage->textCursor());
    cursor.movePosition(QTextCursor::End);

    if (from == "System")
    {
        QColor color = ui->textBrowser_TcpClientMessage->textColor();
        ui->textBrowser_TcpClientMessage->setTextColor(Qt::gray);
        ui->textBrowser_TcpClientMessage->append(message);
        ui->textBrowser_TcpClientMessage->setTextColor(color);
    }
    else
    {
        QTextTable *table = cursor.insertTable(1, 2, tableFormat);
        table->cellAt(0, 0).firstCursorPosition().insertText('<' + from + "> ");
        table->cellAt(0, 1).firstCursorPosition().insertText(message);

        if(message == "ON")
        {
            ui->ledImageButton->setStyleSheet("background-color: transparent;\nborder-image: url(:images/LEDON.png);\nbackground: none;\nborder: none;\nbackground-repeat: none;");
        }
        else if(message == "OFF")
        {
            ui->ledImageButton->setStyleSheet("background-color: transparent;\nborder-image: url(:images/LEDOFF.png);\nbackground: none;\nborder: none;\nbackground-repeat: none;");
        }
    }
    QScrollBar *bar = ui->textBrowser_TcpClientMessage->verticalScrollBar();
    bar->setValue(bar->maximum());
}

/***********************************
 *
 * Send message through TCP client
 *
 ***********************************/
void MainWindow::onTcpClientSendMessage()
{
    QString text = ui->lineEdit_TcpClientSend->text();
    if (text.isEmpty())
    {
        return;
    }

    mytcpclient->sendMessage(text);

    onTcpClientAppendMessage("Me", text);
    ui->lineEdit_TcpClientSend->clear();
}

/******************************************************************************
 ******************************************************************************
 **
 ** TCP Server
 **
 ******************************************************************************
 ******************************************************************************/

/***********************************
 *
 * TCP server listen button clicked
 *
 ***********************************/
void MainWindow::onTcpServerButtonClicked()
{
    disconnect(ui->button_TcpServer, SIGNAL(clicked()), this, SLOT(onTcpServerButtonClicked()));

    if (setupConnection(TCPSERVER))
    {
        ui->statusBar->showMessage(messageTCP + "Listerning to " + localAddr.toString() + ": " + QString::number(tcpServerListenPort), 0);
        connect(ui->button_TcpServer, SIGNAL(clicked()), this, SLOT(onTcpServerStopButtonClicked()));
        ui->button_TcpServer->setText("Stop");
        ui->lineEdit_TcpServerListenPort->setDisabled(true);
        connect(mytcpserver, SIGNAL(myServerConnected(QString, quint16)), this, SLOT(onTcpServerNewConnection(QString, quint16)));
    }
    else
    {
        ui->statusBar->showMessage(messageTCP + "Failed to listen to: " + localAddr.toString() + ": " + QString::number(tcpServerListenPort), 2000);
        connect(ui->button_TcpServer, SIGNAL(clicked()), this, SLOT(onTcpServerButtonClicked()));
    }

    saveSettings();
}

/***********************************
 *
 * TCP server has a new connection
 *
 ***********************************/
void MainWindow::onTcpServerNewConnection(const QString &from, quint16 port)
{
    ui->statusBar->showMessage(messageTCP + "Connected with " + from + ": " + QString::number(port), 0);
    disconnect(mytcpserver, SIGNAL(myServerConnected(QString, quint16)), this, SLOT(onTcpServerNewConnection(QString, quint16)));

    disconnect(ui->button_TcpServer, SIGNAL(clicked()), this, SLOT(onTcpServerStopButtonClicked()));
    ui->button_TcpServer->setText("Disconnect");
    connect(ui->button_TcpServer, SIGNAL(clicked()), this, SLOT(onTcpServerDisconnectButtonClicked()));

    connect(mytcpserver, SIGNAL(myServerDisconnected()), this, SLOT(onTcpServerDisconnected()));
    connect(mytcpserver, SIGNAL(newMessage(QString, QString)), this, SLOT(onTcpServerAppendMessage(QString, QString)));

    ui->button_TcpServerSend->setDisabled(false);
    ui->lineEdit_TcpServerSend->setDisabled(false);
    ui->textBrowser_TcpServerMessage->setDisabled(false);
    connect(ui->button_TcpServerSend, SIGNAL(clicked()), this, SLOT(onTcpServerSendMessage()));
    connect(ui->lineEdit_TcpServerSend, SIGNAL(returnPressed()), this, SLOT(onTcpServerSendMessage()));
}

/***********************************
 *
 * TCP server stop button clicked
 *
 ***********************************/
void MainWindow::onTcpServerStopButtonClicked()
{
    disconnect(ui->button_TcpServer, SIGNAL(clicked()), this, SLOT(onTcpServerStopButtonClicked()));

    ui->statusBar->showMessage(messageTCP + "Stopped", 2000);
    disconnect(mytcpserver, SIGNAL(myServerConnected(QString, quint16)));
    mytcpserver->stopListening();
    ui->button_TcpServer->setText("Start");
    ui->lineEdit_TcpServerListenPort->setDisabled(false);

    ui->button_TcpServerSend->setDisabled(true);
    ui->lineEdit_TcpServerSend->setDisabled(true);
    ui->textBrowser_TcpServerMessage->setDisabled(true);

    connect(ui->button_TcpServer, SIGNAL(clicked()), this, SLOT(onTcpServerButtonClicked()));
}

/***********************************
 *
 * TCP server disconnect button clicked
 *
 ***********************************/
void MainWindow::onTcpServerDisconnectButtonClicked()
{
    mytcpserver->stopConnection();
}

/***********************************
 *
 * TCP server disconnected
 *
 ***********************************/
void MainWindow::onTcpServerDisconnected()
{
    ui->statusBar->showMessage(messageTCP + "Client disconnected, listerning to " + localAddr.toString() + ": " + QString::number(tcpServerListenPort), 0);

    ui->button_TcpServerSend->setDisabled(true);
    ui->lineEdit_TcpServerSend->setDisabled(true);
    ui->textBrowser_TcpServerMessage->setDisabled(true);

    disconnect(ui->button_TcpServer, SIGNAL(clicked()), this, SLOT(onTcpServerDisconnectButtonClicked()));
    disconnect(mytcpserver, SIGNAL(myServerDisconnected()), this, SLOT(onTcpServerDisconnected()));
    disconnect(mytcpserver, SIGNAL(newMessage(QString, QString)), this, SLOT(onTcpServerAppendMessage(QString, QString)));
    disconnect(ui->button_TcpServerSend, SIGNAL(clicked()), this, SLOT(onTcpServerSendMessage()));
    disconnect(ui->lineEdit_TcpServerSend, SIGNAL(returnPressed()), this, SLOT(onTcpServerSendMessage()));

    connect(ui->button_TcpServer, SIGNAL(clicked()), this, SLOT(onTcpServerStopButtonClicked()));
    ui->button_TcpServer->setText("Stop");
    connect(mytcpserver, SIGNAL(myServerConnected(QString, quint16)), this, SLOT(onTcpServerNewConnection(QString, quint16)));
}

/***********************************
 *
 * TCP server append a message
 * to message browser
 *
 ***********************************/
void MainWindow::onTcpServerAppendMessage(const QString &from, const QString &message)
{
    if (from.isEmpty() || message.isEmpty())
    {
        return;
    }

    QTextCursor cursor(ui->textBrowser_TcpServerMessage->textCursor());
    cursor.movePosition(QTextCursor::End);

    if (from == "System")
    {
        QColor color = ui->textBrowser_TcpServerMessage->textColor();
        ui->textBrowser_TcpServerMessage->setTextColor(Qt::gray);
        ui->textBrowser_TcpServerMessage->append(message);
        ui->textBrowser_TcpServerMessage->setTextColor(color);
    }
    else
    {
        QTextTable *table = cursor.insertTable(1, 2, tableFormat);
        table->cellAt(0, 0).firstCursorPosition().insertText('<' + from + "> ");
        table->cellAt(0, 1).firstCursorPosition().insertText(message);
    }
    QScrollBar *bar = ui->textBrowser_TcpServerMessage->verticalScrollBar();
    bar->setValue(bar->maximum());
}

/***********************************
 *
 * Send message through TCP server
 *
 ***********************************/
void MainWindow::onTcpServerSendMessage()
{
    QString text = ui->lineEdit_TcpServerSend->text();
    if (text.isEmpty())
    {
        return;
    }

    mytcpserver->sendMessage(text);

    onTcpServerAppendMessage("Me", text);
    ui->lineEdit_TcpServerSend->clear();
}



/***********************************
 *
 * UI initialization
 *
 ***********************************/
void MainWindow::initUI()
{
    QString rule = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    ui->lineEdit_TcpClientTargetIP->setValidator(new QRegExpValidator(QRegExp("^" + rule + "\\." + rule + "\\." + rule + "\\." + rule + "$"), this));
    ui->lineEdit_TcpClientTargetPort->setValidator(new QIntValidator(0, 65535, this));
    ui->lineEdit_TcpServerListenPort->setValidator(new QIntValidator(0, 65535, this));

    ui->textBrowser_TcpClientMessage->setFocusPolicy(Qt::NoFocus);
    ui->textBrowser_TcpServerMessage->setFocusPolicy(Qt::NoFocus);

    ui->lineEdit_TcpClientSend->setFocusPolicy(Qt::StrongFocus);
    ui->lineEdit_TcpClientSend->setFocus();

    ui->button_TcpClientSend->setDisabled(true);
    ui->lineEdit_TcpClientSend->setDisabled(true);
    ui->textBrowser_TcpClientMessage->setDisabled(true);

    ui->button_TcpServerSend->setDisabled(true);
    ui->lineEdit_TcpServerSend->setDisabled(true);
    ui->textBrowser_TcpServerMessage->setDisabled(true);

    tableFormat.setBorder(0);


}

/***********************************
 *
 * Setup connections
 *
 ***********************************/
bool MainWindow::setupConnection(quint8 type)
{
    bool isSuccess = false;
    localAddr.setAddress(ui->label_LocalIP->text());

    switch (type)
    {
    case TCPSERVER:
        tcpServerListenPort = ui->lineEdit_TcpServerListenPort->text().toInt();
        if (mytcpserver == nullptr)
        {
            mytcpserver = new MyTCPServer;
        }
        isSuccess = mytcpserver->listen(localAddr, tcpServerListenPort);
        break;
    case TCPCLIENT:
        isSuccess = true;
        tcpClientTargetAddr.setAddress(ui->lineEdit_TcpClientTargetIP->text());
        tcpClientTargetPort = ui->lineEdit_TcpClientTargetPort->text().toInt();
        if (mytcpclient == nullptr)
        {
            mytcpclient = new MyTCPClient;
        }
        mytcpclient->connectTo(tcpClientTargetAddr, tcpClientTargetPort);
        break;
    }
    return isSuccess;
}

/***********************************
 *
 * Find IP of local WiFi connections
 *
 ***********************************/
void MainWindow::findLocalIPs()
{
    ui->comboBox_Interface->clear();
    interfaceList.clear();
    QList<QNetworkInterface> listInterface = QNetworkInterface::allInterfaces();
    for (int i = 0; i < listInterface.size(); ++i)
    {
        //qDebug()<<listInterface.at(i).name();
        //if (listInterface.at(i).humanReadableName().contains("Wi-Fi") || listInterface.at(i).humanReadableName().contains("wlp"))
        {
            interfaceList.append(listInterface.at(i));
        }
    }

    if (interfaceList.isEmpty())
    {
        // TODO wifilist is empty
    }
    else
    {
        for (int j = 0; j < interfaceList.size(); ++j)
        {
            ui->comboBox_Interface->addItem(interfaceList.at(j).humanReadableName());
        }
    }
}

/***********************************
 *
 * Load settings from local configuration file
 *
 ***********************************/
void MainWindow::loadSettings()
{
    settingsFileDir = QApplication::applicationDirPath() + "/config.ini";
    QSettings settings(settingsFileDir, QSettings::IniFormat);
    ui->lineEdit_TcpClientTargetIP->setText(settings.value("TCP_CLIENT_TARGET_IP", "127.0.0.1").toString());
    ui->lineEdit_TcpClientTargetPort->setText(settings.value("TCP_CLIENT_TARGET_PORT", 1234).toString());

    ui->lineEdit_TcpServerListenPort->setText(settings.value("TCP_SERVER_LISTEN_PORT", 1234).toString());

    ui->tabWidget->setCurrentIndex(settings.value("TAB_INDEX", 0).toInt());

    int index = settings.value("interfaceIndex", 0).toInt();
    index = ui->comboBox_Interface->currentIndex();
    if (interfaceList.count() == 0) return;
    if (ui->comboBox_Interface->count() >= index)
    {

        ui->comboBox_Interface->setCurrentIndex(index);
        for (int i = 0; i < interfaceList.at(index).addressEntries().size(); ++i)
        {
            if (interfaceList.at(index).addressEntries().at(i).ip().protocol() == QAbstractSocket::IPv4Protocol)
            {
                ui->label_LocalIP->setText(interfaceList.at(index).addressEntries().at(i).ip().toString());
            }
        }
    }
    else if (ui->comboBox_Interface->count() > 0 && ui->comboBox_Interface->count() < index)
    {
        ui->comboBox_Interface->setCurrentIndex(index);
        for (int i = 0; i < interfaceList.at(0).addressEntries().size(); ++i)
        {
            if (interfaceList.at(0).addressEntries().at(i).ip().protocol() == QAbstractSocket::IPv4Protocol)
            {
                ui->label_LocalIP->setText(interfaceList.at(0).addressEntries().at(i).ip().toString());
            }
        }
    }
}

/***********************************
 *
 * Save settings to local configuration file
 *
 ***********************************/
void MainWindow::saveSettings()
{
    QSettings settings(settingsFileDir, QSettings::IniFormat);
    settings.setValue("TCP_CLIENT_TARGET_IP", ui->lineEdit_TcpClientTargetIP->text());
    settings.setValue("TCP_CLIENT_TARGET_PORT", ui->lineEdit_TcpClientTargetPort->text());

    settings.setValue("TCP_SERVER_LISTEN_PORT", ui->lineEdit_TcpServerListenPort->text());

    settings.setValue("INTERFACE_INDEX", ui->comboBox_Interface->currentIndex());

    settings.setValue("TAB_INDEX", ui->tabWidget->currentIndex());
    settings.sync();
}

void MainWindow::onRefreshButtonClicked()
{
    //saveSettings();
    //findLocalIPs();
    loadSettings();
}

void MainWindow::onONButtonClicked()
{
    mytcpserver->sendMessage("ON");
    ui->ledImageButton->setStyleSheet("background-color: transparent;\nborder-image: url(:images/LEDON.png);\nbackground: none;\nborder: none;\nbackground-repeat: none;");
}

void MainWindow::onOFFButtonClicked()
{
    mytcpserver->sendMessage("OFF");
    ui->ledImageButton->setStyleSheet("background-color: transparent;\nborder-image: url(:images/LEDOFF.png);\nbackground: none;\nborder: none;\nbackground-repeat: none;");
}
void MainWindow::onExitButtonClicked()
{
    qApp->quit();
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("ZPeng", "SocketTest");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::restoreWindowState()
{
    QSettings settings("ZPeng", "SocketTest");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

MainWindow::~MainWindow()
{
    delete ui;
}
