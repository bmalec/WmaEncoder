#include "stdafx.h"
#include <mfapi.h>
#include <propvarutil.h>
#include "MediaFoundationSinkWriter.h"

MediaFoundationSinkWriter *MediaFoundationSinkWriter::CreateSinkWriter(const wchar_t *uncFilePath)
{
	IMFSinkWriter *mfSinkWriter;

	HRESULT hr = MFCreateSinkWriterFromURL(uncFilePath, nullptr, nullptr, &mfSinkWriter);

	return new MediaFoundationSinkWriter(mfSinkWriter);
}


void MediaFoundationSinkWriter::SetMetadata(const wchar_t* key, const wchar_t* value)
{
	PROPVARIANT pv;
	HRESULT hr;

	InitPropVariantFromString(value, &pv);
	hr = _mfMetadata->SetProperty(key, &pv);

	hr = S_OK;
}


MediaFoundationSinkWriter::MediaFoundationSinkWriter(IMFSinkWriter *mfSinkWriter)
{
	HRESULT hr;

	_mfSinkWriter = mfSinkWriter;

	// get the interfaces required to write metadata

	IMFMetadataProvider *mfMetadataProvider;

	hr = _mfSinkWriter->GetServiceForStream(MF_SINK_WRITER_MEDIASINK, GUID_NULL, __uuidof(IMFMetadataProvider), (LPVOID *)&mfMetadataProvider);
	hr = mfMetadataProvider->QueryInterface(IID_PPV_ARGS(&_mfMetadata));
	mfMetadataProvider->Release();
}


void MediaFoundationSinkWriter::Transcode(MediaFoundationSourceReader *reader, MediaFoundationTransform *transform)
{
	HRESULT hr;

	IMFSourceReader *mfSourceReader = reader->GetSourceReader();

	// Get the input media type

	IMFMediaType *inputMediaType = reader->GetCurrentMediaType();

	DWORD streamIndex = 0;

	hr = _mfSinkWriter->AddStream(transform->GetMediaType(), &streamIndex);



	hr = _mfSinkWriter->SetInputMediaType(0, inputMediaType, nullptr);

	hr = _mfSinkWriter->BeginWriting();

	DWORD streamFlags;
	LONGLONG timestamp;
	IMFSample *mfSample;

	hr = mfSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &streamFlags, &timestamp, &mfSample);

	while (mfSample != nullptr)
	{
		hr = _mfSinkWriter->WriteSample(0, mfSample);

		mfSample->Release();

		hr = mfSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &streamFlags, &timestamp, &mfSample);

		if (hr != S_OK)
		{
			hr = S_OK;
		}
	}

	hr = _mfSinkWriter->Finalize();


}
