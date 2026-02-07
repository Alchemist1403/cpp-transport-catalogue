#pragma once

#include "transport_catalogue.h"

#include <iosfwd>
#include <string_view>

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output);
