#pragma once
#include <Windows.h>
#include <strsafe.h>
#include <ShlObj.h>
#include <fstream>

const static size_t maxsize = 1024;
static wchar_t mypath[maxsize];

struct Configuration
{
	// part 1 Folders and DB port
	LPWSTR fsxFolder = new wchar_t[maxsize];
	LPWSTR postgresFolder = new wchar_t[maxsize];
	int port = 18200;

	// part 2 display settings
	int insideImportance = 10; // display inside message if more important than X
	int displayImportance = 0; // don't display message if less important than x
	int forbiddenImportance = 25; // imporance above which flying is forbidden
	float standardTimeout = 10.0; // how long in seconds is it displayed // only for entered and left msgs

	Configuration() {
		StringCchCopy(fsxFolder, maxsize, LR"(C:\FSX\)");
		StringCchCopy(postgresFolder, maxsize, LR"(C:\Users\David\Documents\Programming\postgresql-9.5.2-1-windows-x64-binaries\pgsql\bin\)");
	}
};



void inline loadFilePath()
{
	// just getting the value where to write
	PWSTR path;
	size_t path_length;
	SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);
	path_length = lstrlen(path);
	StringCchCopy(mypath, maxsize,path);
	CoTaskMemFree(path);

	const wchar_t folder[] = L"\\SimAirspace";
	StringCchCat(mypath, maxsize, folder);
	// will fail with already exists if already exists
	CreateDirectory(mypath, NULL);
	const wchar_t file[] = L"\\config.ini";
	StringCchCat(mypath, maxsize, file);
}

bool inline FileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void inline loadConfig(Configuration* config)
{
	loadFilePath();
	// only load if its existing and not a directory
	if (FileExists(mypath))
	{
		std::wifstream instream;
		wchar_t* tmp = new wchar_t[128];

		instream.open(mypath);
		instream.getline(config->fsxFolder, maxsize);
		instream.getline(config->postgresFolder, maxsize);
		instream >> config->port;
		instream.getline(tmp, 128);
		instream >> config->insideImportance;
		// get the rest of the line
		instream.getline(tmp, 128);

		instream >> config->displayImportance;
		instream.getline(tmp, 128);
		instream >> config->forbiddenImportance;
		instream.getline(tmp, 128);
		instream >> config->standardTimeout;
		instream.close();
		delete tmp;
	}
}

void inline saveConfig(Configuration configValues) {
	// actually writing the file
	std::wofstream outstream;
	outstream.open(mypath);
	outstream << configValues.fsxFolder << std::endl;
	outstream << configValues.postgresFolder << std::endl;
	outstream << configValues.port  << L" DB_port" << std::endl;

	outstream << configValues.insideImportance << L" Importance_for_inside" << std::endl;
	outstream << configValues.displayImportance << L" Importance_for_display" << std::endl;
	outstream << configValues.forbiddenImportance << L" Importance_for_forbiddenmsg" << std::endl;
	outstream << configValues.standardTimeout << L" msg_timeout" << std::endl;

	outstream.close();
}