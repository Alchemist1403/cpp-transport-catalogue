#pragma once
#include "geo.h"
#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


// Пространство имён имеет такое же название, как и класс, т.к. в последтсвии можно расширить программу до 
// транспортного справочника для любого вида транспорта (подземного, наземного, воздушного и водного)

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
    double route_length = 0.;
};

class TransportCatalogue {
public:

	void AddStop(std::string name, geo::Coordinates pos);
	void AddBus(std::string name, std::vector<StopPtr> stops);
	StopPtr FindStop(std::string_view bus_name) const;
	BusPtr FindBus(std::string_view bus_name) const;
	BusStat GetStat(BusPtr bus) const;
	const std::unordered_set<BusPtr>& GetBusesByStop(StopPtr stop) const;

private:
	std::deque<Stop> stop_pool_;
	std::deque<Bus> bus_pool_;
	std::unordered_map<std::string_view, StopPtr> stop_by_name_;
	std::unordered_map<std::string_view, BusPtr> bus_by_name_;
	std::unordered_map<StopPtr, std::unordered_set<BusPtr>> bus_by_stop_;

};

}
