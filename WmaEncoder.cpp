// WmaEncoder.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include <Windows.h>
#include <mfapi.h>
#include "MediaFoundationSourceReader.h"
#include "MediaFoundationSinkWriter.h"
#include "MediaFoundationTransform.h"
#include "CommandLineParser.h"


MediaFoundationTransform* LoadAppropriateTransform(Parameters *parameters)
{
  MediaFoundationTransform *transform = nullptr;
  WmaEncodingFormat encodingFormat = WmaEncodingFormat::Quality_75;


  // prefer VBR over CBR, if quality is specified then use that and
  // ignore any specified bitrate

  if (parameters->Quality > 0)
  { 
    switch (parameters->Quality)
    {
    case 10:
      encodingFormat = WmaEncodingFormat::Quality_10;
      break;

    case 25:
      encodingFormat = WmaEncodingFormat::Quality_25;
      break;

    case 50:
      encodingFormat = WmaEncodingFormat::Quality_50;
      break;

    case 75:
      encodingFormat = WmaEncodingFormat::Quality_75;
      break;

    case 90:
      encodingFormat = WmaEncodingFormat::Quality_90;
      break;

    case 98:
      encodingFormat = WmaEncodingFormat::Quality_98;
      break;

    case 100:
      encodingFormat = WmaEncodingFormat::Lossless;
      break;

    default:
      printf("Unsupported quality level specified");
    }
  }

  return MediaFoundationTransform::LoadWmaEncoderTransform(encodingFormat);
}


static void SetSinkWriterMetadata(MediaFoundationSinkWriter* writer, MediaFoundationSourceReader* reader, Parameters* parameters)
{
	if (parameters->Album.length() > 0)
	{
		writer->SetMetadata(L"WM/AlbumTitle", parameters->Album.c_str());
	}

	if (parameters->Artist.length() > 0)
	{
		writer->SetMetadata(L"Author", parameters->Artist.c_str());
		writer->SetMetadata(L"WM/AlbumArtist", parameters->Artist.c_str());
	}

	if (parameters->Genre.length() > 0)
	{
		writer->SetMetadata(L"WM/Genre", parameters->Genre.c_str());
	}

	if (parameters->Title.length() > 0)
	{
		writer->SetMetadata(L"Title", parameters->Title.c_str());
	}

	if (parameters->TrackNumber > 0)
	{
		wchar_t buffer[4];
		_itow(parameters->TrackNumber, buffer, 10);

		writer->SetMetadata(L"WM/TrackNumber", buffer);
		writer->SetMetadata(L"WM/Track", buffer);
	}

	if (parameters->Year.length() > 0)
	{
		writer->SetMetadata(L"WM/Year", parameters->Year.c_str());
	}


}



int _tmain(int argc, _TCHAR* argv[])
{
	Parameters parameters;

	if (argc < 1)
	{
		printf("Command line help goes here...");
		return 0;
	}

	CommandLineParser::Parse(argc, argv, &parameters);

  // Initialize COM & Media Foundation

  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	HRESULT hr = MFStartup(MF_VERSION);

  MediaFoundationTransform *transform = LoadAppropriateTransform(&parameters);

  // do some basic parsing of input filename, as FirstFirstFile / FindNext 
  // does not return the full path so we'll have to prepend
  // any directory info specified

  std::wstring inputDirectory;

  size_t lastBackslashPosition = parameters.InputFilename.find_last_of(L"\\");

  if (lastBackslashPosition != std::wstring::npos)
  {
    inputDirectory = parameters.InputFilename.substr(0, lastBackslashPosition + 1);
  }

  WIN32_FIND_DATA findData;

  HANDLE hFindFile = FindFirstFile(parameters.InputFilename.c_str(), &findData);

  if (hFindFile != INVALID_HANDLE_VALUE)
  {
    do
    {
      std::wstring filePath = inputDirectory + std::wstring(findData.cFileName);

	  MediaFoundationSourceReader *reader = MediaFoundationSourceReader::CreateFromUrl(filePath.c_str());

	  MediaFoundationSinkWriter *writer = MediaFoundationSinkWriter::CreateSinkWriter(parameters.OutputFilename.c_str());

	  SetSinkWriterMetadata(writer, reader, &parameters);

	  writer->Transcode(reader, transform);
    } while (FindNextFile(hFindFile, &findData));

    FindClose(hFindFile);
  }





//	MediaFoundationTransform *transform = MediaFoundationTransform::LoadWmaEncoderTransform(WmaEncodingFormat::Quality_75);


	MFShutdown();
  CoUninitialize();

	return 0;
}

