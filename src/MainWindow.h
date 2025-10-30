#pragma once

#include <windows.h>
#include <commctrl.h>
#include "CarManager.h"
#include "resource.h"
#include <string>

class MainWindow {
public:
    MainWindow(HINSTANCE hInstance);
    ~MainWindow();

    bool create();
    void show(int nCmdShow);
    int run();

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ListViewSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
        UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

    void createControls();
    void createMenu();
    void setupListView();
    void refreshListView();
    void addCarToListView(const Car& car, int index);
    void updateListViewColors();
    
    void onAddCar();
    void onEditCar();
    void onDeleteCar();
    void onSearch();
    void onFilterChange();
    void onRefresh();
    
    int getSelectedCarId();
    COLORREF getColorForCar(const Car& car);

    HINSTANCE hInstance;
    HWND hwnd;
    HWND hListView;
    HWND hSearchEdit;
    HWND hSearchButton;
    HWND hFilterCombo;
    HWND hRefreshButton;
    HWND hAddButton;
    HWND hEditButton;
    HWND hDeleteButton;
    HWND hZoomInButton;
    HWND hZoomOutButton;
    
    CarManager carManager;
    std::vector<Car> displayedCars;
    
    // Масштабирование
    int fontSize;
    HFONT hListFont;
    void updateFontSize();
    void updateColumnWidths();
    void adjustColumnsToWindowSize();
    void zoomIn();
    void zoomOut();
    
    // Сохранение и загрузка настроек
    void saveSettings();
    void loadSettings();
    
    // Базовые размеры колонок (для масштабирования)
    static const int baseColumnWidths[9];
};

