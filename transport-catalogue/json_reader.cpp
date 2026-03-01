#include "json_reader.h"
#include "json_builder.h"
#include <sstream>
#include <algorithm>

using namespace std::literals;

namespace json_reader {

svg::Color ParseColorFromArray(const json::Array& arr) {
    if (arr.size() < 3) {
        return svg::Rgb{0, 0, 0};
    }

    uint8_t r = static_cast<uint8_t>(arr[0].AsDouble());
    uint8_t g = static_cast<uint8_t>(arr[1].AsDouble());
    uint8_t b = static_cast<uint8_t>(arr[2].AsDouble());

    if (arr.size() == 4) {
        double a = arr[3].AsDouble();
        return svg::Rgba{r, g, b, a};
    }

    return svg::Rgb{r, g, b};
}

void LoadStop(const json::Dict& stop_dict, transport_catalogue::TransportCatalogue& catalogue) {
    const std::string& name = stop_dict.at("name").AsString();
    double latitude = stop_dict.at("latitude").AsDouble();
    double longitude = stop_dict.at("longitude").AsDouble();
    catalogue.AddStop(name, {latitude, longitude});
}

void SetRoadDistances(const json::Dict& stop_dict, transport_catalogue::TransportCatalogue& catalogue) {
    const std::string& stop_name = stop_dict.at("name").AsString();
    auto stop_ptr = catalogue.FindStop(stop_name);

    if (!stop_ptr) {
        return;
    }

    const auto& road_distances = stop_dict.at("road_distances").AsDict();

    for (const auto& [dest_name, distance_node] : road_distances) {
        auto dest_ptr = catalogue.FindStop(dest_name);
        if (dest_ptr) {
            catalogue.SetDistance(stop_ptr, dest_ptr, distance_node.AsInt());
        }
    }
}

void LoadBus(const json::Dict& bus_dict, transport_catalogue::TransportCatalogue& catalogue) {
    const std::string& name = bus_dict.at("name").AsString();
    bool is_roundtrip = bus_dict.at("is_roundtrip").AsBool();
    std::vector<transport_catalogue::StopPtr> stops;

    for (const auto& stop_node : bus_dict.at("stops").AsArray()) {
        auto stop_ptr = catalogue.FindStop(stop_node.AsString());
        if (stop_ptr) {
            stops.push_back(stop_ptr);
        }
    }

    catalogue.AddBus(name, stops, is_roundtrip);
}

void LoadBaseRequests(const json::Document& doc, transport_catalogue::TransportCatalogue& catalogue) {
    const auto& root = doc.GetRoot().AsDict();
    const auto& base_requests = root.at("base_requests").AsArray();
    
    for (const auto& node : base_requests) {
        const auto& dict = node.AsDict();
        if (dict.at("type").AsString() == "Stop") {
            LoadStop(dict, catalogue);
        }
    }

    for (const auto& node : base_requests) {
        const auto& dict = node.AsDict();
        if (dict.at("type").AsString() == "Stop") {
            SetRoadDistances(dict, catalogue);
        }
    }

    for (const auto& node : base_requests) {
        const auto& dict = node.AsDict();
        if (dict.at("type").AsString() == "Bus") {
            LoadBus(dict, catalogue);
        }
    }
    catalogue.Sorting();
}

json::Node ProcessBusRequest(const json::Dict& request, const RequestHandler& handler) {
    int request_id = request.at("id").AsInt();
    const std::string& bus_name = request.at("name").AsString();
    
    auto stat_opt = handler.GetBusStat(bus_name);
    
    if (!stat_opt) {
        return json::Builder{}
            .StartDict()
                .Key("error_message").Value("not found"s)
                .Key("request_id").Value(request_id)
            .EndDict()
            .Build();
    }
    
    const auto& stat = *stat_opt;
    double curvature = 0.0;
    if (stat.geographic_distance > 0) {
        curvature = static_cast<double>(stat.route_length) / stat.geographic_distance;
    }

    return json::Builder{}
        .StartDict()
            .Key("stop_count").Value(static_cast<int>(stat.total_stops))
            .Key("unique_stop_count").Value(static_cast<int>(stat.unique_stops))
            .Key("route_length").Value(static_cast<int>(stat.route_length))
            .Key("curvature").Value(curvature)
            .Key("request_id").Value(request_id)
        .EndDict()
        .Build();
}

json::Node ProcessStopRequest(const json::Dict& request, const RequestHandler& handler) {
    int request_id = request.at("id").AsInt();
    const std::string& stop_name = request.at("name").AsString();
    
    auto buses_ptr = handler.GetBusesByStop(stop_name);
    
    if (buses_ptr == nullptr) {
        return json::Builder{}
            .StartDict()
                .Key("error_message").Value("not found"s)
                .Key("request_id").Value(request_id)
            .EndDict()
            .Build();
    }
    
    std::vector<std::string> bus_names;
    for (const auto* bus : *buses_ptr) {
        bus_names.push_back(bus->name);
    }
    std::sort(bus_names.begin(), bus_names.end());

    json::Builder array_builder;
    array_builder.StartArray();
    for (const auto& name : bus_names) {
        array_builder.Value(name);
    }
    json::Node buses_array = array_builder.EndArray().Build();

    return json::Builder{}
        .StartDict()
            .Key("buses").Value(buses_array.GetValue())
            .Key("request_id").Value(request_id)
        .EndDict()
        .Build();
}

json::Document ProcessStatRequests(const json::Document& doc, const RequestHandler& handler) {
    const auto& root = doc.GetRoot().AsDict();
    const auto& stat_requests = root.at("stat_requests").AsArray();
    

    json::Builder responses_builder;
    responses_builder.StartArray();
    
    for (const auto& request_node : stat_requests) {
        const auto& request = request_node.AsDict();
        const std::string& type = request.at("type").AsString();
        
        if (type == "Bus") {
            responses_builder.Value(ProcessBusRequest(request, handler).GetValue());
        } else if (type == "Stop") {
            responses_builder.Value(ProcessStopRequest(request, handler).GetValue());
        } else if (type == "Map") {
            int request_id = request.at("id").AsInt();
            auto render_settings = LoadRenderSettings(doc);
            render::MapRenderer renderer(render_settings);
            
            std::ostringstream oss;
            renderer.RenderMap(oss, handler.GetAllBusesSorted(), handler.GetAllStopsSorted());
            std::string svg_string = oss.str();
            
            json::Node map_response = json::Builder{}
                .StartDict()
                    .Key("request_id").Value(request_id)
                    .Key("map").Value(svg_string)
                .EndDict()
                .Build();
            
            responses_builder.Value(map_response.GetValue());
        }
    }
    
    return json::Document(responses_builder.EndArray().Build());
}

render::RenderSettings LoadRenderSettings(const json::Document& doc) {
    render::RenderSettings settings;
    const auto& root = doc.GetRoot().AsDict();
    auto it = root.find("render_settings");

    if (it == root.end() || !it->second.IsDict()) {
        return settings;
    }
    
    const auto& rs = it->second.AsDict();
    
    if (auto w = rs.find("width"); w != rs.end() && w->second.IsDouble()) {
        settings.width = w->second.AsDouble();
    }

    if (auto h = rs.find("height"); h != rs.end() && h->second.IsDouble()) {
        settings.height = h->second.AsDouble();
    }

    if (auto p = rs.find("padding"); p != rs.end() && p->second.IsDouble()) {
        settings.padding = p->second.AsDouble();
    }

    if (auto lw = rs.find("line_width"); lw != rs.end() && lw->second.IsDouble()) {
        settings.line_width = lw->second.AsDouble();
    }

    if (auto sr = rs.find("stop_radius"); sr != rs.end() && sr->second.IsDouble()) {
        settings.stop_radius = sr->second.AsDouble();
    }

    if (auto blfs = rs.find("bus_label_font_size"); blfs != rs.end() && blfs->second.IsInt()) {
        settings.bus_label_font_size = blfs->second.AsInt();
    }

    if (auto blo = rs.find("bus_label_offset"); blo != rs.end() && blo->second.IsArray()) {
        settings.bus_label_offset.clear();
        for (const auto& v : blo->second.AsArray()) {
            settings.bus_label_offset.push_back(v.AsDouble());
        }
    }

    if (auto slfs = rs.find("stop_label_font_size"); slfs != rs.end() && slfs->second.IsInt()) { 
        settings.stop_label_font_size = slfs->second.AsInt();
    }

    if (auto slo = rs.find("stop_label_offset"); slo != rs.end() && slo->second.IsArray()) {
        settings.stop_label_offset.clear();
        for (const auto& v : slo->second.AsArray()) {
            settings.stop_label_offset.push_back(v.AsDouble());
        }
    }

    if (auto uc = rs.find("underlayer_color"); uc != rs.end()) {
        if (uc->second.IsString()) {
            settings.underlayer_color = uc->second.AsString();
        } else if (uc->second.IsArray()) {
            settings.underlayer_color = ParseColorFromArray(uc->second.AsArray());
        }
    }

    if (auto uw = rs.find("underlayer_width"); uw != rs.end() && uw->second.IsDouble()) {
        settings.underlayer_width = uw->second.AsDouble();
    }
    
    if (auto cp = rs.find("color_palette"); cp != rs.end() && cp->second.IsArray()) {
        settings.color_palette.clear();
        for (const auto& cn : cp->second.AsArray()) {
            if (cn.IsString()) {
                settings.color_palette.push_back(cn.AsString());
            } else if (cn.IsArray()) {
                settings.color_palette.push_back(ParseColorFromArray(cn.AsArray()));
            }
        }
    }

    return settings;
}

}  // namespace json_reader