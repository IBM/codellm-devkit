/* BFD back-end for NetBSD/ns32k a.out-ish binaries.
   Copyright (C) 1990, 1991, 1992 Free Software Foundation, Inc.

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#define	BYTES_IN_WORD	4
#undef TARGET_IS_BIG_ENDIAN_P

#define	TARGET_PAGE_SIZE	4096
#define	SEGMENT_SIZE	4096

#define	DEFAULT_ARCH	bfd_arch_ns32k
#define MACHTYPE_OK(mtype) ((mtype) == M_532_NETBSD || (mtype) == M_UNKNOWN)

#define MY(OP) CAT(pc532netbsd_,OP)

#define NAME(x,y) CAT3(ns32kaout,_32_,y)

/* This needs to start with a.out so GDB knows it is an a.out variant.  */
#define TARGETNAME "a.out-ns32k-netbsd"

#define ns32kaout_32_get_section_contents aout_32_get_section_contents

#define MY_text_includes_header 1

/* We can`t use the MYNS macro here for cpp reasons too subtle
 * for me -- IWD
 */
#define MY_bfd_reloc_type_lookup ns32kaout_bfd_reloc_type_lookup

#include "bfd.h"		/* To ensure following declaration is OK */

CONST struct reloc_howto_struct *
MY_bfd_reloc_type_lookup
  PARAMS((bfd *abfd AND
	  bfd_reloc_code_real_type code));


#include "netbsd.h"
