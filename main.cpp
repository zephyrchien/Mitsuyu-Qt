#include "mainwindow.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont font("Consolas");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(10);
    a.setFont(font);
    MainWindow w;
    w.setGeometry(QStyle::alignedRect(
        Qt::LeftToRight, Qt::AlignCenter, w.size(),
        qApp->desktop()->availableGeometry()));
    w.show();
    return a.exec();
}
