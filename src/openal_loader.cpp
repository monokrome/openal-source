#include "cbase.h"
#include "openal_loader.h"
#include "openal_sample.h"

COpenALLoader g_OpenALLoader;

IOpenALSample* COpenALLoader::Load(const char* path)
{
    if (path == NULL)
    {
        return NULL;
    }

    /*
    In most cases a file path is provided as argument, 
    but we support passing just the extension as well
    */
    bool isFile = true; 

    char ext[8];
    V_ExtractFileExtension(path, ext, sizeof(ext));

    if (!V_strlen(ext))
    {
        // Assume that the argument was the extension
        V_strcpy(ext, path);
        isFile = false;
    }

    unsigned short index = m_loaderExtensions.Find(ext);

    if ( m_loaderExtensions.IsValidIndex(index) )
    {
        IOpenALSample *pSample = m_loaderExtensions[index]->Get();

        if (pSample != NULL && isFile)
        {
            pSample->Open(path);
        }

        return pSample;
    }

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
