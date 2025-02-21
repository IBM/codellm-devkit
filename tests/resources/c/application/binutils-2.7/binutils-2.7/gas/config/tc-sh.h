/* This file is tc-sh.h

   Copyright (C) 1993, 94, 95, 1996 Free Software Foundation, Inc.

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
   the Free Software Foundation, 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#define TC_SH

/* This macro translates between an internal fix and an coff reloc type */
#define TC_COFF_FIX2RTYPE(fix) ((fix)->fx_r_type)

#define BFD_ARCH bfd_arch_sh

extern int shl;

#define COFF_MAGIC (shl ? SH_ARCH_MAGIC_LITTLE : SH_ARCH_MAGIC_BIG)

/* Whether -relax was used.  */
extern int sh_relax;

/* When relaxing, we need to generate relocations for alignment
   directives.  */
#define HANDLE_ALIGN(frag) sh_handle_align (frag)
extern void sh_handle_align PARAMS ((fragS *));

/* We need to force out some relocations when relaxing.  */
#define TC_FORCE_RELOCATION(fix) sh_force_relocation (fix)
extern int sh_force_relocation ();

/* We need to write out relocs which have not been completed.  */
#define TC_COUNT_RELOC(fix) ((fix)->fx_addsy != NULL)

#define TC_RELOC_MANGLE(seg, fix, int, paddr) \
  sh_coff_reloc_mangle ((seg), (fix), (int), (paddr))
extern void sh_coff_reloc_mangle ();

#define IGNORE_NONSTANDARD_ESCAPES

#define tc_coff_symbol_emit_hook(a) ; /* not used */

#define DO_NOT_STRIP 0
#define DO_STRIP 0
#define LISTING_HEADER (shl ? "Hitachi Super-H GAS Little Endian" : "Hitachi Super-H GAS Big Endian")
#define NEED_FX_R_TYPE 1
#define RELOC_32 1234

#define TC_KEEP_FX_OFFSET 1

#define TC_COFF_SIZEMACHDEP(frag) tc_coff_sizemachdep(frag)
extern int tc_coff_sizemachdep PARAMS ((fragS *));

#define md_operand(x)

extern const struct relax_type md_relax_table[];
#define TC_GENERIC_RELAX_TABLE md_relax_table

#define tc_frob_file sh_coff_frob_file
extern void sh_coff_frob_file PARAMS (());

/* We align most sections to a 16 byte boundary.  */
#define SUB_SEGMENT_ALIGN(SEG)					\
  (strncmp (obj_segment_name (SEG), ".stabstr", 8) == 0		\
   ? 0								\
   : ((strncmp (obj_segment_name (SEG), ".stab", 5) == 0	\
       || strcmp (obj_segment_name (SEG), ".ctors") == 0	\
       || strcmp (obj_segment_name (SEG), ".dtors") == 0)	\
      ? 2							\
      : 4))

/* We use a special alignment function to insert the correct nop
   pattern.  */
extern int sh_do_align PARAMS ((int, const char *, int));
#define md_do_align(n,fill,len,l) if (sh_do_align (n,fill,len)) goto l

/* We record, for each section, whether we have most recently output a
   CODE reloc or a DATA reloc.  */
struct sh_segment_info_type
{
  int in_code : 1;
};
#define TC_SEGMENT_INFO_TYPE struct sh_segment_info_type

/* We call a routine to emit a reloc for a label, so that the linker
   can align loads and stores without crossing a label.  */
extern void sh_frob_label PARAMS ((void));
#define tc_frob_label(sym) sh_frob_label ()

/* We call a routine to flush pending output in order to output a DATA
   reloc when required.  */
extern void sh_flush_pending_output PARAMS ((void));
#define md_flush_pending_output() sh_flush_pending_output ()

/* end of tc-sh.h */
