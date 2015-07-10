// WmaEncoder.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include <Windows.h>
#include <mfapi.h>
#include <Shlwapi.h>
#include <stdexcept>
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
      throw std::invalid_argument("Unsupported quality level");
    }
  }

  return MediaFoundationTransform::LoadWmaEncoderTransform(encodingFormat);
}


static void SetSinkWriterMetadata(MediaFoundationSinkWriter* writer, MediaFoundationSourceReader* reader, Parameters* parameters)
{
	wchar_t *value;

	value = parameters->Album;

	if (!value)
	{
		value = reader->GetMetadataValue(L"WM/AlbumTitle");
	}

	if (value)
	{
		writer->SetMetadata(L"WM/AlbumTitle", value);
	}

	value = parameters->Artist;

	if (!value)
	{
		value = reader->GetMetadataValue(L"Author");
	}

	if (value)
	{
		writer->SetMetadata(L"Author", value);
	}

	value = parameters->Artist;

	if (!value)
	{
		value = reader->GetMetadataValue(L"WM/AlbumArtist");
	}

	if (value)
	{
		writer->SetMetadata(L"WM/AlbumArtist", value);
	}

	value = parameters->Genre;

	if (!value)
	{
		value = reader->GetMetadataValue(L"WM/Genre");
	}

	if (value)
	{
		writer->SetMetadata(L"WM/Genre", value);
	}

	value = parameters->Title;

	if (!value)
	{
		value = reader->GetMetadataValue(L"Title");
	}

	if (value)
	{
		writer->SetMetadata(L"Title", value);
	}

	value = parameters->TrackNumber;

	if (!value)
	{
		value = reader->GetMetadataValue(L"WM/TrackNumber");
	}

	if (value)
	{
		writer->SetMetadata(L"WM/TrackNumber", value);
	}

	value = parameters->Year;

	if (!value)
	{
		value = reader->GetMetadataValue(L"WM/Year");
	}

	if (value)
	{
		writer->SetMetadata(L"WM/Year", value);
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
  printf("WMAEncoder\n");

  Parameters parameters;

  if (argc < 1)
  {
    printf("Command line help goes here...");
    return 0;
  }

  CommandLineParser::Parse(argc, argv, &parameters);

  // Verify that output folder exists, if specified
  // (and add a '\' to it if it doesn't exist)

  if (*parameters.OutputFolder)
  {
    WIN32_FILE_ATTRIBUTE_DATA fileData;

    BOOL success = GetFileAttributesEx(parameters.OutputFolder, GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &fileData);

    // check if the file system object exists, but it's not a directory...

    if (success && ((fileData.dwFileAttributes & 0x10) == 0))
    {
      printf("Specified output directory is not a directory");
      return 0;
    }

    if (!success)
    {
      printf("Specified output directory does not exist");
      return 0;
    }

    size_t outputFolderLength = wcslen(parameters.OutputFolder);

    if (outputFolderLength < MAX_PATH - 1)
    {
      if (*(parameters.OutputFolder + outputFolderLength - 1) != '\\')
      {
        *(parameters.OutputFolder + outputFolderLength) = '\\';
        *(parameters.OutputFolder + outputFolderLength + 1) = '\0';
      }
    }
  }

try
{
  // Initialize COM & Media Foundation

  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  HRESULT hr = MFStartup(MF_VERSION);

  MediaFoundationTransform *transform = LoadAppropriateTransform(&parameters);

  // Use the Windows shell API to extract the path component from the input filename

  wchar_t srcFileFolder[MAX_PATH];
  wchar_t srcFileName[MAX_PATH];

  wcscpy(srcFileFolder, parameters.InputFilename);

  BOOL ret = PathRemoveFileSpec(srcFileFolder);

  size_t srcFolderLength = wcslen(srcFileFolder);

  if (srcFolderLength < MAX_PATH - 1)
  {
    if (srcFolderLength > 0)
    {
      if (*(srcFileFolder + srcFolderLength - 1) != '\\')
      {
        *(srcFileFolder + srcFolderLength) = '\\';
        *(srcFileFolder + srcFolderLength + 1) = '\0';
      }
    }
  }

  // do some basic parsing of input filename, as FirstFirstFile / FindNext 
  // does not return the full path so we'll have to prepend
  // any directory info specified

  WIN32_FIND_DATA findData;

  HANDLE hFindFile = FindFirstFile(parameters.InputFilename, &findData);

  if (hFindFile != INVALID_HANDLE_VALUE)
  {
    do
    {
      wcscpy(srcFileName, srcFileFolder);
      wcscat(srcFileName, findData.cFileName);

      MediaFoundationSourceReader *reader = MediaFoundationSourceReader::CreateFromUrl(srcFileName);

      // if an output folder is specified, use that

      wchar_t outputFilename[MAX_PATH];

      if (*parameters.OutputFolder)
      {
        wcscpy(outputFilename, parameters.OutputFolder);
        wcscat(outputFilename, findData.cFileName);
        PathRenameExtension(outputFilename, L".wma");
      }
      else
      {
        wcscpy(outputFilename, parameters.OutputFilename);
      }

      MediaFoundationSinkWriter *writer = MediaFoundationSinkWriter::CreateSinkWriter(outputFilename);

      SetSinkWriterMetadata(writer, reader, &parameters);

      writer->Transcode(reader, transform);
    } while (FindNextFile(hFindFile, &findData));

    FindClose(hFindFile);
  }
  else
  {
    // input file does not exit

    throw std::invalid_argument("Input filename does not exist");
  }

  MFShutdown();
  CoUninitialize();
}
catch (std::exception &ex)
{
  printf("ERROR: %s\n", ex.what());
}


  return 0;
}



