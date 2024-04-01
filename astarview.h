#ifndef ASTARVIEW_H
#define ASTARVIEW_H

#include "model/astarmodel.h"
#include <QWidget>
#include <QTimer>

namespace Ui {
    class AStarView;
}

class AStarView : public QWidget
{
    Q_OBJECT
public:
    enum class Status{
        Config,
        Pause,
        Running,
        Finished
    };

    enum class ConfigMode{
        FromPoint,
        ToPoint,
        Obstacle
    };

    explicit AStarView(QWidget *parent = nullptr);

    Status status() const;
    void setStatus(Status newStatus);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

signals:
    void statusChanged();

private:
    Ui::AStarView *ui;
    Q_PROPERTY(Status status READ status WRITE setStatus NOTIFY statusChanged FINAL)

    AStarModel* mModel;
    int mStepTime = 1000;
    Status mStatus = Status::Config;
    ConfigMode mConfig = ConfigMode::Obstacle;
    QTimer mTimer;

    void launchComputations();
    void stopComputations();
    void computeOneStep();
    void clearCalculations();
    void resizeGrid();

private slots:
    void onCellToggled(const QModelIndex &index);
    void setSpeed(int speed);
    void setFromPointConfigMode();
    void setToPointConfigMode();
    void onReinitBtnClicked();
    void onRunBtnClicked();
    void onStatusChanged();
    void onRmObstaclesCLicked();
    void changeGridSize();
    void changeHeuristic(int heuristicIndex);
    void changeDiagonalPolicy(int checkBoxVal);
};

#endif // ASTARVIEW_H
