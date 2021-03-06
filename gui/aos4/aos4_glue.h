//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
#ifndef __AOS4_GLUE_H__
#define __AOS4_GLUE_H__

#include <boost/cstdint.hpp> // for boost::?int??_t 

#include "Renderer.h"

namespace gnash
{

class AOS4Glue
{
  public:
    virtual ~AOS4Glue() { }
    virtual bool init(int argc, char **argv[]) = 0;

    virtual bool prepDrawingArea(int width, int height) = 0;
    virtual Renderer* createRenderHandler(int depth) = 0;
    virtual Renderer* createRenderHandler() = 0;
    virtual void render() = 0;
  protected:
    int _bpp;
};

} // namespace gnash
#endif //__AOS4_GLUE_H__
