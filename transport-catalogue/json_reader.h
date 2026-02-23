#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

namespace json_reader {

void LoadBaseRequests(const json::Document& doc, transport_catalogue::TransportCatalogue& catalogue);
json::Document ProcessStatRequests(const json::Document& doc, const RequestHandler& handler);

render::RenderSettings LoadRenderSettings(const json::Document& doc);
std::pair<std::vector<std::string>, std::vector<std::string>> ExtractRouteNames(const json::Document& doc);

}  // namespace json_reader