#include "AtlasEditor.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <cstdio>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <openssl/md5.h>

#define protected public
#include "Atlas.h"
#undef protected
#include "Map.h"
#include "KeyFrameDatabase.h"
#include "ORBVocabulary.h"

#include <Eigen/Core>
#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QResizeEvent>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
const std::string VOCABULARY_FILE =
    (QDir::homePath() +
     "/ros1_ws/src/orb_slam3_ros_wrapper/config/ORBvoc.txt")
        .toStdString();
static std::string CalculateCheckSum(
    const std::string &filename)
{
    std::string checksum;

    unsigned char c[MD5_DIGEST_LENGTH];

    std::ifstream f(
        filename.c_str(),
        std::ios::in
    );

    if(!f.is_open())
        return checksum;

    MD5_CTX md5Context;

    char buffer[1024];

    MD5_Init(&md5Context);

    while(int count =
          f.readsome(
              buffer,
              sizeof(buffer)))
    {
        MD5_Update(
            &md5Context,
            buffer,
            count
        );
    }

    f.close();

    MD5_Final(
        c,
        &md5Context
    );

    for(int i = 0;
        i < MD5_DIGEST_LENGTH;
        ++i)
    {
        char aux[10];

        sprintf(
            aux,
            "%02x",
            c[i]
        );

        checksum += aux;
    }

    return checksum;
}

AtlasEditor::AtlasEditor(QWidget *parent)
    : QWidget(parent),
      mAtlas(nullptr),
      mCurrentMap(nullptr),
      mKeyFrameDatabase(nullptr),
      mVocabulary(nullptr),
      mXMin(0.0f),
      mXMax(0.0f),
      mYMin(0.0f),
      mYMax(0.0f),
      mZMin(0.0f),
      mZMax(0.0f),
      mSelectingRectangle(false),
      mPointerX(0.0f),
      mPointerY(0.0f),
      mPointerZ(0.0f),
      mLastSelectedPoint(nullptr)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    construirHerramientas();
}

AtlasEditor::~AtlasEditor()
{
    if(mAtlas)
    {
        delete mAtlas;
        mAtlas = nullptr;
    }

    if(mKeyFrameDatabase)
    {
        delete mKeyFrameDatabase;
        mKeyFrameDatabase = nullptr;
    }

    if(mVocabulary)
    {
        delete mVocabulary;
        mVocabulary = nullptr;
    }
}

bool AtlasEditor::loadAtlas(
    const std::string &filename)
{
    if(mAtlas)
    {
        delete mAtlas;
        mAtlas = nullptr;
    }

    if(mKeyFrameDatabase)
    {
        delete mKeyFrameDatabase;
        mKeyFrameDatabase = nullptr;
    }

    if(mVocabulary)
    {
        delete mVocabulary;
        mVocabulary = nullptr;
    }

    mCurrentMap = nullptr;

    mPoints.clear();
    mLastSelectedPoint = nullptr;

    mVocabulary =
        new ORB_SLAM3::ORBVocabulary();

    if(!mVocabulary->loadFromTextFile(
           VOCABULARY_FILE))
    {
        delete mVocabulary;
        mVocabulary = nullptr;

        return false;
    }

    mKeyFrameDatabase =
        new ORB_SLAM3::KeyFrameDatabase(
            *mVocabulary
        );

    std::ifstream ifs(
        filename.c_str(),
        std::ios::binary
    );

    if(!ifs.is_open())
    {
        return false;
    }

    try
    {
        boost::archive::binary_iarchive ia(ifs);
        ia >> mVocabularyFile;
        ia >> mVocabularyChecksum;
        ia >> mAtlas;
    }
    catch(const std::exception &)
    {
        ifs.close();

        mAtlas = nullptr;

        return false;
    }

    ifs.close();

    if(!mAtlas)
        return false;

    mAtlas->SetKeyFrameDababase(mKeyFrameDatabase);
    mAtlas->SetORBVocabulary(mVocabulary);

    {
        std::set<ORB_SLAM3::Map*> mapasUnicos;
        std::vector<ORB_SLAM3::Map*> mapasBackupUnicos;

        for(ORB_SLAM3::Map* pMap : mAtlas->mvpBackupMaps)
        {
            if(pMap && mapasUnicos.insert(pMap).second)
                mapasBackupUnicos.push_back(pMap);
        }



        mAtlas->mvpBackupMaps.swap(mapasBackupUnicos);
    }
    mAtlas->PostLoad();

    std::vector<ORB_SLAM3::Map*> maps =
        mAtlas->GetAllMaps();


    for(ORB_SLAM3::Map* map : maps)
    {
        if(!map)
            continue;

        int totalKF = 0;
        int badKF = 0;
        int validKF = 0;

        const std::vector<ORB_SLAM3::KeyFrame*> keyFrames =
            map->GetAllKeyFrames();

        for(ORB_SLAM3::KeyFrame* kf : keyFrames)
        {
            if(!kf)
                continue;

            totalKF++;

            if(kf->isBad())
            {
                badKF++;
                continue;
            }

            validKF++;
        }


        long totalKFMapPointRefs = 0;
        long validKFMapPointRefs = 0;
        long nullKFMapPointRefs = 0;

        for(ORB_SLAM3::KeyFrame* kf : keyFrames)
        {
            if(!kf || kf->isBad())
                continue;

            const std::vector<ORB_SLAM3::MapPoint*> mapPoints =
                kf->GetMapPointMatches();

            for(ORB_SLAM3::MapPoint* mp : mapPoints)
            {
                totalKFMapPointRefs++;

                if(mp && !mp->isBad())
                    validKFMapPointRefs++;
                else
                    nullKFMapPointRefs++;
            }
        }

    }

    
    for(ORB_SLAM3::Map* map : maps)
    {
        if(!map)
            continue;

        const std::vector<ORB_SLAM3::MapPoint*> mapPoints =
            map->GetAllMapPoints();

        long totalObservations = 0;
        long mapPointsWithoutObservations = 0;
        long mapPointsWithObservations = 0;

        for(ORB_SLAM3::MapPoint* mp : mapPoints)
        {
            if(!mp || mp->isBad())
                continue;

            const std::map<ORB_SLAM3::KeyFrame*,
                           std::tuple<int,int>> observations =
                mp->GetObservations();
           

            if(observations.empty())
            {
                mapPointsWithoutObservations++;
            }
            else
            {
                mapPointsWithObservations++;
                totalObservations += observations.size();
            }

        }

    }

    for(ORB_SLAM3::Map* map : maps)
    {
        if(!map)
            continue;

    }
    if(maps.empty())
        return false;

    mAtlas->ChangeMap(
        maps[0]
    );

    mCurrentMap =
        mAtlas->GetCurrentMap();

    if(!mCurrentMap)
        return false;

    rebuildPointCache();

    update();

    mLoadedAtlasFilename = filename;

    return !mPoints.empty();
}

bool AtlasEditor::saveAtlas(const std::string& filename)
{
    if(!mAtlas)
    {
        std::cerr << "[OSA SAVE] Atlas nulo." << std::endl;
        return false;
    }

    if(filename.empty())
    {
        std::cerr << "[OSA SAVE] Nombre de archivo vacío." << std::endl;
        return false;
    }

    if(filename == mLoadedAtlasFilename)
    {
        std::cerr << "[OSA SAVE] ERROR: se intenta sobrescribir el OSA original."
                  << std::endl;
        return false;
    }

    try
    {
        mAtlas->PreSave();

        std::ofstream ofs(filename, std::ios::binary);

        if(!ofs.is_open())
        {
            std::cerr << "[OSA SAVE] No se pudo abrir: "
                      << filename << std::endl;
            return false;
        }

        boost::archive::binary_oarchive oa(ofs);

        const std::string orbSlam3Root =
            qEnvironmentVariable("ORB_SLAM3_ROOT").toStdString();

        if(orbSlam3Root.empty())
        {
            std::cerr
                << "[OSA SAVE] ORB_SLAM3_ROOT no está definido."
                << std::endl;
            return false;
        }

        std::string vocFile =
            orbSlam3Root + "/Vocabulary/ORBvoc.txt";

        std::size_t found = vocFile.find_last_of("/\\");
        std::string vocName = vocFile.substr(found + 1);
        const std::string checksum =
            "5420bad0713bc97034dd2a9b2f0cc387";

        oa << vocName;
        oa << checksum;
        oa << mAtlas;

        ofs.close();

        std::cout << "[OSA SAVE] Guardado: "
                  << filename << std::endl;

        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << "[OSA SAVE] Excepción: "
                  << e.what() << std::endl;
        return false;
    }
}
void AtlasEditor::rebuildPointCache()
{
    mPoints.clear();

    if(!mCurrentMap)
        return;

    const std::vector<
        ORB_SLAM3::MapPoint*
    > mapPoints =
        mCurrentMap->GetAllMapPoints();

    mPoints.reserve(
        mapPoints.size()
    );

    for(
        ORB_SLAM3::MapPoint *mapPoint :
        mapPoints
    )
    {
        if(!mapPoint)
            continue;

        if(mapPoint->isBad())
            continue;

        const Eigen::Vector3f position =
            mapPoint->GetWorldPos();

        if(!std::isfinite(position.x()) ||
           !std::isfinite(position.y()) ||
           !std::isfinite(position.z()))
        {
            continue;
        }

        PointData point;

        point.mapPoint = mapPoint;

        point.x = position.x();
        point.y = position.y();
        point.z = position.z();

        point.selected = false;

        mPoints.push_back(point);
    }

    updateMapBounds();
}

void AtlasEditor::updateMapBounds()
{
    if(mPoints.empty())
    {
        mXMin = mXMax = 0.0f;
        mYMin = mYMax = 0.0f;
        mZMin = mZMax = 0.0f;

        return;
    }

    mXMin =
        std::numeric_limits<float>::max();

    mXMax =
        std::numeric_limits<float>::lowest();

    mYMin =
        std::numeric_limits<float>::max();

    mYMax =
        std::numeric_limits<float>::lowest();

    mZMin =
        std::numeric_limits<float>::max();

    mZMax =
        std::numeric_limits<float>::lowest();

    for(const PointData &point : mPoints)
    {
        mXMin =
            std::min(
                mXMin,
                point.x
            );

        mXMax =
            std::max(
                mXMax,
                point.x
            );

        mYMin =
            std::min(
                mYMin,
                point.y
            );

        mYMax =
            std::max(
                mYMax,
                point.y
            );

        mZMin =
            std::min(
                mZMin,
                point.z
            );

        mZMax =
            std::max(
                mZMax,
                point.z
            );
    }
}

bool AtlasEditor::pointToScreen(
    const PointData &point,
    QPointF &screenPoint) const
{
    if(mPoints.empty())
        return false;

    const float margin = 40.0f;

    const float mapWidth =
        std::max(
            mXMax - mXMin,
            0.001f
        );

    const float mapHeight =
        std::max(
            mYMax - mYMin,
            0.001f
        );

    const float leftPanel = 220.0f;
    const float rightPanel = 270.0f;

    const float availableWidth =
        std::max(
            static_cast<float>(width()) -
            leftPanel -
            rightPanel -
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

    screenPoint.setX(
        leftPanel +
        margin +
        (point.x - mXMin) * scale
    );

    screenPoint.setY(
        height() -
        margin -
        (point.y - mYMin) * scale
    );
    return true;
}

bool AtlasEditor::screenToMap(
    const QPointF &screenPoint,
    float &x,
    float &y) const
{
    if(mPoints.empty())
        return false;

    const float margin = 40.0f;

    const float mapWidth =
        std::max(
            mXMax - mXMin,
            0.001f
        );

    const float mapHeight =
        std::max(
            mYMax - mYMin,
            0.001f
        );

    const float leftPanel = 220.0f;
    const float rightPanel = 270.0f;

    const float availableWidth =
        std::max(
            static_cast<float>(width()) -
            leftPanel -
            rightPanel -
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

    x =
        mXMin +
        (screenPoint.x() -
         leftPanel -
         margin) /
        scale;

    y =
        mYMin +
        (height() -
         margin -
         screenPoint.y()) /
        scale;

    return true;
}

void AtlasEditor::selectPoint(
    std::size_t index,
    bool toggle)
{
    Q_UNUSED(toggle);

    if(index >= mPoints.size())
        return;

    mPoints[index].selected =
        !mPoints[index].selected;

    if(mPoints[index].selected)
    {
        mLastSelectedPoint =
            mPoints[index].mapPoint;
    }
    else
    {
        mLastSelectedPoint = nullptr;
    }

    actualizarInformacion();

    update();
}

void AtlasEditor::selectRectangle(
    const QRect &rectangle)
{
    const QRect selection =
        rectangle.normalized();

    const bool modoDeseleccionar =
        mModoBorrar &&
        mModoBorrar->isChecked();

    for(PointData &point : mPoints)
    {
        QPointF screenPoint;

        if(!pointToScreen(
               point,
               screenPoint))
        {
            continue;
        }

        if(!selection.contains(
               screenPoint.toPoint()))
        {
            continue;
        }

        if(modoDeseleccionar)
        {
            point.selected = false;
        }
        else
        {
            point.selected = true;
        }
    }

    actualizarInformacion();

    update();
}
void AtlasEditor::clearSelection()
{
    for(PointData &point : mPoints)
        point.selected = false;

    mLastSelectedPoint = nullptr;

    actualizarInformacion();
}

void AtlasEditor::paintEvent(
    QPaintEvent *event)
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

    if(mPoints.empty())
    {
        painter.setPen(Qt::white);

        painter.drawText(
            rect(),
            Qt::AlignCenter,
            "No hay puntos del mapa cargados."
        );

        return;
    }

    painter.setPen(
        QPen(Qt::gray, 1)
    );

    const float margin = 40.0f;

    const float leftPanel = 220.0f;
   const float rightPanel = 270.0f;

    painter.drawLine(
        static_cast<int>(leftPanel + margin),
        height() -
        static_cast<int>(margin),
        width() -
        static_cast<int>(rightPanel + margin),
        height() -
        static_cast<int>(margin)
    );

    painter.drawLine(
        static_cast<int>(leftPanel + margin),
        static_cast<int>(margin),
        static_cast<int>(leftPanel + margin),
        height() -
        static_cast<int>(margin)
    );

    for(const PointData &point : mPoints)
    {
        QPointF screenPoint;

        if(!pointToScreen(
               point,
               screenPoint))
        {
            continue;
        }

        if(point.selected)
        {
            painter.setPen(
                QPen(Qt::red, 4)
            );
        }
        else
        {
            painter.setPen(
                QPen(Qt::green, 2)
            );
        }

        painter.drawPoint(
            screenPoint
        );
    }

    if(mSelectingRectangle)
    {
        painter.setPen(
            QPen(
                Qt::yellow,
                1,
                Qt::DashLine
            )
        );

        painter.drawRect(
            QRect(
                mSelectionStart,
                mSelectionEnd
            ).normalized()
        );
    }

}

void AtlasEditor::mouseMoveEvent(
    QMouseEvent *event)
{
    float x;
    float y;

    if(screenToMap(
           event->pos(),
           x,
           y))
    {
        mPointerX = x;
        mPointerY = y;
        mPointerZ = 0.0f;
    }

    if(mSelectingRectangle)
    {
        mSelectionEnd = event->pos();
    }
    actualizarInformacion();
    update();
}

void AtlasEditor::mousePressEvent(
    QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return;

    if(mPoints.empty())
        return;

    const float selectionRadius = 7.0f;

    int closestIndex = -1;

    float closestDistance =
        std::numeric_limits<float>::max();

    for(std::size_t i = 0;
        i < mPoints.size();
        ++i)
    {
        QPointF screenPoint;

        if(!pointToScreen(
               mPoints[i],
               screenPoint))
        {
            continue;
        }

        const float dx =
            screenPoint.x() -
            event->pos().x();

        const float dy =
            screenPoint.y() -
            event->pos().y();

        const float distance =
            std::sqrt(
                dx * dx +
                dy * dy
            );

        if(distance < selectionRadius &&
           distance < closestDistance)
        {
            closestDistance = distance;
            closestIndex =
                static_cast<int>(i);
        }
    }

    if(closestIndex >= 0)
    {
        selectPoint(
            static_cast<std::size_t>(
                closestIndex
            ),
            true
        );

        mSelectingRectangle = false;

        return;
    }

    mSelectingRectangle = true;

    mSelectionStart =
        event->pos();

    mSelectionEnd =
        event->pos();

    update();
}

void AtlasEditor::mouseReleaseEvent(
    QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return;

    if(!mSelectingRectangle)
        return;

    mSelectionEnd =
        event->pos();

    const QRect rectangle =
        QRect(
            mSelectionStart,
            mSelectionEnd
        ).normalized();

    mSelectingRectangle = false;

    if(rectangle.width() > 3 ||
       rectangle.height() > 3)
    {
        selectRectangle(
            rectangle
        );
    }

    update();
}
void AtlasEditor::wheelEvent(QWheelEvent *event)
{
    if(mPoints.empty())
        return;

    const QPointF mousePosition =
        event->pos();

    float mouseX = 0.0f;
    float mouseY = 0.0f;

    if(!screenToMap(
           mousePosition,
           mouseX,
           mouseY))
    {
        return;
    }

    const int delta =
        event->angleDelta().y();

    if(delta == 0)
        return;

    const float zoomFactor =
        delta > 0 ? 0.8f : 1.25f;

    const float oldXMin = mXMin;
    const float oldXMax = mXMax;
    const float oldYMin = mYMin;
    const float oldYMax = mYMax;

    const float oldWidth =
        oldXMax - oldXMin;

    const float oldHeight =
        oldYMax - oldYMin;

    const float newWidth =
        oldWidth * zoomFactor;

    const float newHeight =
        oldHeight * zoomFactor;

    const float mouseRatioX =
        (mouseX - oldXMin) / oldWidth;

    const float mouseRatioY =
        (mouseY - oldYMin) / oldHeight;

    mXMin =
        mouseX - mouseRatioX * newWidth;

    mXMax =
        mXMin + newWidth;

    mYMin =
        mouseY - mouseRatioY * newHeight;

    mYMax =
        mYMin + newHeight;

    update();

    event->accept();
}
void AtlasEditor::construirHerramientas()
{
    mHerramientas =
        new QWidget(this);

    mInformacion =
        new QWidget(this);

    mHerramientas->setStyleSheet(
        "QWidget { background: #202020; color: white; }"
        "QGroupBox { border: 1px solid #555;"
        " margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin;"
        " left: 10px; padding: 0 4px; }"
        "QLabel { color: white; }"
        "QLineEdit { background: #303030;"
        " color: white; border: 1px solid #666;"
        " padding: 5px; }"
        "QPushButton { background: #303030;"
        " color: white; border: 1px solid #666;"
        " padding: 7px; }"
        "QPushButton:hover { background: #404040; }"
    );

    mInformacion->setStyleSheet(
        "QWidget { background: #202020; color: white; }"
        "QGroupBox { border: 1px solid #555;"
        " margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin;"
        " left: 10px; padding: 0 4px; }"
        "QLabel { color: white; }"
    );

    QVBoxLayout *herramientasLayout =
        new QVBoxLayout(mHerramientas);

    herramientasLayout->setContentsMargins(
        12, 12, 12, 12
    );

    herramientasLayout->setSpacing(10);

    QLabel *tituloHerramientas =
        new QLabel(
            "HERRAMIENTAS",
            mHerramientas
        );

    QFont tituloFont =
        tituloHerramientas->font();

    tituloFont.setBold(true);
    tituloFont.setPointSize(12);

    tituloHerramientas->setFont(
        tituloFont
    );

    herramientasLayout->addWidget(
        tituloHerramientas
    );

    QGroupBox *filtro =
        new QGroupBox(
            "Filtro distancia XY",
            mHerramientas
        );

    QVBoxLayout *filtroLayout =
        new QVBoxLayout(filtro);

    mFiltroDistanciaXY =
        new QLineEdit(filtro);

    mFiltroDistanciaXY->setPlaceholderText(
        "Distancia XY"
    );

    filtroLayout->addWidget(
        mFiltroDistanciaXY
    );

    mBotonAplicarFiltro =
        new QPushButton(
            "Aplicar filtro",
            filtro
        );

    filtroLayout->addWidget(
        mBotonAplicarFiltro
    );
    connect(
        mBotonAplicarFiltro,
        &QPushButton::clicked,
        this,
        &AtlasEditor::aplicarFiltroXY
    );

    herramientasLayout->addWidget(
        filtro
    );

    QGroupBox *borrado =
        new QGroupBox(
            "Edición",
            mHerramientas
        );

    QVBoxLayout *borradoLayout =
        new QVBoxLayout(borrado);

    mModoBorrar =
        new QCheckBox(
            "Modo deseleccionar",
            borrado
        );

    mBotonBorrarSeleccion =
        new QPushButton(
            "Borrar selección",
            borrado
        );
    connect(
        mBotonBorrarSeleccion,
        &QPushButton::clicked,
        this,
        &AtlasEditor::borrarSeleccion
    );

    QPushButton *botonGuardarPrueba =
        new QPushButton(
            "Guardar copia de prueba",
            borrado
        );

    connect(
        botonGuardarPrueba,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if(!mAtlas)
                return;

            QString defaultName =
                QString::fromStdString(
                    mLoadedAtlasFilename
                );

            if(defaultName.endsWith(".osa"))
            {
                defaultName.chop(4);
                defaultName += "_copia_prueba.osa";
            }
            else
            {
                defaultName += "_copia_prueba.osa";
            }

            QString nuevoArchivo =
                QFileDialog::getSaveFileName(
                    this,
                    "Guardar copia de prueba",
                    defaultName,
                    "OSA (*.osa)"
                );

            if(nuevoArchivo.isEmpty())
                return;

            const std::string nuevoFilename =
                nuevoArchivo.toStdString();

            if(nuevoFilename == mLoadedAtlasFilename)
            {
                QMessageBox::warning(
                    this,
                    "Archivo no válido",
                    "Debes utilizar un nombre diferente."
                );

                return;
            }

            if(saveAtlas(nuevoFilename))
            {
                QMessageBox::information(
                    this,
                    "Copia guardada",
                    "La copia del Atlas fue guardada correctamente."
                );
            }
            else
            {
                QMessageBox::critical(
                    this,
                    "Error",
                    "No se pudo guardar la copia."
                );
            }
        }
    );

    borradoLayout->addWidget(
        botonGuardarPrueba
    );

    mBotonBorrarDefinitivo =
        new QPushButton(
            "Borrar definitivamente",
            borrado
        );
    connect(
        mBotonBorrarDefinitivo,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if(!mAtlas)
                return;
    
            bool haySeleccion = false;
    
            for(const PointData &point : mPoints)
            {
                if(point.selected)
                {
                    haySeleccion = true;
                    break;
                }
            }
    
            if(!haySeleccion)
            {
                QMessageBox::information(
                    this,
                    "Borrar definitivamente",
                    "No hay puntos seleccionados."
                );
    
                return;
            }
    
            QMessageBox::StandardButton respuesta =
                QMessageBox::question(
                    this,
                    "Borrar definitivamente",
                    "Los puntos seleccionados serán eliminados "
                    "del Atlas y se guardará un nuevo archivo OSA.\n\n"
                    "El archivo original no será modificado.\n\n"
                    "¿Deseas continuar?",
                    QMessageBox::Yes |
                    QMessageBox::No,
                    QMessageBox::No
            );
    
            if(respuesta != QMessageBox::Yes)
                return;
    
            QString defaultName =
                QString::fromStdString(
                    mLoadedAtlasFilename
                );
    
            if(defaultName.endsWith(".osa"))
            {
                defaultName.chop(4);
                defaultName += "_editado.osa";
            }
            else
            {
                defaultName += "_editado.osa";
            }
    
            QString nuevoArchivo =
                QFileDialog::getSaveFileName(
                    this,
                    "Guardar nuevo mapa OSA",
                    defaultName,
                    "Archivos OSA (*.osa)"
                );
    
            if(nuevoArchivo.isEmpty())
                return;
    
            if(!nuevoArchivo.endsWith(".osa"))
                nuevoArchivo += ".osa";
    
            const std::string nuevoFilename =
                nuevoArchivo.toStdString();
    
            if(nuevoFilename == mLoadedAtlasFilename)
            {
                QMessageBox::warning(
                    this,
                    "Archivo no permitido",
                    "No se puede sobrescribir el archivo OSA original.\n\n"
                    "Debes utilizar un nombre diferente."
                );
    
                return;
            }
            
            for(PointData &point : mPoints)
            {
                if(!point.selected)
                    continue;

                if(point.mapPoint)
                    point.mapPoint->SetBadFlag();
            }

            mLastSelectedPoint = nullptr;

            rebuildPointCache();

            actualizarInformacion();

            update();

            if(saveAtlas(nuevoFilename))
            {
                QMessageBox::information(
                    this,
                    "Mapa guardado",
                    "El nuevo mapa OSA fue guardado y verificado correctamente."
                );
            }
            else
            {
                QMessageBox::critical(
                    this,
                    "Error al guardar",
                    "Los puntos fueron eliminados del Atlas en memoria, "
                    "pero no se pudo guardar o verificar el nuevo archivo OSA."
                );
            }
            
            
        }
    );
    borradoLayout->addWidget(
        mModoBorrar
    );

    borradoLayout->addWidget(
        mBotonBorrarSeleccion
    );

    borradoLayout->addWidget(
        mBotonBorrarDefinitivo
    );

    herramientasLayout->addWidget(
        borrado
    );

    herramientasLayout->addStretch();

    // =========================================================
    // INFORMACIÓN
    // =========================================================

    QVBoxLayout *informacionLayout =
        new QVBoxLayout(mInformacion);

    informacionLayout->setContentsMargins(
        12, 12, 12, 12
    );

    informacionLayout->setSpacing(8);

    QLabel *tituloInformacion =
        new QLabel(
            "INFORMACIÓN",
            mInformacion
        );

    tituloInformacion->setFont(
        tituloFont
    );

    informacionLayout->addWidget(
        tituloInformacion
    );

    QGroupBox *limites =
        new QGroupBox(
            "Límites del mapa",
            mInformacion
        );

    QVBoxLayout *limitesLayout =
        new QVBoxLayout(limites);

    mLabelXMin =
        new QLabel("X min: 0");
    mLabelXMax =
        new QLabel("X max: 0");

    mLabelYMin =
        new QLabel("Y min: 0");
    mLabelYMax =
        new QLabel("Y max: 0");

    mLabelZMin =
        new QLabel("Z min: 0");
    mLabelZMax =
        new QLabel("Z max: 0");

    limitesLayout->addWidget(mLabelXMin);
    limitesLayout->addWidget(mLabelXMax);

    limitesLayout->addWidget(mLabelYMin);
    limitesLayout->addWidget(mLabelYMax);

    limitesLayout->addWidget(mLabelZMin);
    limitesLayout->addWidget(mLabelZMax);

    informacionLayout->addWidget(
        limites
    );

    QGroupBox *distancias =
        new QGroupBox(
            "Distancias",
            mInformacion
        );

    QVBoxLayout *distanciasLayout =
        new QVBoxLayout(distancias);

    mLabelDistanciaX =
        new QLabel("ΔX: 0");

    mLabelDistanciaY =
        new QLabel("ΔY: 0");

    mLabelDistanciaZ =
        new QLabel("ΔZ: 0");

    distanciasLayout->addWidget(
        mLabelDistanciaX
    );

    distanciasLayout->addWidget(
        mLabelDistanciaY
    );

    distanciasLayout->addWidget(
        mLabelDistanciaZ
    );

    informacionLayout->addWidget(
        distancias
    );

    QGroupBox *puntero =
        new QGroupBox(
            "Puntero",
            mInformacion
        );

    QVBoxLayout *punteroLayout =
        new QVBoxLayout(puntero);

    mLabelPunteroX =
        new QLabel("X: 0");

    mLabelPunteroY =
        new QLabel("Y: 0");

    mLabelPunteroZ =
        new QLabel("Z: 0");

    punteroLayout->addWidget(
        mLabelPunteroX
    );

    punteroLayout->addWidget(
        mLabelPunteroY
    );

    punteroLayout->addWidget(
        mLabelPunteroZ
    );

    informacionLayout->addWidget(
        puntero
    );

    QGroupBox *seleccion =
        new QGroupBox(
            "Último punto seleccionado",
            mInformacion
        );

    QVBoxLayout *seleccionLayout =
        new QVBoxLayout(seleccion);

    mLabelSeleccionX =
        new QLabel("X: --");

    mLabelSeleccionY =
        new QLabel("Y: --");

    mLabelSeleccionZ =
        new QLabel("Z: --");

    seleccionLayout->addWidget(
        mLabelSeleccionX
    );

    seleccionLayout->addWidget(
        mLabelSeleccionY
    );

    seleccionLayout->addWidget(
        mLabelSeleccionZ
    );

    informacionLayout->addWidget(
        seleccion
    );

    informacionLayout->addStretch();

    actualizarInformacion();
}

void AtlasEditor::actualizarInformacion()
{
    if(!mLabelXMin)
        return;

    mLabelXMin->setText(
        QString("X min: %1").arg(mXMin, 0, 'f', 3)
    );

    mLabelXMax->setText(
        QString("X max: %1").arg(mXMax, 0, 'f', 3)
    );

    mLabelYMin->setText(
        QString("Y min: %1").arg(mYMin, 0, 'f', 3)
    );

    mLabelYMax->setText(
        QString("Y max: %1").arg(mYMax, 0, 'f', 3)
    );

    mLabelZMin->setText(
        QString("Z min: %1").arg(mZMin, 0, 'f', 3)
    );

    mLabelZMax->setText(
        QString("Z max: %1").arg(mZMax, 0, 'f', 3)
    );

    mLabelPunteroX->setText(
        QString("X: %1").arg(mPointerX, 0, 'f', 3)
    );

    mLabelPunteroY->setText(
        QString("Y: %1").arg(mPointerY, 0, 'f', 3)
    );

    mLabelPunteroZ->setText(
        QString("Z: %1").arg(mPointerZ, 0, 'f', 3)
    );

    const float distanciaX =
        mXMax - mXMin;

    const float distanciaY =
        mYMax - mYMin;

    const float distanciaZ =
        mZMax - mZMin;

    mLabelDistanciaX->setText(
        QString("ΔX: %1")
            .arg(distanciaX, 0, 'f', 3)
    );

    mLabelDistanciaY->setText(
        QString("ΔY: %1")
            .arg(distanciaY, 0, 'f', 3)
    );

    mLabelDistanciaZ->setText(
        QString("ΔZ: %1")
            .arg(distanciaZ, 0, 'f', 3)
    );

    if(mLastSelectedPoint)
    {
        const Eigen::Vector3f position =
            mLastSelectedPoint->GetWorldPos();

        mLabelSeleccionX->setText(
            QString("X: %1")
                .arg(position.x(), 0, 'f', 3)
        );

        mLabelSeleccionY->setText(
            QString("Y: %1")
                .arg(position.y(), 0, 'f', 3)
        );

        mLabelSeleccionZ->setText(
            QString("Z: %1")
                .arg(position.z(), 0, 'f', 3)
        );
    }
    else
    {
        mLabelSeleccionX->setText(
            "X: --"
        );

        mLabelSeleccionY->setText(
            "Y: --"
        );

        mLabelSeleccionZ->setText(
            "Z: --"
        );
    }
}

void AtlasEditor::resizeEvent(
    QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    const int leftWidth = 220;
    const int rightWidth = 270;

    if(mHerramientas)
    {
        mHerramientas->setGeometry(
            0,
            0,
            leftWidth,
            height()
        );
    }

    if(mInformacion)
    {
        mInformacion->setGeometry(
            width() - rightWidth,
            0,
            rightWidth,
            height()
        );
    }
}
void AtlasEditor::aplicarFiltroXY()
{
    if(mPoints.size() < 2)
        return;

    bool ok = false;

    const float threshold =
        mFiltroDistanciaXY->text().toFloat(
            &ok
        );

    if(!ok || threshold <= 0.0f)
        return;

    const double thresholdSquared =
        static_cast<double>(threshold) *
        static_cast<double>(threshold);

    for(PointData &point : mPoints)
    {
        double nearestSquared =
            std::numeric_limits<double>::infinity();

        for(const PointData &other : mPoints)
        {
            if(&point == &other)
                continue;

            const double dx =
                static_cast<double>(point.x) -
                static_cast<double>(other.x);

            const double dy =
                static_cast<double>(point.y) -
                static_cast<double>(other.y);

            const double distanceSquared =
                dx * dx +
                dy * dy;

            if(distanceSquared <
               nearestSquared)
            {
                nearestSquared =
                    distanceSquared;
            }
        }

        if(nearestSquared > thresholdSquared)
        {
            point.selected = true;
        }
    }

    actualizarInformacion();

    update();
}
void AtlasEditor::borrarSeleccion()
{
    if(!mAtlas || !mCurrentMap)
        return;

    bool huboSeleccion = false;

    for(PointData &point : mPoints)
    {
        if(!point.selected)
            continue;

        huboSeleccion = true;

        if(point.mapPoint)
        {
            point.mapPoint->SetBadFlag();
        }
    }

    if(!huboSeleccion)
        return;

    mLastSelectedPoint = nullptr;

    rebuildPointCache();

    actualizarInformacion();

    update();
}
std::vector<QPointF> AtlasEditor::getMapPoints2D() const
{
    std::vector<QPointF> points2D;

    points2D.reserve(mPoints.size());

    for(const PointData &point : mPoints)
    {
        points2D.emplace_back(
            point.x,
            point.y
        );
    }

    return points2D;
}
std::vector<float> AtlasEditor::getMapPointsZ() const
{
    std::vector<float> zValues;

    zValues.reserve(
        mPoints.size()
    );

    for(const PointData &point : mPoints)
    {
        zValues.push_back(
            point.z
        );
    }

    return zValues;
}
void AtlasEditor::borrarMapPoints2D(
    const std::vector<int> &indices
)
{
    if(!mAtlas || !mCurrentMap)
        return;

    for(int index : indices)
    {
        if(index < 0)
            continue;

        if(
            index >=
            static_cast<int>(mPoints.size())
        )
        {
            continue;
        }

        PointData &point =
            mPoints[index];

        if(point.mapPoint)
        {
            point.mapPoint->SetBadFlag();
        }
    }

    rebuildPointCache();

    actualizarInformacion();

    update();
}
