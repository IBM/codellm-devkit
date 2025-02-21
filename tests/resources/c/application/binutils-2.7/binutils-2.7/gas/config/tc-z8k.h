/* This file is tc-z8k.h

   Copyright (C) 1987-1992 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */


#define TC_Z8K
#define TARGET_BYTES_BIG_ENDIAN 1
#define LOCAL_LABEL(x) 0
/* This macro translates between an internal fix and an coff reloc type */
#define TC_COFF_FIX2RTYPE(fixP) abort();

#define BFD_ARCH bfd_arch_z8k
#define COFF_MAGIC 0x8000
#define TC_COUNT_RELOC(x) (1)
#define IGNORE_NONSTANDARD_ESCAPES

#define TC_RELOC_MANGLE(s,a,b,c) tc_reloc_mangle(a,b,c)

#define DO_NOT_STRIP 0
#define DO_STRIP 0
#define LISTING_HEADER "Zilog Z8000 GAS "
#define NEED_FX_R_TYPE 1
#define RELOC_32 1234

#define md_operand(x)

/* end of tc-z8k.h */
