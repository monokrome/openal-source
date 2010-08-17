#include "cbase.h"
#include "openal_loader.h"
#include "openal_sample.h"

COpenALLoader g_OpenALLoader;

IOpenALSample* COpenALLoader::Load(char* fileType)
{
	unsigned short index = m_loaderExtensions.Find(fileType);

    if ( m_loaderExtensions.IsValidIndex(index) )
        return m_loaderExtensions[index]->Get();

	return NULL;
}

void COpenALLoader::Register(IOpenALLoaderExt *extension, char *fileType)
{
	if (m_loaderExtensions.Find(fileType) == m_loaderExtensions.InvalidHandle())
		m_loaderExtensions.Insert(fileType, extension);

	else
		Warning("OpenAL Loader: Tried to register \"%s\" type but it's already registered.\n");
}

void COpenALLoader::Deregister(IOpenALLoaderExt *extension, char *fileType)
{
	unsigned short index = m_loaderExtensions.Find(fileType);

	if (index == m_loaderExtensions.InvalidHandle())
	{
		Warning("OpenAL Loader: Attempted to remove \"%s\" reference but that type isn't registered.\n");
		return;
	}

	if (m_loaderExtensions[index] == extension)
		m_loaderExtensions.Remove(fileType);
}
