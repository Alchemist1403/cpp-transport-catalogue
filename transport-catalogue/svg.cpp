#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    RenderObject(context);
    context.out << std::endl;
}

std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit([&out](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Rgb>) {
            out << "rgb(" << int(arg.red) << "," << int(arg.green) << "," << int(arg.blue) << ")";
        } else if constexpr (std::is_same_v<T, Rgba>) {
            out << "rgba(" << int(arg.red) << "," << int(arg.green) << "," << int(arg.blue) << "," << arg.opacity << ")";
        } else {
            out << arg;
        }
    }, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap value) {
    std::string_view sv;
    switch (value) {
        case StrokeLineCap::BUTT:
            sv = "butt";
            break;
        case StrokeLineCap::ROUND:
            sv = "round";
            break;
        case StrokeLineCap::SQUARE:
            sv = "square";
            break;
    }
    return out << sv;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin value) {
    std::string_view sv;
    switch (value) {
        case StrokeLineJoin::ARCS:
            sv = "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            sv = "bevel";
            break;
        case StrokeLineJoin::MITER:
            sv = "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            sv = "miter-clip";
            break;
        case StrokeLineJoin::ROUND:
            sv = "round";
            break;
    }
    return out << sv;
}


// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;

    RenderAttrs(context.out);

    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    bool first_point = true;
    out << "<polyline points=\"";
    for (const auto& point: points_) {

        if (first_point) {
            out << point.x << "," << point.y;
            first_point = false;
            continue;
        }

        out << " " << point.x << "," << point.y;
    }

    out << "\"";

    RenderAttrs(context.out);
    out << "/>"sv;
}


// ---------- Text ------------------

Text& Text::SetPosition(Point pos){
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this; 
}

Text& Text::SetFontSize(uint32_t size){
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family){
    font_family_ = font_family;
    return *this;
}


Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = font_weight;
    return *this;
}


Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void HtmlEncodeString(std::ostream& out, std::string_view sv) {
    for (char c : sv) {
        switch (c) {
            case '"':
                out << "&quot;"sv;
                break;
            case '<':
                out << "&lt;"sv;
                break;
            case '>':
                out << "&gt;"sv;
                break;
            case '&':
                out << "&amp;"sv;
                break;
            case '\'':
                out << "&apos;"sv;
                break;
            default:
                out.put(c);
        }
    }
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";

    RenderAttrs(context.out);

    out << " x=\"" << position_.x << "\"" << " y=\"" << position_.y << "\"";
    out << " dx=\"" << offset_.x << "\"" << " dy=\"" << offset_.y << "\"";
    out << " font-size=\"" << font_size_ << "\"";

    if (!font_family_.empty()) {
        out << " font-family=\"" << font_family_ << "\"";
    }

    if (!font_weight_.empty()) {
        out << " font-weight=\"" << font_weight_ << "\"";
    }


    out << ">";
    HtmlEncodeString(out, data_);
    out << "</text>"sv;
}



// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";

    for (const auto& obj : objects_) {
        obj->Render(RenderContext(out,2));
    }
    out << "</svg>\n";
}


}  // namespace svg