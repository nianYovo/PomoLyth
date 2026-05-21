$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $projectRoot

$env:PATH = "E:\Qt\6.11.0\mingw_64\bin;E:\Qt\Tools\mingw1310_64\bin;" + $env:PATH

if (-not (Test-Path ".\build\PomoLyth.exe")) {
    cmake -S . -B build -G "MinGW Makefiles" `
        -DCMAKE_PREFIX_PATH=E:/Qt/6.11.0/mingw_64 `
        -DCMAKE_CXX_COMPILER=E:/Qt/Tools/mingw1310_64/bin/g++.exe
    cmake --build build -j 4
}

Start-Process -FilePath ".\build\PomoLyth.exe" -WorkingDirectory $projectRoot
