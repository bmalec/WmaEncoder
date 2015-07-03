#include "stdafx.h"
#include <regex>
#include "CommandLineParser.h"

enum class CommandLineOption { None, Bitrate, OutputFolder, Title, Artist, Album, Year, Genre, TrackNumber, Quality };

struct ParameterMap
{
public:
	CommandLineOption Option;
	WCHAR* Token;
};

static ParameterMap parameterMapping[] = {
	{CommandLineOption::Bitrate, L"-b"},
	{CommandLineOption::OutputFolder, L"-o"},
	{ CommandLineOption::Title, L"--tt" },
	{ CommandLineOption::Artist, L"--ta" },
	{ CommandLineOption::Album, L"--tl" },
	{ CommandLineOption::Year, L"--ty" },
	{ CommandLineOption::Genre, L"--tg" },
	{ CommandLineOption::TrackNumber, L"--tn" },
    { CommandLineOption::Quality, L"-V" }
};





void CommandLineParser::Parse(int argc, WCHAR* argv[], Parameters* parameters)
{
	CommandLineOption pendingOption = CommandLineOption::None;

	for (int i = 1; i < argc; i++)
	{
		WCHAR *currentToken = argv[i];

		if (pendingOption != CommandLineOption::None)
		{
			long temp;

			switch (pendingOption)
			{
			case CommandLineOption::Album:
				parameters->Album = std::wstring(currentToken);
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::Artist:
				parameters->Artist = std::wstring(currentToken);
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::Genre:
				parameters->Genre = std::wstring(currentToken);
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::Title:
				parameters->Title = std::wstring(currentToken);
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::TrackNumber:
				temp = wcstol(currentToken, NULL, 10);
				parameters->TrackNumber = temp;
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::Bitrate:
				temp = wcstol(currentToken, NULL, 10);
				parameters->Bitrate = (int)temp;
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::Quality:
				temp = wcstol(currentToken, NULL, 10);
				parameters->Quality = (int)temp;
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::OutputFolder:
				parameters->OutputFolder = std::wstring(currentToken);
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::Year:
				parameters->Year = std::wstring(currentToken);
				pendingOption = CommandLineOption::None;
				break;
			}
		}
		else
		{
			// no pending command line option we need read the parameter for, so 
			// either this token will be a command line option or filename

			for (int j = 0; j < (sizeof(parameterMapping) / sizeof(parameterMapping[0])); j++)
			{
				if (wcscmp(currentToken, parameterMapping[j].Token) == 0)
				{
					pendingOption = parameterMapping[j].Option;
					break;
				}
			}

			if (pendingOption == CommandLineOption::None)
			{
				if (parameters->InputFilename.length() == 0)
				{
					parameters->InputFilename = std::wstring(currentToken);
				}
				else if (parameters->OutputFilename.length() == 0)
				{
					parameters->OutputFilename = std::wstring(currentToken);
				}
			}
			


		}
	}


}
