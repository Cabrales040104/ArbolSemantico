SHELL := cmd.exe
.SHELLFLAGS := /C

QMAKE = qmake6
MAKE = mingw32-make

.PHONY: all build start clean rebuild

all: build start

Makefile.qt: LogicaArbol.pro
	$(QMAKE) -o Makefile.qt LogicaArbol.pro

build: Makefile.qt
	$(MAKE) -f Makefile.qt

start: build
	start "" "bin\LogicaArbol.exe"

clean:
	-if exist Makefile.qt $(MAKE) -f Makefile.qt clean
	-if exist bin rmdir /s /q bin
	-if exist Makefile.qt del /q Makefile.qt
	-if exist .qmake.stash del /q .qmake.stash

rebuild: clean build