#ifndef ASTARMODEL_H
#define ASTARMODEL_H

#include "path_finder.hpp"
#include "a_star.hpp"
#include <QAbstractTableModel>
#include <QVector>

class AStarModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum AStarRoles{
        cellTypeRole = Qt::UserRole + 1
    };
    enum TypeOfCell{
        voidCell = 1,
        seenCell,
        obstacleCell,
        pathCell,
        fromCell,
        toCell
    };
    enum HeuristicType{
        Euclidian = 0,
        Manhattan = 1,
        Chebyshev = 2,
        Octile = 3
    };
    inline static const QVector<HeuristicType> heuristics{
        HeuristicType::Euclidian,
        HeuristicType::Manhattan,
        HeuristicType::Chebyshev,
        HeuristicType::Octile
    };

    // in function of TypeOfCell data will return a different color for Qt::BackgroundRole
    explicit AStarModel(QObject *parent = nullptr);

    //read model
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    //write model
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override; //Qt::ItemIsEditable

    // Utilitary
    TypeOfCell getTypeOfCell(const QModelIndex &index) const;

    // specific to A*
    void changeSize(size_t width, size_t height); //size_x, size_y
    a_star::State compute();
    void setStart(const QModelIndex &index);
    void setStop(const QModelIndex &index);
    void toggleObstacle(const QModelIndex &index);
    void removeObstacles();
    void diagonalEnabled(bool diag);
    void clear();
    void setHeuristic(HeuristicType heuristicType);


private:
    a_star::PathFinder mDataModel;
    const a_star::State* mState{&mDataModel.getState()};
};

#endif // ASTARMODEL_H
