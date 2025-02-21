/* read.h - of read.c
   Copyright (C) 1986, 90, 92, 93, 94, 95, 1996 Free Software Foundation, Inc.

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

extern char *input_line_pointer;/* -> char we are parsing now. */

#define PERMIT_WHITESPACE	/* Define to make whitespace be allowed in */
/* many syntactically unnecessary places. */
/* Normally undefined. For compatibility */
/* with ancient GNU cc. */
/* #undef PERMIT_WHITESPACE */

#ifdef PERMIT_WHITESPACE
#define SKIP_WHITESPACE() {if (* input_line_pointer == ' ') ++ input_line_pointer;}
#else
#define SKIP_WHITESPACE() know(*input_line_pointer != ' ' )
#endif


#define	LEX_NAME	(1)	/* may continue a name */
#define LEX_BEGIN_NAME	(2)	/* may begin a name */

#define is_name_beginner(c) \
  ( lex_type[(unsigned char) (c)] & LEX_BEGIN_NAME )
#define is_part_of_name(c) \
  ( lex_type[(unsigned char) (c)] & LEX_NAME       )

#ifndef is_a_char
#define CHAR_MASK	(0xff)
#define NOT_A_CHAR	(CHAR_MASK+1)
#define is_a_char(c)	(((unsigned)(c)) <= CHAR_MASK)
#endif /* is_a_char() */

extern char lex_type[];
extern char is_end_of_line[];

extern int is_it_end_of_statement PARAMS ((void));

extern int target_big_endian;

/* These are initialized by the CPU specific target files (tc-*.c).  */
extern const char comment_chars[];
extern const char line_comment_chars[];
extern const char line_separator_chars[];

/* This flag whether to generate line info for asm file */
extern int generate_asm_lineno;

/* The offset in the absolute section.  */
extern addressT abs_section_offset;

/* The label on a line, used by some of the pseudo-ops.  */
extern symbolS *line_label;

/* This is used to support MRI common sections.  */
extern symbolS *mri_common_symbol;

/* Possible arguments to .linkonce.  */
enum linkonce_type
{
  LINKONCE_UNSET = 0,
  LINKONCE_DISCARD,
  LINKONCE_ONE_ONLY,
  LINKONCE_SAME_SIZE,
  LINKONCE_SAME_CONTENTS
};

extern void pop_insert PARAMS ((const pseudo_typeS *));
extern unsigned int get_stab_string_offset
  PARAMS ((const char *string, const char *stabstr_secname));
extern char *demand_copy_C_string PARAMS ((int *len_pointer));
extern char get_absolute_expression_and_terminator
  PARAMS ((long *val_pointer));
extern offsetT get_absolute_expression PARAMS ((void));
extern unsigned int next_char_of_string PARAMS ((void));
extern void s_mri_sect PARAMS ((char *));
extern char *mri_comment_field PARAMS ((char *));
extern void mri_comment_end PARAMS ((char *, int));
extern void add_include_dir PARAMS ((char *path));
extern void cons PARAMS ((int nbytes));
extern void demand_empty_rest_of_line PARAMS ((void));
extern void emit_expr PARAMS ((expressionS *exp, unsigned int nbytes));
extern void equals PARAMS ((char *sym_name));
extern void float_cons PARAMS ((int float_type));
extern void ignore_rest_of_line PARAMS ((void));
extern void pseudo_set PARAMS ((symbolS * symbolP));
extern void read_a_source_file PARAMS ((char *name));
extern void read_begin PARAMS ((void));
extern void s_abort PARAMS ((int));
extern void s_align_bytes PARAMS ((int arg));
extern void s_align_ptwo PARAMS ((int));
extern void s_app_file PARAMS ((int));
extern void s_app_line PARAMS ((int));
extern void s_comm PARAMS ((int));
extern void s_data PARAMS ((int));
extern void s_desc PARAMS ((int));
extern void s_else PARAMS ((int arg));
extern void s_end PARAMS ((int arg));
extern void s_endif PARAMS ((int arg));
extern void s_err PARAMS ((int));
extern void s_fail PARAMS ((int));
extern void s_fill PARAMS ((int));
extern void s_float_space PARAMS ((int mult));
extern void s_globl PARAMS ((int arg));
extern void s_if PARAMS ((int arg));
extern void s_ifc PARAMS ((int arg));
extern void s_ifdef PARAMS ((int arg));
extern void s_ifeqs PARAMS ((int arg));
extern void s_ignore PARAMS ((int arg));
extern void s_include PARAMS ((int arg));
extern void s_irp PARAMS ((int arg));
extern void s_lcomm PARAMS ((int needs_align));
extern void s_linkonce PARAMS ((int));
extern void s_lsym PARAMS ((int));
extern void s_macro PARAMS ((int));
extern void s_mexit PARAMS ((int));
extern void s_mri PARAMS ((int));
extern void s_mri_common PARAMS ((int));
extern void s_org PARAMS ((int));
extern void s_print PARAMS ((int));
extern void s_purgem PARAMS ((int));
extern void s_rept PARAMS ((int));
extern void s_set PARAMS ((int));
extern void s_space PARAMS ((int mult));
extern void s_stab PARAMS ((int what));
extern void s_struct PARAMS ((int));
extern void s_text PARAMS ((int));
extern void stringer PARAMS ((int append_zero));
extern void s_xstab PARAMS ((int what));
extern void s_rva PARAMS ((int));
extern void read_print_statistics PARAMS ((FILE *));

/* end of read.h */
