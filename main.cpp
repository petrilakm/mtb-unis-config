#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QTranslator tran;

    QApplication a(argc, argv);
    MainWindow w;

    const QLocale lang("en");
    QLocale::setDefault(lang);
    bool tranok = tran.load(":/lang_en");
    if (tranok) {
        qApp->installTranslator(&tran);
    }

    w.show();
    return a.exec();
}
