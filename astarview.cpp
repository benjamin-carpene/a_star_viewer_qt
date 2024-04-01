#include "astarview.h"
#include "ui_astarview.h"

#include <QVector>

AStarView::AStarView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AStarView)
{
    // The form generated ui
    ui->setupUi(this);

    // Set layouts
    setLayout(ui->verticalLayout);
    ui->configBox->setLayout(ui->configLayout);

    // connect and initialize widgets values
    // model and tableView
    mModel = new AStarModel(this);
    ui->tableView->setModel(mModel);
    connect(ui->tableView, &QTableView::clicked, this, &AStarView::onCellToggled);

    // Size
    connect(ui->widthSlider, &QSlider::valueChanged, this, &AStarView::changeGridSize);
    connect(ui->heightSlider, &QSlider::valueChanged, this, &AStarView::changeGridSize);
    ui->widthSlider->setValue(30);
    ui->heightSlider->setValue(20);

    //config buttons
    connect(ui->ReinitBtn, &QPushButton::clicked, this, &AStarView::onReinitBtnClicked);
    connect(ui->startBtn, &QPushButton::clicked, this, &AStarView::setFromPointConfigMode);
    connect(ui->stopBtn, &QPushButton::clicked, this, &AStarView::setToPointConfigMode);
    connect(ui->deleteObstacleBtn, &QPushButton::clicked, this, &AStarView::onRmObstaclesCLicked);

    // Run btn (play/pause)
    connect(ui->runBtn, &QPushButton::clicked, this, &AStarView::onRunBtnClicked);

    // Speed slider & computations
    connect(ui->speedSlider, &QSlider::valueChanged, this, &AStarView::setSpeed);
    ui->speedSlider->setValue(1);
    connect(&mTimer, &QTimer::timeout, this, &AStarView::computeOneStep);
    connect(this, &AStarView::statusChanged, this, &AStarView::onStatusChanged);

    // Heuristic combobox
    for(auto heuristic : AStarModel::heuristics){
        Q_UNUSED(heuristic);
        ui->heuristicCombo->addItem("");
    }
    ui->heuristicCombo->setItemText(AStarModel::HeuristicType::Euclidian, "Euclidian heuristic");
    ui->heuristicCombo->setItemText(AStarModel::HeuristicType::Manhattan, "Manhattan heuristic");
    ui->heuristicCombo->setItemText(AStarModel::HeuristicType::Chebyshev, "Chebyshev heuristic");
    ui->heuristicCombo->setItemText(AStarModel::HeuristicType::Octile, "Octile heuristic");
    connect(ui->heuristicCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AStarView::changeHeuristic);
    ui->heuristicCombo->setCurrentIndex(AStarModel::HeuristicType::Manhattan);

    // Diagonal movements checkbox
    connect(ui->diagonalBox, &QCheckBox::stateChanged, this, &AStarView::changeDiagonalPolicy);
    ui->diagonalBox->setChecked(false); changeDiagonalPolicy(0);
}

void AStarView::clearCalculations()
{
    mModel->clear();
}

void AStarView::setSpeed(int speed)
{
    mStepTime = 1000/speed;
    mTimer.setInterval(mStepTime);
    // update associated label text
    ui->speedLabel->setText(QString("Speed : %1 Hz").arg(speed));
}

void AStarView::onCellToggled(const QModelIndex &index)
{
    if(status() == Status::Config){
        switch(mConfig){
        case ConfigMode::Obstacle:
            mModel->toggleObstacle(index);
            break;
        case ConfigMode::FromPoint:
            mModel->setStart(index);
            mConfig = ConfigMode::Obstacle;
            break;
        case ConfigMode::ToPoint:
            mModel->setStop(index);
            mConfig = ConfigMode::Obstacle;
            break;
        }
    }
}

void AStarView::setFromPointConfigMode()
{
    mConfig = ConfigMode::FromPoint;
    setStatus(Status::Config);
}

void AStarView::setToPointConfigMode()
{
    mConfig = ConfigMode::ToPoint;
    setStatus(Status::Config);
}

void AStarView::onReinitBtnClicked()
{
    stopComputations();
    clearCalculations();
    setStatus(Status::Config);
}

void AStarView::computeOneStep()
{
    auto modelState = mModel->compute();

    switch(modelState){
    case a_star::State::finished:
        stopComputations();
        setStatus(Status::Finished);
        break;
    default:
        break;
    }
}

void AStarView::launchComputations()
{
    computeOneStep();
    mTimer.start(mStepTime);
}

void AStarView::stopComputations()
{
    mTimer.stop();
}

void AStarView::onRunBtnClicked()
{
    switch(status()){
    case Status::Config:
        mConfig = ConfigMode::Obstacle;
        setStatus(Status::Running);
        break;
    case Status::Pause:
        setStatus(Status::Running);
        break;
    case Status::Running:
        setStatus(Status::Pause);
        break;
    case Status::Finished:
        clearCalculations();
        setStatus(Status::Running);
        break;
    }
}

void AStarView::onRmObstaclesCLicked()
{
    mModel->removeObstacles();
}

void AStarView::changeGridSize()
{
    int width = ui->widthSlider->value();
    int height = ui->heightSlider->value();
    // Change grid size (cols * rows)
    mModel->changeSize(width, height);
    resizeGrid();
    //update labels
    ui->widthLabel->setText(QString("Width : %1").arg(width));
    ui->heightLabel->setText(QString("Height: %1").arg(height));
}

void AStarView::resizeGrid()
{
    // compute the new width and height for cells
    int width = ui->tableView->size().width() / ui->widthSlider->value();
    int height = ui->tableView->size().height() / ui->heightSlider->value();
    int length = std::min(width, height);
    ui->tableView->verticalHeader()->setDefaultSectionSize(length);
    ui->tableView->horizontalHeader()->setDefaultSectionSize(length);
}

void AStarView::changeHeuristic(int heuristicIndex)
{
    mModel->setHeuristic(static_cast<AStarModel::HeuristicType>(heuristicIndex));
}

void AStarView::changeDiagonalPolicy(int checkBoxVal)
{
    switch (checkBoxVal){
    case 0: // unchecked
        mModel->diagonalEnabled(false);
        break;
    case 2: // checked
        mModel->diagonalEnabled(true);
        break;
    default: //undefined
        break;
    }
}

AStarView::Status AStarView::status() const
{
    return mStatus;
}

void AStarView::setStatus(Status newStatus)
{
    if (mStatus == newStatus)
        return;
    mStatus = newStatus;
    emit statusChanged();

    // init machine's new state
    switch(status()){
    case Status::Running:
        ui->runBtn->setText("Pause");
        launchComputations();
        break;
    case Status::Pause:
        ui->runBtn->setText("Play");
        stopComputations();
        break;
    case Status::Finished:
        ui->runBtn->setText("Play");
    default:
        break;
    }
}

void AStarView::onStatusChanged()
{
    // enables / disables config controls
    bool isEnabled = status() == Status::Config;
    QVector<QWidget*> confWidgets {
        ui->startBtn,
        ui->stopBtn,
        ui->deleteObstacleBtn,
        ui->heightSlider,
        ui->widthSlider,
        ui->heuristicCombo,
        ui->diagonalBox
    };
    std::for_each(confWidgets.begin(), confWidgets.end(), [isEnabled](auto widgetPtr){widgetPtr->setEnabled(isEnabled);});
}

void AStarView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    resizeGrid();
}
