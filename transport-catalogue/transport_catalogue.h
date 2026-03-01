#pragma once
#include "geo.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <algorithm>

namespace transport_catalogue {

struct Stop {
    std::string name;
    geo::Coordinates position;
};

using StopPtr = const Stop*;

struct Bus {
    std::string name;
    std::vector<StopPtr> stops;
    bool is_roundtrip = false;
};

using BusPtr = const Bus*;

struct BusStat {
    size_t total_stops = 0;
    size_t unique_stops = 0;
    size_t route_length = 0;
    double geographic_distance = 0.;
};

class TransportCatalogue {
public:
    void AddStop(const std::string& name, geo::Coordinates pos);
    void AddBus(const std::string& name, const std::vector<StopPtr>& stops, bool is_roundtrip = false);

    StopPtr FindStop(std::string_view name) const;
    BusPtr FindBus(std::string_view name) const;

    BusStat GetStat(BusPtr bus) const;
    const std::unordered_set<BusPtr>& GetBusesByStop(StopPtr stop) const;

    void SetDistance(StopPtr from, StopPtr to, int meters);
    int GetDistance(StopPtr from, StopPtr to) const;


    const std::deque<Bus>& GetAllBuses() const { return bus_pool_; }
    const std::deque<Stop>& GetAllStops() const { return stop_pool_; }

    void Sorting();
    const std::vector<BusPtr>& GetAllBusesSorted() const { return buses_sorted_; }
    const std::vector<StopPtr>& GetAllStopsSorted() const { return stops_sorted_; }

private:
    struct PairHasher {
        template <typename First, typename Second>
        size_t operator()(const std::pair<First, Second>& obj) const {
            return std::hash<First>()(obj.first) + 37 * std::hash<Second>()(obj.second);
        }
    };

    std::deque<Stop> stop_pool_;
    std::deque<Bus> bus_pool_;
    
    // Отсортированные векторы. Сортировка производится методом Sorting один раз после заполнения БД
    std::vector<BusPtr> buses_sorted_;
    std::vector<StopPtr> stops_sorted_;

    std::unordered_map<std::string_view, StopPtr> stop_by_name_;
    std::unordered_map<std::string_view, BusPtr> bus_by_name_;
    std::unordered_map<StopPtr, std::unordered_set<BusPtr>> bus_by_stop_;
    std::unordered_map<std::pair<StopPtr, StopPtr>, int, PairHasher> distances_;
};

}  // namespace transport_catalogue