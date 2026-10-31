#pragma once

#include "FormulaLogica.hpp"

#include <string>

namespace LogicaArbol {

class AnalizadorDeReglas {
public:
    BaseLeida analizar(const std::string& texto) const;
};

std::string normalizarTexto(const std::string& texto);
std::string quitarAcentos(const std::string& texto);

}