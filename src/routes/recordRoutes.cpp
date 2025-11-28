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
    server.Get("/records", [this](const httplib::Request& /*req*/, httplib::Response& res) {
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

    // GET /statistics - Obtener estadísticas del Linear Hashing
    server.Get("/statistics", [this](const httplib::Request& /*req*/, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        std::string response = controller->getStatistics();
        res.set_content(response, "application/json");
    });

    // GET /buckets - Obtener información detallada de buckets
    server.Get("/buckets", [this](const httplib::Request& /*req*/, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        std::string response = controller->getBucketsInfo();
        res.set_content(response, "application/json");
    });

    // GET /state - Obtener el estado completo del hash table
    server.Get("/state", [this](const httplib::Request& /*req*/, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        std::string response = controller->getHashTableState();
        res.set_content(response, "application/json");
    });
}
