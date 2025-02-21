/* BFD library support routines for the Hitachi-SH architecture.
   Copyright (C) 1993 Free Software Foundation, Inc.
   Hacked by Steve Chamberlain of Cygnus Support.

This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"


int bfd_default_scan_num_mach();

static boolean 
scan_mach (info, string)
     const struct bfd_arch_info *info;
     const char *string;
{
  if (strcmp(string,"sh") == 0) return true;
  if (strcmp(string,"SH") == 0) return true;
  return false;
}


#if 0
/* This routine is provided two arch_infos and returns whether
   they'd be compatible */

static const bfd_arch_info_type *
compatible (a,b)
     const bfd_arch_info_type *a;
     const bfd_arch_info_type *b;
{
  if (a->arch != b->arch || a->mach != b->mach)
   return NULL;
  return a;
}
#endif

const bfd_arch_info_type bfd_sh_arch =
{
  32,				/* 32 bits in a word */
  32,				/* 32 bits in an address */
  8,				/* 8 bits in a byte */
  bfd_arch_sh,
  0,				/* only 1 machine */
  "sh",				/* arch_name  */
  "sh",				/* printable name */
  1,
  true,				/* the default machine */
  bfd_default_compatible,
  scan_mach,
  0,
};
