# Linear Hashing REST API

Sistema de almacenamiento de registros usando la estructura de datos **Linear Hashing** implementada en C++ con una API REST.

## Características

- ✅ Implementación completa de Linear Hashing
- ✅ API REST con endpoints JSON
- ✅ Operaciones CRUD (Create, Read, Update, Delete)
- ✅ Sin dependencias externas (excepto cpp-httplib header-only)
- ✅ Manejo automático de colisiones
- ✅ Redimensionamiento dinámico

## Estructura del Proyecto

```
.
├── main.cpp                          # Punto de entrada del servidor
├── httplib.h                         # Biblioteca HTTP (header-only)
├── CMakeLists.txt                    # Configuración de compilación
├── src/
│   ├── models/
│   │   ├── LinearHashing.h          # Definición de Linear Hashing
│   │   └── LinearHashing.cpp        # Implementación de Linear Hashing
│   ├── controllers/
│   │   ├── RecordController.h       # Controlador de lógica de negocio
│   │   └── RecordController.cpp
│   └── routes/
│       ├── recordRoutes.h           # Definición de rutas REST
│       └── recordRoutes.cpp
└── README.md
```

## Requisitos

- CMake 3.10 o superior
- Compilador C++ con soporte para C++17 (GCC, Clang, MSVC)
- Windows/Linux/macOS

## Compilación

### Windows (con CMake y MinGW/MSVC)

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Linux/macOS

```bash
mkdir build
cd build
cmake ..
make
```

## Ejecución

Después de compilar, ejecuta el binario:

```bash
# Windows
.\untitled6.exe

# Linux/macOS
./untitled6
```

El servidor se iniciará en `http://localhost:8080`

## API Endpoints

### 1. **POST /records** - Crear/Actualizar un registro

Inserta un nuevo registro o actualiza uno existente.

**Request:**
```bash
curl -X POST http://localhost:8080/records \
  -H "Content-Type: application/json" \
  -d "{\"key\":\"user1\",\"value\":\"John Doe\"}"
```

**Response:**
```json
{"key":"user1","value":"John Doe"}
```

### 2. **GET /records** - Listar todos los registros

Obtiene todos los registros almacenados.

**Request:**
```bash
curl http://localhost:8080/records
```

**Response:**
```json
{
  "count": 2,
  "records": [
    {"key":"user1","value":"John Doe"},
    {"key":"user2","value":"Jane Smith"}
  ]
}
```

### 3. **GET /records/{key}** - Buscar un registro por clave

Busca un registro específico por su clave.

**Request:**
```bash
curl http://localhost:8080/records/user1
```

**Response (éxito):**
```json
{"key":"user1","value":"John Doe"}
```

**Response (no encontrado):**
```json
{"error":"Record not found"}
```

### 4. **DELETE /records/{key}** - Eliminar un registro

Elimina un registro por su clave.

**Request:**
```bash
curl -X DELETE http://localhost:8080/records/user1
```

**Response (éxito):**
```json
{"message":"Record deleted successfully"}
```

**Response (no encontrado):**
```json
{"error":"Record not found"}
```

### 5. **GET /** - Información de la API

```bash
curl http://localhost:8080/
```

**Response:**
```json
{
  "message":"Linear Hashing REST API",
  "version":"1.0",
  "endpoints":[
    "POST /records",
    "GET /records",
    "GET /records/{key}",
    "DELETE /records/{key}"
  ]
}
```

## Ejemplos de Uso Completo

### Ejemplo 1: Crear múltiples registros

```bash
# PowerShell
Invoke-RestMethod -Method POST -Uri "http://localhost:8080/records" -Headers @{"Content-Type"="application/json"} -Body '{"key":"name","value":"Alice"}'
Invoke-RestMethod -Method POST -Uri "http://localhost:8080/records" -Headers @{"Content-Type"="application/json"} -Body '{"key":"age","value":"25"}'
Invoke-RestMethod -Method POST -Uri "http://localhost:8080/records" -Headers @{"Content-Type"="application/json"} -Body '{"key":"city","value":"New York"}'

# Bash/Linux
curl -X POST http://localhost:8080/records -H "Content-Type: application/json" -d '{"key":"name","value":"Alice"}'
curl -X POST http://localhost:8080/records -H "Content-Type: application/json" -d '{"key":"age","value":"25"}'
curl -X POST http://localhost:8080/records -H "Content-Type: application/json" -d '{"key":"city","value":"New York"}'
```

### Ejemplo 2: Listar todos

```bash
# PowerShell
Invoke-RestMethod -Uri "http://localhost:8080/records"

# Bash/Linux
curl http://localhost:8080/records
```

### Ejemplo 3: Buscar y eliminar

```bash
# Buscar
curl http://localhost:8080/records/name

# Eliminar
curl -X DELETE http://localhost:8080/records/name

# Verificar que fue eliminado
curl http://localhost:8080/records/name
```

## Cómo Funciona Linear Hashing

Linear Hashing es una técnica de hashing dinámico que:

1. **Crece gradualmente**: Los buckets se dividen uno a la vez, no todos al mismo tiempo
2. **No requiere reorganización completa**: Solo redistribuye los registros del bucket dividido
3. **Factor de carga controlado**: Se divide cuando el factor de carga supera un umbral (default: 0.75)
4. **Dos funciones hash**: Usa `h₀` y `h₁` donde `h₁ = 2 × h₀` para redistribuir registros

### Parámetros Configurables

En `LinearHashing.cpp` (constructor):
- `initialSize`: Número inicial de buckets (default: 4)
- `maxLoadFactor`: Factor de carga máximo antes de dividir (default: 0.75)

## Arquitectura

```
Cliente HTTP
    ↓
[RecordRoutes] ← Define endpoints REST
    ↓
[RecordController] ← Lógica de negocio + Serialización JSON
    ↓
[LinearHashing] ← Estructura de datos (Buckets + Listas enlazadas)
```
