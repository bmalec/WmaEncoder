#include "stdafx.h"
#include <mfapi.h>
#include <propvarutil.h>
#include <Mferror.h>
#include <Wmcodecdsp.h>
#include "MediaFoundationMediaType.h"
#include "MediaFoundationTransform.h"

void MediaFoundationTransform::SetBooleanProperty(PROPERTYKEY key, bool value)
{
	PROPVARIANT propVar;

	InitPropVariantFromBoolean(value, &propVar);
	HRESULT hr = _propertyStore->SetValue(key, propVar);
	hr = S_OK;
}


void MediaFoundationTransform::SetUint32Property(PROPERTYKEY key, UINT32 value)
{
	PROPVARIANT propVar;

	InitPropVariantFromUInt32(value, &propVar);
	HRESULT hr = _propertyStore->SetValue(key, propVar);
	hr = S_OK;
}


MediaFoundationTransform::MediaFoundationTransform(IMFActivate *activationObj, WmaEncodingFormat encodingFormat)
{
	UINT32 nameLen;

	// Transform name is an MFActivate object property, so get that first

	activationObj->GetString(MFT_FRIENDLY_NAME_Attribute, _transformName, sizeof(_transformName), &nameLen);

	HRESULT hr = activationObj->ActivateObject(IID_PPV_ARGS(&_mfEncoder));

	hr = _mfEncoder->QueryInterface(IID_PPV_ARGS(&_propertyStore));

	// Configure the tranform to perform the requested compression format

	switch (encodingFormat)
	{
	case WmaEncodingFormat::Lossless:
		SetBooleanProperty(MFPKEY_VBRENABLED, true);
		SetBooleanProperty(MFPKEY_CONSTRAIN_ENUMERATED_VBRQUALITY, true);
		SetUint32Property(MFPKEY_DESIRED_VBRQUALITY, 100);
		break;

	case WmaEncodingFormat::Quality_10:
		SetBooleanProperty(MFPKEY_VBRENABLED, true);
		SetBooleanProperty(MFPKEY_CONSTRAIN_ENUMERATED_VBRQUALITY, true);
		SetUint32Property(MFPKEY_DESIRED_VBRQUALITY, 10);
		break;

	case WmaEncodingFormat::Quality_25:
		SetBooleanProperty(MFPKEY_VBRENABLED, true);
		SetBooleanProperty(MFPKEY_CONSTRAIN_ENUMERATED_VBRQUALITY, true);
		SetUint32Property(MFPKEY_DESIRED_VBRQUALITY, 25);
		break;

	case WmaEncodingFormat::Quality_50:
		SetBooleanProperty(MFPKEY_VBRENABLED, true);
		SetBooleanProperty(MFPKEY_CONSTRAIN_ENUMERATED_VBRQUALITY, true);
		SetUint32Property(MFPKEY_DESIRED_VBRQUALITY, 50);
		break;

	case WmaEncodingFormat::Quality_75:
		SetBooleanProperty(MFPKEY_VBRENABLED, true);
		SetBooleanProperty(MFPKEY_CONSTRAIN_ENUMERATED_VBRQUALITY, true);
		SetUint32Property(MFPKEY_DESIRED_VBRQUALITY, 75);
		break;

	case WmaEncodingFormat::Quality_90:
		SetBooleanProperty(MFPKEY_VBRENABLED, true);
		SetBooleanProperty(MFPKEY_CONSTRAIN_ENUMERATED_VBRQUALITY, true);
		SetUint32Property(MFPKEY_DESIRED_VBRQUALITY, 90);
		break;

	case WmaEncodingFormat::Quality_98:
		SetBooleanProperty(MFPKEY_VBRENABLED, true);
		SetBooleanProperty(MFPKEY_CONSTRAIN_ENUMERATED_VBRQUALITY, true);
		SetUint32Property(MFPKEY_DESIRED_VBRQUALITY, 98);
		break;

	}

	hr = _propertyStore->Commit();

	// enumerate output types and try to find the appropriate one for our purposes

	DWORD index = 0;

	while (true)
	{
		IMFMediaType *mediaType;

		hr = _mfEncoder->GetOutputAvailableType(0, index++, &mediaType);

		if (hr == MF_E_NO_MORE_TYPES)
			break;

		// Get the AM_MEDIA_TYPE structure from the media type, in case we want to need
		// to differentiate between Standard and Pro WMA codecs in the future...

		AM_MEDIA_TYPE *amMediaType;
	    mediaType->GetRepresentation(AM_MEDIA_TYPE_REPRESENTATION, (LPVOID *) &amMediaType);
	    WAVEFORMATEX *waveFormat = (WAVEFORMATEX *) amMediaType->pbFormat;

		// there's only a few things we're interested in with the output type, so only bother grabbing those values

		UINT32 channelCount;
		UINT32 samplesPerSecond;
		UINT32 bitsPerSample;

		hr = mediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channelCount);
		hr = mediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);
		hr = mediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);

		if ((channelCount == 2) && (bitsPerSample == 16) && (samplesPerSecond == 44100))
		{
			_mfMediaType = mediaType;
			break;
		}
	}

  index = 0;
}


IMFMediaType *MediaFoundationTransform::GetMediaType()
{
	return _mfMediaType;
}


MediaFoundationTransform *MediaFoundationTransform::LoadWmaEncoderTransform(WmaEncodingFormat encodingFormat)
{
	MediaFoundationTransform *result = nullptr;
	UINT32 transformCount;
	IMFActivate **transformActivationObjs;
	MFT_REGISTER_TYPE_INFO typeInfo;

	typeInfo.guidMajorType = MFMediaType_Audio;
	typeInfo.guidSubtype = (encodingFormat == WmaEncodingFormat::Lossless) ? MFAudioFormat_WMAudio_Lossless : MFAudioFormat_WMAudioV8;

	HRESULT hr = MFTEnumEx(MFT_CATEGORY_AUDIO_ENCODER, MFT_ENUM_FLAG_TRANSCODE_ONLY, nullptr,  &typeInfo, &transformActivationObjs, &transformCount);

	// early out if return code is bad or no transforms found

	if ((hr != S_OK) || (transformCount < 1))
		return nullptr;

	// Regardless how many activation objects returned, just instantiate the first one
	// (would I want to instantiate another?  Why?  Which one?)

	result = new MediaFoundationTransform(*transformActivationObjs, encodingFormat);

	// release all the stupid activation pointers (because COM was such a GREAT idea)

	for (UINT32 i = 0; i < transformCount; i++)
	{
		IMFActivate *temp = *(transformActivationObjs + i);
		temp->Release();
	}

	// free the stupid activation array object (because COM was such an f'ing great idea)
	// (did I ever mention I think COM was just... stupid?)

	CoTaskMemFree(transformActivationObjs);

	return result;
}