#include "mapwidget.h"
#include <QPainter>
#include <iostream>

class MapWidget;
class QPainter;

ImagesMap MapWidget::images = ImagesMap();

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(QSize(MAP_WIDTH, MAP_WIDTH));

    if (MapWidget::images.size() == 0) {
        images["sea.png"] = QImage(":/images/sea.png");
        images["sea_s.png"] = QImage(":/images/sea_s.png");
        images["sea_t.png"] = QImage(":/images/sea_t.png");
        images["ship.png"] = QImage(":/images/ship.png");
        images["ship_s.png"] = QImage(":/images/ship_s.png");
    }

    int i, j;
    for (i = 0; i < MAX_CELLS; ++i)
        for (j = 0; j < MAX_CELLS; ++j)
            cells[i][j] = Water;

    setState(View);
    setMouseTracking(true);
}

MapWidget::~MapWidget()
{
}

void MapWidget::setState(MapState state)
{
    switch (state) {
    case Edit:
        rotation = Qt::Vertical;
        shipSize = MAX_DESKS;
        placedTimes = 0;
        possibleShip = qMakePair(-1, -1);
        break;
    case View:
        break;
    case Turn:
        break;
    }
    this->state = state;
    update();
}

bool MapWidget::possiblePlaceShip(int x, int y)
{
    int i = y / CELL_WIDTH, j = x / CELL_WIDTH;

    bool outScope = !(
        (
            rotation == Qt::Horizontal &&
            j+shipSize <= MAX_CELLS
        ) || (
            rotation == Qt::Vertical &&
            i+shipSize <= MAX_CELLS
        )
    );

    if (outScope)
        return false;

    if (rotation == Qt::Horizontal) {
        if (j-1 > 0 && cells[i][j-1] != Water)
            return false;
        else if (j+shipSize < MAX_CELLS && cells[i][j+shipSize] != Water)
            return false;
        for (int k = j-1; k < shipSize+j+1; ++k) {
            if (k < 0 || k >= MAX_CELLS)
                continue;
            if (cells[i][k] != Water)
                return false;
            if (i-1 >= 0 && cells[i-1][k] != Water)
                return false;
            else if (i+1 < MAX_CELLS && cells[i+1][k] != Water)
                return false;
        }
    } else {
        if (i-1 > 0 && cells[i-1][j] != Water)
            return false;
        else if (i+shipSize < MAX_CELLS && cells[i+shipSize][j] != Water)
            return false;
        for (int k = i-1; k < shipSize+i+1; ++k) {
            if (k < 0 || k >= MAX_CELLS)
                continue;
            if (cells[k][j] != Water)
                return false;
            if (j-1 >= 0 && cells[k][j-1] != Water)
                return false;
            else if (j+1 < MAX_CELLS && cells[k][j+1] != Water)
                return false;
        }
    }

    return true;
}

void MapWidget::clear()
{
    int i, j;
    for (i = 0; i < MAX_CELLS; ++i)
        for (j = 0; j < MAX_CELLS; ++j)
            cells[i][j] = Water;
}

void MapWidget::setPossibleShip(int x, int y)
{
    static int lasti = -1, lastj = -1;

    if (!possiblePlaceShip(x, y)) {
        possibleShip.first = -1; possibleShip.second = -1;
        return;
    }

    int i = y / CELL_WIDTH, j = x / CELL_WIDTH;

    possibleShip.first = i;
    possibleShip.second = j;

    if (i != lasti || j != lastj)
        update();
}

void MapWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::blue);
    int i, j;
    for (i = 0; i < MAX_CELLS; ++i) {
        for (j = 0; j < MAX_CELLS; ++j) {
            int x = j*CELL_WIDTH, y = i*CELL_WIDTH;
            switch (cells[i][j]) {
            case Water:
                switch (state) {
                case Turn:
                    if (mouseInCell(x, y))
                        painter.drawImage(x, y, images["sea_t.png"]);
                    else
                        painter.drawImage(x, y, images["sea.png"]);
                    break;
                case View:
                    painter.drawImage(x, y, images["sea.png"]);
                    break;
                case Edit:
                    painter.drawImage(x, y, images["sea.png"]);
                    if (mouseInCell(x, y))
                        setPossibleShip(x, y);
                    break;
                }
                break;
            case WaterStriked:
                painter.drawImage(x, y, images["sea_s.png"]);
                break;
            case Ship:
                painter.drawImage(x, y, images["ship.png"]);
                break;
            case ShipStriked:
                painter.drawImage(x, y, images["ship_s.png"]);
                break;
            }
            painter.drawRect(x, y, CELL_WIDTH, CELL_WIDTH);
        }
    }

    if (state == Edit && possibleShip.first != -1) {
        painter.setPen(Qt::red);
        if (rotation == Qt::Vertical)
            painter.drawRect(possibleShip.second * CELL_WIDTH + 5,
                             possibleShip.first * CELL_WIDTH + 5,
                             CELL_WIDTH - 10,
                             CELL_WIDTH * shipSize - 10);
        else
            painter.drawRect(possibleShip.second * CELL_WIDTH + 5,
                             possibleShip.first * CELL_WIDTH + 5,
                             CELL_WIDTH * shipSize - 10,
                             CELL_WIDTH - 10);
        painter.setPen(Qt::blue);
    }
}

void MapWidget::placeShip(int x, int y)
{
    int i = y / CELL_WIDTH, j = x / CELL_WIDTH, k;

    if (rotation == Qt::Horizontal) {
        for (k = 0; k < shipSize; ++k)
            cells[i][k+j] = Ship;
    } else {
        for (k = 0; k < shipSize; ++k)
            cells[k+i][j] = Ship;
    }

    placedTimes++;
    if (MAX_DESKS+1-shipSize == placedTimes) {
        shipSize--;
        placedTimes = 0;
    }

    static int t = 0;
    if (shipSize == 0) {
        setState(View);
        emit eventOccured(tr("%1 is ready to play")
            .arg(Environ::instance()["user"]));
        t = 0;
    } else {
        emit eventOccured(tr("%1 places the %2 ship")
            .arg(Environ::instance()["user"])
            .arg(++t));
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    mouse = event->pos();
    update();
}

void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (state == Edit) {
        if (event->button() == Qt::RightButton ||
            event->button() == Qt::MidButton
        ) {
            rotation = rotation == Qt::Horizontal
                                   ? Qt::Vertical : Qt::Horizontal;
        } else if (event->button() == Qt::LeftButton) {
            if (possiblePlaceShip(event->x(), event->y()))
                placeShip(event->x(), event->y());
        }
    }
}

inline bool MapWidget::mouseInCell(int x, int y)
{
    return mouse.x() >= x && mouse.x() < x+CELL_WIDTH && \
           mouse.y() >= y && mouse.y() < y+CELL_WIDTH;
}