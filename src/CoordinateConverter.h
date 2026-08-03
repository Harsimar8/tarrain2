#pragma once

struct Point2D
{
    double x;
    double y;
};

class CoordinateConverter
{
public:
    CoordinateConverter(double originLat, double originLon);

    Point2D toLocal(double lat, double lon) const;

private:
    double originLat;
    double originLon;
};