#include "../include/AnalizadorDeReglas.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <stdexcept>
#include <utility>

namespace LogicaArbol {
namespace {

void reemplazar(std::string& texto, const std::string& buscar, const std::string& reemplazo)
{
    std::size_t posicion = 0;
    while ((posicion = texto.find(buscar, posicion)) != std::string::npos) {
        texto.replace(posicion, buscar.size(), reemplazo);
        posicion += reemplazo.size();
    }
}

std::string minusculasUtf8(std::string texto)
{
    const std::pair<const char*, const char*> pares[] = {
        {"Á", "á"}, {"É", "é"}, {"Í", "í"}, {"Ó", "ó"}, {"Ú", "ú"},
        {"Ü", "ü"}, {"Ñ", "ñ"}
    };
    for (const auto& par : pares) {
        reemplazar(texto, par.first, par.second);
    }
    for (char& character : texto) {
        const auto byte = static_cast<unsigned char>(character);
        if (byte < 128) {
            character = static_cast<char>(std::tolower(byte));
        }
    }
    return texto;
}

std::string recortar(const std::string& texto)
{
    const auto inicio = texto.find_first_not_of(" \t\r\n");
    if (inicio == std::string::npos) {
        return {};
    }
    const auto fin = texto.find_last_not_of(" \t\r\n");
    return texto.substr(inicio, fin - inicio + 1);
}

std::vector<std::string> tokenizar(const std::string& entrada)
{
    std::string texto = minusculasUtf8(entrada);
    reemplazar(texto, "¡", " ");
    reemplazar(texto, "¿", " ");
    reemplazar(texto, "↔️", " <-> ");
    reemplazar(texto, "↔", " <-> ");
    reemplazar(texto, "<->", " <-> ");
    reemplazar(texto, "→", " -> ");
    reemplazar(texto, "->", " -> ");
    reemplazar(texto, "∧", " y ");
    reemplazar(texto, "∨", " o ");
    reemplazar(texto, "¬", " no ");

    std::vector<std::string> tokens;
    std::string actual;
    auto guardar = [&]() {
        if (!actual.empty()) {
            tokens.push_back(actual);
            actual.clear();
        }
    };

    for (std::size_t indice = 0; indice < texto.size(); ++indice) {
        const char character = texto[indice];
        if (character == '(' || character == ')') {
            guardar();
            tokens.emplace_back(1, character);
        } else if (std::isspace(static_cast<unsigned char>(character))
            || character == ',' || character == '.' || character == ';' || character == ':'
            || character == '!' || character == '?' || character == '\'') {
            guardar();
        } else {
            actual.push_back(character);
        }
    }
    guardar();

    std::vector<std::string> compactados;
    for (std::size_t indice = 0; indice < tokens.size();) {
        if (indice + 3 < tokens.size() && tokens[indice] == "si" && tokens[indice + 1] == "y"
            && tokens[indice + 2] == "solo" && tokens[indice + 3] == "si") {
            compactados.emplace_back("<->");
            indice += 4;
        } else {
            compactados.push_back(tokens[indice++]);
        }
    }
    return compactados;
}

std::string juntar(const std::vector<std::string>& tokens, std::size_t inicio, std::size_t fin)
{
    std::string texto;
    for (std::size_t indice = inicio; indice < fin; ++indice) {
        if (!texto.empty() && tokens[indice] != ")" && tokens[indice - 1] != "(") {
            texto.push_back(' ');
        }
        texto += tokens[indice];
    }
    return texto;
}

std::string letraDe(std::size_t indice)
{
    static const std::string letras = "pqrstuvwxyzabcdefghijklmno";
    return indice < letras.size() ? std::string(1, letras[indice]) : "p" + std::to_string(indice - letras.size() + 1);
}

class ParserDeLinea {
public:
    ParserDeLinea(std::vector<std::string> tokens, std::vector<Atomo>& atomos,
        std::map<std::string, std::string>& letrasPorClave)
        : tokens_(std::move(tokens)), atomos_(atomos), letrasPorClave_(letrasPorClave) {}

    FormulaPtr analizar()
    {
        if (tokens_.empty()) {
            throw std::runtime_error("la línea no contiene una proposición.");
        }
        return analizarRango(0, tokens_.size());
    }

private:
    std::vector<std::string> tokens_;
    std::vector<Atomo>& atomos_;
    std::map<std::string, std::string>& letrasPorClave_;

    bool rodeadoPorParentesis(std::size_t inicio, std::size_t fin) const
    {
        if (fin - inicio < 2 || tokens_[inicio] != "(") {
            return false;
        }
        int nivel = 0;
        for (std::size_t indice = inicio; indice < fin; ++indice) {
            if (tokens_[indice] == "(") ++nivel;
            if (tokens_[indice] == ")") --nivel;
            if (nivel == 0 && indice + 1 < fin) return false;
            if (nivel < 0) throw std::runtime_error("hay un paréntesis de cierre sin apertura.");
        }
        if (nivel != 0) throw std::runtime_error("hay paréntesis desbalanceados.");
        return true;
    }

    bool esOperador(const std::string& token, Conector conector) const
    {
        switch (conector) {
        case Conector::Bicondicional: return token == "<->";
        case Conector::Implicacion: return token == "->" || token == "implica";
        case Conector::Disyuncion: return token == "o" || token == "u";
        case Conector::Conjuncion: return token == "y" || token == "pero";
        default: return false;
        }
    }

    FormulaPtr analizarRango(std::size_t inicio, std::size_t fin)
    {
        if (inicio >= fin) throw std::runtime_error("falta una proposición junto a un conectivo.");
        while (rodeadoPorParentesis(inicio, fin)) {
            ++inicio;
            --fin;
            if (inicio >= fin) throw std::runtime_error("los paréntesis no contienen una proposición.");
        }

        int nivel = 0;
        for (std::size_t indice = inicio; indice < fin; ++indice) {
            if (tokens_[indice] == "(") ++nivel;
            else if (tokens_[indice] == ")") {
                --nivel;
                if (nivel < 0) throw std::runtime_error("hay un paréntesis de cierre sin apertura.");
            }
        }
        if (nivel != 0) throw std::runtime_error("hay paréntesis desbalanceados.");

        if (tokens_[inicio] == "si") {
            int profundidad = 0;
            std::size_t entonces = fin;
            for (std::size_t indice = inicio + 1; indice < fin; ++indice) {
                if (tokens_[indice] == "(") ++profundidad;
                else if (tokens_[indice] == ")") --profundidad;
                else if (profundidad == 0 && tokens_[indice] == "entonces") {
                    entonces = indice;
                    break;
                }
            }
            if (entonces == fin) throw std::runtime_error("'si' necesita 'entonces' para completar la regla.");
            if (entonces == inicio + 1 || entonces + 1 == fin) {
                throw std::runtime_error("'si' y 'entonces' deben ir acompañados de proposiciones.");
            }
            const FormulaPtr antecedente = analizarRango(inicio + 1, entonces);
            const FormulaPtr consecuente = analizarRango(entonces + 1, fin);
            return crearBinaria(Conector::Implicacion, antecedente, consecuente);
        }

        const Conector orden[] = { Conector::Bicondicional, Conector::Implicacion,
            Conector::Disyuncion, Conector::Conjuncion };
        for (Conector conector : orden) {
            std::size_t elegido = fin;
            nivel = 0;
            for (std::size_t indice = inicio; indice < fin; ++indice) {
                if (tokens_[indice] == "(") ++nivel;
                else if (tokens_[indice] == ")") --nivel;
                else if (nivel == 0 && esOperador(tokens_[indice], conector)) {
                    elegido = indice;
                    if (conector == Conector::Implicacion || conector == Conector::Bicondicional) break;
                }
            }
            if (elegido != fin) {
                if (elegido == inicio || elegido + 1 == fin) {
                    throw std::runtime_error("falta una proposición junto a un conectivo.");
                }
                const FormulaPtr izquierda = analizarRango(inicio, elegido);
                const FormulaPtr derecha = analizarRango(elegido + 1, fin);
                return crearBinaria(conector, izquierda, derecha);
            }
        }

        if (tokens_[inicio] == "no") {
            if (inicio + 1 == fin) throw std::runtime_error("'no' necesita una proposición.");
            return crearNegacion(analizarRango(inicio + 1, fin));
        }

        for (std::size_t indice = inicio; indice < fin; ++indice) {
            if (tokens_[indice] == "(" || tokens_[indice] == ")") {
                throw std::runtime_error("los paréntesis deben encerrar una proposición completa.");
            }
            if (tokens_[indice] == "entonces") {
                throw std::runtime_error("'entonces' solo puede aparecer después de una regla iniciada con 'si'.");
            }
        }

        const std::string significado = recortar(juntar(tokens_, inicio, fin));
        const std::string clave = quitarAcentos(significado);
        const auto existente = letrasPorClave_.find(clave);
        if (existente != letrasPorClave_.end()) return crearAtomo(existente->second);

        const std::string letra = letraDe(atomos_.size());
        letrasPorClave_[clave] = letra;
        atomos_.push_back({ letra, significado, clave });
        return crearAtomo(letra);
    }
};

}

std::string normalizarTexto(const std::string& texto)
{
    auto tokens = tokenizar(texto);
    return juntar(tokens, 0, tokens.size());
}

std::string quitarAcentos(const std::string& texto)
{
    std::string resultado = minusculasUtf8(texto);
    const std::pair<const char*, const char*> pares[] = {
        {"á", "a"}, {"é", "e"}, {"í", "i"}, {"ó", "o"}, {"ú", "u"},
        {"ü", "u"}, {"ñ", "n"}
    };
    for (const auto& par : pares) reemplazar(resultado, par.first, par.second);
    return resultado;
}

BaseLeida AnalizadorDeReglas::analizar(const std::string& texto) const
{
    std::string contenido = texto;
    while (!contenido.empty() && (contenido.back() == '\n' || contenido.back() == '\r')) contenido.pop_back();
    if (recortar(contenido).empty()) throw std::runtime_error("la entrada está vacía; escriba al menos una regla.");

    BaseLeida base;
    std::map<std::string, std::string> letrasPorClave;
    std::size_t inicio = 0;
    std::size_t numero = 1;
    while (inicio <= contenido.size()) {
        const auto fin = contenido.find('\n', inicio);
        const std::string original = recortar(contenido.substr(inicio,
            fin == std::string::npos ? std::string::npos : fin - inicio));
        if (original.empty()) throw std::runtime_error("la línea " + std::to_string(numero) + " está vacía.");

        ParserDeLinea parser(tokenizar(original), base.atomos, letrasPorClave);
        FormulaPtr formula;
        try {
            formula = parser.analizar();
        } catch (const std::exception& error) {
            throw std::runtime_error("línea " + std::to_string(numero) + ": " + error.what());
        }
        base.reglas.push_back({ numero, original, formula });
        if (fin == std::string::npos) break;
        inicio = fin + 1;
        ++numero;
    }
    return base;
}

}