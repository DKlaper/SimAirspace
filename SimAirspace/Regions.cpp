
#include "Regions.h"

/*
First I thought I need to do some hacjking for performance but postgis performance is great so I didn't use this.
So this class represents a double grid
so there's one grid starting at North POLARBORDER deg and W 180 = 0 
and makes squares that are SIZE big.
Then theres a second grid offset by half the SIZE in both dimensions
*/
static const size_t POLARBORDER = 70;
static const size_t POLARID = (100+POLARBORDER) * 1000;
static const size_t SIZE = 5;
static const double OFFSET = SIZE / 2.0;

/*
Important things to know:
latitude is top down so we always ceil 
longitude is from 180W = 0 to 179E = 359
Currently polar border is fixed so border O grid is cut off
*/
RegionIdPair* getRegionIds(double lat, double lon) {
	if (fabs(lat) > POLARBORDER) // just catch polar case
	{
		int polId = copysign(199000, lat);
		return new RegionIdPair(polId, polId);
	}

	int latRegA = SIZE*int(ceil(lat / SIZE));
	int latRegO = latRegA-OFFSET;
	if (lat + OFFSET > latRegA) // greater than because the line still belongs to the top one
	{
		latRegO += SIZE;
	}

	int lonRegA = SIZE*floor(double(lon + 180) / SIZE); 
	int lonRegO = lonRegA - OFFSET;

	if ((lon+180) + OFFSET >= lonRegA+SIZE) // greater or equals because the line already belongs to the next east quadrant
	{
		lonRegO += SIZE;
	}

	if (lonRegO < 0) // dealing with the offset grid wrapping around at 360 to 0
	{
		lonRegO = 360 - OFFSET;
	}

	int aReg = 100000 + 1000 * abs(latRegA) + lonRegA%360; // this modulo just deals with 180E 
	int oReg = 100000 + 1000 * abs(latRegO) + lonRegO%360; // this modulo is actually not necessary with the current grid definition.
	RegionIdPair* tupleRes = new RegionIdPair(copysign(aReg, latRegA), copysign(oReg, latRegO));
	return tupleRes;
}