#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <QWebSocket>
#include <QLabel>
#include <vector>

class AtlasEditor;
class NavigationWidget;
class QStackedWidget;
class QWidget;
class QPushButton;
class QLabel;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(
        QWidget *parent = nullptr
    );

    ~MainWindow();

private slots:

    void cargarMapa();

    void mostrarNavegacion();


    void volverInicio();

    void activarRecepcion();
    void activarOrbslam();

    void procesarROS();
    void comprobarRosbridge();
    void comprobarJpeg();
    void comprobarOrbslam();
    void actualizarPuntoB(float x, float y);
    void avanzarRobot();
    
private:

    void construirInterfaz();

    void actualizarEstadoMenu();

    void recibirPose(
        const geometry_msgs::PoseStamped::ConstPtr &msg
    );

    QWidget *mInicio;

    QWidget *mPaginaNavegar;

    QStackedWidget *mCentral;

    QPushButton *mBotonCargar;

    QPushButton *mBotonNavegar;


    AtlasEditor *mAtlasEditor;

    NavigationWidget *mNavigationWidget;

    bool mMapaCargado;

    void borrarSeleccionNavegacion();

    std::vector<int>mIndicesSeleccionadosNavegacion;
    std::vector<QPointF> mMapPoints2D;
    std::vector<float> mMapPointsZ;

    ros::NodeHandle *mRosNodeHandle;

    ros::Subscriber mPoseSubscriber;

    QProcess *mRosbridgeProcess;
    QProcess *mJpegProcess;
    QProcess *mOrbslamProcess;
    
    QTimer *mRosTimer;
    QTimer *mRutaTimer;
    QTimer *mRosbridgeTimer;
    QTimer *mJpegTimer;
    QTimer *mOrbslamTimer;

    bool mRecepcionActiva;
    bool mCalibrandoRobot;

    QLabel *mLabelRobotX;

    QLabel *mLabelRobotY;

    QLabel *mLabelRobotYaw;

    QLabel *mLabelEstadoRecepcion;
    QLabel *mZMinLabel;
    QLabel *mZMaxLabel;
    QPushButton *mBotonPuntoB;
    QPushButton *mBotonAvanzar;
    QWebSocket *mRobotWebSocket;
    QLabel *mLabelPuntoBX;
    QLabel *mLabelPuntoBY;
    QWidget *mPanelIzquierdoNavegacion;
    QLabel *mAvisoNavegacion;
    void mostrarAvisoNavegacion(
        const QString &mensaje
    );
};

#endif
