#pragma once

#include "MotorDeTableau.hpp"

#include <QGraphicsView>
#include <map>

class QGraphicsScene;
class QResizeEvent;
class QWheelEvent;

namespace LogicaArbol {

class VistaGrafica : public QGraphicsView {
public:
    explicit VistaGrafica(QWidget* padre = nullptr);
    void mostrar(const ResultadoTableau& resultado);

protected:
    void wheelEvent(QWheelEvent* evento) override;
    void resizeEvent(QResizeEvent* evento) override;

private:
    QGraphicsScene* escena_;
    std::map<int, QPointF> posiciones_;

    qreal asignarPosiciones(const std::shared_ptr<NodoTableau>& nodo, int profundidad, int& hoja);
    void dibujarConexiones(const std::shared_ptr<NodoTableau>& nodo);
    void dibujarNodos(const std::shared_ptr<NodoTableau>& nodo);
    void ajustarAlContenido();
};

}