cmake_minimum_required(VERSION 3.15)
project(CarManagementApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для Windows приложения
if(WIN32)
    add_executable(${PROJECT_NAME} WIN32
        src/main.cpp
        src/Car.cpp
        src/Car.h
        src/CarManager.cpp
        src/CarManager.h
        src/MainWindow.cpp
        src/MainWindow.h
        src/AddCarDialog.cpp
        src/AddCarDialog.h
        src/resource.h
    )
    
    target_link_libraries(${PROJECT_NAME} 
        comctl32
        comdlg32
    )
    
    # Включаем Unicode
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
    
    # Настройка кодировки для MSVC
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
    endif()
endif()

