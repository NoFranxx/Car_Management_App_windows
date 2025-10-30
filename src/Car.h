#pragma once

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>

class Car {
public:
    Car();
    Car(const std::wstring& brand, const std::wstring& model, 
        const std::wstring& ownerName, const std::wstring& plateNumber,
        time_t arrivalDate, time_t plannedDepartureDate,
        const std::wstring& comment = L"");

    // Getters
    int getId() const { return id; }
    std::wstring getBrand() const { return brand; }
    std::wstring getModel() const { return model; }
    std::wstring getOwnerName() const { return ownerName; }
    std::wstring getOwnerPhone() const { return ownerPhone; }
    std::wstring getPlateNumber() const { return plateNumber; }
    time_t getArrivalDate() const { return arrivalDate; }
    time_t getPlannedDepartureDate() const { return plannedDepartureDate; }
    std::wstring getComment() const { return comment; }

    // Setters
    void setId(int id) { this->id = id; }
    void setBrand(const std::wstring& brand) { this->brand = brand; }
    void setModel(const std::wstring& model) { this->model = model; }
    void setOwnerName(const std::wstring& name) { this->ownerName = name; }
    void setOwnerPhone(const std::wstring& phone) { this->ownerPhone = phone; }
    void setPlateNumber(const std::wstring& number) { this->plateNumber = number; }
    void setArrivalDate(time_t date) { this->arrivalDate = date; }
    void setPlannedDepartureDate(time_t date) { this->plannedDepartureDate = date; }
    void setComment(const std::wstring& comment) { this->comment = comment; }

    // Вычисление дней пребывания
    int getDaysParked() const;
    
    // Статус цвета на основе дней
    enum ColorStatus {
        NORMAL,     // < 15 дней
        WARNING,    // 15-29 дней (жёлтый)
        CRITICAL    // >= 30 дней (красный)
    };
    
    ColorStatus getColorStatus() const;
    
    // Форматирование даты
    static std::wstring formatDate(time_t date);
    static time_t parseDate(const std::wstring& dateStr);
    
    // Сериализация
    std::wstring serialize() const;
    static Car deserialize(const std::wstring& data);

private:
    int id;
    std::wstring brand;
    std::wstring model;
    std::wstring ownerName;
    std::wstring ownerPhone;
    std::wstring plateNumber;
    time_t arrivalDate;
    time_t plannedDepartureDate;
    std::wstring comment;
    
    static int nextId;
};

