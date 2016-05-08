#pragma once
#include "BGLDecompiler.h"
#include <array>
#include <sstream>
#include "Configuration.h"

class AirspaceDef
{
public:
	static Configuration conf;
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
			// note the bigger than sign because we want the most important to be first
			// also note that no 2 airspaceImportance values may be equal otherwise it will be deleted from the set
			return airspaceImportance[a.type] > airspaceImportance[b.type];
		}
		else {
			return namecmp < 0;
		}

	}
};

