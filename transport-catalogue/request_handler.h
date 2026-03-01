#pragma once
#include "transport_catalogue.h"
#include <optional>
#include <string_view>

class RequestHandler {
public:
    explicit RequestHandler(const transport_catalogue::TransportCatalogue& db);

    std::optional<transport_catalogue::BusStat> GetBusStat(std::string_view bus_name) const;
    const std::unordered_set<transport_catalogue::BusPtr>* GetBusesByStop(std::string_view stop_name) const;
    const std::vector<transport_catalogue::BusPtr>& GetAllBusesSorted() const { return db_.GetAllBusesSorted(); } 
    const std::vector<transport_catalogue::StopPtr>& GetAllStopsSorted() const { return db_.GetAllStopsSorted(); } 

private:
    const transport_catalogue::TransportCatalogue& db_;
};