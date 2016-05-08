// SimAirspace.cpp : Defines the entry point for the console application.
//
#include <windows.h>
#include <shellapi.h>
#include <strsafe.h>
#include <WinInet.h>
#include "SimConnect.h"
#include <stdio.h>
#include "Regions.h"
#include "BGLDecompiler.h"
#include "PSQLConn.h"
#include "SimConnectHelpers.h"

#pragma comment(lib, "urlmon.lib")

const LPWSTR combinedShapeFileURL = LR"(http://sggate.arc.nasa.gov:9518/airspace/shape/tfr.shp.zip)";
const LPWSTR tfrZipFilename = LR"(tfr.shp.zip)";
bool tfrUpdateEnabled = true; // set to false to not ever delete your tfr file
HANDLE  hSimConnect = NULL;

void updateTFRDB(LPCWSTR pgFolder)
{
	// create sql script
	PSQLConn conn;
	conn.connect(); // destructor deconnects;
	LPWSTR pg_shp = new wchar_t[4096];
	StringCchCopy(pg_shp, 4096, L"/c ");
	StringCchCat(pg_shp+3, 4093, pgFolder);
	StringCchCat(pg_shp + 3 + lstrlen(pgFolder), 4093 - lstrlen(pgFolder), L"shp2pgsql.exe -d -G -I -S tfr\\tfr.shp > newTFR.sql");
	int res = (int)ShellExecute(NULL, L"open", L"cmd.exe", pg_shp, NULL, SW_HIDE);

	// execute the generated script
	LPWSTR psql = new wchar_t[4096];
	StringCchCopy(psql, 4096, pgFolder);
	StringCchCat(psql + lstrlen(pgFolder), 4096 - lstrlen(pgFolder), L"psql.exe");
	LPWSTR dbc = new wchar_t[1024];
	swprintf_s(dbc, 1024, L"-f newTFR.sql -d %S -p %d -U %S -w", conn.db, conn.port, conn.uname);
	ShellExecute(NULL, L"open", psql, dbc, NULL, SW_HIDE);
}

void readTfrData(LPCWSTR pgFolder)
{
	bool reload = true;
	// first get file modified date
	WIN32_FILE_ATTRIBUTE_DATA fileattrib;
	if (GetFileAttributesEx(tfrZipFilename, GetFileExInfoStandard, &fileattrib) != 0) //success
	{
		ULARGE_INTEGER writeTime;
		writeTime.HighPart = fileattrib.ftLastWriteTime.dwHighDateTime;
		writeTime.LowPart = fileattrib.ftLastWriteTime.dwLowDateTime;

		FILETIME now;
		ULARGE_INTEGER nowTime;
		GetSystemTimeAsFileTime(&now);
		nowTime.HighPart = now.dwHighDateTime;
		nowTime.LowPart = now.dwLowDateTime;

		// 100 ns * 10'000'000 = 1 second
		unsigned long long diff = (nowTime.QuadPart - writeTime.QuadPart)/10000000ULL;
		if (diff > 60 * 60 * 12) // if older than  12 hours
		{
			reload = true;
		}
		else {
			reload = false;
		}
	}

	// part 2 load and unzip if necessary
	if (reload && tfrUpdateEnabled)
	{
		DeleteFile(tfrZipFilename);
		printf("Downloading new TFR data\n");
		HRESULT res = URLDownloadToFile(NULL, combinedShapeFileURL, tfrZipFilename, 0, NULL);
		if (res == S_OK)
		{
			printf("Downloading succeeded\n");
			ShellExecute(NULL, L"open", L"unzip.exe", L"-o -d tfr tfr.shp.zip", NULL, SW_HIDE);
			// part 3 use shp2pgsql insert
			updateTFRDB(pgFolder);
		}
		else {
			printf("downloading failed\n");
		}
	}
}

void startup(LPCWSTR pgFolder)
{
	LPWSTR pg_ctl = new wchar_t[4096];
	StringCchCopy(pg_ctl, 4096, pgFolder);
	StringCchCat(pg_ctl+lstrlen(pgFolder), 4096-lstrlen(pgFolder), L"pg_ctl.exe");
	LPWSTR param = LR"(start -D db )";
	// start the database  (show no window)
	int res = (int)ShellExecute(NULL, L"open", pg_ctl, param, NULL, SW_HIDE);
	if (res < 32)
	{
		printf("Could not start db");
		exit(-99);
	}
	// then read TFRs from Nasa tool
	readTfrData(pgFolder);
}

void shutdown(LPCWSTR pgFolder)
{
	LPWSTR pg_ctl = new wchar_t[4096];
	StringCchCopy(pg_ctl, 4096, pgFolder);
	StringCchCat(pg_ctl + lstrlen(pgFolder), 4096 - lstrlen(pgFolder), L"pg_ctl.exe");
	LPWSTR param = LR"(stop -D tmp\ -w)";
	// start the database  (show no window)
	int res = (int)ShellExecute(NULL, L"open", pg_ctl, param, NULL, SW_HIDE);
	if (res < 32)
	{
		printf("db shutdown failed");
		exit(-99);
	}
}

int main(int argc, char* argv[]) {
	loadConfig(&AirspaceDef::conf);
	if (argc == 2 && strcmp(argv[1], "initdb") == 0)
	{
		// write the config
		saveConfig(AirspaceDef::conf);

		// setup DB 
	}
	// init step 2 read airspaces
	if (argc == 2 && strcmp(argv[1], "initasp") == 0)
	{

		// todo clear airspace table first.
		printf("Parsing Bgls");
		char* filepath = new char[1024];
		wcstombs(filepath, AirspaceDef::conf.fsxFolder, 1024);
		int baselen = strlen(filepath);
		strcpy_s(filepath + baselen, 1024 - baselen, R"(\Scenery\World\Scenery\BNXWorld0.bgl)");
		parseFile(filepath);
		strcpy_s(filepath + baselen, 1024 - baselen, R"(\Scenery\World\Scenery\BNXWorld1.bgl)");
		parseFile(filepath);
		strcpy_s(filepath + baselen, 1024 - baselen, R"(\Scenery\World\Scenery\bvcf.bgl)");
		parseFile(filepath);
		delete filepath;
		printf("Airspaces Inserted");
		return 0;
	}

	if( SUCCEEDED( SimConnect_Open(&hSimConnect, "SimAirspace", NULL, 0, 0, 0) ) )
	{
		startup(AirspaceDef::conf.postgresFolder);
		setup(hSimConnect);
		while (quit == 0)
		{
			SimConnect_CallDispatch(hSimConnect, CheckData, NULL);
			Sleep(400);
		}
		shutdown(AirspaceDef::conf.postgresFolder);
	}
	else {
		printf("Could not open simconnect");
	}

	return 0;
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