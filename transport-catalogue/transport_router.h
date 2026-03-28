#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace transport_router {

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

struct RouteItem {
    std::string type;
    std::string stop_name;
    std::string bus;
    int span_count = 0;
    double time = 0.0;
};

struct Route {
    double total_time = 0.0;
    std::vector<RouteItem> items;
    bool found = false;
};

class TransportRouter {
public:
    TransportRouter(const transport_catalogue::TransportCatalogue& catalogue,
                    const RoutingSettings& settings);
    
    Route BuildRoute(const std::string& from_stop, const std::string& to_stop) const;
    
private:
    using Graph = graph::DirectedWeightedGraph<double>;
    using Router = graph::Router<double>;
    
    void BuildGraph();
    
    const transport_catalogue::TransportCatalogue& catalogue_;
    RoutingSettings settings_;
    
    std::unique_ptr<Graph> graph_;
    std::unique_ptr<Router> router_;
    
    std::unordered_map<std::string, graph::VertexId> stop_name_to_vertex_;
    std::unordered_map<graph::VertexId, std::string> vertex_to_stop_name_;
    
    struct EdgeInfo {
        std::string bus_name;
        int span_count;
    };
    std::unordered_map<graph::EdgeId, EdgeInfo> edge_info_;
};

}  // namespace transport_router