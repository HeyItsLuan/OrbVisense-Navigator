#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include <QWidget>
#include <QPointF>
#include <vector>
#include <QTimer>
#include <QString>
class NavigationWidget : public QWidget
{
    Q_OBJECT

public:

    explicit NavigationWidget(
        QWidget *parent = nullptr
    );
    ~NavigationWidget();
    void setRobotPose(
        float x,
        float y,
        float yaw
    );
    void ejecutarPasoRobot(
        int left,
        int right
    );
    void setMapPoints2D(
        const std::vector<QPointF> &points
    );
    void setMapPointsZ(
        const std::vector<float> &zValues
    );
    void activarSeleccionContorno();
    void cambiarZMinimoPublico(
        int direccion
    );
    void cambiarZMaximoPublico(
        int direccion
    );
    float getZMinimo() const;
    float getZMaximo() const;
    void activarSeleccionPuntoB();
    void calcularRuta();
    float distanciaPuntoSegmento(
        const QPointF &punto,
        const QPointF &inicio,
        const QPointF &fin
    ) const;
    bool segmentoSeguroParaRobot(
        const QPointF &inicio,
        const QPointF &fin
    ) const;
    bool puntoDentroContorno(
        const QPointF &punto
    ) const;
    bool segmentoDentroContorno(
        const QPointF &inicio,
        const QPointF &fin
    ) const;
    void actualizarRutaConRobot();
    float getPuntoBX() const;
    float getPuntoBY() const;
    bool puntoBValido() const;
    void iniciarMovimientoRuta();

signals:

    void pointerPositionChanged(
        float x,
        float y
    );
    void pointSelectionChanged(
        const std::vector<int> &indices
    );
    void robotCommand(
        int left,
        int right
    );
    void puntoBSeleccionado(float x, float y);
    void avisoNavegacion(const QString &mensaje);

protected:

    void paintEvent(
        QPaintEvent *event
    ) override;

    void mouseMoveEvent(
        QMouseEvent *event
    ) override;

    void mousePressEvent(
        QMouseEvent *event
    ) override;

    void mouseReleaseEvent(
        QMouseEvent *event
    ) override;

private:

    float mRobotX;
    float mRobotY;
    float mRobotYaw;

    float mPointerX;
    float mPointerY;

    bool mPointerValid;

    std::vector<QPointF> mMapPoints;
    std::vector<float> mMapPointsZ;

    std::vector<float> mZLevels;

    int mZMinIndex;
    int mZMaxIndex;

    std::vector<bool> mSelectedPoints;

    std::vector<QPointF> mContour;

    float mMinX;
    float mMaxX;
    float mMinY;
    float mMaxY;

    bool mSelecting;

    QPointF mSelectionStart;
    QPointF mSelectionEnd;

    bool mSeleccionandoContorno;

    QPointF mContourSeed;
    bool mContourSeedValid;

    void rebuildContour();

    void emitSelectionChanged();

    QPointF worldToScreen(
        const QPointF &point
    ) const;

    QPointF screenToWorld(
        const QPointF &point
    ) const;
    void actualizarFiltroZ();

    void emitirFiltroZ();

    void cambiarZMinimo(
        int direccion
    );

    void cambiarZMaximo(
        int direccion
    );
    bool mSeleccionandoPuntoB;
    QPointF mPuntoB;
    bool mPuntoBValido;
    std::vector<QPointF> mRuta;
    bool mRutaActiva;
    float mToleranciaRuta;
    float mToleranciaDestino;

    static constexpr float mRobotScale = 0.10f;
    static constexpr float mRobotWidth = mRobotScale * 1.30f;
    static constexpr float mZonaAproximacion = mRobotScale * 0.50f;

    bool mRobotStepActivo;
    void continuarMovimientoRuta();

    int mPasosMovimiento;
    bool mMovimientoRutaActivo;
    int mIndiceRutaActual;
    bool mEsperandoConfirmacionDestino;
};
#endif
