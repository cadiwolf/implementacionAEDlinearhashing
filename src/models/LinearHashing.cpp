#include "LinearHashing.h"
#include <functional>
#include <cmath>
#include <string>
#include <memory>
#include <optional>
#include <sstream>
#include <algorithm>

// ==================== VALIDACIÓN DE BUCKET ====================

bool Bucket::isValid() const {
    // Verificar que el contador coincida con el número real de nodos
    int actual_count = 0;
    auto current = head;
    while (current != nullptr) {
        actual_count++;
        current = current->next;
    }
    return actual_count == count;
}

// ==================== CONSTRUCTOR ====================

/**
 * Inicialización del Linear Hash Table
 *
 * Concepto: Comenzamos con n0 buckets (initial_size) y nivel i=1
 * El nivel i determina cuántos bits del hash usaremos inicialmente
 */
LinearHashing::LinearHashing(int initial_size, double max_load)
    : i(1), split_ptr(0), num_entries(0), initial_buckets(initial_size),
      max_load_factor(max_load), total_splits(0), total_collisions(0) {

    // Validación: El tamaño inicial debe ser positivo y razonable
    if (initial_size <= 0) {
        throw LinearHashingException("El tamaño inicial debe ser mayor que 0");
    }

    if (initial_size > 1000000) {
        throw LinearHashingException("El tamaño inicial es demasiado grande");
    }

    // Validación: El factor de carga debe estar entre 0 y 1
    if (max_load <= 0.0 || max_load > 1.0) {
        throw LinearHashingException("El factor de carga debe estar entre 0 y 1");
    }

    // Inicializar buckets vacíos
    buckets.resize(initial_size);
}

// ==================== FUNCIONES HASH ====================

/**
 * Función hash base usando std::hash
 *
 * Concepto: Convertimos la clave en un valor numérico usando una función hash
 * Esta función debe distribuir uniformemente las claves
 */
size_t LinearHashing::hash_function(const std::string& key) const {
    try {
        std::hash<std::string> hasher;
        return hasher(key);
    } catch (const std::exception& e) {
        throw LinearHashingException("Error al calcular hash: " + std::string(e.what()));
    }
}

/**
 * Cálculo del índice del bucket usando la técnica de Linear Hashing
 *
 * Concepto clave del paper:
 * 1. Usamos una máscara de i bits: mask = (1 << i) - 1
 * 2. Calculamos m = h(key) & mask
 * 3. Si m < split_ptr, significa que ese bucket ya fue dividido,
 *    entonces usamos i+1 bits para obtener más precisión
 *
 * Esto permite que el hash table crezca gradualmente sin rehashing completo
 */
size_t LinearHashing::get_bucket_index(const std::string& key) const {
    size_t h = hash_function(key);

    // Usar máscara de i bits
    size_t mask = (1u << i) - 1;
    size_t m = h & mask;

    // Si el índice apunta a un bucket ya dividido, usar i+1 bits
    if (m < static_cast<size_t>(split_ptr)) {
        mask = (1u << (i + 1)) - 1;
        m = h & mask;
    }

    // Validación de seguridad: el índice debe estar dentro del rango
    if (m >= buckets.size()) {
        throw LinearHashingException("Índice de bucket fuera de rango: " +
                                    std::to_string(m) + " >= " + std::to_string(buckets.size()));
    }

    return m;
}

// ==================== VALIDACIONES ====================

/**
 * Valida que una clave sea válida
 */
void LinearHashing::validate_key(const std::string& key) const {
    if (key.empty()) {
        throw InvalidKeyException("(clave vacía)");
    }

    // Opcional: agregar más validaciones según necesidades
    if (key.length() > 1000) {
        throw InvalidKeyException(key.substr(0, 50) + "... (clave demasiado larga)");
    }
}

/**
 * Verifica la integridad de toda la estructura
 */
bool LinearHashing::check_integrity() const {
    // Verificar que todos los buckets sean válidos
    for (const auto& bucket : buckets) {
        if (!bucket.isValid()) {
            return false;
        }
    }

    // Verificar que split_ptr esté en rango válido
    if (split_ptr < 0 || split_ptr >= static_cast<int>(buckets.size())) {
        return false;
    }

    // Verificar que el nivel sea razonable
    if (i < 1 || i > 30) {
        return false;
    }

    return true;
}

// ==================== OPERACIÓN DE SPLIT ====================

/**
 * Determina si es necesario realizar un split
 *
 * Concepto: El factor de carga controla cuándo dividir
 * load_factor = total_entries / total_buckets
 */
bool LinearHashing::needs_split() const {
    if (buckets.empty()) {
        return false;
    }

    double current_load = static_cast<double>(num_entries) / static_cast<double>(buckets.size());
    return current_load > max_load_factor;
}

/**
 * Operación de Split - El corazón del Linear Hashing
 *
 * Concepto del paper:
 * 1. Agregamos un nuevo bucket al final del array
 * 2. Redistribuimos los registros del bucket apuntado por split_ptr
 *    usando la función hash con i+1 bits
 * 3. Los registros se dividen entre el bucket original y el nuevo
 * 4. Avanzamos split_ptr al siguiente bucket
 * 5. Si split_ptr llega al final de la ronda actual (2^i buckets),
 *    incrementamos i y reseteamos split_ptr a 0
 *
 * Ventaja: Solo rehashemos un bucket a la vez, no toda la tabla
 */
void LinearHashing::split() {
    try {
        // Validar que split_ptr esté en rango
        if (split_ptr < 0 || split_ptr >= static_cast<int>(buckets.size())) {
            throw LinearHashingException("split_ptr fuera de rango durante split");
        }

        // 1. Crear un nuevo bucket al final
        buckets.emplace_back();

        // 2. Obtener referencia al bucket a dividir
        Bucket& old_bucket = buckets[split_ptr];
        Bucket& new_bucket = buckets.back();

        // 3. Redistribuir registros usando i+1 bits
        // Usamos una lista temporal para evitar problemas de iteración
        std::vector<std::pair<std::string, std::string>> entries_to_redistribute;

        auto current = old_bucket.head;
        while (current != nullptr) {
            entries_to_redistribute.emplace_back(current->record.key, current->record.value);
            current = current->next;
        }

        // Limpiar el bucket original
        old_bucket.head = nullptr;
        old_bucket.count = 0;

        // Redistribuir cada entrada usando hash con i+1 bits
        size_t mask = (1u << (i + 1)) - 1;

        for (const auto& entry : entries_to_redistribute) {
            size_t h = hash_function(entry.first);
            size_t new_index = h & mask;

            // Determinar si va al bucket original o al nuevo
            Bucket& target_bucket = (new_index == static_cast<size_t>(split_ptr))
                                    ? old_bucket
                                    : new_bucket;

            // Insertar en el bucket correspondiente
            auto new_node = std::make_shared<Node>(entry.first, entry.second);
            new_node->next = target_bucket.head;
            target_bucket.head = new_node;
            target_bucket.count++;
        }

        // 4. Avanzar split_ptr
        split_ptr++;

        // 5. Si completamos una ronda (dividimos todos los buckets del nivel actual),
        //    incrementar el nivel y resetear split_ptr
        // En el nivel i, tenemos initial_buckets * 2^(i-1) buckets al inicio
        size_t buckets_at_level_start = initial_buckets * (1u << (i - 1));

        if (static_cast<size_t>(split_ptr) >= buckets_at_level_start) {
            i++;
            split_ptr = 0;
        }

        total_splits++;

        // Validar integridad después del split
        if (!check_integrity()) {
            throw LinearHashingException("Integridad comprometida después del split");
        }

    } catch (const std::bad_alloc& e) {
        throw LinearHashingException("Error de memoria durante split: " + std::string(e.what()));
    } catch (const LinearHashingException&) {
        throw; // Re-lanzar excepciones propias
    } catch (const std::exception& e) {
        throw LinearHashingException("Error inesperado durante split: " + std::string(e.what()));
    }
}

// ==================== OPERACIONES CRUD ====================

/**
 * Inserción de un registro
 *
 * Concepto:
 * 1. Calcular el bucket usando get_bucket_index()
 * 2. Buscar si la clave ya existe (actualizar si es así)
 * 3. Si no existe, agregar al inicio del bucket (chaining)
 * 4. Verificar si necesitamos hacer split
 */
bool LinearHashing::insert(const std::string& key, const std::string& value) {
    // Validar entrada
    validate_key(key);

    try {
        // Calcular índice del bucket
        size_t index = get_bucket_index(key);

        // Buscar si la clave ya existe
        auto current = buckets[index].head;
        while (current != nullptr) {
            if (current->record.key == key) {
                // Actualizar valor existente
                current->record.value = value;
                return false; // No es una inserción nueva
            }
            current = current->next;
        }

        // La clave no existe, insertar nuevo nodo al inicio
        auto new_node = std::make_shared<Node>(key, value);
        new_node->next = buckets[index].head;

        // Si había elementos, contar colisión
        if (buckets[index].head != nullptr) {
            total_collisions++;
        }

        buckets[index].head = new_node;
        buckets[index].count++;
        num_entries++;

        // Verificar si necesitamos hacer split
        if (needs_split()) {
            split();
        }

        return true; // Inserción exitosa

    } catch (const InvalidKeyException&) {
        throw; // Re-lanzar excepciones de validación
    } catch (const std::exception& e) {
        throw LinearHashingException("Error durante inserción: " + std::string(e.what()));
    }
}

/**
 * Búsqueda de un registro
 *
 * Concepto:
 * 1. Calcular el bucket usando get_bucket_index()
 * 2. Recorrer la lista enlazada del bucket buscando la clave
 * 3. Retornar el valor si se encuentra, nullopt si no
 */
std::optional<std::string> LinearHashing::search(const std::string& key) const {
    try {
        // Validar entrada
        validate_key(key);

        // Calcular índice del bucket
        size_t index = get_bucket_index(key);

        // Buscar en la lista enlazada del bucket
        auto current = buckets[index].head;
        while (current != nullptr) {
            if (current->record.key == key) {
                return current->record.value;
            }
            current = current->next;
        }

        // No encontrado
        return std::nullopt;

    } catch (const InvalidKeyException&) {
        throw;
    } catch (const std::exception& e) {
        throw LinearHashingException("Error durante búsqueda: " + std::string(e.what()));
    }
}

/**
 * Eliminación de un registro
 *
 * Concepto:
 * 1. Calcular el bucket usando get_bucket_index()
 * 2. Buscar el nodo en la lista enlazada
 * 3. Eliminar el nodo y ajustar los punteros
 * 4. Actualizar contadores
 */
bool LinearHashing::remove(const std::string& key) {
    try {
        // Validar entrada
        validate_key(key);

        // Calcular índice del bucket
        size_t index = get_bucket_index(key);

        auto current = buckets[index].head;
        std::shared_ptr<Node> prev = nullptr;

        // Buscar el nodo a eliminar
        while (current != nullptr) {
            if (current->record.key == key) {
                // Nodo encontrado, eliminarlo
                if (prev == nullptr) {
                    // Es el primer nodo
                    buckets[index].head = current->next;
                } else {
                    // No es el primer nodo
                    prev->next = current->next;
                }

                buckets[index].count--;
                num_entries--;
                return true; // Eliminación exitosa
            }

            prev = current;
            current = current->next;
        }

        // No encontrado
        return false;

    } catch (const InvalidKeyException&) {
        throw;
    } catch (const std::exception& e) {
        throw LinearHashingException("Error durante eliminación: " + std::string(e.what()));
    }
}

/**
 * Obtener todos los registros almacenados
 *
 * Útil para debugging y exportación de datos
 */
std::vector<Record> LinearHashing::getAllRecords() const {
    std::vector<Record> records;
    records.reserve(num_entries); // Pre-alocar espacio

    try {
        // Recorrer todos los buckets
        for (const auto& bucket : buckets) {
            auto current = bucket.head;
            while (current != nullptr) {
                records.push_back(current->record);
                current = current->next;
            }
        }

        return records;

    } catch (const std::exception& e) {
        throw LinearHashingException("Error al obtener registros: " + std::string(e.what()));
    }
}

/**
 * Obtener información completa de buckets con sus entradas
 *
 * Retorna un vector de tuplas: (índice, count, vector de registros)
 * Esto permite visualizar exactamente qué registros están en cada bucket
 */
std::vector<std::tuple<int, int, std::vector<Record>>> LinearHashing::get_buckets_with_entries() const {
    std::vector<std::tuple<int, int, std::vector<Record>>> result;
    result.reserve(buckets.size());

    try {
        for (size_t i = 0; i < buckets.size(); i++) {
            const auto& bucket = buckets[i];
            std::vector<Record> entries;
            entries.reserve(bucket.count);

            // Recorrer la lista enlazada del bucket
            auto current = bucket.head;
            while (current != nullptr) {
                entries.push_back(current->record);
                current = current->next;
            }

            // Agregar tupla (índice, count, entries)
            result.emplace_back(static_cast<int>(i), bucket.count, entries);
        }

        return result;

    } catch (const std::exception& e) {
        throw LinearHashingException("Error al obtener buckets con entradas: " + std::string(e.what()));
    }
}

// ==================== ESTADÍSTICAS ====================

/**
 * Calcula el factor de carga actual
 */
double LinearHashing::get_load_factor() const {
    if (buckets.empty()) {
        return 0.0;
    }
    return static_cast<double>(num_entries) / static_cast<double>(buckets.size());
}

/**
 * Genera un reporte detallado de estadísticas
 *
 * Útil para debugging y análisis de rendimiento
 */
std::string LinearHashing::get_statistics() const {
    std::ostringstream oss;

    oss << "=== Estadísticas de Linear Hashing ===\n";
    oss << "Número de entradas: " << num_entries << "\n";
    oss << "Número de buckets: " << buckets.size() << "\n";
    oss << "Nivel actual (i): " << i << "\n";
    oss << "Split pointer: " << split_ptr << "\n";
    oss << "Factor de carga: " << get_load_factor() << "\n";
    oss << "Factor de carga máximo: " << max_load_factor << "\n";
    oss << "Total de splits realizados: " << total_splits << "\n";
    oss << "Total de colisiones: " << total_collisions << "\n";

    // Distribución de entradas por bucket
    int max_bucket_size = 0;
    int min_bucket_size = num_entries;
    int empty_buckets = 0;

    for (const auto& bucket : buckets) {
        max_bucket_size = std::max(max_bucket_size, bucket.count);
        min_bucket_size = std::min(min_bucket_size, bucket.count);
        if (bucket.count == 0) {
            empty_buckets++;
        }
    }

    oss << "Tamaño máximo de bucket: " << max_bucket_size << "\n";
    oss << "Tamaño mínimo de bucket: " << min_bucket_size << "\n";
    oss << "Buckets vacíos: " << empty_buckets << "\n";
    oss << "======================================\n";

    return oss.str();
}
