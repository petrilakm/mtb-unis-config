#ifndef WINCONFIG_H
#define WINCONFIG_H

#include <QDialog>

namespace Ui {
class winConfig;
}

class winConfig : public QDialog
{
    Q_OBJECT

public:
    explicit winConfig(QWidget *parent = nullptr);
    ~winConfig();

private:
    Ui::winConfig *ui;
private slots:
    void onCancel(void);
    void onReset(void);
};

#endif // WINCONFIG_H
