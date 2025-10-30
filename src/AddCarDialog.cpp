#include "AddCarDialog.h"
#include <windowsx.h>
#include <string>

// Статическая оконная процедура для subclassing
LRESULT CALLBACK AddCarDialog::SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    
    AddCarDialog* dlg = reinterpret_cast<AddCarDialog*>(dwRefData);

    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_ARRIVAL_BUTTON:
            if (dlg) dlg->selectDate(hwnd, true);
            return 0;
        case IDC_DEPARTURE_BUTTON:
            if (dlg) dlg->selectDate(hwnd, false);
            return 0;
        case IDC_OK_BUTTON:
            if (dlg && dlg->validateAndSave(hwnd)) {
                dlg->dialogResult = true;
                RemoveWindowSubclass(hwnd, SubclassProc, uIdSubclass);
                DestroyWindow(hwnd);
                PostQuitMessage(0);
            }
            return 0;
        case IDC_CANCEL_BUTTON:
            if (dlg) dlg->dialogResult = false;
            RemoveWindowSubclass(hwnd, SubclassProc, uIdSubclass);
            DestroyWindow(hwnd);
            PostQuitMessage(0);
            return 0;
        }
        break;
    case WM_CLOSE:
        if (dlg) dlg->dialogResult = false;
        RemoveWindowSubclass(hwnd, SubclassProc, uIdSubclass);
        DestroyWindow(hwnd);
        PostQuitMessage(0);
        return 0;
    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, SubclassProc, uIdSubclass);
        break;
    }
    
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

AddCarDialog::AddCarDialog(HWND parent, Car* existingCar)
    : parentHwnd(parent), isEditMode(existingCar != nullptr), dialogResult(false) {
    if (existingCar) {
        car = *existingCar;
    }
}

AddCarDialog::~AddCarDialog() {
}

bool AddCarDialog::show() {
    // Создаём модальное диалоговое окно
    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE,
        L"#32770", // Класс диалога
        isEditMode ? L"Редактировать автомобиль" : L"Добавить автомобиль",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 500,
        parentHwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!hwnd) {
        return false;
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    
    initializeControls(hwnd);
    
    // Центрируем окно
    RECT rcParent, rcDialog;
    GetWindowRect(parentHwnd, &rcParent);
    GetWindowRect(hwnd, &rcDialog);
    int x = rcParent.left + (rcParent.right - rcParent.left - (rcDialog.right - rcDialog.left)) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - (rcDialog.bottom - rcDialog.top)) / 2;
    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    // Обработка сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_QUIT || !IsWindow(hwnd)) {
            break;
        }
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return dialogResult;
}

void AddCarDialog::initializeControls(HWND hwnd) {
    int yPos = 20;
    int labelWidth = 150;
    int editWidth = 300;
    int xLabel = 20;
    int xEdit = xLabel + labelWidth + 10;
    int rowHeight = 40;

    // Марка (обязательное)
    CreateWindowW(L"STATIC", L"* Марка машины:", WS_CHILD | WS_VISIBLE,
        xLabel, yPos, labelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hBrand = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        xEdit, yPos, editWidth, 25, hwnd, (HMENU)IDC_BRAND_EDIT, GetModuleHandle(NULL), NULL);
    yPos += rowHeight;

    // Модель (обязательное)
    CreateWindowW(L"STATIC", L"* Модель машины:", WS_CHILD | WS_VISIBLE,
        xLabel, yPos, labelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hModel = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        xEdit, yPos, editWidth, 25, hwnd, (HMENU)IDC_MODEL_EDIT, GetModuleHandle(NULL), NULL);
    yPos += rowHeight;

    // ФИО владельца (обязательное)
    CreateWindowW(L"STATIC", L"* Мастер приёмщик:", WS_CHILD | WS_VISIBLE,
        xLabel, yPos, labelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hOwner = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        xEdit, yPos, editWidth, 25, hwnd, (HMENU)IDC_OWNER_EDIT, GetModuleHandle(NULL), NULL);
    yPos += rowHeight;

    // Механик владельца (обязательное)
    CreateWindowW(L"STATIC", L"* Механик:", WS_CHILD | WS_VISIBLE,
        xLabel, yPos, labelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hOwnerPhone = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        xEdit, yPos, editWidth, 25, hwnd, (HMENU)IDC_OWNER_PHONE_EDIT, GetModuleHandle(NULL), NULL);
    yPos += rowHeight;

    // Номер автомобиля (ОБЯЗАТЕЛЬНОЕ!)
    CreateWindowW(L"STATIC", L"* Номер автомобиля:", WS_CHILD | WS_VISIBLE,
        xLabel, yPos, labelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hPlate = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        xEdit, yPos, editWidth, 25, hwnd, (HMENU)IDC_PLATE_EDIT, GetModuleHandle(NULL), NULL);
    yPos += rowHeight;

    // Дата прихода (ОБЯЗАТЕЛЬНОЕ!)
    CreateWindowW(L"STATIC", L"* Дата прибытия (дд.мм.гггг чч:мм):", WS_CHILD | WS_VISIBLE,
        xLabel, yPos, labelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hArrival = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        xEdit, yPos, editWidth - 60, 25, hwnd, (HMENU)IDC_ARRIVAL_EDIT, GetModuleHandle(NULL), NULL);
    CreateWindowW(L"BUTTON", L"...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        xEdit + editWidth - 50, yPos, 50, 25, hwnd, (HMENU)IDC_ARRIVAL_BUTTON, GetModuleHandle(NULL), NULL);
    yPos += rowHeight;

    // Планируемая дата ухода (ОБЯЗАТЕЛЬНОЕ!)
    CreateWindowW(L"STATIC", L"* Дата отправки (дд.мм.гггг чч:мм):", WS_CHILD | WS_VISIBLE,
        xLabel, yPos, labelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hDeparture = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        xEdit, yPos, editWidth - 60, 25, hwnd, (HMENU)IDC_DEPARTURE_EDIT, GetModuleHandle(NULL), NULL);
    CreateWindowW(L"BUTTON", L"...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        xEdit + editWidth - 50, yPos, 50, 25, hwnd, (HMENU)IDC_DEPARTURE_BUTTON, GetModuleHandle(NULL), NULL);
    yPos += rowHeight;

    // Комментарий (необязательное)
    CreateWindowW(L"STATIC", L"Комментарий (необяз.):", WS_CHILD | WS_VISIBLE,
        xLabel, yPos, labelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    HWND hComment = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
        xEdit, yPos, editWidth, 60, hwnd, (HMENU)IDC_COMMENT_EDIT, GetModuleHandle(NULL), NULL);
    yPos += 80;
    
    // Подсказка об обязательных полях
    CreateWindowW(L"STATIC", L"* - обязательные поля", WS_CHILD | WS_VISIBLE,
        xLabel, yPos, editWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    yPos += 25;

    // Кнопки OK и Отмена
    CreateWindowW(L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        xEdit, yPos, 100, 30, hwnd, (HMENU)IDC_OK_BUTTON, GetModuleHandle(NULL), NULL);
    CreateWindowW(L"BUTTON", L"Отмена", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        xEdit + 110, yPos, 100, 30, hwnd, (HMENU)IDC_CANCEL_BUTTON, GetModuleHandle(NULL), NULL);

    // Если режим редактирования, заполняем поля
    if (isEditMode) {
        SetWindowTextW(hBrand, car.getBrand().c_str());
        SetWindowTextW(hModel, car.getModel().c_str());
        SetWindowTextW(hOwner, car.getOwnerName().c_str());
        SetWindowTextW(hOwnerPhone, car.getOwnerPhone().c_str());
        SetWindowTextW(hPlate, car.getPlateNumber().c_str());
        SetWindowTextW(hArrival, Car::formatDate(car.getArrivalDate()).c_str());
        SetWindowTextW(hDeparture, Car::formatDate(car.getPlannedDepartureDate()).c_str());
        SetWindowTextW(hComment, car.getComment().c_str());
    } else {
        // Устанавливаем текущую дату для даты прихода
        time_t now = time(nullptr);
        SetWindowTextW(hArrival, Car::formatDate(now).c_str());
        
        // Дата ухода = текущая дата + 1 час (в днях это примерно текущий день)
        time_t departureTime = now + (1 * 60 * 60); // +1 час
        SetWindowTextW(hDeparture, Car::formatDate(departureTime).c_str());
    }

    // Установка обработчика сообщений окна с использованием subclassing
    SetWindowSubclass(hwnd, SubclassProc, 0, reinterpret_cast<DWORD_PTR>(this));
}

void AddCarDialog::selectDate(HWND hwnd, bool isArrival) {
    // Инициализируем Common Controls для календаря (на всякий случай)
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_DATE_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Получаем позицию родительского окна для центрирования
    RECT rcParent;
    GetWindowRect(hwnd, &rcParent);
    
    // Вычисляем размер календаря
    RECT rcCal = {0, 0, 0, 0};
    MonthCal_GetMinReqRect(NULL, &rcCal); // Получаем минимальный размер
    
    int width = rcCal.right - rcCal.left;
    int height = rcCal.bottom - rcCal.top;
    if (width == 0) width = 240;
    if (height == 0) height = 180;
    
    // Создаём окно с календарём
    HWND hCalendar = CreateWindowExW(
        WS_EX_TOPMOST,
        MONTHCAL_CLASS,
        L"",
        WS_POPUP | WS_BORDER | WS_VISIBLE,
        rcParent.left + 100, 
        rcParent.top + 100, 
        width + 20, 
        height + 20,
        hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!hCalendar) {
        DWORD error = GetLastError();
        wchar_t errorMsg[256];
        swprintf_s(errorMsg, 256, L"Не удалось создать календарь!\nОшибка: %lu\nИспользуйте ручной ввод даты.", error);
        MessageBoxW(hwnd, errorMsg, L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Устанавливаем текущую дату или дату из поля ввода
    HWND hEdit = GetDlgItem(hwnd, isArrival ? IDC_ARRIVAL_EDIT : IDC_DEPARTURE_EDIT);
    wchar_t dateText[100];
    GetWindowTextW(hEdit, dateText, 100);
    
    SYSTEMTIME st = { 0 };
    if (wcslen(dateText) > 0) {
        time_t currentDate = Car::parseDate(dateText);
        if (currentDate != -1) {
            struct tm timeinfo;
            localtime_s(&timeinfo, &currentDate);
            st.wYear = timeinfo.tm_year + 1900;
            st.wMonth = timeinfo.tm_mon + 1;
            st.wDay = timeinfo.tm_mday;
            MonthCal_SetCurSel(hCalendar, &st);
        }
    } else {
        // Устанавливаем текущую дату
        GetLocalTime(&st);
        MonthCal_SetCurSel(hCalendar, &st);
    }
    
    // Для даты прихода разрешаем выбор дат в прошлом
    if (isArrival) {
        SYSTEMTIME minDate = { 2000, 1, 0, 1, 0, 0, 0, 0 }; // С 1 января 2000 года
        MonthCal_SetRange(hCalendar, GDTR_MIN, &minDate);
    }
    
    // Показываем календарь
    UpdateWindow(hCalendar);
    SetFocus(hCalendar);
    BringWindowToTop(hCalendar);
    
    // Модальный цикл сообщений для календаря
    MSG msg;
    bool dateSelected = false;
    SYSTEMTIME selectedTime = { 0 };
    
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.hwnd == hCalendar || IsChild(hCalendar, msg.hwnd)) {
            // Проверяем клик мыши на календаре
            if (msg.message == WM_LBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                RECT rcCal;
                GetWindowRect(hCalendar, &rcCal);
                
                if (PtInRect(&rcCal, pt)) {
                    if (MonthCal_GetCurSel(hCalendar, &selectedTime)) {
                        dateSelected = true;
                        break;
                    }
                }
            }
            // Клавиша Enter - выбрать дату
            else if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
                if (MonthCal_GetCurSel(hCalendar, &selectedTime)) {
                    dateSelected = true;
                    break;
                }
            }
            // Клавиша Escape - отмена
            else if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
                break;
            }
        }
        // Клик вне календаря - закрыть
        else if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN) {
            break;
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    if (dateSelected) {
        // Получаем текущее время из поля (если есть) или используем 12:00
        wchar_t currentText[100];
        GetWindowTextW(hEdit, currentText, 100);
        int hour = 12, minute = 0;
        
        // Пытаемся извлечь время из текущего значения
        swscanf_s(currentText, L"%*d.%*d.%*d %d:%d", &hour, &minute);
        
        struct tm timeinfo = { 0 };
        timeinfo.tm_year = selectedTime.wYear - 1900;
        timeinfo.tm_mon = selectedTime.wMonth - 1;
        timeinfo.tm_mday = selectedTime.wDay;
        timeinfo.tm_hour = hour;
        timeinfo.tm_min = minute;
        time_t selectedDate = mktime(&timeinfo);
        
        std::wstring dateStr = Car::formatDate(selectedDate);
        SetWindowTextW(hEdit, dateStr.c_str());
    }
    
    DestroyWindow(hCalendar);
}

bool AddCarDialog::validateAndSave(HWND hwnd) {
    wchar_t buffer[256];

    // Получаем данные из полей
    GetWindowTextW(GetDlgItem(hwnd, IDC_BRAND_EDIT), buffer, 256);
    std::wstring brand(buffer);
    
    GetWindowTextW(GetDlgItem(hwnd, IDC_MODEL_EDIT), buffer, 256);
    std::wstring model(buffer);
    
    GetWindowTextW(GetDlgItem(hwnd, IDC_OWNER_EDIT), buffer, 256);
    std::wstring owner(buffer);
    
    GetWindowTextW(GetDlgItem(hwnd, IDC_OWNER_PHONE_EDIT), buffer, 256);
    std::wstring ownerPhone(buffer);
    
    GetWindowTextW(GetDlgItem(hwnd, IDC_PLATE_EDIT), buffer, 256);
    std::wstring plate(buffer);
    
    GetWindowTextW(GetDlgItem(hwnd, IDC_ARRIVAL_EDIT), buffer, 256);
    std::wstring arrivalStr(buffer);
    
    GetWindowTextW(GetDlgItem(hwnd, IDC_DEPARTURE_EDIT), buffer, 256);
    std::wstring departureStr(buffer);
    
    GetWindowTextW(GetDlgItem(hwnd, IDC_COMMENT_EDIT), buffer, 256);
    std::wstring comment(buffer);

    // Валидация обязательных полей
    if (brand.empty()) {
        MessageBoxW(hwnd, L"Пожалуйста, укажите марку автомобиля!", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }
    
    if (model.empty()) {
        MessageBoxW(hwnd, L"Пожалуйста, укажите модель автомобиля!", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }
    
    if (owner.empty()) {
        MessageBoxW(hwnd, L"Пожалуйста, укажите мастера приёмщика!", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }
    
    if (ownerPhone.empty()) {
        MessageBoxW(hwnd, L"Пожалуйста, укажите механика!", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }
    
    if (plate.empty()) {
        MessageBoxW(hwnd, L"Пожалуйста, укажите номер автомобиля!", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }
    
    if (arrivalStr.empty() || departureStr.empty()) {
        MessageBoxW(hwnd, L"Пожалуйста, укажите даты прибытия и отправки!", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }

    // Парсинг дат
    time_t arrivalDate = Car::parseDate(arrivalStr);
    time_t departureDate = Car::parseDate(departureStr);

    if (arrivalDate == -1 || departureDate == -1) {
        MessageBoxW(hwnd, L"Неверный формат даты! Используйте формат дд.мм.гггг чч:мм", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }

    // Проверка: дата ухода не должна быть раньше даты прибытия
    if (departureDate < arrivalDate) {
        MessageBoxW(hwnd, L"Дата отправки не может быть раньше даты прибытия!", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }

    // Создаём или обновляем объект Car
    if (isEditMode) {
        car.setBrand(brand);
        car.setModel(model);
        car.setOwnerName(owner);
        car.setOwnerPhone(ownerPhone);
        car.setPlateNumber(plate);
        car.setArrivalDate(arrivalDate);
        car.setPlannedDepartureDate(departureDate);
        car.setComment(comment);
    } else {
        car = Car(brand, model, owner, plate, arrivalDate, departureDate, comment);
        car.setOwnerPhone(ownerPhone);
    }

    return true;
}

