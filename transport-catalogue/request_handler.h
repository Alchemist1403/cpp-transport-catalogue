#pragma once
#include "transport_catalogue.h"
#include "transport_router.h"
#include <optional>
#include <string_view>
#include <memory>

class RequestHandler {
public:
    explicit RequestHandler(const transport_catalogue::TransportCatalogue& db,
                           const transport_router::RoutingSettings& routing_settings);
    
    std::optional<transport_catalogue::BusStat> GetBusStat(std::string_view bus_name) const;
    const std::unordered_set<transport_catalogue::BusPtr>* GetBusesByStop(std::string_view stop_name) const;
    std::vector<transport_catalogue::BusPtr> GetAllBusesSorted() const;
    std::vector<transport_catalogue::StopPtr> GetAllStopsSorted() const;
    
    transport_router::Route BuildRoute(const std::string& from_stop, const std::string& to_stop) const;
    
private:
    const transport_catalogue::TransportCatalogue& db_;
    std::unique_ptr<transport_router::TransportRouter> router_;
};