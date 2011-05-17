#ifndef __OPENAL_OGGSAMPLE_H
#define __OPENAL_OGGSAMPLE_H

#define FLAC__NO_DLL

#include "Filesystem.h"
#include "openal_sample.h"
#include "openal_loader.h"
#include "FLAC++/decoder.h"
#include "FLAC/stream_decoder.h"


#define MAX_PATH_LENGTH 1024

class COpenALFLACSample : public IOpenALSample, public FLAC::Decoder::Stream
{
public:
    COpenALFLACSample();
    ~COpenALFLACSample();

    virtual void Open(const char* filename);
    virtual void Close();

    bool CheckStream(ALuint buffer);

protected:
    // Flac decoding stuff
    FLAC__StreamDecoderReadStatus   read_callback(FLAC__byte buffer[], size_t *bytes);
    FLAC__StreamDecoderSeekStatus   seek_callback(FLAC__uint64 absolute_byte_offset);
    FLAC__StreamDecoderTellStatus   tell_callback(FLAC__uint64 *absolute_byte_offset);
    FLAC__StreamDecoderLengthStatus length_callback(FLAC__uint64 *stream_length);
    FLAC__StreamDecoderWriteStatus  write_callback(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
    
    bool                            eof_callback();
    void                            metadata_callback(const ::FLAC__StreamMetadata *metadata);
    void                            error_callback(FLAC__StreamDecoderErrorStatus status);

private:
    FileHandle_t flacFile;

    char* m_pWriteData;
    char* m_pWriteDataEnd;
    int size;
    int sampleRate;
    int sizeOfLast;
    bool m_bHitEOF;
};

class COpenALFLACLoaderExt : public IOpenALLoaderExt
{
public:
    virtual bool Init();
	virtual void Shutdown();

    virtual IOpenALSample* Get();
};

#endif
