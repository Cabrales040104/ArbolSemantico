QT += widgets
CONFIG += c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = LogicaArbol

DESTDIR = bin
OBJECTS_DIR = bin/obj
MOC_DIR = bin/moc
INCLUDEPATH += include

SOURCES += \
    src/InicioDeLogica.cpp \
    src/FormulaLogica.cpp \
    src/AnalizadorDeReglas.cpp \
    src/MotorDeTableau.cpp \
    src/VistaGrafica.cpp \
    src/VentanaPrincipal.cpp

HEADERS += \
    include/FormulaLogica.hpp \
    include/AnalizadorDeReglas.hpp \
    include/MotorDeTableau.hpp \
    include/VistaGrafica.hpp \
    include/VentanaPrincipal.hpp \
    include/CartografoReglas.hpp \
    include/ForjadorFormula.hpp \
    include/UtileriaTexto.hpp \
    include/BriznaAtomica.hpp \
    include/CedulaFormula.hpp \
    include/InventarioAtomos.hpp \
    include/NexoSintactico.hpp \
    include/RamaSemantica.hpp \
    include/TroncoSemantico.hpp