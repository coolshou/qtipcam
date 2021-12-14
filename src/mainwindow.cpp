#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QCloseEvent>
#include <QUrlQuery>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    _cfg = new QSettings();
    loadSetting();

    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    manager = new QNetworkAccessManager();
//    connect(manager, SIGNAL(finished(QNetworkReply*)),
//            this, SLOT(managerFinished(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(handleAuthRequired(QNetworkReply*,QAuthenticator*)));
    //TODO: detect IPCam
    // detectIPCam();
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

QString MainWindow::getDigestAuth(){
    /*
     Authorization: Digest username="admin", realm="DCS-5030L_A7", nonce="37c39755a278d15c45a228701c1dae20", uri="/setControlPanTilt", response="d5d1f7317a693c8024f0853c1a4de384", qop=auth, nc=000001a7, cnonce="3b552718be0ead2e"
*/
    //要先取得 nonce ，所以需要多一個Request

    QString concatenated = "username=\"" + _username + "\" " + "realm=\"DCS-5030L_A7\"";
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Digest " + data;
    return headerData;

}

QString MainWindow::getBasicAuth(){

    QString concatenated = _username + ":" + _passwd;
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;
    return headerData;
}
void MainWindow::start(){
    if (_type == int(camtype::CS_673W)) {
        //const QUrl url1 = QUrl("rtsp://admin:123456home@192.168.10.32:554/play1.sdp");
        //rtsp profile 1
        QString s ="rtsp://";
        s.append(_username+":"+_passwd+"@"+_url+":"+QString::number(_port)+"/play1.sdp");
        QUrl url1 = QUrl(s);
        const QNetworkRequest requestRtsp1(url1);
        _player->setMedia(requestRtsp1);
        _player->play();
    } else if ((_type == int(camtype::TV_IP651W)) ||
               (_type == int(camtype::DCH_5030L)) ){
        QString headerData = getBasicAuth();

        //QString headerData = getDigestAuth();

        QString s ="http://";
        s.append(_username);
        if (_passwd != "") {
            s.append(":"+_passwd);
        }
        s.append("@"+_url+"/video/mjpg.cgi");
//        QUrl url1 = QUrl(s);
        //const QNetworkRequest requestRtsp1(url1);
        //request=QNetworkRequest(url1);
//        videomanager = new QNetworkAccessManager();
//        connect(videomanager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
//                this, SLOT(handleAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
        request.setUrl(QUrl(s));
        request.setRawHeader("Authorization", headerData.toLocal8Bit());
        //QNetworkReply *reply = videomanager->get(request);
        _player->setMedia(request);
        //_player->setMedia(reply);
        _player->play();
    }

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
void MainWindow::handleAuthRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    QByteArray response = reply->readAll();

    qDebug() << "handleAuthenticationRequired" << response << endl;
    auth->setUser(_username);
    auth->setPassword(_passwd);
    Q_UNUSED(reply);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    _player->stop();
    //TODO: check something
    saveSetting();
    qDebug() << "closeEvent";
    event->accept();
    //Q_UNUSED(event);
}

//QString MainWindow::_genAuthdata()
//{
//    QString concatenated = _username+":"+_passwd; //username:password
//    QByteArray data = concatenated.toLocal8Bit().toBase64();
//    QString headerData = "Basic " + data;
//    return headerData;
//}

void MainWindow::doMoveRequest(int x, int y)
{
    QString s ="http://";
    if (_type == int(camtype::CS_673W)) {
        //http://192.168.10.32/cgi/ptdc.cgi?command=set_relative_pos&posX=0&posY=2
//        QString s ="http://";
        QString p = "/cgi/ptdc.cgi?command=set_relative_pos";
        QString px = "&posX="+ QString::number(x);
        QString py = "&posY="+ QString::number(y);
        s.append(_url).append(p).append(px).append(py);
        request.setUrl(QUrl(s));
        QNetworkReply *reply = manager->get(request);
        connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(slotError(QNetworkReply::NetworkError)));

    }else if ((_type == int(camtype::TV_IP651W)) || (_type == int(camtype::DCH_5030L))) {
        QString p = "";
//        QString headerData = getBasicAuth();
        if (_type == int(camtype::TV_IP651W)){
            p = "/cgi/pantiltcontrol.cgi";
        }
        if (_type == int(camtype::DCH_5030L)){
            p = "/setControlPanTilt";
        }
//        s.append(_username);
//        if (_passwd != "") {
//            s.append(":"+_passwd);
//        }
//        s.append("@"+_url);
        s.append(_url);
        s.append(p);
        request.setUrl(QUrl(s));
//        request.setRawHeader("Authorization", headerData.toLocal8Bit());
        QUrlQuery postData;
        postData.addQueryItem("PanSingleMoveDegree", QString::number(x));
        postData.addQueryItem("TiltSingleMoveDegree", QString::number(y));
        postData.addQueryItem("PanTiltSingleMove", "5");
        //QByteArray data;
        //data.append("PanSingleMoveDegree="+QString::number(x));
        //data.append("TiltSingleMoveDegree="+QString::number(y));
        //data.append("PanTiltSingleMove=5");
//        QNetworkReply *reply = manager->post(request, postData);
        QNetworkReply *reply = manager->post(request,
                                             postData.toString(QUrl::FullyEncoded).toUtf8());
        connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(slotError(QNetworkReply::NetworkError)));
    }

        /*camtype::DCH_5030L
         * frm 6
        <form action="/setControlPanTilt" METHOD="POST" autocomplete="off">
        <INPUT type="hidden" name="PanSingleMoveDegree">
        <INPUT type="hidden" name="TiltSingleMoveDegree">
        <INPUT type="hidden" name="PanTiltSingleMove">
        </form>
        frm7
        <form action="/setControlPanTilt" METHOD="POST" autocomplete="off">
        <INPUT type="hidden" name="PanTiltPresetPositionMove" value="0">
        </form>
        */
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
    _type = _cfg->value("type", int(camtype::CS_673W)).toInt();
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
    _cfg->setValue("type", int(_type));
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
    _singleMove = 1;
    doMoveRequest(0, _y_offset);
}

void MainWindow::on_pb_lu_clicked()
{
    _singleMove = 0;
    doMoveRequest(-_x_offset, _y_offset);
}

void MainWindow::on_pb_ru_clicked()
{
    _singleMove = 2;
    doMoveRequest(_x_offset, _y_offset);
}

void MainWindow::on_pb_l_clicked()
{
    _singleMove = 3;
    doMoveRequest(-_x_offset, 0);
}

void MainWindow::on_pb_r_clicked()
{
    _singleMove = 5;
    doMoveRequest(_x_offset, 0);
}

void MainWindow::on_pb_ld_clicked()
{
    _singleMove = 6;
    doMoveRequest(-_x_offset, -_y_offset);
}

void MainWindow::on_pb_d_clicked()
{
    _singleMove = 7;
    doMoveRequest(0, -_y_offset);
}

void MainWindow::on_pb_rd_clicked()
{
    _singleMove = 8;
    doMoveRequest(_x_offset, -_y_offset);
}

void MainWindow::on_pb_h_clicked()
{
    if (_type == int(camtype::CS_673W)) {
        //http://192.168.10.32/cgi/ptdc.cgi?command=go_home
        QString s ="http://";
        QString p = "/cgi/ptdc.cgi?command=go_home";
        s.append(_url).append(p);

        request.setUrl(QUrl(s));
        manager->get(request);
    } else if ((_type == int(camtype::TV_IP651W)) ||
               (_type == int(camtype::DCH_5030L))) {
        _singleMove = 4;
        doMoveRequest(_x_offset, _y_offset);
    } else {
        qDebug() << "unknown cam type: " << _type;
    }
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

void MainWindow::on_actionConfig_triggered()
{
    loadSetting();
    dlg = new DialogCfg(_cfg, this);
    dlg->show();
}
