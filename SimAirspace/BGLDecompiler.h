#pragma once

#include <fstream>
#include <intsafe.h>
#include <set>
#include <string>

struct Header {
	DWORD magicVersion;
	DWORD size;
	DWORD lowDate;
	DWORD highDate;
	DWORD magicTwo;
	DWORD sectionCount;
	DWORD QMIDs[8];
};
enum RecordType : DWORD {
	ARPT = 0x0003, VORILS = 0x0013, NDB = 0x0017, MARKERS = 0x0018, NAME = 0x0019, BOUNDARY = 0x0020, BOUNDS = 0x0021,
	GEOPOL = 0x0023, SCENERY = 0x0025, NAMELIST = 0x0027, MDL = 0x002b, ADDARPT = 0x002c, EXCLUSION = 0x002e};

struct Section {
	RecordType type;
	DWORD subsectionSize;
	DWORD subsectionCount;
	DWORD subsectionStart;
	DWORD subsectionLength;
};
struct SubSection {
	DWORD qmid;
	DWORD recordCount;
	DWORD idxOffset;
	DWORD size;
};

struct SubSectionBoundary {
	DWORD offset;
	DWORD size;
};

enum AirspaceType : BYTE {
	NONE, CENTER, CLASS_A, CLASS_B, CLASS_C, CLASS_D, CLASS_E, CLASS_F, CLASS_G,
	TOWER, CLEARANCE, GROUND, DEPARTURE, APPROACH, MOA, RESTRICTED, PROHIBITED,
	WARNING, ALERT, DANGER, NATIONAL_PARK, MODEC, RADAR, TRAINING, TFR
};

const std::string airspaceTypes[] = { "NONE", "CENTER", "CLASS_A", "CLASS_B", "CLASS_C", "CLASS_D", "CLASS_E", "CLASS_F", "CLASS_G",
"TOWER", "CLEARANCE", "GROUND", "DEPARTURE", "APPROACH", "MOA", "RESTRICTED", "PROHIBITED",
"WARNING", "ALERT", "DANGER", "NATIONAL_PARK", "MODEC", "RADAR", "TRAINING", "TFR" };

// this is used for the ordering thus no 2 may have the same importance otherwise the  set will delete some non equal values  ---> explains those weird values
const int airspaceImportance[] = { -99, -50, 10, 9, 8, 7, 6, -48, -46, -89, -90, -91, -92, -93,  15, 40, 50, 18, 19, 20, 14, 13, 5, 11, 30 };

#pragma pack(1)
struct Name {
	WORD id;
	DWORD size;
	char string[100];
};

enum BoundaryType : BYTE
{ START=1, LINE=2, ORIGIN=3, ARCW=4, ARCCW=5, CIRCLE=6};

struct BoundaryLoc {
	BoundaryType type;
	BYTE idx; // idx of origin/arc/circle
	DWORD latitude;
	DWORD longitude;
};

struct BoundaryDesc {
	WORD id;
	DWORD size;
	WORD pntcnt;
};

#pragma pack(1)
struct Airspace {
	WORD id;
	DWORD size;
	AirspaceType asType;
	BYTE altType;
	DWORD minLon;
	DWORD minLat;
	DWORD minAlt;
	DWORD maxLon;
	DWORD maxLat;
	DWORD maxAlt;
};

double getLatitude(DWORD lat);

double getLongitude(DWORD lon);

void parseFile(const char * filename);
