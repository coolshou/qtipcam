#ifndef DIALOGCFG_H
#define DIALOGCFG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class DialogCfg;
}

class DialogCfg : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCfg(QSettings *cfg, QWidget *parent = nullptr);
    ~DialogCfg();
    void loadSetting();
private slots:
    void on_buttonBox_accepted();
    void saveSetting();
    void on_buttonBox_rejected();

private:
    Ui::DialogCfg *ui;
    QSettings *_cfg;
};

#endif // DIALOGCFG_H
