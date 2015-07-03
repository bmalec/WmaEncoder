#include "stdafx.h"
#include <mfapi.h>
#include "MediaFoundationMediaType.h"


MediaFoundationMediaType::MediaFoundationMediaType(IMFMediaType *mfMediaType)
{
	UINT32 count;

	HRESULT hr = mfMediaType->GetCount(&count);

	for (int i = 0; i < count; i++)
	{
		GUID key;
		MF_ATTRIBUTE_TYPE type;

		hr = mfMediaType->GetItemByIndex(i, &key, nullptr);
		hr = mfMediaType->GetItemType(key, &type);

		if (key == MF_MT_AUDIO_AVG_BYTES_PER_SECOND)
		{
			mfMediaType->GetUINT32(key, &_avgBytesPerSec);
		}
		else if (key == MF_MT_AUDIO_BLOCK_ALIGNMENT)
		{
			mfMediaType->GetUINT32(key, &_audioBlockAlignment);
		}
		else if (key == MF_MT_AUDIO_NUM_CHANNELS)
		{
			mfMediaType->GetUINT32(key, &_channelCount);
		}
		else if (key == MF_MT_MAJOR_TYPE)
		{
			mfMediaType->GetGUID(key, &_majorType);
		}
		else if (key == MF_MT_AUDIO_SAMPLES_PER_SECOND)
		{
			mfMediaType->GetUINT32(key, &_samplesPerSecond);
		}
		else if (key == MF_MT_AUDIO_PREFER_WAVEFORMATEX)
		{
			mfMediaType->GetUINT32(key, &_preferWaveFormatEx);
		}
		else if (key == MF_MT_USER_DATA)
		{
			UINT32 blobSize;

			mfMediaType->GetBlobSize(key, &blobSize);
			_userData = (BYTE *)malloc(blobSize);
			mfMediaType->GetBlob(key, _userData, blobSize, &blobSize);
		}
		else if (key == MF_MT_FIXED_SIZE_SAMPLES)
		{
			mfMediaType->GetUINT32(key, &_fixedSizeSamples);
		}
		else if (key == MF_MT_ALL_SAMPLES_INDEPENDENT)
		{
			mfMediaType->GetUINT32(key, &_allSamplesIndependent);
		}
		else if (key == MF_MT_AUDIO_BITS_PER_SAMPLE)
		{
			mfMediaType->GetUINT32(key, &_bitsPerSample);
		}
		else if (key == MF_MT_SUBTYPE)
		{
			mfMediaType->GetGUID(key, &_subType);
		}
		else
		{
			hr = 0;
		}



		hr = 0;

	}

}


UINT32 MediaFoundationMediaType::GetBitsPerSample()
{
	return _bitsPerSample;
}


UINT32 MediaFoundationMediaType::GetChannelCount()
{
	return _channelCount;
}



UINT32 MediaFoundationMediaType::GetSamplesPerSecond()
{
	return _samplesPerSecond;

}
