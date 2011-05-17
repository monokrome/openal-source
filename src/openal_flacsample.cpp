#include "cbase.h"
#include "openal.h"
#include "openal_flacsample.h"

//#include "FLAC++/decoder.h"
//#include "FLAC/stream_decoder.h"

COpenALFLACSample::COpenALFLACSample()
{
    m_pWriteData = NULL;
    m_pWriteDataEnd = NULL;
    size = 0;
    sampleRate = 0;
    sizeOfLast = 0;
    m_bHitEOF = false;
}

COpenALFLACSample::~COpenALFLACSample()
{
}

void COpenALFLACSample::Open(const char* filename)
{
	char abspath[MAX_PATH_LENGTH];

    m_pWriteData = NULL;
    m_pWriteDataEnd = NULL;
    size = 0;
    sampleRate = 0;
    sizeOfLast = 0;
    m_bHitEOF = false;

    if (!FLAC::Decoder::Stream::is_valid())
    {
        FLAC__StreamDecoderState state = FLAC::Decoder::Stream::get_state();
        Warning("FLAC: Unable to initialize: %s", FLAC__StreamDecoderStateString[state] );
        return;
    }

	// Gets an absolute path to the provided filename
	g_OpenALGameSystem.GetSoundPath(filename, abspath, sizeof(abspath));

	flacFile = filesystem->Open(abspath, "rb");

	if (!flacFile)
	{
		Warning("FLAC: Could not open flac file: %s. Aborting.\n", filename);
		return;
	}

    FLAC__StreamDecoderInitStatus status = FLAC::Decoder::Stream::init();

    if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
    {
        filesystem->Close(flacFile);
        Warning("FLAC: Critical stream decoder init status: %s", FLAC__StreamDecoderInitStatusString[status] );
        return;
    }

    m_pWriteData = NULL;
    m_bHitEOF = false;
    
    m_bFinished = false; // Sample has just started, assume not finished

	Init();
}

void COpenALFLACSample::Close()
{
	m_bReady = false;
    m_bHitEOF = false;
	ClearBuffers();
    filesystem->Close(flacFile);

    if ( !FLAC::Decoder::Stream::finish() )
        Warning("FLAC; Memory allocation error on stream finish!\n");
}

bool COpenALFLACSample::CheckStream(ALuint buffer)
{
	if ( !IsReady() ) return false;

	char data[OPENAL_BUFFER_SIZE];
    size = 0;

    m_pWriteData = data;
    m_pWriteDataEnd = m_pWriteData + OPENAL_BUFFER_SIZE;

    while ( size < OPENAL_BUFFER_SIZE )
    {
        FLAC__StreamDecoderState state = FLAC::Decoder::Stream::get_state();

        if (state == FLAC__STREAM_DECODER_END_OF_STREAM)
        {
            if (m_bLooping)
            {
                //FLAC::Decoder::Stream::flush();
                //FLAC::Decoder::Stream::seek_absolute(0);

                FLAC::Decoder::Stream::reset();
            }
            else
            {
                m_bHitEOF = true;
                break;
            }
        }
        else if ( state >= FLAC__STREAM_DECODER_OGG_ERROR )
        {
            // Critical error occured
            Warning("FLAC: Decoding returned with critical state: %s", FLAC__StreamDecoderStateString[state] );
            break;
        }

        if ( !FLAC::Decoder::Stream::process_single() )
        {
            Warning("FLAC: Processing of a single frame failed!\n");
            break;
        }
        
        // if we can't fit an additional frame into the buffer, quit
        if (sizeOfLast > m_pWriteDataEnd-m_pWriteData )
        {
            break;
        }
    }

    if (m_bHitEOF)
    {
        m_bFinished = true;
        return false;
    }
    else
    {
        BufferData(buffer, format, data, size, sampleRate);
    }

	return true;
}

FLAC__StreamDecoderReadStatus COpenALFLACSample::read_callback(FLAC__byte buffer[], size_t *bytes)
{
    if(*bytes > 0) 
    {
        int size = filesystem->ReadEx(buffer, sizeof(FLAC__byte), *bytes, flacFile);

        if (size < 0)
        {
            m_bHitEOF = true;
            return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
        }
        else if (size == 0)
        {
            m_bHitEOF = true;
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }
        else
        {
            return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
        }
    }
    else
    {
        m_bHitEOF = true;
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
}

FLAC__StreamDecoderSeekStatus COpenALFLACSample::seek_callback(FLAC__uint64 absolute_byte_offset)
{
    filesystem->Seek( flacFile, (off_t)absolute_byte_offset, FILESYSTEM_SEEK_HEAD );
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

FLAC__StreamDecoderTellStatus COpenALFLACSample::tell_callback(FLAC__uint64 *absolute_byte_offset)
{
    int offset = filesystem->Tell( flacFile );
    *absolute_byte_offset = (FLAC__uint64)offset;
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus COpenALFLACSample::length_callback(FLAC__uint64 *stream_length)
{
    int size = filesystem->Size( flacFile );
    *stream_length = (FLAC__uint64)size;
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

bool COpenALFLACSample::eof_callback()
{
    bool hitEOF = filesystem->EndOfFile( flacFile );

    if (hitEOF)
    {
        m_bHitEOF = true;
    }

    return hitEOF;
}

FLAC__StreamDecoderWriteStatus COpenALFLACSample::write_callback(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[])
{
    register signed int sample0, sample1;

    // TODO: Write functions for handling 8-bit audio as well
    if (frame->header.bits_per_sample != 16)
    {
        Warning("FLAC: Unsupported bit-rate: %i", frame->header.bits_per_sample);
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    
    /* write decoded PCM samples */
    if (frame->header.channels == 2)
    {
        // Stereo
        for(unsigned int i = 0; i < frame->header.blocksize; i++) 
        {
            if (m_pWriteData != m_pWriteDataEnd)
            {
                sample0 = buffer[0][i];
                sample1 = buffer[1][i];

                m_pWriteData[0] = sample0 >> 0;
                m_pWriteData[1] = sample0 >> 8;

                m_pWriteData[2] = sample1 >> 0;
                m_pWriteData[3] = sample1 >> 8;

                m_pWriteData += 4;
            }
            else
            {
                m_bHitEOF = true;
                return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
            }
        }

        sizeOfLast = frame->header.blocksize * 2 * 2;   
        size += sizeOfLast;
    }
    else
    {
        // Mono
        for(unsigned int i = 0; i < frame->header.blocksize; i++) 
        {
            if (m_pWriteData != m_pWriteDataEnd)
            {
                sample0 = buffer[0][i];

                m_pWriteData[0] = sample0 >> 0;
                m_pWriteData[1] = sample0 >> 8;

                m_pWriteData += 2;
            }
            else
            {
                m_bHitEOF = true;
                return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
            }
        }

        sizeOfLast = frame->header.blocksize * 2;   
        size += sizeOfLast;
    }
    
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void COpenALFLACSample::metadata_callback(const FLAC__StreamMetadata *metadata)
{
    int bits = metadata->data.stream_info.bits_per_sample;
    int channels = metadata->data.stream_info.channels;
    sampleRate = metadata->data.stream_info.sample_rate;

    if (bits == 16)
    {
        format = channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    }
    else if ( bits == 8 )
    {
        format = channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
    }
    else
    {
        Warning("FLAC: Unsupported sample bit size: %i\n", bits);
    }

    // Debug header
    if (!m_bLooping)
    {
        Msg("FLAC: %i bits %s audio at %i\n",
            bits,
            channels == 2 ? "stereo" : "mono",
            sampleRate);
    }
}

void COpenALFLACSample::error_callback(::FLAC__StreamDecoderErrorStatus status)
{
    // All calls to this function is critical
    Warning("FLAC: Got decoding error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

COpenALFLACLoaderExt flacLoader;

bool COpenALFLACLoaderExt::Init()
{
	g_OpenALLoader.Register(this, "flac");

	return true;
}

void COpenALFLACLoaderExt::Shutdown()
{
	g_OpenALLoader.Deregister(this, "flac");
}

IOpenALSample* COpenALFLACLoaderExt::Get()
{
	return new COpenALFLACSample();
}
