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

