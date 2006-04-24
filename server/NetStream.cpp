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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "NetStream.h"

namespace gnash {

NetStream::NetStream() {
}

NetStream::~NetStream() {
}


void
NetStream::close()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
NetStream::pause()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
NetStream::play()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
NetStream::seek()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
NetStream::setBufferTime()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void
netstream_new(const fn_call& fn)
{
    netstream_as_object *netstream_obj = new netstream_as_object;

    netstream_obj->set_member("close", &netstream_close);
    netstream_obj->set_member("pause", &netstream_pause);
    netstream_obj->set_member("play", &netstream_play);
    netstream_obj->set_member("seek", &netstream_seek);
    netstream_obj->set_member("setbuffertime", &netstream_setbuffertime);

    fn.result->set_as_object(netstream_obj);
}
void netstream_close(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void netstream_pause(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void netstream_play(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void netstream_seek(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void netstream_setbuffertime(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

