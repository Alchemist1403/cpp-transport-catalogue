#pragma once
#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"
#include <algorithm>

namespace render {

inline const double EPSILON = 1e-6;
inline bool IsZero(double v) { return std::abs(v) < EPSILON; }

struct RenderSettings {
    double width = 1200.0, height = 1200.0, padding = 50.0;
    double line_width = 14.0, stop_radius = 5.0;
    int bus_label_font_size = 20, stop_label_font_size = 20;
    std::vector<double> bus_label_offset = {7.0, 15.0};
    std::vector<double> stop_label_offset = {7.0, -3.0};
    svg::Color underlayer_color = "rgba(255,255,255,0.85)";
    double underlayer_width = 3.0;
    std::vector<svg::Color> color_palette = {
        "green", "orange", "red", "blue", "purple", "brown", "pink", "gray"
    };
};

class SphereProjector {
public:
    template <typename It>
    SphereProjector(It begin, It end, double w, double h, double pad) : padding_(pad) {
        if (begin == end) return;
        auto [l, r] = std::minmax_element(begin, end, [](auto a, auto b){ return a.lng < b.lng; });
        min_lon_ = l->lng; double max_lon = r->lng;
        auto [b, t] = std::minmax_element(begin, end, [](auto a, auto b){ return a.lat < b.lat; });
        double min_lat = b->lat; max_lat_ = t->lat;
        std::optional<double> wz, hz;
        if (!IsZero(max_lon - min_lon_)) wz = (w - 2*pad) / (max_lon - min_lon_);
        if (!IsZero(max_lat_ - min_lat)) hz = (h - 2*pad) / (max_lat_ - min_lat);
        if (wz && hz) zoom_ = std::min(*wz, *hz);
        else if (wz) zoom_ = *wz;
        else if (hz) zoom_ = *hz;
    }
    svg::Point operator()(geo::Coordinates c) const {
        return {(c.lng - min_lon_) * zoom_ + padding_, (max_lat_ - c.lat) * zoom_ + padding_};
    }
private:
    double padding_, min_lon_ = 0, max_lat_ = 0, zoom_ = 0;
};

class MapRenderer {
public:
    explicit MapRenderer(const RenderSettings& s) : settings_(s) {}
    void RenderMap(std::ostream& out, const std::vector<transport_catalogue::BusPtr>& buses, [[maybe_unused]] const std::vector<transport_catalogue::StopPtr>& stops) const;
private:
    void RenderBusRoute(svg::Document& doc, const transport_catalogue::Bus* bus, const SphereProjector& proj, size_t idx) const;
    void RenderBusLabel(svg::Document& doc, const transport_catalogue::Bus* bus, const transport_catalogue::Stop* stop, const SphereProjector& proj, size_t idx) const;
    void RenderStop(svg::Document& doc, const transport_catalogue::Stop* stop, const SphereProjector& proj) const;
    void RenderStopLabel(svg::Document& doc, const transport_catalogue::Stop* stop, const SphereProjector& proj) const;
    svg::Color GetColor(size_t idx) const;
    RenderSettings settings_;
};

}  // namespace render