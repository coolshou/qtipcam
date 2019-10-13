#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    _cfg = new QSettings();
    loadSetting();

    manager = new QNetworkAccessManager();
//    connect(manager, SIGNAL(finished(QNetworkReply*)),
//            this, SLOT(managerFinished(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(handleAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
    //TODO: detect IPCam
    detectIPCam();
    //_controlpage= new QWebEnginePage;
    //connect(_controlpage, SIGNAL(authenticationRequired(QUrl,QAuthenticator*)),
    //        SLOT(handleAuthenticationRequired(QUrl,QAuthenticator*)));

    ui->setupUi(this);
    _vw1 = new QVideoWidget();
    _player = new QMediaPlayer();
    ui->vbox->addWidget(_vw1);
    _player->setVideoOutput(_vw1);
    start();
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::start(){
    //const QUrl url1 = QUrl("rtsp://admin:123456home@192.168.10.32:554/play1.sdp");
    //rtsp profile 1
    QString s ="rtsp://";
    s.append(_username+":"+_passwd+"@"+_url+":"+QString::number(_port)+"/play1.sdp");
    QUrl url1 = QUrl(s);
    const QNetworkRequest requestRtsp1(url1);
    _player->setMedia(requestRtsp1);
    _player->play();

}
void MainWindow::detectIPCam(){
    // TODO, redirect? title?
    QString s ="http://";
    s.append(_url);

    request.setUrl(QUrl(s));
    _detectreply = manager->get(request);
    connect(_detectreply, SIGNAL(readyRead()), this, SLOT(slotDetectReadyRead()));

}
void MainWindow::slotDetectReadyRead()
{
    QString answer =_detectreply->readAll();
    qDebug() << "slotDetectReadyRead:\n" << answer << endl;
}
void MainWindow::handleAuthenticationRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    auth->setUser(_username);
    auth->setPassword(_passwd);
    Q_UNUSED(reply);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //TODO: check something
    saveSetting();
    Q_UNUSED(event);
}
QString MainWindow::_genAuthdata()
{
    QString concatenated = _username+":"+_passwd; //username:password
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;
    return headerData;
}
void MainWindow::doMoveRequest(int x, int y)
{
    //http://192.168.10.32/cgi/ptdc.cgi?command=set_relative_pos&posX=0&posY=2
    QString s ="http://";
    QString p = "/cgi/ptdc.cgi?command=set_relative_pos";
    QString px = "&posX="+ QString::number(x);
    QString py = "&posY="+ QString::number(y);
    s.append(_url).append(p).append(px).append(py);

    request.setUrl(QUrl(s));
    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
}
void MainWindow::slotReadyRead(){
    qDebug() << "slotReadyRead";
}
void MainWindow::slotError(QNetworkReply::NetworkError code)
{
    qDebug() << "slotError:" << code;
}
void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::loadSetting()
{
    _cfg->beginGroup("main");
    _protocal = _cfg->value("protocal", "rtsp").toString();
    _url = _cfg->value("url", "192.168.10.32").toString();
    _username = _cfg->value("username", "admin").toString();
    _passwd = _cfg->value("passwd", "admin").toString();
    _port = _cfg->value("port", 554).toInt();
    _x_offset = _cfg->value("x_offset", 2).toInt();
    _y_offset = _cfg->value("y_offset", 2).toInt();
    _cfg->endGroup();
}

void MainWindow::managerFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        qDebug() << reply->errorString();
        return;
    }

    QString answer = reply->readAll();
    qDebug() << "managerFinished:\n" << answer;
}

void MainWindow::saveSetting()
{
    _cfg->beginGroup("main");
    _cfg->setValue("protocal", _protocal);
    _cfg->setValue("url", _url);
    _cfg->setValue("username", _username);
    _cfg->setValue("passwd", _passwd);
    _cfg->setValue("port", _port);
    _cfg->setValue("x_offset", _x_offset);
    _cfg->setValue("y_offset", _y_offset);
    _cfg->endGroup();
}

void MainWindow::on_pb_u_clicked()
{
    doMoveRequest(0, _y_offset);
}

void MainWindow::on_pb_lu_clicked()
{
    doMoveRequest(-_x_offset, _y_offset);
}

void MainWindow::on_pb_ru_clicked()
{
    doMoveRequest(_x_offset, _y_offset);
}

void MainWindow::on_pb_l_clicked()
{
    doMoveRequest(-_x_offset, 0);
}

void MainWindow::on_pb_r_clicked()
{
    doMoveRequest(_x_offset, 0);
}

void MainWindow::on_pb_ld_clicked()
{
    doMoveRequest(-_x_offset, -_y_offset);
}

void MainWindow::on_pb_d_clicked()
{
    doMoveRequest(0, -_y_offset);
}

void MainWindow::on_pb_rd_clicked()
{
    doMoveRequest(_x_offset, -_y_offset);
}

void MainWindow::on_pb_h_clicked()
{
    //http://192.168.10.32/cgi/ptdc.cgi?command=go_home
    QString s ="http://";
    QString p = "/cgi/ptdc.cgi?command=go_home";
    s.append(_url).append(p);

    request.setUrl(QUrl(s));
    manager->get(request);
}

void MainWindow::on_sb_x_valueChanged(int x)
{
    _x_offset = x;
}

void MainWindow::on_sb_y_valueChanged(int y)
{
    _y_offset = y;
}

void MainWindow::on_pb_Patrol_clicked()
{
    //http://192.168.10.32/cgi/ptdc.cgi?command=single_patrol
    QString s ="http://";
    QString p = "/cgi/ptdc.cgi?command=single_patrol";
    s.append(_url).append(p);
    request.setUrl(QUrl(s));
    manager->get(request);
}

void MainWindow::on_pb_pStop_clicked()
{
    QString s ="http://";
    QString p = "/cgi/ptdc.cgi?command=stop";
    s.append(_url).append(p);
    request.setUrl(QUrl(s));
    manager->get(request);
}
