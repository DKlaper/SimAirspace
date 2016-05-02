
#include "BGLDecompiler.h"
#include "AirspaceDef.h"
#include "PSQLConn.h"

#define LONGITUDEFACTOR (360.0 / ((3.0 * 0x10000000) - 180.0))
#define LATITUDEFACTOR (180.0 / (2.0 * 0x10000000))

AirspaceDef* curSpace;
PSQLConn conn;

DWORD getSubsectionSize(DWORD sz)
{
	return ((sz & 0x10000) | 0x40000) >> 0x0E;
}

double getLatitude(DWORD lat)
{
	return 90 - lat * LATITUDEFACTOR;
}

double getLongitude(DWORD lon)
{
	return lon *  LONGITUDEFACTOR - 180.0;
}

void readData(std::ifstream& in, const DWORD& offset, const RecordType type)
{
	Airspace asp;
	BoundaryDesc desc;
	BoundaryLoc loc;
	Name name;
	in.seekg(offset);

	switch (type) {
	case BOUNDARY:
		curSpace = new AirspaceDef();
		in.read((char*)&asp, sizeof(Airspace));
		//printf("Airspace: %d type, %x alt, %d size, minalt %f, maxalt %f\n", asp.asType, asp.altType, asp.size, asp.minAlt/1000.0, asp.maxAlt/1000.0);
		curSpace->type = asp.asType;
		curSpace->setAltitudes(asp.altType, asp.minAlt / 1000.0, asp.maxAlt / 1000.0);
		while ( (int)in.tellg() < asp.size + offset ) // 6 is size of type+sizeid so smaller than that is not possible
		{
			readData(in, in.tellg(), (RecordType)in.peek());
		}
		//printf("Airspace %s: minAlt %f %f, firstPoint %d %f %f\n", curSpace->name, curSpace->minAGLalt, curSpace->minMSLalt, curSpace->points[0].type, getLatitude(curSpace->points[0].latitude), getLongitude(curSpace->points[0].longitude));
		conn.insertAirspace(curSpace);
		delete curSpace;
		curSpace = NULL;
		break;

	case NONE:
		//printf("None region found, just skipping at %x\n", (int)in.tellg());
		in.seekg(in.tellg()+(std::streampos)sizeof(DWORD));
		break;
		

	case NAME:
		in.read((char*)&name, 6);
		name.string[name.size - 6] = '\0';
		in.read((char*)&name.string, name.size - 6);
		//printf("Name size %d read: %s\n", name.size, name.string);
		if (curSpace != NULL)
		{
			strcpy_s(curSpace->name, name.string);
		}
		break;

	case BOUNDS:
		in.read((char*)&desc, sizeof(BoundaryDesc));
		if (curSpace != NULL)
		{
			curSpace->pntcnt = desc.pntcnt;
			curSpace->points = new BoundaryLoc[curSpace->pntcnt];
		}
		for (int i = 0; i < desc.pntcnt; ++i)
		{
			in.read((char*)&loc, sizeof(BoundaryLoc));
			/*if (loc.type != CIRCLE)
			{
				printf("%x at N%f and E%f, idx %d\n", loc.type, getLatitude(loc.latitude), getLongitude(loc.longitude), loc.idx);
			}
			else {
				printf("%x radius %f\n", loc.type, *reinterpret_cast<float*>(&loc.longitude));
			}*/
			curSpace->points[i] = loc;
		}
		break;

	case EXCLUSION:
		printf("Ignoring EXCLUSIONS\n");
		in.seekg(in.tellg() + (std::streampos)20); //fixed size
		break;

	case MDL:
	case NAMELIST:
		printf("Have no size thus we need to skip MDL or NAMELIST\n");
		return;

	default:
		printf("Currently not being read type %x at %x\n", type, (int)in.tellg());
		WORD id;
		in.read((char*)&id, sizeof(WORD));
		DWORD size;
		in.read((char*)&size, sizeof(DWORD));
		in.seekg(std::streamoff(size-6), std::ios_base::cur);
		break;
	}

}

void parseFile(const char* filename)
{
	conn.connect();
	Header head;
	Section sec;
	SubSection subsec;
	SubSectionBoundary subbnd;
	std::set<DWORD> entries;
	std::ifstream istream(filename, std::ios::in | std::ios::binary);

	// read the section headers and subsections then call the offset for the corresponding type
	istream.read((char*)&head, sizeof(Header));
	printf("%d sections and header size %d\n", head.sectionCount, head.size);
	for (int i = 0; i < head.sectionCount; ++i)
	{
		istream.read((char*)&sec, sizeof(Section));
		printf("%x type and subsectionSize %d and subsectionCount %d and subsectionStart %d, subsectionLength %d\n", sec.type, getSubsectionSize(sec.subsectionSize), sec.subsectionCount, sec.subsectionStart, sec.subsectionLength);

		if (sec.type == EXCLUSION || sec.type == MDL || sec.type == NAMELIST || sec.type == SCENERY || sec.type == ADDARPT) // skipping totally unsupported types
		{
			printf("Skipping unsupported type %x\n", sec.type);
			continue;
		}

		if (getSubsectionSize(sec.subsectionSize) == 16) // Ignore the terrain stuff
		{
			std::streampos lastpos = istream.tellg();
			istream.seekg(sec.subsectionStart);
			for (int j = 0; j < sec.subsectionCount; ++j)
			{
				istream.read((char*)&subsec, sizeof(SubSection));
				//printf("%x Idx %d and records %d and size %d\n", (int)istream.tellg(), subsec.idxOffset, subsec.recordCount, subsec.size);
				if (sec.type == BOUNDARY || sec.type == GEOPOL) // special case of offset with separate record headers
				{
					std::streampos prevpos = istream.tellg();
					istream.seekg(sec.subsectionStart + 16 * sec.subsectionCount + subsec.idxOffset*sizeof(SubSectionBoundary));
					for (int k = 0; k < subsec.recordCount; ++k)
					{
						istream.read((char*)&subbnd, sizeof(SubSectionBoundary));
						entries.emplace(subbnd.offset);
						//printf("%x Offset %x and size %d\n", (int)istream.tellg(), subbnd.offset, subbnd.size);
					}
					//printf("----\n");
					istream.seekg(prevpos);
				}
				else { // then we can use the index as offset
					entries.emplace(subsec.idxOffset);
				}
			}
			istream.seekg(lastpos);
			printf("Different records: %d\n", entries.size());
			
			// now we can finally read the real data
			for (const DWORD& entry : entries)
			{
				std::streampos prev = istream.tellg();
				readData(istream, entry, sec.type);
				istream.seekg(prev);
			}

			entries.clear();

		}

	}
	printf("file: %s done, now overall total airspaces found: %d, inserted successfully: %d\n", filename, conn.totalcnt, conn.successfulcnt);
	istream.close();
}


