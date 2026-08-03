#include "CoordinateConverter.h"
#include <cmath>

static constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;
static constexpr double EARTH_RADIUS = 6378137.0;

CoordinateConverter::CoordinateConverter(double lat, double lon)
{
    originLat = lat;
    originLon = lon;
}

Point2D CoordinateConverter::toLocal(double lat, double lon) const
{
    double dLat = (lat - originLat) * DEG_TO_RAD;
    double dLon = (lon - originLon) * DEG_TO_RAD;

    double x = dLon * EARTH_RADIUS * std::cos(originLat * DEG_TO_RAD);
    double y = dLat * EARTH_RADIUS;

    return {x, y};
}