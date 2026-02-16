#include "stat_reader.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>

using namespace std;

void PrintBusStat(const transport_catalogue::TransportCatalogue& transport_catalogue, string_view bus_name, ostream& output) {
    auto bus = transport_catalogue.FindBus(bus_name);
    if (!bus) {
        output << "Bus " << bus_name << ": not found" << endl;
        return;
    }

    auto stat = transport_catalogue.GetStat(bus);
    output << "Bus " << bus_name << ": " 
        << stat.total_stops << " stops on route, "
        << stat.unique_stops << " unique stops, "
        << fixed << setprecision(6) << stat.route_length << " route length, " 
        << stat.route_length / stat.geographic_distance << " curvature" 
        << endl;
}

void PrintStopStat(const transport_catalogue::TransportCatalogue& transport_catalogue, string_view stop_name, ostream& output) {

    auto stop = transport_catalogue.FindStop(stop_name);
    if (!stop) {
        output << "Stop " << stop_name << ": not found" << endl;
        return;
    }

    set<string> buses;
    for (auto bus : transport_catalogue.GetBusesByStop(stop)) {
        buses.insert(bus->name);
    }

    if (buses.empty()) {
        output << "Stop " << stop_name <<": no buses" << endl;
    } else {
        output << "Stop " << stop_name << ": buses ";

        for (const auto& bus_name : buses) {
            output << bus_name << " ";
        }

        output << endl;
    }

}

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, string_view request, ostream& output) {

    if (request.substr(0, 4) == "Bus "){

        PrintBusStat(transport_catalogue, request.substr(4), output);

    } else if (request.substr(0, 5) == "Stop ") {

        PrintStopStat(transport_catalogue, request.substr(5), output);

    }
}
