#include "mainwindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setAttribute(Qt::AA_UseDesktopOpenGL);
    QFile file(":/qss/res/qss/black.css");
    file.open(QFile::ReadOnly);

    QString styleSheet = QLatin1String(file.readAll());

    qApp->setStyleSheet(styleSheet);

    MainWindow w;
    w.show();

    return a.exec();
}
