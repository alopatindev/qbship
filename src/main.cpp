#include <QApplication>
#include <QFileInfo>
#include "mainwindow.h"

class QApplication;
class QFileInfo;
class MainWindow;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QFileInfo(argv[0]).baseName());
    MainWindow window;
    window.show();
    return app.exec();
}
