#include "NavigationWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <QPolygonF>
#include <QTransform>
#include <QDebug>
#include <limits>
NavigationWidget::NavigationWidget(
    QWidget *parent
)
    : QWidget(parent),
      mRobotX(0.0f),
      mRobotY(0.0f),
      mRobotYaw(0.0f),
      mPointerX(0.0f),
      mPointerY(0.0f),
      mPointerValid(false),
      mZMinIndex(0),
      mZMaxIndex(0),
      mMinX(0.0f),
      mMaxX(0.0f),
      mMinY(0.0f),
      mMaxY(0.0f),
      mSelecting(false),
      mSeleccionandoContorno(false),
      mContourSeed(0.0, 0.0),
      mContourSeedValid(false),
      mSelectionStart(),
      mSelectionEnd(),
      mSeleccionandoPuntoB(false),
      mPuntoB(0.0, 0.0),
      mPuntoBValido(false),
      mRutaActiva(false),
      mToleranciaRuta(mRobotScale / 3.0f),
      mToleranciaDestino(mRobotScale / 13.0f),
      mRobotStepActivo(false),
      mPasosMovimiento(0),
      mMovimientoRutaActivo(false),
      mIndiceRutaActual(1),
      mEsperandoConfirmacionDestino(false)
{
    setMouseTracking(true);
    setAutoFillBackground(true);
}


NavigationWidget::~NavigationWidget()
{
}


void NavigationWidget::setRobotPose(
    float x,
    float y,
    float yaw
)
{
    mRobotX = x;
    mRobotY = y;
    mRobotYaw = yaw;

    update();
}


void NavigationWidget::setMapPoints2D(
    const std::vector<QPointF> &points
)
{
    mMapPoints = points;

    mSelectedPoints.assign(
        mMapPoints.size(),
        false
    );

    mContour.clear();

    if(!mMapPoints.empty())
    {
        mMinX =
            static_cast<float>(
                mMapPoints[0].x()
            );

        mMaxX =
            static_cast<float>(
                mMapPoints[0].x()
            );

        mMinY =
            static_cast<float>(
                mMapPoints[0].y()
            );

        mMaxY =
            static_cast<float>(
                mMapPoints[0].y()
            );
        for(
            const QPointF &point :
            mMapPoints
        )
        {
            mMinX =
                std::min(
                    mMinX,
                    static_cast<float>(
                        point.x()
                    )
                );

            mMaxX =
                std::max(
                    mMaxX,
                    static_cast<float>(
                        point.x()
                    )
                );

            mMinY =
                std::min(
                    mMinY,
                    static_cast<float>(
                        point.y()
                    )
                );

            mMaxY =
                std::max(
                    mMaxY,
                    static_cast<float>(
                        point.y()
                    )
                );
        }

        rebuildContour();
    }

    update();
}
void NavigationWidget::setMapPointsZ(
    const std::vector<float> &zValues
)
{
    mMapPointsZ =
        zValues;

    mZLevels.clear();

    for(float z : mMapPointsZ)
    {
        if(!std::isfinite(z))
            continue;

        mZLevels.push_back(
            z
        );
    }

    std::sort(
        mZLevels.begin(),
        mZLevels.end()
    );

    mZLevels.erase(
        std::unique(
            mZLevels.begin(),
            mZLevels.end()
        ),
        mZLevels.end()
    );

    if(mZLevels.empty())
    {
        mZMinIndex = 0;
        mZMaxIndex = 0;
        return;
    }

    mZMinIndex = 0;

    mZMaxIndex =
        static_cast<int>(
            mZLevels.size()
        ) - 1;

    actualizarFiltroZ();

    update();
}
void NavigationWidget::actualizarFiltroZ()
{
    if(mZLevels.empty())
        return;

    if(mZMinIndex < 0)
        mZMinIndex = 0;

    if(
        mZMinIndex >=
        static_cast<int>(
            mZLevels.size()
        )
    )
    {
        mZMinIndex =
            static_cast<int>(
                mZLevels.size()
            ) - 1;
    }

    if(mZMaxIndex < 0)
        mZMaxIndex = 0;

    if(
        mZMaxIndex >=
        static_cast<int>(
            mZLevels.size()
        )
    )
    {
        mZMaxIndex =
            static_cast<int>(
                mZLevels.size()
            ) - 1;
    }

    if(
        mZMinIndex >
        mZMaxIndex
    )
    {
        mZMinIndex =
            mZMaxIndex;
    }
}
void NavigationWidget::cambiarZMinimoPublico(
    int direccion
)
{
    cambiarZMinimo(
        direccion
    );
}

void NavigationWidget::cambiarZMaximoPublico(
    int direccion
)
{
    cambiarZMaximo(
        direccion
    );
}

float NavigationWidget::getZMinimo() const
{
    if(mZLevels.empty())
        return 0.0f;

    return mZLevels[mZMinIndex];
}

float NavigationWidget::getZMaximo() const
{
    if(mZLevels.empty())
        return 0.0f;

    return mZLevels[mZMaxIndex];
}
void NavigationWidget::cambiarZMinimo(
    int direccion
)
{
    if(mZLevels.empty())
        return;

    const int nuevoIndice =
        mZMinIndex +
        direccion;

    if(
        nuevoIndice < 0 ||
        nuevoIndice >=
        static_cast<int>(
            mZLevels.size()
        )
    )
    {
        return;
    }

    mZMinIndex =
        nuevoIndice;

    if(
        mZMinIndex >
        mZMaxIndex
    )
    {
        mZMaxIndex =
            mZMinIndex;
    }

    actualizarFiltroZ();

    emit emitirFiltroZ();
    update();
}
void NavigationWidget::cambiarZMaximo(
    int direccion
)
{
    if(mZLevels.empty())
        return;

    const int nuevoIndice =
        mZMaxIndex +
        direccion;

    if(
        nuevoIndice < 0 ||
        nuevoIndice >=
        static_cast<int>(
            mZLevels.size()
        )
    )
    {
        return;
    }

    mZMaxIndex =
        nuevoIndice;

    if(
        mZMaxIndex <
        mZMinIndex
    )
    {
        mZMinIndex =
            mZMaxIndex;
    }

    actualizarFiltroZ();

    emit emitirFiltroZ();
    update();
}
void NavigationWidget::emitirFiltroZ()
{
    if(mZLevels.empty())
        return;

    qDebug()
        << "[Z FILTER]"
        << "MIN:"
        << mZLevels[mZMinIndex]
        << "MAX:"
        << mZLevels[mZMaxIndex];
}
QPointF NavigationWidget::worldToScreen(
    const QPointF &point
) const
{
    if(mMapPoints.empty())
        return QPointF();

    const float margin = 60.0f;

    const float mapWidth =
        std::max(
            mMaxX - mMinX,
            0.001f
        );

    const float mapHeight =
        std::max(
            mMaxY - mMinY,
            0.001f
        );

    const float availableWidth =
        std::max(
            static_cast<float>(width()) -
            2.0f * margin,
            1.0f
        );

    const float availableHeight =
        std::max(
            static_cast<float>(height()) -
            2.0f * margin,
            1.0f
        );

    const float scale =
        std::min(
            availableWidth / mapWidth,
            availableHeight / mapHeight
        );

    const float screenX =
        margin +
        static_cast<float>(
            (point.x() - mMinX) * scale
        );

    const float screenY =
        height() -
        margin -
        static_cast<float>(
            (point.y() - mMinY) * scale
        );

    return QPointF(
        screenX,
        screenY
    );
}


QPointF NavigationWidget::screenToWorld(
    const QPointF &point
) const
{
    if(mMapPoints.empty())
        return QPointF();

    const float margin = 60.0f;

    const float mapWidth =
        std::max(
            mMaxX - mMinX,
            0.001f
        );

    const float mapHeight =
        std::max(
            mMaxY - mMinY,
            0.001f
        );

    const float availableWidth =
        std::max(
            static_cast<float>(width()) -
            2.0f * margin,
            1.0f
        );

    const float availableHeight =
        std::max(
            static_cast<float>(height()) -
            2.0f * margin,
            1.0f
        );

    const float scale =
        std::min(
            availableWidth / mapWidth,
            availableHeight / mapHeight
        );

    const float worldX =
        mMinX +
        static_cast<float>(
            (point.x() - margin) / scale
        );

    const float worldY =
        mMinY +
        static_cast<float>(
            (height() - margin - point.y()) /
            scale
        );

    return QPointF(
        worldX,
        worldY
    );
}


void NavigationWidget::mousePressEvent(
    QMouseEvent *event
)
{
    if(
        event->button() ==
        Qt::LeftButton &&
        mSeleccionandoPuntoB
    )
    {
        mPuntoB =
            screenToWorld(
                event->pos()
            );

        mPuntoBValido = true;

        mSeleccionandoPuntoB = false;

        setCursor(Qt::ArrowCursor);

        qDebug()
            << "[POINT B] Selected:"
            << mPuntoB.x()
            << mPuntoB.y();

        emit puntoBSeleccionado(
            static_cast<float>(mPuntoB.x()),
            static_cast<float>(mPuntoB.y())
        );

        update();

        return;
    }
    if(
        event->button() ==
        Qt::LeftButton &&
        mSeleccionandoContorno
    )
    {
        mContourSeed =
            screenToWorld(
                event->pos()
            );

        mContourSeedValid = true;

        mSeleccionandoContorno = false;

        setCursor(
            Qt::ArrowCursor
        );

        qDebug()
            << "[CONTOUR] Area selected:"
            << mContourSeed.x()
            << mContourSeed.y();
    
        rebuildContour();

        update();

        return;
    }
    if(
        event->button() !=
        Qt::LeftButton
    )
    {
        QWidget::mousePressEvent(event);
        return;
    }

    if(mMapPoints.empty())
        return;

    mSelecting = true;

    mSelectionStart =
        event->pos();

    mSelectionEnd =
        event->pos();

    update();
}


void NavigationWidget::mouseMoveEvent(
    QMouseEvent *event
)
{
    if(!mMapPoints.empty())
    {
        QPointF world =
            screenToWorld(
                event->pos()
            );

        mPointerX =
            static_cast<float>(
                world.x()
            );

        mPointerY =
            static_cast<float>(
                world.y()
            );

        mPointerValid = true;

        emit pointerPositionChanged(
            mPointerX,
            mPointerY
        );

        if(mSelecting)
        {
            mSelectionEnd =
                event->pos();
        }

        update();
    }

    QWidget::mouseMoveEvent(event);
}


void NavigationWidget::mouseReleaseEvent(
    QMouseEvent *event
)
{
    if(
        event->button() !=
        Qt::LeftButton
    )
    {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    if(!mSelecting)
        return;

    mSelectionEnd =
        event->pos();

    mSelecting = false;

    const QRectF selectionRect =
        QRectF(
            mSelectionStart,
            mSelectionEnd
        ).normalized();

    const float minimumDragSize = 5.0f;

    const bool isDrag =
        selectionRect.width() >=
            minimumDragSize ||
        selectionRect.height() >=
            minimumDragSize;

    if(isDrag)
    {
        for(size_t i = 0;
            i < mMapPoints.size();
            ++i)
        {
            const QPointF screen =
                worldToScreen(
                    mMapPoints[i]
                );

            if(
                selectionRect.contains(
                    screen
                )
            )
            {
                mSelectedPoints[i] = true;
            }
        }

        emitSelectionChanged();
    }
    else
    {
        const QPointF clickPosition =
            event->pos();

        const float selectionRadius =
            8.0f;

        int nearestIndex = -1;

        float nearestDistanceSquared =
            selectionRadius *
            selectionRadius;

        for(size_t i = 0;
            i < mMapPoints.size();
            ++i)
        {
            const QPointF screen =
                worldToScreen(
                    mMapPoints[i]
                );

            const float dx =
                static_cast<float>(
                    screen.x() -
                    clickPosition.x()
                );

            const float dy =
                static_cast<float>(
                    screen.y() -
                    clickPosition.y()
                );

            const float distanceSquared =
                dx * dx +
                dy * dy;

            if(
                distanceSquared <=
                nearestDistanceSquared
            )
            {
                nearestDistanceSquared =
                    distanceSquared;

                nearestIndex =
                    static_cast<int>(i);
            }
        }

        if(nearestIndex >= 0)
        {
            mSelectedPoints[
                nearestIndex
            ] =
                !mSelectedPoints[
                    nearestIndex
                ];

            emitSelectionChanged();
        }
    }

    update();

    QWidget::mouseReleaseEvent(event);
}


void NavigationWidget::emitSelectionChanged()
{
    std::vector<int> indices;

    for(size_t i = 0;
        i < mSelectedPoints.size();
        ++i)
    {
        if(mSelectedPoints[i])
        {
            indices.push_back(
                static_cast<int>(i)
            );
        }
    }

    emit pointSelectionChanged(
        indices
    );
}
void NavigationWidget::activarSeleccionContorno()
{
    mSeleccionandoContorno = true;

    setCursor(
        Qt::CrossCursor
    );

    update();
}
void NavigationWidget::rebuildContour()
{
    mContour.clear();

    if(!mContourSeedValid)
        return;

    if(mMapPoints.size() < 3)
        return;

    /*
     * =========================================================
     * RADIAL CONTOUR FROM THE SELECTED POINT
     *
     * The click represents the interior of the empty zone.
     *
     * From that point we cast many rays in all
     * directions. Each ray looks for the first MapPoint that
     * appears in that direction.
     *
     * The points found are sorted by angle and
     * connected with straight segments.
     * =========================================================
     */

    const int numRays = 360;

    /*
     * Maximum distance a ray can travel.
     *
     * It is computed from the total size of the map to avoid
     * depending on a fixed scale.
     */
    const float mapWidth =
        std::max(
            mMaxX - mMinX,
            0.001f
        );

    const float mapHeight =
        std::max(
            mMaxY - mMinY,
            0.001f
        );

    const float maxDistance =
        std::sqrt(
            mapWidth * mapWidth +
            mapHeight * mapHeight
        );

    /*
     * Angular tolerance.
     *
     * A MapPoint does not have to lie exactly on the ray.
     * It is accepted if it is close enough to it.
     */
    const float angularTolerance =
        0.025f;

    /*
     * Minimum distance from the center.
     *
     * Prevents selecting points that are practically on top
     * of the click.
     */
    const float minimumDistance =
        std::max(
            std::min(
                mapWidth,
                mapHeight
            ) * 0.005f,
            0.001f
        );

    struct Candidate
    {
        int index;
        float distance;
    };

    std::vector<Candidate> candidates;

    candidates.reserve(
        numRays
    );

    /*
     * =========================================================
     * FIND THE FIRST MAPPOINT FOR EACH DIRECTION
     * =========================================================
     */

    for(
        int ray = 0;
        ray < numRays;
        ++ray
    )
    {
        const float angle =
            2.0f *
            static_cast<float>(M_PI) *
            static_cast<float>(ray) /
            static_cast<float>(numRays);

        const float directionX =
            std::cos(angle);

        const float directionY =
            std::sin(angle);

        int bestIndex = -1;

        float bestDistance =
            maxDistance;

        float bestPerpendicularDistance =
            std::numeric_limits<float>::max();

        for(
            int i = 0;
            i <
            static_cast<int>(
                mMapPoints.size()
            );
            ++i
        )
        {
            const QPointF &point =
                mMapPoints[i];

            const float dx =
                static_cast<float>(
                    point.x() -
                    mContourSeed.x()
                );

            const float dy =
                static_cast<float>(
                    point.y() -
                    mContourSeed.y()
                );

            const float distance =
                std::sqrt(
                    dx * dx +
                    dy * dy
                );

            if(
                distance <
                minimumDistance
            )
            {
                continue;
            }

            if(
                distance >
                maxDistance
            )
            {
                continue;
            }

            /*
             * Projection of the point onto the ray.
             *
             * If it is negative, the point is behind the center.
             */
            const float projection =
                dx * directionX +
                dy * directionY;

            if(projection <= 0.0f)
                continue;

            /*
             * Perpendicular distance between the MapPoint
             * and the ray.
             */
            const float perpendicularDistance =
                std::fabs(
                    dx * directionY -
                    dy * directionX
                );

            /*
             * The tolerance grows with distance.
             *
             * This allows detecting points even when the
             * MapPoints are not exactly aligned with the ray.
             */
            const float allowedDistance =
                std::max(
                    angularTolerance * distance,
                    0.003f
                );

            if(
                perpendicularDistance >
                allowedDistance
            )
            {
                continue;
            }

            /*
             * We want the point closest to the center along this ray.
             */
            if(
                distance < bestDistance
            )
            {
                bestDistance =
                    distance;

                bestPerpendicularDistance =
                    perpendicularDistance;

                bestIndex =
                    i;
            }
            else if(
                std::fabs(
                    distance -
                    bestDistance
                ) < 0.001f &&
                perpendicularDistance <
                bestPerpendicularDistance
            )
            {
                bestPerpendicularDistance =
                    perpendicularDistance;

                bestIndex =
                    i;
            }
        }

        if(bestIndex >= 0)
        {
            candidates.push_back(
                {
                    bestIndex,
                    bestDistance
                }
            );
        }
    }

    /*
     * =========================================================
     * REMOVE DUPLICATES
     *
     * Many rays can find the same MapPoint.
     * We keep each MapPoint only once.
     * =========================================================
     */

    std::set<int> uniqueIndices;

    std::vector<int> contourIndices;

    for(
        const Candidate &candidate :
        candidates
    )
    {
        if(
            uniqueIndices.insert(
                candidate.index
            ).second
        )
        {
            contourIndices.push_back(
                candidate.index
            );
        }
    }


    /*
     * =========================================================
     * SORT THE MAPPOINTS AROUND THE CLICK
     * =========================================================
     */

    std::sort(
        contourIndices.begin(),
        contourIndices.end(),
        [this](
            int a,
            int b
        )
        {
            const float angleA =
                std::atan2(
                    static_cast<float>(
                        mMapPoints[a].y() -
                        mContourSeed.y()
                    ),
                    static_cast<float>(
                        mMapPoints[a].x() -
                        mContourSeed.x()
                    )
                );

            const float angleB =
                std::atan2(
                    static_cast<float>(
                        mMapPoints[b].y() -
                        mContourSeed.y()
                    ),
                    static_cast<float>(
                        mMapPoints[b].x() -
                        mContourSeed.x()
                    )
                );

            return angleA < angleB;
        }
    );

    /*
     * =========================================================
     * BUILD THE CONTOUR
     *
     * The segments connect real MapPoints directly.
     * =========================================================
     */

    for(
        int index :
        contourIndices
    )
    {
        if(
            index < 0 ||
            index >=
            static_cast<int>(
                mMapPoints.size()
            )
        )
        {
            continue;
        }

        mContour.push_back(
            mMapPoints[index]
        );
    }

     /*
     * paintEvent draws the consecutive segments.
     * Since they are sorted by angle, the last one connects
     * back to the first and closes the contour.
     */
}
void NavigationWidget::paintEvent(
    QPaintEvent *event
)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setRenderHint(
        QPainter::Antialiasing,
        true
    );

    painter.fillRect(
        rect(),
        Qt::black
    );

    painter.setPen(Qt::white);

    painter.drawText(
        20,
        30,
        "NAVIGATION"
    );

    if(mMapPoints.empty())
    {
        painter.drawText(
            rect(),
            Qt::AlignCenter,
            "No map points."
        );

        return;
    }

    /*
     * ---------------------------------------------------------
     * POINTS
     * ---------------------------------------------------------
     */

    for(size_t i = 0;
        i < mMapPoints.size();
        ++i
    )
    {
        if(
            mMapPointsZ.size() ==
            mMapPoints.size() &&
            !mZLevels.empty()
        )
        {
            const float z =
                mMapPointsZ[i];

            if(
                z < mZLevels[mZMinIndex] ||
                z > mZLevels[mZMaxIndex]
            )
            {
                continue;
            }
        }   

        if(
            i < mSelectedPoints.size() &&
            mSelectedPoints[i]
        )
        {
            painter.setPen(Qt::red);
        }
        else
        {
            painter.setPen(Qt::green);
        }

        const QPointF screen =
            worldToScreen(
                mMapPoints[i]
            );

        painter.drawEllipse(
            screen,
            3.0,
            3.0
        );
    }
    
    /*
     * ---------------------------------------------------------
     * B POINT
     * ---------------------------------------------------------
     */
    if(mPuntoBValido)
    {
        const QPointF screenB =
            worldToScreen(
                mPuntoB
            );

        painter.setPen(
            QPen(
                Qt::yellow,
                3
            )
        );

        painter.setBrush(
            Qt::red
        );

        painter.drawEllipse(
            screenB,
            7.0,
            7.0
        );

        painter.setPen(
            QPen(
                Qt::white,
                2
            )
        );    

        QFont font =
            painter.font();

        font.setBold(true);
        font.setPointSize(12);

        painter.setFont(font);

        painter.drawText(
            screenB +
            QPointF(
                10.0,
                -10.0
            ),
            "B"
        );
    }
    /*
     * ---------------------------------------------------------
     * ROUTE
     * ---------------------------------------------------------
     */
    if(mRutaActiva && mRuta.size() >= 2)
    {
        QPen rutaPen(
            Qt::cyan,
            3,
            Qt::DashLine
        );

        painter.setPen(rutaPen);
        painter.setBrush(Qt::NoBrush);

        QPolygonF rutaPolygon;

        for(const QPointF &punto : mRuta)
        {
            rutaPolygon.append(
                worldToScreen(punto)
            );
        }

        painter.drawPolyline(
            rutaPolygon
        );
    }

    /*
     * ---------------------------------------------------------
     * INNER CONTOUR
     * ---------------------------------------------------------
     */

    if(mContour.size() >= 3)
    {
        QPen contourPen(
            Qt::blue
        );

        contourPen.setWidth(2);

        painter.setPen(
            contourPen
        );

        painter.setBrush(
            Qt::NoBrush
        );

        QPolygonF polygon;

        for(
            const QPointF &point :
            mContour
        )
        {
            polygon.append(
                worldToScreen(point)
            );
        }

        painter.drawPolygon(
            polygon
        );
    }

    /*
     * ---------------------------------------------------------
     * POINTER
     * ---------------------------------------------------------
     */

    if(mPointerValid)
    {
        const QPointF pointerScreen =
            worldToScreen(
                QPointF(
                    mPointerX,
                    mPointerY
                )
            );

        QPen pointerPen(
            Qt::red
        );

        pointerPen.setWidth(2);

        painter.setPen(
            pointerPen
        );

        painter.drawLine(
            pointerScreen.x() - 8,
            pointerScreen.y(),
            pointerScreen.x() + 8,
            pointerScreen.y()
        );

        painter.drawLine(
            pointerScreen.x(),
            pointerScreen.y() - 8,
            pointerScreen.x(),
            pointerScreen.y() + 8
        );
    }

    /*
     * ---------------------------------------------------------
     * SELECTION RECTANGLE
     * ---------------------------------------------------------
     */

    if(mSelecting)
    {
        QRectF selectionRect =
            QRectF(
                mSelectionStart,
                mSelectionEnd
            ).normalized();

        QPen selectionPen(
            Qt::red
        );

        selectionPen.setWidth(1);
        selectionPen.setStyle(
            Qt::DashLine
        );

        painter.setPen(
            selectionPen
        );

        painter.setBrush(
            Qt::NoBrush
        );

        painter.drawRect(
            selectionRect
        );
    }
// =========================================================
// ROBOT
// =========================================================

QPointF robot =
    worldToScreen(
        QPointF(
            mRobotX,
            mRobotY
        )
    );

const float length = 30.0f;
const float headLength = 10.0f;
const float headWidth = 7.0f;

QPolygonF arrow;

arrow
    << QPointF(
        robot.x() + length,
        robot.y()
    )
    << QPointF(
        robot.x() + length - headLength,
        robot.y() - headWidth
    )
    << QPointF(
        robot.x() + length - headLength,
        robot.y() + headWidth
    );

QTransform transform;

transform.translate(
    robot.x(),
    robot.y()
);

transform.rotate(
    -mRobotYaw * 180.0 / M_PI
);

transform.translate(
    -robot.x(),
    -robot.y()
);

arrow =
    transform.map(
        arrow
    );

// ROBOT STRUCTURE
//
// All dimensions are in map coordinates.
// The robot point remains as the reference.
// The front of the robot corresponds to +X before applying mRobotYaw.
// =================================================
// OVERALL ROBOT SCALE
// =================================================
//
// This is the only variable that needs to be changed
// to resize the entire robot.
// With 0.10f the current size is preserved exactly.
//

const float robotScale = mRobotScale;
const float robotWidth = mRobotWidth;
// =================================================
// CHASSIS
// =================================================

const float chassisLength = robotScale * 1.00f;
const float chassisWidth = robotScale * 0.80f;

// The robot point is NOT at the center.
// The chassis is offset:
// 0.03 backward
// 0.07 forward.

const float rearOffset = robotScale * 0.30f;

const float frontOffset = robotScale * 0.70f;

QPointF robotWorld(
    mRobotX,
    mRobotY
);

// =================================================
// ROTATION ACCORDING TO THE ROBOT ORIENTATION
// =================================================

const double cosYaw =
    std::cos(mRobotYaw);

const double sinYaw =
    std::sin(mRobotYaw);

auto rotarPunto =
    [&](const QPointF &punto)
    {
        const double dx =
            punto.x() - robotWorld.x();

        const double dy =
            punto.y() - robotWorld.y();

        return QPointF(
            robotWorld.x() +
            dx * cosYaw -
            dy * sinYaw,

            robotWorld.y() +
            dx * sinYaw +
            dy * cosYaw
        );
    };

// =================================================
// MAP → SCREEN POLYGON CONVERSION
// =================================================

auto mapaAPantalla =
    [&](const QPolygonF &polygon)
    {
        QPolygonF resultado;

        for(const QPointF &punto : polygon)
        {
            resultado.append(
                worldToScreen(punto)
            );
        }

        return resultado;
    };

// =================================================
// CREATE THE ROBOT'S LOCAL RECTANGLE
// =================================================

auto crearRectangulo =
    [&](float xMin,
        float xMax,
        float yMin,
        float yMax)
    {
        QPolygonF polygon;

        polygon
            << rotarPunto(
                QPointF(
                    robotWorld.x() + xMin,
                    robotWorld.y() + yMin
                )
            )

            << rotarPunto(
                QPointF(
                    robotWorld.x() + xMax,
                    robotWorld.y() + yMin
                )
            )

            << rotarPunto(
                QPointF(
                    robotWorld.x() + xMax,
                    robotWorld.y() + yMax
                )
            )

            << rotarPunto(
                QPointF(
                    robotWorld.x() + xMin,
                    robotWorld.y() + yMax
                )
            );

        return polygon;
    };

// =================================================
// WHITE CHASSIS
// =================================================

QPolygonF chassis =
    crearRectangulo(
        -rearOffset,
        frontOffset,
        -chassisWidth / 2.0f,
        chassisWidth / 2.0f
    );

painter.setBrush(
    Qt::white
);

painter.setPen(
    QPen(
        Qt::black,
        1
    )
);

painter.drawPolygon(
    mapaAPantalla(
        chassis
    )
);

// =================================================
// WHEELS
//
// LONGITUDINALLY:
//
// 0.02 | 0.025 | 0.01 | 0.025 | 0.02
//
// = 0.10
// =================================================

const float wheelLength = robotScale * 0.25f;

const float wheelGap = robotScale * 0.10f;

const float wheelEdgeMargin = robotScale * 0.20f;
// The two wheels occupy:
//
// 0.02 + 0.025 + 0.01 + 0.025 + 0.02
//
// The margins lie outside the wheels.

const float wheelRearStart =
    -rearOffset +
    wheelEdgeMargin;

const float wheelFrontStart =
    wheelRearStart +
    wheelLength +
    wheelGap;

// =================================================
// LATERAL POSITION OF THE WHEELS
// =================================================

const float wheelWidth = robotScale * 0.25f;
// Separation chassis wheels



const float wheelSideGap = 0.0f;

const float halfChassisWidth =
    chassisWidth / 2.0f;

const float leftWheelY =
    halfChassisWidth +
    wheelSideGap;

const float rightWheelY =
    -halfChassisWidth -
    wheelSideGap -
    wheelWidth;

// =================================================
// REAR LEFT WHEEL
// =================================================

QPolygonF rearLeftWheel =
    crearRectangulo(
        wheelRearStart,
        wheelRearStart + wheelLength,
        leftWheelY,
        leftWheelY + wheelWidth
    );

// =================================================
// FRONT LEFT WHEEL
// =================================================

QPolygonF frontLeftWheel =
    crearRectangulo(
        wheelFrontStart,
        wheelFrontStart + wheelLength,
        leftWheelY,
        leftWheelY + wheelWidth
    );

// =================================================
// REAR RIGHT WHEEL
// =================================================

QPolygonF rearRightWheel =
    crearRectangulo(
        wheelRearStart,
        wheelRearStart + wheelLength,
        rightWheelY,
        rightWheelY + wheelWidth
    );

// =================================================
// FRONT RIGHT WHEEL
// =================================================

QPolygonF frontRightWheel =
    crearRectangulo(
        wheelFrontStart,
        wheelFrontStart + wheelLength,
        rightWheelY,
        rightWheelY + wheelWidth
    );

// =================================================
// DRAW THE FOUR WHEELS
// =================================================

painter.setBrush(
    Qt::black
);

painter.setPen(
    QPen(
        Qt::gray,
        1
    )
);

painter.drawPolygon(
    mapaAPantalla(
        rearLeftWheel
    )
);

painter.drawPolygon(
    mapaAPantalla(
        frontLeftWheel
    )
);

painter.drawPolygon(
    mapaAPantalla(
        rearRightWheel
    )
);

painter.drawPolygon(
    mapaAPantalla(
        frontRightWheel
    )
);

// ================================================
// PART BETWEEN THE RED POINT AND THE CHASSIS
// ================================================

// From the red point to the rear edge
// of the chassis.
// No fill and with a black border.

const float barraLength = robotScale * 0.30f;

const float barraWidth = robotScale * 0.80f;

QPolygonF barraRobot =
    crearRectangulo(
        -barraLength,
        0.0f,
        -barraWidth / 2.0f,
        barraWidth / 2.0f
    );

painter.setBrush(
    Qt::NoBrush
);

painter.setPen(
    QPen(
        Qt::black,
        1
    )
);

painter.drawPolygon(
    mapaAPantalla(
        barraRobot
    )
);
// =================================================
// INNER BLACK RECTANGLE
// =================================================

const float piezaInteriorLength = robotScale * 0.15f;

const float piezaInteriorWidth = robotScale * 0.80f;

QPolygonF piezaInterior =
    crearRectangulo(
        0.0f,
        piezaInteriorLength,
        -piezaInteriorWidth / 2.0f,
        piezaInteriorWidth / 2.0f
    );

painter.setBrush(
    Qt::gray
);

painter.setPen(
    QPen(
        Qt::black,
        1
    )
);

painter.drawPolygon(
    mapaAPantalla(
        piezaInterior
    )
);
// =================================================
// SMALL RECTANGLE IN FRONT OF THE POINT
// =================================================

// The left edge starts exactly at the red point.

const float piezaFrontLength = robotScale * 0.25f;

const float piezaFrontWidth = robotScale * 0.20f;

QPolygonF piezaFront =
    crearRectangulo(
        0.0f,
        piezaFrontLength,
        -piezaFrontWidth / 2.0f,
        piezaFrontWidth / 2.0f
    );

painter.setBrush(
    Qt::black
);

painter.setPen(
    QPen(
        Qt::gray,
        1
    )
);

painter.drawPolygon(
    mapaAPantalla(
        piezaFront
    )
);

// ---------------------------------------------------------
// ORIENTATION TRIANGLE
// ---------------------------------------------------------

painter.setBrush(
    Qt::blue
);

painter.setPen(
    Qt::black
);

painter.drawPolygon(
    arrow
);

painter.setBrush(
    Qt::red
);

painter.drawEllipse(
    robot,
    3.0,
    3.0
);
}
void NavigationWidget::activarSeleccionPuntoB()
{
    mSeleccionandoPuntoB = true;

    setCursor(Qt::CrossCursor);

    qDebug()
        << "[POINT B] Selection enabled.";
} 
void NavigationWidget::calcularRuta()
{
    if(!mPuntoBValido)
    {
        qDebug()
            << "[ROUTE] Cannot calculate: invalid Point B.";

        return;
    }

    const QPointF puntoA(
        mRobotX,
        mRobotY
    );

    /*
     * =========================================================
     * 1. DIRECT ROUTE A → B
     * =========================================================
     */

    if(
        puntoDentroContorno(puntoA) &&
        puntoDentroContorno(mPuntoB) &&
        segmentoDentroContorno(
            puntoA,
            mPuntoB
        ) &&
        segmentoSeguroParaRobot(
            puntoA,
            mPuntoB
        )
    )
    {
        mRuta.clear();

        mRuta.push_back(
            puntoA
        );

        mRuta.push_back(
            mPuntoB
        );

        mRutaActiva = true;

        qDebug()
            << "[ROUTE] Direct route is safe.";

        update();

        return;
    }

    /*
     * =========================================================
     * 2. BUILD DETOUR POINTS
     *
     * Each contour vertex generates a point located toward
     * the interior, leaving room for the robot's entire
     * body.
     * =========================================================
     */

    if(mContour.size() < 3)
    {
        mRuta.clear();
        mRutaActiva = false;

        qDebug()
            << "[ROUTE] Not enough contour available.";

        update();

        return;
    }

    const float robotRadius =
        std::sqrt(
            mRobotScale * mRobotScale +
            mRobotWidth * mRobotWidth
        ) / 2.0f;

    const float margen =
        robotRadius * 1.20f;

    std::vector<QPointF> nodos;

    nodos.reserve(
        mContour.size() + 2
    );

    /*
     * Node 0 = current robot position.
     * Node 1 = Point B.
     */
    nodos.push_back(
        puntoA
    );

    nodos.push_back(
        mPuntoB
    );

    /*
     * Create one interior node for each contour vertex.
     */
    for(
        std::size_t i = 0;
        i < mContour.size();
        ++i
    )
    {
        const QPointF &vertice =
            mContour[i];

        const float dx =
            static_cast<float>(
                mContourSeed.x() -
                vertice.x()
            );

        const float dy =
            static_cast<float>(
                mContourSeed.y() -
                vertice.y()
            );

        const float longitud =
            std::sqrt(
                dx * dx +
                dy * dy
            );

        if(longitud <= 0.000001f)
            continue;

        const QPointF candidato(
            vertice.x() +
            dx / longitud * margen,

            vertice.y() +
            dy / longitud * margen
        );

        if(
            !puntoDentroContorno(
                candidato
            )
        )
        {
            continue;
        }

        nodos.push_back(
            candidato
        );
    }

    if(nodos.size() < 3)
    {
        mRuta.clear();
        mRutaActiva = false;

        qDebug()
            << "[ROUTE] Could not generate interior nodes.";

        update();

        return;
    }

    /*
     * =========================================================
     * 3. VISIBILITY GRAPH
     *
     * Two nodes can be connected only if:
     *
     *  - the entire segment stays inside the contour;
     *  - the robot's entire body keeps a safe distance
     *    from the contour.
     * =========================================================
     */

    const int cantidadNodos =
        static_cast<int>(
            nodos.size()
        );

    const float infinito =
        std::numeric_limits<float>::max();

    std::vector<float> distancia(
        cantidadNodos,
        infinito
    );

    std::vector<int> anterior(
        cantidadNodos,
        -1
    );

    std::vector<bool> visitado(
        cantidadNodos,
        false
    );

    distancia[0] = 0.0f;

    /*
     * =========================================================
     * 4. DIJKSTRA
     * =========================================================
     */

    for(
        int iteracion = 0;
        iteracion < cantidadNodos;
        ++iteracion
    )
    {
        int actual = -1;

        float mejorDistancia =
            infinito;

        for(
            int i = 0;
            i < cantidadNodos;
            ++i
        )
        {
            if(visitado[i])
                continue;

            if(distancia[i] < mejorDistancia)
            {
                mejorDistancia =
                    distancia[i];

                actual = i;
            }
        }

        if(actual < 0)
            break;

        visitado[actual] = true;

        if(actual == 1)
            break;

        for(
            int vecino = 0;
            vecino < cantidadNodos;
            ++vecino
        )
        {
            if(
                vecino == actual ||
                visitado[vecino]
            )
            {
                continue;
            }

            if(
                !segmentoDentroContorno(
                    nodos[actual],
                    nodos[vecino]
                )
            )
            {
                continue;
            }

            if(
                !segmentoSeguroParaRobot(
                    nodos[actual],
                    nodos[vecino]
                )
            )
            {
                continue;
            }

            const float dx =
                static_cast<float>(
                    nodos[vecino].x() -
                    nodos[actual].x()
                );

            const float dy =
                static_cast<float>(
                    nodos[vecino].y() -
                    nodos[actual].y()
                );

            const float longitud =
                std::sqrt(
                    dx * dx +
                    dy * dy
                );

            const float nuevaDistancia =
                distancia[actual] +
                longitud;

            if(
                nuevaDistancia <
                distancia[vecino]
            )
            {
                distancia[vecino] =
                    nuevaDistancia;

                anterior[vecino] =
                    actual;
            }
        }
    }

    /*
     * =========================================================
     * 5. RECONSTRUCT ROUTE
     * =========================================================
     */

    if(
        anterior[1] < 0
    )
    {
        mRuta.clear();
        mRutaActiva = false;

        qDebug()
            << "[ROUTE] No safe path to B exists.";

        emit avisoNavegacion(
            "Route unreachable with the robot's current dimensions."
        );
        update();

        return;
    }

    std::vector<QPointF> rutaInvertida;

    int nodoActual = 1;

    while(nodoActual >= 0)
    {
        rutaInvertida.push_back(
            nodos[nodoActual]
        );

        if(nodoActual == 0)
            break;

        nodoActual =
            anterior[nodoActual];
    }

    if(rutaInvertida.empty())
    {
        mRuta.clear();
        mRutaActiva = false;

        return;
    }

    std::reverse(
        rutaInvertida.begin(),
        rutaInvertida.end()
    );

    mRuta =
        rutaInvertida;

    mRutaActiva = true;

    qDebug()
        << "[ROUTE] Alternative route found."
        << "Points:"
        << mRuta.size();

    update();
}
float NavigationWidget::distanciaPuntoSegmento(
    const QPointF &punto,
    const QPointF &inicio,
    const QPointF &fin
) const
{
    const double dx =
        fin.x() - inicio.x();

    const double dy =
        fin.y() - inicio.y();

    const double longitud2 =
        dx * dx + dy * dy;

    if(longitud2 <= 1e-12)
    {
        const double ex =
            punto.x() - inicio.x();

        const double ey =
            punto.y() - inicio.y();

        return static_cast<float>(
            std::sqrt(
                ex * ex +
                ey * ey
            )
        );
    }

    double t =
        (
            (punto.x() - inicio.x()) * dx +
            (punto.y() - inicio.y()) * dy
        ) /
        longitud2;

    t =
        std::max(
            0.0,
            std::min(
                1.0,
                t
            )
        );

    const double puntoMasCercanoX =
        inicio.x() + t * dx;

    const double puntoMasCercanoY =
        inicio.y() + t * dy;

    const double ex =
        punto.x() - puntoMasCercanoX;

    const double ey =
        punto.y() - puntoMasCercanoY;

    return static_cast<float>(
        std::sqrt(
            ex * ex +
            ey * ey
        )
    );
}
bool NavigationWidget::segmentoSeguroParaRobot(
    const QPointF &inicio,
    const QPointF &fin
) const
{
    if(mContour.size() < 3)
        return true;

    /*
     * Robot bounding radius.
     *
     * The diagonal of the rectangle is used:
     *
     *              length
     *        ┌──────────────┐
     * width  │     ROBOT    │
     *        └──────────────┘
     *
     * The radius is half of that diagonal.
     *
     * This way the robot stays completely
     * inside the free zone regardless
     * of its orientation.
     */

    const float robotRadius =
        std::sqrt(
            mRobotScale * mRobotScale +
            mRobotWidth * mRobotWidth
        ) / 2.0f;

    /*
     * Check the distance from the path
     * to each contour segment.
     */

    for(
        std::size_t i = 0;
        i < mContour.size();
        ++i
    )
    {
        const QPointF &contornoInicio =
            mContour[i];

        const QPointF &contornoFin =
            mContour[
                (i + 1) % mContour.size()
            ];

        /*
         * Minimum distance between the A → B
         * path and this contour segment.
         *
         * First we check several points of the
         * path against the contour segment.
         */

        const int muestras = 20;

        for(
            int muestra = 0;
            muestra <= muestras;
            ++muestra
        )
        {
            const float t =
                static_cast<float>(muestra) /
                static_cast<float>(muestras);

            const QPointF puntoRuta(
                inicio.x() +
                t * (fin.x() - inicio.x()),

                inicio.y() +
                t * (fin.y() - inicio.y())
            );

            const float distancia =
                distanciaPuntoSegmento(
                    puntoRuta,
                    contornoInicio,
                    contornoFin
                );

            if(distancia < robotRadius)
                return false;
        }
    }

    return true;
}
bool NavigationWidget::puntoDentroContorno(
    const QPointF &punto
) const
{
    if(mContour.size() < 3)
        return false;

    bool dentro = false;

    for(
        std::size_t i = 0;
        i < mContour.size();
        ++i
    )
    {
        const QPointF &a =
            mContour[i];

        const QPointF &b =
            mContour[
                (i + 1) % mContour.size()
            ];

        const bool cruza =
            (
                (a.y() > punto.y()) !=
                (b.y() > punto.y())
            );

        if(!cruza)
            continue;

        const double xInterseccion =
            a.x() +
            (
                (punto.y() - a.y()) *
                (b.x() - a.x())
            ) /
            (b.y() - a.y());

        if(
            punto.x() <
            xInterseccion
        )
        {
            dentro = !dentro;
        }
    }

    return dentro;
}
bool NavigationWidget::segmentoDentroContorno(
    const QPointF &inicio,
    const QPointF &fin
) const
{
    if(mContour.size() < 3)
        return true;

    const float distancia =
        std::sqrt(
            static_cast<float>(
                (fin.x() - inicio.x()) *
                (fin.x() - inicio.x()) +
                (fin.y() - inicio.y()) *
                (fin.y() - inicio.y())
            )
        );

    const int muestras =
        std::max(
            10,
            static_cast<int>(
                distancia / (mRobotScale * 0.25f)
            )
        );

    for(
        int i = 0;
        i <= muestras;
        ++i
    )
    {
        const float t =
            static_cast<float>(i) /
            static_cast<float>(muestras);

        const QPointF punto(
            inicio.x() +
            t * (fin.x() - inicio.x()),

            inicio.y() +
            t * (fin.y() - inicio.y())
        );

        if(!puntoDentroContorno(punto))
            return false;
    }

    return true;
}
void NavigationWidget::actualizarRutaConRobot()
{


    if(!mRutaActiva)
        return;
    if(!mPuntoBValido)
        return;

    /*
     * (0,0) means we do not yet have a valid
     * robot position.
     *
     * In that case we do not modify the current route.
     */
    if(
        mRobotX == 0.0f &&
        mRobotY == 0.0f
    )
    {
        return;
    }

    const QPointF robotActual(
        mRobotX,
        mRobotY
    );

    /*
     * =========================================================
     * DESTINATION REACHED
     * =========================================================
     */

    const double dxDestino =
        mPuntoB.x() -
        robotActual.x();

    const double dyDestino =
        mPuntoB.y() -
        robotActual.y();

    const double distanciaDestino =
        std::sqrt(
            dxDestino * dxDestino +
            dyDestino * dyDestino
        );

    if(
        distanciaDestino <=
        mToleranciaDestino
    )
    {
        mRutaActiva = false;

        update();

        return;
    }

    if(mRuta.size() < 2)
        return;

    /*
     * =========================================================
     * FIND THE CLOSEST ROUTE SEGMENT
     * =========================================================
     *
     * The route can have several segments:
     *
     * A → P1 → P2 → B
     *
     * That is why we cannot simply use front()/back().
     */

    float distanciaMinima =
        std::numeric_limits<float>::max();

    int segmentoMasCercano = -1;

    for(
        int i = 0;
        i + 1 <
        static_cast<int>(mRuta.size());
        ++i
    )
    {
        const float distancia =
            distanciaPuntoSegmento(
                robotActual,
                mRuta[i],
                mRuta[i + 1]
            );

        if(
            distancia <
            distanciaMinima
        )
        {
            distanciaMinima =
                distancia;

            segmentoMasCercano =
                i;
        }
    }

    /*
     * =========================================================
     * THE ROBOT IS STILL NEAR THE ROUTE
     * =========================================================
     */

    if(
        segmentoMasCercano >= 0 &&
        distanciaMinima <=
        mToleranciaRuta
    )
    {
        return;
    }

    /*
     * =========================================================
     * THE ROBOT MOVED AWAY FROM THE ROUTE
     *
     * We keep Point B and recalculate from the robot's
     * current position.
     * =========================================================
     */

    calcularRuta();

    if(mRutaActiva)
    {
        mIndiceRutaActual = 1;
    }
}
float NavigationWidget::getPuntoBX() const
{
    return static_cast<float>(mPuntoB.x());
}

float NavigationWidget::getPuntoBY() const
{
    return static_cast<float>(mPuntoB.y());
}

bool NavigationWidget::puntoBValido() const
{
    return mPuntoBValido;
}
void NavigationWidget::ejecutarPasoRobot(
    int left,
    int right
)
{
    if(mRobotStepActivo)
        return;

    mRobotStepActivo = true;

    emit robotCommand(
        left,
        right
    );

    QTimer::singleShot(
        100,
        this,
        [this]()
        {
            emit robotCommand(
                0,
                0
            );

            QTimer::singleShot(
                200,
                this,
                [this]()
                {
                    mRobotStepActivo = false;

                    if(mMovimientoRutaActivo)
                        continuarMovimientoRuta();
                }
            );
        }
    );
}
void NavigationWidget::iniciarMovimientoRuta()
{
    if(mMovimientoRutaActivo)
        return;

    mPasosMovimiento = 0;
    mMovimientoRutaActivo = true;

    continuarMovimientoRuta();
}


void NavigationWidget::continuarMovimientoRuta()
{
    if(!mMovimientoRutaActivo)
        return;

    if(mRobotStepActivo)
        return;

    if(!mRutaActiva)
    {
        mMovimientoRutaActivo = false;
        return;
    }

    if(mRuta.size() < 2)
    {
        mMovimientoRutaActivo = false;
        return;
    }

    if(
        mIndiceRutaActual < 1 ||
        mIndiceRutaActual >=
        static_cast<int>(mRuta.size())
    )
    {
        mMovimientoRutaActivo = false;
        return;
    }

    const QPointF objetivo =
        mRuta[mIndiceRutaActual];

    const double dx =
        objetivo.x() - mRobotX;

    const double dy =
        objetivo.y() - mRobotY;

    const double distancia =
        std::sqrt(
            dx * dx +
            dy * dy
        );

    const float tolerancia =
        mZonaAproximacion;

    if(
        distancia <= tolerancia
    )
    {
        if(
            mIndiceRutaActual + 1 >=
            static_cast<int>(mRuta.size())
        )
        {
            mMovimientoRutaActivo = false;
            mRutaActiva = false;

            emit avisoNavegacion(
                "Destination point reached."
            );

            update();

            return;
        }
        ++mIndiceRutaActual;

        mPasosMovimiento = 0;

        continuarMovimientoRuta();

        return;
    }

    if(mPasosMovimiento >= 10)
    {
        mPasosMovimiento = 0;

        QTimer::singleShot(
            2000,
            this,
            [this]()
            {
                if(!mMovimientoRutaActivo)
                    return;

                actualizarRutaConRobot();

                if(mMovimientoRutaActivo)
                    continuarMovimientoRuta();
            }
        );

        return;
    }

    ++mPasosMovimiento;

    const double anguloObjetivo =
        std::atan2(
            dy,
            dx
        );

    double errorAdelante =
        anguloObjetivo - mRobotYaw;

    while(errorAdelante > M_PI)
        errorAdelante -= 2.0 * M_PI;

    while(errorAdelante < -M_PI)
        errorAdelante += 2.0 * M_PI;


    const double anguloObjetivoAtras =
        anguloObjetivo + M_PI;

    double errorAtras =
        anguloObjetivoAtras - mRobotYaw;

    while(errorAtras > M_PI)
        errorAtras -= 2.0 * M_PI;

    while(errorAtras < -M_PI)
        errorAtras += 2.0 * M_PI;


    const double umbralAlineacion =
        10.0 * M_PI / 180.0;


    const double magnitudAdelante =
        std::abs(errorAdelante);

    const double magnitudAtras =
        std::abs(errorAtras);


    if(
        magnitudAdelante <=
        umbralAlineacion
    )
    {
        ejecutarPasoRobot(
            145,
            145
        );

        return;
    }


    if(
        magnitudAtras <=
        umbralAlineacion
    )
    {
        ejecutarPasoRobot(
            -145,
            -145
        );

        return;
    }


    if(
        magnitudAdelante <=
        magnitudAtras
    )
    {
        if(errorAdelante > 0.0)
        {
            ejecutarPasoRobot(
                -185,
                185
            );
        }
        else
        {
            ejecutarPasoRobot(
                185,
                -185
            );
        }

       return;
    }


    if(errorAtras > 0.0)
    {
        ejecutarPasoRobot(
            -185,
            185
        );
    }
    else
    {
        ejecutarPasoRobot(
            185,
            -185
        );
    }
}
