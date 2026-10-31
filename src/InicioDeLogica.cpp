#include "VentanaPrincipal.hpp"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication aplicacion(argc, argv);
    LogicaArbol::VentanaPrincipal ventana;
    ventana.show();
    return aplicacion.exec();
}