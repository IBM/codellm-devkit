/* This file is tc-w65.h

   Copyright (C) 1995 Free Software Foundation, Inc.

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


#define TC_W65

/* This macro translates between an internal fix and an coff reloc type */
#define TC_COFF_FIX2RTYPE(fixP) tc_coff_fix2rtype(fixP)

#define BFD_ARCH bfd_arch_w65
#define COFF_MAGIC 0x6500

#define IGNORE_NONSTANDARD_ESCAPES

#define TC_RELOC_MANGLE(s,a,b,c) tc_reloc_mangle(a,b,c)

#define DO_NOT_STRIP 0
#define DO_STRIP 0
#define LISTING_HEADER "W65816 GAS "
#define NEED_FX_R_TYPE 1
#define RELOC_32 1234

#define TC_COFF_SIZEMACHDEP(frag) tc_coff_sizemachdep(frag)
#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) w65_expression (EXP, NBYTES)
#define TC_COUNT_RELOC(x) (1)
#define TC_CONS_RELOC tc_cons_reloc
#define DONT_OVERFLOW
int tc_cons_reloc;

#define md_operand(x)

extern struct relax_type md_relax_table[];
#define TC_GENERIC_RELAX_TABLE md_relax_table

/* end of tc-w65.h */
