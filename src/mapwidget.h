#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QImage>
#include <QMap>
#include <QString>
#include <QPoint>
#include <QPair>
#include <QMouseEvent>
#include "singleton.h"

class QWidget;
class QImage;
class QString;
class QPoint;
class QMouseEvent;
typedef QMap<QString, QImage> ImagesMap;
enum CellState {Water, WaterStriked, Ship, ShipStriked};
enum MapState {Edit, View, Turn};

static const int MAX_CELLS = 10;
static const int CELL_WIDTH = 32;
static const int MAP_WIDTH = MAX_CELLS * CELL_WIDTH;
static const int MAX_DESKS = 4;

class MapWidget : public QWidget
{
    Q_OBJECT

    static ImagesMap images;
    CellState cells[MAX_CELLS][MAX_CELLS];
    MapState state;
    QPoint mouse;

    Qt::Orientation rotation;
    int shipSize, placedTimes;

    inline bool mouseInCell(int x, int y);
    void placeShip(int x, int y);
    bool possiblePlaceShip(int x, int y);
    void setPossibleShip(int x, int y);
    QPair<int, int> possibleShip;
public:
    MapWidget(QWidget *parent = 0);
    ~MapWidget();
    void setState(MapState state);
    void clear();

signals:
    void eventOccured(const QString &);

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif
