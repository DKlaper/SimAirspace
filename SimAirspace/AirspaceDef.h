#pragma once
#include "BGLDecompiler.h"
#include <array>
#include <sstream>

class AirspaceDef
{
public:
	AirspaceDef();
	~AirspaceDef();

	char * getWKTRepresentation();

	void setAltitudes(BYTE altType, double minAlt, double maxAlt);
	
	char name[150];
	AirspaceType type;
	double minMSLalt;
	double maxMSLalt;
	double minAGLalt;
	double maxAGLalt;
	DWORD pntcnt;
	BoundaryLoc* points;

};

struct airspaceCompare
{
	bool operator() (const AirspaceDef& a, const AirspaceDef& b) const 
	{
		int namecmp = strcmp(a.name, b.name);
		bool sametype = a.type == b.type;
		if (sametype && namecmp == 0)
		{
			return false;
		}
		else if (!sametype)
		{
			return a.type < b.type;
		}
		else {
			return namecmp < 0;
		}

	}
};

