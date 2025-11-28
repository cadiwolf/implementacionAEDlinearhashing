#include "RecordController.h"
#include <sstream>

RecordController::RecordController() {
    // Iniciar con 2 buckets (más estándar y didáctico)
    storage = std::make_shared<LinearHashing>(2, 0.75);
}

std::string RecordController::toJson(const std::string& key, const std::string& value) {
    std::ostringstream oss;
    oss << "{\"key\":\"" << key << "\",\"value\":\"" << value << "\"}";
    return oss.str();
}

std::string RecordController::errorJson(const std::string& message) {
    std::ostringstream oss;
    oss << "{\"error\":\"" << message << "\"}";
    return oss.str();
}

std::string RecordController::successJson(const std::string& message) {
    std::ostringstream oss;
    oss << "{\"message\":\"" << message << "\"}";
    return oss.str();
}

std::string RecordController::createRecord(const std::string& key, const std::string& value) {
    if (key.empty()) {
        return errorJson("Key cannot be empty");
    }

    storage->insert(key, value);
    return toJson(key, value);
}

std::string RecordController::getRecord(const std::string& key) {
    auto result = storage->search(key);

    if (result.has_value()) {
        return toJson(key, result.value());
    } else {
        return errorJson("Record not found");
    }
}

std::string RecordController::getAllRecords() {
    auto records = storage->getAllRecords();

    std::ostringstream oss;
    oss << "{\"count\":" << records.size() << ",\"records\":[";

    for (size_t i = 0; i < records.size(); i++) {
        oss << toJson(records[i].key, records[i].value);
        if (i < records.size() - 1) {
            oss << ",";
        }
    }

    oss << "]}";
    return oss.str();
}

std::string RecordController::deleteRecord(const std::string& key) {
    bool removed = storage->remove(key);

    if (removed) {
        return successJson("Record deleted successfully");
    } else {
        return errorJson("Record not found");
    }
}

// ==================== MÉTODOS PARA VISUALIZACIÓN ====================

/**
 * Obtiene estadísticas generales del Linear Hashing
 */
std::string RecordController::getStatistics() {
    try {
        std::ostringstream oss;
        oss << "{";
        oss << "\"numEntries\":" << storage->size() << ",";
        oss << "\"numBuckets\":" << storage->bucket_count() << ",";
        oss << "\"level\":" << storage->get_level() << ",";
        oss << "\"splitPointer\":" << storage->get_split_pointer() << ",";
        oss << "\"loadFactor\":" << storage->get_load_factor() << ",";
        oss << "\"totalSplits\":" << storage->get_total_splits() << ",";
        oss << "\"totalCollisions\":" << storage->get_total_collisions();
        oss << "}";
        return oss.str();
    } catch (const std::exception& e) {
        return errorJson(std::string("Error al obtener estadísticas: ") + e.what());
    }
}

/**
 * Obtiene información detallada de cada bucket
 */
std::string RecordController::getBucketsInfo() {
    try {
        std::ostringstream oss;

        oss << "{\"buckets\":[";

        int numBuckets = storage->bucket_count();

        // Por ahora, retornamos estructura básica de buckets
        for (int i = 0; i < numBuckets; i++) {
            oss << "{\"index\":" << i << ",\"count\":0,\"entries\":[]}";
            if (i < numBuckets - 1) oss << ",";
        }

        oss << "]}";
        return oss.str();
    } catch (const std::exception& e) {
        return errorJson(std::string("Error al obtener buckets: ") + e.what());
    }
}

/**
 * Obtiene el estado completo del hash table para visualización
 */
std::string RecordController::getHashTableState() {
    try {
        std::ostringstream oss;

        oss << "{";

        // ==================== ESTADÍSTICAS ====================
        oss << "\"stats\":{";
        oss << "\"numEntries\":" << storage->size() << ",";
        oss << "\"numBuckets\":" << storage->bucket_count() << ",";
        oss << "\"level\":" << storage->get_level() << ",";
        oss << "\"splitPointer\":" << storage->get_split_pointer() << ",";
        oss << "\"loadFactor\":" << storage->get_load_factor() << ",";
        oss << "\"totalSplits\":" << storage->get_total_splits() << ",";
        oss << "\"totalCollisions\":" << storage->get_total_collisions();
        oss << "},";

        // ==================== BUCKETS CON ENTRADAS ====================
        oss << "\"buckets\":[";

        auto buckets_data = storage->get_buckets_with_entries();

        for (size_t i = 0; i < buckets_data.size(); i++) {
            const auto& [index, count, entries] = buckets_data[i];

            oss << "{";
            oss << "\"index\":" << index << ",";
            oss << "\"count\":" << count << ",";
            oss << "\"entries\":[";

            for (size_t j = 0; j < entries.size(); j++) {
                oss << toJson(entries[j].key, entries[j].value);
                if (j < entries.size() - 1) oss << ",";
            }

            oss << "]}";
            if (i < buckets_data.size() - 1) oss << ",";
        }

        oss << "]";
        oss << "}";

        return oss.str();
    } catch (const std::exception& e) {
        return errorJson(std::string("Error al obtener estado: ") + e.what());
    }
}
