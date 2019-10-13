#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QNetworkRequest>
#include <QVideoWidget>
#include <QSettings>
#include <QWebEnginePage>
#include <QUrl>
#include <QAuthenticator>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void doMoveRequest(int x, int y);
    void saveSetting();


    void detectIPCam();
    void start();
public slots:
    void handleAuthenticationRequired(QNetworkReply *reply, QAuthenticator *auth);
    void slotDetectReadyRead();
protected:
    void closeEvent(QCloseEvent *event);
private:
    QString _genAuthdata();
private slots:
    void slotReadyRead();
    void slotError(QNetworkReply::NetworkError);
    void on_actionQuit_triggered();
    void on_pb_u_clicked();
    void loadSetting();
    void managerFinished(QNetworkReply *reply);

    void on_pb_lu_clicked();
    void on_pb_ru_clicked();
    void on_pb_l_clicked();
    void on_pb_r_clicked();
    void on_pb_ld_clicked();
    void on_pb_d_clicked();
    void on_pb_rd_clicked();
    void on_pb_h_clicked();
    void on_sb_x_valueChanged(int x);
    void on_sb_y_valueChanged(int y);

    void on_pb_Patrol_clicked();

    void on_pb_pStop_clicked();

private:
    Ui::MainWindow *ui;
    QVideoWidget *_vw1;
    QMediaPlayer *_player;
    QNetworkAccessManager *manager;
    QNetworkRequest request;
    QWebEnginePage *_controlpage;
    QNetworkReply *_detectreply;
    QSettings *_cfg;
    QString _type;
    QString _protocal;
    QString _url;
    QString _username;
    QString _passwd;
    int _port;
    int _x_offset;
    int _y_offset;
};

#endif // MAINWINDOW_H
