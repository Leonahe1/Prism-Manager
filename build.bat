@echo off
REM ========================================
REM Prism Build Script
REM Creates separate Debug and Release build directories
REM ========================================

echo ========================================
echo Prism Build Script
echo ========================================
echo.

REM Check if CMake exists
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake not found! Please install CMake and add it to PATH.
    pause
    exit /b 1
)

REM Ask user which build type
echo Select build type:
echo   1. Debug
echo   2. Release
echo   3. Both
echo.
set /p BUILD_CHOICE="Enter choice (1/2/3): "

if "%BUILD_CHOICE%"=="1" goto BUILD_DEBUG
if "%BUILD_CHOICE%"=="2" goto BUILD_RELEASE
if "%BUILD_CHOICE%"=="3" goto BUILD_BOTH
echo [ERROR] Invalid choice!
pause
exit /b 1

:BUILD_BOTH
echo.
echo ========================================
echo Building Debug and Release
echo ========================================
call :BUILD_DEBUG_INTERNAL
call :BUILD_RELEASE_INTERNAL
goto END

:BUILD_DEBUG
echo.
echo ========================================
echo Building Debug
echo ========================================
call :BUILD_DEBUG_INTERNAL
goto END

:BUILD_RELEASE
echo.
echo ========================================
echo Building Release
echo ========================================
call :BUILD_RELEASE_INTERNAL
goto END

:BUILD_DEBUG_INTERNAL
echo.
echo [1/3] Creating build-debug directory...
if not exist build-debug mkdir build-debug
cd build-debug

echo [2/3] Generating Debug build files...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug -DQt5_DIR="D:\Qt\Qt5.14.2\5.14.2\msvc2017_64\lib\cmake\Qt5"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

echo [3/3] Compiling Debug...
cmake --build . --config Debug
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Debug build completed!
echo Output: build-debug\bin\Prism.exe
cd ..
exit /b 0

:BUILD_RELEASE_INTERNAL
echo.
echo [1/3] Creating build-release directory...
if not exist build-release mkdir build-release
cd build-release

echo [2/3] Generating Release build files...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DQt5_DIR="D:\Qt\Qt5.14.2\5.14.2\msvc2017_64\lib\cmake\Qt5"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

echo [3/3] Compiling Release...
cmake --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Release build completed!
echo Output: build-release\bin\Prism.exe
cd ..
exit /b 0

:END
echo.
echo ========================================
echo Build completed!
echo ========================================
echo.
echo Output locations:
echo   - Debug: build-debug\bin\Prism.exe
echo   - Release: build-release\bin\Prism.exe
echo.
pause
