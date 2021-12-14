#include "dialogcfg.h"
#include "ui_dialogcfg.h"

#include "camtype.h"

DialogCfg::DialogCfg(QSettings *cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCfg)
{
    _cfg = cfg;
    ui->setupUi(this);
    ui->cb_type->clear();
    for (int i = 0; i < CamList.size(); ++i){
        //ui->cb_type->addItem(CamList.at(i));
         qDebug() << CamList.at(i).toLocal8Bit().constData();// << Qt::endl;
         ui->cb_type->addItem(CamList.at(i).toLocal8Bit().constData());
    };

    // TODO: camtype combobox list
    loadSetting();
}

DialogCfg::~DialogCfg()
{
    delete ui;
}

void DialogCfg::loadSetting()
{
    _cfg->beginGroup("main");
    int type = _cfg->value("type", int(camtype::CS_673W)).toInt();
    ui->cb_type->setCurrentIndex(type);
    QString _protocal = _cfg->value("protocal", "rtsp").toString();
    if (_protocal == "rtsp") {
        ui->rb_rtsp->setChecked(true);
    } else{
        ui->rb_http->setChecked(true);
    }

    QString _url = _cfg->value("url", "192.168.10.32").toString();
    ui->le_url->setText(_url);
    QString _username = _cfg->value("username", "admin").toString();
    ui->le_username->setText(_username);
    QString _passwd = _cfg->value("passwd", "admin").toString();
    ui->le_passwd->setText(_passwd);
    int _port = _cfg->value("port", 554).toInt();
    ui->sb_port->setValue(_port);
    _cfg->endGroup();
}
void DialogCfg::saveSetting()
{
    _cfg->beginGroup("main");
    _cfg->setValue("type", ui->cb_type->currentIndex());
    QString _protocal;
    if (ui->rb_rtsp->isChecked()){
        _protocal = "rtsp";
    }
    if (ui->rb_http->isChecked()){
        _protocal = "http";
    }
    _cfg->setValue("protocal", _protocal);
    _cfg->setValue("url", ui->le_url->text());
    _cfg->setValue("username", ui->le_username->text());
    _cfg->setValue("passwd", ui->le_passwd->text());
    _cfg->setValue("port", ui->sb_port->value());
    _cfg->endGroup();

}

void DialogCfg::on_buttonBox_accepted()
{
    saveSetting();
    close();
}

void DialogCfg::on_buttonBox_rejected()
{
    close();
}
