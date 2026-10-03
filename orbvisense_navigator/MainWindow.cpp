#include "MainWindow.h"

#include "AtlasEditor.h"
#include "NavigationWidget.h"

#include <QDir>
#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QProcess>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <cmath>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      mInicio(nullptr),
      mPaginaNavegar(nullptr),
      mCentral(nullptr),
      mBotonCargar(nullptr),
      mBotonNavegar(nullptr),
      mAtlasEditor(nullptr),
      mNavigationWidget(nullptr),
      mPanelIzquierdoNavegacion(nullptr),
      mBotonAvanzar(nullptr),
      mRobotWebSocket(nullptr),
      mZMinLabel(nullptr),
      mZMaxLabel(nullptr),
      mMapaCargado(false),
      mRosNodeHandle(nullptr),
      mRosTimer(nullptr),
      mRutaTimer(nullptr),
      mRosbridgeProcess(nullptr),
      mJpegProcess(nullptr),
      mOrbslamProcess(nullptr),
      mOrbslamTimer(nullptr),
      mRosbridgeTimer(nullptr),
      mJpegTimer(nullptr),
      mRecepcionActiva(false),
      mCalibrandoRobot(false),
      mLabelRobotX(nullptr),
      mLabelRobotY(nullptr),
      mLabelRobotYaw(nullptr),
      mLabelEstadoRecepcion(nullptr),
      mAvisoNavegacion(nullptr)
{
    setWindowTitle("OrbVIsense Navigator");

    resize(1400, 850);

    mRosNodeHandle =
        new ros::NodeHandle;

    mRosTimer =
        new QTimer(this);
    mRobotWebSocket =
        new QWebSocket(
            QString(),
            QWebSocketProtocol::VersionLatest,
            this
        );

    connect(
        mRobotWebSocket,
        &QWebSocket::connected,
        this,
        [this]()
        {
            qDebug()
                << "[ROBOT] WebSocket connected to rosbridge.";

            QJsonObject advertise;

            advertise["op"] = "advertise";
            advertise["topic"] = "/robot/pwm";
            advertise["type"] = "robot_pwm/PWM";

            QJsonDocument document(
                advertise
            );

            QString texto =
                QString::fromUtf8(
                    document.toJson(
                        QJsonDocument::Compact
                    )
                );

            mRobotWebSocket->sendTextMessage(
                texto
            );

            qDebug()
                << "[ROBOT] Topic advertised:"
                << texto;
        }
    );

    connect(
        mRobotWebSocket,
        &QWebSocket::disconnected,
        this,
        []()
        {
            qDebug()
                << "[ROBOT] WebSocket disconnected.";
        }
    );
    


    connect(
        mRosTimer,
        &QTimer::timeout,
        this,
        &MainWindow::procesarROS
    );

    mRosTimer->start(20);
    mRutaTimer =
        new QTimer(this);

    connect(
        mRutaTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            mNavigationWidget->
                actualizarRutaConRobot();
        }
    );

    mRutaTimer->start(2000);
    construirInterfaz();
    connect(
        mNavigationWidget,
        &NavigationWidget::avisoNavegacion,
        this,
        &MainWindow::mostrarAvisoNavegacion
    );
    connect(
        mNavigationWidget,
        &NavigationWidget::robotCommand,
        this,
        [this](
            int left,
            int right
        )
        {
                if(
                !mRobotWebSocket ||
                mRobotWebSocket->state() !=
                QAbstractSocket::ConnectedState
            )
            {
                qDebug()
                    << "[ROBOT] WebSocket not connected.";

                return;
            }

            QJsonObject msg;

            msg["left"] = left;
            msg["right"] = right;

            QJsonObject publish;

            publish["op"] = "publish";
            publish["topic"] = "/robot/pwm";
            publish["type"] = "robot_pwm/PWM";
            publish["msg"] = msg;

            QJsonDocument document(
                publish
            );

            mRobotWebSocket->sendTextMessage(
                QString::fromUtf8(
                    document.toJson(
                        QJsonDocument::Compact
                    )
                )
            );
        }
    );
}

MainWindow::~MainWindow()
{
    qDebug()
        << "[SHUTDOWN] Closing ROS process..";

    if(mOrbslamTimer)
        mOrbslamTimer->stop();
    if(mRutaTimer)
        mRutaTimer->stop();

    if(mJpegTimer)
        mJpegTimer->stop();

    if(mRosbridgeTimer)
        mRosbridgeTimer->stop();

    if(
        mJpegProcess &&
        mJpegProcess->state() != QProcess::NotRunning
    )
    {
        qDebug()
            << "[SHUTDOWN] Closing jpeg_to_mono...";

        mJpegProcess->terminate();

        if(!mJpegProcess->waitForFinished(2000))
            mJpegProcess->kill();
    }

    if(
        mRosbridgeProcess &&
        mRosbridgeProcess->state() != QProcess::NotRunning
    )
    {
        qDebug()
            << "[SHUTDOWN] Closing rosbridge...";

        mRosbridgeProcess->terminate();

        if(!mRosbridgeProcess->waitForFinished(2000))
            mRosbridgeProcess->kill();
    }
        if(
        mOrbslamProcess &&
        mOrbslamProcess->state() != QProcess::NotRunning
    )
    {
        mOrbslamProcess->terminate();

        if(!mOrbslamProcess->waitForFinished(3000))
            mOrbslamProcess->kill();
    }

  
}



void MainWindow::construirInterfaz()
{
    mCentral = new QStackedWidget(this);

    // =========================================================
    // MAIN PAGE
    // =========================================================

    mInicio = new QWidget(mCentral);

    QVBoxLayout *inicioLayout =
        new QVBoxLayout(mInicio);

    inicioLayout->setContentsMargins(
        40, 40, 40, 40
    );

    inicioLayout->setSpacing(20);

    inicioLayout->addStretch();

    QPushButton *titulo =
        new QPushButton(
            "OrbVIsense Navigator",
            mInicio
        );

    titulo->setEnabled(false);

    QFont tituloFont =
        titulo->font();

    tituloFont.setBold(true);
    tituloFont.setPointSize(20);

    titulo->setFont(
        tituloFont
    );

    inicioLayout->addWidget(
        titulo
    );

    mBotonCargar =
        new QPushButton(
            "LOAD MAP",
            mInicio
        );

    mBotonCargar->setMinimumHeight(
        50
    );

    inicioLayout->addWidget(
        mBotonCargar
    );

    mBotonNavegar =
        new QPushButton(
            "NAVIGATION",
            mInicio
        );

    mBotonNavegar->setMinimumHeight(
        50
    );

    inicioLayout->addWidget(
        mBotonNavegar
    );

  

    inicioLayout->addStretch();
    QLabel *autores =
        new QLabel(
            "JUAN CAMILO RODRIGUEZ GARCIA\n"
            "JUAN CARLOS PIRACOCA TAPIERO",
            mInicio
        );

    autores->setAlignment(
        Qt::AlignRight
    );

    autores->setStyleSheet(
        "QLabel {"
        "font-size: 11px;"
        "color: #666666;"
        "}"
    );

    inicioLayout->addWidget(
        autores
    );
 
    // =========================================================
    // NAVIGATION PAGE
    // =========================================================

    mPaginaNavegar =
        new QWidget(mCentral);

    QVBoxLayout *navegarLayout =
        new QVBoxLayout(
            mPaginaNavegar
        );

    navegarLayout->setContentsMargins(
        0, 0, 0, 0
    );
    // ---------------------------------------------------------
    // TOP BACK BUTTON
    // ---------------------------------------------------------

    QPushButton *botonVolverNavegarSuperior =
        new QPushButton(
            "RETURN",
            mPaginaNavegar
        );

    botonVolverNavegarSuperior->setFixedWidth(
        100
    );

    botonVolverNavegarSuperior->setMinimumHeight(
        35
    );

    navegarLayout->addWidget(
        botonVolverNavegarSuperior,
        0,
        Qt::AlignLeft
    );
    QWidget *zonaNavegacion =
        new QWidget(
            mPaginaNavegar
        );

    QHBoxLayout *zonaLayout =
        new QHBoxLayout(
            zonaNavegacion
        );

    zonaLayout->setContentsMargins(
        0, 0, 0, 0
    );

    // ---------------------------------------------------------
    // Map
    // ---------------------------------------------------------
    mAtlasEditor =
        new AtlasEditor(
            mPaginaNavegar
        );
    mAtlasEditor->hide();
    mNavigationWidget =
        new NavigationWidget(
            mPaginaNavegar
        );
// ---------------------------------------------------------
// LEFT NAVIGATION BAR
// Appears only when ORB-SLAM3 is active.
// ---------------------------------------------------------

    mPanelIzquierdoNavegacion =
        new QWidget(
            zonaNavegacion
        );

    mPanelIzquierdoNavegacion->setFixedWidth(
        220
    );

    QVBoxLayout *panelIzquierdoLayout =
        new QVBoxLayout(
            mPanelIzquierdoNavegacion
        );

    panelIzquierdoLayout->setContentsMargins(
        15, 45, 15, 15
    );

    panelIzquierdoLayout->setSpacing(
        12
    );

    mPanelIzquierdoNavegacion->hide();
// ---------------------------------------------------------
// RIGHT BAR
// ---------------------------------------------------------

    QWidget *panelNavegacion =
        new QWidget(
            mPaginaNavegar
        );

    panelNavegacion->setFixedWidth(
        260
    );

    QVBoxLayout *panelLayout =
        new QVBoxLayout(
            panelNavegacion
        );

    panelLayout->setContentsMargins(
        15, 15, 15, 15
    );

    panelLayout->setSpacing(
        12
    );
    

    // ---------------------------------------------------------
    // ---------------------------------------------------------
    // TITLE
    // ---------------------------------------------------------

    QLabel *tituloPanel =
        new QLabel(
            "PARAMETERS",
            panelNavegacion
        );

    QFont tituloPanelFont =
        tituloPanel->font();

    tituloPanelFont.setBold(true);
    tituloPanelFont.setPointSize(12);

    tituloPanel->setFont(
        tituloPanelFont
    );

    panelLayout->addWidget(
        tituloPanel
    );
    // =========================================================
    // LEFT BAR
    // =========================================================

    // ---------------------------------------------------------
    // ROBOT RECEPTION
    // ---------------------------------------------------------

    QPushButton *botonActivarRecepcion =
        new QPushButton(
            "ENABLE RECEPTION",
            panelNavegacion
        );

    botonActivarRecepcion->setMinimumHeight(
        45
    );

    panelIzquierdoLayout->addWidget(
        botonActivarRecepcion
    );


    mLabelEstadoRecepcion =
        new QLabel(
            "Reception: INACTIVE",
            panelNavegacion
        );

    mLabelRobotX =
        new QLabel(
            "X: ---",
            panelNavegacion
        );

    mLabelRobotY =
        new QLabel(
            "Y: ---",
            panelNavegacion
        );

    mLabelRobotYaw =
        new QLabel(
            "Yaw: ---",
            panelNavegacion
        );

    panelIzquierdoLayout->addWidget(
        mLabelEstadoRecepcion
    );

    panelIzquierdoLayout->addWidget(
        mLabelRobotX
    );

    panelIzquierdoLayout->addWidget(
        mLabelRobotY
    );

    panelIzquierdoLayout->addWidget(
        mLabelRobotYaw
    );


    // ---------------------------------------------------------
    // SELECT POINT B
    // ---------------------------------------------------------

    mBotonPuntoB =
        new QPushButton(
            "SELECT POINT B",
            panelNavegacion
        );

    mBotonPuntoB->setMinimumHeight(
        45
    );

    panelIzquierdoLayout->addWidget(
        mBotonPuntoB
    );


    mLabelPuntoBX =
        new QLabel(
            "X: ---",
            panelNavegacion
        );

    mLabelPuntoBY =
        new QLabel(
            "Y: ---",
            panelNavegacion
        );

    panelIzquierdoLayout->addWidget(
        mLabelPuntoBX
    );

    panelIzquierdoLayout->addWidget(
        mLabelPuntoBY
    );
    connect(
        mBotonPuntoB,
        &QPushButton::clicked,
        this,
        [this]()
        {
            mNavigationWidget->
                activarSeleccionPuntoB();
        }
    );

    // ---------------------------------------------------------
    // CALCULATE ROUTE
    // Visual button. It does not run any function yet.
    // ---------------------------------------------------------

    QPushButton *botonCalcularRuta =
        new QPushButton(
            "CALCULATE ROUTE",
            panelNavegacion
        );

    botonCalcularRuta->setMinimumHeight(
        45
    );

    panelIzquierdoLayout->addWidget(
        botonCalcularRuta
    );
    connect(
        botonCalcularRuta,
        &QPushButton::clicked,
        this,
        [this]()
        {
            mNavigationWidget->
                calcularRuta();
        }
    );

    // ---------------------------------------------------------
    // START ROUTE
    // Keeps exactly the same functionality as ADVANCE.
    // ---------------------------------------------------------

    mBotonAvanzar =
        new QPushButton(
            "START ROUTE",
            mPanelIzquierdoNavegacion
        );

    mBotonAvanzar->setMinimumHeight(
        45
    );

    panelIzquierdoLayout->addWidget(
        mBotonAvanzar
    );

    connect(
        mBotonAvanzar,
        &QPushButton::clicked,
        this,
        &MainWindow::avanzarRobot
    );


    // =========================================================
    // RIGHT BAR
    // =========================================================

    // ---------------------------------------------------------
    // Area selection to generate contour
    // ---------------------------------------------------------

    QPushButton *botonSeleccionarContorno =
        new QPushButton(
            "SELECT CONTOUR",
            panelNavegacion
        );

    botonSeleccionarContorno->setMinimumHeight(
        45
    );

    panelLayout->addWidget(
        botonSeleccionarContorno
    );


    // ---------------------------------------------------------
    // Visual separator
    // ---------------------------------------------------------

    panelLayout->addSpacing(
        15
    );


    // ---------------------------------------------------------
    // Pointer position
    // ---------------------------------------------------------

    QLabel *tituloPuntero =
        new QLabel(
            "POINTER",
            panelNavegacion
        );

    QFont tituloPunteroFont =
        tituloPuntero->font();

    tituloPunteroFont.setBold(true);

    tituloPuntero->setFont(
        tituloPunteroFont
    );

    panelLayout->addWidget(
        tituloPuntero
    );


    QLabel *labelPunteroX =
        new QLabel(
            "X: ---",
            panelNavegacion
        );

    QLabel *labelPunteroY =
        new QLabel(
            "Y: ---",
            panelNavegacion
        );

    panelLayout->addWidget(
        labelPunteroX
    );

    panelLayout->addWidget(
        labelPunteroY
    );


    // ---------------------------------------------------------
    // DELETE SELECTED
    // ---------------------------------------------------------

    QPushButton *botonBorrarNavegacion =
        new QPushButton(
            "DELETE SELECTED",
            panelNavegacion
        );

    botonBorrarNavegacion->setMinimumHeight(
        45
    );

    panelLayout->addWidget(
        botonBorrarNavegacion
    );

  
    // ---------------------------------------------------------
    // MIN Z
    // ---------------------------------------------------------

    QLabel *tituloZMin =
        new QLabel(
            "MIN Z",
            panelNavegacion
        );

    panelLayout->addWidget(
        tituloZMin
    );

    QHBoxLayout *layoutZMin =
        new QHBoxLayout();

    QPushButton *botonZMinAnterior =
        new QPushButton(
            "◀",
            panelNavegacion
        );

    botonZMinAnterior->setAutoRepeat(true);
    botonZMinAnterior->setAutoRepeatDelay(100);
    botonZMinAnterior->setAutoRepeatInterval(2);

    mZMinLabel =
        new QLabel(
            "0.000",
            panelNavegacion
        );

    mZMinLabel->setAlignment(
        Qt::AlignCenter
    );

    QPushButton *botonZMinSiguiente =
        new QPushButton(
            "▶",
            panelNavegacion
        );

    botonZMinSiguiente->setAutoRepeat(true);
    botonZMinSiguiente->setAutoRepeatDelay(100);
    botonZMinSiguiente->setAutoRepeatInterval(2);

    layoutZMin->addWidget(
        botonZMinAnterior
    );

    layoutZMin->addWidget(
        mZMinLabel
    );

    layoutZMin->addWidget(
        botonZMinSiguiente
    );

    panelLayout->addLayout(
        layoutZMin
    );


    // ---------------------------------------------------------
    // MAX Z
    // ---------------------------------------------------------

    QLabel *tituloZMax =
        new QLabel(
            "MAX Z",
            panelNavegacion
        );

    panelLayout->addWidget(
        tituloZMax
    );

    QHBoxLayout *layoutZMax =
        new QHBoxLayout();

    QPushButton *botonZMaxAnterior =
        new QPushButton(
            "◀",
            panelNavegacion
        );

    botonZMaxAnterior->setAutoRepeat(true);
    botonZMaxAnterior->setAutoRepeatDelay(100);
    botonZMaxAnterior->setAutoRepeatInterval(2);

    mZMaxLabel =
        new QLabel(
            "0.000",
            panelNavegacion
        );

    mZMaxLabel->setAlignment(
        Qt::AlignCenter
    );

    QPushButton *botonZMaxSiguiente =
        new QPushButton(
            "▶",
            panelNavegacion
        );

    botonZMaxSiguiente->setAutoRepeat(true);
    botonZMaxSiguiente->setAutoRepeatDelay(100);
    botonZMaxSiguiente->setAutoRepeatInterval(2);

    layoutZMax->addWidget(
        botonZMaxAnterior
    );

    layoutZMax->addWidget(
        mZMaxLabel
    );

    layoutZMax->addWidget(
        botonZMaxSiguiente
    );

    panelLayout->addLayout(
        layoutZMax
    );
    // ---------------------------------------------------------
    // ENABLE ORB-SLAM3
    // ---------------------------------------------------------

    QPushButton *botonActivarOrbslam =
        new QPushButton(
            "ENABLE ORB-SLAM3",
            panelNavegacion
        );

    botonActivarOrbslam->setMinimumHeight(
        45
    );

    panelLayout->addWidget(
        botonActivarOrbslam
    );
    // ---------------------------------------------------------
    // Add left panel
    // ---------------------------------------------------------

    zonaLayout->addWidget(
        mPanelIzquierdoNavegacion
    );

    zonaLayout->addWidget(
        mNavigationWidget,
        1
    );
    // ---------------------------------------------------------
    // Add right panel
    // ---------------------------------------------------------

    zonaLayout->addWidget(
        panelNavegacion
    );

    navegarLayout->addWidget(
        zonaNavegacion,
        1
    );

    // =========================================================
    // STACK
    // =========================================================

    mCentral->addWidget(
        mInicio
    );

    mCentral->addWidget(
        mPaginaNavegar
    );
    mAvisoNavegacion =
        new QLabel(
            mPaginaNavegar
        );

    mAvisoNavegacion->setAlignment(
        Qt::AlignCenter
    );

    mAvisoNavegacion->setWordWrap(
        true
    );

    mAvisoNavegacion->setStyleSheet(
        "QLabel {"
        "background-color: rgba(20,20,20,220);"
        "color: white;"
        "border-radius: 10px;"
        "padding: 12px 20px;"
        "font-size: 15px;"
        "font-weight: bold;"
        "}"
    );

    mAvisoNavegacion->hide();

    setCentralWidget(
        mCentral
    );

    // =========================================================
    // CONNECTIONS
    // =========================================================
    connect(
        botonZMinAnterior,
        &QPushButton::clicked,
        this,
        [this]()
        {
            mNavigationWidget->cambiarZMinimoPublico(
                -1
            );

            mZMinLabel->setText(
                QString::number(
                    mNavigationWidget->getZMinimo(),
                    'f',
                    3
                )
            );
        }
    );

    connect(
        botonZMinSiguiente,
        &QPushButton::clicked,
        this,
        [this]()
        {
            mNavigationWidget->cambiarZMinimoPublico(
                1
            );

            mZMinLabel->setText(
                QString::number(
                    mNavigationWidget->getZMinimo(),
                    'f',
                    3
                )
            );
        }
    );

    connect(
        botonZMaxAnterior,
        &QPushButton::clicked,
        this,
        [this]()
        {
            mNavigationWidget->cambiarZMaximoPublico(
                -1
            );

            mZMaxLabel->setText(
                QString::number(
                    mNavigationWidget->getZMaximo(),
                    'f',
                    3
                )
            );
        }
    );

    connect(
        botonZMaxSiguiente,
        &QPushButton::clicked,
        this,
        [this]()
        {
            mNavigationWidget->cambiarZMaximoPublico(
                1
            );

            mZMaxLabel->setText(
                QString::number(
                    mNavigationWidget->getZMaximo(),
                    'f',
                    3
                )
            );
        }
    );
    connect(
        mBotonCargar,
        &QPushButton::clicked,
        this,
        &MainWindow::cargarMapa
    );

    connect(
        mBotonNavegar,
        &QPushButton::clicked,
        this,
        &MainWindow::mostrarNavegacion
    );

   
    connect(
        botonVolverNavegarSuperior,
        &QPushButton::clicked,
        this,
        &MainWindow::volverInicio
    );


    connect(
        botonBorrarNavegacion,
        &QPushButton::clicked,
        this,
        &MainWindow::borrarSeleccionNavegacion
    );


    connect(
        mNavigationWidget,
        &NavigationWidget::pointSelectionChanged,
        this,
        [this](const std::vector<int> &indices)
        {
            mIndicesSeleccionadosNavegacion =
                indices;
        }
    );
    connect(
        mNavigationWidget,
        &NavigationWidget::puntoBSeleccionado,
        this,
        &MainWindow::actualizarPuntoB
    );


    // ---------------------------------------------------------
    // Enable reception button
    // ---------------------------------------------------------

    connect(
        botonActivarRecepcion,
        &QPushButton::clicked,
        this,
        &MainWindow::activarRecepcion
    );
    connect(
        botonActivarOrbslam,
        &QPushButton::clicked,
        this,
        &MainWindow::activarOrbslam
    );

    // ---------------------------------------------------------
    // Select contour area
    // ---------------------------------------------------------

    connect(
        botonSeleccionarContorno,
        &QPushButton::clicked,
        this,
        [this]()
        {
            mNavigationWidget->activarSeleccionContorno();
        }
    );
    // ---------------------------------------------------------
    // Pointer -> XY coordinates
    // ---------------------------------------------------------

    connect(
        mNavigationWidget,
        &NavigationWidget::pointerPositionChanged,
        this,
        [labelPunteroX, labelPunteroY](
            float x,
            float y
        )
        {
            labelPunteroX->setText(
                QString("X: %1")
                    .arg(
                        x,
                        0,
                        'f',
                        3
                    )
            );

            labelPunteroY->setText(
                QString("Y: %1")
                    .arg(
                        y,
                        0,
                        'f',
                        3
                    )
            );
        }
    );


    mCentral->setCurrentWidget(
        mInicio
    );

    actualizarEstadoMenu();
}

void MainWindow::actualizarPuntoB(
    float x,
    float y
)
{
    mLabelPuntoBX->setText(
        QString("X: %1").arg(x, 0, 'f', 3)
    );

    mLabelPuntoBY->setText(
        QString("Y: %1").arg(y, 0, 'f', 3)
    );
}
void MainWindow::avanzarRobot()
{
    if(!mNavigationWidget)
        return;

    mNavigationWidget->iniciarMovimientoRuta();
}
void MainWindow::actualizarEstadoMenu()
{
    mBotonNavegar->setEnabled(
        mMapaCargado
    );

}


void MainWindow::cargarMapa()
{
    const QString filename =
        QFileDialog::getOpenFileName(
            this,
            "Select ORB-SLAM3 map",
            QDir::homePath(),
            "ORB-SLAM3 Maps (*.osa)"
        );

    if(filename.isEmpty())
        return;

    if(!mAtlasEditor->loadAtlas(
           filename.toStdString()))
    {
        QMessageBox::critical(
            this,
            "Error",
            "Could not load the .osa Atlas."
        );

        return;
    }

    mMapPoints2D =
        mAtlasEditor->getMapPoints2D();

    mMapPointsZ =
        mAtlasEditor->getMapPointsZ();

    mMapaCargado = true;

    actualizarEstadoMenu();

    mCentral->setCurrentWidget(
        mInicio
    );
}


void MainWindow::mostrarNavegacion()
{
    if(!mMapaCargado)
        return;

    mNavigationWidget->setMapPoints2D(
        mMapPoints2D
    );

    mNavigationWidget->setMapPointsZ(
        mMapPointsZ
    );

    mZMinLabel->setText(
        QString::number(
            mNavigationWidget->getZMinimo(),
            'f',
            3
        )
    );

    mZMaxLabel->setText(
        QString::number(
            mNavigationWidget->getZMaximo(),
            'f',
            3
        )
    );

    mCentral->setCurrentWidget(
        mPaginaNavegar
    );
}



void MainWindow::volverInicio()
{
    mCentral->setCurrentWidget(
        mInicio
    );
}


void MainWindow::borrarSeleccionNavegacion()
{
    if(
        mIndicesSeleccionadosNavegacion.empty()
    )
    {
        return;
    }

    mAtlasEditor->borrarMapPoints2D(
        mIndicesSeleccionadosNavegacion
    );

    std::vector<int> indices =
        mIndicesSeleccionadosNavegacion;

    std::sort(
        indices.begin(),
        indices.end(),
        std::greater<int>()
    );

    for(int index : indices)
    {
        if(
            index < 0 ||
            index >=
            static_cast<int>(
                mMapPoints2D.size()
            )
        )
        {
            continue;
        }

        mMapPoints2D.erase(
            mMapPoints2D.begin() + index
        );

        if(
            index <
            static_cast<int>(
                mMapPointsZ.size()
            )
        )
        {
            mMapPointsZ.erase(
                mMapPointsZ.begin() + index
            );
        }
    }

    mIndicesSeleccionadosNavegacion.clear();

    mNavigationWidget->setMapPoints2D(
        mMapPoints2D
    );

    mNavigationWidget->setMapPointsZ(
        mMapPointsZ
    );
}


void MainWindow::activarRecepcion()
{
    if(!mRosNodeHandle)
        return;

    if(mRecepcionActiva)
        return;

    mPoseSubscriber =
        mRosNodeHandle->subscribe(
            "/orb_slam3/camera_pose",
            1,
            &MainWindow::recibirPose,
            this
        );

    mRecepcionActiva = true;

    mLabelEstadoRecepcion->setText(
        "Reception: ACTIVE"
    );

    qDebug()
        << "[ROS] Subscribed to /orb_slam3/camera_pose";
}


void MainWindow::procesarROS()
{

    if(!ros::ok())
        return;

    ros::spinOnce();
}


void MainWindow::recibirPose(
    const geometry_msgs::PoseStamped::ConstPtr &msg
)
{
    const double x =
        msg->pose.position.x;

    const double y =
        msg->pose.position.y;

    const double qx =
        msg->pose.orientation.x;

    const double qy =
        msg->pose.orientation.y;

    const double qz =
        msg->pose.orientation.z;

    const double qw =
        msg->pose.orientation.w;

    /*
     * Quaternion to yaw.
     *
     * yaw = atan2(
     *     2(wz + xy),
     *     1 - 2(y² + z²)
     * )
     */
// =========================================================
// ORIENTATION ACCORDING TO THE ORB-SLAM3 CONVENTION
// =========================================================

    const double forwardX =
        2.0 * (
            qx * qz +
            qw * qy
        );

    const double forwardY =
        2.0 * (
            qy * qz -
            qw * qx
        );

    const double yaw =
        std::atan2(
            forwardY,
            forwardX
        );

    const double yawDeg =
        yaw * 180.0 / M_PI;
    mNavigationWidget->setRobotPose(
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(yaw)
    );
        

    mLabelRobotX->setText(
        QString("X: %1")
            .arg(
                x,
                0,
                'f',
                3
            )
    );

    mLabelRobotY->setText(
        QString("Y: %1")
            .arg(
                y,
                0,
                'f',
                3
            )
    );

    mLabelRobotYaw->setText(
        QString("Yaw: %1°")
            .arg(
                yawDeg,
                0,
                'f',
                2
            )
    );


}
void MainWindow::activarOrbslam()
{
    QTcpSocket socket;

    socket.connectToHost(
        "127.0.0.1",
        11311
    );

    if(!socket.waitForConnected(500))
    {
        qDebug()
            << "[ORBSLAM] ROS Master is not available.";

        QMessageBox::critical(
            this,
            "ENABLE ORB-SLAM3",
            "ROS Master is not running.\n\n"
            "You must run 'roscore' in a terminal "
            "before starting OrbVIsense Navigator."
        );

        QCoreApplication::quit();

        return;
    }

    socket.disconnectFromHost();

    qDebug()
        << "[ORBSLAM] External ROS Master available.";

    if(
        mRosbridgeProcess &&
        mRosbridgeProcess->state() != QProcess::NotRunning
    )
    {
        qDebug()
            << "[ORBSLAM] rosbridge is already running.";

        return;
    }

    if(!mRosbridgeProcess)
    {
        mRosbridgeProcess =
            new QProcess(this);
    }

    qDebug()
        << "[ORBSLAM] Starting rosbridge WebSocket...";

    mRosbridgeProcess->start(
        "/bin/bash",
        QStringList()
            << "-lc"
            << "source /opt/ros/noetic/setup.bash && "
               "source ~/ros1_ws/devel/setup.bash && "
               "roslaunch rosbridge_server rosbridge_websocket.launch"
    );

    if(!mRosbridgeTimer)
    {
        mRosbridgeTimer =
            new QTimer(this);

        connect(
            mRosbridgeTimer,
            &QTimer::timeout,
            this,
            &MainWindow::comprobarRosbridge
        );
    }

    mRosbridgeTimer->start(500);
}

void MainWindow::comprobarRosbridge()
{
    if(!mRosbridgeProcess)
        return;

    if(
        mRosbridgeProcess->state() ==
        QProcess::NotRunning
    )
    {
        mRosbridgeTimer->stop();

        qDebug()
            << "[ORBSLAM] rosbridge exited prematurely.";

        QMessageBox::warning(
            this,
            "ENABLE ORB-SLAM3",
            "rosbridge WebSocket could not be started."
        );

        return;
    }

    QTcpSocket socket;

    socket.connectToHost(
        "127.0.0.1",
        9090
    );

    if(
        socket.waitForConnected(100)
    )
    {
        socket.disconnectFromHost();

        mRosbridgeTimer->stop();

        qDebug()
            << "[ORBSLAM] rosbridge WebSocket available.";
        if(
            mRobotWebSocket &&
            mRobotWebSocket->state() ==
            QAbstractSocket::UnconnectedState
        )
        {
            qDebug()
            //Remember change ip adress
                << "[ROBOT] Connecting to rosbridge:"
                << "ws://10.42.0.1:9090";

            mRobotWebSocket->open(
                QUrl(
                    "ws://10.42.0.1:9090"
                )
            );
        }

        if(
            mJpegProcess &&
            mJpegProcess->state() != QProcess::NotRunning
        )
        {
            qDebug()
                << "[ORBSLAM] jpeg_to_mono is already running.";

            return;
        }

        if(!mJpegProcess)
        {
            mJpegProcess =
                new QProcess(this);
        }

        qDebug()
            << "[ORBSLAM] Starting jpeg_to_mono...";

        mJpegProcess->start(
            "/bin/bash",
            QStringList()
                << "-lc"
                << "source /opt/ros/noetic/setup.bash && "
                   "source ~/ros1_ws/devel/setup.bash && "
                   "rosrun jpeg_to_mono jpeg_to_mono_node"
        );

        if(!mJpegTimer)
        {
            mJpegTimer =
                new QTimer(this);

            connect(
                mJpegTimer,
                &QTimer::timeout,
                this,
                &MainWindow::comprobarJpeg
            );
        }

        mJpegTimer->start(500);
    }
}
void MainWindow::comprobarJpeg()
{
    if(!mJpegProcess)
        return;

    if(
        mJpegProcess->state() ==
        QProcess::NotRunning
    )
    {
        mJpegTimer->stop();

        qDebug()
            << "[ORBSLAM] jpeg_to_mono exited prematurely.";

        QMessageBox::warning(
            this,
            "ENABLE ORB-SLAM3",
            "jpeg_to_mono could not be started."
        );

        return;
    }

    mJpegTimer->stop();

    qDebug()
        << "[ORBSLAM] jpeg_to_mono active.";

    if(
        mOrbslamProcess &&
        mOrbslamProcess->state() != QProcess::NotRunning
    )
    {
        qDebug()
            << "[ORBSLAM] ORB-SLAM3 is already running.";

        return;
    }

    if(!mOrbslamProcess)
    {
        mOrbslamProcess =
            new QProcess(this);
    }

    mOrbslamProcess->setProcessChannelMode(
        QProcess::ForwardedChannels
    );

    qDebug()
        << "[ORBSLAM] Starting ORB-SLAM3...";

    mOrbslamProcess->start(
        "/bin/bash",
        QStringList()
            << "-lc"
            << "source /opt/ros/noetic/setup.bash && "
               "source ~/ros1_ws/devel/setup.bash && "
               "exec roslaunch orb_slam3_ros_wrapper euroc_monoimu.launch"
    );

    if(!mOrbslamTimer)
    {
        mOrbslamTimer =
            new QTimer(this);

        connect(
            mOrbslamTimer,
            &QTimer::timeout,
            this,
            &MainWindow::comprobarOrbslam
        );
    }

    mOrbslamTimer->start(500);
}
void MainWindow::comprobarOrbslam()
{
    if(!mOrbslamProcess)
        return;

    if(
        mOrbslamProcess->state() ==
        QProcess::NotRunning
    )
    {
        mOrbslamTimer->stop();

        qDebug()
            << "[ORBSLAM] ORB-SLAM3 exited prematurely.";

        QMessageBox::warning(
            this,
            "ENABLE ORB-SLAM3",
            "ORB-SLAM3 could not be started."
        );

        return;
    }

    mOrbslamTimer->stop();

    qDebug()
        << "[ORBSLAM] ORB-SLAM3 active.";
    mPanelIzquierdoNavegacion->show();

    mNavigationWidget->update();
}
void MainWindow::mostrarAvisoNavegacion(
    const QString &mensaje
)
{
    if(!mAvisoNavegacion)
        return;

    mAvisoNavegacion->setText(
        mensaje
    );

    mAvisoNavegacion->adjustSize();

    const int x =
        (mPaginaNavegar->width() -
         mAvisoNavegacion->width()) / 2;

    const int y =
        30;

    mAvisoNavegacion->move(
        x,
        y
    );

    mAvisoNavegacion->show();
    mAvisoNavegacion->raise();

    QTimer::singleShot(
        3500,
        this,
        [this]()
        {
            if(mAvisoNavegacion)
                mAvisoNavegacion->hide();
        }
    );
}
