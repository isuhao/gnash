// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifndef GNASH_AS_PROP_FLAGS_H
#define GNASH_AS_PROP_FLAGS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include "container.h"

namespace gnash {

/// Flags defining the level of protection of a member
struct as_prop_flags
{
	/// Numeric flags
	int m_flags;

	/// if true, this value is protected (internal to gnash)
	bool m_is_protected;

	/// mask for flags
	static const int as_prop_flags_mask = 0x7;

	/// Default constructor
	as_prop_flags() : m_flags(0), m_is_protected(false)
	{
	}

	/// Constructor
	as_prop_flags(const bool read_only, const bool dont_delete, const bool dont_enum)
		:
		m_flags(((read_only) ? 0x4 : 0) | ((dont_delete) ? 0x2 : 0) | ((dont_enum) ? 0x1 : 0)),
		m_is_protected(false)
	{
	}

	/// Constructor, from numerical value
	as_prop_flags(const int flags)
		: m_flags(flags), m_is_protected(false)
	{
	}

	/// accessor to m_readOnly
	bool get_read_only() const { return (((this->m_flags & 0x4)!=0)?true:false); }

	/// accessor to m_dontDelete
	bool get_dont_delete() const { return (((this->m_flags & 0x2)!=0)?true:false); }

	/// accessor to m_dontEnum
	bool get_dont_enum() const { return (((this->m_flags & 0x1)!=0)?true:false);	}

	/// accesor to the numerical flags value
	int get_flags() const { return this->m_flags; }

	/// accessor to m_is_protected
	bool get_is_protected() const { return this->m_is_protected; }

	/// setter to m_is_protected
	void set_get_is_protected(const bool is_protected) { this->m_is_protected = is_protected; }

	/// set the numerical flags value (return the new value )
	/// If unlocked is false, you cannot un-protect from over-write,
	/// you cannot un-protect from deletion and you cannot
	/// un-hide from the for..in loop construct
	int set_flags(const int setTrue, const int set_false = 0)
	{
		if (!this->get_is_protected())
		{
			this->m_flags = this->m_flags & (~set_false);
			this->m_flags |= setTrue;
		}

		return get_flags();
	}
};



} // namespace gnash

#endif // GNASH_AS_PROP_FLAGS_H
