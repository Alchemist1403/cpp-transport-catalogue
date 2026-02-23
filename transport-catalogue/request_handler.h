#pragma once
#include "transport_catalogue.h"
#include <optional>
#include <string_view>

class RequestHandler {
public:
    explicit RequestHandler(const transport_catalogue::TransportCatalogue& db);

    std::optional<transport_catalogue::BusStat> GetBusStat(std::string_view bus_name) const;
    const std::unordered_set<transport_catalogue::BusPtr>* GetBusesByStop(std::string_view stop_name) const;

    std::pair<std::vector<transport_catalogue::BusPtr>, std::vector<transport_catalogue::StopPtr>> GetAllRoutesForRendering(const std::vector<std::string>& bus_names, const std::vector<std::string>& stop_names) const;

private:
    const transport_catalogue::TransportCatalogue& db_;
};