#pragma once

#include "FormulaLogica.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace LogicaArbol {

enum class EstadoRama {
    EnCurso,
    Cerrada,
    Abierta
};

struct NodoTableau {
    int id = 0;
    FormulaPtr formula;
    std::string etiqueta;
    std::string reglaDeExpansion;
    bool aplicada = false;
    EstadoRama estado = EstadoRama::EnCurso;
    std::string contradiccion;
    std::weak_ptr<NodoTableau> padre;
    std::vector<std::shared_ptr<NodoTableau>> hijos;
};

struct ValorAtomo {
    std::string letra;
    std::optional<bool> valor;
};

struct RamaFinal {
    std::shared_ptr<NodoTableau> extremo;
    EstadoRama estado = EstadoRama::EnCurso;
    std::string contradiccion;
    std::vector<ValorAtomo> asignacion;
};

struct ResultadoTableau {
    std::shared_ptr<NodoTableau> raiz;
    std::vector<RamaFinal> ramas;
    std::size_t cerradas = 0;
    std::size_t abiertas = 0;
};

class MotorDeTableau {
public:
    ResultadoTableau construir(const BaseLeida& base) const;
};

}