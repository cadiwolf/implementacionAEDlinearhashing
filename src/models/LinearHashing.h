#ifndef LINEARHASHING_H
#define LINEARHASHING_H

#include <string>
#include <vector>
#include <memory>
#include <optional>

// Estructura para almacenar un registro (clave-valor)
struct Record {
    std::string key;
    std::string value;

    Record(const std::string& k, const std::string& v) : key(k), value(v) {}
};

// Nodo de la lista enlazada para manejar colisiones
struct Node {
    Record record;
    std::shared_ptr<Node> next;

    Node(const std::string& k, const std::string& v)
        : record(k, v), next(nullptr) {}
};

// Bucket que contiene una lista enlazada de registros
struct Bucket {
    std::shared_ptr<Node> head;
    int count;

    Bucket() : head(nullptr), count(0) {}
};

class LinearHashing {
private:
    std::vector<Bucket> buckets;
    int level;              // Nivel actual de hashing
    int next;               // Próximo bucket a dividir
    int numRecords;         // Número total de registros
    int initialBuckets;     // Número inicial de buckets
    double loadFactor;      // Factor de carga máximo

    // Función hash
    int hash(const std::string& key, int level) const;

    // Obtiene el índice del bucket para una clave
    int getBucketIndex(const std::string& key) const;

    // Divide un bucket
    void split();

    // Verifica si necesita hacer split
    bool needsSplit() const;

public:
    LinearHashing(int initialSize = 4, double maxLoadFactor = 0.75);

    // Operaciones principales
    bool insert(const std::string& key, const std::string& value);
    std::optional<std::string> search(const std::string& key) const;
    bool remove(const std::string& key);
    std::vector<Record> getAllRecords() const;

    // Información del estado
    int size() const { return numRecords; }
    int bucketCount() const { return buckets.size(); }
};

#endif // LINEARHASHING_H

