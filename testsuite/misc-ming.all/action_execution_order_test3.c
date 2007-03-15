/*
 *   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 

/*
 * Zou Lunkai, zoulunkai@gmail.com
 *
 * Within the same frame, if PlaceObject is after DoAction, then OnLoad is also 
 * called after executing actionscripts in DoAction; if RemoveObject is after DoAction, 
 * then OnUnload is also called after executing actionscripts in DoAction.
 *
 * The actual order of tags are dependent on compiler, so you need to 
 * verify first if the order of tags is what you expect.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "action_execution_order_test3.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc_red1, mc_red2, mc_red3, dejagnuclip;
  SWFShape  sh_red;

  const char *srcdir=".";
  if ( argc>1 ) 
    srcdir=argv[1];
  else
  {
      fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
      return 1;
  }

  Ming_init();
  mo = newSWFMovieWithVersion(OUTPUT_VERSION);
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate (mo, 12.0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); /* 1st frame */

  add_actions(mo, "_root.x1 += \"as_in_DoAction+\"; ");
  
  mc_red1 = newSWFMovieClip();
  sh_red = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red1, (SWFBlock)sh_red);  
  SWFMovieClip_nextFrame(mc_red1); /* mc_red1, 1st frame */
   
  /* add mc_red1 to _root and name it as "mc_red1" */
  SWFDisplayItem it_red1, it_red2, it_red3;
  it_red1 = SWFMovie_add(mo, (SWFBlock)mc_red1);  
  SWFDisplayItem_setDepth(it_red1, 10); 
  SWFDisplayItem_addAction(it_red1,
    compileSWFActionCode("_root.x1 += \"onLoad+\";"),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it_red1,
    compileSWFActionCode("_root.x1 += \"onUnload+\";"),
    SWFACTION_UNLOAD);
  SWFDisplayItem_setName(it_red1, "mc_red1"); 

  SWFMovie_nextFrame(mo); /* 2nd frame */

  add_actions(mo, "_root.x1 += \"as_in_DoAction+\"; ");
  SWFDisplayItem_remove(it_red1);
  SWFMovie_nextFrame(mo); /* 3th frame */
  
  check_equals(mo, "_root.x1", "'as_in_DoAction+onLoad+as_in_DoAction+onUnload+'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 4th frame */

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



