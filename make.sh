#!/bin/bash

# Задаём Wine окружение
export WINEPREFIX=$HOME/.psyq
export WINEARCH=win32

# Если аргумент 'clean', выполняем очистку
if [ "$1" == "clean" ]; then
    echo "Cleaning build files..."
    # rm -f main.exe
    rm -f root/MAIN.EXE
    rm -f output.cue
    rm -f output.bin
    # rm -f output.iso
    echo "Clean complete."
    exit 0
fi

# Иначе, выполняем сборку
wineconsole tools/make32.bat

# Перемещаем собранный exe
mv main.exe root/MAIN.EXE

# Создаём образ ISO
mkpsxiso -y build.xml

# Запускаем эмулятор
pcsx-redux --loadiso ~/example/output.cue --run
