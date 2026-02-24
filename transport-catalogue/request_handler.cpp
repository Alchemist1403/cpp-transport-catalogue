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

std::pair<const std::vector<transport_catalogue::BusPtr>&, const std::vector<transport_catalogue::StopPtr>&>  RequestHandler::GetAllBusesAndStops() const {
    return {db_.GetAllBusesSorted(), db_.GetAllStopsSorted()};
}

