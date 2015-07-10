#include "stdafx.h"
#include <mfapi.h>
#include <propvarutil.h>
#include "MediaFoundationSinkWriter.h"

static IMFMediaType *CreateIntermediateMediaType(IMFMediaType *inputMediaType)
{
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

  IMFMediaType *result;

  HRESULT hr = MFCreateMediaType(&result);
  hr = MFInitMediaTypeFromWaveFormatEx(result, &wfex, sizeof(wfex));

  return result;
}



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
  DWORD streamIndex = 0;

	IMFSourceReader *mfSourceReader = reader->GetSourceReader();
	hr = _mfSinkWriter->AddStream(transform->GetMediaType(), &streamIndex);

  // Get the input media type

  IMFMediaType *inputMediaType = reader->GetCurrentMediaType();

  // Initially, I attempted to set the sink writer InputMediaType to what was returned from a call to GetCurrentMediaType() on the
  // SourceReader.  In some cases, that worked (.WAV to .WMA, WMA lossless to WMA lossless).  But in other cases, it didn't.
  // However I discovered if I re-created the media type by creating a new MediaType instance using the main attributes
  // (bits per sample, # of tracks, etc) of  the input media type and telling both the SinkWriter and SourceReader to use
  // this new MediaType, things "magically" worked.
  //
  // Oftentimes I hear the phrase, "Hope is not a strategy."  I agree, hope is indeed not a strategy.  With Media Foundation, 
  // the only strategy is magic.

  IMFMediaType *intermediateMediaType = CreateIntermediateMediaType(inputMediaType);

	hr = reader->GetSourceReader()->SetCurrentMediaType(0, nullptr, intermediateMediaType);
	hr = _mfSinkWriter->SetInputMediaType(0, intermediateMediaType, nullptr);

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
