#pragma once

#include "AnalizadorDeReglas.hpp"
#include "MotorDeTableau.hpp"

#include <QMainWindow>

class QPlainTextEdit;
class QListWidget;
class QLabel;
class QTableWidget;
class QTabWidget;
class QTextEdit;
class QTreeWidgetItem;
class QTreeWidget;

namespace LogicaArbol {

class VistaGrafica;

class VentanaPrincipal : public QMainWindow {
public:
    explicit VentanaPrincipal(QWidget* padre = nullptr);

private:
    QPlainTextEdit* entrada_;
    QLabel* estadoApp_;
    QTabWidget* pestanas_;
    QTableWidget* tablaAtomos_;
    QListWidget* listaFormulas_;
    VistaGrafica* vistaGrafica_;
    QTreeWidget* arbolDetalle_;
    QTextEdit* resultados_;
    AnalizadorDeReglas analizador_;
    MotorDeTableau motor_;

    void analizarEntrada();
    void mostrarDetalle(const std::shared_ptr<NodoTableau>& nodo, QTreeWidget* arbol,
        int& siguientePaso, QTreeWidgetItem* padre = nullptr);
};

}