#include "PSQLConn.h"

char *SQLL = "select ST_Area(ST_GeographyFromText('POLYGON (( 20 80 , 40 23, 20 0, 20 80 ))'));";
//char *SQLL = "select ST_Intersects(geog, ST_GeographyFromText('POLYGON (( 20 80 , 40 23, 20 0, 20 80 ))')) from DummyGeo;";

//prepared statement names
const char* ASPINSERT = "aspInsert"; 
const char* ASPGET = "aspGet";

PSQLConn::PSQLConn()
{
}


PSQLConn::~PSQLConn()
{
	disconnect();
}

std::vector<AirspaceDef>* PSQLConn::getCurrentAirspaces(double aglalt, double mslalt, double lat, double lon)
{
	PGresult* res;
	char ** params = new char*[3];
	std::vector<AirspaceDef>* results = new std::vector<AirspaceDef>();

	std::string val = std::to_string(aglalt);
	// ugly cstring hack
	params[0] = new char[val.length() + 1];
	strcpy_s(params[0], val.length() + 1, val.c_str());

	val = std::to_string(mslalt);
	params[1] = new char[val.length() + 1];
	strcpy_s(params[1], val.length() + 1, val.c_str());

	params[2] = new char[35];
	sprintf_s(params[2], 35, "POINT ( %10.6f %9.6f )", lon, lat);
	printf("%s\n", params[2]);
	res = PQexecPrepared(dbconn, ASPGET, 3, params, NULL, NULL, 0);

	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		printf("ERROR, Fetch All Failed: %s\n", PQerrorMessage(dbconn));
		PQclear(res);
	}
	else {
		AirspaceDef def;

		for (int i = 0; i < PQntuples(res); ++i)
		{
			// for now just get name and type
			strcpy_s(def.name, PQgetvalue(res, i, 0));
			def.type = static_cast<AirspaceType>(atoi(PQgetvalue(res, i, 1)));
			results->push_back(def);
		}
		PQclear(res);
	}

	delete[] params[0];
	delete[] params[1];
	delete[] params[2];
	delete[] params;
	return results;
}

	

void PSQLConn::insertAirspace(AirspaceDef* asp)
{
	
	PGresult *res;
	if (PQstatus(dbconn) != CONNECTION_OK)
	{
		printf("Connection not available: %s\n", PQerrorMessage(dbconn));
		return; 
	}
	
	totalcnt++;
	char ** vals = new char*[7];
	vals[0] = asp->name;

	std::string val = std::to_string(asp->type);
	// ugly cstring hack
	vals[1] = new char[val.length() + 1];
	strcpy_s(vals[1], val.length() + 1, val.c_str());

	val = std::to_string(asp->minAGLalt);
	vals[2] = new char[val.length() + 1];
	strcpy_s(vals[2], val.length() + 1, val.c_str());

	val = std::to_string(asp->maxAGLalt);
	vals[3] = new char[val.length() + 1];
	strcpy_s(vals[3], val.length() + 1, val.c_str());

	val = std::to_string(asp->minMSLalt);
	vals[4] = new char[val.length() + 1];
	strcpy_s(vals[4], val.length() + 1, val.c_str());

	val = std::to_string(asp->maxMSLalt);
	vals[5] = new char[val.length() + 1];
	strcpy_s(vals[5], val.length() + 1, val.c_str());

	vals[6] = asp->getWKTRepresentation();
	res = PQexecPrepared(dbconn, ASPINSERT, 7, vals, NULL, NULL, 0);


	if (PQresultStatus(res) == PGRES_COMMAND_OK)
	{
		//printf("inserted airspace successfully");
		successfulcnt++;
	}
	else {
		printf("ERROR, Insert failed failed: %s\n", PQerrorMessage(dbconn));
		/*for (int i = 0; i < 7; ++i)
		{
			printf("Param %s\n", vals[i]);
		}*/
	}
	// clean up
	delete[] vals[1];
	delete[] vals[2];
	delete[] vals[3];
	delete[] vals[4];
	delete[] vals[5];
	delete[] vals[6];
	delete[] vals;
	PQclear(res);
}

bool PSQLConn::connect()
{
	char buff[200];
	PGresult *res;
	int nFields, i, j;

	printf("Attempting to Connect to Database Server:\n");
	printf("Database: %s\n", db);
	printf("Server  : %s\n", dbserver);

	sprintf(buff, "dbname=%s host=%s port=5432 user=%s password=%s",
		db, dbserver, uname, pass);

	dbconn = PQconnectdb(buff);

	if (PQstatus(dbconn) != CONNECTION_OK)
	{
		printf("Connection Failed: %s\n", PQerrorMessage(dbconn));
		return false;
	}
	else
	{
		printf("Connected Successfully!\n");

		sprintf(buff, "BEGIN; DECLARE my_portal CURSOR FOR %s", SQLL);

		res = PQexec(dbconn, buff);
		if (PQresultStatus(res) != PGRES_COMMAND_OK)
		{
			printf("Error executing SQL!: %s\n", PQerrorMessage(dbconn));
			PQclear(res);
			return false;
		}
		else
		{
			PQclear(res);
			res = PQexec(dbconn, "FETCH ALL in my_portal");

			if (PQresultStatus(res) != PGRES_TUPLES_OK)
			{
				printf("ERROR, Fetch All Failed: %s", PQerrorMessage(dbconn));
				PQclear(res);
				return false;
			}
			else
			{
				nFields = PQnfields(res);

				// Print out the field names
				for (i = 0; i<nFields; i++)
					printf("%-15s", PQfname(res, i));

				printf("\n");

				// Print out the rows
				for (i = 0; i<PQntuples(res); i++)
				{
					for (j = 0; j<nFields; j++)
						printf("%-15s", PQgetvalue(res, i, j));

					printf("\n");
				}

				res = PQexec(dbconn, "END");
				PQclear(res);
			}
		}
		res = PQprepare(dbconn, ASPINSERT, "INSERT INTO Airspace (name, astype, minAGLalt, maxAGLalt, minMSLalt, maxMSLalt,extent) Values ($1, $2, $3, $4, $5, $6, ST_GeographyFromText($7));", 0, NULL);
		if (PQresultStatus(res) != PGRES_COMMAND_OK)
		{
			printf("ERROR, PrepareStatement failed: %s", PQerrorMessage(dbconn));
			PQclear(res);
			return false;
		}
		else { PQclear(res); }


		res = PQprepare(dbconn, ASPGET, "select name, astype, minAGLalt, maxAGLalt, minMSLalt, maxMSLalt from Airspace where ((minAGLalt != 'nan' AND minAGLalt <= $1) OR (minMSLalt != 'nan' AND minMSLalt <= $2)) AND ((maxAGLalt != 'nan' AND maxAGLalt >= $1) OR (maxMSLalt != 'nan' AND maxMSLalt >= $2)) AND ST_INTERSECTS(extent, ST_GEOGRAPHYFROMTEXT($3))", 0, NULL);
		if (PQresultStatus(res) != PGRES_COMMAND_OK)
		{
			printf("ERROR, PrepareStatement failed: %s", PQerrorMessage(dbconn));
			PQclear(res);
			return false;
		}
		else { PQclear(res); }


	}
	return true;


}

void PSQLConn::disconnect() {
	PQfinish(dbconn);
}