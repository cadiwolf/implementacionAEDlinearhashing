#include "httplib.h"
#include "src/routes/recordRoutes.h"
#include <iostream>

int main() {
    httplib::Server server;

    // Configurar las rutas
    RecordRoutes routes;
    routes.setupRoutes(server);

    // CORS global preflight (por si no se maneja en rutas)
    server.Options(R"(/.*)", [](const httplib::Request& /*req*/, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 200;
    });

    // Ruta de bienvenida
    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(
            "{\"message\":\"Linear Hashing REST API\","
            "\"version\":\"1.0\","
            "\"endpoints\":["
            "\"POST /records\","
            "\"GET /records\","
            "\"GET /records/{key}\","
            "\"DELETE /records/{key}\""
            "]}",
            "application/json"
        );
    });

    std::cout << "Servidor iniciado en http://localhost:8080" << std::endl;
    std::cout << "Presiona Ctrl+C para detener el servidor" << std::endl;

    // Iniciar el servidor en el puerto 8080
    if (!server.listen("0.0.0.0", 8080)) {
        std::cerr << "Error: No se pudo iniciar el servidor en el puerto 8080" << std::endl;
        return 1;
    }

    return 0;
}
