#pragma once

#include "Car.h"
#include <vector>
#include <algorithm>
#include <string>
#include <windows.h>

// Простая текстовая БД вместо SQLite (проще для Windows)
class CarManager {
public:
    CarManager();
    ~CarManager();

    // Управление автомобилями
    void addCar(const Car& car);
    void updateCar(int id, const Car& car);
    void deleteCar(int id);
    Car* getCar(int id);
    
    // Получение списка автомобилей
    std::vector<Car>& getCars() { return cars; }
    const std::vector<Car>& getCars() const { return cars; }
    
    // Получение отсортированного списка (критические наверху)
    std::vector<Car> getSortedCars() const;
    
    // Поиск и фильтрация
    std::vector<Car> search(const std::wstring& query) const;
    std::vector<Car> filterByBrand(const std::wstring& brand) const;
    std::vector<Car> filterByModel(const std::wstring& model) const;
    std::vector<Car> filterByOwner(const std::wstring& owner) const;
    std::vector<Car> filterByDateRange(time_t startDate, time_t endDate, bool isArrival) const;
    
    // Сохранение/загрузка
    bool saveToFile(const std::wstring& filename);
    bool loadFromFile(const std::wstring& filename);

private:
    std::vector<Car> cars;
    std::wstring dataFile;
};

