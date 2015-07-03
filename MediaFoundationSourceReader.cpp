#include "stdafx.h"
#include <Windows.h>
#include <propvarutil.h>
#include <map>
#include "MediaFoundationSourceReader.h"


MediaFoundationSourceReader::MediaFoundationSourceReader(IMFSourceReader *mfSourceReader)
{
	HRESULT hr;

	_mfSourceReader = mfSourceReader;

	IMFMetadataProvider *mfMetadataProvider;
	IMFMetadata *_mfMetadata;

	hr = _mfSourceReader->GetServiceForStream(MF_SOURCE_READER_MEDIASOURCE, GUID_NULL, __uuidof(IMFMetadataProvider), (LPVOID *)&mfMetadataProvider);

	if (hr == S_OK)
	{
		hr = mfMetadataProvider->QueryInterface(IID_PPV_ARGS(&_mfMetadata));
		mfMetadataProvider->Release();

		PROPVARIANT metadataKeys;

		_mfMetadata->GetAllPropertyNames(&metadataKeys);

		PWSTR *pwstr;
		ULONG count;

		PropVariantToStringVectorAlloc(metadataKeys, &pwstr, &count);

		_metadataItemCount = count;

		_metadata = (MetadataKeyValuePair *)malloc(count * sizeof(MetadataKeyValuePair));

		for (int i = 0; i < _metadataItemCount; i++)
		{
			PROPVARIANT metadataValue;

			hr = _mfMetadata->GetProperty(*(pwstr + i), &metadataValue);

			MetadataKeyValuePair *kvp = (_metadata + i);

			wcscpy_s(kvp->Key, sizeof(kvp->Key) / sizeof(kvp->Key[0]), *(pwstr + i));

			kvp->Value = metadataValue;

			hr = S_OK;
		}

		CoTaskMemFree(pwstr);

		PropVariantClear(&metadataKeys);
	}

}


MediaFoundationSourceReader *MediaFoundationSourceReader::CreateFromUrl(const wchar_t *url)
{
	IMFSourceReader *mfSourceReader;

	HRESULT hr = MFCreateSourceReaderFromURL(url, nullptr, &mfSourceReader);

	return new MediaFoundationSourceReader(mfSourceReader);
}


IMFMediaType *MediaFoundationSourceReader::GetCurrentMediaType()
{
	IMFMediaType *mfMediaType;

	HRESULT hr = _mfSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &mfMediaType);

	return mfMediaType;
}


IMFSourceReader *MediaFoundationSourceReader::GetSourceReader()
{
	return _mfSourceReader;
}

