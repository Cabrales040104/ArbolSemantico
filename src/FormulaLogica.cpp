#include "FormulaLogica.hpp"

#include <utility>

namespace LogicaArbol {

FormulaPtr crearAtomo(const std::string& letra)
{
    auto formula = std::make_shared<Formula>();
    formula->conector = Conector::Atomo;
    formula->letra = letra;
    return formula;
}

FormulaPtr crearNegacion(const FormulaPtr& formula)
{
    auto resultado = std::make_shared<Formula>();
    resultado->conector = Conector::Negacion;
    resultado->izquierda = formula;
    return resultado;
}

FormulaPtr crearBinaria(Conector conector, const FormulaPtr& izquierda, const FormulaPtr& derecha)
{
    auto formula = std::make_shared<Formula>();
    formula->conector = conector;
    formula->izquierda = izquierda;
    formula->derecha = derecha;
    return formula;
}

int precedencia(Conector conector)
{
    switch (conector) {
    case Conector::Bicondicional: return 1;
    case Conector::Implicacion: return 2;
    case Conector::Disyuncion: return 3;
    case Conector::Conjuncion: return 4;
    case Conector::Negacion: return 5;
    case Conector::Atomo: return 6;
    }
    return 0;
}

std::string simbolo(Conector conector)
{
    switch (conector) {
    case Conector::Negacion: return "¬";
    case Conector::Conjuncion: return "∧";
    case Conector::Disyuncion: return "∨";
    case Conector::Implicacion: return "→";
    case Conector::Bicondicional: return "↔";
    case Conector::Atomo: return {};
    }
    return {};
}

namespace {

std::string escribir(const FormulaPtr& formula, int padre, bool ladoDerecho)
{
    if (!formula) {
        return {};
    }

    std::string texto;
    if (formula->conector == Conector::Atomo) {
        texto = formula->letra;
    } else if (formula->conector == Conector::Negacion) {
        const auto& hija = formula->izquierda;
        texto = simbolo(Conector::Negacion) + escribir(hija, precedencia(Conector::Negacion), false);
        if (hija && hija->conector != Conector::Atomo && hija->conector != Conector::Negacion) {
            texto = simbolo(Conector::Negacion) + "(" + escribir(hija, 0, false) + ")";
        }
    } else {
        texto = escribir(formula->izquierda, precedencia(formula->conector), false)
            + " " + simbolo(formula->conector) + " "
            + escribir(formula->derecha, precedencia(formula->conector), true);
    }

    const int propia = precedencia(formula->conector);
    const bool necesitaParentesis = padre > propia
        || (ladoDerecho && padre == propia
            && (formula->conector == Conector::Implicacion || formula->conector == Conector::Bicondicional));
    return necesitaParentesis ? "(" + texto + ")" : texto;
}

}

std::string escribirFormula(const FormulaPtr& formula)
{
    return escribir(formula, 0, false);
}

bool esLiteral(const FormulaPtr& formula)
{
    return formula && (formula->conector == Conector::Atomo
        || (formula->conector == Conector::Negacion && formula->izquierda
            && formula->izquierda->conector == Conector::Atomo));
}

}