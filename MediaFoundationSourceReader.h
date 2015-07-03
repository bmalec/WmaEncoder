#pragma once

#include <mfidl.h>
#include <mfreadwrite.h>

class MediaFoundationSourceReader
{
	struct MetadataKeyValuePair
	{
		wchar_t Key[30];
		PROPVARIANT Value;
	};

private:
	IMFSourceReader *_mfSourceReader;
	MetadataKeyValuePair *_metadata;
	int _metadataItemCount;

protected:
	MediaFoundationSourceReader(IMFSourceReader *mfSourceReader);

public:
	static MediaFoundationSourceReader *CreateFromUrl(const wchar_t *url);

	IMFMediaType *GetCurrentMediaType();
	IMFSourceReader *GetSourceReader();
};
