#ifndef LINEARHASHING_H
#define LINEARHASHING_H

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <stdexcept>

// ==================== EXCEPCIONES PERSONALIZADAS ====================
// Manejo de errores específicos para operaciones de Linear Hashing

class LinearHashingException : public std::runtime_error {
public:
    explicit LinearHashingException(const std::string& message)
        : std::runtime_error(message) {}
};

class InvalidKeyException : public LinearHashingException {
public:
    explicit InvalidKeyException(const std::string& key)
        : LinearHashingException("Clave inválida: '" + key + "'") {}
};

class BucketOverflowException : public LinearHashingException {
public:
    explicit BucketOverflowException(int bucketIndex)
        : LinearHashingException("Overflow en bucket: " + std::to_string(bucketIndex)) {}
};

// ==================== ESTRUCTURAS DE DATOS ====================

// Estructura para almacenar un registro (par clave-valor)
struct Record {
    std::string key;
    std::string value;

    Record(const std::string& k, const std::string& v) : key(k), value(v) {}
};

// Nodo de la lista enlazada para manejar colisiones dentro de un bucket
struct Node {
    Record record;
    std::shared_ptr<Node> next;

    Node(const std::string& k, const std::string& v)
        : record(k, v), next(nullptr) {}
};

// Bucket: Contenedor de registros con manejo de colisiones mediante chaining
struct Bucket {
    std::shared_ptr<Node> head;
    int count;  // Número de elementos en este bucket

    Bucket() : head(nullptr), count(0) {}

    // Validar integridad del bucket
    bool isValid() const;
};

// ==================== CLASE PRINCIPAL ====================

/**
 * Implementación de Linear Hashing según el paper de Litwin (1980)
 *
 * Conceptos clave:
 * - i: nivel actual (número de bits del hash a usar)
 * - n: número de buckets en uso
 * - split_ptr: puntero al siguiente bucket a dividir
 * - Crecimiento dinámico sin necesidad de rehashing completo
 */
class LinearHashing {
private:
    // ==================== ESTADO DEL HASH TABLE ====================

    std::vector<Bucket> buckets;        // Array dinámico de buckets
    int i;                              // Nivel actual: número de bits del hash en uso
    int split_ptr;                      // Puntero al próximo bucket a dividir
    int num_entries;                    // Número total de registros almacenados
    int initial_buckets;                // Tamaño inicial (n0)
    double max_load_factor;             // Factor de carga máximo antes de split

    // Estadísticas para debugging y monitoreo
    int total_splits;                   // Contador de splits realizados
    int total_collisions;               // Contador de colisiones

    // ==================== FUNCIONES HASH ====================

    /**
     * Función hash base: h(key) mod (2^level * initial_buckets)
     * Aplica el algoritmo de hashing estándar limitado al rango actual
     */
    size_t hash_function(const std::string& key) const;

    /**
     * Calcula el índice del bucket usando la técnica de Linear Hashing:
     * 1. Calcula m = h(key) & ((1 << i) - 1)  // usa i bits
     * 2. Si m < split_ptr, recalcula con i+1 bits
     *
     * Esta es la esencia del Linear Hashing: determinar dinámicamente
     * cuántos bits usar basándose en la posición del split_ptr
     */
    size_t get_bucket_index(const std::string& key) const;

    // ==================== OPERACIONES DE SPLIT ====================

    /**
     * Split: Operación fundamental del Linear Hashing
     * 1. Agrega un nuevo bucket al final
     * 2. Redistribuye los registros del bucket apuntado por split_ptr
     * 3. Incrementa split_ptr (resetea si completa una ronda)
     * 4. Incrementa i si se duplicó el rango de buckets
     */
    void split();

    /**
     * Determina si es necesario realizar un split
     * Criterio: load_factor = num_entries / n > max_load_factor
     */
    bool needs_split() const;

    // ==================== VALIDACIONES ====================

    /**
     * Valida que una clave no esté vacía y cumpla restricciones
     */
    void validate_key(const std::string& key) const;

    /**
     * Verifica la integridad de la estructura de datos
     */
    bool check_integrity() const;

public:
    // ==================== CONSTRUCTOR ====================

    /**
     * Constructor del Linear Hash Table
     * @param initial_size: Número inicial de buckets (debe ser potencia de 2)
     * @param max_load: Factor de carga máximo permitido
     * @throws LinearHashingException si initial_size no es válido
     */
    LinearHashing(int initial_size = 2, double max_load = 0.75);

    // ==================== OPERACIONES CRUD ====================

    /**
     * Inserta o actualiza un registro
     * @return true si se insertó nuevo, false si se actualizó existente
     * @throws InvalidKeyException si la clave es inválida
     */
    bool insert(const std::string& key, const std::string& value);

    /**
     * Busca un registro por clave
     * @return std::optional con el valor si existe, std::nullopt si no
     */
    std::optional<std::string> search(const std::string& key) const;

    /**
     * Elimina un registro
     * @return true si se eliminó, false si no existía
     * @throws InvalidKeyException si la clave es inválida
     */
    bool remove(const std::string& key);

    /**
     * Retorna todos los registros almacenados
     */
    std::vector<Record> getAllRecords() const;

    /**
     * Retorna información completa de buckets con sus entradas
     * Estructura: vector de (índice, count, vector de registros)
     */
    std::vector<std::tuple<int, int, std::vector<Record>>> get_buckets_with_entries() const;

    // ==================== INFORMACIÓN Y ESTADÍSTICAS ====================

    int size() const { return num_entries; }
    int bucket_count() const { return buckets.size(); }
    int get_level() const { return i; }
    int get_split_pointer() const { return split_ptr; }
    double get_load_factor() const;
    int get_total_splits() const { return total_splits; }
    int get_total_collisions() const { return total_collisions; }

    /**
     * Imprime estadísticas detalladas para debugging
     */
    std::string get_statistics() const;
};

#endif // LINEARHASHING_H
