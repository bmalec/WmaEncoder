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
				parameters->Album = currentToken;
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::Artist:
				parameters->Artist = currentToken;
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::Genre:
				parameters->Genre = currentToken;
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::Title:
				parameters->Title = currentToken;
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::TrackNumber:
				parameters->TrackNumber = currentToken;
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
				wcscpy(parameters->OutputFolder, currentToken);
				pendingOption = CommandLineOption::None;
				break;

			case CommandLineOption::Year:
				parameters->Year = currentToken;
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
				if (wcslen(parameters->InputFilename) == 0)
				{
          wcscpy(parameters->InputFilename, currentToken);
				}
				else if (wcslen(parameters->OutputFilename) == 0)
				{
					wcscpy(parameters->OutputFilename, currentToken);
				}
			}
			


		}
	}


}
