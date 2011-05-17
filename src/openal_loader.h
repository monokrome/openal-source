#ifndef __OPENAL_LOADER_H
#define __OPENAL_LOADER_H

#include "cbase.h"
#include "tier1/utlhashdict.h"
#include "openal_sample.h"

/***
 * Each sample type defines a loader extension that will be used by COpenALLoader
 * to retrieve a reference to the provided type of sample.
 **/
class IOpenALLoaderExt : public CAutoGameSystem
{
public:
	virtual IOpenALSample* Get() = 0;
};

/***
 * The master loader attempts to load audio files based on a file type hint passed to
 * the Load method.
 **/
class COpenALLoader
{
public:
	IOpenALSample* Load(const char* fileType);

	void Register(IOpenALLoaderExt *extension, char *fileType);
	void Deregister(IOpenALLoaderExt *extension, char *fileType);

private:
	CUtlHashDict<IOpenALLoaderExt*> m_loaderExtensions;
};

extern COpenALLoader g_OpenALLoader;

#endif
