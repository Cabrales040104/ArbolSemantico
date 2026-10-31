# LogicaArbol

LogicaArbol es una aplicación de escritorio en C++17 y Qt 6 Widgets que convierte reglas proposicionales escritas en español a fórmulas y construye un tableau semántico para comprobar la consistencia de la base de conocimiento.

## Requisitos

- Windows 10 o posterior.
- MSYS2 con el entorno MINGW64.
- GCC/G++, Qt 6 Widgets, `qmake6` y `mingw32-make` del entorno MINGW64 disponibles en `PATH`.
- Una terminal `cmd.exe` abierta en la raíz del proyecto.

Los paquetes de MSYS2 necesarios son `mingw-w64-x86_64-gcc`, `mingw-w64-x86_64-qt6-base` y `mingw-w64-x86_64-make`.

## Compilar y ejecutar

Desde `cmd.exe`, en la carpeta del proyecto:

```bat
set "PATH=C:\msys64\mingw64\bin;%PATH%"
mingw32-make
```

El objetivo predeterminado genera `Makefile.qt` mediante qmake6, compila solo los archivos modificados y abre `bin\LogicaArbol.exe`. En VS Code, `Ctrl+Shift+B` ejecuta la misma tarea de compilación y ejecución. También están disponibles:

```bat
mingw32-make build
mingw32-make start
mingw32-make clean
mingw32-make rebuild
```

El ejecutable se crea en `bin/`; los objetos y archivos MOC de Qt quedan en `bin/obj/` y `bin/moc/`. `dist/` se reserva para paquetes de distribución.

## Ejemplo

Entrada:

```text
Si es de día, entonces voy a trabajar
Si llueve, entonces no voy a trabajar
Es de día
```

Átomos y fórmulas:

| Átomo | Significado | Fórmula |
| --- | --- | --- |
| p | es de día | R1: p → q |
| q | voy a trabajar | R2: r → ¬q |
| r | llueve | R3: p |

Resultado esperado: dos ramas cerradas, una rama abierta y el modelo `p = V, q = V, r = F`. La base de conocimiento es consistente.

## Conectivos

| Expresión en español | Símbolo | Precedencia |
| --- | --- | --- |
| no | ¬ | 1, mayor |
| y | ∧ | 2 |
| o | ∨ | 3 |
| si … entonces | → | 4 |
| si y solo si | ↔ | 5, menor |

Se aceptan además paréntesis, `pero` como conjunción, `u` como disyunción y los símbolos `¬`, `∧`, `∨`, `→` y `↔`. Se ignoran diferencias entre mayúsculas/minúsculas, acentos, espacios repetidos, comas y puntuación final al comparar proposiciones.

## Limitaciones

- El lector es proposicional: no interpreta cuantificadores, tiempos verbales, sinónimos ni la estructura lingüística más allá de los conectivos identificados.
- Las palabras aisladas `y` y `o` se interpretan como conectivos; para usarlas dentro del texto de un átomo habría que reformular esa proposición.
- Las reglas condicionales se escriben como `Si A, entonces B`. La forma invertida `B si A` no se interpreta como condicional.
- `no` niega la proposición inmediata según la precedencia lógica; se recomienda usar paréntesis para expresiones compuestas ambiguas.
- Los valores «libre» de un modelo abierto indican átomos que no aparecen en esa rama y pueden elegirse en cualquiera de los dos valores.
- El motor limita el cálculo a 100000 extremos de rama para evitar agotamiento de recursos en entradas explosivas.

## Estructura

```text
LogicaArbol.pro
makefile
README.md
.gitignore
bin/                         Salida compilada, ignorada por Git
dist/                        Paquetes de distribución, ignorados por Git
include/
  FormulaLogica.hpp
  AnalizadorDeReglas.hpp
  MotorDeTableau.hpp
  VistaGrafica.hpp
  VentanaPrincipal.hpp
src/
  InicioDeLogica.cpp
  FormulaLogica.cpp
  AnalizadorDeReglas.cpp
  MotorDeTableau.cpp
  VistaGrafica.cpp
  VentanaPrincipal.cpp
```

## Comprobaciones recomendadas

| Entrada | Resultado esperado |
| --- | --- |
| `ES DE DÍA,` y en otra línea `es de dia.` | Un solo átomo `p`; ambas líneas generan `p`. |
| `No voy a trabajar.` y `Voy a trabajar` | Fórmulas `¬p` y `p`; una rama cerrada. |
| `El suelo está mojado si y solo si llueve` | Una fórmula bicondicional con dos átomos y dos ramas abiertas. |
| `Si llueve` | Error de sintaxis: falta `entonces`. |
| `No` | Error de sintaxis: falta la proposición negada. |
| `Llueve y (es de día` | Error de sintaxis: paréntesis desbalanceados. |
| Una línea vacía entre dos reglas | Error que identifica el número de la línea vacía. |
