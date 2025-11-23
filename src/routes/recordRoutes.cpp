#include "recordRoutes.h"
#include <sstream>

RecordRoutes::RecordRoutes() {
    controller = std::make_shared<RecordController>();
}

// Función auxiliar para parsear JSON simple (key y value)
std::pair<std::string, std::string> parseJson(const std::string& json) {
    std::string key, value;

    size_t keyPos = json.find("\"key\"");
    size_t valuePos = json.find("\"value\"");

    if (keyPos != std::string::npos) {
        size_t start = json.find(":", keyPos) + 1;
        start = json.find("\"", start) + 1;
        size_t end = json.find("\"", start);
        key = json.substr(start, end - start);
    }

    if (valuePos != std::string::npos) {
        size_t start = json.find(":", valuePos) + 1;
        start = json.find("\"", start) + 1;
        size_t end = json.find("\"", start);
        value = json.substr(start, end - start);
    }

    return {key, value};
}

void RecordRoutes::setupRoutes(httplib::Server& server) {
    // CORS preflight (permite conexiones desde el navegador)
    server.Options(R"(/.*)", [](const httplib::Request& /*req*/, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 200;
    });

    // POST /records - Crear un nuevo registro
    server.Post("/records", [this](const httplib::Request& req, httplib::Response& res) {
        // CORS
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        auto [key, value] = parseJson(req.body);

        if (key.empty()) {
            res.status = 400;
            res.set_content("{\"error\":\"Invalid JSON or missing key\"}", "application/json");
            return;
        }

        std::string response = controller->createRecord(key, value);
        res.set_content(response, "application/json");
    });

    // GET /records - Obtener todos los registros
    server.Get("/records", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        std::string response = controller->getAllRecords();
        res.set_content(response, "application/json");
    });

    // GET /records/:key - Obtener un registro por clave
    server.Get(R"(/records/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        std::string key = req.matches[1];
        std::string response = controller->getRecord(key);

        if (response.find("error") != std::string::npos) {
            res.status = 404;
        }

        res.set_content(response, "application/json");
    });

    // DELETE /records/:key - Eliminar un registro por clave
    server.Delete(R"(/records/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        std::string key = req.matches[1];
        std::string response = controller->deleteRecord(key);

        if (response.find("error") != std::string::npos) {
            res.status = 404;
        }

        res.set_content(response, "application/json");
    });
}
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
