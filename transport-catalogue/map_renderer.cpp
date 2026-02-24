#include "map_renderer.h"
#include <iomanip>
using namespace std::literals;

namespace render {

svg::Color MapRenderer::GetColor(size_t idx) const {
    if (settings_.color_palette.empty()) {
        return "black";
    }
    return settings_.color_palette[idx % settings_.color_palette.size()];
}

void MapRenderer::RenderMap(std::ostream& out, const std::vector<transport_catalogue::BusPtr>& buses, [[maybe_unused]] const std::vector<transport_catalogue::StopPtr>& stops) const {
    std::vector<geo::Coordinates> coords;
    for (const auto& bus : buses)
        for (const auto& stop : bus->stops) {
            coords.push_back(stop->position);
        }
    
    SphereProjector proj(coords.begin(), coords.end(), settings_.width, settings_.height, settings_.padding);
    svg::Document doc;
    
    for (size_t i = 0; i < buses.size(); ++i) {
        RenderBusRoute(doc, buses[i], proj, i);
    }

    for (size_t i = 0; i < buses.size(); ++i) {
        const auto* bus = buses[i];
        if (bus->stops.empty()) {
            continue;
        }
        
        const auto* first_stop = bus->stops.front();
        const auto* last_stop = bus->stops.back();
        
        RenderBusLabel(doc, bus, first_stop, proj, i);
        if (last_stop != first_stop) {
            RenderBusLabel(doc, bus, last_stop, proj, i);
        }
    }
    
    std::unordered_set<const transport_catalogue::Stop*> unique_stops;
    for (const auto* bus : buses) {
        for (const auto* stop : bus->stops) {
            unique_stops.insert(stop);
        }
    }
    
    std::vector<const transport_catalogue::Stop*> sorted_stops(
        unique_stops.begin(), unique_stops.end());
    
    std::sort(sorted_stops.begin(), sorted_stops.end(),
        [](const transport_catalogue::Stop* a, const transport_catalogue::Stop* b) {
            return a->name < b->name;
        });
    
    for (const auto* stop : sorted_stops) {
        RenderStop(doc, stop, proj);
    }
    
    for (const auto* stop : sorted_stops) {
        RenderStopLabel(doc, stop, proj);
    }
    
    doc.Render(out);
}

void MapRenderer::RenderBusRoute(svg::Document& doc, const transport_catalogue::Bus* bus, const SphereProjector& proj, size_t idx) const {
    if (bus->stops.empty()) {
        return;
    }
    
    svg::Polyline pl;
    pl.SetStrokeColor(GetColor(idx))
      .SetFillColor(svg::NoneColor)
      .SetStrokeWidth(settings_.line_width)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    for (const auto& s : bus->stops) {
        pl.AddPoint(proj(s->position));
    }

    if (!bus->is_roundtrip && bus->stops.size() > 1) {
            for (auto it = bus->stops.rbegin() + 1; it != bus->stops.rend(); ++it) {
                pl.AddPoint(proj((*it)->position));
            }

    }

    doc.Add(std::move(pl));
}

void MapRenderer::RenderBusLabel(svg::Document& doc, const transport_catalogue::Bus* bus, const transport_catalogue::Stop* stop, const SphereProjector& proj, size_t idx) const {
    svg::Point pos = proj(stop->position);
    
    // Подложка
    svg::Text bg;
    bg.SetPosition(pos)
      .SetOffset({settings_.bus_label_offset[0], settings_.bus_label_offset[1]})
      .SetFontSize(settings_.bus_label_font_size)
      .SetFontFamily("Verdana")
      .SetFontWeight("bold")
      .SetData(bus->name)
      .SetFillColor(settings_.underlayer_color)
      .SetStrokeColor(settings_.underlayer_color)
      .SetStrokeWidth(settings_.underlayer_width)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    doc.Add(std::move(bg));
    
    // Текст
    svg::Text txt;
    txt.SetPosition(pos)
       .SetOffset({settings_.bus_label_offset[0], settings_.bus_label_offset[1]})
       .SetFontSize(settings_.bus_label_font_size)
       .SetFontFamily("Verdana")
       .SetFontWeight("bold")
       .SetData(bus->name)
       .SetFillColor(GetColor(idx));
    doc.Add(std::move(txt));
}

void MapRenderer::RenderStop(svg::Document& doc, const transport_catalogue::Stop* stop, const SphereProjector& proj) const {
    svg::Circle c;
    c.SetCenter(proj(stop->position))
     .SetRadius(settings_.stop_radius)
     .SetFillColor("white");
    doc.Add(std::move(c));
}

void MapRenderer::RenderStopLabel(svg::Document& doc, const transport_catalogue::Stop* stop, const SphereProjector& proj) const {
    svg::Point pos = proj(stop->position);
    
    // Подложка
    svg::Text bg;
    bg.SetPosition(pos)
      .SetOffset({settings_.stop_label_offset[0], settings_.stop_label_offset[1]})
      .SetFontSize(settings_.stop_label_font_size)
      .SetFontFamily("Verdana")
      .SetData(stop->name)
      .SetFillColor(settings_.underlayer_color)
      .SetStrokeColor(settings_.underlayer_color)
      .SetStrokeWidth(settings_.underlayer_width)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    doc.Add(std::move(bg));
    
    // Текст
    svg::Text txt;
    txt.SetPosition(pos)
       .SetOffset({settings_.stop_label_offset[0], settings_.stop_label_offset[1]})
       .SetFontSize(settings_.stop_label_font_size)
       .SetFontFamily("Verdana")
       .SetData(stop->name)
       .SetFillColor("black");
    doc.Add(std::move(txt));
}

}  // namespace render