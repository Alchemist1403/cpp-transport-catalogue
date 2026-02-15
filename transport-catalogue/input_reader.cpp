#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>
#include <sstream>

namespace comands_io {

// Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)

geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

// Удаляет пробелы в начале и конце строки

std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

// Разбивает строку string на n строк, с помощью указанного символа-разделителя delim

std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

//Парсит маршрут.
//Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
//Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]

std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(Trim(line.substr(not_space, colon_pos - not_space))),
     
            std::string(line.substr(colon_pos + 1))};
}


void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const {

    for (const auto& command : commands_) {
        if (command.command == "Stop") {
            auto coord_end = command.description.find(',');
            if (coord_end == std::string_view::npos) continue;
            auto second_comma = command.description.find(',', coord_end + 1);
            std::string coord_str;
            if (second_comma == std::string_view::npos) {
                coord_str = std::string(command.description);
            } else {
                coord_str = std::string(command.description.substr(0, second_comma));
            }
            auto coordinates = ParseCoordinates(coord_str);
            catalogue.AddStop(command.id, coordinates);
        }
    }

    for (const auto& command : commands_) {
        if (command.command == "Bus") {
            std::vector<transport_catalogue::StopPtr> stops;
            auto stop_names = ParseRoute(command.description);
            for (auto stop_name : stop_names) {
                stop_name = Trim(stop_name);
                auto stop = catalogue.FindStop(stop_name);
                if (stop) {
                    stops.push_back(stop);
                }
            }
            if (!stops.empty()) {
                catalogue.AddBus(command.id, stops);
            }
        }
    }


    for (const auto& command : commands_) {
        if (command.command == "Stop") {

            auto parts = Split(command.description, ',');

            for (size_t i = 2; i < parts.size(); ++i) {
                std::string_view token = Trim(parts[i]);

                size_t m_pos = token.find('m');
                if (m_pos == std::string_view::npos) continue;

                size_t to_pos = token.find(" to ", m_pos);
                if (to_pos == std::string_view::npos) continue;

                std::string dist_str(token.substr(0, m_pos));
                std::string target_name(token.substr(to_pos + 4));
                target_name = std::string(Trim(target_name));

                int distance = std::stoi(dist_str);
                auto from_stop = catalogue.FindStop(command.id);
                auto to_stop = catalogue.FindStop(target_name);
                if (from_stop && to_stop) {
                    catalogue.SetDistance(from_stop, to_stop, distance);
                }

            }
        }
    }
}

}





