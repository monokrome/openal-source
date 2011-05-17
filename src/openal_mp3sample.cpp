#include "cbase.h"
#include "openal.h"
#include "openal_mp3sample.h"

#include <libmad/mad.h>

//ConVar openal_mp3_debug("openal_mp3_debug", "1", 0, "Provides debug information from the ", true, 0, true, 1 );



static struct audio_dither left_dither, right_dither;

static const char *MadErrorString(const struct mad_stream *Stream)
{
	switch(Stream->error)
	{
		/* Generic unrecoverable errors. */
		case MAD_ERROR_BUFLEN:
			return("input buffer too small (or EOF)");
		case MAD_ERROR_BUFPTR:
			return("invalid (null) buffer pointer");
		case MAD_ERROR_NOMEM:
			return("not enough memory");

		/* Frame header related unrecoverable errors. */
		case MAD_ERROR_LOSTSYNC:
			return("lost synchronization");
		case MAD_ERROR_BADLAYER:
			return("reserved header layer value");
		case MAD_ERROR_BADBITRATE:
			return("forbidden bitrate value");
		case MAD_ERROR_BADSAMPLERATE:
			return("reserved sample frequency value");
		case MAD_ERROR_BADEMPHASIS:
			return("reserved emphasis value");

		/* Recoverable errors */
		case MAD_ERROR_BADCRC:
			return("CRC check failed");
		case MAD_ERROR_BADBITALLOC:
			return("forbidden bit allocation value");
		case MAD_ERROR_BADSCALEFACTOR:
			return("bad scalefactor index");
		case MAD_ERROR_BADFRAMELEN:
			return("bad frame length");
		case MAD_ERROR_BADBIGVALUES:
			return("bad big_values count");
		case MAD_ERROR_BADBLOCKTYPE:
			return("reserved block_type");
		case MAD_ERROR_BADSCFSI:
			return("bad scalefactor selection info");
		case MAD_ERROR_BADDATAPTR:
			return("bad main_data_begin pointer");
		case MAD_ERROR_BADPART3LEN:
			return("bad audio data length");
		case MAD_ERROR_BADHUFFTABLE:
			return("bad Huffman table select");
		case MAD_ERROR_BADHUFFDATA:
			return("Huffman data overrun");
		case MAD_ERROR_BADSTEREO:
			return("incompatible block_type for JS");

		/* Unknown error. This switch may be out of sync with libmad's
		 * defined error codes.
		 */
		default:
			return("Unknown error code");
	}
}

static inline
unsigned long prng(unsigned long state)
{
    return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

inline
signed long audio_linear_dither(unsigned int bits, mad_fixed_t sample,
struct audio_dither *dither,
struct audio_stats *stats)
{
    unsigned int scalebits;
    mad_fixed_t output, mask, random;

    enum {
        MIN = -MAD_F_ONE,
        MAX =  MAD_F_ONE - 1
    };

    /* noise shape */
    sample += dither->error[0] - dither->error[1] + dither->error[2];

    dither->error[2] = dither->error[1];
    dither->error[1] = dither->error[0] / 2;

    /* bias */
    output = sample + (1L << (MAD_F_FRACBITS + 1 - bits - 1));

    scalebits = MAD_F_FRACBITS + 1 - bits;
    mask = (1L << scalebits) - 1;

    /* dither */
    random  = prng(dither->random);
    output += (random & mask) - (dither->random & mask);

    dither->random = random;

    /* clip */
    if (output >= stats->peak_sample) 
    {
        if (output > MAX) 
        {
            ++stats->clipped_samples;

            if (output - MAX > stats->peak_clipping)
                stats->peak_clipping = output - MAX;

            output = MAX;

            if (sample > MAX)
                sample = MAX;
        }

        stats->peak_sample = output;
    }
    else if (output < -stats->peak_sample) 
    {
        if (output < MIN) 
        {
            ++stats->clipped_samples;

            if (MIN - output > stats->peak_clipping)
                stats->peak_clipping = MIN - output;

            output = MIN;

            if (sample < MIN)
                sample = MIN;
        }

        stats->peak_sample = -output;
    }

    /* quantize */
    output &= ~mask;

    /* error feedback */
    dither->error[0] = sample - output;

    /* scale */
    return output >> scalebits;
}

/*
 * NAME:	audio_pcm_s16le()
 * DESCRIPTION:	write a block of signed 16-bit little-endian PCM samples
 */
unsigned int audio_pcm_s16le(unsigned char *data, unsigned int nsamples,
			     mad_fixed_t const *left, mad_fixed_t const *right,
			     struct audio_stats *stats)
{
  unsigned int len;
  register signed int sample0, sample1;

  len = nsamples;

  if (right) {  /* stereo */
      while (len--) {
	sample0 = audio_linear_dither(16, *left++,  &left_dither,  stats);
	sample1 = audio_linear_dither(16, *right++, &right_dither, stats);

	data[0] = sample0 >> 0;
	data[1] = sample0 >> 8;
	data[2] = sample1 >> 0;
	data[3] = sample1 >> 8;

	data += 4;
      }

    return nsamples * 2 * 2;
  }
  else {  /* mono */
      while (len--) {
	sample0 = audio_linear_dither(16, *left++, &left_dither, stats);

	data[0] = sample0 >> 0;
	data[1] = sample0 >> 8;

	data += 2;
      }

    return nsamples * 2;
  }
}

/*
 * NAME:	audio_pcm_s16be()
 * DESCRIPTION:	write a block of signed 16-bit big-endian PCM samples
 */
unsigned int audio_pcm_s16be(unsigned char *data, unsigned int nsamples,
			     mad_fixed_t const *left, mad_fixed_t const *right,
			     enum audio_mode mode, struct audio_stats *stats)
{
  unsigned int len;
  register signed int sample0, sample1;

  len = nsamples;

  if (right) {  /* stereo */
      while (len--) {
	sample0 = audio_linear_dither(16, *left++,  &left_dither,  stats);
	sample1 = audio_linear_dither(16, *right++, &right_dither, stats);

	data[0] = sample0 >> 8;
	data[1] = sample0 >> 0;
	data[2] = sample1 >> 8;
	data[3] = sample1 >> 0;

	data += 4;
      }

    return nsamples * 2 * 2;
  }
  else {  /* mono */
      while (len--) {
	sample0 = audio_linear_dither(16, *left++, &left_dither, stats);

	data[0] = sample0 >> 8;
	data[1] = sample0 >> 0;

	data += 2;
      }

    return nsamples * 2;
  }
}

COpenALMp3Sample::COpenALMp3Sample()
{
}

COpenALMp3Sample::~COpenALMp3Sample()
{
}

void COpenALMp3Sample::Open(const char* filename)
{
	char abspath[MAX_PATH_LENGTH];

	// Open the file
	g_OpenALGameSystem.GetSoundPath(filename, abspath, sizeof(abspath));
	mp3File = filesystem->Open(abspath, "rb");

    if (mp3File == FILESYSTEM_INVALID_HANDLE)
	{
		Warning("Mp3: Could not open mp3 file: %s\n. Aborting.", filename);
		return;
	}

    // Reset member variables
    FrameCount = 0;
    freq = 0;
    hitEOF = false;

    // Reset input data
    Input.data = (unsigned char *)malloc(INPUT_BUFFER_SIZE);
    Input.length = 0;

    // Reset statistics
    Stats.clipped_samples = 0;
    Stats.peak_clipping = 0;
    Stats.peak_sample = 0;

    m_bFinished = false; // Sample has just started, assume not finished

	Init();
}

void COpenALMp3Sample::Close()
{
    filesystem->Close(mp3File);
	m_bReady = false;
	ClearBuffers();
}

bool COpenALMp3Sample::InitFormat()
{
    mad_stream_init(&Stream);
    mad_frame_init(&Frame);
    mad_synth_init(&Synth);
    mad_timer_reset(&Timer);

    return true;
}

#define READ_BUFFER_SIZE (INPUT_BUFFER_SIZE/2)

bool COpenALMp3Sample::CheckStream(ALuint buffer)
{
	if (!IsReady()) return false;

//     if ( filesystem->EndOfFile(mp3File) )
//         return false;

    char data[OPENAL_BUFFER_SIZE];
    //unsigned char InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD];
    //char ReadBuffer[READ_BUFFER_SIZE];

    char *OutputPtr = data;
    char *OutputPtrEnd = OutputPtr+(OPENAL_BUFFER_SIZE);
    unsigned char *GuardPtr = NULL;

    int nsamples = 0;

    int size = 0;

    while (size < OPENAL_BUFFER_SIZE)
    {
        // Fill the input buffer
        // This is only called if:
        // (1): The stream buffer is NULL, this is the first the loop is being run
        // (2): The stream buffer has been read, but there's a partial frame left at the end
        if (Stream.buffer == NULL || Stream.error == MAD_ERROR_BUFLEN )
        {
            if (hitEOF)
            {
                // We have decoded everything, now leave
                break;
            }

            // Number of bytes read by the file system
            int len = 0;

            if (Stream.next_frame != NULL)
            {
                // We have parts of a frame left in the buffer
                // Move the rest of the last frame over to the beginning of the input buffer

                // Input.length will be set to the amount of bytes added to Input.data
                memmove(Input.data, Stream.next_frame, Input.length = &Input.data[Input.length] - Stream.next_frame);
            } 

            // Execute the read operation, keeping the buffer size constant
            len = filesystem->Read(Input.data + Input.length, INPUT_BUFFER_SIZE - Input.length, mp3File);

            // If we've reached the end, guard the buffer
            if (filesystem->EndOfFile(mp3File))
            {
                // Notify the loop that this is the last buffer to be decoded
                hitEOF = true;

                // Because of the way libmad decodes mp3 frames, it may read a couple
                // of extra bits past the end of the file. To safely handle this, place 
                // a guard at the end of the buffer

                // Gives us the end of the buffer
                GuardPtr = Input.data + Input.length;

                // This prevents the decoder from reading bad memory
                memset(GuardPtr, 0, MAD_BUFFER_GUARD);

                // Add this to the buffer size, so it may be read as well
                len += MAD_BUFFER_GUARD;
            }

            if (len < 0)
            {
                // An error happened, take us out
                Warning("Mp3: Unable to read file\n");
                return false;
            }
            else if (len == 0)
            {
                // Nothing more to read

                // This should really never happen, because of the EOF notification.
                // However, we could be given a file that's basically empty,
                // so in that case we still need this check

                return false;
            }
    
            // Buffer the stream
            mad_stream_buffer(&Stream, Input.data, Input.length += len );

            // Reset errors before next decode
            Stream.error = MAD_ERROR_NONE;
        }

        // Decode the next mpeg frame
        if (mad_frame_decode(&Frame, &Stream))
        {
            if (MAD_RECOVERABLE(Stream.error))
            {
                // Something happened, not necessarily bad
                if (Stream.error != MAD_ERROR_LOSTSYNC || Stream.this_frame != GuardPtr)
                {
                    Msg("Mp3: Recoverable frame error: %s\n", MadErrorString(&Stream) );
                }
                continue;
            }
            else
            {
                // Do we need to read more into the buffer?
                if (Stream.error == MAD_ERROR_BUFLEN)
                {
                    // We end up here when there's something in the buffer, but
                    // not enough to read another frame.
                    continue;
                }
                else
                {
                    Warning("Mp3: Unrecoverable frame error: %s \n", MadErrorString(&Stream) );
                    break;
                }
            }
        }

        // Read header data from first frame
        if (FrameCount == 0)
        {
            // The data in the first header should be representable for the whole file

            mad_header *Header = &Frame.header;
            const char	*Layer,
				*Mode,
				*Emphasis;

	        /* Convert the layer number to it's printed representation. */
	        switch(Header->layer)
	        {
		        case MAD_LAYER_I:
			        Layer="I";
			        break;
		        case MAD_LAYER_II:
			        Layer="II";
			        break;
		        case MAD_LAYER_III:
			        Layer="III";
			        break;
		        default:
			        Layer="(unexpected layer value)";
			        break;
	        }

	        /* Convert the audio mode to it's printed representation. */
	        switch(Header->mode)
	        {
		        case MAD_MODE_SINGLE_CHANNEL:
			        Mode="single channel";
			        break;
		        case MAD_MODE_DUAL_CHANNEL:
			        Mode="dual channel";
			        break;
		        case MAD_MODE_JOINT_STEREO:
			        Mode="joint (MS/intensity) stereo";
			        break;
		        case MAD_MODE_STEREO:
			        Mode="normal LR stereo";
			        break;
		        default:
			        Mode="(unexpected mode value)";
			        break;
	        }

	        /* Convert the emphasis to it's printed representation. Note that
	         * the MAD_EMPHASIS_RESERVED enumeration value appeared in libmad
	         * version 0.15.0b.
	         */
	        switch(Header->emphasis)
	        {
		        case MAD_EMPHASIS_NONE:
			        Emphasis="no";
			        break;
		        case MAD_EMPHASIS_50_15_US:
			        Emphasis="50/15 us";
			        break;
		        case MAD_EMPHASIS_CCITT_J_17:
			        Emphasis="CCITT J.17";
			        break;
		        default:
			        Emphasis="(unexpected emphasis value)";
			        break;
	        }

            Msg("Mp3: %lu kb/s audio MPEG layer %s stream %s CRC, "
                "%s with %s emphasis at %d Hz sample rate\n",
                Header->bitrate/1000,Layer,
                Header->flags&MAD_FLAG_PROTECTION?"with":"without",
                Mode,Emphasis,Header->samplerate);

            // Pass the sample rate on to the BufferData() function
            freq = Header->samplerate;

            // Determine the format based on the number of channels specified in the header
            format = MAD_NCHANNELS(Header) == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;

            // Here we have the frame size in bytes
            //int Padding = Header->flags & MAD_FLAG_PADDING;
            //FrameSize = 144 * Header->bitrate / (Header->samplerate + Padding);
        }

        // Increment the frame count
        FrameCount++;

        mad_timer_add(&Timer,Frame.header.duration);

        // This is the stage where we would apply filtering
        //ApplyFilter();

        // Convert the frame into PCM samples
        mad_synth_frame(&Synth, &Frame);

        nsamples = Synth.pcm.length;    // Number of samples
        int len = nsamples;             // Also number of samples, but we use this to iterate

        mad_fixed_t const *left = Synth.pcm.samples[0];     // Left channel
        mad_fixed_t const *right = Synth.pcm.samples[1];    // Right channel

        register signed int sample0, sample1; // Store the pcm samples in a register

        if ( MAD_NCHANNELS(&Frame.header)==2 ) 
        {      
            /* stereo */
            while (len--) 
            {
                sample0 = audio_linear_dither(16, *left++,  &left_dither,  &Stats);
                sample1 = audio_linear_dither(16, *right++, &right_dither, &Stats);

                if (OutputPtr != OutputPtrEnd)
                {
                    OutputPtr[0] = sample0 >> 0;
                    OutputPtr[1] = sample0 >> 8;
                    OutputPtr[2] = sample1 >> 0;
                    OutputPtr[3] = sample1 >> 8;

                    // Move two samples ahead
                    OutputPtr += 4;
                }       
                else
                {
                    // No more room in output buffer

                    // This is also one of the cases that really should never happen,
                    // but you never know

                    continue;
                }
            }

            size += nsamples * 2 * 2;
        }
        else 
        {   
            /* mono */      
            while (len--) 
            {
                sample0 = audio_linear_dither(16, *left++, &left_dither, &Stats);

                if (OutputPtr != OutputPtrEnd)
                {
                    OutputPtr[0] = sample0 >> 0;
                    OutputPtr[1] = sample0 >> 8;

                    OutputPtr += 2;
                } 
                else
                {
                    // No more room in output buffer

                    // This is also one of the cases that really should never happen,
                    // but you never know

                    continue;
                }

                // Move to next sample
                OutputPtr += 2;
            }

            size += nsamples * 2;
        }

        // If there's not enough room for another frame, break the loop
        int remainingBufferSize = (OutputPtrEnd-OutputPtr); // How much more room is there?

        // This obviously assumes that we read the same amount of samples each time.
        // It may not be true, but it stays constant enough for this approach to be possible
        if ( remainingBufferSize < ( format == AL_FORMAT_STEREO16 ? (nsamples * 4) : (nsamples * 2) ) )
        {
            // We have decoded as far as we can in the buffer.
            // If we read one more frame now, we'd read more than we can put in the output buffer

            // Therefore we exit here
            break;
        }
    }

    // If we get to this point and the size is 0, then it means that we've finished
    // reading the file
    if (size == 0)
    {
        // Notify that we can be destroyed
        m_bFinished = true;

        // Output some interesting information about the mp3 file
        DevMsg("Mp3: Decoded a total of %i frames\nStats:\n-Clipped samples: %i\n-Peak clipping: %i\n-Peak Sample: %i\n", FrameCount, Stats.clipped_samples, Stats.peak_clipping, Stats.peak_sample);

        // Destroy the data we have used to decode this with
        mad_synth_finish(&Synth);
        mad_frame_finish(&Frame);
        mad_stream_finish(&Stream);

        // Free the input buffer to avoid leaking memory
        free(Input.data);

        // We're not active any more, so don't fill any new buffers
        return false;
    }

    // Send the output buffer to OpenAL
    BufferData(buffer, format, data, size, freq);

    // We have more stuff we can put in the buffer
	return true;
}

void COpenALMp3Sample::DestroyFormat()
{
}

COpenALMp3LoaderExt mp3Loader;

bool COpenALMp3LoaderExt::Init()
{
    g_OpenALLoader.Register(this, "mp3");

    return true;
}

void COpenALMp3LoaderExt::Shutdown()
{
    g_OpenALLoader.Deregister(this, "mp3");
}

IOpenALSample* COpenALMp3LoaderExt::Get()
{
    return new COpenALMp3Sample();
}