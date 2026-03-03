#include "transport_catalogue.h"
#include "geo.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates pos) {
    stop_pool_.push_back({std::move(name), pos});
    auto added_ptr = &stop_pool_.back();
    stop_by_name_[added_ptr->name] = added_ptr;
}

void TransportCatalogue::AddBus(const std::string& name, const std::vector<StopPtr>& stops, bool is_roundtrip) {
    bus_pool_.push_back({std::move(name), std::move(stops), is_roundtrip});
    auto added_bus = &bus_pool_.back();
    bus_by_name_[added_bus->name] = added_bus;
    for (auto stop : added_bus->stops) {
        bus_by_stop_[stop].insert(added_bus);
    }
}

StopPtr TransportCatalogue::FindStop(std::string_view name) const {
    auto iter = stop_by_name_.find(name);
    if (iter == stop_by_name_.end()) {
        return nullptr;
    }
    return iter->second;
}

BusPtr TransportCatalogue::FindBus(std::string_view name) const {
    auto iter = bus_by_name_.find(name);
    if (iter == bus_by_name_.end()) {
        return nullptr;
    }
    return iter->second;
}

const std::unordered_set<BusPtr>& TransportCatalogue::GetBusesByStop(StopPtr stop) const {
    static const std::unordered_set<BusPtr> dummy;
    auto iter = bus_by_stop_.find(stop);
    return iter == bus_by_stop_.end() ? dummy : iter->second;
}

BusStat TransportCatalogue::GetStat(BusPtr bus) const {
    BusStat stat;
    std::unordered_set<std::string_view> seen_stops;
    
    for (size_t i = 0; i < bus->stops.size(); ++i) {
        auto stop = bus->stops[i];
        ++stat.total_stops;
        if (seen_stops.count(stop->name) == 0) {
            ++stat.unique_stops;
            seen_stops.insert(stop->name);
        }
        if (i > 0) {
            auto prev = bus->stops[i-1];
            stat.route_length += GetDistance(prev, stop);
            stat.geographic_distance += geo::ComputeDistance(prev->position, stop->position);
        }
    }
    
    if (!bus->is_roundtrip && bus->stops.size() > 1) {
        for (size_t i = bus->stops.size() - 1; i > 0; --i) {
            auto from = bus->stops[i];
            auto to = bus->stops[i-1];
            stat.route_length += GetDistance(from, to);
            stat.geographic_distance += geo::ComputeDistance(from->position, to->position);
            ++stat.total_stops;
        }
    }
    
    return stat;
}

void TransportCatalogue::SetDistance(StopPtr from, StopPtr to, int meters) {
    if (from && to) {
        distances_[{from, to}] = meters;
    }
}

int TransportCatalogue::GetDistance(StopPtr from, StopPtr to) const {
    if (from == to) {
        return 0;
    }

    auto it = distances_.find({from, to});
    if (it != distances_.end()) {
        return it->second;
    }

    auto rev_it = distances_.find({to, from});
    if (rev_it != distances_.end()) {
        return rev_it->second;
    }
    return 0;
}

}  // namespace transport_catalogue
