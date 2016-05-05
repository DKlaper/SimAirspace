#pragma once
#include <iterator>
#include <windows.h>
#include "SimConnect.h"
#include "PSQLConn.h"
#include "SimTextManager.h"

PSQLConn connSC;
SimTextManager simText;
std::set<AirspaceDef, airspaceCompare>* currentAirspaces = new std::set<AirspaceDef, airspaceCompare>();

struct PlaneData {
	SIMCONNECT_DATA_LATLONALT data;
	double agl;
};

enum SimConnectIDs {
	DATADEF, REQUEST, STARTUP
};

int quit = 0;

void setup(HANDLE hSimConnect)
{
	connSC.connect();
	simText.setHandle(hSimConnect);
	SimConnect_AddToDataDefinition(hSimConnect, DATADEF, "STRUCT LATLONALT", NULL, SIMCONNECT_DATATYPE_LATLONALT);
	SimConnect_AddToDataDefinition(hSimConnect, DATADEF, "PLANE ALT ABOVE GROUND", "meter");
	SimConnect_SubscribeToSystemEvent(hSimConnect, STARTUP, "SimStart");
	// after 2 seconds return changed data every second
	SimConnect_RequestDataOnSimObject(hSimConnect, REQUEST, DATADEF, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SECOND, SIMCONNECT_DATA_REQUEST_FLAG_CHANGED, 2);
}

void dealWithAirspaces(std::set<AirspaceDef, airspaceCompare>* airspaces)
{
	// updates the list of airspaces and queues the text
	std::vector<AirspaceDef>* enteredAsp = new std::vector<AirspaceDef>();
	std::set_difference(airspaces->begin(), airspaces->end(), currentAirspaces->begin(), currentAirspaces->end(), std::back_inserter<std::vector<AirspaceDef>>(*enteredAsp), airspaceCompare());

	std::vector<AirspaceDef>* leftAsp = new std::vector<AirspaceDef>();
	std::set_difference(currentAirspaces->begin(), currentAirspaces->end(), airspaces->begin(), airspaces->end(), std::back_inserter<std::vector<AirspaceDef>>(*leftAsp), airspaceCompare());
	// we're done comparing the old ones, let's make current be current.
	delete currentAirspaces;
	currentAirspaces = airspaces;
	
	// process entered and left Queue to generate appropriate message
 	simText.process(currentAirspaces, enteredAsp, leftAsp);

	delete enteredAsp;
	delete leftAsp;

}

void CALLBACK CheckData(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
	SIMCONNECT_RECV_SIMOBJECT_DATA* data = NULL;
	SIMCONNECT_RECV_EVENT* evt = NULL;
	switch (pData->dwID)
	{
	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
		data = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;
		if (data->dwRequestID == REQUEST) {
			PlaneData* res = (PlaneData*)&data->dwData;
			printf("pos %f %f alt %f m agl %f m\n", res->data.Latitude, res->data.Longitude, res->data.Altitude, res->agl);
			auto airspaces = connSC.getCurrentAirspaces(res->agl, res->data.Altitude, res->data.Latitude, res->data.Longitude);
			dealWithAirspaces(airspaces);
			for (AirspaceDef entry : *airspaces)
			{
				printf("You are currently in airspace %s of type %s\n", entry.name, airspaceTypes[entry.type].c_str());
			}
		}
		break;
	case SIMCONNECT_RECV_ID_QUIT:
	{
		printf("\n***** SIMCONNECT_RECV_ID_QUIT *****\n");
		quit = 1;
		break;
	}

	case SIMCONNECT_RECV_ID_EXCEPTION:
	{
		SIMCONNECT_RECV_EXCEPTION *except = (SIMCONNECT_RECV_EXCEPTION*)pData;
		printf("\n\n***** EXCEPTION=%d  SendID=%d  uOffset=%d  cbData=%d\n", except->dwException, except->dwSendID, except->dwIndex, cbData);
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT:
		evt = (SIMCONNECT_RECV_EVENT*)pData;
		if (evt->uEventID == TEXTREQUESTID)
		{
			simText.currentSimConnectTextResult = (SIMCONNECT_TEXT_RESULT)(evt->dwData);
		}
		else if (evt->uEventID == STARTUP)
		{
			currentAirspaces->clear();
			simText.reset();
		}
		else {
			printf("Unknown event id %d\n", evt->uEventID);
		}
		break;

	default:
		printf("Unknown event %d\n", pData->dwID);
		break;
	}
}