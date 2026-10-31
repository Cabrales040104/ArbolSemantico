#include "VentanaPrincipal.hpp"

#include "VistaGrafica.hpp"

#include <QHeaderView>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <exception>
#include <map>
#include <optional>

namespace LogicaArbol {
namespace {

QString convertir(const std::string& texto)
{
    return QString::fromUtf8(texto.data(), static_cast<int>(texto.size()));
}

QFrame* crearTarjeta(const QString& distintivo, const QString& titulo,
    const QString& detalle, const QString& color, QWidget* padre)
{
    auto* tarjeta = new QFrame(padre);
    tarjeta->setObjectName(QStringLiteral("tarjetaDato"));
    tarjeta->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto* fila = new QHBoxLayout(tarjeta);
    fila->setContentsMargins(14, 11, 14, 11);
    fila->setSpacing(14);

    auto* insignia = new QLabel(distintivo, tarjeta);
    insignia->setObjectName(QStringLiteral("insigniaTarjeta"));
    insignia->setAlignment(Qt::AlignCenter);
    insignia->setMinimumSize(54, 46);
    insignia->setMaximumWidth(72);
    insignia->setStyleSheet(QStringLiteral(
        "QLabel#insigniaTarjeta { background:%1; color:#fff7f8; border-radius:9px;"
        " font-size:12pt; font-weight:700; padding:4px; }").arg(color));

    auto* textos = new QVBoxLayout;
    textos->setSpacing(4);
    auto* etiqueta = new QLabel(titulo, tarjeta);
    etiqueta->setObjectName(QStringLiteral("tituloTarjeta"));
    etiqueta->setWordWrap(true);
    auto* descripcion = new QLabel(detalle, tarjeta);
    descripcion->setObjectName(QStringLiteral("detalleTarjeta"));
    descripcion->setWordWrap(true);
    textos->addWidget(etiqueta);
    textos->addWidget(descripcion);
    fila->addWidget(insignia, 0, Qt::AlignTop);
    fila->addLayout(textos, 1);
    return tarjeta;
}

QFrame* crearTarjetaConector(const QString& simbolo, const QString& nombre,
    const QString& ejemplo, QWidget* padre)
{
    auto* tarjeta = new QFrame(padre);
    tarjeta->setObjectName(QStringLiteral("tarjetaConector"));
    auto* fila = new QHBoxLayout(tarjeta);
    fila->setContentsMargins(11, 9, 11, 9);
    fila->setSpacing(10);

    auto* marca = new QLabel(simbolo, tarjeta);
    marca->setObjectName(QStringLiteral("simboloConector"));
    marca->setAlignment(Qt::AlignCenter);
    marca->setMinimumSize(40, 40);
    auto* textos = new QVBoxLayout;
    textos->setSpacing(2);
    auto* etiqueta = new QLabel(nombre, tarjeta);
    etiqueta->setObjectName(QStringLiteral("nombreConector"));
    auto* uso = new QLabel(ejemplo, tarjeta);
    uso->setObjectName(QStringLiteral("usoConector"));
    uso->setWordWrap(true);
    textos->addWidget(etiqueta);
    textos->addWidget(uso);
    fila->addWidget(marca);
    fila->addLayout(textos, 1);
    return tarjeta;
}

QWidget* crearSeccionLista(const QString& titulo, const QString& descripcion,
    QListWidget* lista, QWidget* padre)
{
    auto* pagina = new QWidget(padre);
    auto* diseno = new QVBoxLayout(pagina);
    diseno->setContentsMargins(16, 16, 16, 16);
    diseno->setSpacing(8);
    auto* encabezado = new QLabel(titulo, pagina);
    encabezado->setObjectName(QStringLiteral("tituloSeccion"));
    auto* ayuda = new QLabel(descripcion, pagina);
    ayuda->setObjectName(QStringLiteral("ayudaSeccion"));
    ayuda->setWordWrap(true);
    diseno->addWidget(encabezado);
    diseno->addWidget(ayuda);
    diseno->addWidget(lista, 1);
    return pagina;
}

void agregarTarjeta(QListWidget* lista, const QString& distintivo,
    const QString& titulo, const QString& detalle, const QString& color)
{
    auto* elemento = new QListWidgetItem(lista);
    auto* tarjeta = crearTarjeta(distintivo, titulo, detalle, color, lista);
    elemento->setSizeHint(QSize(0, 76));
    lista->setItemWidget(elemento, tarjeta);
}

QString explicacionExpansion(const FormulaPtr& formula)
{
    if (!formula) return QStringLiteral("No hay una fórmula para desarrollar.");
    if (formula->conector == Conector::Negacion && formula->izquierda) {
        switch (formula->izquierda->conector) {
        case Conector::Negacion:
            return QStringLiteral("Doble negación: se elimina “no no”.");
        case Conector::Conjuncion:
            return QStringLiteral("Negación de “y”: la rama se divide entre las dos posibilidades negadas.");
        case Conector::Disyuncion:
            return QStringLiteral("Negación de “o”: ambas partes negadas siguen en la misma rama.");
        case Conector::Implicacion:
            return QStringLiteral("Negación de una implicación: se agregan el antecedente y la conclusión negada.");
        case Conector::Bicondicional:
            return QStringLiteral("Negación de “si y solo si”: la rama se divide en dos posibilidades.");
        case Conector::Atomo:
            break;
        }
    }

    switch (formula->conector) {
    case Conector::Conjuncion:
        return QStringLiteral("“Y”: ambas partes se agregan a la misma rama.");
    case Conector::Disyuncion:
        return QStringLiteral("“O”: se abre una rama para cada alternativa.");
    case Conector::Implicacion:
        return QStringLiteral("“Si… entonces”: se abren las opciones “no se cumple el antecedente” o “se cumple la conclusión”.");
    case Conector::Bicondicional:
        return QStringLiteral("“Si y solo si”: se abren dos combinaciones posibles.");
    case Conector::Atomo:
    case Conector::Negacion:
        break;
    }
    return QStringLiteral("La fórmula permanece en esta rama.");
}

QString estadoDe(const NodoTableau& nodo)
{
    if (nodo.estado == EstadoRama::Cerrada) return QStringLiteral("Rama cerrada");
    if (nodo.estado == EstadoRama::Abierta) return QStringLiteral("Rama abierta");
    if (nodo.aplicada) return QStringLiteral("Aplicada");
    if (esLiteral(nodo.formula)) return QStringLiteral("Literal");
    return QStringLiteral("Pendiente");
}

std::optional<bool> evaluarFormula(const FormulaPtr& formula,
    const std::map<std::string, bool>& valores)
{
    if (!formula) return std::nullopt;
    if (formula->conector == Conector::Atomo) {
        const auto valor = valores.find(formula->letra);
        return valor == valores.end() ? std::optional<bool>{} : valor->second;
    }

    const auto izquierda = evaluarFormula(formula->izquierda, valores);
    if (formula->conector == Conector::Negacion) {
        return izquierda.has_value() ? std::optional<bool>{ !*izquierda } : std::nullopt;
    }

    const auto derecha = evaluarFormula(formula->derecha, valores);
    switch (formula->conector) {
    case Conector::Conjuncion:
        if ((izquierda.has_value() && !*izquierda) || (derecha.has_value() && !*derecha)) return false;
        if (izquierda.has_value() && derecha.has_value()) return true;
        return std::nullopt;
    case Conector::Disyuncion:
        if ((izquierda.has_value() && *izquierda) || (derecha.has_value() && *derecha)) return true;
        if (izquierda.has_value() && derecha.has_value()) return false;
        return std::nullopt;
    case Conector::Implicacion:
        if ((izquierda.has_value() && !*izquierda) || (derecha.has_value() && *derecha)) return true;
        if (izquierda.has_value() && derecha.has_value()) return false;
        return std::nullopt;
    case Conector::Bicondicional:
        if (izquierda.has_value() && derecha.has_value()) return *izquierda == *derecha;
        return std::nullopt;
    case Conector::Atomo:
    case Conector::Negacion:
        break;
    }
    return std::nullopt;
}

}

VentanaPrincipal::VentanaPrincipal(QWidget* padre)
    : QMainWindow(padre), entrada_(new QPlainTextEdit), estadoApp_(new QLabel),
      pestanas_(new QTabWidget),
      tablaAtomos_(new QTableWidget), listaFormulas_(new QListWidget),
      vistaGrafica_(new VistaGrafica), arbolDetalle_(new QTreeWidget), resultados_(new QTextEdit)
{
    setWindowTitle(QStringLiteral("LogicaArbol - Arbol semantico"));
    resize(1180, 800);
    setMinimumSize(820, 600);

    auto* contenedor = new QWidget(this);
    auto* diseno = new QVBoxLayout(contenedor);
    diseno->setContentsMargins(20, 16, 20, 18);
    diseno->setSpacing(12);

    auto* encabezado = new QFrame(contenedor);
    encabezado->setObjectName(QStringLiteral("barraSuperior"));
    auto* disenoEncabezado = new QHBoxLayout(encabezado);
    disenoEncabezado->setContentsMargins(18, 11, 18, 11);
    disenoEncabezado->setSpacing(14);

    auto* identidad = new QVBoxLayout;
    identidad->setSpacing(1);
    auto* titulo = new QLabel(QStringLiteral("Lógica proposicional"), encabezado);
    titulo->setObjectName(QStringLiteral("titulo"));
    auto* subtitulo = new QLabel(
        QStringLiteral("Analiza reglas y descubre si pueden ser verdaderas al mismo tiempo."),
        encabezado);
    subtitulo->setObjectName(QStringLiteral("subtitulo"));
    identidad->addWidget(titulo);
    identidad->addWidget(subtitulo);
    estadoApp_->setObjectName(QStringLiteral("estadoApp"));
    estadoApp_->setAlignment(Qt::AlignCenter);
    estadoApp_->setText(QStringLiteral("Listo para analizar"));
    disenoEncabezado->addLayout(identidad, 1);
    disenoEncabezado->addWidget(estadoApp_, 0, Qt::AlignVCenter);

    auto* paginaEntrada = new QWidget(pestanas_);
    auto* disenoEntrada = new QVBoxLayout(paginaEntrada);
    disenoEntrada->setContentsMargins(24, 22, 24, 22);
    disenoEntrada->setSpacing(12);
    auto* rotulo = new QLabel(QStringLiteral("Comienza con tus reglas"), paginaEntrada);
    rotulo->setObjectName(QStringLiteral("rotuloEntrada"));
    auto* instrucciones = new QLabel(
        QStringLiteral("Escribe una proposición por línea. Usa palabras como “si… entonces”, “no”, “y” u “o”."),
        paginaEntrada);
    instrucciones->setObjectName(QStringLiteral("instrucciones"));
    instrucciones->setWordWrap(true);
    entrada_->setPlaceholderText(QStringLiteral("Escriba una regla por línea"));
    entrada_->setMinimumHeight(180);
    entrada_->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    entrada_->setPlainText(QStringLiteral(
        "Si es de día, entonces voy a trabajar\n"
        "Si llueve, entonces no voy a trabajar\n"
        "Es de día"));

    auto* pieEntrada = new QHBoxLayout;
    pieEntrada->setSpacing(14);
    auto* ejemplo = new QLabel(
        QStringLiteral("Ejemplo: Si llueve, entonces llevo paraguas  ·  Llueve"),
        paginaEntrada);
    ejemplo->setObjectName(QStringLiteral("ejemploEntrada"));
    ejemplo->setWordWrap(true);
    auto* botonAnalizar = new QPushButton(QStringLiteral("Analizar reglas"), paginaEntrada);
    botonAnalizar->setObjectName(QStringLiteral("botonAnalizar"));
    botonAnalizar->setMinimumHeight(46);
    botonAnalizar->setMinimumWidth(180);
    botonAnalizar->setCursor(Qt::PointingHandCursor);
    pieEntrada->addWidget(ejemplo, 1);
    pieEntrada->addWidget(botonAnalizar);
    disenoEntrada->addWidget(rotulo);
    disenoEntrada->addWidget(instrucciones);
    disenoEntrada->addWidget(entrada_);

    auto* panelConectores = new QFrame(paginaEntrada);
    panelConectores->setObjectName(QStringLiteral("panelConectores"));
    auto* disenoConectores = new QVBoxLayout(panelConectores);
    disenoConectores->setContentsMargins(13, 10, 13, 12);
    disenoConectores->setSpacing(8);
    auto* tituloConectores = new QLabel(QStringLiteral("Conectores que puedes usar"), panelConectores);
    tituloConectores->setObjectName(QStringLiteral("rotuloConectores"));
    auto* rejillaConectores = new QGridLayout;
    rejillaConectores->setContentsMargins(0, 0, 0, 0);
    rejillaConectores->setHorizontalSpacing(8);
    rejillaConectores->setVerticalSpacing(8);
    const QString conectores[][3] = {
        { QStringLiteral("∧"), QStringLiteral("Y"), QStringLiteral("A y B") },
        { QStringLiteral("∨"), QStringLiteral("O"), QStringLiteral("A o B") },
        { QStringLiteral("¬"), QStringLiteral("NO"), QStringLiteral("No A") },
        { QStringLiteral("→"), QStringLiteral("SI… ENTONCES"), QStringLiteral("Si A, entonces B") },
        { QStringLiteral("↔"), QStringLiteral("SI Y SOLO SI"), QStringLiteral("A si y solo si B") }
    };
    for (int indice = 0; indice < 5; ++indice) {
        rejillaConectores->addWidget(
            crearTarjetaConector(conectores[indice][0], conectores[indice][1],
                conectores[indice][2], panelConectores),
            indice / 3, indice % 3);
    }
    disenoConectores->addWidget(tituloConectores);
    disenoConectores->addLayout(rejillaConectores);
    disenoEntrada->addWidget(panelConectores);

    disenoEntrada->addLayout(pieEntrada);

    listaFormulas_->setFrameShape(QFrame::NoFrame);
    listaFormulas_->setSpacing(8);
    listaFormulas_->setUniformItemSizes(false);
    listaFormulas_->setSelectionMode(QAbstractItemView::NoSelection);
    listaFormulas_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    tablaAtomos_->setColumnCount(2);
    tablaAtomos_->setHorizontalHeaderLabels(
        { QStringLiteral("Letra"), QStringLiteral("Significado de la proposición") });
    tablaAtomos_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tablaAtomos_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tablaAtomos_->verticalHeader()->hide();
    tablaAtomos_->setAlternatingRowColors(true);
    tablaAtomos_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaAtomos_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaAtomos_->setShowGrid(false);
    tablaAtomos_->setMinimumHeight(180);
    tablaAtomos_->setStyleSheet(QStringLiteral(
        "QTableWidget { background:#101014; alternate-background-color:#171419;"
        " color:#f1eef0; border:1px solid #332a30; border-radius:8px; gridline-color:#332a30; }"
        "QTableWidget::item { padding:9px; border-bottom:1px solid #282127; }"
        "QTableWidget::item:selected { background:#35111d; color:#ffffff; }"
        "QHeaderView::section { background:#1a1519; color:#ff8197; border:0;"
        " border-bottom:2px solid #9f1838; padding:10px; font-weight:700; }"));
    auto* paginaProposiciones = new QWidget(pestanas_);
    auto* disenoProposiciones = new QVBoxLayout(paginaProposiciones);
    disenoProposiciones->setContentsMargins(24, 22, 24, 22);
    disenoProposiciones->setSpacing(10);
    auto* tituloProposiciones = new QLabel(QStringLiteral("Proposiciones detectadas"), paginaProposiciones);
    tituloProposiciones->setObjectName(QStringLiteral("tituloSeccion"));
    auto* ayudaProposiciones = new QLabel(
        QStringLiteral("Cada idea simple recibe una letra. Las letras son las mismas que aparecen en las reglas y en las asignaciones de valores."),
        paginaProposiciones);
    ayudaProposiciones->setObjectName(QStringLiteral("ayudaSeccion"));
    ayudaProposiciones->setWordWrap(true);
    disenoProposiciones->addWidget(tituloProposiciones);
    disenoProposiciones->addWidget(ayudaProposiciones);
    disenoProposiciones->addWidget(tablaAtomos_, 1);

    arbolDetalle_->setColumnCount(4);
    arbolDetalle_->setHeaderLabels({ QStringLiteral("Paso"), QStringLiteral("Fórmula"),
        QStringLiteral("Qué significa"), QStringLiteral("Estado") });
    arbolDetalle_->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    arbolDetalle_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    arbolDetalle_->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    arbolDetalle_->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    arbolDetalle_->setIndentation(20);
    arbolDetalle_->setAlternatingRowColors(true);
    arbolDetalle_->setWordWrap(true);

    resultados_->setReadOnly(true);
    resultados_->setFont(QFont(QStringLiteral("Segoe UI"), 11));

    auto* paginaResultado = new QWidget(pestanas_);
    auto* disenoResultado = new QVBoxLayout(paginaResultado);
    disenoResultado->setContentsMargins(24, 22, 24, 22);
    disenoResultado->setSpacing(10);
    auto* tituloResultado = new QLabel(QStringLiteral("Resultado"), paginaResultado);
    tituloResultado->setObjectName(QStringLiteral("tituloSeccion"));
    auto* ayudaResultado = new QLabel(
        QStringLiteral("Aquí tienes la conclusión y las interpretaciones que la justifican."),
        paginaResultado);
    ayudaResultado->setObjectName(QStringLiteral("ayudaSeccion"));
    ayudaResultado->setWordWrap(true);
    disenoResultado->addWidget(tituloResultado);
    disenoResultado->addWidget(ayudaResultado);
    disenoResultado->addWidget(resultados_, 1);

    auto* paginaTraduccion = crearSeccionLista(QStringLiteral("De las frases a la lógica"),
        QStringLiteral("Cada regla conserva su texto original y muestra debajo cómo se representa con símbolos."),
        listaFormulas_, pestanas_);
    auto* paginaArbol = new QWidget(pestanas_);
    auto* disenoArbol = new QVBoxLayout(paginaArbol);
    disenoArbol->setContentsMargins(16, 16, 16, 16);
    disenoArbol->setSpacing(8);
    auto* tituloArbol = new QLabel(QStringLiteral("Vista del árbol"), paginaArbol);
    tituloArbol->setObjectName(QStringLiteral("tituloSeccion"));
    auto* ayudaArbol = new QLabel(
        QStringLiteral("Cada bifurcación representa una posibilidad. Las ramas verdes quedan abiertas; las rojas contienen contradicciones."),
        paginaArbol);
    ayudaArbol->setObjectName(QStringLiteral("ayudaSeccion"));
    ayudaArbol->setWordWrap(true);
    disenoArbol->addWidget(tituloArbol);
    disenoArbol->addWidget(ayudaArbol);
    disenoArbol->addWidget(vistaGrafica_, 1);

    auto* paginaPasos = new QWidget(pestanas_);
    auto* disenoPasos = new QVBoxLayout(paginaPasos);
    disenoPasos->setContentsMargins(16, 16, 16, 16);
    disenoPasos->setSpacing(8);
    auto* tituloPasos = new QLabel(QStringLiteral("Desarrollo paso a paso"), paginaPasos);
    tituloPasos->setObjectName(QStringLiteral("tituloSeccion"));
    auto* ayudaPasos = new QLabel(
        QStringLiteral("Abre o cierra las ramas para ver qué regla se aplicó y por qué termina cada posibilidad."),
        paginaPasos);
    ayudaPasos->setObjectName(QStringLiteral("ayudaSeccion"));
    ayudaPasos->setWordWrap(true);
    disenoPasos->addWidget(tituloPasos);
    disenoPasos->addWidget(ayudaPasos);
    disenoPasos->addWidget(arbolDetalle_, 1);

    pestanas_->setDocumentMode(true);
    pestanas_->setMovable(false);
    pestanas_->setUsesScrollButtons(true);
    pestanas_->addTab(paginaEntrada, QStringLiteral("1  Entrada"));
    pestanas_->addTab(paginaResultado, QStringLiteral("2  Resultado"));
    pestanas_->addTab(paginaTraduccion, QStringLiteral("3  Traducción"));
    pestanas_->addTab(paginaProposiciones, QStringLiteral("4  Proposiciones"));
    pestanas_->addTab(paginaArbol, QStringLiteral("5  Árbol"));
    pestanas_->addTab(paginaPasos, QStringLiteral("6  Pasos"));
    pestanas_->setTabToolTip(0, QStringLiteral("Escribe o modifica las reglas para analizar."));
    pestanas_->setTabToolTip(1, QStringLiteral("Consulta la conclusión del análisis."));
    pestanas_->setTabToolTip(2, QStringLiteral("Consulta la traducción lógica de cada regla."));
    pestanas_->setTabToolTip(3, QStringLiteral("Consulta las ideas y las letras asignadas."));
    pestanas_->setTabToolTip(4, QStringLiteral("Explora visualmente las ramas del tableau."));
    pestanas_->setTabToolTip(5, QStringLiteral("Consulta el desarrollo detallado del árbol."));

    diseno->addWidget(encabezado);
    diseno->addWidget(pestanas_, 1);
    setCentralWidget(contenedor);

    connect(botonAnalizar, &QPushButton::clicked, this, [this]() { analizarEntrada(); });
    setStyleSheet(QStringLiteral(
        "QMainWindow { background: #09090c; }"
        "QWidget { color: #f1eef0; font-family: 'Segoe UI'; font-size: 10pt; }"
        "QFrame#barraSuperior { background: #101014; border: 1px solid #302027;"
        " border-left: 3px solid #ff1744; border-radius: 10px; }"
        "QLabel#titulo { color: #fff7f8; font-size: 17pt; font-weight: 700; }"
        "QLabel#subtitulo { color: #aaa1a6; font-size: 9pt; }"
        "QLabel#estadoApp { background: #211017; color: #ff5273; padding: 8px 14px;"
        " border: 1px solid #6f1830; border-radius: 14px; font-weight: 700; }"
        "QLabel#rotuloEntrada { color: #ff4265; font-size: 12pt; font-weight: 700; }"
        "QLabel#instrucciones { color: #aaa1a6; }"
        "QFrame#panelConectores { background:#0d0d11; border:1px solid #30252b;"
        " border-radius:10px; }"
        "QLabel#rotuloConectores { color:#eee5e9; font-size:10pt; font-weight:700; }"
        "QFrame#tarjetaConector { background:#151217; border:1px solid #35252d;"
        " border-radius:8px; }"
        "QFrame#tarjetaConector:hover { background:#21141a; border-color:#ff1744; }"
        "QLabel#simboloConector { background:#32111d; color:#ff4265; border:1px solid #79203a;"
        " border-radius:8px; font-size:15pt; font-weight:700; }"
        "QLabel#nombreConector { color:#f5edf0; font-size:8pt; font-weight:700; }"
        "QLabel#usoConector { color:#aaa1a6; font-size:8pt; }"
        "QLabel#ejemploEntrada { background: #151217; color: #d5cbd0;"
        " border-left: 3px solid #ff1744; padding: 12px; border-radius: 5px; }"
        "QLabel#tituloSeccion { color: #ff4265; font-size: 15pt; font-weight: 700; }"
        "QLabel#ayudaSeccion { color: #aaa1a6; padding-bottom: 4px; }"
        "QFrame#tarjetaDato { background: #141317; border: 1px solid #30252b;"
        " border-radius: 10px; }"
        "QFrame#tarjetaDato:hover { background: #1c151a; border-color: #a51d3b; }"
        "QLabel#tituloTarjeta { color: #fff1f4; font-size: 11pt; font-weight: 700; }"
        "QLabel#detalleTarjeta { color: #c0b5bb; }"
        "QPlainTextEdit, QListWidget, QTreeWidget, QTextEdit { background: #101014;"
        " color: #f1eef0; border: 1px solid #332a30; border-radius: 8px;"
        " selection-background-color: #79152e; selection-color: #ffffff; padding: 7px; }"
        "QListWidget { background: #0c0c10; border: 0; }"
        "QPlainTextEdit:focus { border: 1px solid #ff1744; }"
        "QPushButton#botonAnalizar { background: #ff1744; color: #ffffff; border: 1px solid #ff5273;"
        " padding: 10px 24px; border-radius: 8px; font-weight: 700; }"
        "QPushButton#botonAnalizar:hover { background: #ff3b60; border-color: #ff9aab; }"
        "QPushButton#botonAnalizar:pressed { background: #c90032; }"
        "QTabWidget::pane { background: #101014; border: 1px solid #30252b;"
        " border-radius: 10px; top: -1px; }"
        "QTabBar::tab { background: #141317; color: #aaa1a6; padding: 11px 16px;"
        " margin-right: 4px; border: 1px solid #30252b; border-bottom: 0;"
        " border-top-left-radius: 7px; border-top-right-radius: 7px; }"
        "QTabBar::tab:hover { background: #21151b; color: #ffd5dd; }"
        "QTabBar::tab:selected { background: #101014; color: #ff4265;"
        " border-top: 3px solid #ff1744; font-weight: 700; }"
        "QHeaderView::section { background: #1a1519; color: #ff8197; border: 0;"
        " border-bottom: 2px solid #9f1838; padding: 9px; font-weight: 700; }"
        "QTreeWidget { alternate-background-color: #151318; }"
        "QTreeWidget::item { padding: 7px 4px; border-bottom: 1px solid #282127; }"
        "QTreeWidget::item:selected { background: #35111d; color: #fff1f4; }"
        "QScrollBar:vertical { background: #101014; width: 12px; margin: 2px; }"
        "QScrollBar::handle:vertical { background: #5b2635; min-height: 24px; border-radius: 5px; }"
        "QScrollBar::handle:vertical:hover { background: #ff1744; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"));

    analizarEntrada();
    pestanas_->setCurrentIndex(0);
}

void VentanaPrincipal::mostrarDetalle(const std::shared_ptr<NodoTableau>& nodo,
    QTreeWidget* arbol, int& siguientePaso, QTreeWidgetItem* padre)
{
    auto* item = padre ? new QTreeWidgetItem(padre) : new QTreeWidgetItem(arbol);
    item->setText(0, QString::number(++siguientePaso));
    item->setText(1, convertir(escribirFormula(nodo->formula)));
    QString explicacion;
    if (nodo->estado == EstadoRama::Cerrada) {
        explicacion = QStringLiteral("Se encontró una contradicción: ")
            + convertir(nodo->contradiccion) + QStringLiteral(".");
    } else if (nodo->estado == EstadoRama::Abierta) {
        explicacion = QStringLiteral("La rama termina sin contradicciones; representa un modelo posible.");
    } else if (nodo->aplicada) {
        explicacion = explicacionExpansion(nodo->formula);
    } else if (!nodo->reglaDeExpansion.empty()) {
        explicacion = QStringLiteral("Se añade esta consecuencia al aplicar la regla ")
            + convertir(nodo->reglaDeExpansion) + QStringLiteral(".");
    } else if (!nodo->etiqueta.empty()) {
        explicacion = QStringLiteral("Regla inicial ")
            + convertir(nodo->etiqueta) + QStringLiteral(" de la base de conocimiento.");
    } else {
        explicacion = QStringLiteral("Esta fórmula forma parte de la rama.");
    }
    item->setText(2, explicacion);
    item->setText(3, estadoDe(*nodo));
    if (nodo->estado == EstadoRama::Cerrada) {
        item->setForeground(3, QBrush(QColor("#ff5273")));
        item->setToolTip(3, QStringLiteral("Contradicción: ") + convertir(nodo->contradiccion));
    } else if (nodo->estado == EstadoRama::Abierta) {
        item->setForeground(3, QBrush(QColor("#3ee6a8")));
    }
    for (const auto& hijo : nodo->hijos) mostrarDetalle(hijo, arbol, siguientePaso, item);
}

void VentanaPrincipal::analizarEntrada()
{
    try {
        const BaseLeida base = analizador_.analizar(entrada_->toPlainText().toUtf8().toStdString());
        const ResultadoTableau resultado = motor_.construir(base);

        tablaAtomos_->setRowCount(static_cast<int>(base.atomos.size()));
        for (std::size_t indice = 0; indice < base.atomos.size(); ++indice) {
            const auto& atomo = base.atomos[indice];
            auto* letra = new QTableWidgetItem(convertir(atomo.letra));
            letra->setTextAlignment(Qt::AlignCenter);
            letra->setForeground(QBrush(QColor("#ff5273")));
            QFont fuente(QStringLiteral("Segoe UI"), 11, QFont::Bold);
            letra->setFont(fuente);
            letra->setFlags(letra->flags() & ~Qt::ItemIsEditable);
            auto* significado = new QTableWidgetItem(convertir(atomo.significado));
            significado->setFlags(significado->flags() & ~Qt::ItemIsEditable);
            tablaAtomos_->setItem(static_cast<int>(indice), 0, letra);
            tablaAtomos_->setItem(static_cast<int>(indice), 1, significado);
        }

        listaFormulas_->clear();
        for (const auto& regla : base.reglas) {
            agregarTarjeta(listaFormulas_, QStringLiteral("R%1").arg(regla.numero),
                convertir(regla.original),
                QStringLiteral("Traducción: %1").arg(convertir(escribirFormula(regla.formula))),
                QStringLiteral("#8f1735"));
        }

        vistaGrafica_->mostrar(resultado);
        arbolDetalle_->clear();
        int siguientePaso = 0;
        mostrarDetalle(resultado.raiz, arbolDetalle_, siguientePaso);
        arbolDetalle_->expandToDepth(1);

        const QString colorCerradas = QStringLiteral("#ff4265");
        const QString colorAbiertas = QStringLiteral("#3ee6a8");
        QString resumen = QStringLiteral(
            "<html><body style='font-family:Segoe UI; color:#f1eef0; background:#101014;'>"
            "<h2 style='color:#ff4265; margin-bottom:12px;'>Resumen del análisis</h2>"
            "<table width='100%' cellspacing='6'><tr>"
            "<td style='background:#17151a; padding:10px; border:1px solid #35252d; border-radius:8px;'>"
            "<span style='color:#ff8197; font-size:18pt; font-weight:700;'>%1</span><br>"
            "<span style='color:#aaa1a6;'>Reglas</span></td>"
            "<td style='background:#17151a; padding:10px; border:1px solid #35252d; border-radius:8px;'>"
            "<span style='color:#ff8197; font-size:18pt; font-weight:700;'>%2</span><br>"
            "<span style='color:#aaa1a6;'>Proposiciones</span></td>"
            "<td style='background:#1b1419; padding:10px; border:1px solid #642035; border-radius:8px;'>"
            "<span style='color:%3; font-size:18pt; font-weight:700;'>%4</span><br>"
            "<span style='color:#aaa1a6;'>Ramas cerradas</span></td>"
            "<td style='background:#111b18; padding:10px; border:1px solid #1d5945; border-radius:8px;'>"
            "<span style='color:%5; font-size:18pt; font-weight:700;'>%6</span><br>"
            "<span style='color:#aaa1a6;'>Ramas abiertas</span></td>"
            "</tr></table><br>")
            .arg(base.reglas.size())
            .arg(base.atomos.size())
            .arg(colorCerradas)
            .arg(resultado.cerradas)
            .arg(colorAbiertas)
            .arg(resultado.abiertas);
        if (resultado.abiertas == 0) {
            resumen += QStringLiteral(
                "<p style='background:#281018; color:#ff718b; padding:12px; border:1px solid #8e1c3c; border-radius:8px;"
                " font-weight:700;'>La base de conocimiento es inconsistente.</p>");
            resumen += QStringLiteral(
                "<p>Una rama cerrada contiene una contradicción. Como no hay ramas abiertas,"
                " no existe una interpretación que haga verdaderas todas las reglas.</p>");
        } else {
            resumen += QStringLiteral(
                "<p style='background:#111b18; color:#3ee6a8; padding:12px; border:1px solid #1d5945; border-radius:8px;"
                " font-weight:700;'>La base de conocimiento es consistente.</p>"
                "<p>Una rama abierta da una interpretación posible. Aquí puedes revisar el valor de cada idea"
                " y comprobar cómo queda cada regla.</p>"
                "<h3 style='color:#ff4265;'>Interpretaciones posibles</h3>");
            for (std::size_t indice = 0; indice < resultado.ramas.size(); ++indice) {
                const auto& rama = resultado.ramas[indice];
                if (rama.estado != EstadoRama::Abierta) continue;

                std::map<std::string, bool> valores;
                for (const auto& asignacion : rama.asignacion) {
                    valores[asignacion.letra] = asignacion.valor.value_or(false);
                }

                resumen += QStringLiteral(
                    "<div style='background:#141317; border:1px solid #3a2931; border-radius:10px;"
                    " padding:14px; margin:10px 0;'>"
                    "<h3 style='color:#ff4265; margin-top:0;'>Interpretación %1</h3>"
                    "<p style='color:#aaa1a6;'>Asignación de valores: V = verdadero y F = falso."
                    " Si una idea está libre, puede tomar cualquiera de los dos valores; aquí se elige F"
                    " para mostrar un ejemplo concreto.</p>"
                    "<table width='100%' cellspacing='0' style='border-collapse:collapse;'>"
                    "<tr style='background:#1a1519; color:#ff8197;'>"
                    "<th align='left' style='padding:9px;'>Átomo</th>"
                    "<th align='left' style='padding:9px;'>Proposición</th>"
                    "<th align='left' style='padding:9px;'>Valor</th></tr>")
                    .arg(indice + 1);

                for (const auto& asignacion : rama.asignacion) {
                    const auto atomo = std::find_if(base.atomos.begin(), base.atomos.end(),
                        [&asignacion](const Atomo& candidato) {
                            return candidato.letra == asignacion.letra;
                        });
                    const QString significado = atomo == base.atomos.end()
                        ? convertir(asignacion.letra)
                        : convertir(atomo->significado);
                    const bool valor = asignacion.valor.value_or(false);
                    const QString valorTexto = valor
                        ? QStringLiteral("V · VERDADERO")
                        : QStringLiteral("F · FALSO");
                    const QString colorValor = valor
                        ? QStringLiteral("#3ee6a8")
                        : QStringLiteral("#ff718b");
                    const QString fondoFila = valor
                        ? QStringLiteral("#111b18")
                        : QStringLiteral("#211217");
                    QString notaLibre;
                    if (!asignacion.valor.has_value()) {
                        notaLibre = QStringLiteral(
                            " <span style='color:#ffc857;'>(libre; elegido para este modelo)</span>");
                    }
                    resumen += QStringLiteral(
                        "<tr style='background:%1; border-bottom:1px solid #332a30;'>"
                        "<td style='padding:9px; color:#f1eef0;'><b>%2</b></td>"
                        "<td style='padding:9px; color:#c0b5bb;'>%3</td>"
                        "<td style='padding:9px; color:%4;'><b>%5</b>%6</td></tr>")
                        .arg(fondoFila)
                        .arg(convertir(asignacion.letra).toHtmlEscaped())
                        .arg(significado.toHtmlEscaped())
                        .arg(colorValor)
                        .arg(valorTexto)
                        .arg(notaLibre);
                }

                resumen += QStringLiteral(
                    "</table><h4 style='color:#f1eef0;'>¿Se cumple cada regla?</h4>");
                for (const auto& regla : base.reglas) {
                    const auto valor = evaluarFormula(regla.formula, valores);
                    QString estado;
                    QString color;
                    QString fondo;
                    if (!valor.has_value()) {
                        estado = QStringLiteral("NO DETERMINADO");
                        color = QStringLiteral("#ffc857");
                        fondo = QStringLiteral("#241d12");
                    } else if (*valor) {
                        estado = QStringLiteral("VERDADERA");
                        color = QStringLiteral("#3ee6a8");
                        fondo = QStringLiteral("#111b18");
                    } else {
                        estado = QStringLiteral("FALSA");
                        color = QStringLiteral("#ff718b");
                        fondo = QStringLiteral("#211217");
                    }
                    resumen += QStringLiteral(
                        "<table width='100%' cellspacing='0' style='margin:5px 0; background:%1;"
                        " border:1px solid #332a30; border-radius:6px;'><tr>"
                        "<td style='padding:9px;'><b style='color:#f1eef0;'>R%2. %3</b><br>"
                        "<span style='color:#aaa1a6;'>Fórmula: %4</span></td>"
                        "<td align='right' style='padding:9px;'><b style='color:%5;'>%6</b></td>"
                        "</tr></table>")
                        .arg(fondo)
                        .arg(regla.numero)
                        .arg(convertir(regla.original).toHtmlEscaped())
                        .arg(convertir(escribirFormula(regla.formula)).toHtmlEscaped())
                        .arg(color)
                        .arg(estado);
                }
                resumen += QStringLiteral("</div>");
            }
        }
        resultados_->setHtml(resumen + QStringLiteral("</body></html>"));
        if (resultado.abiertas == 0) {
            estadoApp_->setText(QStringLiteral("%1 reglas · inconsistente").arg(base.reglas.size()));
            estadoApp_->setStyleSheet(QStringLiteral(
                "QLabel#estadoApp { background:#281018; color:#ff718b; border:1px solid #8e1c3c;"
                " padding:8px 14px; border-radius:14px; font-weight:700; }"));
        } else {
            estadoApp_->setText(QStringLiteral("%1 reglas · consistente").arg(base.reglas.size()));
            estadoApp_->setStyleSheet(QStringLiteral(
                "QLabel#estadoApp { background:#111b18; color:#3ee6a8; border:1px solid #1d5945;"
                " padding:8px 14px; border-radius:14px; font-weight:700; }"));
        }
        pestanas_->setCurrentIndex(1);
    } catch (const std::exception& error) {
        const QString mensaje = convertir(error.what());
        resultados_->setPlainText(QStringLiteral("No se pudo analizar la entrada.\n\n") + mensaje);
        estadoApp_->setText(QStringLiteral("Revisa las reglas"));
        estadoApp_->setStyleSheet(QStringLiteral(
            "QLabel#estadoApp { background:#281018; color:#ff718b; border:1px solid #8e1c3c;"
            " padding:8px 14px; border-radius:14px; font-weight:700; }"));
        pestanas_->setCurrentIndex(0);
        QMessageBox::warning(this, QStringLiteral("Error de sintaxis"), mensaje);
    }
}

}