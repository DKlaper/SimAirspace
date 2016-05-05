#pragma once
#include <vector>
#include <windows.h>
#include <set>
#include <list>
#include "AirspaceDef.h"
#include "SimConnect.h"

const int TEXTREQUESTID = 9989;

enum MessageType : short {
	ENTERED, LEFT, INSIDE
};

struct Message {
	char name[150];
	AirspaceType type;
	SIMCONNECT_TEXT_TYPE textType;
	float timeoutSecs;
	MessageType msgType;
};

class SimTextManager
{
public:
	SimTextManager();
	~SimTextManager();
	void setHandle(HANDLE hsim);
	void reset();
	void process(std::set<AirspaceDef, airspaceCompare>* currentAirspaces, std::vector<AirspaceDef>* enteredAsp, std::vector<AirspaceDef>* leftAsp);
	SIMCONNECT_TEXT_RESULT currentSimConnectTextResult = SIMCONNECT_TEXT_RESULT_QUEUED;
private:
	void displayNext();
	void redisplayCurrent();
	void updateView();
	HANDLE hSimConnect;
};
