#pragma once

#include <mftransform.h>

enum class WmaEncodingFormat { Lossless, ConstantBitRate, Quality_10, Quality_25, Quality_50, Quality_75, Quality_90, Quality_98 };

class MediaFoundationTransform
{
private:
	IMFTransform   *_mfEncoder;
	IPropertyStore *_propertyStore;
	IMFMediaType   *_mfMediaType = nullptr;
	WCHAR           _transformName[128];

	void SetBooleanProperty(PROPERTYKEY key, bool value);
	void SetUint32Property(PROPERTYKEY key, UINT32 value);

protected:
	MediaFoundationTransform(IMFActivate *activationObject, WmaEncodingFormat encodingFormat);

public:
	static MediaFoundationTransform *LoadWmaEncoderTransform(WmaEncodingFormat encodingFormat);

	IMFMediaType *GetMediaType();
	IMFTransform *GetTransform();




};
