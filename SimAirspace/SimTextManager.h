#pragma once
#include <vector>
#include <set>
#include "AirspaceDef.h"

class SimTextManager
{
public:
	SimTextManager();
	~SimTextManager();

	void process(std::set<AirspaceDef, airspaceCompare>* currentAirspaces, std::vector<AirspaceDef>* enteredAsp, std::vector<AirspaceDef>* leftAsp);
};

