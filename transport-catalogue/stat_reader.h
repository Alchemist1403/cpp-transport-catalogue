#pragma once

#include "transport_catalogue.h"

#include <iosfwd>
#include <string_view>

void PrintBusStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view bus_name, std::ostream& output);
void PrintStopStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view stop_name, std::ostream& output);
void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output);
