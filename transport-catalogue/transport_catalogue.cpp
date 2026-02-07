#include "transport_catalogue.h"
#include <unordered_set>
#include <optional>
#include <iostream>

using std::string;
using std::string_view;

namespace transport_catalogue {

// Методы добавления
void TransportCatalogue::AddStop(string name, geo::Coordinates pos) {
    stop_pool_.push_back({std::move(name), pos});
    auto added_ptr = &stop_pool_.back();
    stop_by_name_[added_ptr->name] = added_ptr;
}

void TransportCatalogue::AddBus(string name, std::vector<StopPtr> stops) {
    bus_pool_.push_back({std::move(name), std::move(stops)});
    auto added_bus = &bus_pool_.back();
    bus_by_name_[added_bus->name] = added_bus;

    for (auto stop : added_bus->stops) {
        bus_by_stop_[stop].insert(added_bus);
    }
}

// Методы поиска
StopPtr transport_catalogue::TransportCatalogue::FindStop(string_view bus_name) const {
    auto iter = stop_by_name_.find(bus_name);
    if (iter == stop_by_name_.end()) {
            return nullptr;
    }
    return iter->second;
}

BusPtr transport_catalogue::TransportCatalogue::FindBus(string_view bus_name) const {
    auto iter = bus_by_name_.find(bus_name);
    if (iter == bus_by_name_.end()) {
        return nullptr;
    }
    return iter->second;
}

// Методы получения информации
const std::unordered_set<transport_catalogue::BusPtr>& TransportCatalogue::GetBusesByStop(StopPtr stop) const {
    static const std::unordered_set<BusPtr> dummy;
    auto iter = bus_by_stop_.find(stop);
    return iter == bus_by_stop_.end() ? dummy : iter->second;
}

BusStat transport_catalogue::TransportCatalogue::GetStat(transport_catalogue::BusPtr bus) const {
    BusStat stat;

    std::unordered_set<string_view> seen_stops;
    std::optional<geo::Coordinates> prev_pos;

    for (auto stop: bus->stops) {
        ++stat.total_stops;

        if (seen_stops.count(stop->name) == 0) {
            ++stat.unique_stops;
            seen_stops.insert(stop->name);
        }

        if (prev_pos) {
            stat.route_length += ComputeDistance(*prev_pos, stop->position);
        }
        prev_pos = stop->position;
    }
    return stat;
}

}
