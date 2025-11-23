#include "RecordController.h"
#include <sstream>

RecordController::RecordController() {
    storage = std::make_shared<LinearHashing>(4, 0.75);
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

