// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
// $Id: video_stream_instance.h,v 1.19 2007/09/07 22:24:41 strk Exp $

#ifndef GNASH_VIDEO_STREAM_INSTANCE_H
#define GNASH_VIDEO_STREAM_INSTANCE_H

#include "character.h" // for inheritance
#include "video_stream_def.h"
#include "embedVideoDecoder.h"
#include "snappingrange.h"

// Forward declarations
namespace gnash {
	class NetStream;
}

namespace gnash {

/// VideoStream ActionScript object
//
/// A VideoStream provides audio/video frames either
/// embedded into the SWF itself or loaded from the
/// network using an associated NetStream object.
///
class video_stream_instance : public character
{

public:

	boost::intrusive_ptr<video_stream_definition>	m_def;
	
	video_stream_instance(video_stream_definition* def,
			character* parent, int id);

	~video_stream_instance();

	geometry::Range2d<float> getBounds() const
	{
		// TODO: return the bounds of the dynamically
		//       loaded video if not embedded ?
		return m_def->get_bound().getRange();
	}

	virtual void	advance(float delta_time);
	void	display();

	// For sure isActionScriptReferenceable...
	bool wantsInstanceName()
	{
		return true; // text fields can be referenced 
	}	

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

	/// Set the input stream for this video
	void setStream(boost::intrusive_ptr<NetStream> ns);

protected:

#ifdef GNASH_USE_GC
	/// Mark video-specific reachable resources and invoke
	/// the parent's class version (markCharacterReachable)
	//
	/// video-specific reachable resources are:
	///	- Associated NetStream if any (_ns) 
	///
	virtual void markReachableResources() const;
#endif // GNASH_USE_GC

private:

	// m_video_source - A Camera object that is capturing video data or a NetStream object.
	// To drop the connection to the Video object, pass null for source.
	// FIXME: don't use as_object, but a more meaningful type
	//as_object* m_video_source;

	// Who owns this ? Should it be an intrusive ptr ?
	boost::intrusive_ptr<NetStream> _ns;

	/// Decoder for embedded video
	std::auto_ptr<embedVideoDecoder> m_decoder;
};

void video_class_init(as_object& global);

}	// end namespace gnash


#endif // GNASH_VIDEO_STREAM_INSTANCE_H
