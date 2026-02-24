#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"

using namespace std::literals;

int main() {
    json::Document doc = json::Load(std::cin);
    
    transport_catalogue::TransportCatalogue catalogue;
    json_reader::LoadBaseRequests(doc, catalogue);

    RequestHandler request_handler(catalogue);
    auto result = json_reader::ProcessStatRequests(doc, request_handler);
    
    json::Print(result, std::cout);
    
    return 0;
}