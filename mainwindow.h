#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "astarview.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    AStarView *mAStarView{};

};
#endif // MAINWINDOW_H
