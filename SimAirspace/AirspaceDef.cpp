#include "AirspaceDef.h"
#include "CreateCircularPolygon.h"


AirspaceDef::AirspaceDef()
{
	points = NULL;
	strcpy_s(name, "UNNAMED\0");
}

AirspaceDef::~AirspaceDef()
{
	if (points != NULL)
	{
		delete points;
	}
}

Configuration AirspaceDef::conf;

char* AirspaceDef::getWKTRepresentation()
{
	std::ostringstream outstr;
	double curOriginLat = 0, curOriginLon = 0, sourceLat = 0, sourceLon = 0;
	float radius = 0;
	BYTE idx;
	outstr.write("POLYGON (( ", 10);
	for (int i = 0; i < pntcnt; ++i)
	{
		BoundaryLoc pnt = points[i];
		switch (pnt.type)
		{
		case START:
		case LINE:
			sourceLon = getLongitude(pnt.longitude);
			sourceLat = getLatitude(pnt.latitude); // each line/start must be remembered for arc
			outstr << sourceLon << " " << sourceLat;
			if (i != pntcnt - 1)
			{
				outstr << ", ";
			}
			break;

		case ORIGIN:
			curOriginLat = getLatitude(pnt.latitude);
			curOriginLon = getLongitude(pnt.longitude);
			idx = pnt.idx;
			break;
		case CIRCLE:
			radius = *reinterpret_cast<float*>(&pnt.longitude);
			if (pnt.idx != idx)
			{
				printf("Warning, index does not match\n");
			}
			makeCircle(curOriginLat, curOriginLon, radius, outstr);
			break;
		case ARCW: 
		case ARCCW:
			if (pnt.idx != idx)
			{
				printf("Warning, index does not match\n");
			}
			makeArc(curOriginLat, curOriginLon, sourceLat, sourceLon, getLatitude(pnt.latitude),getLongitude(pnt.longitude), pnt.type==ARCW, outstr);
			if (i != pntcnt - 1)
			{
				outstr << ", ";
			}
			sourceLon = getLongitude(pnt.longitude);
			sourceLat = getLatitude(pnt.latitude); // Arc itself could also be start of new arc!!!!!
			break;

		}
	}
	outstr << " ))";
	std::string val = outstr.str();
	char* res = new char[val.length() + 1];
	strcpy_s(res, val.length() + 1, val.c_str());
	return res;
}

void AirspaceDef::setAltitudes(BYTE altType, double minAlt, double maxAlt)
{
	BYTE maxtp = (altType & 0b11110000) >> 4;
	BYTE mintp = altType & 0b00001111;
	//printf("minAltType %d and maxAltType %d\n", mintp, maxtp);
	if (mintp == 1)
	{
		minAGLalt = minAlt;
		minMSLalt = std::numeric_limits<double>::quiet_NaN();
	}
	else if (mintp == 2)
	{
		minMSLalt = minAlt;
		minAGLalt = std::numeric_limits<double>::quiet_NaN();
	}
	else { // unknown or unlimited
		minMSLalt = -1 * INFINITY;
		minAGLalt = -1 * INFINITY;
	}

	if (maxtp == 1)
	{
		maxAGLalt = maxAlt;
		maxMSLalt = std::numeric_limits<double>::quiet_NaN();
	}
	else if (maxtp == 2)
	{
		maxMSLalt = maxAlt;
		maxAGLalt = std::numeric_limits<double>::quiet_NaN();
	}
	else { // unknown or unlimited
		maxAGLalt = INFINITY;
		maxMSLalt = INFINITY;
	}

	//printf("%x type AGL %f %f MSL %f %f\n", altType, minAGLalt, maxAGLalt, minMSLalt, maxMSLalt);
}