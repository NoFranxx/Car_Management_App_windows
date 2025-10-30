#include "MainWindow.h"
#include "AddCarDialog.h"
#include <windowsx.h>
#include <commctrl.h>
#include <sstream>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Базовые размеры колонок (при fontSize = 16)
const int MainWindow::baseColumnWidths[9] = {
    120, // Марка
    120, // Модель
    180, // Мастер приёмщик
    130, // Механик
    110, // Номер авто
    150, // Дата прибытия
    150, // Дата отправки
    200, // Дней с получения автомобиля
    250  // Комментарий
};

MainWindow::MainWindow(HINSTANCE hInstance) 
    : hInstance(hInstance), hwnd(nullptr), hListView(nullptr),
      hSearchEdit(nullptr), hSearchButton(nullptr), hFilterCombo(nullptr),
      hRefreshButton(nullptr), hAddButton(nullptr), hEditButton(nullptr),
      hDeleteButton(nullptr), hZoomInButton(nullptr), hZoomOutButton(nullptr),
      fontSize(16), hListFont(nullptr) {
    // Загружаем сохранённые настройки
    loadSettings();
}

MainWindow::~MainWindow() {
    // Сохраняем настройки перед выходом
    saveSettings();
    
    if (hListFont) {
        DeleteObject(hListFont);
    }
}

bool MainWindow::create() {
    // Регистрация класса окна
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"CarManagementWindowClass";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        return false;
    }

    // Создание окна (с флагом максимизации)
    hwnd = CreateWindowExW(
        0,
        L"CarManagementWindowClass",
        L"Учёт автомобилей",
        WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1200, 650,
        NULL,
        NULL,
        hInstance,
        this
    );

    if (!hwnd) {
        return false;
    }

    // createMenu(); // Убрано - используем цветные кнопки вместо меню
    createControls();
    setupListView();
    refreshListView();
    
    // Растягиваем колонки на весь экран при запуске
    adjustColumnsToWindowSize();

    return true;
}

void MainWindow::show(int nCmdShow) {
    // Всегда показываем окно в развернутом режиме
    ShowWindow(hwnd, SW_MAXIMIZE);
    UpdateWindow(hwnd);
}

int MainWindow::run() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* window = nullptr;

    if (msg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    } else {
        window = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window) {
        switch (msg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
            case IDM_FILE_EXIT:
                PostQuitMessage(0);
                return 0;
            case IDM_CAR_ADD:
                window->onAddCar();
                return 0;
            case IDM_CAR_EDIT:
                window->onEditCar();
                return 0;
            case IDM_CAR_DELETE:
                window->onDeleteCar();
                return 0;
            case IDM_CAR_REFRESH:
                window->onRefresh();
                return 0;
            case IDC_SEARCH_BUTTON:
                window->onSearch();
                return 0;
            case IDC_REFRESH_BUTTON:
                window->onRefresh();
                return 0;
            case IDC_ADD_BUTTON:
                window->onAddCar();
                return 0;
            case IDC_EDIT_BUTTON:
                window->onEditCar();
                return 0;
            case IDC_DELETE_BUTTON:
                window->onDeleteCar();
                return 0;
            case IDC_ZOOM_IN_BUTTON:
                window->zoomIn();
                return 0;
            case IDC_ZOOM_OUT_BUTTON:
                window->zoomOut();
                return 0;
            case IDC_FILTER_COMBO:
                if (HIWORD(wParam) == CBN_SELCHANGE) {
                    window->onFilterChange();
                }
                return 0;
            }
            break;

        case WM_MOUSEWHEEL: {
            // Ctrl + колесико мыши для масштабирования
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                if (delta > 0) {
                    window->zoomIn();
                } else {
                    window->zoomOut();
                }
                return 0;
            }
            break;
        }

        case WM_NOTIFY: {
            LPNMHDR pnmh = reinterpret_cast<LPNMHDR>(lParam);
            if (pnmh->idFrom == IDC_LISTVIEW) {
                if (pnmh->code == NM_DBLCLK) {
                    window->onEditCar();
                    return 0;
                }
                else if (pnmh->code == NM_CUSTOMDRAW) {
                    LPNMLVCUSTOMDRAW lplvcd = reinterpret_cast<LPNMLVCUSTOMDRAW>(lParam);
                    
                    switch (lplvcd->nmcd.dwDrawStage) {
                    case CDDS_PREPAINT:
                        return CDRF_NOTIFYITEMDRAW;
                    
                    case CDDS_ITEMPREPAINT:
                        if (lplvcd->nmcd.dwItemSpec < window->displayedCars.size()) {
                            const Car& car = window->displayedCars[lplvcd->nmcd.dwItemSpec];
                            Car::ColorStatus status = car.getColorStatus();
                            
                            if (status == Car::CRITICAL) {
                                lplvcd->clrText = RGB(200, 0, 0);
                                lplvcd->clrTextBk = RGB(255, 220, 220);
                            } else if (status == Car::WARNING) {
                                lplvcd->clrText = RGB(150, 100, 0);
                                lplvcd->clrTextBk = RGB(255, 255, 200);
                            }
                        }
                        return CDRF_NEWFONT;
                    }
                }
            }
            break;
        }

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT dis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
            if (dis->CtlType == ODT_BUTTON) {
                // Определяем цвет кнопки
                COLORREF bgColor, textColor;
                if (dis->CtlID == IDC_ADD_BUTTON) {
                    bgColor = RGB(40, 167, 69);  // Зеленый
                    textColor = RGB(255, 255, 255);
                } else if (dis->CtlID == IDC_EDIT_BUTTON) {
                    bgColor = RGB(0, 123, 255);  // Синий
                    textColor = RGB(255, 255, 255);
                } else if (dis->CtlID == IDC_DELETE_BUTTON) {
                    bgColor = RGB(220, 53, 69);  // Красный
                    textColor = RGB(255, 255, 255);
                } else {
                    return DefWindowProc(hwnd, msg, wParam, lParam);
                }

                // Если кнопка нажата, делаем цвет темнее
                if (dis->itemState & ODS_SELECTED) {
                    int r = GetRValue(bgColor) * 0.8;
                    int g = GetGValue(bgColor) * 0.8;
                    int b = GetBValue(bgColor) * 0.8;
                    bgColor = RGB(r, g, b);
                }

                // Рисуем фон кнопки
                HBRUSH hBrush = CreateSolidBrush(bgColor);
                FillRect(dis->hDC, &dis->rcItem, hBrush);
                DeleteObject(hBrush);

                // Рисуем рамку
                HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                HPEN hOldPen = (HPEN)SelectObject(dis->hDC, hPen);
                SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));
                Rectangle(dis->hDC, dis->rcItem.left, dis->rcItem.top, dis->rcItem.right, dis->rcItem.bottom);
                SelectObject(dis->hDC, hOldPen);
                DeleteObject(hPen);

                // Рисуем текст
                wchar_t text[100];
                GetWindowTextW(dis->hwndItem, text, 100);
                SetTextColor(dis->hDC, textColor);
                SetBkMode(dis->hDC, TRANSPARENT);
                DrawTextW(dis->hDC, text, -1, &dis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                return TRUE;
            }
            break;
        }

        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            
            // Изменяем размер элементов управления
            if (window->hSearchEdit) {
                SetWindowPos(window->hSearchEdit, NULL, 10, 10, 200, 25, SWP_NOZORDER);
            }
            if (window->hSearchButton) {
                SetWindowPos(window->hSearchButton, NULL, 220, 10, 80, 25, SWP_NOZORDER);
            }
            if (window->hFilterCombo) {
                SetWindowPos(window->hFilterCombo, NULL, 310, 10, 150, 200, SWP_NOZORDER);
            }
            if (window->hRefreshButton) {
                SetWindowPos(window->hRefreshButton, NULL, 470, 10, 100, 25, SWP_NOZORDER);
            }
            if (window->hAddButton) {
                SetWindowPos(window->hAddButton, NULL, 580, 10, 100, 25, SWP_NOZORDER);
            }
            if (window->hEditButton) {
                SetWindowPos(window->hEditButton, NULL, 690, 10, 100, 25, SWP_NOZORDER);
            }
            if (window->hDeleteButton) {
                SetWindowPos(window->hDeleteButton, NULL, 800, 10, 100, 25, SWP_NOZORDER);
            }
            if (window->hZoomInButton) {
                SetWindowPos(window->hZoomInButton, NULL, 910, 10, 40, 25, SWP_NOZORDER);
            }
            if (window->hZoomOutButton) {
                SetWindowPos(window->hZoomOutButton, NULL, 960, 10, 40, 25, SWP_NOZORDER);
            }
            if (window->hListView) {
                SetWindowPos(window->hListView, NULL, 10, 45, width - 20, height - 55, SWP_NOZORDER);
                // Растягиваем колонки на новый размер окна
                window->adjustColumnsToWindowSize();
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void MainWindow::createControls() {
    // Поле поиска
    hSearchEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        10, 10, 200, 25,
        hwnd,
        (HMENU)IDC_SEARCH_EDIT,
        hInstance,
        NULL
    );

    // Кнопка поиска
    hSearchButton = CreateWindowW(
        L"BUTTON",
        L"Поиск",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        220, 10, 80, 25,
        hwnd,
        (HMENU)IDC_SEARCH_BUTTON,
        hInstance,
        NULL
    );

    // Комбобокс фильтра
    hFilterCombo = CreateWindowW(
        L"COMBOBOX",
        L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        310, 10, 150, 200,
        hwnd,
        (HMENU)IDC_FILTER_COMBO,
        hInstance,
        NULL
    );

    // Добавляем варианты фильтра
    SendMessageW(hFilterCombo, CB_ADDSTRING, 0, (LPARAM)L"Все автомобили");
    SendMessageW(hFilterCombo, CB_ADDSTRING, 0, (LPARAM)L"Критические (≥30 дней)");
    SendMessageW(hFilterCombo, CB_ADDSTRING, 0, (LPARAM)L"Предупреждение (15-29 дней)");
    SendMessageW(hFilterCombo, CB_ADDSTRING, 0, (LPARAM)L"Нормальные (<15 дней)");
    SendMessageW(hFilterCombo, CB_SETCURSEL, 0, 0);

    // Кнопка обновления
    hRefreshButton = CreateWindowW(
        L"BUTTON",
        L"Обновить",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        470, 10, 100, 25,
        hwnd,
        (HMENU)IDC_REFRESH_BUTTON,
        hInstance,
        NULL
    );

    // Цветные кнопки управления автомобилями
    // Кнопка "Добавить" - зеленая
    hAddButton = CreateWindowW(
        L"BUTTON",
        L"+ Добавить",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
        580, 10, 100, 25,
        hwnd,
        (HMENU)IDC_ADD_BUTTON,
        hInstance,
        NULL
    );

    // Кнопка "Редактировать" - синяя
    hEditButton = CreateWindowW(
        L"BUTTON",
        L"Изменить",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
        690, 10, 100, 25,
        hwnd,
        (HMENU)IDC_EDIT_BUTTON,
        hInstance,
        NULL
    );

    // Кнопка "Удалить" - красная
    hDeleteButton = CreateWindowW(
        L"BUTTON",
        L"X Удалить",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
        800, 10, 100, 25,
        hwnd,
        (HMENU)IDC_DELETE_BUTTON,
        hInstance,
        NULL
    );

    // Кнопки масштабирования
    hZoomInButton = CreateWindowW(
        L"BUTTON",
        L"+",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        910, 10, 40, 25,
        hwnd,
        (HMENU)IDC_ZOOM_IN_BUTTON,
        hInstance,
        NULL
    );

    hZoomOutButton = CreateWindowW(
        L"BUTTON",
        L"-",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        960, 10, 40, 25,
        hwnd,
        (HMENU)IDC_ZOOM_OUT_BUTTON,
        hInstance,
        NULL
    );

    // ListView
    InitCommonControls();
    hListView = CreateWindowExW(
        0,
        L"SysListView32",
        L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_BORDER,
        10, 45, 960, 500,
        hwnd,
        (HMENU)IDC_LISTVIEW,
        hInstance,
        NULL
    );

    ListView_SetExtendedListViewStyle(hListView, 
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
    
    // Устанавливаем начальный шрифт
    updateFontSize();
}

void MainWindow::updateFontSize() {
    // Удаляем старый шрифт
    if (hListFont) {
        DeleteObject(hListFont);
    }
    
    // Создаём новый шрифт
    hListFont = CreateFontW(
        fontSize,                    // Высота
        0,                          // Ширина
        0,                          // Угол наклона
        0,                          // Ориентация
        FW_NORMAL,                  // Вес
        FALSE,                      // Курсив
        FALSE,                      // Подчёркивание
        FALSE,                      // Зачёркивание
        DEFAULT_CHARSET,            // Набор символов
        OUT_DEFAULT_PRECIS,         // Точность вывода
        CLIP_DEFAULT_PRECIS,        // Точность отсечения
        DEFAULT_QUALITY,            // Качество
        DEFAULT_PITCH | FF_DONTCARE,// Шаг и семейство
        L"Segoe UI"                 // Название шрифта
    );
    
    // Применяем шрифт к ListView
    SendMessageW(hListView, WM_SETFONT, (WPARAM)hListFont, TRUE);
    
    // Обновляем ширину колонок пропорционально масштабу
    updateColumnWidths();
    
    // Обновляем отображение
    refreshListView();
}

void MainWindow::updateColumnWidths() {
    // Коэффициент масштабирования (базовый размер шрифта = 16)
    float scale = (float)fontSize / 16.0f;
    
    // Масштабируем каждую колонку
    for (int i = 0; i < 9; i++) {
        int newWidth = (int)(baseColumnWidths[i] * scale);
        ListView_SetColumnWidth(hListView, i, newWidth);
    }
    
    // Растягиваем колонки на весь экран
    adjustColumnsToWindowSize();
}

void MainWindow::adjustColumnsToWindowSize() {
    // Получаем ширину ListView
    RECT rcList;
    GetClientRect(hListView, &rcList);
    int listWidth = rcList.right - rcList.left;
    
    // Вычитаем ширину вертикального скроллбара (если есть)
    int scrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);
    listWidth -= scrollBarWidth;
    
    // Вычисляем текущую ширину всех колонок кроме последней
    int usedWidth = 0;
    for (int i = 0; i < 8; i++) {
        usedWidth += ListView_GetColumnWidth(hListView, i);
    }
    
    // Устанавливаем ширину последней колонки на оставшееся пространство
    int lastColumnWidth = listWidth - usedWidth;
    
    // Минимальная ширина для последней колонки
    float scale = (float)fontSize / 16.0f;
    int minWidth = (int)(baseColumnWidths[8] * scale);
    
    if (lastColumnWidth < minWidth) {
        lastColumnWidth = minWidth;
    }
    
    ListView_SetColumnWidth(hListView, 8, lastColumnWidth);
}

void MainWindow::zoomIn() {
    if (fontSize < 32) {
        fontSize += 2;
        updateFontSize();
    }
}

void MainWindow::zoomOut() {
    if (fontSize > 10) {
        fontSize -= 2;
        updateFontSize();
    }
}

void MainWindow::saveSettings() {
    // Получаем путь к файлу настроек
    wchar_t fullPath[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, fullPath);
    std::wstring configFile = std::wstring(fullPath) + L"\\app_settings.ini";
    
    // Сохраняем размер шрифта
    std::wstring fontSizeStr = std::to_wstring(fontSize);
    
    HANDLE hFile = CreateFileW(
        configFile.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile != INVALID_HANDLE_VALUE) {
        std::wstring data = L"\xFEFF"; // BOM для UTF-16LE
        data += L"fontSize=" + fontSizeStr + L"\r\n";
        
        DWORD bytesWritten;
        WriteFile(
            hFile,
            data.c_str(),
            static_cast<DWORD>(data.length() * sizeof(wchar_t)),
            &bytesWritten,
            NULL
        );
        
        CloseHandle(hFile);
    }
}

void MainWindow::loadSettings() {
    // Получаем путь к файлу настроек
    wchar_t fullPath[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, fullPath);
    std::wstring configFile = std::wstring(fullPath) + L"\\app_settings.ini";
    
    HANDLE hFile = CreateFileW(
        configFile.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        // Файл не существует - используем значения по умолчанию
        fontSize = 16;
        return;
    }
    
    // Получаем размер файла
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == 0 || fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        fontSize = 16;
        return;
    }
    
    // Читаем содержимое
    std::vector<wchar_t> buffer(fileSize / sizeof(wchar_t) + 1);
    DWORD bytesRead;
    BOOL result = ReadFile(hFile, buffer.data(), fileSize, &bytesRead, NULL);
    CloseHandle(hFile);
    
    if (!result) {
        fontSize = 16;
        return;
    }
    
    buffer[bytesRead / sizeof(wchar_t)] = L'\0';
    std::wstring data(buffer.data());
    
    // Пропускаем BOM если есть
    if (!data.empty() && data[0] == 0xFEFF) {
        data = data.substr(1);
    }
    
    // Парсим настройки
    size_t pos = data.find(L"fontSize=");
    if (pos != std::wstring::npos) {
        pos += 9; // Длина "fontSize="
        size_t endPos = data.find(L"\r\n", pos);
        if (endPos == std::wstring::npos) {
            endPos = data.length();
        }
        std::wstring fontSizeStr = data.substr(pos, endPos - pos);
        int loadedFontSize = _wtoi(fontSizeStr.c_str());
        
        // Валидация значения
        if (loadedFontSize >= 10 && loadedFontSize <= 32) {
            fontSize = loadedFontSize;
        } else {
            fontSize = 16;
        }
    } else {
        fontSize = 16;
    }
}

void MainWindow::createMenu() {
    HMENU hMenuBar = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    HMENU hCarMenu = CreatePopupMenu();

    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_EXIT, L"Выход");
    AppendMenuW(hCarMenu, MF_STRING, IDM_CAR_ADD, L"Добавить автомобиль");
    AppendMenuW(hCarMenu, MF_STRING, IDM_CAR_EDIT, L"Редактировать");
    AppendMenuW(hCarMenu, MF_STRING, IDM_CAR_DELETE, L"Удалить");
    AppendMenuW(hCarMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hCarMenu, MF_STRING, IDM_CAR_REFRESH, L"Обновить список");

    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"Файл");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hCarMenu, L"Автомобили");

    SetMenu(hwnd, hMenuBar);
}

void MainWindow::setupListView() {
    // Добавляем колонки
    LVCOLUMNW lvc = { 0 };
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lvc.pszText = (LPWSTR)L"Марка";
    lvc.cx = baseColumnWidths[0];
    ListView_InsertColumn(hListView, COLUMN_BRAND, &lvc);

    lvc.pszText = (LPWSTR)L"Модель";
    lvc.cx = baseColumnWidths[1];
    ListView_InsertColumn(hListView, COLUMN_MODEL, &lvc);

    lvc.pszText = (LPWSTR)L"Мастер приёмщик";
    lvc.cx = baseColumnWidths[2];
    ListView_InsertColumn(hListView, COLUMN_OWNER, &lvc);

    lvc.pszText = (LPWSTR)L"Механик";
    lvc.cx = baseColumnWidths[3];
    ListView_InsertColumn(hListView, COLUMN_OWNER_PHONE, &lvc);

    lvc.pszText = (LPWSTR)L"Номер авто";
    lvc.cx = baseColumnWidths[4];
    ListView_InsertColumn(hListView, COLUMN_PLATE, &lvc);

    lvc.pszText = (LPWSTR)L"Дата прибытия";
    lvc.cx = baseColumnWidths[5];
    ListView_InsertColumn(hListView, COLUMN_ARRIVAL, &lvc);

    lvc.pszText = (LPWSTR)L"Дата отправки";
    lvc.cx = baseColumnWidths[6];
    ListView_InsertColumn(hListView, COLUMN_DEPARTURE, &lvc);

    lvc.pszText = (LPWSTR)L"Дней с получения автомобиля";
    lvc.cx = baseColumnWidths[7];
    ListView_InsertColumn(hListView, COLUMN_DAYS, &lvc);

    lvc.pszText = (LPWSTR)L"Комментарий";
    lvc.cx = baseColumnWidths[8];
    ListView_InsertColumn(hListView, COLUMN_COMMENT, &lvc);
}

void MainWindow::refreshListView() {
    ListView_DeleteAllItems(hListView);
    
    displayedCars = carManager.getSortedCars();
    
    for (size_t i = 0; i < displayedCars.size(); ++i) {
        addCarToListView(displayedCars[i], static_cast<int>(i));
    }
}

void MainWindow::addCarToListView(const Car& car, int index) {
    // Сохраняем все строки в локальные переменные, чтобы они не удалились
    std::wstring brand = car.getBrand();
    std::wstring model = car.getModel();
    std::wstring ownerName = car.getOwnerName();
    std::wstring ownerPhone = car.getOwnerPhone();
    std::wstring plateNumber = car.getPlateNumber();
    std::wstring arrivalDate = Car::formatDate(car.getArrivalDate());
    std::wstring departureDate = Car::formatDate(car.getPlannedDepartureDate());
    std::wstringstream ss;
    ss << car.getDaysParked();
    std::wstring daysStr = ss.str();
    std::wstring comment = car.getComment();

    LVITEMW lvi = { 0 };
    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.iItem = index;
    lvi.lParam = car.getId();

    // Марка
    lvi.pszText = (LPWSTR)brand.c_str();
    lvi.iSubItem = COLUMN_BRAND;
    ListView_InsertItem(hListView, &lvi);

    // Модель
    lvi.mask = LVIF_TEXT;
    lvi.pszText = (LPWSTR)model.c_str();
    lvi.iSubItem = COLUMN_MODEL;
    ListView_SetItem(hListView, &lvi);

    // Мастер приёмщик
    lvi.pszText = (LPWSTR)ownerName.c_str();
    lvi.iSubItem = COLUMN_OWNER;
    ListView_SetItem(hListView, &lvi);

    // Механик
    lvi.pszText = (LPWSTR)ownerPhone.c_str();
    lvi.iSubItem = COLUMN_OWNER_PHONE;
    ListView_SetItem(hListView, &lvi);

    // Номер автомобиля
    lvi.pszText = (LPWSTR)plateNumber.c_str();
    lvi.iSubItem = COLUMN_PLATE;
    ListView_SetItem(hListView, &lvi);

    // Дата прихода
    lvi.pszText = (LPWSTR)arrivalDate.c_str();
    lvi.iSubItem = COLUMN_ARRIVAL;
    ListView_SetItem(hListView, &lvi);

    // Плановая дата ухода
    lvi.pszText = (LPWSTR)departureDate.c_str();
    lvi.iSubItem = COLUMN_DEPARTURE;
    ListView_SetItem(hListView, &lvi);

    // Дней пребывания
    lvi.pszText = (LPWSTR)daysStr.c_str();
    lvi.iSubItem = COLUMN_DAYS;
    ListView_SetItem(hListView, &lvi);

    // Комментарий
    lvi.pszText = (LPWSTR)comment.c_str();
    lvi.iSubItem = COLUMN_COMMENT;
    ListView_SetItem(hListView, &lvi);
}

void MainWindow::onAddCar() {
    AddCarDialog dialog(hwnd);
    if (dialog.show()) {
        carManager.addCar(dialog.getCar());
        refreshListView();
        
        // Показываем уведомление об успешном добавлении
        std::wstring msg = L"Автомобиль добавлен!\nДанные сохранены.";
        MessageBoxW(hwnd, msg.c_str(), L"Успех", MB_OK | MB_ICONINFORMATION);
    }
}

void MainWindow::onEditCar() {
    int carId = getSelectedCarId();
    if (carId == -1) {
        MessageBoxW(hwnd, L"Пожалуйста, выберите автомобиль для редактирования.", L"Информация", MB_OK | MB_ICONINFORMATION);
        return;
    }

    Car* car = carManager.getCar(carId);
    if (car) {
        AddCarDialog dialog(hwnd, car);
        if (dialog.show()) {
            carManager.updateCar(carId, dialog.getCar());
            refreshListView();
            
            // Показываем уведомление об успешном обновлении
            MessageBoxW(hwnd, L"Данные обновлены и сохранены!", L"Успех", MB_OK | MB_ICONINFORMATION);
        }
    }
}

void MainWindow::onDeleteCar() {
    int carId = getSelectedCarId();
    if (carId == -1) {
        MessageBoxW(hwnd, L"Пожалуйста, выберите автомобиль для удаления.", L"Информация", MB_OK | MB_ICONINFORMATION);
        return;
    }

    int result = MessageBoxW(hwnd, L"Вы уверены, что хотите удалить этот автомобиль?", L"Подтверждение", MB_YESNO | MB_ICONQUESTION);
    if (result == IDYES) {
        carManager.deleteCar(carId);
        refreshListView();
        
        // Показываем уведомление об успешном удалении
        MessageBoxW(hwnd, L"Автомобиль удалён!\nДанные сохранены.", L"Успех", MB_OK | MB_ICONINFORMATION);
    }
}

void MainWindow::onSearch() {
    wchar_t searchText[256];
    GetWindowTextW(hSearchEdit, searchText, 256);
    
    if (wcslen(searchText) == 0) {
        refreshListView();
        return;
    }

    std::wstring query(searchText);
    displayedCars = carManager.search(query);
    
    // Сортируем результаты поиска
    std::sort(displayedCars.begin(), displayedCars.end(), [](const Car& a, const Car& b) {
        Car::ColorStatus statusA = a.getColorStatus();
        Car::ColorStatus statusB = b.getColorStatus();
        if (statusA != statusB) {
            return statusA > statusB;
        }
        return a.getDaysParked() > b.getDaysParked();
    });
    
    ListView_DeleteAllItems(hListView);
    for (size_t i = 0; i < displayedCars.size(); ++i) {
        addCarToListView(displayedCars[i], static_cast<int>(i));
    }
}

void MainWindow::onFilterChange() {
    int selection = SendMessageW(hFilterCombo, CB_GETCURSEL, 0, 0);
    
    if (selection == 0) {
        // Все автомобили
        refreshListView();
    } else {
        std::vector<Car> allCars = carManager.getSortedCars();
        displayedCars.clear();
        
        for (const auto& car : allCars) {
            Car::ColorStatus status = car.getColorStatus();
            
            if ((selection == 1 && status == Car::CRITICAL) ||
                (selection == 2 && status == Car::WARNING) ||
                (selection == 3 && status == Car::NORMAL)) {
                displayedCars.push_back(car);
            }
        }
        
        ListView_DeleteAllItems(hListView);
        for (size_t i = 0; i < displayedCars.size(); ++i) {
            addCarToListView(displayedCars[i], static_cast<int>(i));
        }
    }
}

void MainWindow::onRefresh() {
    refreshListView();
}

int MainWindow::getSelectedCarId() {
    int selectedIndex = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (selectedIndex == -1) {
        return -1;
    }

    LVITEMW lvi = { 0 };
    lvi.mask = LVIF_PARAM;
    lvi.iItem = selectedIndex;
    ListView_GetItem(hListView, &lvi);

    return static_cast<int>(lvi.lParam);
}

COLORREF MainWindow::getColorForCar(const Car& car) {
    switch (car.getColorStatus()) {
    case Car::CRITICAL:
        return RGB(255, 200, 200);
    case Car::WARNING:
        return RGB(255, 255, 200);
    default:
        return RGB(255, 255, 255);
    }
}

