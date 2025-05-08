#!/bin/bash

# === win cfg ===
WIN_PROJECT_PATH="C:\\Users\\smlkr\\Desktop\\psx-animation"
WSL_PROJECT_PATH="/mnt/c/Users/smlkr/Desktop/psx-animation"
WIN_MAKE_BAT="${WIN_PROJECT_PATH}\\tools\\make32.bat"
WIN_PCSX="C:\\Users\\smlkr\\Desktop\\psx-animation\\pcsx-redux\\pcsx-redux.exe"

# === Linux Wine cfg ===
WINE_PROJECT_PATH="$HOME/psx-animation"
WINE_PCSX="pcsx-redux"

# === arg ===
MODE="$1"

# === clean func ===
clean() {
    echo "[CLEAN] Cleaning build files..."
    rm -f "$WSL_PROJECT_PATH/root/MAIN.EXE"
    rm -f "$WSL_PROJECT_PATH/output.cue"
    rm -f "$WSL_PROJECT_PATH/output.bin"
    rm -f "$WSL_PROJECT_PATH/main.exe"
    echo "[CLEAN] Done."
}

# === if 'clean' - ===
if [ "$MODE" == "clean" ]; then
    clean
    exit 0
fi

# === Windows mode ===
if [ "$MODE" == "win" ]; then
    echo "[BUILD] Running Windows build script..."
    /mnt/c/Windows/System32/cmd.exe /c "cd ${WIN_PROJECT_PATH}\\tools && make32.bat"

    if [ ! -f "$WSL_PROJECT_PATH/main.exe" ]; then
        echo "❌ [ERROR] main.exe not found. Compilation failed."
        exit 1
    fi

    echo "[BUILD] Moving main.exe to root/MAIN.EXE"
    mv "$WSL_PROJECT_PATH/main.exe" "$WSL_PROJECT_PATH/root/MAIN.EXE"

    echo "[ISO] Generating ISO image..."
    cd "$WSL_PROJECT_PATH" || exit 1
    mkpsxiso -y build.xml

    if [ ! -f "$WSL_PROJECT_PATH/output.cue" ]; then
        echo "❌ [ERROR] ISO build failed (output.cue not found)."
        exit 1
    fi

    echo "[EMU] Launching pcsx-redux..."
    /mnt/c/Windows/System32/cmd.exe /c "${WIN_PCSX} --loadiso ${WIN_PROJECT_PATH}\\root\\MAIN.EXE --run"

# === Linux Wine (arch) ===
elif [ "$MODE" == "linux" ]; then
    export WINEPREFIX=$HOME/.psyq
    export WINEARCH=win32

    echo "[BUILD] Running Wine build script..."
    wineconsole tools/make32.bat

    if [ ! -f "main.exe" ]; then
        echo "❌ [ERROR] main.exe not found. Compilation failed."
        exit 1
    fi

    echo "[BUILD] Moving main.exe to root/MAIN.EXE"
    mv main.exe root/MAIN.EXE

    echo "[ISO] Generating ISO image..."
    mkpsxiso -y build.xml

    if [ ! -f "output.cue" ]; then
        echo "❌ [ERROR] ISO build failed (output.cue not found)."
        exit 1
    fi

    echo "[EMU] Launching pcsx-redux..."
    $WINE_PCSX --loadiso "$(pwd)/output.cue" --run

else
    echo "❗ Usage: ./make.sh [win|linux|clean]"
    exit 1
fi