#include "mainwindow.h"
#include <QBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    mAStarView = new AStarView(this);
    setCentralWidget(mAStarView);
}

MainWindow::~MainWindow() {}
