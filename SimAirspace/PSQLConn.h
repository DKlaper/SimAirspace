#pragma once
#include "libpq-fe.h"
#include "AirspaceDef.h"
#include <string>
#include <vector>

class PSQLConn
{
public:
	int totalcnt;
	int successfulcnt;
	PSQLConn();
	~PSQLConn();
	std::set<AirspaceDef, airspaceCompare>* getCurrentAirspaces(double aglalt, double mslalt, double lat, double lon);
	void insertAirspace(AirspaceDef* asp);
	bool connect();
	
	char *db = "simairspacedb";
	char *dbserver = "localhost";
	char *uname = "David";
	char *pass = "";
private:
	PGconn   *dbconn;
	void disconnect();

};

