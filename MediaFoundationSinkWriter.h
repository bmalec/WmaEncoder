#pragma once

#include <mfidl.h>
#include <mfreadwrite.h>
#include "MediaFoundationSourceReader.h"
#include "MediaFoundationTransform.h"

class MediaFoundationSinkWriter
{
private:
	IMFSinkWriter*       _mfSinkWriter;
//	IMFMetadataProvider* _mfMetadataProvider;
	IMFMetadata*         _mfMetadata;

protected:
	MediaFoundationSinkWriter(IMFSinkWriter* mfSinkWriter);

public:
	static MediaFoundationSinkWriter* CreateSinkWriter(const wchar_t* uncFilePath);

	void SetMetadata(const wchar_t* key, const wchar_t* value);
	void Transcode(MediaFoundationSourceReader* reader, MediaFoundationTransform* transform);
};

