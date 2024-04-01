#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.setWindowTitle("A* viewer");
    mainWindow.setMinimumSize(650, 650);
    mainWindow.setGeometry(100, 100, 810, 800); // Set window geometry
    mainWindow.show();

    return app.exec();
}

