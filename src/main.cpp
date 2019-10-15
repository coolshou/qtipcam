#include "mainwindow.h"
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("qtipcam");
    a.setOrganizationName("coolshou.idv");
    a.setOrganizationDomain("coolshou.idv");

    QStringList argList = QCoreApplication::arguments();
    MainWindow w;
    if(argList.contains("--fullscreen")) {
        w.showFullScreen();
    }
    else if (argList.contains("--maximized")) {
        w.showMaximized();
    }
    else if (argList.contains("--help")) {
        std::cout << "Parameters:\n\t"
                     "--fullscreen:\tOpen Screen in mode FullScreen\n\t"
                     "--maximized:\tOpen Screen in mode Maximized"
                     "" << std::endl;
        exit(0);
    }
    else {
        w.show();
    }

    return a.exec();
}
