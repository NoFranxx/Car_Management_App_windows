#include "CarManager.h"
#include <windows.h>
#include <algorithm>

CarManager::CarManager() : dataFile(L"cars_data.txt") {
    loadFromFile(dataFile);
}

CarManager::~CarManager() {
    saveToFile(dataFile);
}

void CarManager::addCar(const Car& car) {
    cars.push_back(car);
    bool success = saveToFile(dataFile);
    
    // Логируем результат
    wchar_t fullPath[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, fullPath);
    std::wstring logFile = std::wstring(fullPath) + L"\\save_log.txt";
    
    HANDLE hLog = CreateFileW(logFile.c_str(), FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hLog != INVALID_HANDLE_VALUE) {
        std::wstring logMsg = L"AddCar: ";
        logMsg += success ? L"SUCCESS" : L"FAILED";
        logMsg += L"\r\n";
        DWORD written;
        WriteFile(hLog, logMsg.c_str(), static_cast<DWORD>(logMsg.length() * sizeof(wchar_t)), &written, NULL);
        CloseHandle(hLog);
    }
}

void CarManager::updateCar(int id, const Car& updatedCar) {
    for (auto& car : cars) {
        if (car.getId() == id) {
            car = updatedCar;
            car.setId(id);
            saveToFile(dataFile);
            return;
        }
    }
}

void CarManager::deleteCar(int id) {
    cars.erase(
        std::remove_if(cars.begin(), cars.end(),
            [id](const Car& car) { return car.getId() == id; }),
        cars.end()
    );
    saveToFile(dataFile);
}

Car* CarManager::getCar(int id) {
    for (auto& car : cars) {
        if (car.getId() == id) {
            return &car;
        }
    }
    return nullptr;
}

std::vector<Car> CarManager::getSortedCars() const {
    std::vector<Car> sorted = cars;
    
    // Сортировка: критические (>=14 дней) наверху, затем предупреждения, затем обычные
    std::sort(sorted.begin(), sorted.end(), [](const Car& a, const Car& b) {
        Car::ColorStatus statusA = a.getColorStatus();
        Car::ColorStatus statusB = b.getColorStatus();
        
        if (statusA != statusB) {
            return statusA > statusB; // CRITICAL > WARNING > NORMAL
        }
        
        // Если статус одинаковый, сортируем по дням пребывания (больше дней наверху)
        return a.getDaysParked() > b.getDaysParked();
    });
    
    return sorted;
}

std::vector<Car> CarManager::search(const std::wstring& query) const {
    std::vector<Car> results;
    std::wstring lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::towlower);
    
    for (const auto& car : cars) {
        std::wstring brand = car.getBrand();
        std::wstring model = car.getModel();
        std::wstring owner = car.getOwnerName();
        std::wstring plate = car.getPlateNumber();
        
        std::transform(brand.begin(), brand.end(), brand.begin(), ::towlower);
        std::transform(model.begin(), model.end(), model.begin(), ::towlower);
        std::transform(owner.begin(), owner.end(), owner.begin(), ::towlower);
        std::transform(plate.begin(), plate.end(), plate.begin(), ::towlower);
        
        if (brand.find(lowerQuery) != std::wstring::npos ||
            model.find(lowerQuery) != std::wstring::npos ||
            owner.find(lowerQuery) != std::wstring::npos ||
            plate.find(lowerQuery) != std::wstring::npos) {
            results.push_back(car);
        }
    }
    
    return results;
}

std::vector<Car> CarManager::filterByBrand(const std::wstring& brand) const {
    std::vector<Car> results;
    std::wstring lowerBrand = brand;
    std::transform(lowerBrand.begin(), lowerBrand.end(), lowerBrand.begin(), ::towlower);
    
    for (const auto& car : cars) {
        std::wstring carBrand = car.getBrand();
        std::transform(carBrand.begin(), carBrand.end(), carBrand.begin(), ::towlower);
        if (carBrand.find(lowerBrand) != std::wstring::npos) {
            results.push_back(car);
        }
    }
    
    return results;
}

std::vector<Car> CarManager::filterByModel(const std::wstring& model) const {
    std::vector<Car> results;
    std::wstring lowerModel = model;
    std::transform(lowerModel.begin(), lowerModel.end(), lowerModel.begin(), ::towlower);
    
    for (const auto& car : cars) {
        std::wstring carModel = car.getModel();
        std::transform(carModel.begin(), carModel.end(), carModel.begin(), ::towlower);
        if (carModel.find(lowerModel) != std::wstring::npos) {
            results.push_back(car);
        }
    }
    
    return results;
}

std::vector<Car> CarManager::filterByOwner(const std::wstring& owner) const {
    std::vector<Car> results;
    std::wstring lowerOwner = owner;
    std::transform(lowerOwner.begin(), lowerOwner.end(), lowerOwner.begin(), ::towlower);
    
    for (const auto& car : cars) {
        std::wstring carOwner = car.getOwnerName();
        std::transform(carOwner.begin(), carOwner.end(), carOwner.begin(), ::towlower);
        if (carOwner.find(lowerOwner) != std::wstring::npos) {
            results.push_back(car);
        }
    }
    
    return results;
}

std::vector<Car> CarManager::filterByDateRange(time_t startDate, time_t endDate, bool isArrival) const {
    std::vector<Car> results;
    
    for (const auto& car : cars) {
        time_t checkDate = isArrival ? car.getArrivalDate() : car.getPlannedDepartureDate();
        if (checkDate >= startDate && checkDate <= endDate) {
            results.push_back(car);
        }
    }
    
    return results;
}

bool CarManager::saveToFile(const std::wstring& filename) {
    // Получаем полный путь к файлу
    wchar_t fullPath[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, fullPath);
    std::wstring fullFilename = std::wstring(fullPath) + L"\\" + filename;
    
    // Формируем данные для записи
    std::wstring data;
    data += L"\xFEFF"; // BOM для UTF-16LE
    
    for (const auto& car : cars) {
        data += car.serialize() + L"\r\n";
    }
    
    // Записываем через Windows API для надёжности
    HANDLE hFile = CreateFileW(
        fullFilename.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    DWORD bytesWritten;
    BOOL result = WriteFile(
        hFile,
        data.c_str(),
        static_cast<DWORD>(data.length() * sizeof(wchar_t)),
        &bytesWritten,
        NULL
    );
    
    CloseHandle(hFile);
    
    return result && (bytesWritten == data.length() * sizeof(wchar_t));
}

bool CarManager::loadFromFile(const std::wstring& filename) {
    // Получаем полный путь к файлу
    wchar_t fullPath[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, fullPath);
    std::wstring fullFilename = std::wstring(fullPath) + L"\\" + filename;
    
    // Открываем файл через Windows API
    HANDLE hFile = CreateFileW(
        fullFilename.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return false; // Файл не существует - нормально для первого запуска
    }
    
    // Получаем размер файла
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == 0 || fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return false;
    }
    
    // Читаем содержимое
    std::vector<wchar_t> buffer(fileSize / sizeof(wchar_t) + 1);
    DWORD bytesRead;
    BOOL result = ReadFile(hFile, buffer.data(), fileSize, &bytesRead, NULL);
    CloseHandle(hFile);
    
    if (!result) {
        return false;
    }
    
    buffer[bytesRead / sizeof(wchar_t)] = L'\0';
    std::wstring data(buffer.data());
    
    // Пропускаем BOM если есть
    size_t startPos = 0;
    if (!data.empty() && data[0] == 0xFEFF) {
        startPos = 1;
    }
    
    // Парсим строки
    cars.clear();
    std::wstring line;
    
    for (size_t i = startPos; i < data.length(); ++i) {
        if (data[i] == L'\n' || data[i] == L'\r') {
            if (!line.empty()) {
                Car car = Car::deserialize(line);
                if (car.getId() > 0) {
                    cars.push_back(car);
                }
                line.clear();
            }
            // Пропускаем \r\n
            if (i + 1 < data.length() && data[i] == L'\r' && data[i + 1] == L'\n') {
                i++;
            }
        } else {
            line += data[i];
        }
    }
    
    // Последняя строка без \n
    if (!line.empty()) {
        Car car = Car::deserialize(line);
        if (car.getId() > 0) {
            cars.push_back(car);
        }
    }
    
    return true;
}

