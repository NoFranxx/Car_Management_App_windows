#include <windows.h>
#include <commctrl.h>
#include "MainWindow.h"

#pragma comment(lib, "comctl32.lib")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    // Инициализация Common Controls (расширенная версия)
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_DATE_CLASSES | ICC_WIN95_CLASSES;
    
    if (!InitCommonControlsEx(&icex)) {
        MessageBoxW(NULL, L"Не удалось инициализировать Common Controls!", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Создание и отображение главного окна
    MainWindow mainWindow(hInstance);
    
    if (!mainWindow.create()) {
        MessageBoxW(NULL, L"Не удалось создать окно!", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    mainWindow.show(nCmdShow);
    
    return mainWindow.run();
}

