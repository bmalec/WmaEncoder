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

	WAVEFORMATEX wfex;

	UINT32 value;

	wfex.cbSize = 0;
	wfex.wFormatTag = WAVE_FORMAT_PCM;

	inputMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &value);
	wfex.nChannels = value;

	inputMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &value);
	wfex.nSamplesPerSec = value;

	inputMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &value);
	wfex.wBitsPerSample = value;

	wfex.nBlockAlign = (wfex.nChannels * wfex.wBitsPerSample) / 8;
	wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;

	IMFMediaType *mediaTypeHack;

	hr = MFCreateMediaType(&mediaTypeHack);
	hr = MFInitMediaTypeFromWaveFormatEx(mediaTypeHack, &wfex, sizeof(wfex));

	hr = reader->GetSourceReader()->SetCurrentMediaType(0, nullptr, mediaTypeHack);

	hr = _mfSinkWriter->SetInputMediaType(0, mediaTypeHack, nullptr);

  DWORD dw = GetLastError();

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
