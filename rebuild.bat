@echo off
chcp 65001 > nul
echo ========================================
echo Полная пересборка проекта
echo ========================================
echo.

REM Закрытие запущенного приложения
taskkill /F /IM CarManagementApp.exe 2>nul
timeout /t 1 /nobreak >nul

REM Удаление старой папки сборки
if exist build (
    echo Удаление старой сборки...
    rmdir /s /q build 2>nul
    if exist build (
        echo ВНИМАНИЕ: Не удалось полностью удалить папку build.
        echo Возможно, файлы используются другим процессом.
        echo Закройте все запущенные экземпляры приложения и попробуйте снова.
        pause
        exit /b 1
    )
    echo Готово.
    echo.
)

REM Создание новой папки для сборки
echo Создание папки сборки...
mkdir build
cd build

REM Генерация проекта CMake
echo Генерация проекта CMake...
cmake ..
if %errorlevel% neq 0 (
    echo ОШИБКА при генерации проекта!
    cd ..
    pause
    exit /b %errorlevel%
)

echo Генерация завершена.
echo.

REM Сборка проекта
echo Сборка проекта (Release)...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ОШИБКА при сборке проекта!
    cd ..
    pause
    exit /b %errorlevel%
)

cd ..

echo.
echo ========================================
echo СБОРКА ЗАВЕРШЕНА УСПЕШНО!
echo ========================================
echo Исполняемый файл находится в:
echo   build\Release\CarManagementApp.exe
echo.
echo Для запуска используйте команду:
echo   start build\Release\CarManagementApp.exe
echo ========================================
echo.
pause

