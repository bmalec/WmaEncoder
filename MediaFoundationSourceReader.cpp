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

		PWSTR *metadataPropertyKeys;
		ULONG metadataPropertyCount;

		PropVariantToStringVectorAlloc(metadataKeys, &metadataPropertyKeys, &metadataPropertyCount);

		_metadataItemCount = metadataPropertyCount;

		_metadata = (MetadataKeyValuePair *)malloc(metadataPropertyCount * sizeof(MetadataKeyValuePair));

		for (int i = 0; i < _metadataItemCount; i++)
		{
			PROPVARIANT metadataValue;
			PWSTR pvStringBuffer;

			wchar_t *metadataKey = *(metadataPropertyKeys + i);

			hr = _mfMetadata->GetProperty(metadataKey, &metadataValue);

			MetadataKeyValuePair *kvp = (_metadata + i);

			wcscpy_s(kvp->Key, sizeof(kvp->Key) / sizeof(kvp->Key[0]), metadataKey);

			hr = PropVariantToStringAlloc(metadataValue, &pvStringBuffer);

			kvp->Value = (wchar_t *) malloc((wcslen(pvStringBuffer) + 1) * sizeof(wchar_t));

			wcscpy_s(kvp->Value, wcslen(pvStringBuffer) + 1, pvStringBuffer);

			CoTaskMemFree(pvStringBuffer);
//			kvp->Value = metadataValue;

			hr = S_OK;
		}

		CoTaskMemFree(metadataPropertyKeys);

		PropVariantClear(&metadataKeys);
	}

}


wchar_t *MediaFoundationSourceReader::GetMetadataValue(wchar_t *metadataKey)
{
	wchar_t *result = nullptr;

	for (int i = 0; i < _metadataItemCount; i++)
	{
		if (wcscmp((_metadata + i)->Key, metadataKey) == 0)
		{
			result = (_metadata + i)->Value;
			break;
		}
	}

	return result;
}


MediaFoundationSourceReader *MediaFoundationSourceReader::CreateFromUrl(const wchar_t *url)
{
	IMFSourceReader *mfSourceReader;

  if (!SUCCEEDED(MFCreateSourceReaderFromURL(url, nullptr, &mfSourceReader)))
  {
    throw std::invalid_argument("Unable to open media file");
  }

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

