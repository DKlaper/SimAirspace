#pragma once
#include <windows.h>
#include "SimConnect.h"
#include "PSQLConn.h"

PSQLConn connSC;

struct PlaneData {
	SIMCONNECT_DATA_LATLONALT data;
	double agl;
};

enum SimConnectIDs {
	DATADEF, REQUEST, REQUESTTEXT
};

int quit = 0;

void setup(HANDLE hSimConnect)
{
	connSC.connect();
	SimConnect_AddToDataDefinition(hSimConnect, DATADEF, "STRUCT LATLONALT", NULL, SIMCONNECT_DATATYPE_LATLONALT);
	SimConnect_AddToDataDefinition(hSimConnect, DATADEF, "PLANE ALT ABOVE GROUND", "meter");
	// after 2 seconds return changed data every second
	SimConnect_RequestDataOnSimObject(hSimConnect, REQUEST, DATADEF, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SECOND, SIMCONNECT_DATA_REQUEST_FLAG_CHANGED, 2);
}

void dealWithAirspaces(std::vector<AirspaceDef>* airspaces)
{
	// updates the list of airspaces and queues the text
	for (AirspaceDef entry : *airspaces)
	{
		// if contains -> remove from list
		// else: add to current txt queue
	}
	// create new list from current
	// look at old list -> add to txt queue left messages

	// process Text Queue to generate appropriate message
}

void CALLBACK CheckData(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
	SIMCONNECT_RECV_SIMOBJECT_DATA* data = NULL;

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
			delete airspaces;
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

	// TODO deal with simconnect text result in particular manage which ones were displayed

	default:
		printf("Unknown event %d\n", pData->dwID);
		break;
	}
}