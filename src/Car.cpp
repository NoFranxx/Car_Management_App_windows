#include "Car.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <vector>

int Car::nextId = 1;

Car::Car() 
    : id(0), arrivalDate(0), plannedDepartureDate(0) {
}

Car::Car(const std::wstring& brand, const std::wstring& model,
         const std::wstring& ownerName, const std::wstring& plateNumber,
         time_t arrivalDate, time_t plannedDepartureDate,
         const std::wstring& comment)
    : id(nextId++), brand(brand), model(model), ownerName(ownerName),
      plateNumber(plateNumber), arrivalDate(arrivalDate),
      plannedDepartureDate(plannedDepartureDate), comment(comment) {
}

int Car::getDaysParked() const {
    time_t now = time(nullptr);
    double seconds = difftime(now, arrivalDate);
    return static_cast<int>(seconds / (60 * 60 * 24));
}

Car::ColorStatus Car::getColorStatus() const {
    int days = getDaysParked();
    if (days >= 30) return CRITICAL;
    if (days >= 15) return WARNING;
    return NORMAL;
}

std::wstring Car::formatDate(time_t date) {
    struct tm timeinfo;
    localtime_s(&timeinfo, &date);
    wchar_t buffer[80];
    wcsftime(buffer, 80, L"%d.%m.%Y %H:%M", &timeinfo);
    return std::wstring(buffer);
}

time_t Car::parseDate(const std::wstring& dateStr) {
    struct tm timeinfo = {};
    int day, month, year, hour = 12, minute = 0;
    
    // Пробуем парсить дату с временем в формате дд.мм.гггг чч:мм
    int result = swscanf_s(dateStr.c_str(), L"%d.%d.%d %d:%d", &day, &month, &year, &hour, &minute);
    
    // Если не получилось, пробуем формат без времени дд.мм.гггг
    if (result < 3) {
        result = swscanf_s(dateStr.c_str(), L"%d.%d.%d", &day, &month, &year);
        if (result != 3) {
            return -1;  // Ошибка парсинга
        }
        hour = 12;
        minute = 0;
    }
    
    // Валидация значений
    if (day < 1 || day > 31 || month < 1 || month > 12 || year < 1900 || year > 2100) {
        return -1;  // Некорректные значения
    }
    
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        return -1;  // Некорректное время
    }
    
    timeinfo.tm_mday = day;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = 0;
    timeinfo.tm_isdst = -1;  // Автоопределение летнего времени
    
    return mktime(&timeinfo);
}

std::wstring Car::serialize() const {
    std::wstringstream ss;
    ss << id << L"|" 
       << brand << L"|" 
       << model << L"|"
       << ownerName << L"|"
       << ownerPhone << L"|"
       << plateNumber << L"|"
       << arrivalDate << L"|"
       << plannedDepartureDate << L"|"
       << comment;
    return ss.str();
}

Car Car::deserialize(const std::wstring& data) {
    Car car;
    std::wstringstream ss(data);
    std::wstring token;
    std::vector<std::wstring> tokens;
    
    while (std::getline(ss, token, L'|')) {
        tokens.push_back(token);
    }
    
    if (tokens.size() >= 8) {
        car.id = _wtoi(tokens[0].c_str());
        car.brand = tokens[1];
        car.model = tokens[2];
        car.ownerName = tokens[3];
        car.ownerPhone = tokens[4];
        car.plateNumber = tokens[5];
        car.arrivalDate = _wtoi64(tokens[6].c_str());
        car.plannedDepartureDate = _wtoi64(tokens[7].c_str());
        if (tokens.size() > 8) {
            car.comment = tokens[8];
        }
    } else if (tokens.size() >= 7) {
        // Обратная совместимость со старым форматом без номера Механика
        car.id = _wtoi(tokens[0].c_str());
        car.brand = tokens[1];
        car.model = tokens[2];
        car.ownerName = tokens[3];
        car.ownerPhone = L""; // Пустой Механик для старых записей
        car.plateNumber = tokens[4];
        car.arrivalDate = _wtoi64(tokens[5].c_str());
        car.plannedDepartureDate = _wtoi64(tokens[6].c_str());
        if (tokens.size() > 7) {
            car.comment = tokens[7];
        }
    }
    
    if (car.id >= nextId) {
        nextId = car.id + 1;
    }
    
    return car;
}

