#pragma once

#include <utility>
#include <math.h>

typedef std::pair<int, int> RegionIdPair;
RegionIdPair * getRegionIds(double lat, double lon);
