#include <clocale>
#include <sstream>
#include <stdexcept>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QCloseEvent>
#include <QUrlQuery>

#include "../lib/qthelper.hpp"

static void wakeup(void *ctx)
{
    // This callback is invoked from any mpv thread (but possibly also
    // recursively from a thread that is calling the mpv API). Just notify
    // the Qt GUI thread to wake up (so that it can process events with
    // mpv_wait_event()), and return as quickly as possible.
    MainWindow *mainwindow = (MainWindow *)ctx;
    emit mainwindow->mpv_events();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _cfg = new QSettings();
    loadSetting();
    _nonce = "";
    _realm = "";

    if (_type == int(camtype::DCS_930L)) {
        //ui->w_control->setEnable(false);
    }
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
    //try mpv
    mpv = mpv_create();
    if (!mpv)
        throw std::runtime_error("can't create mpv instance");

    // If you have a HWND, use: int64_t wid = (intptr_t)hwnd;
//    int64_t wid = mpv_container->winId();
    int64_t wid = ui->wVideo->winId();
    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);
    // Enable default bindings, because we're lazy. Normally, a player using
    // mpv as backend would implement its own key bindings.
    mpv_set_option_string(mpv, "input-default-bindings", "yes");

    // Enable keyboard input on the X11 window. For the messy details, see
    // --input-vo-keyboard on the manpage.
    mpv_set_option_string(mpv, "input-vo-keyboard", "yes");

    // Let us receive property change events with MPV_EVENT_PROPERTY_CHANGE if
    // this property changes.
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);

    mpv_observe_property(mpv, 0, "track-list", MPV_FORMAT_NODE);
    mpv_observe_property(mpv, 0, "chapter-list", MPV_FORMAT_NODE);

    // Request log messages with level "info" or higher.
    // They are received as MPV_EVENT_LOG_MESSAGE.
    mpv_request_log_messages(mpv, "info");

    // From this point on, the wakeup function will be called. The callback
    // can come from any thread, so we use the QueuedConnection mechanism to
    // relay the wakeup in a thread-safe way.
    connect(this, &MainWindow::mpv_events, this, &MainWindow::onMpvEvents,
            Qt::QueuedConnection);
    mpv_set_wakeup_callback(mpv, wakeup, this);

    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("mpv failed to initialize");
    /*
    _vw1 = new QVideoWidget();
    _player = new QMediaPlayer();
    connect(_player, SIGNAL(error()), this, SLOT(onError()));
    connect(_player, SIGNAL(stateChanged), this, SLOT(onStateChanged()));
    ui->vbox->addWidget(_vw1);
    _player->setVideoOutput(_vw1);

    */
    start();
}

MainWindow::~MainWindow()
{
    delete ui;
}
/*
void MainWindow::onStateChanged(QMediaPlayer::State state){
    qDebug() << "onStateChanged:" << state;
}
void MainWindow::onError(QMediaPlayer::Error error)
{
 //   qDebug() << "onError:" << _player->errorString() << ": statue:" << _player->mediaStatus();
    qDebug() << "ERROR: " << error;
//    QString text = ui.textEdit->toPlainText();
//    text.append("\n").append(d->mediaPlayer->errorString()).append(" : ").append(d->mediaPlayer->mediaStatus());
//    ui.textEdit->setText(text);
}
*/
QString MainWindow::getDigestAuth(){
    /*
     Authorization: Digest username="admin", realm="DCS-5030L_A7", nonce="37c39755a278d15c45a228701c1dae20", uri="/setControlPanTilt", response="d5d1f7317a693c8024f0853c1a4de384", qop=auth, nc=000001a7, cnonce="3b552718be0ead2e"
*/
    //要先取得 nonce ，所以需要多一個Request

    QString concatenated = "username=\"" + _username + "\", " + "realm=\""+ _realm +"\"";
    //QByteArray data = concatenated.toLocal8Bit().toBase64();
    //QString headerData = "Digest " + data;
    QString headerData = "Digest " + concatenated;
    return headerData;

}

QString MainWindow::getBasicAuth(){

    QString concatenated = _username + ":" + _passwd;
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;
    return headerData;
}
void MainWindow::start(){
    QString filename = "";
    if (_type == int(camtype::CS_673W)) {
        //const QUrl url1 = QUrl("rtsp://admin:123456home@192.168.10.32:554/play1.sdp");
        //rtsp profile 1
        QString s ="rtsp://";
        s.append(_username+":"+_passwd+"@"+_url+":"+QString::number(_port)+"/play1.sdp");
        filename =s;
//        QUrl url1 = QUrl(s);
//        const QNetworkRequest requestRtsp1(url1);
//        _player->setMedia(requestRtsp1);
//        _player->play();
    } else if ((_type == int(camtype::TV_IP651W)) ||
               (_type == int(camtype::DCS_5030L)) ||
               (_type == int(camtype::DCS_930L))){
        //check if we need digest auth?nonce
        qDebug() << "first post";
        QByteArray byteArrayObject;
        byteArrayObject.append("");
        QString s ="http://";
        s.append(_username);
        s.append(":"+_passwd);
        request.setUrl(QUrl(s+"@"+_url));
//        QNetworkReply *reply =manager->get(request);
        QNetworkReply *reply =manager->post(request, byteArrayObject);
        if (reply->error()) {
            QByteArray dataReceived = reply->readAll();
            qDebug() << "dataReceived:" << dataReceived;
            qDebug() << "reply->error:" << reply->error();
        }

        s.append("@"+_url+"/video/mjpg.cgi");
        filename =s;
//        QUrl url1 = QUrl(s);
        //const QNetworkRequest requestRtsp1(url1);
        //request=QNetworkRequest(url1);
//        videomanager = new QNetworkAccessManager();
//        connect(videomanager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
//                this, SLOT(handleAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
//        request.setUrl(QUrl(s));
//        if (headerData != "") {
//            //request.setRawHeader("Authorization", headerData.toLocal8Bit());
//            request.setRawHeader("Authorization", headerData.toLocal8Bit());
//        }
//        //QNetworkReply *reply = videomanager->get(request);
//        _player->setMedia(request);
//        //_player->setMedia(reply);
//        _player->play();
//        }
    }
    if (true){
        if (filename != ""){
            const QByteArray c_filename = filename.toUtf8();
            const char *args[] = {"loadfile", c_filename.data(), NULL};
            mpv_command_async(mpv, 0, args);
        }else{
            qDebug() << "Did not support cam type: " << _type;
        }
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
    qDebug() << "handleAuthenticationRequired:" << response;
    qDebug() << "options:" << auth->options();

    _realm = auth->realm();
    auth->setUser(_username);
    auth->setPassword(_passwd);
    auth->setOption("realm", _realm);
    Q_UNUSED(reply);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (mpv)
        mpv_terminate_destroy(mpv);

    //_player->stop();
    //TODO: check something
    saveSetting();
    qDebug() << "closeEvent";
    event->accept();
    //Q_UNUSED(event);
}

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

    }else if ((_type == int(camtype::TV_IP651W)) || (_type == int(camtype::DCS_5030L))) {

        QString p = "";
        QString headerData = "";
//        QString headerData = getBasicAuth();
//        if (_realm !=""){
//            //QString headerData = getBasicAuth();
//            headerData = getDigestAuth();
//        }
        if (_type == int(camtype::TV_IP651W)){
            p = "/cgi/pantiltcontrol.cgi";
        }
        if (_type == int(camtype::DCS_5030L)){
            p = "/setControlPanTilt";
        }
        s.append(_username);
        s.append(":"+_passwd);
        s.append("@"+_url);
        s.append(p);
        request.setUrl(QUrl(s));
//        if (headerData !=""){
//            request.setRawHeader("Authorization", headerData.toLocal8Bit());
//        }
        QUrlQuery postData;
        postData.addQueryItem("PanSingleMoveDegree", QString::number(x));
        postData.addQueryItem("TiltSingleMoveDegree", QString::number(y));
        postData.addQueryItem("PanTiltSingleMove", "5");
        //QByteArray data;
        //data.append("PanSingleMoveDegree="+QString::number(x));
        //data.append("TiltSingleMoveDegree="+QString::number(y));
        //data.append("PanTiltSingleMove=5");
//        QNetworkReply *reply = manager->post(request, postData);
        qDebug() << "post request";
        QNetworkReply *reply = manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
        connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(slotError(QNetworkReply::NetworkError)));
    }

        /*camtype::DCS_5030L
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
void MainWindow::slotError(QNetworkReply::NetworkError error)
{
    qDebug() << "slotError:" << error;
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
        qDebug() << "managerFinished:" << reply->errorString();
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
               (_type == int(camtype::DCS_5030L))) {
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

void MainWindow::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                std::stringstream ss;
                ss << "At: " << time;
                statusBar()->showMessage(QString::fromStdString(ss.str()));
            } else if (prop->format == MPV_FORMAT_NONE) {
                // The property is unavailable, which probably means playback
                // was stopped.
                statusBar()->showMessage("");
            }
        } else if (strcmp(prop->name, "chapter-list") == 0 ||
                   strcmp(prop->name, "track-list") == 0)
        {
            // Dump the properties as JSON for demo purposes.
#if QT_VERSION >= 0x050000
//            if (prop->format == MPV_FORMAT_NODE) {
//                QVariant v = mpv::qt::node_to_variant((mpv_node *)prop->data);
//                // Abuse JSON support for easily printing the mpv_node contents.
//                QJsonDocument d = QJsonDocument::fromVariant(v);
//                append_log("Change property " + QString(prop->name) + ":\n");
//                append_log(d.toJson().data());
//            }
#endif
        }
        break;
    }
    case MPV_EVENT_VIDEO_RECONFIG: {
        // Retrieve the new video size.
        int64_t w, h;
        if (mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &w) >= 0 &&
            mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &h) >= 0 &&
            w > 0 && h > 0)
        {
            // Note that the MPV_EVENT_VIDEO_RECONFIG event doesn't necessarily
            // imply a resize, and you should check yourself if the video
            // dimensions really changed.
            // mpv itself will scale/letter box the video to the container size
            // if the video doesn't fit.
            std::stringstream ss;
            ss << "Reconfig: " << w << " " << h;
            statusBar()->showMessage(QString::fromStdString(ss.str()));
        }
        break;
    }
    case MPV_EVENT_LOG_MESSAGE: {
        struct mpv_event_log_message *msg = (struct mpv_event_log_message *)event->data;
        std::stringstream ss;
        ss << "[" << msg->prefix << "] " << msg->level << ": " << msg->text;
//        append_log(QString::fromStdString(ss.str()));
        qDebug() << QString::fromStdString(ss.str()) ;
        break;
    }
    case MPV_EVENT_SHUTDOWN: {
        mpv_terminate_destroy(mpv);
        mpv = NULL;
        break;
    }
    default: ;
        // Ignore uninteresting or unknown events.
    }
}
// This slot is invoked by wakeup() (through the mpv_events signal).
void MainWindow::onMpvEvents()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;
        handle_mpv_event(event);
    }
}
