// sound.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Code to handle SWF sound-related tags.


#include "sound_definition.h"
#include "gnash.h" // for get_sound_handler
#include "sound_handler.h" // for use

namespace gnash {


sound_sample::~sound_sample()
{
	sound::sound_handler* handler = get_sound_handler();
	if (handler)
	{
		handler->delete_sound(m_sound_handler_id);
	}
}

} // namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
