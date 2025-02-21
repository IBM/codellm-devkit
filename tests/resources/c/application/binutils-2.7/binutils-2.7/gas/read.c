/* read.c - read a source file -
   Copyright (C) 1986, 87, 90, 91, 92, 93, 94, 95, 1996
   Free Software Foundation, Inc.

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
the Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#if 0
#define MASK_CHAR (0xFF)	/* If your chars aren't 8 bits, you will
				   change this a bit.  But then, GNU isn't
				   spozed to run on your machine anyway.
				   (RMS is so shortsighted sometimes.)
				   */
#else
#define MASK_CHAR ((int)(unsigned char)-1)
#endif


/* This is the largest known floating point format (for now). It will
   grow when we do 4361 style flonums. */

#define MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT (16)

/* Routines that read assembler source text to build spagetti in memory.
   Another group of these functions is in the expr.c module.  */

/* for isdigit() */
#include <ctype.h>

#include "as.h"
#include "subsegs.h"
#include "sb.h"
#include "macro.h"
#include "libiberty.h"
#include "obstack.h"
#include "listing.h"
#include "ecoff.h"

#ifndef TC_START_LABEL
#define TC_START_LABEL(x,y) (x==':')
#endif

/* The NOP_OPCODE is for the alignment fill value.
 * fill it a nop instruction so that the disassembler does not choke
 * on it
 */
#ifndef NOP_OPCODE
#define NOP_OPCODE 0x00
#endif

char *input_line_pointer;	/*->next char of source file to parse. */

int generate_asm_lineno = 0;	/* flag to generate line stab for .s file */

#if BITS_PER_CHAR != 8
/*  The following table is indexed by[(char)] and will break if
    a char does not have exactly 256 states (hopefully 0:255!)!  */
die horribly;
#endif

#ifndef LEX_AT
/* The m88k unfortunately uses @ as a label beginner.  */
#define LEX_AT 0
#endif

#ifndef LEX_BR
/* The RS/6000 assembler uses {,},[,] as parts of symbol names.  */
#define LEX_BR 0
#endif

#ifndef LEX_PCT
/* The Delta 68k assembler permits % inside label names.  */
#define LEX_PCT 0
#endif

#ifndef LEX_QM
/* The PowerPC Windows NT assemblers permits ? inside label names.  */
#define LEX_QM 0
#endif

#ifndef LEX_DOLLAR
/* The a29k assembler does not permits labels to start with $.  */
#define LEX_DOLLAR 3
#endif

/* used by is_... macros. our ctype[] */
char lex_type[256] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* @ABCDEFGHIJKLMNO */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* PQRSTUVWXYZ[\]^_ */
  0, 0, 0, 0, LEX_DOLLAR, LEX_PCT, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, /* _!"#$%&'()*+,-./ */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, LEX_QM,	/* 0123456789:;<=>? */
  LEX_AT, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* @ABCDEFGHIJKLMNO */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, LEX_BR, 0, LEX_BR, 0, 3, /* PQRSTUVWXYZ[\]^_ */
  0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* `abcdefghijklmno */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, LEX_BR, 0, LEX_BR, 0, 0, /* pqrstuvwxyz{|}~. */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


/*
 * In: a character.
 * Out: 1 if this character ends a line.
 */
#define _ (0)
char is_end_of_line[256] =
{
#ifdef CR_EOL
  _, _, _, _, _, _, _, _, _, _, 99, _, _, 99, _, _,	/* @abcdefghijklmno */
#else
  _, _, _, _, _, _, _, _, _, _, 99, _, _, _, _, _,	/* @abcdefghijklmno */
#endif
  _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* */
#ifdef TC_HPPA
  _,99, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* _!"#$%&'()*+,-./ */
  _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* 0123456789:;<=>? */
#else
  _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* */
  _, _, _, _, _, _, _, _, _, _, _, 99, _, _, _, _,	/* 0123456789:;<=>? */
#endif
  _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* */
  _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* */
  _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* */
  _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* */
  _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* */
  _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* */
  _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* */
  _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* */
  _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,	/* */
};
#undef _

/* Functions private to this file. */

static char *buffer;	/* 1st char of each buffer of lines is here. */
static char *buffer_limit;	/*->1 + last char in buffer. */

#ifdef TARGET_BYTES_BIG_ENDIAN
/* Hack to deal with tc-*.h defining TARGET_BYTES_BIG_ENDIAN to empty
   instead of to 0 or 1.  */
#if 5 - TARGET_BYTES_BIG_ENDIAN - 5 == 10
#undef  TARGET_BYTES_BIG_ENDIAN
#define TARGET_BYTES_BIG_ENDIAN 1
#endif
int target_big_endian = TARGET_BYTES_BIG_ENDIAN;
#else
int target_big_endian /* = 0 */;
#endif

static char *old_buffer;	/* JF a hack */
static char *old_input;
static char *old_limit;

/* Variables for handling include file directory list. */

char **include_dirs;	/* List of pointers to directories to
			   search for .include's */
int include_dir_count;	/* How many are in the list */
int include_dir_maxlen = 1;/* Length of longest in list */

#ifndef WORKING_DOT_WORD
struct broken_word *broken_words;
int new_broken_words;
#endif

/* The current offset into the absolute section.  We don't try to
   build frags in the absolute section, since no data can be stored
   there.  We just keep track of the current offset.  */
addressT abs_section_offset;

/* If this line had an MRI style label, it is stored in this variable.
   This is used by some of the MRI pseudo-ops.  */
symbolS *line_label;

/* This global variable is used to support MRI common sections.  We
   translate such sections into a common symbol.  This variable is
   non-NULL when we are in an MRI common section.  */
symbolS *mri_common_symbol;

/* In MRI mode, after a dc.b pseudo-op with an odd number of bytes, we
   need to align to an even byte boundary unless the next pseudo-op is
   dc.b, ds.b, or dcb.b.  This variable is set to 1 if an alignment
   may be needed.  */
static int mri_pending_align;

static int scrub_from_string PARAMS ((char **));
static void do_align PARAMS ((int, char *, int));
static int hex_float PARAMS ((int, char *));
static void do_org PARAMS ((segT, expressionS *, int));
char *demand_copy_string PARAMS ((int *lenP));
static segT get_segmented_expression PARAMS ((expressionS *expP));
static segT get_known_segmented_expression PARAMS ((expressionS * expP));
static void pobegin PARAMS ((void));
static int get_line_sb PARAMS ((sb *));


void
read_begin ()
{
  const char *p;

  pobegin ();
  obj_read_begin_hook ();

  /* Something close -- but not too close -- to a multiple of 1024.
     The debugging malloc I'm using has 24 bytes of overhead.  */
  obstack_begin (&notes, chunksize);
  obstack_begin (&cond_obstack, chunksize);

  /* Use machine dependent syntax */
  for (p = line_separator_chars; *p; p++)
    is_end_of_line[(unsigned char) *p] = 1;
  /* Use more.  FIXME-SOMEDAY. */

  if (flag_mri)
    lex_type['?'] = 3;
}

/* set up pseudo-op tables */

static struct hash_control *po_hash;

static const pseudo_typeS potable[] =
{
  {"abort", s_abort, 0},
  {"align", s_align_ptwo, 0},
  {"ascii", stringer, 0},
  {"asciz", stringer, 1},
  {"balign", s_align_bytes, 0},
  {"balignw", s_align_bytes, -2},
  {"balignl", s_align_bytes, -4},
/* block */
  {"byte", cons, 1},
  {"comm", s_comm, 0},
  {"common", s_mri_common, 0},
  {"common.s", s_mri_common, 1},
  {"data", s_data, 0},
  {"dc", cons, 2},
  {"dc.b", cons, 1},
  {"dc.d", float_cons, 'd'},
  {"dc.l", cons, 4},
  {"dc.s", float_cons, 'f'},
  {"dc.w", cons, 2},
  {"dc.x", float_cons, 'x'},
  {"dcb", s_space, 2},
  {"dcb.b", s_space, 1},
  {"dcb.d", s_float_space, 'd'},
  {"dcb.l", s_space, 4},
  {"dcb.s", s_float_space, 'f'},
  {"dcb.w", s_space, 2},
  {"dcb.x", s_float_space, 'x'},
  {"ds", s_space, 2},
  {"ds.b", s_space, 1},
  {"ds.d", s_space, 8},
  {"ds.l", s_space, 4},
  {"ds.p", s_space, 12},
  {"ds.s", s_space, 4},
  {"ds.w", s_space, 2},
  {"ds.x", s_space, 12},
  {"debug", s_ignore, 0},
#ifdef S_SET_DESC
  {"desc", s_desc, 0},
#endif
/* dim */
  {"double", float_cons, 'd'},
/* dsect */
  {"eject", listing_eject, 0},	/* Formfeed listing */
  {"else", s_else, 0},
  {"elsec", s_else, 0},
  {"end", s_end, 0},
  {"endc", s_endif, 0},
  {"endif", s_endif, 0},
/* endef */
  {"equ", s_set, 0},
  {"err", s_err, 0},
  {"exitm", s_mexit, 0},
/* extend */
  {"extern", s_ignore, 0},	/* We treat all undef as ext */
  {"appfile", s_app_file, 1},
  {"appline", s_app_line, 0},
  {"fail", s_fail, 0},
  {"file", s_app_file, 0},
  {"fill", s_fill, 0},
  {"float", float_cons, 'f'},
  {"format", s_ignore, 0},
  {"global", s_globl, 0},
  {"globl", s_globl, 0},
  {"hword", cons, 2},
  {"if", s_if, (int) O_ne},
  {"ifc", s_ifc, 0},
  {"ifdef", s_ifdef, 0},
  {"ifeq", s_if, (int) O_eq},
  {"ifeqs", s_ifeqs, 0},
  {"ifge", s_if, (int) O_ge},
  {"ifgt", s_if, (int) O_gt},
  {"ifle", s_if, (int) O_le},
  {"iflt", s_if, (int) O_lt},
  {"ifnc", s_ifc, 1},
  {"ifndef", s_ifdef, 1},
  {"ifne", s_if, (int) O_ne},
  {"ifnes", s_ifeqs, 1},
  {"ifnotdef", s_ifdef, 1},
  {"include", s_include, 0},
  {"int", cons, 4},
  {"irp", s_irp, 0},
  {"irep", s_irp, 0},
  {"irpc", s_irp, 1},
  {"irepc", s_irp, 1},
  {"lcomm", s_lcomm, 0},
  {"lflags", listing_flags, 0},	/* Listing flags */
  {"linkonce", s_linkonce, 0},
  {"list", listing_list, 1},	/* Turn listing on */
  {"llen", listing_psize, 1},
  {"long", cons, 4},
  {"lsym", s_lsym, 0},
  {"macro", s_macro, 0},
  {"mexit", s_mexit, 0},
  {"mri", s_mri, 0},
  {".mri", s_mri, 0},	/* Special case so .mri works in MRI mode.  */
  {"name", s_ignore, 0},
  {"noformat", s_ignore, 0},
  {"nolist", listing_list, 0},	/* Turn listing off */
  {"nopage", listing_nopage, 0},
  {"octa", cons, 16},
  {"offset", s_struct, 0},
  {"org", s_org, 0},
  {"p2align", s_align_ptwo, 0},
  {"p2alignw", s_align_ptwo, -2},
  {"p2alignl", s_align_ptwo, -4},
  {"page", listing_eject, 0},
  {"plen", listing_psize, 0},
  {"print", s_print, 0},
  {"psize", listing_psize, 0},	/* set paper size */
  {"purgem", s_purgem, 0},
  {"quad", cons, 8},
  {"rep", s_rept, 0},
  {"rept", s_rept, 0},
  {"rva", s_rva, 4},
  {"sbttl", listing_title, 1},	/* Subtitle of listing */
/* scl */
/* sect */
  {"set", s_set, 0},
  {"short", cons, 2},
  {"single", float_cons, 'f'},
/* size */
  {"space", s_space, 0},
  {"skip", s_space, 0},
  {"spc", s_ignore, 0},
  {"stabd", s_stab, 'd'},
  {"stabn", s_stab, 'n'},
  {"stabs", s_stab, 's'},
  {"string", stringer, 1},
  {"struct", s_struct, 0},
/* tag */
  {"text", s_text, 0},

  /* This is for gcc to use.  It's only just been added (2/94), so gcc
     won't be able to use it for a while -- probably a year or more.
     But once this has been released, check with gcc maintainers
     before deleting it or even changing the spelling.  */
  {"this_GCC_requires_the_GNU_assembler", s_ignore, 0},
  /* If we're folding case -- done for some targets, not necessarily
     all -- the above string in an input file will be converted to
     this one.  Match it either way...  */
  {"this_gcc_requires_the_gnu_assembler", s_ignore, 0},

  {"title", listing_title, 0},	/* Listing title */
  {"ttl", listing_title, 0},
/* type */
/* use */
/* val */
  {"xcom", s_comm, 0},
  {"xdef", s_globl, 0},
  {"xref", s_ignore, 0},
  {"xstabs", s_xstab, 's'},
  {"word", cons, 2},
  {"zero", s_space, 0},
  {NULL}			/* end sentinel */
};

static int pop_override_ok = 0;
static const char *pop_table_name;

void
pop_insert (table)
     const pseudo_typeS *table;
{
  const char *errtxt;
  const pseudo_typeS *pop;
  for (pop = table; pop->poc_name; pop++)
    {
      errtxt = hash_insert (po_hash, pop->poc_name, (char *) pop);
      if (errtxt && (!pop_override_ok || strcmp (errtxt, "exists")))
	as_fatal ("error constructing %s pseudo-op table: %s", pop_table_name,
		  errtxt);
    }
}

#ifndef md_pop_insert
#define md_pop_insert()		pop_insert(md_pseudo_table)
#endif

#ifndef obj_pop_insert
#define obj_pop_insert()	pop_insert(obj_pseudo_table)
#endif

static void 
pobegin ()
{
  po_hash = hash_new ();

  /* Do the target-specific pseudo ops. */
  pop_table_name = "md";
  md_pop_insert ();

  /* Now object specific.  Skip any that were in the target table. */
  pop_table_name = "obj";
  pop_override_ok = 1;
  obj_pop_insert ();

  /* Now portable ones.  Skip any that we've seen already. */
  pop_table_name = "standard";
  pop_insert (potable);
}

#define HANDLE_CONDITIONAL_ASSEMBLY()					\
  if (ignore_input ())							\
    {									\
      while (! is_end_of_line[(unsigned char) *input_line_pointer++])	\
	if (input_line_pointer == buffer_limit)				\
	  break;							\
      continue;								\
    }


/* This function is used when scrubbing the characters between #APP
   and #NO_APP.  */

static char *scrub_string;
static char *scrub_string_end;

static int
scrub_from_string (from)
     char **from;
{
  int size;

  *from = scrub_string;
  size = scrub_string_end - scrub_string;
  scrub_string = scrub_string_end;
  return size;
}

/*	read_a_source_file()
 *
 * We read the file, putting things into a web that
 * represents what we have been reading.
 */
void 
read_a_source_file (name)
     char *name;
{
  register char c;
  register char *s;		/* string of symbol, '\0' appended */
  register int temp;
  pseudo_typeS *pop;

  buffer = input_scrub_new_file (name);

  listing_file (name);
  listing_newline ("");

  while ((buffer_limit = input_scrub_next_buffer (&input_line_pointer)) != 0)
    {				/* We have another line to parse. */
      know (buffer_limit[-1] == '\n');	/* Must have a sentinel. */
    contin:			/* JF this goto is my fault I admit it.
				   Someone brave please re-write the whole
				   input section here?  Pleeze???  */
      while (input_line_pointer < buffer_limit)
	{
	  /* We have more of this buffer to parse. */

	  /*
	   * We now have input_line_pointer->1st char of next line.
	   * If input_line_pointer [-1] == '\n' then we just
	   * scanned another line: so bump line counters.
	   */
	  if (is_end_of_line[(unsigned char) input_line_pointer[-1]])
	    {
#ifdef md_start_line_hook
	      md_start_line_hook ();
#endif

	      if (input_line_pointer[-1] == '\n')
		bump_line_counters ();

	      line_label = NULL;

	      if (flag_m68k_mri
#ifdef LABELS_WITHOUT_COLONS
		  || 1
#endif
		  )
		{
		  /* Text at the start of a line must be a label, we
		     run down and stick a colon in.  */
		  if (is_name_beginner (*input_line_pointer))
		    {
		      char *line_start = input_line_pointer;
		      char c;

		      HANDLE_CONDITIONAL_ASSEMBLY ();

		      c = get_symbol_end ();

		      /* In MRI mode, the EQU pseudoop must be
			 handled specially.  */
		      if (flag_m68k_mri)
			{
			  char *rest = input_line_pointer + 1;

			  if (*rest == ':')
			    ++rest;
			  if (*rest == ' ' || *rest == '\t')
			    ++rest;
			  if ((strncasecmp (rest, "EQU", 3) == 0
			       || strncasecmp (rest, "SET", 3) == 0)
			      && (rest[3] == ' ' || rest[3] == '\t'))
			    {
			      input_line_pointer = rest + 3;
			      equals (line_start);
			      continue;
			    }
			}

		      line_label = colon (line_start);

		      *input_line_pointer = c;
		      if (c == ':')
			input_line_pointer++;
		    }
		}
	    }

	  /*
	   * We are at the begining of a line, or similar place.
	   * We expect a well-formed assembler statement.
	   * A "symbol-name:" is a statement.
	   *
	   * Depending on what compiler is used, the order of these tests
	   * may vary to catch most common case 1st.
	   * Each test is independent of all other tests at the (top) level.
	   * PLEASE make a compiler that doesn't use this assembler.
	   * It is crufty to waste a compiler's time encoding things for this
	   * assembler, which then wastes more time decoding it.
	   * (And communicating via (linear) files is silly!
	   * If you must pass stuff, please pass a tree!)
	   */
	  if ((c = *input_line_pointer++) == '\t'
	      || c == ' '
	      || c == '\f'
	      || c == 0)
	    {
	      c = *input_line_pointer++;
	    }
	  know (c != ' ');	/* No further leading whitespace. */
	  LISTING_NEWLINE ();
	  /*
	   * C is the 1st significant character.
	   * Input_line_pointer points after that character.
	   */
	  if (is_name_beginner (c))
	    {
	      /* want user-defined label or pseudo/opcode */
	      HANDLE_CONDITIONAL_ASSEMBLY ();

	      s = --input_line_pointer;
	      c = get_symbol_end ();	/* name's delimiter */
	      /*
	       * C is character after symbol.
	       * That character's place in the input line is now '\0'.
	       * S points to the beginning of the symbol.
	       *   [In case of pseudo-op, s->'.'.]
	       * Input_line_pointer->'\0' where c was.
	       */
	      if (TC_START_LABEL(c, input_line_pointer))
		{
		  if (flag_m68k_mri)
		    {
		      char *rest = input_line_pointer + 1;

		      /* In MRI mode, \tsym: set 0 is permitted.  */

		      if (*rest == ':')
			++rest;
		      if (*rest == ' ' || *rest == '\t')
			++rest;
		      if ((strncasecmp (rest, "EQU", 3) == 0
			   || strncasecmp (rest, "SET", 3) == 0)
			  && (rest[3] == ' ' || rest[3] == '\t'))
			{
			  input_line_pointer = rest + 3;
			  equals (s);
			  continue;
			}
		    }

		  line_label = colon (s);	/* user-defined label */
		  *input_line_pointer++ = ':';	/* Put ':' back for error messages' sake. */
		  /* Input_line_pointer->after ':'. */
		  SKIP_WHITESPACE ();


		}
	      else if (c == '='
		       || (input_line_pointer[1] == '='
#ifdef TC_EQUAL_IN_INSN
			   && ! TC_EQUAL_IN_INSN (c, input_line_pointer)
#endif
			   ))
		{
		  equals (s);
		  demand_empty_rest_of_line ();
		}
	      else
		{		/* expect pseudo-op or machine instruction */
		  pop = NULL;

#define IGNORE_OPCODE_CASE
#ifdef IGNORE_OPCODE_CASE
		  {
		    char *s2 = s;
		    while (*s2)
		      {
			if (isupper (*s2))
			  *s2 = tolower (*s2);
			s2++;
		      }
		  }
#endif

		  if (flag_m68k_mri
#ifdef NO_PSEUDO_DOT
		      || 1
#endif
		      )
		    {
		      /* The MRI assembler and the m88k use pseudo-ops
                         without a period.  */
		      pop = (pseudo_typeS *) hash_find (po_hash, s);
		      if (pop != NULL && pop->poc_handler == NULL)
			pop = NULL;
		    }

		  if (pop != NULL
		      || (! flag_m68k_mri && *s == '.'))
		    {
		      /*
		       * PSEUDO - OP.
		       *
		       * WARNING: c has next char, which may be end-of-line.
		       * We lookup the pseudo-op table with s+1 because we
		       * already know that the pseudo-op begins with a '.'.
		       */

		      if (pop == NULL)
			pop = (pseudo_typeS *) hash_find (po_hash, s + 1);

		      /* In MRI mode, we may need to insert an
                         automatic alignment directive.  What a hack
                         this is.  */
		      if (mri_pending_align
			  && (pop == NULL
			      || ! ((pop->poc_handler == cons
				     && pop->poc_val == 1)
				    || (pop->poc_handler == s_space
					&& pop->poc_val == 1))))
			{
			  do_align (1, (char *) NULL, 0);
			  mri_pending_align = 0;
			}

		      /* Print the error msg now, while we still can */
		      if (pop == NULL)
			{
			  as_bad ("Unknown pseudo-op:  `%s'", s);
			  *input_line_pointer = c;
			  s_ignore (0);
			  continue;
			}

		      /* Put it back for error messages etc. */
		      *input_line_pointer = c;
		      /* The following skip of whitespace is compulsory.
			 A well shaped space is sometimes all that separates
			 keyword from operands. */
		      if (c == ' ' || c == '\t')
			input_line_pointer++;
		      /*
		       * Input_line is restored.
		       * Input_line_pointer->1st non-blank char
		       * after pseudo-operation.
		       */
		      (*pop->poc_handler) (pop->poc_val);

		      /* If that was .end, just get out now.  */
		      if (pop->poc_handler == s_end)
			goto quit;
		    }
		  else
		    {		/* machine instruction */
		      int inquote = 0;

		      if (mri_pending_align)
			{
			  do_align (1, (char *) NULL, 0);
			  mri_pending_align = 0;
			}

		      /* WARNING: c has char, which may be end-of-line. */
		      /* Also: input_line_pointer->`\0` where c was. */
		      *input_line_pointer = c;
		      while (!is_end_of_line[(unsigned char) *input_line_pointer]
			     || inquote
#ifdef TC_EOL_IN_INSN
			     || TC_EOL_IN_INSN (input_line_pointer)
#endif
			     )
			{
			  if (flag_m68k_mri && *input_line_pointer == '\'')
			    inquote = ! inquote;
			  input_line_pointer++;
			}

		      c = *input_line_pointer;
		      *input_line_pointer = '\0';

#ifdef OBJ_GENERATE_ASM_LINENO
		      if (generate_asm_lineno == 0)
			{
		          if (ecoff_no_current_file ())
			    generate_asm_lineno = 1;
			}
		      if (generate_asm_lineno == 1)
		        {
			  unsigned int lineno;
			  char *s;

			  as_where (&s, &lineno);
			  OBJ_GENERATE_ASM_LINENO (s, lineno);
		        }
#endif

		      if (macro_defined)
			{
			  sb out;
			  const char *err;

			  if (check_macro (s, &out, '\0', &err))
			    {
			      if (err != NULL)
				as_bad (err);
			      *input_line_pointer++ = c;
			      input_scrub_include_sb (&out,
						      input_line_pointer);
			      sb_kill (&out);
			      buffer_limit =
				input_scrub_next_buffer (&input_line_pointer);
			      continue;
			    }
			}

		      md_assemble (s);	/* Assemble 1 instruction. */

		      *input_line_pointer++ = c;

		      /* We resume loop AFTER the end-of-line from
			 this instruction. */
		    }		/* if (*s=='.') */
		}		/* if c==':' */
	      continue;
	    }			/* if (is_name_beginner(c) */


	  /* Empty statement?  */
	  if (is_end_of_line[(unsigned char) c])
	    continue;

	  if ((LOCAL_LABELS_DOLLAR || LOCAL_LABELS_FB)
	      && isdigit (c))
	    {
	      /* local label  ("4:") */
	      char *backup = input_line_pointer;

	      HANDLE_CONDITIONAL_ASSEMBLY ();

	      temp = c - '0';

	      while (isdigit (*input_line_pointer))
		{
		  temp = (temp * 10) + *input_line_pointer - '0';
		  ++input_line_pointer;
		}		/* read the whole number */

	      if (LOCAL_LABELS_DOLLAR
		  && *input_line_pointer == '$'
		  && *(input_line_pointer + 1) == ':')
		{
		  input_line_pointer += 2;

		  if (dollar_label_defined (temp))
		    {
		      as_fatal ("label \"%d$\" redefined", temp);
		    }

		  define_dollar_label (temp);
		  colon (dollar_label_name (temp, 0));
		  continue;
		}

	      if (LOCAL_LABELS_FB
		  && *input_line_pointer++ == ':')
		{
		  fb_label_instance_inc (temp);
		  colon (fb_label_name (temp, 0));
		  continue;
		}

	      input_line_pointer = backup;
	    }			/* local label  ("4:") */

	  if (c && strchr (line_comment_chars, c))
	    {			/* Its a comment.  Better say APP or NO_APP */
	      char *ends;
	      char *new_buf;
	      char *new_tmp;
	      unsigned int new_length;
	      char *tmp_buf = 0;

	      bump_line_counters ();
	      s = input_line_pointer;
	      if (strncmp (s, "APP\n", 4))
		continue;	/* We ignore it */
	      s += 4;

	      ends = strstr (s, "#NO_APP\n");

	      if (!ends)
		{
		  unsigned int tmp_len;
		  unsigned int num;

		  /* The end of the #APP wasn't in this buffer.  We
		     keep reading in buffers until we find the #NO_APP
		     that goes with this #APP  There is one.  The specs
		     guarentee it. . . */
		  tmp_len = buffer_limit - s;
		  tmp_buf = xmalloc (tmp_len + 1);
		  memcpy (tmp_buf, s, tmp_len);
		  do
		    {
		      new_tmp = input_scrub_next_buffer (&buffer);
		      if (!new_tmp)
			break;
		      else
			buffer_limit = new_tmp;
		      input_line_pointer = buffer;
		      ends = strstr (buffer, "#NO_APP\n");
		      if (ends)
			num = ends - buffer;
		      else
			num = buffer_limit - buffer;

		      tmp_buf = xrealloc (tmp_buf, tmp_len + num);
		      memcpy (tmp_buf + tmp_len, buffer, num);
		      tmp_len += num;
		    }
		  while (!ends);

		  input_line_pointer = ends ? ends + 8 : NULL;

		  s = tmp_buf;
		  ends = s + tmp_len;

		}
	      else
		{
		  input_line_pointer = ends + 8;
		}

	      scrub_string = s;
	      scrub_string_end = ends;

	      new_length = ends - s;
	      new_buf = (char *) xmalloc (new_length);
	      new_tmp = new_buf;
	      for (;;)
		{
		  int space;
		  int size;

		  space = (new_buf + new_length) - new_tmp;
		  size = do_scrub_chars (scrub_from_string, new_tmp, space);

		  if (size < space)
		    {
		      new_tmp += size;
		      break;
		    }

		  new_buf = xrealloc (new_buf, new_length + 100);
		  new_tmp = new_buf + new_length;
		  new_length += 100;
		}

	      if (tmp_buf)
		free (tmp_buf);
	      old_buffer = buffer;
	      old_input = input_line_pointer;
	      old_limit = buffer_limit;
	      buffer = new_buf;
	      input_line_pointer = new_buf;
	      buffer_limit = new_tmp;
	      continue;
	    }

	  HANDLE_CONDITIONAL_ASSEMBLY ();

#ifdef tc_unrecognized_line
	  if (tc_unrecognized_line (c))
	    continue;
#endif

	  /* as_warn("Junk character %d.",c);  Now done by ignore_rest */
	  input_line_pointer--;	/* Report unknown char as ignored. */
	  ignore_rest_of_line ();
	}			/* while (input_line_pointer<buffer_limit) */

#ifdef md_after_pass_hook
      md_after_pass_hook ();
#endif

      if (old_buffer)
	{
	  free (buffer);
	  bump_line_counters ();
	  if (old_input != 0)
	    {
	      buffer = old_buffer;
	      input_line_pointer = old_input;
	      buffer_limit = old_limit;
	      old_buffer = 0;
	      goto contin;
	    }
	}
    }				/* while (more buffers to scan) */

 quit:
  input_scrub_close ();		/* Close the input file */
}

/* For most MRI pseudo-ops, the line actually ends at the first
   nonquoted space.  This function looks for that point, stuffs a null
   in, and sets *STOPCP to the character that used to be there, and
   returns the location.

   Until I hear otherwise, I am going to assume that this is only true
   for the m68k MRI assembler.  */

char *
mri_comment_field (stopcp)
     char *stopcp;
{
#ifdef TC_M68K

  char *s;
  int inquote = 0;

  know (flag_m68k_mri);

  for (s = input_line_pointer;
       ((! is_end_of_line[(unsigned char) *s] && *s != ' ' && *s != '\t')
	|| inquote);
       s++)
    {
      if (*s == '\'')
	inquote = ! inquote;
    }
  *stopcp = *s;
  *s = '\0';
  return s;

#else

  char *s;

  for (s = input_line_pointer; ! is_end_of_line[(unsigned char) *s]; s++)
    ;
  *stopcp = *s;
  *s = '\0';
  return s;

#endif

}

/* Skip to the end of an MRI comment field.  */

void
mri_comment_end (stop, stopc)
     char *stop;
     int stopc;
{
  know (flag_mri);

  input_line_pointer = stop;
  *stop = stopc;
  while (! is_end_of_line[(unsigned char) *input_line_pointer])
    ++input_line_pointer;
}

void 
s_abort (ignore)
     int ignore;
{
  as_fatal (".abort detected.  Abandoning ship.");
}

/* Guts of .align directive.  */
static void 
do_align (n, fill, len)
     int n;
     char *fill;
     int len;
{
#ifdef md_do_align
  md_do_align (n, fill, len, just_record_alignment);
#endif
  if (!fill)
    {
      /* @@ Fix this right for BFD!  */
      static char zero;
      static char nop_opcode = NOP_OPCODE;

      if (now_seg != data_section && now_seg != bss_section)
	{
	  fill = &nop_opcode;
	}
      else
	{
	  fill = &zero;
	}
      len = 1;
    }

  /* Only make a frag if we HAVE to. . . */
  if (n && !need_pass_2)
    {
      if (len <= 1)
	frag_align (n, *fill);
      else
	frag_align_pattern (n, fill, len);
    }

#ifdef md_do_align
 just_record_alignment:
#endif

  record_alignment (now_seg, n);
}

/* For machines where ".align 4" means align to a 4 byte boundary. */
void 
s_align_bytes (arg)
     int arg;
{
  register unsigned int temp;
  char temp_fill;
  unsigned int i = 0;
  unsigned long max_alignment = 1 << 15;
  char *stop = NULL;
  char stopc;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  if (is_end_of_line[(unsigned char) *input_line_pointer])
    {
      if (arg < 0)
	temp = 0;
      else
	temp = arg;	/* Default value from pseudo-op table */
    }
  else
    temp = get_absolute_expression ();

  if (temp > max_alignment)
    {
      as_bad ("Alignment too large: %d. assumed.", temp = max_alignment);
    }

  /* For the sparc, `.align (1<<n)' actually means `.align n' so we
     have to convert it.  */
  if (temp != 0)
    {
      for (i = 0; (temp & 1) == 0; temp >>= 1, ++i)
	;
    }
  if (temp != 1)
    as_bad ("Alignment not a power of 2");

  temp = i;
  if (*input_line_pointer == ',')
    {
      offsetT fillval;
      int len;

      input_line_pointer++;
      fillval = get_absolute_expression ();
      if (arg >= 0)
	len = 1;
      else
	len = - arg;
      if (len <= 1)
	{
	  temp_fill = fillval;
	  do_align (temp, &temp_fill, len);
	}
      else
	{
	  char ab[16];

	  if (len > sizeof ab)
	    abort ();
	  md_number_to_chars (ab, fillval, len);
	  do_align (temp, ab, len);
	}
    }
  else
    {
      if (arg < 0)
	as_warn ("expected fill pattern missing");
      do_align (temp, (char *) NULL, 0);
    }

  if (flag_mri)
    mri_comment_end (stop, stopc);

  demand_empty_rest_of_line ();
}

/* For machines where ".align 4" means align to 2**4 boundary. */
void 
s_align_ptwo (arg)
     int arg;
{
  register int temp;
  char temp_fill;
  long max_alignment = 15;
  char *stop = NULL;
  char stopc;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  temp = get_absolute_expression ();
  if (temp > max_alignment)
    as_bad ("Alignment too large: %d. assumed.", temp = max_alignment);
  else if (temp < 0)
    {
      as_bad ("Alignment negative. 0 assumed.");
      temp = 0;
    }
  if (*input_line_pointer == ',')
    {
      offsetT fillval;
      int len;

      input_line_pointer++;
      fillval = get_absolute_expression ();
      if (arg >= 0)
	len = 1;
      else
	len = - arg;
      if (len <= 1)
	{
	  temp_fill = fillval;
	  do_align (temp, &temp_fill, len);
	}
      else
	{
	  char ab[16];

	  if (len > sizeof ab)
	    abort ();
	  md_number_to_chars (ab, fillval, len);
	  do_align (temp, ab, len);
	}
    }
  else
    {
      if (arg < 0)
	as_warn ("expected fill pattern missing");
      do_align (temp, (char *) NULL, 0);
    }

  if (flag_mri)
    mri_comment_end (stop, stopc);

  demand_empty_rest_of_line ();
}

void 
s_comm (ignore)
     int ignore;
{
  register char *name;
  register char c;
  register char *p;
  offsetT temp;
  register symbolS *symbolP;
  char *stop = NULL;
  char stopc;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  name = input_line_pointer;
  c = get_symbol_end ();
  /* just after name is now '\0' */
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad ("Expected comma after symbol-name: rest of line ignored.");
      if (flag_mri)
	mri_comment_end (stop, stopc);
      ignore_rest_of_line ();
      return;
    }
  input_line_pointer++;		/* skip ',' */
  if ((temp = get_absolute_expression ()) < 0)
    {
      as_warn (".COMMon length (%ld.) <0! Ignored.", (long) temp);
      if (flag_mri)
	mri_comment_end (stop, stopc);
      ignore_rest_of_line ();
      return;
    }
  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;
  if (S_IS_DEFINED (symbolP))
    {
      as_bad ("Ignoring attempt to re-define symbol `%s'.",
	      S_GET_NAME (symbolP));
      if (flag_mri)
	mri_comment_end (stop, stopc);
      ignore_rest_of_line ();
      return;
    }
  if (S_GET_VALUE (symbolP))
    {
      if (S_GET_VALUE (symbolP) != (valueT) temp)
	as_bad ("Length of .comm \"%s\" is already %ld. Not changed to %ld.",
		S_GET_NAME (symbolP),
		(long) S_GET_VALUE (symbolP),
		(long) temp);
    }
  else
    {
      S_SET_VALUE (symbolP, (valueT) temp);
      S_SET_EXTERNAL (symbolP);
    }
#ifdef OBJ_VMS
  {
    extern int flag_one;
    if ( (!temp) || !flag_one)
      S_GET_OTHER(symbolP) = const_flag;
  }
#endif /* not OBJ_VMS */
  know (symbolP->sy_frag == &zero_address_frag);

  if (flag_mri)
    mri_comment_end (stop, stopc);

  demand_empty_rest_of_line ();
}				/* s_comm() */

/* The MRI COMMON pseudo-op.  We handle this by creating a common
   symbol with the appropriate name.  We make s_space do the right
   thing by increasing the size.  */

void
s_mri_common (small)
     int small;
{
  char *name;
  char c;
  char *alc = NULL;
  symbolS *sym;
  offsetT align;
  char *stop = NULL;
  char stopc;

  if (! flag_mri)
    {
      s_comm (0);
      return;
    }

  stop = mri_comment_field (&stopc);

  SKIP_WHITESPACE ();

  name = input_line_pointer;
  if (! isdigit ((unsigned char) *name))
    c = get_symbol_end ();
  else
    {
      do
	{
	  ++input_line_pointer;
	}
      while (isdigit ((unsigned char) *input_line_pointer));
      c = *input_line_pointer;
      *input_line_pointer = '\0';

      if (line_label != NULL)
	{
	  alc = (char *) xmalloc (strlen (S_GET_NAME (line_label))
				  + (input_line_pointer - name)
				  + 1);
	  sprintf (alc, "%s%s", name, S_GET_NAME (line_label));
	  name = alc;
	}
    }

  sym = symbol_find_or_make (name);
  *input_line_pointer = c;
  if (alc != NULL)
    free (alc);

  if (*input_line_pointer != ',')
    align = 0;
  else
    {
      ++input_line_pointer;
      align = get_absolute_expression ();
    }

  if (S_IS_DEFINED (sym))
    {
#if defined (S_IS_COMMON) || defined (BFD_ASSEMBLER)
      if (! S_IS_COMMON (sym))
#endif
	{
	  as_bad ("attempt to re-define symbol `%s'", S_GET_NAME (sym));
	  mri_comment_end (stop, stopc);
	  ignore_rest_of_line ();
	  return;
	}
    }

  S_SET_EXTERNAL (sym);
  mri_common_symbol = sym;

#ifdef S_SET_ALIGN
  if (align != 0)
    S_SET_ALIGN (sym, align);
#endif

  if (line_label != NULL)
    {
      line_label->sy_value.X_op = O_symbol;
      line_label->sy_value.X_add_symbol = sym;
      line_label->sy_value.X_add_number = S_GET_VALUE (sym);
      line_label->sy_frag = &zero_address_frag;
      S_SET_SEGMENT (line_label, expr_section);
    }

  /* FIXME: We just ignore the small argument, which distinguishes
     COMMON and COMMON.S.  I don't know what we can do about it.  */

  /* Ignore the type and hptype.  */
  if (*input_line_pointer == ',')
    input_line_pointer += 2;
  if (*input_line_pointer == ',')
    input_line_pointer += 2;

  mri_comment_end (stop, stopc);

  demand_empty_rest_of_line ();
}

void
s_data (ignore)
     int ignore;
{
  segT section;
  register int temp;

  temp = get_absolute_expression ();
  if (flag_readonly_data_in_text)
    {
      section = text_section;
      temp += 1000;
    }
  else
    section = data_section;

  subseg_set (section, (subsegT) temp);

#ifdef OBJ_VMS
  const_flag = 0;
#endif
  demand_empty_rest_of_line ();
}

/* Handle the .appfile pseudo-op.  This is automatically generated by
   do_scrub_chars when a preprocessor # line comment is seen with a
   file name.  This default definition may be overridden by the object
   or CPU specific pseudo-ops.  This function is also the default
   definition for .file; the APPFILE argument is 1 for .appfile, 0 for
   .file.  */

void 
s_app_file (appfile)
     int appfile;
{
  register char *s;
  int length;

  /* Some assemblers tolerate immediately following '"' */
  if ((s = demand_copy_string (&length)) != 0)
    {
      /* If this is a fake .appfile, a fake newline was inserted into
	 the buffer.  Passing -2 to new_logical_line tells it to
	 account for it.  */
      new_logical_line (s, appfile ? -2 : -1);

      /* In MRI mode, the preprocessor may have inserted an extraneous
         backquote.  */
      if (flag_m68k_mri
	  && *input_line_pointer == '\''
	  && is_end_of_line[(unsigned char) input_line_pointer[1]])
	++input_line_pointer;

      demand_empty_rest_of_line ();
#ifdef LISTING
      if (listing)
	listing_source_file (s);
#endif
    }
#ifdef obj_app_file
  obj_app_file (s);
#endif
}

/* Handle the .appline pseudo-op.  This is automatically generated by
   do_scrub_chars when a preprocessor # line comment is seen.  This
   default definition may be overridden by the object or CPU specific
   pseudo-ops.  */

void
s_app_line (ignore)
     int ignore;
{
  int l;

  /* The given number is that of the next line.  */
  l = get_absolute_expression () - 1;
  if (l < 0)
    /* Some of the back ends can't deal with non-positive line numbers.
       Besides, it's silly.  */
    as_warn ("Line numbers must be positive; line number %d rejected.", l+1);
  else
    {
      new_logical_line ((char *) NULL, l);
#ifdef LISTING
      if (listing)
	listing_source_line (l);
#endif
    }
  demand_empty_rest_of_line ();
}

/* Handle the .end pseudo-op.  Actually, the real work is done in
   read_a_source_file.  */

void
s_end (ignore)
     int ignore;
{
  if (flag_mri)
    {
      /* The MRI assembler permits the start symbol to follow .end,
         but we don't support that.  */
      SKIP_WHITESPACE ();
      if (! is_end_of_line[(unsigned char) *input_line_pointer]
	  && *input_line_pointer != '*'
	  && *input_line_pointer != '!')
	as_warn ("start address not supported");
    }
}

/* Handle the .err pseudo-op.  */

void
s_err (ignore)
     int ignore;
{
  as_bad (".err encountered");
  demand_empty_rest_of_line ();
}

/* Handle the MRI fail pseudo-op.  */

void
s_fail (ignore)
     int ignore;
{
  offsetT temp;
  char *stop = NULL;
  char stopc;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  temp = get_absolute_expression ();
  if (temp >= 500)
    as_warn (".fail %ld encountered", (long) temp);
  else
    as_bad (".fail %ld encountered", (long) temp);

  if (flag_mri)
    mri_comment_end (stop, stopc);

  demand_empty_rest_of_line ();
}

void 
s_fill (ignore)
     int ignore;
{
  long temp_repeat = 0;
  long temp_size = 1;
  register long temp_fill = 0;
  char *p;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  temp_repeat = get_absolute_expression ();
  if (*input_line_pointer == ',')
    {
      input_line_pointer++;
      temp_size = get_absolute_expression ();
      if (*input_line_pointer == ',')
	{
	  input_line_pointer++;
	  temp_fill = get_absolute_expression ();
	}
    }
  /* This is to be compatible with BSD 4.2 AS, not for any rational reason.  */
#define BSD_FILL_SIZE_CROCK_8 (8)
  if (temp_size > BSD_FILL_SIZE_CROCK_8)
    {
      as_warn (".fill size clamped to %d.", BSD_FILL_SIZE_CROCK_8);
      temp_size = BSD_FILL_SIZE_CROCK_8;
    }
  if (temp_size < 0)
    {
      as_warn ("Size negative: .fill ignored.");
      temp_size = 0;
    }
  else if (temp_repeat <= 0)
    {
      if (temp_repeat < 0)
	as_warn ("Repeat < 0, .fill ignored");
      temp_size = 0;
    }

  if (temp_size && !need_pass_2)
    {
      p = frag_var (rs_fill, (int) temp_size, (int) temp_size, (relax_substateT) 0, (symbolS *) 0, temp_repeat, (char *) 0);
      memset (p, 0, (unsigned int) temp_size);
      /* The magic number BSD_FILL_SIZE_CROCK_4 is from BSD 4.2 VAX
       * flavoured AS.  The following bizzare behaviour is to be
       * compatible with above.  I guess they tried to take up to 8
       * bytes from a 4-byte expression and they forgot to sign
       * extend. Un*x Sux. */
#define BSD_FILL_SIZE_CROCK_4 (4)
      md_number_to_chars (p, (valueT) temp_fill,
			  (temp_size > BSD_FILL_SIZE_CROCK_4
			   ? BSD_FILL_SIZE_CROCK_4
			   : (int) temp_size));
      /* Note: .fill (),0 emits no frag (since we are asked to .fill 0 bytes)
       * but emits no error message because it seems a legal thing to do.
       * It is a degenerate case of .fill but could be emitted by a compiler.
       */
    }
  demand_empty_rest_of_line ();
}

void 
s_globl (ignore)
     int ignore;
{
  char *name;
  int c;
  symbolS *symbolP;
  char *stop = NULL;
  char stopc;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  do
    {
      name = input_line_pointer;
      c = get_symbol_end ();
      symbolP = symbol_find_or_make (name);
      *input_line_pointer = c;
      SKIP_WHITESPACE ();
      S_SET_EXTERNAL (symbolP);
      if (c == ',')
	{
	  input_line_pointer++;
	  SKIP_WHITESPACE ();
	  if (*input_line_pointer == '\n')
	    c = '\n';
	}
    }
  while (c == ',');

  if (flag_mri)
    mri_comment_end (stop, stopc);

  demand_empty_rest_of_line ();
}

/* Handle the MRI IRP and IRPC pseudo-ops.  */

void
s_irp (irpc)
     int irpc;
{
  char *file;
  unsigned int line;
  sb s;
  const char *err;
  sb out;

  as_where (&file, &line);

  sb_new (&s);
  while (! is_end_of_line[(unsigned char) *input_line_pointer])
    sb_add_char (&s, *input_line_pointer++);

  sb_new (&out);

  err = expand_irp (irpc, 0, &s, &out, get_line_sb, '\0');
  if (err != NULL)
    as_bad_where (file, line, "%s", err);

  sb_kill (&s);

  input_scrub_include_sb (&out, input_line_pointer);
  sb_kill (&out);
  buffer_limit = input_scrub_next_buffer (&input_line_pointer);
}

/* Handle the .linkonce pseudo-op.  This tells the assembler to mark
   the section to only be linked once.  However, this is not supported
   by most object file formats.  This takes an optional argument,
   which is what to do about duplicates.  */

void
s_linkonce (ignore)
     int ignore;
{
  enum linkonce_type type;

  SKIP_WHITESPACE ();

  type = LINKONCE_DISCARD;

  if (! is_end_of_line[(unsigned char) *input_line_pointer])
    {
      char *s;
      char c;

      s = input_line_pointer;
      c = get_symbol_end ();
      if (strcasecmp (s, "discard") == 0)
	type = LINKONCE_DISCARD;
      else if (strcasecmp (s, "one_only") == 0)
	type = LINKONCE_ONE_ONLY;
      else if (strcasecmp (s, "same_size") == 0)
	type = LINKONCE_SAME_SIZE;
      else if (strcasecmp (s, "same_contents") == 0)
	type = LINKONCE_SAME_CONTENTS;
      else
	as_warn ("unrecognized .linkonce type `%s'", s);

      *input_line_pointer = c;
    }

#ifdef obj_handle_link_once
  obj_handle_link_once (type);
#else /* ! defined (obj_handle_link_once) */
#ifdef BFD_ASSEMBLER
  {
    flagword flags;

    if ((bfd_applicable_section_flags (stdoutput) & SEC_LINK_ONCE) == 0)
      as_warn (".linkonce is not supported for this object file format");

    flags = bfd_get_section_flags (stdoutput, now_seg);
    flags |= SEC_LINK_ONCE;
    switch (type)
      {
      default:
	abort ();
      case LINKONCE_DISCARD:
	flags |= SEC_LINK_DUPLICATES_DISCARD;
	break;
      case LINKONCE_ONE_ONLY:
	flags |= SEC_LINK_DUPLICATES_ONE_ONLY;
	break;
      case LINKONCE_SAME_SIZE:
	flags |= SEC_LINK_DUPLICATES_SAME_SIZE;
	break;
      case LINKONCE_SAME_CONTENTS:
	flags |= SEC_LINK_DUPLICATES_SAME_CONTENTS;
	break;
      }
    if (! bfd_set_section_flags (stdoutput, now_seg, flags))
      as_bad ("bfd_set_section_flags: %s",
	      bfd_errmsg (bfd_get_error ()));
  }
#else /* ! defined (BFD_ASSEMBLER) */
  as_warn (".linkonce is not supported for this object file format");
#endif /* ! defined (BFD_ASSEMBLER) */
#endif /* ! defined (obj_handle_link_once) */

  demand_empty_rest_of_line ();
}

void 
s_lcomm (needs_align)
     /* 1 if this was a ".bss" directive, which may require a 3rd argument
	(alignment); 0 if it was an ".lcomm" (2 args only)  */
     int needs_align;
{
  register char *name;
  register char c;
  register char *p;
  register int temp;
  register symbolS *symbolP;
  segT current_seg = now_seg;
  subsegT current_subseg = now_subseg;
  const int max_alignment = 15;
  int align = 0;
  segT bss_seg = bss_section;

  name = input_line_pointer;
  c = get_symbol_end ();
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE ();

  /* Accept an optional comma after the name.  The comma used to be
     required, but Irix 5 cc does not generate it.  */
  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;
      SKIP_WHITESPACE ();
    }

  if (*input_line_pointer == '\n')
    {
      as_bad ("Missing size expression");
      return;
    }

  if ((temp = get_absolute_expression ()) < 0)
    {
      as_warn ("BSS length (%d.) <0! Ignored.", temp);
      ignore_rest_of_line ();
      return;
    }

#if defined (TC_MIPS) || defined (TC_ALPHA)
  if (OUTPUT_FLAVOR == bfd_target_ecoff_flavour
      || OUTPUT_FLAVOR == bfd_target_elf_flavour)
    {
      /* For MIPS and Alpha ECOFF or ELF, small objects are put in .sbss.  */
      if (temp <= bfd_get_gp_size (stdoutput))
	{
	  bss_seg = subseg_new (".sbss", 1);
	  seg_info (bss_seg)->bss = 1;
#ifdef BFD_ASSEMBLER
	  if (! bfd_set_section_flags (stdoutput, bss_seg, SEC_ALLOC))
	    as_warn ("error setting flags for \".sbss\": %s",
		     bfd_errmsg (bfd_get_error ()));
#endif
	}
    }
#endif
   if (!needs_align)
     {
       /* FIXME. This needs to be machine independent. */
       if (temp >= 8)
	 align = 3;
       else if (temp >= 4)
	 align = 2;
       else if (temp >= 2)
	 align = 1;
       else
	 align = 0;

       record_alignment(bss_seg, align);
     }

  if (needs_align)
    {
      align = 0;
      SKIP_WHITESPACE ();
      if (*input_line_pointer != ',')
	{
	  as_bad ("Expected comma after size");
	  ignore_rest_of_line ();
	  return;
	}
      input_line_pointer++;
      SKIP_WHITESPACE ();
      if (*input_line_pointer == '\n')
	{
	  as_bad ("Missing alignment");
	  return;
	}
      align = get_absolute_expression ();
      if (align > max_alignment)
	{
	  align = max_alignment;
	  as_warn ("Alignment too large: %d. assumed.", align);
	}
      else if (align < 0)
	{
	  align = 0;
	  as_warn ("Alignment negative. 0 assumed.");
	}
      record_alignment (bss_seg, align);
    }				/* if needs align */
  else
    {
      /* Assume some objects may require alignment on some systems.  */
#ifdef TC_ALPHA
      if (temp > 1)
	{
	  align = ffs (temp) - 1;
	  if (temp % (1 << align))
	    abort ();
	}
#endif
    }

  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;

  if (
#if defined(OBJ_AOUT) | defined(OBJ_BOUT)
       S_GET_OTHER (symbolP) == 0 &&
       S_GET_DESC (symbolP) == 0 &&
#endif /* OBJ_AOUT or OBJ_BOUT */
       (S_GET_SEGMENT (symbolP) == bss_seg
	|| (!S_IS_DEFINED (symbolP) && S_GET_VALUE (symbolP) == 0)))
    {
      char *pfrag;

      subseg_set (bss_seg, 1);

      if (align)
	frag_align (align, 0);
					/* detach from old frag	*/
      if (S_GET_SEGMENT (symbolP) == bss_seg)
	symbolP->sy_frag->fr_symbol = NULL;

      symbolP->sy_frag = frag_now;
      pfrag = frag_var (rs_org, 1, 1, (relax_substateT)0, symbolP,
			temp, (char *)0);
      *pfrag = 0;

      S_SET_SEGMENT (symbolP, bss_seg);

#ifdef OBJ_COFF
      /* The symbol may already have been created with a preceding
         ".globl" directive -- be careful not to step on storage class
         in that case.  Otherwise, set it to static. */
      if (S_GET_STORAGE_CLASS (symbolP) != C_EXT)
	{
	  S_SET_STORAGE_CLASS (symbolP, C_STAT);
	}
#endif /* OBJ_COFF */

#ifdef S_SET_SIZE
      S_SET_SIZE (symbolP, temp);
#endif
    }
  else
    as_bad ("Ignoring attempt to re-define symbol `%s'.",
	    S_GET_NAME (symbolP));

  subseg_set (current_seg, current_subseg);

  demand_empty_rest_of_line ();
}				/* s_lcomm() */

void 
s_lsym (ignore)
     int ignore;
{
  register char *name;
  register char c;
  register char *p;
  expressionS exp;
  register symbolS *symbolP;

  /* we permit ANY defined expression: BSD4.2 demands constants */
  name = input_line_pointer;
  c = get_symbol_end ();
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      *p = 0;
      as_bad ("Expected comma after name \"%s\"", name);
      *p = c;
      ignore_rest_of_line ();
      return;
    }
  input_line_pointer++;
  expression (&exp);
  if (exp.X_op != O_constant
      && exp.X_op != O_register)
    {
      as_bad ("bad expression");
      ignore_rest_of_line ();
      return;
    }
  *p = 0;
  symbolP = symbol_find_or_make (name);

  /* FIXME-SOON I pulled a (&& symbolP->sy_other == 0 &&
     symbolP->sy_desc == 0) out of this test because coff doesn't have
     those fields, and I can't see when they'd ever be tripped.  I
     don't think I understand why they were here so I may have
     introduced a bug. As recently as 1.37 didn't have this test
     anyway.  xoxorich. */

  if (S_GET_SEGMENT (symbolP) == undefined_section
      && S_GET_VALUE (symbolP) == 0)
    {
      /* The name might be an undefined .global symbol; be sure to
	 keep the "external" bit. */
      S_SET_SEGMENT (symbolP,
		     (exp.X_op == O_constant
		      ? absolute_section
		      : reg_section));
      S_SET_VALUE (symbolP, (valueT) exp.X_add_number);
    }
  else
    {
      as_bad ("Symbol %s already defined", name);
    }
  *p = c;
  demand_empty_rest_of_line ();
}				/* s_lsym() */

/* Read a line into an sb.  */

static int
get_line_sb (line)
     sb *line;
{
  if (input_line_pointer[-1] == '\n')
    bump_line_counters ();

  if (input_line_pointer >= buffer_limit)
    {
      buffer_limit = input_scrub_next_buffer (&input_line_pointer);
      if (buffer_limit == 0)
	return 0;
    }

  while (! is_end_of_line[(unsigned char) *input_line_pointer])
    sb_add_char (line, *input_line_pointer++);
  while (input_line_pointer < buffer_limit
	 && is_end_of_line[(unsigned char) *input_line_pointer])
    {
      if (input_line_pointer[-1] == '\n')
	bump_line_counters ();
      ++input_line_pointer;
    }
  return 1;
}

/* Define a macro.  This is an interface to macro.c, which is shared
   between gas and gasp.  */

void
s_macro (ignore)
     int ignore;
{
  char *file;
  unsigned int line;
  sb s;
  sb label;
  const char *err;

  as_where (&file, &line);

  sb_new (&s);
  while (! is_end_of_line[(unsigned char) *input_line_pointer])
    sb_add_char (&s, *input_line_pointer++);

  sb_new (&label);
  if (line_label != NULL)
    sb_add_string (&label, S_GET_NAME (line_label));

  err = define_macro (0, &s, &label, get_line_sb);
  if (err != NULL)
    as_bad_where (file, line, "%s", err);
  else
    {
      if (line_label != NULL)
	{
	  S_SET_SEGMENT (line_label, undefined_section);
	  S_SET_VALUE (line_label, 0);
	  line_label->sy_frag = &zero_address_frag;
	}
    }

  sb_kill (&s);
}

/* Handle the .mexit pseudo-op, which immediately exits a macro
   expansion.  */

void
s_mexit (ignore)
     int ignore;
{
  buffer_limit = input_scrub_next_buffer (&input_line_pointer);
}

/* Switch in and out of MRI mode.  */

void
s_mri (ignore)
     int ignore;
{
  int on, old_flag;

  on = get_absolute_expression ();
  old_flag = flag_mri;
  if (on != 0)
    {
      flag_mri = 1;
#ifdef TC_M68K
      flag_m68k_mri = 1;
#endif
    }
  else
    {
      flag_mri = 0;
      flag_m68k_mri = 0;
    }

#ifdef MRI_MODE_CHANGE
  if (on != old_flag)
    MRI_MODE_CHANGE (on);
#endif

  demand_empty_rest_of_line ();
}

/* Handle changing the location counter.  */

static void
do_org (segment, exp, fill)
     segT segment;
     expressionS *exp;
     int fill;
{
  if (segment != now_seg && segment != absolute_section)
    as_bad ("invalid segment \"%s\"; segment \"%s\" assumed",
	    segment_name (segment), segment_name (now_seg));

  if (now_seg == absolute_section)
    {
      if (fill != 0)
	as_warn ("ignoring fill value in absolute section");
      if (exp->X_op != O_constant)
	{
	  as_bad ("only constant offsets supported in absolute section");
	  exp->X_add_number = 0;
	}
      abs_section_offset = exp->X_add_number;
    }
  else
    {
      char *p;

      p = frag_var (rs_org, 1, 1, (relax_substateT) 0, exp->X_add_symbol,
		    exp->X_add_number, (char *) NULL);
      *p = fill;
    }
}

void 
s_org (ignore)
     int ignore;
{
  register segT segment;
  expressionS exp;
  register long temp_fill;

  /* The m68k MRI assembler has a different meaning for .org.  It
     means to create an absolute section at a given address.  We can't
     support that--use a linker script instead.  */
  if (flag_m68k_mri)
    {
      as_bad ("MRI style ORG pseudo-op not supported");
      ignore_rest_of_line ();
      return;
    }

  /* Don't believe the documentation of BSD 4.2 AS.  There is no such
     thing as a sub-segment-relative origin.  Any absolute origin is
     given a warning, then assumed to be segment-relative.  Any
     segmented origin expression ("foo+42") had better be in the right
     segment or the .org is ignored.

     BSD 4.2 AS warns if you try to .org backwards. We cannot because
     we never know sub-segment sizes when we are reading code.  BSD
     will crash trying to emit negative numbers of filler bytes in
     certain .orgs. We don't crash, but see as-write for that code.

     Don't make frag if need_pass_2==1.  */
  segment = get_known_segmented_expression (&exp);
  if (*input_line_pointer == ',')
    {
      input_line_pointer++;
      temp_fill = get_absolute_expression ();
    }
  else
    temp_fill = 0;

  if (!need_pass_2)
    do_org (segment, &exp, temp_fill);

  demand_empty_rest_of_line ();
}				/* s_org() */

/* Handle parsing for the MRI SECT/SECTION pseudo-op.  This should be
   called by the obj-format routine which handles section changing
   when in MRI mode.  It will create a new section, and return it.  It
   will set *TYPE to the section type: one of 'C' (code), 'D' (data),
   'M' (mixed), or 'R' (romable).  If BFD_ASSEMBLER is defined, the
   flags will be set in the section.  */

void
s_mri_sect (type)
     char *type;
{
#ifdef TC_M68K

  char *name;
  char c;
  segT seg;

  SKIP_WHITESPACE ();
  
  name = input_line_pointer;
  if (! isdigit ((unsigned char) *name))
    c = get_symbol_end ();
  else
    {
      do
	{
	  ++input_line_pointer;
	}
      while (isdigit ((unsigned char) *input_line_pointer));
      c = *input_line_pointer;
      *input_line_pointer = '\0';
    }

  name = xstrdup (name);

  *input_line_pointer = c;

  seg = subseg_new (name, 0);

  if (*input_line_pointer == ',')
    {
      int align;

      ++input_line_pointer;
      align = get_absolute_expression ();
      record_alignment (seg, align);
    }

  *type = 'C';
  if (*input_line_pointer == ',')
    {
      c = *++input_line_pointer;
      c = toupper ((unsigned char) c);
      if (c == 'C' || c == 'D' || c == 'M' || c == 'R')
	*type = c;
      else
	as_bad ("unrecognized section type");
      ++input_line_pointer;

#ifdef BFD_ASSEMBLER
      {
	flagword flags;

	flags = SEC_NO_FLAGS;
	if (*type == 'C')
	  flags = SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE;
	else if (*type == 'D' || *type == 'M')
	  flags = SEC_ALLOC | SEC_LOAD | SEC_DATA;
	else if (*type == 'R')
	  flags = SEC_ALLOC | SEC_LOAD | SEC_DATA | SEC_READONLY | SEC_ROM;
	if (flags != SEC_NO_FLAGS)
	  {
	    if (! bfd_set_section_flags (stdoutput, seg, flags))
	      as_warn ("error setting flags for \"%s\": %s",
		       bfd_section_name (stdoutput, seg),
		       bfd_errmsg (bfd_get_error ()));
	  }
      }
#endif
    }

  /* Ignore the HP type.  */
  if (*input_line_pointer == ',')
    input_line_pointer += 2;

  demand_empty_rest_of_line ();

#else /* ! TC_M68K */
#ifdef TC_I960

  char *name;
  char c;
  segT seg;

  SKIP_WHITESPACE ();

  name = input_line_pointer;
  c = get_symbol_end ();

  name = xstrdup (name);

  *input_line_pointer = c;

  seg = subseg_new (name, 0);

  if (*input_line_pointer != ',')
    *type = 'C';
  else
    {
      char *sectype;

      ++input_line_pointer;
      SKIP_WHITESPACE ();
      sectype = input_line_pointer;
      c = get_symbol_end ();
      if (*sectype == '\0')
	*type = 'C';
      else if (strcasecmp (sectype, "text") == 0)
	*type = 'C';
      else if (strcasecmp (sectype, "data") == 0)
	*type = 'D';
      else if (strcasecmp (sectype, "romdata") == 0)
	*type = 'R';
      else
	as_warn ("unrecognized section type `%s'", sectype);
      *input_line_pointer = c;
    }

  if (*input_line_pointer == ',')
    {
      char *seccmd;

      ++input_line_pointer;
      SKIP_WHITESPACE ();
      seccmd = input_line_pointer;
      c = get_symbol_end ();
      if (strcasecmp (seccmd, "absolute") == 0)
	{
	  as_bad ("absolute sections are not supported");
	  *input_line_pointer = c;
	  ignore_rest_of_line ();
	  return;
	}
      else if (strcasecmp (seccmd, "align") == 0)
	{
	  int align;

	  *input_line_pointer = c;
	  align = get_absolute_expression ();
	  record_alignment (seg, align);
	}
      else
	{
	  as_warn ("unrecognized section command `%s'", seccmd);
	  *input_line_pointer = c;
	}
    }

  demand_empty_rest_of_line ();	  

#else /* ! TC_I960 */
  /* The MRI assembler seems to use different forms of .sect for
     different targets.  */
  abort ();
#endif /* ! TC_I960 */
#endif /* ! TC_M68K */
}

/* Handle the .print pseudo-op.  */

void
s_print (ignore)
     int ignore;
{
  char *s;
  int len;

  s = demand_copy_C_string (&len);
  printf ("%s\n", s);
  demand_empty_rest_of_line ();
}

/* Handle the .purgem pseudo-op.  */

void
s_purgem (ignore)
     int ignore;
{
  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

  do
    {
      char *name;
      char c;

      SKIP_WHITESPACE ();
      name = input_line_pointer;
      c = get_symbol_end ();
      delete_macro (name);
      *input_line_pointer = c;
      SKIP_WHITESPACE ();
    }
  while (*input_line_pointer++ == ',');

  --input_line_pointer;
  demand_empty_rest_of_line ();
}

/* Handle the .rept pseudo-op.  */

void
s_rept (ignore)
     int ignore;
{
  int count;
  sb one;
  sb many;

  count = get_absolute_expression ();

  sb_new (&one);
  if (! buffer_and_nest ("REPT", "ENDR", &one, get_line_sb))
    {
      as_bad ("rept without endr");
      return;
    }

  sb_new (&many);
  while (count-- > 0)
    sb_add_sb (&many, &one);

  sb_kill (&one);

  input_scrub_include_sb (&many, input_line_pointer);
  sb_kill (&many);
  buffer_limit = input_scrub_next_buffer (&input_line_pointer);
}

void 
s_set (ignore)
     int ignore;
{
  register char *name;
  register char delim;
  register char *end_name;
  register symbolS *symbolP;

  /*
   * Especial apologies for the random logic:
   * this just grew, and could be parsed much more simply!
   * Dean in haste.
   */
  name = input_line_pointer;
  delim = get_symbol_end ();
  end_name = input_line_pointer;
  *end_name = delim;
  SKIP_WHITESPACE ();

  if (*input_line_pointer != ',')
    {
      *end_name = 0;
      as_bad ("Expected comma after name \"%s\"", name);
      *end_name = delim;
      ignore_rest_of_line ();
      return;
    }

  input_line_pointer++;
  *end_name = 0;

  if (name[0] == '.' && name[1] == '\0')
    {
      /* Turn '. = mumble' into a .org mumble */
      register segT segment;
      expressionS exp;

      segment = get_known_segmented_expression (&exp);

      if (!need_pass_2)
	do_org (segment, &exp, 0);

      *end_name = delim;
      return;
    }

  if ((symbolP = symbol_find (name)) == NULL
      && (symbolP = md_undefined_symbol (name)) == NULL)
    {
      symbolP = symbol_new (name, undefined_section, 0, &zero_address_frag);
#ifdef OBJ_COFF
      /* "set" symbols are local unless otherwise specified. */
      SF_SET_LOCAL (symbolP);
#endif /* OBJ_COFF */

    }				/* make a new symbol */

  symbol_table_insert (symbolP);

  *end_name = delim;
  pseudo_set (symbolP);
  demand_empty_rest_of_line ();
}				/* s_set() */

void 
s_space (mult)
     int mult;
{
  expressionS exp;
  expressionS val;
  char *p = 0;
  char *stop = NULL;
  char stopc;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  expression (&exp);

  SKIP_WHITESPACE ();
  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;
      expression (&val);
    }
  else
    {
      val.X_op = O_constant;
      val.X_add_number = 0;
    }

  if (val.X_op != O_constant
      || val.X_add_number < - 0x80
      || val.X_add_number > 0xff
      || (mult != 0 && mult != 1 && val.X_add_number != 0))
    {
      if (exp.X_op != O_constant)
	as_bad ("Unsupported variable size or fill value");
      else
	{
	  offsetT i;

	  if (mult == 0)
	    mult = 1;
	  for (i = 0; i < exp.X_add_number; i++)
	    emit_expr (&val, mult);
	}
    }
  else
    {
      if (exp.X_op == O_constant)
	{
	  long repeat;

	  repeat = exp.X_add_number;
	  if (mult)
	    repeat *= mult;
	  if (repeat <= 0)
	    {
	      if (! flag_mri || repeat < 0)
		as_warn (".space repeat count is %s, ignored",
			 repeat ? "negative" : "zero");
	      goto getout;
	    }

	  /* If we are in the absolute section, just bump the offset.  */
	  if (now_seg == absolute_section)
	    {
	      abs_section_offset += repeat;
	      goto getout;
	    }

	  /* If we are secretly in an MRI common section, then
	     creating space just increases the size of the common
	     symbol.  */
	  if (mri_common_symbol != NULL)
	    {
	      S_SET_VALUE (mri_common_symbol,
			   S_GET_VALUE (mri_common_symbol) + repeat);
	      goto getout;
	    }

	  if (!need_pass_2)
	    p = frag_var (rs_fill, 1, 1, (relax_substateT) 0, (symbolS *) 0,
			  repeat, (char *) 0);
	}
      else
	{
	  if (now_seg == absolute_section)
	    {
	      as_bad ("space allocation too complex in absolute section");
	      subseg_set (text_section, 0);
	    }
	  if (mri_common_symbol != NULL)
	    {
	      as_bad ("space allocation too complex in common section");
	      mri_common_symbol = NULL;
	    }
	  if (!need_pass_2)
	    p = frag_var (rs_space, 1, 1, (relax_substateT) 0,
			  make_expr_symbol (&exp), 0L, (char *) 0);
	}

      if (p)
	*p = val.X_add_number;
    }

 getout:
  if (flag_mri)
    mri_comment_end (stop, stopc);

  demand_empty_rest_of_line ();
}

/* This is like s_space, but the value is a floating point number with
   the given precision.  This is for the MRI dcb.s pseudo-op and
   friends.  */

void
s_float_space (float_type)
     int float_type;
{
  offsetT count;
  int flen;
  char temp[MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT];
  char *stop = NULL;
  char stopc;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  count = get_absolute_expression ();

  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad ("missing value");
      if (flag_mri)
	mri_comment_end (stop, stopc);
      ignore_rest_of_line ();
      return;
    }

  ++input_line_pointer;

  SKIP_WHITESPACE ();

  /* Skip any 0{letter} that may be present.  Don't even check if the
   * letter is legal.  */
  if (input_line_pointer[0] == '0' && isalpha (input_line_pointer[1]))
    input_line_pointer += 2;

  /* Accept :xxxx, where the x's are hex digits, for a floating point
     with the exact digits specified.  */
  if (input_line_pointer[0] == ':')
    {
      flen = hex_float (float_type, temp);
      if (flen < 0)
	{
	  if (flag_mri)
	    mri_comment_end (stop, stopc);
	  ignore_rest_of_line ();
	  return;
	}
    }
  else
    {
      char *err;

      err = md_atof (float_type, temp, &flen);
      know (flen <= MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT);
      know (flen > 0);
      if (err)
	{
	  as_bad ("Bad floating literal: %s", err);
	  if (flag_mri)
	    mri_comment_end (stop, stopc);
	  ignore_rest_of_line ();
	  return;
	}
    }

  while (--count >= 0)
    {
      char *p;

      p = frag_more (flen);
      memcpy (p, temp, (unsigned int) flen);
    }

  if (flag_mri)
    mri_comment_end (stop, stopc);

  demand_empty_rest_of_line ();
}

/* Handle the .struct pseudo-op, as found in MIPS assemblers.  */

void
s_struct (ignore)
     int ignore;
{
  char *stop = NULL;
  char stopc;

  if (flag_mri)
    stop = mri_comment_field (&stopc);
  abs_section_offset = get_absolute_expression ();
  subseg_set (absolute_section, 0);
  if (flag_mri)
    mri_comment_end (stop, stopc);
  demand_empty_rest_of_line ();
}

void
s_text (ignore)
     int ignore;
{
  register int temp;

  temp = get_absolute_expression ();
  subseg_set (text_section, (subsegT) temp);
  demand_empty_rest_of_line ();
#ifdef OBJ_VMS
  const_flag &= ~IN_DEFAULT_SECTION;
#endif
}				/* s_text() */


void 
demand_empty_rest_of_line ()
{
  SKIP_WHITESPACE ();
  if (is_end_of_line[(unsigned char) *input_line_pointer])
    {
      input_line_pointer++;
    }
  else
    {
      ignore_rest_of_line ();
    }
  /* Return having already swallowed end-of-line. */
}				/* Return pointing just after end-of-line. */

void
ignore_rest_of_line ()		/* For suspect lines: gives warning. */
{
  if (!is_end_of_line[(unsigned char) *input_line_pointer])
    {
      if (isprint (*input_line_pointer))
	as_bad ("Rest of line ignored. First ignored character is `%c'.",
		*input_line_pointer);
      else
	as_bad ("Rest of line ignored. First ignored character valued 0x%x.",
		*input_line_pointer);
      while (input_line_pointer < buffer_limit
	     && !is_end_of_line[(unsigned char) *input_line_pointer])
	{
	  input_line_pointer++;
	}
    }
  input_line_pointer++;		/* Return pointing just after end-of-line. */
  know (is_end_of_line[(unsigned char) input_line_pointer[-1]]);
}

/*
 *			pseudo_set()
 *
 * In:	Pointer to a symbol.
 *	Input_line_pointer->expression.
 *
 * Out:	Input_line_pointer->just after any whitespace after expression.
 *	Tried to set symbol to value of expression.
 *	Will change symbols type, value, and frag;
 */
void
pseudo_set (symbolP)
     symbolS *symbolP;
{
  expressionS exp;
#if (defined (OBJ_AOUT) || defined (OBJ_BOUT)) && ! defined (BFD_ASSEMBLER)
  int ext;
#endif /* OBJ_AOUT or OBJ_BOUT */

  know (symbolP);		/* NULL pointer is logic error. */
#if (defined (OBJ_AOUT) || defined (OBJ_BOUT)) && ! defined (BFD_ASSEMBLER)
  ext = S_IS_EXTERNAL (symbolP);
#endif /* OBJ_AOUT or OBJ_BOUT */

  (void) expression (&exp);

  if (exp.X_op == O_illegal)
    as_bad ("illegal expression; zero assumed");
  else if (exp.X_op == O_absent)
    as_bad ("missing expression; zero assumed");
  else if (exp.X_op == O_big)
    as_bad ("%s number invalid; zero assumed",
	    exp.X_add_number > 0 ? "bignum" : "floating point");
  else if (exp.X_op == O_subtract
	   && (S_GET_SEGMENT (exp.X_add_symbol)
	       == S_GET_SEGMENT (exp.X_op_symbol))
	   && SEG_NORMAL (S_GET_SEGMENT (exp.X_add_symbol))
	   && exp.X_add_symbol->sy_frag == exp.X_op_symbol->sy_frag)
    {
      exp.X_op = O_constant;
      exp.X_add_number = (S_GET_VALUE (exp.X_add_symbol)
			  - S_GET_VALUE (exp.X_op_symbol));
    }

  switch (exp.X_op)
    {
    case O_illegal:
    case O_absent:
    case O_big:
      exp.X_add_number = 0;
      /* Fall through.  */
    case O_constant:
      S_SET_SEGMENT (symbolP, absolute_section);
#if (defined (OBJ_AOUT) || defined (OBJ_BOUT)) && ! defined (BFD_ASSEMBLER)
      if (ext)
	S_SET_EXTERNAL (symbolP);
      else
	S_CLEAR_EXTERNAL (symbolP);
#endif /* OBJ_AOUT or OBJ_BOUT */
      S_SET_VALUE (symbolP, (valueT) exp.X_add_number);
      symbolP->sy_frag = &zero_address_frag;
      break;

    case O_register:
      S_SET_SEGMENT (symbolP, reg_section);
      S_SET_VALUE (symbolP, (valueT) exp.X_add_number);
      symbolP->sy_frag = &zero_address_frag;
      break;

    case O_symbol:
      if (S_GET_SEGMENT (exp.X_add_symbol) == undefined_section
	  || exp.X_add_number != 0)
	symbolP->sy_value = exp;
      else
	{
	  symbolS *s = exp.X_add_symbol;

	  S_SET_SEGMENT (symbolP, S_GET_SEGMENT (s));
#if (defined (OBJ_AOUT) || defined (OBJ_BOUT)) && ! defined (BFD_ASSEMBLER)
	  if (ext)
	    S_SET_EXTERNAL (symbolP);
	  else
	    S_CLEAR_EXTERNAL (symbolP);
#endif /* OBJ_AOUT or OBJ_BOUT */
	  S_SET_VALUE (symbolP,
		       exp.X_add_number + S_GET_VALUE (s));
	  symbolP->sy_frag = s->sy_frag;
	  copy_symbol_attributes (symbolP, s);
	}
      break;

    default:
      /* The value is some complex expression.
	 FIXME: Should we set the segment to anything?  */
      symbolP->sy_value = exp;
      break;
    }
}

/*
 *			cons()
 *
 * CONStruct more frag of .bytes, or .words etc.
 * Should need_pass_2 be 1 then emit no frag(s).
 * This understands EXPRESSIONS.
 *
 * Bug (?)
 *
 * This has a split personality. We use expression() to read the
 * value. We can detect if the value won't fit in a byte or word.
 * But we can't detect if expression() discarded significant digits
 * in the case of a long. Not worth the crocks required to fix it.
 */

/* Select a parser for cons expressions.  */

/* Some targets need to parse the expression in various fancy ways.
   You can define TC_PARSE_CONS_EXPRESSION to do whatever you like
   (for example, the HPPA does this).  Otherwise, you can define
   BITFIELD_CONS_EXPRESSIONS to permit bitfields to be specified, or
   REPEAT_CONS_EXPRESSIONS to permit repeat counts.  If none of these
   are defined, which is the normal case, then only simple expressions
   are permitted.  */

static void
parse_mri_cons PARAMS ((expressionS *exp, unsigned int nbytes));

#ifndef TC_PARSE_CONS_EXPRESSION
#ifdef BITFIELD_CONS_EXPRESSIONS
#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) parse_bitfield_cons (EXP, NBYTES)
static void 
parse_bitfield_cons PARAMS ((expressionS *exp, unsigned int nbytes));
#endif
#ifdef REPEAT_CONS_EXPRESSIONS
#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) parse_repeat_cons (EXP, NBYTES)
static void
parse_repeat_cons PARAMS ((expressionS *exp, unsigned int nbytes));
#endif

/* If we haven't gotten one yet, just call expression.  */
#ifndef TC_PARSE_CONS_EXPRESSION
#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) expression (EXP)
#endif
#endif

/* worker to do .byte etc statements */
/* clobbers input_line_pointer, checks */
/* end-of-line. */
static void 
cons_worker (nbytes, rva)
     register int nbytes;	/* 1=.byte, 2=.word, 4=.long */
     int rva;
{
  int c;
  expressionS exp;
  char *stop = NULL;
  char stopc;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  if (is_it_end_of_statement ())
    {
      if (flag_mri)
	mri_comment_end (stop, stopc);
      demand_empty_rest_of_line ();
      return;
    }

#ifdef md_cons_align
  md_cons_align (nbytes);
#endif

  c = 0;
  do
    {
      if (flag_m68k_mri)
	parse_mri_cons (&exp, (unsigned int) nbytes);
      else
	TC_PARSE_CONS_EXPRESSION (&exp, (unsigned int) nbytes);

      if (rva)
	{
	  if (exp.X_op == O_symbol)
	    exp.X_op = O_symbol_rva;
	  else
	    as_fatal ("rva without symbol");
	}
      emit_expr (&exp, (unsigned int) nbytes);
      ++c;
    }
  while (*input_line_pointer++ == ',');

  /* In MRI mode, after an odd number of bytes, we must align to an
     even word boundary, unless the next instruction is a dc.b, ds.b
     or dcb.b.  */
  if (flag_mri && nbytes == 1 && (c & 1) != 0)
    mri_pending_align = 1;

  input_line_pointer--;		/* Put terminator back into stream. */

  if (flag_mri)
    mri_comment_end (stop, stopc);

  demand_empty_rest_of_line ();
}


void
cons (size)
     int size;
{
  cons_worker (size, 0);
}

void 
s_rva (size)
     int size;
{
  cons_worker (size, 1);
}


/* Put the contents of expression EXP into the object file using
   NBYTES bytes.  If need_pass_2 is 1, this does nothing.  */

void
emit_expr (exp, nbytes)
     expressionS *exp;
     unsigned int nbytes;
{
  operatorT op;
  register char *p;
  valueT extra_digit = 0;

  /* Don't do anything if we are going to make another pass.  */
  if (need_pass_2)
    return;

  op = exp->X_op;

  /* Allow `.word 0' in the absolute section.  */
  if (now_seg == absolute_section)
    {
      if (op != O_constant || exp->X_add_number != 0)
	as_bad ("attempt to store value in absolute section");
      abs_section_offset += nbytes;
      return;
    }

  /* Handle a negative bignum.  */
  if (op == O_uminus
      && exp->X_add_number == 0
      && exp->X_add_symbol->sy_value.X_op == O_big
      && exp->X_add_symbol->sy_value.X_add_number > 0)
    {
      int i;
      unsigned long carry;

      exp = &exp->X_add_symbol->sy_value;

      /* Negate the bignum: one's complement each digit and add 1.  */
      carry = 1;
      for (i = 0; i < exp->X_add_number; i++)
	{
	  unsigned long next;

	  next = (((~ (generic_bignum[i] & LITTLENUM_MASK))
		   & LITTLENUM_MASK)
		  + carry);
	  generic_bignum[i] = next & LITTLENUM_MASK;
	  carry = next >> LITTLENUM_NUMBER_OF_BITS;
	}

      /* We can ignore any carry out, because it will be handled by
	 extra_digit if it is needed.  */

      extra_digit = (valueT) -1;
      op = O_big;
    }

  if (op == O_absent || op == O_illegal)
    {
      as_warn ("zero assumed for missing expression");
      exp->X_add_number = 0;
      op = O_constant;
    }
  else if (op == O_big && exp->X_add_number <= 0)
    {
      as_bad ("floating point number invalid; zero assumed");
      exp->X_add_number = 0;
      op = O_constant;
    }
  else if (op == O_register)
    {
      as_warn ("register value used as expression");
      op = O_constant;
    }

  p = frag_more ((int) nbytes);

#ifndef WORKING_DOT_WORD
  /* If we have the difference of two symbols in a word, save it on
     the broken_words list.  See the code in write.c.  */
  if (op == O_subtract && nbytes == 2)
    {
      struct broken_word *x;

      x = (struct broken_word *) xmalloc (sizeof (struct broken_word));
      x->next_broken_word = broken_words;
      broken_words = x;
      x->frag = frag_now;
      x->word_goes_here = p;
      x->dispfrag = 0;
      x->add = exp->X_add_symbol;
      x->sub = exp->X_op_symbol;
      x->addnum = exp->X_add_number;
      x->added = 0;
      new_broken_words++;
      return;
    }
#endif

  /* If we have an integer, but the number of bytes is too large to
     pass to md_number_to_chars, handle it as a bignum.  */
  if (op == O_constant && nbytes > sizeof (valueT))
    {
      valueT val;
      int gencnt;

      if (! exp->X_unsigned && exp->X_add_number < 0)
	extra_digit = (valueT) -1;
      val = (valueT) exp->X_add_number;
      gencnt = 0;
      do
	{
	  generic_bignum[gencnt] = val & LITTLENUM_MASK;
	  val >>= LITTLENUM_NUMBER_OF_BITS;
	  ++gencnt;
	}
      while (val != 0);
      op = exp->X_op = O_big;
      exp->X_add_number = gencnt;
    }

  if (op == O_constant)
    {
      register valueT get;
      register valueT use;
      register valueT mask;
      register valueT unmask;

      /* JF << of >= number of bits in the object is undefined.  In
	 particular SPARC (Sun 4) has problems */
      if (nbytes >= sizeof (valueT))
	mask = 0;
      else
	mask = ~(valueT) 0 << (BITS_PER_CHAR * nbytes);	/* Don't store these bits. */

      unmask = ~mask;		/* Do store these bits. */

#ifdef NEVER
      "Do this mod if you want every overflow check to assume SIGNED 2's complement data.";
      mask = ~(unmask >> 1);	/* Includes sign bit now. */
#endif

      get = exp->X_add_number;
      use = get & unmask;
      if ((get & mask) != 0 && (get & mask) != mask)
	{		/* Leading bits contain both 0s & 1s. */
	  as_warn ("Value 0x%lx truncated to 0x%lx.",
		   (unsigned long) get, (unsigned long) use);
	}
      /* put bytes in right order. */
      md_number_to_chars (p, use, (int) nbytes);
    }
  else if (op == O_big)
    {
      int size;
      LITTLENUM_TYPE *nums;

      know (nbytes % CHARS_PER_LITTLENUM == 0);

      size = exp->X_add_number * CHARS_PER_LITTLENUM;
      if (nbytes < size)
	{
	  as_warn ("Bignum truncated to %d bytes", nbytes);
	  size = nbytes;
	}

      if (target_big_endian)
	{
	  while (nbytes > size)
	    {
	      md_number_to_chars (p, extra_digit, CHARS_PER_LITTLENUM);
	      nbytes -= CHARS_PER_LITTLENUM;
	      p += CHARS_PER_LITTLENUM;
	    }

	  nums = generic_bignum + size / CHARS_PER_LITTLENUM;
	  while (size > 0)
	    {
	      --nums;
	      md_number_to_chars (p, (valueT) *nums, CHARS_PER_LITTLENUM);
	      size -= CHARS_PER_LITTLENUM;
	      p += CHARS_PER_LITTLENUM;
	    }
	}
      else
	{
	  nums = generic_bignum;
	  while (size > 0)
	    {
	      md_number_to_chars (p, (valueT) *nums, CHARS_PER_LITTLENUM);
	      ++nums;
	      size -= CHARS_PER_LITTLENUM;
	      p += CHARS_PER_LITTLENUM;
	      nbytes -= CHARS_PER_LITTLENUM;
	    }

	  while (nbytes > 0)
	    {
	      md_number_to_chars (p, extra_digit, CHARS_PER_LITTLENUM);
	      nbytes -= CHARS_PER_LITTLENUM;
	      p += CHARS_PER_LITTLENUM;
	    }
	}
    }
  else
    {
      memset (p, 0, nbytes);

      /* Now we need to generate a fixS to record the symbol value.
	 This is easy for BFD.  For other targets it can be more
	 complex.  For very complex cases (currently, the HPPA and
	 NS32K), you can define TC_CONS_FIX_NEW to do whatever you
	 want.  For simpler cases, you can define TC_CONS_RELOC to be
	 the name of the reloc code that should be stored in the fixS.
	 If neither is defined, the code uses NO_RELOC if it is
	 defined, and otherwise uses 0.  */

#ifdef BFD_ASSEMBLER
#ifdef TC_CONS_FIX_NEW
      TC_CONS_FIX_NEW (frag_now, p - frag_now->fr_literal, nbytes, exp);
#else
      fix_new_exp (frag_now, p - frag_now->fr_literal, (int) nbytes, exp, 0,
		   /* @@ Should look at CPU word size.  */
		   nbytes == 2 ? BFD_RELOC_16
		      : nbytes == 8 ? BFD_RELOC_64
		      : BFD_RELOC_32);
#endif
#else
#ifdef TC_CONS_FIX_NEW
      TC_CONS_FIX_NEW (frag_now, p - frag_now->fr_literal, nbytes, exp);
#else
      /* Figure out which reloc number to use.  Use TC_CONS_RELOC if
	 it is defined, otherwise use NO_RELOC if it is defined,
	 otherwise use 0.  */
#ifndef TC_CONS_RELOC
#ifdef NO_RELOC
#define TC_CONS_RELOC NO_RELOC
#else
#define TC_CONS_RELOC 0
#endif
#endif
      fix_new_exp (frag_now, p - frag_now->fr_literal, (int) nbytes, exp, 0,
		   TC_CONS_RELOC);
#endif /* TC_CONS_FIX_NEW */
#endif /* BFD_ASSEMBLER */
    }
}

#ifdef BITFIELD_CONS_EXPRESSIONS

/* i960 assemblers, (eg, asm960), allow bitfields after ".byte" as
   w:x,y:z, where w and y are bitwidths and x and y are values.  They
   then pack them all together. We do a little better in that we allow
   them in words, longs, etc. and we'll pack them in target byte order
   for you.

   The rules are: pack least significat bit first, if a field doesn't
   entirely fit, put it in the next unit.  Overflowing the bitfield is
   explicitly *not* even a warning.  The bitwidth should be considered
   a "mask".

   To use this function the tc-XXX.h file should define
   BITFIELD_CONS_EXPRESSIONS.  */

static void 
parse_bitfield_cons (exp, nbytes)
     expressionS *exp;
     unsigned int nbytes;
{
  unsigned int bits_available = BITS_PER_CHAR * nbytes;
  char *hold = input_line_pointer;

  (void) expression (exp);

  if (*input_line_pointer == ':')
    {			/* bitfields */
      long value = 0;

      for (;;)
	{
	  unsigned long width;

	  if (*input_line_pointer != ':')
	    {
	      input_line_pointer = hold;
	      break;
	    }			/* next piece is not a bitfield */

	  /* In the general case, we can't allow
	     full expressions with symbol
	     differences and such.  The relocation
	     entries for symbols not defined in this
	     assembly would require arbitrary field
	     widths, positions, and masks which most
	     of our current object formats don't
	     support.

	     In the specific case where a symbol
	     *is* defined in this assembly, we
	     *could* build fixups and track it, but
	     this could lead to confusion for the
	     backends.  I'm lazy. I'll take any
	     SEG_ABSOLUTE. I think that means that
	     you can use a previous .set or
	     .equ type symbol.  xoxorich. */

	  if (exp->X_op == O_absent)
	    {
	      as_warn ("using a bit field width of zero");
	      exp->X_add_number = 0;
	      exp->X_op = O_constant;
	    }			/* implied zero width bitfield */

	  if (exp->X_op != O_constant)
	    {
	      *input_line_pointer = '\0';
	      as_bad ("field width \"%s\" too complex for a bitfield", hold);
	      *input_line_pointer = ':';
	      demand_empty_rest_of_line ();
	      return;
	    }			/* too complex */

	  if ((width = exp->X_add_number) > (BITS_PER_CHAR * nbytes))
	    {
	      as_warn ("field width %lu too big to fit in %d bytes: truncated to %d bits",
		       width, nbytes, (BITS_PER_CHAR * nbytes));
	      width = BITS_PER_CHAR * nbytes;
	    }			/* too big */

	  if (width > bits_available)
	    {
	      /* FIXME-SOMEDAY: backing up and reparsing is wasteful.  */
	      input_line_pointer = hold;
	      exp->X_add_number = value;
	      break;
	    }			/* won't fit */

	  hold = ++input_line_pointer; /* skip ':' */

	  (void) expression (exp);
	  if (exp->X_op != O_constant)
	    {
	      char cache = *input_line_pointer;

	      *input_line_pointer = '\0';
	      as_bad ("field value \"%s\" too complex for a bitfield", hold);
	      *input_line_pointer = cache;
	      demand_empty_rest_of_line ();
	      return;
	    }			/* too complex */

	  value |= ((~(-1 << width) & exp->X_add_number)
		    << ((BITS_PER_CHAR * nbytes) - bits_available));

	  if ((bits_available -= width) == 0
	      || is_it_end_of_statement ()
	      || *input_line_pointer != ',')
	    {
	      break;
	    }			/* all the bitfields we're gonna get */

	  hold = ++input_line_pointer;
	  (void) expression (exp);
	}			/* forever loop */

      exp->X_add_number = value;
      exp->X_op = O_constant;
      exp->X_unsigned = 1;
    }				/* if looks like a bitfield */
}				/* parse_bitfield_cons() */

#endif /* BITFIELD_CONS_EXPRESSIONS */

/* Handle an MRI style string expression.  */

static void
parse_mri_cons (exp, nbytes)
     expressionS *exp;
     unsigned int nbytes;
{
  if (*input_line_pointer != '\''
      && (input_line_pointer[1] != '\''
	  || (*input_line_pointer != 'A'
	      && *input_line_pointer != 'E')))
    TC_PARSE_CONS_EXPRESSION (exp, nbytes);
  else
    {
      int scan = 0;
      unsigned int result = 0;

      /* An MRI style string.  Cut into as many bytes as will fit into
	 a nbyte chunk, left justify if necessary, and separate with
	 commas so we can try again later.  */
      if (*input_line_pointer == 'A')
	++input_line_pointer;
      else if (*input_line_pointer == 'E')
	{
	  as_bad ("EBCDIC constants are not supported");
	  ++input_line_pointer;
	}

      input_line_pointer++;
      for (scan = 0; scan < nbytes; scan++)
	{
	  if (*input_line_pointer == '\'')
	    {
	      if (input_line_pointer[1] == '\'')
		{
		  input_line_pointer++;
		}
	      else
		break;
	    }
	  result = (result << 8) | (*input_line_pointer++);
	}

      /* Left justify */
      while (scan < nbytes)
	{
	  result <<= 8;
	  scan++;
	}
      /* Create correct expression */
      exp->X_op = O_constant;
      exp->X_add_number = result;
      /* Fake it so that we can read the next char too */
      if (input_line_pointer[0] != '\'' ||
	  (input_line_pointer[0] == '\'' && input_line_pointer[1] == '\''))
	{
	  input_line_pointer -= 2;
	  input_line_pointer[0] = ',';
	  input_line_pointer[1] = '\'';
	}
      else
	input_line_pointer++;
    }
}

#ifdef REPEAT_CONS_EXPRESSIONS

/* Parse a repeat expression for cons.  This is used by the MIPS
   assembler.  The format is NUMBER:COUNT; NUMBER appears in the
   object file COUNT times.

   To use this for a target, define REPEAT_CONS_EXPRESSIONS.  */

static void
parse_repeat_cons (exp, nbytes)
     expressionS *exp;
     unsigned int nbytes;
{
  expressionS count;
  register int i;

  expression (exp);

  if (*input_line_pointer != ':')
    {
      /* No repeat count.  */
      return;
    }

  ++input_line_pointer;
  expression (&count);
  if (count.X_op != O_constant
      || count.X_add_number <= 0)
    {
      as_warn ("Unresolvable or nonpositive repeat count; using 1");
      return;
    }

  /* The cons function is going to output this expression once.  So we
     output it count - 1 times.  */
  for (i = count.X_add_number - 1; i > 0; i--)
    emit_expr (exp, nbytes);
}

#endif /* REPEAT_CONS_EXPRESSIONS */

/* Parse a floating point number represented as a hex constant.  This
   permits users to specify the exact bits they want in the floating
   point number.  */

static int
hex_float (float_type, bytes)
     int float_type;
     char *bytes;
{
  int length;
  int i;

  switch (float_type)
    {
    case 'f':
    case 'F':
    case 's':
    case 'S':
      length = 4;
      break;

    case 'd':
    case 'D':
    case 'r':
    case 'R':
      length = 8;
      break;

    case 'x':
    case 'X':
      length = 12;
      break;

    case 'p':
    case 'P':
      length = 12;
      break;

    default:
      as_bad ("Unknown floating type type '%c'", float_type);
      return -1;
    }

  /* It would be nice if we could go through expression to parse the
     hex constant, but if we get a bignum it's a pain to sort it into
     the buffer correctly.  */
  i = 0;
  while (hex_p (*input_line_pointer) || *input_line_pointer == '_')
    {
      int d;

      /* The MRI assembler accepts arbitrary underscores strewn about
	 through the hex constant, so we ignore them as well. */
      if (*input_line_pointer == '_')
	{
	  ++input_line_pointer;
	  continue;
	}

      if (i >= length)
	{
	  as_warn ("Floating point constant too large");
	  return -1;
	}
      d = hex_value (*input_line_pointer) << 4;
      ++input_line_pointer;
      while (*input_line_pointer == '_')
	++input_line_pointer;
      if (hex_p (*input_line_pointer))
	{
	  d += hex_value (*input_line_pointer);
	  ++input_line_pointer;
	}
      if (target_big_endian)
	bytes[i] = d;
      else
	bytes[length - i - 1] = d;
      ++i;
    }

  if (i < length)
    {
      if (target_big_endian)
	memset (bytes + i, 0, length - i);
      else
	memset (bytes, 0, length - i);
    }

  return length;
}

/*
 *			float_cons()
 *
 * CONStruct some more frag chars of .floats .ffloats etc.
 * Makes 0 or more new frags.
 * If need_pass_2 == 1, no frags are emitted.
 * This understands only floating literals, not expressions. Sorry.
 *
 * A floating constant is defined by atof_generic(), except it is preceded
 * by 0d 0f 0g or 0h. After observing the STRANGE way my BSD AS does its
 * reading, I decided to be incompatible. This always tries to give you
 * rounded bits to the precision of the pseudo-op. Former AS did premature
 * truncatation, restored noisy bits instead of trailing 0s AND gave you
 * a choice of 2 flavours of noise according to which of 2 floating-point
 * scanners you directed AS to use.
 *
 * In:	input_line_pointer->whitespace before, or '0' of flonum.
 *
 */

void
float_cons (float_type)
     /* Clobbers input_line-pointer, checks end-of-line. */
     register int float_type;	/* 'f':.ffloat ... 'F':.float ... */
{
  register char *p;
  int length;			/* Number of chars in an object. */
  register char *err;		/* Error from scanning floating literal. */
  char temp[MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT];

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

  do
    {
      /* input_line_pointer->1st char of a flonum (we hope!). */
      SKIP_WHITESPACE ();

      /* Skip any 0{letter} that may be present. Don't even check if the
       * letter is legal. Someone may invent a "z" format and this routine
       * has no use for such information. Lusers beware: you get
       * diagnostics if your input is ill-conditioned.
       */
      if (input_line_pointer[0] == '0' && isalpha (input_line_pointer[1]))
	input_line_pointer += 2;

      /* Accept :xxxx, where the x's are hex digits, for a floating
         point with the exact digits specified.  */
      if (input_line_pointer[0] == ':')
	{
	  ++input_line_pointer;
	  length = hex_float (float_type, temp);
	  if (length < 0)
	    {
	      ignore_rest_of_line ();
	      return;
	    }
	}
      else
	{
	  err = md_atof (float_type, temp, &length);
	  know (length <= MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT);
	  know (length > 0);
	  if (err)
	    {
	      as_bad ("Bad floating literal: %s", err);
	      ignore_rest_of_line ();
	      return;
	    }
	}

      if (!need_pass_2)
	{
	  int count;

	  count = 1;

#ifdef REPEAT_CONS_EXPRESSIONS
	  if (*input_line_pointer == ':')
	    {
	      expressionS count_exp;

	      ++input_line_pointer;
	      expression (&count_exp);
	      if (count_exp.X_op != O_constant
		  || count_exp.X_add_number <= 0)
		{
		  as_warn ("unresolvable or nonpositive repeat count; using 1");
		}
	      else
		count = count_exp.X_add_number;
	    }
#endif

	  while (--count >= 0)
	    {
	      p = frag_more (length);
	      memcpy (p, temp, (unsigned int) length);
	    }
	}
      SKIP_WHITESPACE ();
    }
  while (*input_line_pointer++ == ',');

  --input_line_pointer;		/* Put terminator back into stream.  */
  demand_empty_rest_of_line ();
}				/* float_cons() */

/*
 *			stringer()
 *
 * We read 0 or more ',' seperated, double-quoted strings.
 *
 * Caller should have checked need_pass_2 is FALSE because we don't check it.
 */


void 
stringer (append_zero)		/* Worker to do .ascii etc statements. */
     /* Checks end-of-line. */
     register int append_zero;	/* 0: don't append '\0', else 1 */
{
  register unsigned int c;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  /*
   * The following awkward logic is to parse ZERO or more strings,
   * comma seperated. Recall a string expression includes spaces
   * before the opening '\"' and spaces after the closing '\"'.
   * We fake a leading ',' if there is (supposed to be)
   * a 1st, expression. We keep demanding expressions for each
   * ','.
   */
  if (is_it_end_of_statement ())
    {
      c = 0;			/* Skip loop. */
      ++input_line_pointer;	/* Compensate for end of loop. */
    }
  else
    {
      c = ',';			/* Do loop. */
    }
  while (c == ',' || c == '<' || c == '"')
    {
      SKIP_WHITESPACE ();
      switch (*input_line_pointer)
	{
	case '\"':
	  ++input_line_pointer;	/*->1st char of string. */
	  while (is_a_char (c = next_char_of_string ()))
	    {
	      FRAG_APPEND_1_CHAR (c);
	    }
	  if (append_zero)
	    {
	      FRAG_APPEND_1_CHAR (0);
	    }
	  know (input_line_pointer[-1] == '\"');
	  break;
	case '<':
	  input_line_pointer++;
	  c = get_single_number ();
	  FRAG_APPEND_1_CHAR (c);
	  if (*input_line_pointer != '>')
	    {
	      as_bad ("Expected <nn>");
	    }
	  input_line_pointer++;
	  break;
	case ',':
	  input_line_pointer++;
	  break;
	}
      SKIP_WHITESPACE ();
      c = *input_line_pointer;
    }

  demand_empty_rest_of_line ();
}				/* stringer() */

/* FIXME-SOMEDAY: I had trouble here on characters with the
    high bits set.  We'll probably also have trouble with
    multibyte chars, wide chars, etc.  Also be careful about
    returning values bigger than 1 byte.  xoxorich. */

unsigned int 
next_char_of_string ()
{
  register unsigned int c;

  c = *input_line_pointer++ & CHAR_MASK;
  switch (c)
    {
    case '\"':
      c = NOT_A_CHAR;
      break;

    case '\n':
      as_warn ("Unterminated string: Newline inserted.");
      bump_line_counters ();
      break;

#ifndef NO_STRING_ESCAPES
    case '\\':
      switch (c = *input_line_pointer++)
	{
	case 'b':
	  c = '\b';
	  break;

	case 'f':
	  c = '\f';
	  break;

	case 'n':
	  c = '\n';
	  break;

	case 'r':
	  c = '\r';
	  break;

	case 't':
	  c = '\t';
	  break;

	case 'v':
	  c = '\013';
	  break;

	case '\\':
	case '"':
	  break;		/* As itself. */

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  {
	    long number;
	    int i;

	    for (i = 0, number = 0; isdigit (c) && i < 3; c = *input_line_pointer++, i++)
	      {
		number = number * 8 + c - '0';
	      }
	    c = number & 0xff;
	  }
	  --input_line_pointer;
	  break;

	case 'x':
	case 'X':
	  {
	    long number;

	    number = 0;
	    c = *input_line_pointer++;
	    while (isxdigit (c))
	      {
		if (isdigit (c))
		  number = number * 16 + c - '0';
		else if (isupper (c))
		  number = number * 16 + c - 'A' + 10;
		else
		  number = number * 16 + c - 'a' + 10;
		c = *input_line_pointer++;
	      }
	    c = number & 0xff;
	    --input_line_pointer;
	  }
	  break;

	case '\n':
	  /* To be compatible with BSD 4.2 as: give the luser a linefeed!! */
	  as_warn ("Unterminated string: Newline inserted.");
	  c = '\n';
	  bump_line_counters ();
	  break;

	default:

#ifdef ONLY_STANDARD_ESCAPES
	  as_bad ("Bad escaped character in string, '?' assumed");
	  c = '?';
#endif /* ONLY_STANDARD_ESCAPES */

	  break;
	}			/* switch on escaped char */
      break;
#endif /* ! defined (NO_STRING_ESCAPES) */

    default:
      break;
    }				/* switch on char */
  return (c);
}				/* next_char_of_string() */

static segT
get_segmented_expression (expP)
     register expressionS *expP;
{
  register segT retval;

  retval = expression (expP);
  if (expP->X_op == O_illegal
      || expP->X_op == O_absent
      || expP->X_op == O_big)
    {
      as_bad ("expected address expression; zero assumed");
      expP->X_op = O_constant;
      expP->X_add_number = 0;
      retval = absolute_section;
    }
  return retval;
}

static segT 
get_known_segmented_expression (expP)
     register expressionS *expP;
{
  register segT retval;

  if ((retval = get_segmented_expression (expP)) == undefined_section)
    {
      /* There is no easy way to extract the undefined symbol from the
	 expression.  */
      if (expP->X_add_symbol != NULL
	  && S_GET_SEGMENT (expP->X_add_symbol) != expr_section)
	as_warn ("symbol \"%s\" undefined; zero assumed",
		 S_GET_NAME (expP->X_add_symbol));
      else
	as_warn ("some symbol undefined; zero assumed");
      retval = absolute_section;
      expP->X_op = O_constant;
      expP->X_add_number = 0;
    }
  know (retval == absolute_section || SEG_NORMAL (retval));
  return (retval);
}				/* get_known_segmented_expression() */

offsetT
get_absolute_expression ()
{
  expressionS exp;

  expression (&exp);
  if (exp.X_op != O_constant)
    {
      if (exp.X_op != O_absent)
	as_bad ("bad or irreducible absolute expression; zero assumed");
      exp.X_add_number = 0;
    }
  return exp.X_add_number;
}

char				/* return terminator */
get_absolute_expression_and_terminator (val_pointer)
     long *val_pointer;		/* return value of expression */
{
  /* FIXME: val_pointer should probably be offsetT *.  */
  *val_pointer = (long) get_absolute_expression ();
  return (*input_line_pointer++);
}

/*
 *			demand_copy_C_string()
 *
 * Like demand_copy_string, but return NULL if the string contains any '\0's.
 * Give a warning if that happens.
 */
char *
demand_copy_C_string (len_pointer)
     int *len_pointer;
{
  register char *s;

  if ((s = demand_copy_string (len_pointer)) != 0)
    {
      register int len;

      for (len = *len_pointer; len > 0; len--)
	{
	  if (*s == 0)
	    {
	      s = 0;
	      len = 1;
	      *len_pointer = 0;
	      as_bad ("This string may not contain \'\\0\'");
	    }
	}
    }
  return s;
}

/*
 *			demand_copy_string()
 *
 * Demand string, but return a safe (=private) copy of the string.
 * Return NULL if we can't read a string here.
 */
char *
demand_copy_string (lenP)
     int *lenP;
{
  register unsigned int c;
  register int len;
  char *retval;

  len = 0;
  SKIP_WHITESPACE ();
  if (*input_line_pointer == '\"')
    {
      input_line_pointer++;	/* Skip opening quote. */

      while (is_a_char (c = next_char_of_string ()))
	{
	  obstack_1grow (&notes, c);
	  len++;
	}
      /* JF this next line is so demand_copy_C_string will return a
	 null terminated string. */
      obstack_1grow (&notes, '\0');
      retval = obstack_finish (&notes);
    }
  else
    {
      as_warn ("Missing string");
      retval = NULL;
      ignore_rest_of_line ();
    }
  *lenP = len;
  return (retval);
}				/* demand_copy_string() */

/*
 *		is_it_end_of_statement()
 *
 * In:	Input_line_pointer->next character.
 *
 * Do:	Skip input_line_pointer over all whitespace.
 *
 * Out:	1 if input_line_pointer->end-of-line.
*/
int 
is_it_end_of_statement ()
{
  SKIP_WHITESPACE ();
  return (is_end_of_line[(unsigned char) *input_line_pointer]);
}				/* is_it_end_of_statement() */

void 
equals (sym_name)
     char *sym_name;
{
  register symbolS *symbolP;	/* symbol we are working with */
  char *stop;
  char stopc;

  input_line_pointer++;
  if (*input_line_pointer == '=')
    input_line_pointer++;

  while (*input_line_pointer == ' ' || *input_line_pointer == '\t')
    input_line_pointer++;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  if (sym_name[0] == '.' && sym_name[1] == '\0')
    {
      /* Turn '. = mumble' into a .org mumble */
      register segT segment;
      expressionS exp;

      segment = get_known_segmented_expression (&exp);
      if (!need_pass_2)
	do_org (segment, &exp, 0);
    }
  else
    {
      symbolP = symbol_find_or_make (sym_name);
      pseudo_set (symbolP);
    }

  if (flag_mri)
    mri_comment_end (stop, stopc);
}				/* equals() */

/* .include -- include a file at this point. */

/* ARGSUSED */
void 
s_include (arg)
     int arg;
{
  char *newbuf;
  char *filename;
  int i;
  FILE *try;
  char *path;

  if (! flag_m68k_mri)
    filename = demand_copy_string (&i);
  else
    {
      SKIP_WHITESPACE ();
      i = 0;
      while (! is_end_of_line[(unsigned char) *input_line_pointer]
	     && *input_line_pointer != ' '
	     && *input_line_pointer != '\t')
	{
	  obstack_1grow (&notes, *input_line_pointer);
	  ++input_line_pointer;
	  ++i;
	}
      obstack_1grow (&notes, '\0');
      filename = obstack_finish (&notes);
      while (! is_end_of_line[(unsigned char) *input_line_pointer])
	++input_line_pointer;
    }
  demand_empty_rest_of_line ();
  path = xmalloc ((unsigned long) i + include_dir_maxlen + 5 /* slop */ );
  for (i = 0; i < include_dir_count; i++)
    {
      strcpy (path, include_dirs[i]);
      strcat (path, "/");
      strcat (path, filename);
      if (0 != (try = fopen (path, "r")))
	{
	  fclose (try);
	  goto gotit;
	}
    }
  free (path);
  path = filename;
gotit:
  /* malloc Storage leak when file is found on path.  FIXME-SOMEDAY. */
  newbuf = input_scrub_include_file (path, input_line_pointer);
  buffer_limit = input_scrub_next_buffer (&input_line_pointer);
}				/* s_include() */

void 
add_include_dir (path)
     char *path;
{
  int i;

  if (include_dir_count == 0)
    {
      include_dirs = (char **) xmalloc (2 * sizeof (*include_dirs));
      include_dirs[0] = ".";	/* Current dir */
      include_dir_count = 2;
    }
  else
    {
      include_dir_count++;
      include_dirs = (char **) realloc (include_dirs,
				include_dir_count * sizeof (*include_dirs));
    }

  include_dirs[include_dir_count - 1] = path;	/* New one */

  i = strlen (path);
  if (i > include_dir_maxlen)
    include_dir_maxlen = i;
}				/* add_include_dir() */

void 
s_ignore (arg)
     int arg;
{
  while (!is_end_of_line[(unsigned char) *input_line_pointer])
    {
      ++input_line_pointer;
    }
  ++input_line_pointer;
}


void
read_print_statistics (file)
     FILE *file;
{
  hash_print_statistics (file, "pseudo-op table", po_hash);
}

/* end of read.c */
