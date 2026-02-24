#pragma once
#include "transport_catalogue.h"
#include <optional>
#include <string_view>

class RequestHandler {
public:
    explicit RequestHandler(const transport_catalogue::TransportCatalogue& db);

    std::optional<transport_catalogue::BusStat> GetBusStat(std::string_view bus_name) const;
    const std::unordered_set<transport_catalogue::BusPtr>* GetBusesByStop(std::string_view stop_name) const;
    std::pair<const std::vector<transport_catalogue::BusPtr>&, const std::vector<transport_catalogue::StopPtr>&> GetAllBusesAndStops() const;

private:
    const transport_catalogue::TransportCatalogue& db_;
};