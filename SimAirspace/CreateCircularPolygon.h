#pragma once

#include "GeographicLib/Geodesic.hpp"
#include <sstream>

using namespace GeographicLib;

// Header only library that creates WKT polygons for circles and arcs

// Lat and Lon always in degrees and radius in meters

// number off degrees between two polygons

const int DEGOFF = 10;
const Geodesic& geod = Geodesic::WGS84();

std::pair<double, double>* getEarthPoint(double originLat, double originLon, double radius, double azi)
{
	double&& tgtLat = 0, &&tgtLon = 0;
	geod.Direct(originLat, originLon, azi, radius, tgtLat, tgtLon);
	return new std::pair<double, double>(tgtLat, tgtLon);
}

void makeCircle(double originLat, double originLon, double radius, std::ostringstream& output)
{
	std::pair<double, double>* first = NULL;
	for (double i = -180; i < 180; i += DEGOFF)
	{
		std::pair<double, double>* point = getEarthPoint(originLat, originLon, radius, i);

		output << point->second << " " << point->first << ", ";
		if (first != NULL)
		{
			delete point;
		}
		else { // first time don't delete yet, we need it as last as well.
			first = point;
		}
	}
	output << first->second << " " << first->first;
	delete first;
}

void makeArc(double originLat, double originLon, double sourceLat, double sourceLon, double tgtLat, double tgtLon, bool clockwise, std::ostringstream& output)
{
	double &&dist = 0, &&azi1 = 0, &&azi2 = 0;
	geod.Inverse(originLat, originLon, sourceLat, sourceLon, dist, azi1, azi2);
	double radius = dist;
	double startAngle = azi1;
	geod.Inverse(originLat, originLon, tgtLat, tgtLon, dist, azi1, azi2);
	if (fabs(dist - radius) / radius > 0.05) // Specify tolerance 
	{
		printf("Caution: Arc is not symmetric dist %f %f\n", dist, radius);
	}
	double endAngle = azi1;

	if (clockwise && endAngle < startAngle)
	{
		endAngle += 360;
	}
	else if (!clockwise && endAngle > startAngle)
	{
		startAngle += 360;
	}

	if (clockwise)
	{// we start slightly offset since the first point is already in the polygon
		for (double ang = startAngle+DEGOFF/2.0; ang < endAngle; ang += DEGOFF)
		{
			std::pair<double, double>* point = getEarthPoint(originLat, originLon, dist, ang > 180 ? ang - 360 : ang);
			output << point->second << " " << point->first << ", ";
			delete point;
		}
	}
	else {
		for (double ang = startAngle - DEGOFF / 2.0; ang > endAngle; ang -= DEGOFF)
		{
			std::pair<double, double>* point = getEarthPoint(originLat, originLon, dist, ang > 180 ? ang - 360 : ang);
			output << point->second << " " << point->first << ", ";
			delete point;
		}
	}
	// the final point in the ARC elem
	output << tgtLon << " " << tgtLat;
}

