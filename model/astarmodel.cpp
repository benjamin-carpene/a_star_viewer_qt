#include "astarmodel.h"
#include "heuristics.hpp"

#include <QColor>
#include <QBrush>
#include <algorithm>
#include <memory>

// plain function
bool isInsidePointSet(const a_star::PointSet& ps, a_star::Vector2D point){
    return std::find(ps.begin(), ps.end(), point) != ps.end();
}

// Ctor
AStarModel::AStarModel(QObject *parent)
    :QAbstractTableModel{parent}, mDataModel{{30, 20}}, mState{&mDataModel.getState()}
{
    mDataModel.setFromPoint({0,0});
    mDataModel.setToPoint({10,10});
}

AStarModel::TypeOfCell AStarModel::getTypeOfCell(const QModelIndex &index) const
{
    a_star::Vector2D point(index.column(), index.row());
    auto snap = mDataModel.computeSnapShot();

    if(point == snap.from) //the order of ifs matters here
        return TypeOfCell::fromCell;
    else if(point == snap.to)
        return TypeOfCell::toCell;
    else if(isInsidePointSet(snap.currentPath, point))
        return TypeOfCell::pathCell;
    else if(isInsidePointSet(snap.seenNodes, point))
        return TypeOfCell::seenCell;
    else if(isInsidePointSet(snap.obstacles, point))
        return TypeOfCell::obstacleCell;

    return TypeOfCell::voidCell;
}

int AStarModel::rowCount(const QModelIndex &parent) const
{
    return mDataModel.getMapSize().y;
}

int AStarModel::columnCount(const QModelIndex &parent) const
{
    return mDataModel.getMapSize().x;
}

QVariant AStarModel::data(const QModelIndex &index, int role) const
{
    switch(role){
    case Qt::BackgroundRole:{
        auto cellType = getTypeOfCell(index);
        QColor color;

        switch(cellType){
        case TypeOfCell::voidCell:
            color = Qt::white;
            break;
        case TypeOfCell::seenCell:
            color = Qt::gray;
            break;
        case TypeOfCell::obstacleCell:
            color = Qt::red;
            break;
        case TypeOfCell::pathCell:
            color = Qt::green;
            break;
        case TypeOfCell::fromCell:
            color = Qt::blue;
            break;
        case TypeOfCell::toCell:
            color = Qt::darkBlue;
            break;
        }

        return static_cast<QVariant>(QBrush(color));
        break;
    }
    case AStarRoles::cellTypeRole:
        return getTypeOfCell(index);
        break;
    }

    return QVariant();
}

bool AStarModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role == AStarRoles::cellTypeRole){
        TypeOfCell cellType = static_cast<TypeOfCell>(value.toInt());
        a_star::Vector2D point(index.column(), index.row());

        switch(cellType){
        case TypeOfCell::voidCell:
            mDataModel.removeObstacle(point);
            break;
        case TypeOfCell::obstacleCell:
            mDataModel.addObstacle(point);
            break;
        case TypeOfCell::fromCell:{
            mDataModel.removeObstacle(point); //maybe it was an obstacle
            a_star::Vector2D prev = mDataModel.getFromPoint();//handle the previous from cell
            mDataModel.setFromPoint(point); //actually change the data
            //up also the value that was previously from
            setData(index.sibling(prev.y, prev.x), TypeOfCell::voidCell, AStarRoles::cellTypeRole);
            break;
        }
        case TypeOfCell::toCell:{
            mDataModel.removeObstacle(point);
            a_star::Vector2D prev = mDataModel.getToPoint();
            mDataModel.setToPoint(point);
            setData(index.sibling(prev.y, prev.x), TypeOfCell::voidCell, AStarRoles::cellTypeRole);
            break;
        }
        default:
            break;
        }

        emit dataChanged(index, index, {AStarRoles::cellTypeRole});
        return true;
    }
    return false; // view can only set cellTypeRole
}

Qt::ItemFlags AStarModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

void AStarModel::changeSize(size_t width, size_t height)
{
    beginResetModel();
    mDataModel.setMapSize(a_star::Vector2D(width, height));
    endResetModel();
}

a_star::State AStarModel::compute()
{
    auto size = mDataModel.getMapSize();
    if(*mState == a_star::State::unstarted){
        mDataModel.init();
        mDataModel.step(); mDataModel.step(); // first 2 steps (first new cell)
    }
    else if(*mState == a_star::State::started){
        mDataModel.step();
    }
    emit dataChanged(index(0,0), index(size.y-1, size.x-1), {AStarRoles::cellTypeRole});

    return *mState;
}

void AStarModel::setStart(const QModelIndex &index)
{
    setData(index, TypeOfCell::fromCell, AStarRoles::cellTypeRole);
}

void AStarModel::setStop(const QModelIndex &index)
{
    setData(index, TypeOfCell::toCell, AStarRoles::cellTypeRole);
}

void AStarModel::toggleObstacle(const QModelIndex &index)
{
    a_star::Vector2D point(index.column(), index.row());
    if(mDataModel.isInsideObstacle(point))
        mDataModel.removeObstacle(point);
    else
        mDataModel.addObstacle(point);
    emit dataChanged(index, index, {AStarRoles::cellTypeRole});
}

void AStarModel::removeObstacles()
{
    mDataModel.clearObstacles();
    auto size = mDataModel.getMapSize();
    emit dataChanged(index(0,0), index(size.y-1, size.x-1), {AStarRoles::cellTypeRole});
}

void AStarModel::diagonalEnabled(bool diag)
{
    if(diag)
        mDataModel.enableDiagonal();
    else
        mDataModel.disableDiagonal();
}

void AStarModel::clear()
{
    mDataModel.reinitialize();
    auto size = mDataModel.getMapSize();
    emit dataChanged(index(0,0), index(size.y-1, size.x-1), {AStarRoles::cellTypeRole});
}

void AStarModel::setHeuristic(HeuristicType heuristicType)
{
    using namespace a_star::heuristics;

    std::unique_ptr<HeuristicInterface> heuristicPtr{nullptr};
    switch(heuristicType){
    case HeuristicType::Euclidian:
        heuristicPtr = std::make_unique<EuclidianHeuristic>();
        break;
    case HeuristicType::Manhattan:
        heuristicPtr = std::make_unique<ManhattanHeuristic>();
        break;
    case HeuristicType::Chebyshev:
        heuristicPtr = std::make_unique<ChebyshevHeuristic>();
        break;
    case HeuristicType::Octile:
        heuristicPtr = std::make_unique<OctileHeuristic>();
        break;
    }

    mDataModel.setHeuristic(std::move(heuristicPtr));
}
