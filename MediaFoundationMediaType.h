#pragma once

#include <mfobjects.h>

class MediaFoundationMediaType
{
public:
	MediaFoundationMediaType(IMFMediaType *mfMediaType);

	UINT32 GetBitsPerSample();
	UINT32 GetChannelCount();
	UINT32 GetSamplesPerSecond();

private:
	UINT32 _avgBytesPerSec;
	UINT32 _audioBlockAlignment;
	UINT32 _channelCount;
	GUID   _majorType;
	UINT32 _samplesPerSecond;
	UINT32 _preferWaveFormatEx;
	BYTE  *_userData;
	UINT32 _fixedSizeSamples;
	UINT32 _allSamplesIndependent;
	UINT32 _bitsPerSample;
	GUID   _subType;
};
