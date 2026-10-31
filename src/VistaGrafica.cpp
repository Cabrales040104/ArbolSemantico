#include "VistaGrafica.hpp"

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QResizeEvent>
#include <QTextDocument>
#include <QTransform>
#include <QWheelEvent>

#include <algorithm>

namespace LogicaArbol {
namespace {

QColor colorDelSubarbol(const std::shared_ptr<NodoTableau>& nodo)
{
    if (nodo->hijos.empty()) {
        if (nodo->estado == EstadoRama::Abierta) return QColor("#3ee6a8");
        if (nodo->estado == EstadoRama::Cerrada) return QColor("#ff5273");
        return QColor("#ff1744");
    }

    QColor color = colorDelSubarbol(nodo->hijos.front());
    for (std::size_t indice = 1; indice < nodo->hijos.size(); ++indice) {
        if (colorDelSubarbol(nodo->hijos[indice]) != color) return QColor("#ff1744");
    }
    return color;
}

}

VistaGrafica::VistaGrafica(QWidget* padre)
    : QGraphicsView(padre), escena_(new QGraphicsScene(this))
{
    setScene(escena_);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setBackgroundBrush(QColor("#0c0c10"));
}

qreal VistaGrafica::asignarPosiciones(const std::shared_ptr<NodoTableau>& nodo,
    int profundidad, int& hoja)
{
    constexpr qreal separacionHorizontal = 350.0;
    constexpr qreal separacionVertical = 190.0;
    qreal centro = 0.0;
    if (nodo->hijos.empty()) {
        centro = static_cast<qreal>(hoja++) * separacionHorizontal;
    } else {
        qreal suma = 0.0;
        for (const auto& hijo : nodo->hijos) suma += asignarPosiciones(hijo, profundidad + 1, hoja);
        centro = suma / static_cast<qreal>(nodo->hijos.size());
    }
    posiciones_[nodo->id] = QPointF(centro, static_cast<qreal>(profundidad) * separacionVertical);
    return centro;
}

void VistaGrafica::dibujarConexiones(const std::shared_ptr<NodoTableau>& nodo)
{
    constexpr qreal altoNodo = 142.0;
    const QPointF origen = posiciones_.at(nodo->id);

    auto dibujarRama = [this](const QPainterPath& camino, qreal ancho) {
        QColor haloColor("#ff1744");
        haloColor.setAlpha(48);
        auto* halo = escena_->addPath(camino,
            QPen(haloColor, ancho + 10.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        halo->setZValue(-2.2);
        auto* base = escena_->addPath(camino,
            QPen(QColor("#8f102c"), ancho + 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        base->setZValue(-2.1);
        auto* rama = escena_->addPath(camino,
            QPen(QColor("#ff1744"), ancho, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        rama->setZValue(-2.0);
    };

    const QPointF inicio(origen.x(), origen.y() + altoNodo / 2.0);
    if (nodo->hijos.size() == 1) {
        const QPointF destino = posiciones_.at(nodo->hijos.front()->id);
        const QPointF fin(destino.x(), destino.y() - altoNodo / 2.0);
        const qreal tramo = fin.y() - inicio.y();
        QPainterPath tronco(inicio);
        tronco.cubicTo(inicio.x() - 6.0, inicio.y() + tramo * 0.32,
            fin.x() + 6.0, fin.y() - tramo * 0.32, fin.x(), fin.y());
        dibujarRama(tronco, 7.0);
    } else if (nodo->hijos.size() > 1) {
        const QPointF bifurcacion(inicio.x() + 4.0, inicio.y() + 46.0);
        QPainterPath tronco(inicio);
        tronco.cubicTo(inicio.x() - 7.0, inicio.y() + 18.0,
            bifurcacion.x() + 8.0, bifurcacion.y() - 18.0,
            bifurcacion.x(), bifurcacion.y());
        dibujarRama(tronco, 11.0);

        for (const auto& hijo : nodo->hijos) {
            const QPointF destino = posiciones_.at(hijo->id);
            const QPointF fin(destino.x(), destino.y() - altoNodo / 2.0);
            const qreal tramo = fin.y() - bifurcacion.y();
            QPainterPath rama(bifurcacion);
            const qreal direccion = fin.x() < bifurcacion.x() ? -1.0 : 1.0;
            rama.cubicTo(bifurcacion.x() + direccion * 8.0, bifurcacion.y() + tramo * 0.24,
                fin.x() - direccion * 38.0, fin.y() - tramo * 0.3, fin.x(), fin.y());
            dibujarRama(rama, 6.0);

            auto* nudo = escena_->addEllipse(bifurcacion.x() - 5.0,
                bifurcacion.y() - 5.0, 10.0, 10.0,
                QPen(QColor("#ff8197"), 1.2), QBrush(QColor("#ff1744")));
            nudo->setZValue(-0.9);

            const QColor hoja = colorDelSubarbol(hijo);
            QPainterPath brote;
            const QPointF centroHoja(bifurcacion.x() + direccion * 17.0,
                bifurcacion.y() + 12.0);
            brote.moveTo(centroHoja.x(), centroHoja.y() - 8.0);
            brote.cubicTo(centroHoja.x() + 8.0, centroHoja.y() - 7.0,
                centroHoja.x() + 9.0, centroHoja.y() + 6.0,
                centroHoja.x(), centroHoja.y() + 8.0);
            brote.cubicTo(centroHoja.x() - 8.0, centroHoja.y() + 5.0,
                centroHoja.x() - 7.0, centroHoja.y() - 6.0,
                centroHoja.x(), centroHoja.y() - 8.0);
            brote.closeSubpath();
            QTransform giro;
            giro.translate(centroHoja.x(), centroHoja.y());
            giro.rotate(direccion * 42.0);
            giro.translate(-centroHoja.x(), -centroHoja.y());
            auto* hojaItem = escena_->addPath(giro.map(brote), QPen(hoja.lighter(125), 1.0),
                QBrush(hoja));
            hojaItem->setZValue(-0.5);
        }
    }

    for (const auto& hijo : nodo->hijos) {
        dibujarConexiones(hijo);
    }
}

void VistaGrafica::dibujarNodos(const std::shared_ptr<NodoTableau>& nodo)
{
    constexpr qreal anchoNodo = 252.0;
    constexpr qreal altoNodo = 142.0;
    const QPointF centro = posiciones_.at(nodo->id);
    QColor colorBorde("#ff1744");
    QColor colorFondo("#151116");
    QColor colorLuz("#35121e");
    if (nodo->estado == EstadoRama::Cerrada) {
        colorBorde = QColor("#ff5273");
        colorFondo = QColor("#241017");
        colorLuz = QColor("#501322");
    } else if (nodo->estado == EstadoRama::Abierta) {
        colorBorde = QColor("#3ee6a8");
        colorFondo = QColor("#101a17");
        colorLuz = QColor("#153329");
    } else if (nodo->aplicada) {
        colorBorde = QColor("#ff4265");
        colorFondo = QColor("#1d1117");
        colorLuz = QColor("#401421");
    }

    QPainterPath forma;
    const QRectF bounds(centro.x() - anchoNodo / 2.0,
        centro.y() - altoNodo / 2.0, anchoNodo, altoNodo);
    forma.moveTo(bounds.left() + 13.0, bounds.top());
    forma.lineTo(bounds.right() - 13.0, bounds.top());
    forma.quadTo(bounds.right(), bounds.top(), bounds.right(), bounds.top() + 13.0);
    forma.lineTo(bounds.right(), bounds.bottom() - 13.0);
    forma.quadTo(bounds.right(), bounds.bottom(), bounds.right() - 13.0, bounds.bottom());
    forma.lineTo(bounds.left() + 13.0, bounds.bottom());
    forma.quadTo(bounds.left(), bounds.bottom(), bounds.left(), bounds.bottom() - 13.0);
    forma.lineTo(bounds.left(), bounds.top() + 13.0);
    forma.quadTo(bounds.left(), bounds.top(), bounds.left() + 13.0, bounds.top());
    forma.closeSubpath();

    QLinearGradient degradado(bounds.topLeft(), bounds.bottomRight());
    degradado.setColorAt(0.0, colorLuz);
    degradado.setColorAt(0.35, colorFondo);
    degradado.setColorAt(1.0, QColor("#0b0a0d"));
    auto* sombra = escena_->addPath(forma.translated(0.0, 5.0),
        QPen(QColor("#050607"), 1.0), QBrush(QColor(0, 0, 0, 100)));
    sombra->setZValue(0.8);
    auto* recuadro = escena_->addPath(forma, QPen(colorBorde, 2.4), QBrush(degradado));
    recuadro->setZValue(1.0);

    QString estado;
    if (nodo->estado == EstadoRama::Cerrada) estado = QStringLiteral("✕ RAMA CERRADA\nContradicción: ")
        + QString::fromUtf8(nodo->contradiccion.c_str());
    else if (nodo->estado == EstadoRama::Abierta) estado = QStringLiteral("✓ RAMA ABIERTA");
    else if (nodo->aplicada) estado = QStringLiteral("✓ Aplicada")
        + (nodo->reglaDeExpansion.empty() ? QString() : QStringLiteral(" · ")
            + QString::fromUtf8(nodo->reglaDeExpansion.c_str()));
    else if (esLiteral(nodo->formula)) estado = QStringLiteral("Literal");
    else estado = QStringLiteral("Pendiente");

    const QString encabezado = nodo->etiqueta.empty()
        ? QStringLiteral("PASO %1").arg(nodo->id)
        : QString::fromUtf8(nodo->etiqueta.c_str());
    const QString formula = QString::fromUtf8(escribirFormula(nodo->formula).c_str());
    QString colorEstado = QStringLiteral("#c6bbc0");
    if (nodo->estado == EstadoRama::Cerrada) colorEstado = QStringLiteral("#ff718b");
    else if (nodo->estado == EstadoRama::Abierta) colorEstado = QStringLiteral("#3ee6a8");

    const QString contenido = QStringLiteral(
        "<div style='text-align:center; font-family:Segoe UI;'>"
        "<span style='font-size:8pt; font-weight:700; letter-spacing:1px; color:#ff9bad;'>%1</span>"
        "<br><span style='font-size:15pt; font-weight:700; color:#ffffff;'>%2</span>"
        "<br><span style='font-size:8pt; font-weight:600; color:%3;'>%4</span></div>")
        .arg(encabezado.toHtmlEscaped())
        .arg(formula.toHtmlEscaped())
        .arg(colorEstado)
        .arg(estado.toHtmlEscaped());
    QFont fuente(QStringLiteral("Segoe UI"), 9);
    fuente.setStyleStrategy(QFont::PreferAntialias);
    auto* texto = escena_->addText(QString(), fuente);
    texto->setHtml(contenido);
    texto->setTextWidth(anchoNodo - 44.0);
    texto->setPos(centro.x() - texto->textWidth() / 2.0,
        centro.y() - texto->boundingRect().height() / 2.0);
    texto->setZValue(2.0);

    if (nodo->hijos.empty()
        && (nodo->estado == EstadoRama::Abierta || nodo->estado == EstadoRama::Cerrada)) {
        const bool abierta = nodo->estado == EstadoRama::Abierta;
        const QColor marcador = abierta ? QColor("#3ee6a8") : QColor("#ff5273");
        const QRectF etiqueta(bounds.right() - 11.0, bounds.top() - 9.0, 20.0, 20.0);
        auto* halo = escena_->addEllipse(etiqueta.adjusted(-4.0, -4.0, 4.0, 4.0),
            QPen(QColor(marcador.red(), marcador.green(), marcador.blue(), 70), 4.0),
            QBrush(QColor(0, 0, 0, 0)));
        halo->setZValue(2.1);
        auto* indicador = escena_->addEllipse(etiqueta,
            QPen(marcador, 1.5), QBrush(QColor("#0c0c10")));
        indicador->setZValue(2.2);
        auto* textoEstado = escena_->addText(abierta ? QStringLiteral("✓") : QStringLiteral("×"),
            QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        textoEstado->setDefaultTextColor(marcador);
        textoEstado->setPos(etiqueta.left() + 3.0, etiqueta.top() + 1.0);
        textoEstado->setZValue(2.3);
    }

    for (const auto& hijo : nodo->hijos) dibujarNodos(hijo);
}

void VistaGrafica::mostrar(const ResultadoTableau& resultado)
{
    escena_->clear();
    posiciones_.clear();
    if (!resultado.raiz) return;

    int hoja = 0;
    asignarPosiciones(resultado.raiz, 0, hoja);
    dibujarConexiones(resultado.raiz);
    dibujarNodos(resultado.raiz);
    escena_->setSceneRect(escena_->itemsBoundingRect().adjusted(-48.0, -48.0, 48.0, 48.0));
    ajustarAlContenido();
}

void VistaGrafica::wheelEvent(QWheelEvent* evento)
{
    const qreal factor = evento->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    scale(factor, factor);
    evento->accept();
}

void VistaGrafica::resizeEvent(QResizeEvent* evento)
{
    QGraphicsView::resizeEvent(evento);
    ajustarAlContenido();
}

void VistaGrafica::ajustarAlContenido()
{
    if (!escena_->items().empty()) fitInView(escena_->sceneRect(), Qt::KeepAspectRatio);
}

}