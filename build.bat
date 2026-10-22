@echo off
setlocal enabledelayedexpansion

echo ====================================================
echo  Building 3D Engine Core and Tests (GCC)
echo ====================================================

if not exist build mkdir build

set CC=gcc
set CFLAGS=-Wall -Wextra -O2 -Iinclude
set LIBS=-lm

echo [1/2] Compiling test suite: build\run_tests.exe...
%CC% %CFLAGS% src\dynamic_list.c src\token.c src\parser.c src\evaluator.c tests\test_parser.c -o build\run_tests.exe %LIBS%
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Test suite compilation failed!
    exit /b %ERRORLEVEL%
)

echo [2/2] Compiling CLI tool: build\engine_parser.exe...
%CC% %CFLAGS% src\dynamic_list.c src\token.c src\parser.c src\evaluator.c src\main.c -o build\engine_parser.exe %LIBS%
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CLI tool compilation failed!
    exit /b %ERRORLEVEL%
)

echo.
echo ====================================================
echo  Running Test Suite
echo ====================================================
build\run_tests.exe
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Unit tests failed!
    exit /b %ERRORLEVEL%
)

echo.
echo Build and tests completed successfully!
