#pragma once

#include <memory>
#include <string>
#include <vector>

namespace LogicaArbol {

enum class Conector {
    Atomo,
    Negacion,
    Conjuncion,
    Disyuncion,
    Implicacion,
    Bicondicional
};

struct Formula {
    Conector conector = Conector::Atomo;
    std::string letra;
    std::shared_ptr<Formula> izquierda;
    std::shared_ptr<Formula> derecha;
};

using FormulaPtr = std::shared_ptr<Formula>;

struct Atomo {
    std::string letra;
    std::string significado;
    std::string clave;
};

struct ReglaLeida {
    std::size_t numero = 0;
    std::string original;
    FormulaPtr formula;
};

struct BaseLeida {
    std::vector<Atomo> atomos;
    std::vector<ReglaLeida> reglas;
};

FormulaPtr crearAtomo(const std::string& letra);
FormulaPtr crearNegacion(const FormulaPtr& formula);
FormulaPtr crearBinaria(Conector conector, const FormulaPtr& izquierda, const FormulaPtr& derecha);
int precedencia(Conector conector);
std::string simbolo(Conector conector);
std::string escribirFormula(const FormulaPtr& formula);
bool esLiteral(const FormulaPtr& formula);

}