// SimAirspace.cpp : Defines the entry point for the console application.
//
#include <windows.h>
#include "SimConnect.h"
#include <stdio.h>
#include "Regions.h"
#include "BGLDecompiler.h"
#include "PSQLConn.h"
#include "SimConnectHelpers.h"

HANDLE  hSimConnect = NULL;


void main(void) {

	if( SUCCEEDED( SimConnect_Open(&hSimConnect, "SimAirspace", NULL, 0, 0, 0) ) )
	{
		setup(hSimConnect);
		while (quit == 0)
		{
			SimConnect_CallDispatch(hSimConnect, CheckData, NULL);
			Sleep(400);
		}

	}
	else {
		printf("Could not open simconnect");
	}
}

/*void testmain(void)
{

	PSQLConn conn;
	conn.connect();
	auto airspaces = conn.getCurrentAirspaces(2300.0, 2400.0, 47.035644, 8.859046);
	for (AirspaceDef entry : *airspaces)
	{
		printf("You are currently in airspace %s of type %d\n", entry.name, entry.type);
	}
	delete airspaces;
	//parseFile(R"(C:\FSX\Scenery\World\Scenery\BNXWorld1.bgl)");
	//parseFile(R"(C:\FSX\Scenery\World\Scenery\BNXWorld0.bgl)");
	//parseFile(R"(C:\FSX\Scenery\World\Scenery\bvcf.bgl)");

	RegionIdPair* resa = getRegionIds(90, 12);
	printf("%d %d\n", resa->first, resa->second);
	delete resa;

	resa = getRegionIds(12, 0);
	printf("%d %d\n", resa->first, resa->second);
	delete resa;

	resa = getRegionIds(-69.8, -47.4); 
	printf("%d %d\n", resa->first, resa->second);
	delete resa;

	resa = getRegionIds(0, 0);
	printf("%d %d\n", resa->first, resa->second);
	delete resa;

	resa = getRegionIds(-2.5, -180);
	printf("%d %d\n", resa->first, resa->second);
	delete resa;

	resa = getRegionIds(2.6, 42.6);
	printf("%d %d\n", resa->first, resa->second);
	delete resa;

	resa = getRegionIds(2.4, -175);
	printf("%d %d\n", resa->first, resa->second);
	delete resa;

	resa = getRegionIds(-21, 177); //!!
	printf("%d %d\n", resa->first, resa->second);
	delete resa;

 	return;
}*/