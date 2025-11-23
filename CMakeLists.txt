cmake_minimum_required(VERSION 3.10)
project(LinearHashingRestAPI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Definir versión de Windows 10 para cpp-httplib
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

# Incluir directorios
include_directories(${CMAKE_SOURCE_DIR})

# Agregar todos los archivos fuente
add_executable(untitled6
    main.cpp
    src/models/LinearHashing.cpp
    src/controllers/RecordController.cpp
    src/routes/recordRoutes.cpp
)

# En Windows, agregar ws2_32 para sockets
if(WIN32)
    target_link_libraries(untitled6 ws2_32)
endif()
