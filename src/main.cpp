#include <QApplication>
#include "VentanaRaiz.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    VentanaRaiz ventana;
    ventana.show();

    return app.exec();
}
