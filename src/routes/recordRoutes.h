#ifndef RECORDROUTES_H
#define RECORDROUTES_H

#include "../controllers/RecordController.h"
#include "../../httplib.h"
#include <memory>

class RecordRoutes {
private:
    std::shared_ptr<RecordController> controller;

public:
    RecordRoutes();
    void setupRoutes(httplib::Server& server);
};

#endif // RECORDROUTES_H

