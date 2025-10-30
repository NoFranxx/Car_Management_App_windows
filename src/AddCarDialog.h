#pragma once

#include <windows.h>
#include <commctrl.h>
#include "Car.h"
#include "resource.h"
#include <string>

class AddCarDialog {
public:
    AddCarDialog(HWND parent, Car* existingCar = nullptr);
    ~AddCarDialog();

    bool show();
    Car getCar() const { return car; }

private:
    static LRESULT CALLBACK SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
        UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    void initializeControls(HWND hwnd);
    void selectDate(HWND hwnd, bool isArrival);
    bool validateAndSave(HWND hwnd);
    
    HWND parentHwnd;
    Car car;
    bool isEditMode;
    bool dialogResult;
};

