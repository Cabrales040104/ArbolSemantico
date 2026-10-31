#include "MotorDeTableau.hpp"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <utility>

namespace LogicaArbol {
namespace {

using Alternativas = std::vector<std::vector<FormulaPtr>>;

Alternativas expandir(const FormulaPtr& formula)
{
    switch (formula->conector) {
    case Conector::Conjuncion:
        return { { formula->izquierda, formula->derecha } };
    case Conector::Disyuncion:
        return { { formula->izquierda }, { formula->derecha } };
    case Conector::Implicacion:
        return { { crearNegacion(formula->izquierda) }, { formula->derecha } };
    case Conector::Bicondicional:
        return { { formula->izquierda, formula->derecha },
            { crearNegacion(formula->izquierda), crearNegacion(formula->derecha) } };
    case Conector::Negacion: {
        const auto& hija = formula->izquierda;
        switch (hija->conector) {
        case Conector::Negacion:
            return { { hija->izquierda } };
        case Conector::Disyuncion:
            return { { crearNegacion(hija->izquierda), crearNegacion(hija->derecha) } };
        case Conector::Implicacion:
            return { { hija->izquierda, crearNegacion(hija->derecha) } };
        case Conector::Conjuncion:
            return { { crearNegacion(hija->izquierda) }, { crearNegacion(hija->derecha) } };
        case Conector::Bicondicional:
            return { { hija->izquierda, crearNegacion(hija->derecha) },
                { crearNegacion(hija->izquierda), hija->derecha } };
        case Conector::Atomo:
            break;
        }
        break;
    }
    case Conector::Atomo:
        break;
    }
    return {};
}

std::string nombreExpansion(const FormulaPtr& formula)
{
    if (formula->conector != Conector::Negacion) return simbolo(formula->conector);
    const auto& hija = formula->izquierda;
    if (hija->conector == Conector::Negacion) return "¬¬";
    return "¬" + simbolo(hija->conector);
}

std::vector<std::shared_ptr<NodoTableau>> caminoHasta(const std::shared_ptr<NodoTableau>& extremo)
{
    std::vector<std::shared_ptr<NodoTableau>> camino;
    for (auto actual = extremo; actual; actual = actual->padre.lock()) camino.push_back(actual);
    std::reverse(camino.begin(), camino.end());
    return camino;
}

bool hallarContradiccion(const std::vector<std::shared_ptr<NodoTableau>>& camino, std::string& texto)
{
    std::map<std::string, bool> valores;
    for (const auto& nodo : camino) {
        const auto& formula = nodo->formula;
        std::string letra;
        bool positivo = true;
        if (formula->conector == Conector::Atomo) {
            letra = formula->letra;
        } else if (formula->conector == Conector::Negacion && formula->izquierda
            && formula->izquierda->conector == Conector::Atomo) {
            letra = formula->izquierda->letra;
            positivo = false;
        } else {
            continue;
        }

        const auto existente = valores.find(letra);
        if (existente != valores.end() && existente->second != positivo) {
            texto = (positivo ? "¬" + letra : letra) + " y " + (positivo ? letra : "¬" + letra);
            return true;
        }
        valores[letra] = positivo;
    }
    return false;
}

}

ResultadoTableau MotorDeTableau::construir(const BaseLeida& base) const
{
    if (base.reglas.empty()) throw std::runtime_error("no hay reglas para construir el árbol.");

    ResultadoTableau resultado;
    int siguienteId = 0;
    auto crearNodo = [&](const FormulaPtr& formula, const std::shared_ptr<NodoTableau>& padre,
                         const std::string& etiqueta, const std::string& regla) {
        auto nodo = std::make_shared<NodoTableau>();
        nodo->id = ++siguienteId;
        nodo->formula = formula;
        nodo->etiqueta = etiqueta;
        nodo->reglaDeExpansion = regla;
        if (padre) {
            nodo->padre = padre;
            padre->hijos.push_back(nodo);
        }
        return nodo;
    };

    std::shared_ptr<NodoTableau> ultimo;
    for (const auto& regla : base.reglas) {
        ultimo = crearNodo(regla.formula, ultimo, "R" + std::to_string(regla.numero), {});
        if (!resultado.raiz) resultado.raiz = ultimo;
    }

    std::vector<std::shared_ptr<NodoTableau>> pendientes{ ultimo };
    std::size_t procesados = 0;
    while (!pendientes.empty()) {
        if (++procesados > 100000) throw std::runtime_error("el árbol supera el límite de expansión de 100000 ramas.");
        const auto extremo = pendientes.back();
        pendientes.pop_back();
        const auto camino = caminoHasta(extremo);

        std::string contradiccion;
        if (hallarContradiccion(camino, contradiccion)) {
            extremo->estado = EstadoRama::Cerrada;
            extremo->contradiccion = contradiccion;
            resultado.ramas.push_back({ extremo, EstadoRama::Cerrada, contradiccion, {} });
            ++resultado.cerradas;
            continue;
        }

        std::shared_ptr<NodoTableau> seleccionada;
        std::shared_ptr<NodoTableau> primeraRamificable;
        for (const auto& nodo : camino) {
            if (nodo->aplicada || esLiteral(nodo->formula)) continue;
            const auto alternativas = expandir(nodo->formula);
            if (alternativas.size() == 1) {
                seleccionada = nodo;
                break;
            }
            if (!primeraRamificable) primeraRamificable = nodo;
        }
        if (!seleccionada) seleccionada = primeraRamificable;

        if (!seleccionada) {
            extremo->estado = EstadoRama::Abierta;
            std::map<std::string, bool> valores;
            for (const auto& nodo : camino) {
                const auto& formula = nodo->formula;
                if (formula->conector == Conector::Atomo) {
                    valores[formula->letra] = true;
                } else if (formula->conector == Conector::Negacion && formula->izquierda
                    && formula->izquierda->conector == Conector::Atomo) {
                    valores[formula->izquierda->letra] = false;
                }
            }

            RamaFinal rama;
            rama.extremo = extremo;
            rama.estado = EstadoRama::Abierta;
            for (const auto& atomo : base.atomos) {
                const auto encontrado = valores.find(atomo.letra);
                rama.asignacion.push_back({ atomo.letra,
                    encontrado == valores.end() ? std::optional<bool>{} : std::optional<bool>{ encontrado->second } });
            }
            resultado.ramas.push_back(std::move(rama));
            ++resultado.abiertas;
            continue;
        }

        seleccionada->aplicada = true;
        const auto alternativas = expandir(seleccionada->formula);
        const std::string nombre = nombreExpansion(seleccionada->formula);
        for (const auto& alternativa : alternativas) {
            auto nuevoExtremo = extremo;
            for (const auto& formula : alternativa) {
                nuevoExtremo = crearNodo(formula, nuevoExtremo, {}, nombre);
            }
            pendientes.push_back(nuevoExtremo);
        }
    }

    return resultado;
}

}