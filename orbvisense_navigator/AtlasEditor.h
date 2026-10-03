#ifndef ATLASEDITOR_H
#define ATLASEDITOR_H

#include <QPoint>
#include <QWidget>
#include <QPointF>
#include <string>
#include <vector>

#include "ORBVocabulary.h"

namespace ORB_SLAM3
{
class Atlas;
class Map;
class MapPoint;
class KeyFrameDatabase;
}

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QGroupBox;

class AtlasEditor : public QWidget
{
    Q_OBJECT

public:
    explicit AtlasEditor(QWidget *parent = nullptr);
    ~AtlasEditor();

    bool loadAtlas(const std::string &filename);
    bool saveAtlas(const std::string &filename);
    std::vector<QPointF> getMapPoints2D() const;
    std::vector<float> getMapPointsZ() const;
    void borrarMapPoints2D(
        const std::vector<int> &indices
    );
protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct PointData
    {
        ORB_SLAM3::MapPoint *mapPoint;

        float x;
        float y;
        float z;

        bool selected;
    };

    void rebuildPointCache();
    void updateMapBounds();

    bool pointToScreen(
        const PointData &point,
        QPointF &screenPoint
    ) const;

    bool screenToMap(
        const QPointF &screenPoint,
        float &x,
        float &y
    ) const;

    void selectPoint(
        std::size_t index,
        bool toggle
    );

    void selectRectangle(
        const QRect &rectangle
    );

    void clearSelection();
    void aplicarFiltroXY();
    void borrarSeleccion();
    void construirHerramientas();
    void actualizarInformacion();

    ORB_SLAM3::Atlas *mAtlas;
    ORB_SLAM3::Map *mCurrentMap;

    ORB_SLAM3::KeyFrameDatabase *mKeyFrameDatabase;
    ORB_SLAM3::ORBVocabulary *mVocabulary;

    std::string mVocabularyFile;
    std::string mVocabularyChecksum;
    std::string mLoadedAtlasFilename;

    std::vector<PointData> mPoints;

    float mXMin;
    float mXMax;

    float mYMin;
    float mYMax;

    float mZMin;
    float mZMax;

    bool mSelectingRectangle;

    QPoint mSelectionStart;
    QPoint mSelectionEnd;

    float mPointerX;
    float mPointerY;
    float mPointerZ;

    ORB_SLAM3::MapPoint *mLastSelectedPoint;

    QWidget *mHerramientas;
    QWidget *mInformacion;

    QLineEdit *mFiltroDistanciaXY;
    QCheckBox *mModoBorrar;

    QPushButton *mBotonAplicarFiltro;
    QPushButton *mBotonBorrarSeleccion;
    QPushButton *mBotonBorrarDefinitivo;

    QLabel *mLabelXMin;
    QLabel *mLabelXMax;

    QLabel *mLabelYMin;
    QLabel *mLabelYMax;

    QLabel *mLabelZMin;
    QLabel *mLabelZMax;

    QLabel *mLabelDistanciaX;
    QLabel *mLabelDistanciaY;
    QLabel *mLabelDistanciaZ;

    QLabel *mLabelPunteroX;
    QLabel *mLabelPunteroY;
    QLabel *mLabelPunteroZ;

    QLabel *mLabelSeleccionX;
    QLabel *mLabelSeleccionY;
    QLabel *mLabelSeleccionZ;
};

#endif
