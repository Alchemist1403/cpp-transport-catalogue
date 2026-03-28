#include "request_handler.h"
#include <algorithm>
#include <memory>

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db,
                               const transport_router::RoutingSettings& routing_settings)
    : db_(db)
    , router_(std::make_unique<transport_router::TransportRouter>(db, routing_settings))
{
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

std::vector<transport_catalogue::BusPtr> RequestHandler::GetAllBusesSorted() const {
    std::vector<transport_catalogue::BusPtr> buses_sorted;
    buses_sorted.reserve(db_.GetAllBuses().size());
    for (const auto& bus : db_.GetAllBuses()) {
        buses_sorted.push_back(&bus);
    }
    std::sort(buses_sorted.begin(), buses_sorted.end(),
              [](const transport_catalogue::Bus* a, const transport_catalogue::Bus* b) {
                  return a->name < b->name;
              });
    return buses_sorted;
}

std::vector<transport_catalogue::StopPtr> RequestHandler::GetAllStopsSorted() const {
    std::vector<transport_catalogue::StopPtr> stops_sorted;
    stops_sorted.reserve(db_.GetAllStops().size());
    for (const auto& stop : db_.GetAllStops()) {
        stops_sorted.push_back(&stop);
    }
    std::sort(stops_sorted.begin(), stops_sorted.end(),
              [](const transport_catalogue::Stop* a, const transport_catalogue::Stop* b) {
                  return a->name < b->name;
              });
    return stops_sorted;
}

transport_router::Route RequestHandler::BuildRoute(std::string_view from_stop, std::string_view to_stop) const {
    return router_->GetOptimalRoute(from_stop, to_stop);
}