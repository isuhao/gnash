// sound_handler_sdl.h: Sound handling using standard SDL
//
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


#ifndef SOUND_HANDLER_SDL_H
#define SOUND_HANDLER_SDL_H


#include "sound_handler.h" // for inheritance
#include "AudioDecoder.h"
#include "SimpleBuffer.h"

#include "log.h"

#include <vector>
#include <map> // for composition

#include <iconv.h>
#include <SDL_audio.h>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>


namespace gnash {
namespace sound {

class active_sound;

/// Used to hold the sounddata when doing on-demand-decoding
class sound_data
{
	/// The undecoded data
	std::auto_ptr<SimpleBuffer> _buf;

    void ensureBufferPadding();

public:

    /// Construct a sound with given data, info and volume.
    //
    /// @param data The encoded sound data. May be the NULL pointer for streaming sounds,
    ///     in which case data will be appended later using ::append()
    ///
    /// @param info encoding info
    ///
    /// @pararm nVolume initial volume (0..100). Optional, defaults to 100.
    ///
	sound_data(std::auto_ptr<SimpleBuffer> data, std::auto_ptr<media::SoundInfo> info, int nVolume=100);

	~sound_data();

	/// Object holding information about the sound
	std::auto_ptr<media::SoundInfo> soundinfo;

	std::map<boost::uint32_t,boost::uint32_t> m_frames_size;

	/// Append size bytes to this sound
	//
	/// @param data
	///	Data bytes, allocated with new[]. Ownership transferred.
	///
	/// @param size
	///	Size of the 'data' buffer.
	///
	void append(boost::uint8_t* data, unsigned int size);

	/// Return size of the data buffer
	size_t size() const 
	{
		return _buf->size();
	}

	/// Return a pointer to the underlying buffer
	const boost::uint8_t* data() const {
		return _buf->data();
	}

	/// Return a pointer to the underlying buffer
	boost::uint8_t* data() {
		return _buf->data();
	}

	/// Return a pointer to an offset in the underlying buffer
	//
	/// @param pos The offset value.
	/// 	An assertion will fail if pos > size()
	///
	const boost::uint8_t* data(size_t pos) const {
		assert(pos < _buf->size());
		return _buf->data()+pos;
	}

	/// Return a pointer to an offset in the underlying buffer
	//
	/// @param pos The offset value.
	/// 	An assertion will fail if pos > size()
	///
	boost::uint8_t* data(size_t pos) {
		assert(pos < _buf->size());
		return _buf->data()+pos;
	}

	/// Volume for AS-sounds, range: 0-100.
	/// It's the SWF range that is represented here.
	int volume;

	/// Vector containing the active instances of this sounds being played
	//
	/// NOTE: This class *owns* all active sounds
	///
	typedef std::list<active_sound*> ActiveSounds;

	ActiveSounds m_active_sounds;

	/// Drop all active sounds
	void clearActiveSounds();

	/// Drop an active sound (by iterator)
	//
	/// @return iterator after the one being erased
	///
	ActiveSounds::iterator eraseActiveSound(ActiveSounds::iterator i);
};

/// Used to hold the info about active sounds
//
/// This class contains a pointer to the sound_data used for playing
/// and an optional SimpleBuffer to use when decoding is needed.
///
/// When the SimpleBuffer is NULL we'll play the sound_data bytes directly
/// (we assume they are decoded already)
///
class active_sound
{
public:
	active_sound()
		:
		decoder(0),
		decodingPosition(0),
		playbackPosition(0),
		loopCount(0),
		offset(0),
		current_env(0),
		samples_played(0),
		_encodedData(0)
	{}

	~active_sound()
	{
		deleteDecodedData();
	}

	/// The decoder object used to convert the data into the playable format
	std::auto_ptr<media::AudioDecoder> decoder;

	/// Current decoding position in the encoded stream
	unsigned long decodingPosition;

	/// Current playback position in the decoded stream
	unsigned long playbackPosition;

	/// Numbers of loops: -1 means loop forever, 0 means play once.
	/// For every loop completed, it is decremented.
	long loopCount;

	/// Offset to make playback start in-sync, only used with mp3 streams.
	unsigned int offset;

	/// Sound envelopes for the current sound, which determine the volume level
	/// from a given position. Only used with sound events.
	const std::vector<sound_handler::sound_envelope>* envelopes;

	/// Index of current envelope.
	boost::uint32_t current_env;

	/// Number of samples played so far.
	unsigned long samples_played;

	/// Set the undecoded data pointer
	//
	/// @param newUndecodedData
	///	Pointer to a sound_data being the undecoded data
	/// 	Ownership will NOT be transferred.
	///
	void set_data(sound_data* newUndecodedData);

	/// Returns the data pointer in the encoded datastream
	/// for the given position. Boundaries are checked.
	boost::uint8_t* getEncodedData(unsigned long int pos);

	/// Returns the data pointer in the decoded datastream
	/// for the given position. Boundaries are checked.
	boost::uint8_t* getDecodedData(unsigned long int pos);

	/// Release resources associated with decoded data, if any.
	//
	/// After this call, the active_sound will have no decoded data
	/// buffer, thus any pointer to the decoded data will be fetched
	/// from the undecoded one.
	///
	void deleteDecodedData();

	/// Append size bytes to this raw data 
	//
	/// @param data
	///	Data bytes, allocated with new[]. Ownership transferred.
	///
	/// @param size
	///	Size of the 'data' buffer.
	///
	void appendDecodedData(boost::uint8_t* data, unsigned int size)
	{
		if ( ! _decodedData.get() )
		{
			_decodedData.reset( new SimpleBuffer );
		}
  
		_decodedData->append(data, size);
		delete [] data; // ownership transferred...
	}
  
	/// Set decoded data
	//
	/// @param data
	///	Data bytes, allocated with new[]. Ownership transferred.
	///
	/// @param size
	///	Size of the 'data' buffer.
	///
	void setDecodedData(boost::uint8_t* data, unsigned int size)
	{
		if ( ! _decodedData.get() )
		{
			_decodedData.reset( new SimpleBuffer() );
		}

		_decodedData->resize(0); // shouldn't release memory
		_decodedData->append(data, size);
		delete [] data; // ownership transferred...
	}

	size_t decodedDataSize() const
	{
		if ( _decodedData.get() )
		{
			return _decodedData->size();
		}
		else return 0;
	}
  
	size_t encodedDataSize() const
	{
		return _encodedData ? _encodedData->size() : 0;
	}
  
private:

	/// The encoded data
	sound_data* _encodedData;

	/// The decoded buffer
	//
	/// If NULL, the _encodedData will be considered
	/// decoded instead
	///
	std::auto_ptr<SimpleBuffer> _decodedData;

};

// This is here as it needs definition of active_sound
sound_data::~sound_data()
{
	clearActiveSounds();
}

// Use SDL and ffmpeg/mad/nothing to handle sounds.
class SDL_sound_handler : public sound_handler
{
public:

	typedef std::vector<sound_data*> Sounds;

private:
	/// AS classes (NetStream, Sound) audio callbacks
	typedef std::map< void* /* owner */, aux_streamer_ptr /* callback */> CallbacksMap;
	CallbacksMap m_aux_streamer;

	/// Vector containing all sounds.
	//
	/// Elemenst of the vector are owned by this class
	///
	Sounds	m_sound_data;

	/// Is sound device opened?
	bool soundOpened;

	/// The SDL_audio specs
	SDL_AudioSpec audioSpec;

	void initAudioSpec();
	
	/// Keeps track of numbers of playing sounds
	int soundsPlaying;

	/// Is the audio muted?
	bool muted;
	
	/// Mutex for making sure threads doesn't mess things up
	boost::mutex _mutex;

	// stop and delete all sounds
	void delete_all_sounds();

    /// File stream for dump file
	std::ofstream file_stream;

	// write a .WAV file header
	void write_wave_header(std::ofstream& outfile);

	void mixSoundData(sound_data& sounddata, Uint8* stream, unsigned int buffer_length);

	void mixActiveSound(active_sound& sound, sound_data& sounddata, Uint8* stream, unsigned int buffer_length);

	/// Callback invoked by the SDL audio thread.
	//
	/// Refills the output stream/buffer with data.
	///
	/// We run trough all the attached auxiliary streamers fetching decoded
	/// audio blocks and mixing them into the given output stream.
	///
	/// If sound is compresssed (mp3) a mp3-frame is decoded into a buffer,
	/// and resampled if needed. When the buffer has been sampled, another
	/// frame is decoded until all frames has been decoded.
	/// If a sound is looping it will be decoded from the beginning again.
	///
	/// TODO: make a static method of the SDL_sound_handler class
	///
	/// @param udata
	///	User data pointer (SDL_sound_handler instance in our case).
	///	We'll lock the SDL_sound_handler::_mutex during operations.
	///
	/// @param stream
	/// 	The output stream/buffer to fill
	///
	/// @param buffer_length_in
	///	Length of the buffer.
	///	If zero or negative we log an error and return
	///	(negative is probably an SDL bug, zero dunno yet).
	///
	static void sdl_audio_callback (void *udata, Uint8 *stream, int buffer_length_in);

public:

	SDL_sound_handler();

	SDL_sound_handler(const std::string& wave_file);

	~SDL_sound_handler();

	// See dox in sound_handler.h
	virtual int	create_sound(std::auto_ptr<SimpleBuffer> data, std::auto_ptr<media::SoundInfo> sinfo);

	// see dox in sound_handler.h
	virtual long	fill_stream_data(unsigned char* data, unsigned int data_bytes,
					 unsigned int sample_count, int handle_id);

	// See dox in sound_handler.h
	virtual void	play_sound(int sound_handle, int loopCount, int offset,
				   long start_position, const std::vector<sound_envelope>* envelopes);

	// See dox in sound_handler.h
	virtual void	stop_sound(int sound_handle);

	// See dox in sound_handler.h
	virtual void	delete_sound(int sound_handle);

	// See dox in sound_handler.h
	virtual void reset();

	// See dox in sound_handler.h
	virtual void	stop_all_sounds();

	// See dox in sound_handler.h
	virtual int	get_volume(int sound_handle);

	// See dox in sound_handler.h
	virtual void	set_volume(int sound_handle, int volume);
		
	// See dox in sound_handler.h
	virtual media::SoundInfo* get_sound_info(int sound_handle);

	// See dox in sound_handler.h
	virtual void	mute();

	// See dox in sound_handler.h
	virtual void	unmute();

	// See dox in sound_handler.h
	virtual bool	is_muted();

	// See dox in sound_handler.h
	virtual unsigned int get_duration(int sound_handle);

	// See dox in sound_handler.h
	virtual unsigned int tell(int sound_handle);
	
	// See dox in sound_handler.h
	virtual void	attach_aux_streamer(aux_streamer_ptr ptr, void* owner);

	// See dox in sound_handler.h
	virtual void	detach_aux_streamer(void* owner);

};

} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_HANDLER_SDL_H
