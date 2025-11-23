#ifndef RECORDCONTROLLER_H
#define RECORDCONTROLLER_H

#include "../models/LinearHashing.h"
#include <string>
#include <memory>

class RecordController {
private:
    std::shared_ptr<LinearHashing> storage;

public:
    RecordController();

    // Métodos para manejar las operaciones
    std::string createRecord(const std::string& key, const std::string& value);
    std::string getRecord(const std::string& key);
    std::string getAllRecords();
    std::string deleteRecord(const std::string& key);

    // Método auxiliar para generar JSON
    static std::string toJson(const std::string& key, const std::string& value);
    static std::string errorJson(const std::string& message);
    static std::string successJson(const std::string& message);
};

#endif // RECORDCONTROLLER_H
