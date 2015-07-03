#pragma once

#include <Windows.h>
#include <string>

struct Parameters
{
public:
	INT32 Bitrate = -1;
	std::wstring Title;
	std::wstring Artist;
	std::wstring Album;
	std::wstring Year;
	UINT8 TrackNumber = 0;
	std::wstring Genre;
	UINT8 Quality = 0;
	std::wstring InputFilename;
	std::wstring OutputFilename;
	std::wstring OutputFolder;


};
