#include "request_handler.h"
#include <algorithm>

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db): db_(db) {
}

std::optional<transport_catalogue::BusStat> RequestHandler::GetBusStat(std::string_view bus_name) const {
    auto bus = db_.FindBus(bus_name);

    if (!bus) {
        return std::nullopt;
    }

    return db_.GetStat(bus);
}

const std::unordered_set<transport_catalogue::BusPtr>* RequestHandler::GetBusesByStop(std::string_view stop_name) const {
    auto stop = db_.FindStop(stop_name);
    if (!stop) {
        return nullptr;
    }
    return &db_.GetBusesByStop(stop);
}

std::pair<std::vector<transport_catalogue::BusPtr>, std::vector<transport_catalogue::StopPtr>> RequestHandler::GetAllBusesAndStops(
        const std::vector<std::string>& bus_names,
        const std::vector<std::string>& stop_names
    ) const {
    
    std::vector<transport_catalogue::BusPtr> buses;
    buses.reserve(bus_names.size());
    for (const auto& name : bus_names) {
        auto bus = db_.FindBus(name);
        if (bus) {
            buses.push_back(bus);
        }
    }
    
    std::vector<transport_catalogue::StopPtr> stops;
    stops.reserve(stop_names.size());
    for (const auto& name : stop_names) {
        auto stop = db_.FindStop(name);
        if (stop) {
            stops.push_back(stop);
        }
    }
    
    return {buses, stops};
}