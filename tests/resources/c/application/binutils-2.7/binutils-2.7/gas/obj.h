/* obj.h - defines the object dependent hooks for all object
   format backends.

   Copyright (C) 1987, 1990, 1991, 1992 Free Software Foundation, Inc.

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

char *obj_default_output_file_name PARAMS ((void));
void obj_emit_relocations PARAMS ((char **where, fixS * fixP,
				   relax_addressT segment_address_in_file));
void obj_emit_strings PARAMS ((char **where));
void obj_emit_symbols PARAMS ((char **where, symbolS * symbols));
#ifndef obj_read_begin_hook
void obj_read_begin_hook PARAMS ((void));
#endif
#ifndef BFD_ASSEMBLER
void obj_crawl_symbol_chain PARAMS ((object_headers * headers));
void obj_header_append PARAMS ((char **where, object_headers * headers));
#ifndef obj_pre_write_hook
void obj_pre_write_hook PARAMS ((object_headers * headers));
#endif
#endif

#ifndef obj_symbol_new_hook
void obj_symbol_new_hook PARAMS ((symbolS * symbolP));
#endif

void obj_symbol_to_chars PARAMS ((char **where, symbolS * symbolP));

extern const pseudo_typeS obj_pseudo_table[];

#ifdef BFD_ASSEMBLER
struct format_ops {
  int flavor;
  unsigned dfl_leading_underscore : 1;
  unsigned emit_section_symbols : 1;
  void (*frob_symbol) PARAMS ((symbolS *, int *));
  void (*frob_file) PARAMS ((void));
  bfd_vma (*s_get_size) PARAMS ((symbolS *));
  void (*s_set_size) PARAMS ((symbolS *, bfd_vma));
  bfd_vma (*s_get_align) PARAMS ((symbolS *));
  void (*s_set_align) PARAMS ((symbolS *, bfd_vma));
  void (*copy_symbol_attributes) PARAMS ((symbolS *, symbolS *));
  void (*generate_asm_lineno) PARAMS ((const char *, int));
  void (*process_stab) PARAMS ((segT, int, const char *, int, int, int));
  int (*sec_sym_ok_for_reloc) PARAMS ((asection *));
  void (*pop_insert) PARAMS ((void));
  /* For configurations using ECOFF_DEBUGGING, this callback is used.  */
  void (*ecoff_set_ext) PARAMS ((symbolS *, struct ecoff_extr *));

  void (*read_begin_hook) PARAMS ((void));
  void (*symbol_new_hook) PARAMS ((symbolS *));
};

extern const struct format_ops elf_format_ops;
extern const struct format_ops ecoff_format_ops;

#ifndef this_format
COMMON const struct format_ops *this_format;
#endif
#endif

/* end of obj.h */
