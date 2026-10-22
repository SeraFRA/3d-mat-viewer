@echo off
setlocal enabledelayedexpansion

echo ====================================================
echo  Building 3D Engine Core and Tests (GCC)
echo ====================================================

if not exist build mkdir build

set CC=gcc
set CFLAGS=-Wall -Wextra -O2 -Iinclude
set LIBS=-lm

set CORE_SRC=src\dynamic_list.c src\token.c src\parser.c src\evaluator.c src\mesh.c src\marching_cubes.c

echo [1/3] Compiling parser test suite: build\run_tests.exe...
%CC% %CFLAGS% %CORE_SRC% tests\test_parser.c -o build\run_tests.exe %LIBS%
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Parser test suite compilation failed!
    exit /b %ERRORLEVEL%
)

echo [2/3] Compiling Marching Cubes test suite: build\run_mc_tests.exe...
%CC% %CFLAGS% %CORE_SRC% tests\test_marching_cubes.c -o build\run_mc_tests.exe %LIBS%
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Marching Cubes test suite compilation failed!
    exit /b %ERRORLEVEL%
)

echo [3/3] Compiling CLI tool: build\engine_parser.exe...
%CC% %CFLAGS% %CORE_SRC% src\main.c -o build\engine_parser.exe %LIBS%
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CLI tool compilation failed!
    exit /b %ERRORLEVEL%
)

echo.
echo ====================================================
echo  Running Parser Test Suite
echo ====================================================
build\run_tests.exe
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Parser unit tests failed!
    exit /b %ERRORLEVEL%
)

echo.
echo ====================================================
echo  Running Marching Cubes Test Suite
echo ====================================================
build\run_mc_tests.exe
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Marching Cubes unit tests failed!
    exit /b %ERRORLEVEL%
)

echo.
echo ====================================================
echo  All builds and tests passed successfully!
echo ====================================================
