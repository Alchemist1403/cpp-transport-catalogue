#include "transport_router.h"
#include <algorithm>
#include <cmath>
#include <memory>

namespace transport_router {

TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue& catalogue,
                                  const RoutingSettings& settings)
    : catalogue_(catalogue)
    , settings_(settings)
    , graph_(0)
{
    BuildGraph();
}


void TransportRouter::AddRouteEdge(const std::string& bus_name,
                                    transport_catalogue::StopPtr stop_from,
                                    transport_catalogue::StopPtr stop_to,
                                    int& total_distance,
                                    graph::VertexId from,
                                    graph::VertexId to,
                                    int span_count) {

    int segment_distance = catalogue_.GetDistance(stop_from, stop_to);
    if (segment_distance == 0) {
        return;
    }
                    
    total_distance += segment_distance;

    double travel_time = total_distance / settings_.bus_velocity * 0.06 + settings_.bus_wait_time;
    
    graph::Edge<double> edge;
    edge.from = from;
    edge.to = to;
    edge.weight = travel_time;
    
    graph::EdgeId edge_id = graph_.AddEdge(edge);
    edge_info_[edge_id] = {bus_name, span_count};
}



void TransportRouter::BuildGraph() {
    std::unordered_map<std::string, graph::VertexId> stop_to_id;
    
    for (const auto& stop : catalogue_.GetAllStops()) {
        if (stop_to_id.find(stop.name) == stop_to_id.end()) {
            graph::VertexId id = stop_to_id.size();
            stop_to_id[stop.name] = id;
        }
    }
    
    graph_ = Graph(stop_to_id.size());
    
    stop_name_to_vertex_ = stop_to_id;
    for (const auto& [name, id] : stop_to_id) {
        vertex_to_stop_name_[id] = name;
    }
    
    for (const auto& bus : catalogue_.GetAllBuses()) {
        if (bus.stops.empty()) {
            continue;
        }
        
        if (bus.is_roundtrip) {
            size_t n = bus.stops.size() - 1;

            for (size_t i = 0; i < n; ++i) {

                int total_distance = 0;
                for (size_t span = 1; span <= (n-i); ++span) {
                    AddRouteEdge(bus.name,
                                bus.stops[((i + span - 1) % n)],
                                bus.stops[((i + span) % n)],
                                total_distance,
                                stop_to_id.at(bus.stops[i]->name),
                                stop_to_id.at(bus.stops[((i + span) % n)]->name),
                                static_cast<int>(span));
                }
            }


        } else {

            for (size_t i = 0; i < bus.stops.size(); ++i) {

                int total_distance = 0;
                for (size_t j = i + 1; j < bus.stops.size(); ++j) { 
                    AddRouteEdge(bus.name,
                                bus.stops[j-1],
                                bus.stops[j],
                                total_distance,
                                stop_to_id.at(bus.stops[i]->name),
                                stop_to_id.at(bus.stops[j]->name),
                                static_cast<int>(j - i));

                }
            }


            for (size_t i = 1; i < bus.stops.size(); ++i) {

                int total_distance = 0;
                for (size_t j = i; j > 0; --j) {
                    AddRouteEdge(bus.name,
                                bus.stops[j],
                                bus.stops[j-1],
                                total_distance,
                                stop_to_id.at(bus.stops[i]->name),
                                stop_to_id.at(bus.stops[j - 1]->name),
                                static_cast<int>(i - j + 1));
                }
            }

        }
    }

    router_ = std::make_unique<Router>(graph_);
}

Route TransportRouter::GetOptimalRoute(std::string_view from_stop, std::string_view to_stop) const {
    Route result;
    auto from_it = stop_name_to_vertex_.find(std::string(from_stop));
    auto to_it = stop_name_to_vertex_.find(std::string(to_stop));
    
    if (from_it == stop_name_to_vertex_.end() || to_it == stop_name_to_vertex_.end()) {
        result.found = false;
        return result;
    }
    
    graph::VertexId from_vertex = from_it->second;
    graph::VertexId to_vertex = to_it->second;
    
    if (from_vertex == to_vertex) {
        result.found = true;
        result.total_time = 0.0;
        result.items = {};
        return result;
    }
    
    auto route_info = router_->BuildRoute(from_vertex, to_vertex);
    
    if (!route_info) {
        result.found = false;
        return result;
    }
    
    result.found = true;
    result.total_time = route_info->weight;
    
    std::string current_stop(from_stop);
    
    for (graph::EdgeId edge_id : route_info->edges) {
        RouteItem wait_item;
        wait_item.type = "Wait";
        wait_item.stop_name = current_stop;
        wait_item.time = static_cast<double>(settings_.bus_wait_time);
        result.items.push_back(wait_item);
        
        const auto& edge = graph_.GetEdge(edge_id);
        auto edge_info_it = edge_info_.find(edge_id);
        
        std::string bus_name;
        int span_count = 1;
        double travel_time = edge.weight - settings_.bus_wait_time;
        
        if (edge_info_it != edge_info_.end()) {
            bus_name = edge_info_it->second.bus_name;
            span_count = edge_info_it->second.span_count;
        }
        
        RouteItem bus_item;
        bus_item.type = "Bus";
        bus_item.bus = bus_name;
        bus_item.span_count = span_count;
        bus_item.time = travel_time;
        result.items.push_back(bus_item);
        current_stop = vertex_to_stop_name_.at(edge.to);
    }
    
    return result;
}

}  // namespace transport_router