#pragma once
#include "geo.h"
#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace transport_catalogue {


struct Stop {
    std::string name;
    geo::Coordinates position;
};
using StopPtr = const Stop*;

struct Bus {
    std::string name;
    std::vector<StopPtr> stops;
};
using BusPtr = const Bus*;

struct BusStat {
    size_t total_stops = 0;
    size_t unique_stops = 0;
    size_t route_length = 0.;
    double geographic_distance  = 0.;
};


class TransportCatalogue {
public:

	void AddStop(const std::string& name, geo::Coordinates pos);
	void AddBus(const std::string& name, const std::vector<StopPtr>& stops);
    
	StopPtr FindStop(std::string_view bus_name) const;
	BusPtr FindBus(std::string_view bus_name) const;

	BusStat GetStat(BusPtr bus) const;
	const std::unordered_set<BusPtr>& GetBusesByStop(StopPtr stop) const;

    void SetDistance(StopPtr from, StopPtr to, int meters);
    int GetDistance(StopPtr from, StopPtr to) const;

private:

    struct PairHasher {
        template <typename First, typename Second>
        size_t operator()(const std::pair<First, Second>& obj) const {
            return std::hash<First>()(obj.first) + 37 * std::hash<Second>()(obj.second);
        }
    };

	std::deque<Stop> stop_pool_;
	std::deque<Bus> bus_pool_;
	std::unordered_map<std::string_view, StopPtr> stop_by_name_;
	std::unordered_map<std::string_view, BusPtr> bus_by_name_;
	std::unordered_map<StopPtr, std::unordered_set<BusPtr>> bus_by_stop_;
    std::unordered_map<std::pair<StopPtr, StopPtr>, int, PairHasher> distances_;
};

}
