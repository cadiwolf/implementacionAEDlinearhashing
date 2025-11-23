#include "LinearHashing.h"
#include <functional>
#include <cmath>
#include <string>
#include <memory>
#include <optional>

LinearHashing::LinearHashing(int initialSize, double maxLoadFactor)
    : level(0), next(0), numRecords(0), initialBuckets(initialSize), loadFactor(maxLoadFactor) {
    buckets.resize(initialSize);
}

int LinearHashing::hash(const std::string& key, int lvl) const {
    std::hash<std::string> hasher;
    size_t hashValue = hasher(key);
    int bucketCount = initialBuckets * (1 << lvl); // initialBuckets * 2^level
    return hashValue % bucketCount;
}

int LinearHashing::getBucketIndex(const std::string& key) const {
    int index = hash(key, level);

    // Si el índice es menor que 'next', usar level + 1
    if (index < next) {
        index = hash(key, level + 1);
    }

    return index;
}

bool LinearHashing::needsSplit() const {
    double currentLoad = static_cast<double>(numRecords) / buckets.size();
    return currentLoad > loadFactor;
}

void LinearHashing::split() {
    // Crear un nuevo bucket
    buckets.emplace_back();

    // Obtener el bucket a dividir
    Bucket& oldBucket = buckets[next];
    Bucket& newBucket = buckets.back();

    // Redistribuir los registros
    std::shared_ptr<Node> current = oldBucket.head;
    std::shared_ptr<Node> prev = nullptr;

    while (current != nullptr) {
        int newIndex = hash(current->record.key, level + 1);

        if (newIndex != next) {
            // Mover el nodo al nuevo bucket
            auto nodeToMove = current;
            current = current->next;

            // Remover del bucket viejo
            if (prev == nullptr) {
                oldBucket.head = current;
            } else {
                prev->next = current;
            }
            oldBucket.count--;

            // Agregar al nuevo bucket
            nodeToMove->next = newBucket.head;
            newBucket.head = nodeToMove;
            newBucket.count++;
        } else {
            prev = current;
            current = current->next;
        }
    }

    // Actualizar next y level
    next++;
    if (next >= initialBuckets * (1 << level)) {
        level++;
        next = 0;
    }
}

bool LinearHashing::insert(const std::string& key, const std::string& value) {
    int index = getBucketIndex(key);

    // Verificar si la clave ya existe
    auto current = buckets[index].head;
    while (current != nullptr) {
        if (current->record.key == key) {
            // Actualizar el valor existente
            current->record.value = value;
            return true;
        }
        current = current->next;
    }

    // Insertar nuevo nodo al inicio del bucket
    auto newNode = std::make_shared<Node>(key, value);
    newNode->next = buckets[index].head;
    buckets[index].head = newNode;
    buckets[index].count++;
    numRecords++;

    // Verificar si necesita split
    if (needsSplit()) {
        split();
    }

    return true;
}

std::optional<std::string> LinearHashing::search(const std::string& key) const {
    int index = getBucketIndex(key);

    auto current = buckets[index].head;
    while (current != nullptr) {
        if (current->record.key == key) {
            return current->record.value;
        }
        current = current->next;
    }

    return std::nullopt;
}

bool LinearHashing::remove(const std::string& key) {
    int index = getBucketIndex(key);

    auto current = buckets[index].head;
    std::shared_ptr<Node> prev = nullptr;

    while (current != nullptr) {
        if (current->record.key == key) {
            // Eliminar el nodo
            if (prev == nullptr) {
                buckets[index].head = current->next;
            } else {
                prev->next = current->next;
            }
            buckets[index].count--;
            numRecords--;
            return true;
        }
        prev = current;
        current = current->next;
    }

    return false;
}

std::vector<Record> LinearHashing::getAllRecords() const {
    std::vector<Record> records;

    for (const auto& bucket : buckets) {
        auto current = bucket.head;
        while (current != nullptr) {
            records.push_back(current->record);
            current = current->next;
        }
    }

    return records;
}
