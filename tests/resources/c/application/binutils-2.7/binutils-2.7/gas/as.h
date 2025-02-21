/* as.h - global header file
   Copyright (C) 1987, 90, 91, 92, 93, 94, 95, 1996
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
   the Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef GAS
#define GAS 1
/*
 * I think this stuff is largely out of date.  xoxorich.
 *
 * CAPITALISED names are #defined.
 * "lowercaseH" is #defined if "lowercase.h" has been #include-d.
 * "lowercaseT" is a typedef of "lowercase" objects.
 * "lowercaseP" is type "pointer to object of type 'lowercase'".
 * "lowercaseS" is typedef struct ... lowercaseS.
 *
 * #define DEBUG to enable all the "know" assertion tests.
 * #define SUSPECT when debugging hash code.
 * #define COMMON as "extern" for all modules except one, where you #define
 *	COMMON as "".
 * If TEST is #defined, then we are testing a module: #define COMMON as "".
 */

#include "config.h"

/* This is the code recommended in the autoconf documentation, almost
   verbatim.  If it doesn't work for you, let me know, and notify
   djm@gnu.ai.mit.edu as well.  */
/* Added #undef for DJ Delorie.  The right fix is to ensure that as.h
   is included first, before even any system header files, in all files
   that use it.  KR 1994.11.03 */
/* Added void* version for STDC case.  This is to be compatible with
   the declaration in bison.simple, used for m68k operand parsing.
   --KR 1995.08.08 */
/* Force void* decl for hpux.  This is what Bison uses.  --KR 1995.08.16 */

/* AIX requires this to be the first thing in the file.  */
#ifdef __GNUC__
# undef alloca
# define alloca __builtin_alloca
#else
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
 #pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
#    if !defined (__STDC__) && !defined (__hpux)
char *alloca ();
#    else
void *alloca ();
#    endif /* __STDC__, __hpux */
#   endif /* alloca */
#  endif /* _AIX */
# endif /* HAVE_ALLOCA_H */
#endif

/* Now, tend to the rest of the configuration.  */

/* System include files first... */
#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
/* for size_t, pid_t */
#include <sys/types.h>
#endif

#include <getopt.h>
/* The first getopt value for machine-independent long options.
   150 isn't special; it's just an arbitrary non-ASCII char value.  */
#define OPTION_STD_BASE 150
/* The first getopt value for machine-dependent long options.
   170 gives the standard options room to grow.  */
#define OPTION_MD_BASE 170

#ifdef DEBUG
#undef NDEBUG
#endif
#if !defined (__GNUC__) || __GNUC_MINOR__ <= 5
#define __PRETTY_FUNCTION__  ((char*)0)
#endif
#if 0

/* Handle lossage with assert.h.  */
#ifndef BROKEN_ASSERT
#include <assert.h>
#else /* BROKEN_ASSERT */
#ifndef NDEBUG
#define assert(p) ((p) ? 0 : (as_assert (__FILE__, __LINE__, __PRETTY_FUNCTION__), 0))
#else
#define assert(p) ((p), 0)
#endif
#endif /* BROKEN_ASSERT */

#else

#define assert(P) ((P) ? 0 : (as_assert (__FILE__, __LINE__, __PRETTY_FUNCTION__), 0))
#undef abort
#define abort()		as_abort (__FILE__, __LINE__, __PRETTY_FUNCTION__)

#endif


/* Now GNU header files... */
#include <ansidecl.h>
#ifdef BFD_ASSEMBLER
#include <bfd.h>
#endif

/* Define the standard progress macros.  */
#include <progress.h>

/* This doesn't get taken care of anywhere.  */
#ifndef __MWERKS__  /* Metrowerks C chokes on the "defined (inline)" */
#if !defined (__GNUC__) && !defined (inline)
#define inline
#endif
#endif /* !__MWERKS__ */

/* Other stuff from config.h.  */
#ifdef NEED_DECLARATION_MALLOC
extern PTR malloc ();
extern PTR realloc ();
#endif
#ifdef NEED_DECLARATION_FREE
extern void free ();
#endif
#ifdef NEED_DECLARATION_ERRNO
extern int errno;
#endif

/* This is needed for VMS.  */
#if ! defined (HAVE_UNLINK) && defined (HAVE_REMOVE)
#define unlink remove
#endif

/* Hack to make "gcc -Wall" not complain about obstack macros.  */
#if !defined (memcpy) && !defined (bcopy)
#define bcopy(src,dest,size)	memcpy(dest,src,size)
#endif

#ifdef BFD_ASSEMBLER
/* This one doesn't get declared, but we're using it anyways.  This
   should be fixed -- either it's part of the external interface or
   it's not.  */
extern PTR bfd_alloc_by_size_t PARAMS ((bfd *abfd, size_t sz));
#endif

/* Make Saber happier on obstack.h.  */
#ifdef SABER
#undef  __PTR_TO_INT
#define __PTR_TO_INT(P) ((int)(P))
#undef  __INT_TO_PTR
#define __INT_TO_PTR(P) ((char *)(P))
#endif

#ifndef __LINE__
#define __LINE__ "unknown"
#endif /* __LINE__ */

#ifndef __FILE__
#define __FILE__ "unknown"
#endif /* __FILE__ */

#ifndef FOPEN_WB
#ifdef GO32
#include "fopen-bin.h"
#else
#include "fopen-same.h"
#endif
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif

#define obstack_chunk_alloc xmalloc
#define obstack_chunk_free xfree

#define xfree free

#define BAD_CASE(val) \
{ \
      as_fatal("Case value %ld unexpected at line %d of file \"%s\"\n", \
	       (long) val, __LINE__, __FILE__); \
	   }

/* Version 2.1 of Solaris had problems with this declaration, but I
   think that bug has since been fixed.  If it causes problems on your
   system, just delete it.  */
extern char *strstr ();

#include "flonum.h"

/* These are assembler-wide concepts */

#ifdef BFD_ASSEMBLER
extern bfd *stdoutput;
typedef bfd_vma addressT;
typedef bfd_signed_vma offsetT;
#else
typedef unsigned long addressT;
typedef long offsetT;
#endif

/* Type of symbol value, etc.  For use in prototypes.  */
typedef addressT valueT;

#ifndef COMMON
#ifdef TEST
#define COMMON			/* declare our COMMONs storage here. */
#else
#define COMMON extern		/* our commons live elswhere */
#endif
#endif
/* COMMON now defined */

#ifdef DEBUG
#ifndef know
#define know(p) assert(p)	/* Verify our assumptions! */
#endif /* not yet defined */
#else
#define know(p)			/* know() checks are no-op.ed */
#endif

/* input_scrub.c */

/*
 * Supplies sanitised buffers to read.c.
 * Also understands printing line-number part of error messages.
 */


/* subsegs.c     Sub-segments. Also, segment(=expression type)s.*/

#ifndef BFD_ASSEMBLER

#ifdef MANY_SEGMENTS
#include "bfd.h"
#define N_SEGMENTS 40
#define SEG_NORMAL(x) ((x) >= SEG_E0 && (x) <= SEG_E39)
#define SEG_LIST SEG_E0,SEG_E1,SEG_E2,SEG_E3,SEG_E4,SEG_E5,SEG_E6,SEG_E7,SEG_E8,SEG_E9,\
		 SEG_E10,SEG_E11,SEG_E12,SEG_E13,SEG_E14,SEG_E15,SEG_E16,SEG_E17,SEG_E18,SEG_E19,\
		 SEG_E20,SEG_E21,SEG_E22,SEG_E23,SEG_E24,SEG_E25,SEG_E26,SEG_E27,SEG_E28,SEG_E29,\
		 SEG_E30,SEG_E31,SEG_E32,SEG_E33,SEG_E34,SEG_E35,SEG_E36,SEG_E37,SEG_E38,SEG_E39
#define SEG_TEXT SEG_E0
#define SEG_DATA SEG_E1
#define SEG_BSS SEG_E2
#define SEG_LAST SEG_E39
#else
#define N_SEGMENTS 3
#define SEG_NORMAL(x) ((x) == SEG_TEXT || (x) == SEG_DATA || (x) == SEG_BSS)
#define SEG_LIST SEG_TEXT,SEG_DATA,SEG_BSS
#endif

typedef enum _segT
  {
    SEG_ABSOLUTE = 0,
    SEG_LIST,
    SEG_UNKNOWN,
    SEG_GOOF,			/* Only happens if AS has a logic error. */
    /* Invented so we don't crash printing */
    /* error message involving weird segment. */
    SEG_EXPR,			/* Intermediate expression values. */
    SEG_DEBUG,			/* Debug segment */
    SEG_NTV,			/* Transfert vector preload segment */
    SEG_PTV,			/* Transfert vector postload segment */
    SEG_REGISTER		/* Mythical: a register-valued expression */
  } segT;

#define SEG_MAXIMUM_ORDINAL (SEG_REGISTER)
#else
typedef asection *segT;
#define SEG_NORMAL(SEG)		((SEG) != absolute_section	\
				 && (SEG) != undefined_section	\
				 && (SEG) != reg_section	\
				 && (SEG) != expr_section)
#endif
typedef int subsegT;

/* What subseg we are accreting now? */
COMMON subsegT now_subseg;

/* Segment our instructions emit to. */
COMMON segT now_seg;

#ifdef BFD_ASSEMBLER
#define segment_name(SEG)	bfd_get_section_name (stdoutput, SEG)
#else
extern char const *const seg_name[];
#define segment_name(SEG)	seg_name[(int) (SEG)]
#endif

#ifndef BFD_ASSEMBLER
extern int section_alignment[];
#endif

#ifdef BFD_ASSEMBLER
extern segT reg_section, expr_section;
/* Shouldn't these be eliminated someday?  */
extern segT text_section, data_section, bss_section;
#define absolute_section	bfd_abs_section_ptr
#define undefined_section	bfd_und_section_ptr
#else
#define reg_section		SEG_REGISTER
#define expr_section		SEG_EXPR
#define text_section		SEG_TEXT
#define data_section		SEG_DATA
#define bss_section		SEG_BSS
#define absolute_section	SEG_ABSOLUTE
#define undefined_section	SEG_UNKNOWN
#endif

/* relax() */

enum _relax_state
  {
    /* Variable chars to be repeated fr_offset times.
       Fr_symbol unused. Used with fr_offset == 0 for a
       constant length frag. */
    rs_fill = 1,

    /* Align: Fr_offset: power of 2.  Variable chars: fill pattern. */
    rs_align,

    /* Align code: fr_offset: power of 2.  Fill pattern is machine
       specific, defaulting to all zeros.  */
    rs_align_code,

    /* Org: Fr_offset, fr_symbol: address. 1 variable char: fill
       character. */
    rs_org,

#ifndef WORKING_DOT_WORD
    /* JF: gunpoint */
    rs_broken_word,
#endif

    /* machine-specific relaxable (or similarly alterable) instruction */
    rs_machine_dependent,

    /* .space directive with expression operand that needs to be computed
       later.  Similar to rs_org, but different.
       fr_symbol: operand
       1 variable char: fill character  */
    rs_space
  };

typedef enum _relax_state relax_stateT;

/* This type is used in prototypes, so it can't be a type that will be
   widened for argument passing.  */
typedef unsigned int relax_substateT;

/* Enough bits for address, but still an integer type.
   Could be a problem, cross-assembling for 64-bit machines.  */
typedef addressT relax_addressT;


/* frags.c */

/*
 * A code fragment (frag) is some known number of chars, followed by some
 * unknown number of chars. Typically the unknown number of chars is an
 * instruction address whose size is yet unknown. We always know the greatest
 * possible size the unknown number of chars may become, and reserve that
 * much room at the end of the frag.
 * Once created, frags do not change address during assembly.
 * We chain the frags in (a) forward-linked list(s). The object-file address
 * of the 1st char of a frag is generally not known until after relax().
 * Many things at assembly time describe an address by {object-file-address
 * of a particular frag}+offset.

 BUG: it may be smarter to have a single pointer off to various different
 notes for different frag kinds. See how code pans
 */
struct frag
{
  /* Object file address. */
  addressT fr_address;
  /* Chain forward; ascending address order.  Rooted in frch_root. */
  struct frag *fr_next;

  /* (Fixed) number of chars we know we have.  May be 0. */
  offsetT fr_fix;
  /* (Variable) number of chars after above.  May be 0. */
  offsetT fr_var;
  /* For variable-length tail. */
  struct symbol *fr_symbol;
  /* For variable-length tail. */
  offsetT fr_offset;
  /* Points to opcode low addr byte, for relaxation.  */
  char *fr_opcode;

#ifndef NO_LISTING
  struct list_info_struct *line;
#endif

  /* What state is my tail in? */
  relax_stateT fr_type;
  relax_substateT fr_subtype;

  /* These are needed only on the NS32K machines.  But since we don't
     include targ-cpu.h until after this structure has been defined,
     we can't really conditionalize it.  This code should be
     rearranged a bit to make that possible.

     In the meantime, if we get stuck like this with any other target,
     create a union here.  */
  char fr_pcrel_adjust, fr_bsr;

  /* Where the frag was created, or where it became a variant frag.  */
  char *fr_file;
  unsigned int fr_line;

  /* Data begins here.  */
  char fr_literal[1];
};

#define SIZEOF_STRUCT_FRAG \
((char *)zero_address_frag.fr_literal-(char *)&zero_address_frag)
/* We want to say fr_literal[0] above. */

typedef struct frag fragS;

/* Current frag we are building.  This frag is incomplete.  It is, however,
   included in frchain_now.  The fr_fix field is bogus; instead, use:
   obstack_next_free(&frags)-frag_now->fr_literal.  */
COMMON fragS *frag_now;
extern int frag_now_fix ();

/* For foreign-segment symbol fixups. */
COMMON fragS zero_address_frag;
/* For local common (N_BSS segment) fixups. */
COMMON fragS bss_address_frag;

/* main program "as.c" (command arguments etc) */

COMMON unsigned char flag_no_comments; /* -f */
COMMON unsigned char flag_debug; /* -D */
COMMON unsigned char flag_signed_overflow_ok; /* -J */
#ifndef WORKING_DOT_WORD
COMMON unsigned char flag_warn_displacement; /* -K */
#endif

/* True if local symbols should be retained.  */
COMMON int flag_keep_locals; /* -L */

/* True if we are assembling in MRI mode.  */
COMMON int flag_mri;

/* True if we are assembling in m68k MRI mode.  */
COMMON int flag_m68k_mri;

/* Should the data section be made read-only and appended to the text
   section?  */
COMMON unsigned char flag_readonly_data_in_text; /* -R */

/* True if warnings should be inhibited.  */
COMMON int flag_no_warnings; /* -W */

/* True if we should attempt to generate output even if non-fatal errors
   are detected.  */
COMMON unsigned char flag_always_generate_output; /* -Z */

/* This is true if the assembler should output time and space usage. */

COMMON unsigned char flag_print_statistics;

/* name of emitted object file */
COMMON char *out_file_name;

/* TRUE if we need a second pass. */
COMMON int need_pass_2;

/* TRUE if we should do no relaxing, and
   leave lots of padding.  */
COMMON int linkrelax;

/* TRUE if we should produce a listing.  */
extern int listing;

/* Maximum level of macro nesting.  */
extern int max_macro_nest;

/* Obstack chunk size.  Keep large for efficient space use, make small to
   increase malloc calls for monitoring memory allocation.  */
extern int chunksize;

struct _pseudo_type
  {
    /* assembler mnemonic, lower case, no '.' */
    const char *poc_name;
    /* Do the work */
    void (*poc_handler) PARAMS ((int));
    /* Value to pass to handler */
    int poc_val;
  };

typedef struct _pseudo_type pseudo_typeS;

/* Prefer varargs for non-ANSI compiler, since some will barf if the
   ellipsis definition is used with a no-arguments declaration.  */
#if defined (HAVE_VARARGS_H) && !defined (__STDC__)
#undef HAVE_STDARG_H
#endif

#if defined (HAVE_STDARG_H)
#define USE_STDARG
#endif
#if !defined (USE_STDARG) && defined (HAVE_VARARGS_H)
#define USE_VARARGS
#endif

#ifdef USE_STDARG
#if __GNUC__ >= 2
/* for use with -Wformat */
#define PRINTF_LIKE(FCN)	void FCN (const char *format, ...) \
					__attribute__ ((format (printf, 1, 2)))
#define PRINTF_WHERE_LIKE(FCN)	void FCN (char *file, unsigned int line, \
					  const char *format, ...) \
					__attribute__ ((format (printf, 3, 4)))
#else /* ANSI C with stdarg, but not GNU C */
#define PRINTF_LIKE(FCN)	void FCN PARAMS ((const char *format, ...))
#define PRINTF_WHERE_LIKE(FCN)	void FCN PARAMS ((char *file, \
						  unsigned int line, \
					  	  const char *format, ...))
#endif
#else /* not using stdarg */
#define PRINTF_LIKE(FCN)	void FCN ()
#define PRINTF_WHERE_LIKE(FCN)	void FCN ()
#endif

PRINTF_LIKE (as_bad);
PRINTF_LIKE (as_fatal);
PRINTF_LIKE (as_tsktsk);
PRINTF_LIKE (as_warn);
PRINTF_WHERE_LIKE (as_bad_where);
PRINTF_WHERE_LIKE (as_warn_where);
void as_assert PARAMS ((const char *, int, const char *));
void as_abort PARAMS ((const char *, int, const char *));

void fprint_value PARAMS ((FILE *file, addressT value));
void sprint_value PARAMS ((char *buf, addressT value));

int had_errors PARAMS ((void));
int had_warnings PARAMS ((void));

void print_version_id PARAMS ((void));
char *app_push PARAMS ((void));
char *atof_ieee PARAMS ((char *str, int what_kind, LITTLENUM_TYPE * words));
char *input_scrub_include_file PARAMS ((char *filename, char *position));
char *input_scrub_new_file PARAMS ((char *filename));
char *input_scrub_next_buffer PARAMS ((char **bufp));
PTR xmalloc PARAMS ((unsigned long size));
PTR xrealloc PARAMS ((PTR ptr, unsigned long n));
int do_scrub_chars PARAMS ((int (*get) (char **), char *to, int tolen));
int gen_to_words PARAMS ((LITTLENUM_TYPE * words, int precision,
			  long exponent_bits));
int had_err PARAMS ((void));
int ignore_input PARAMS ((void));
int seen_at_least_1_file PARAMS ((void));
void app_pop PARAMS ((char *arg));
void as_howmuch PARAMS ((FILE * stream));
void as_perror PARAMS ((const char *gripe, const char *filename));
void as_where PARAMS ((char **namep, unsigned int *linep));
void bump_line_counters PARAMS ((void));
void do_scrub_begin PARAMS ((int));
void input_scrub_begin PARAMS ((void));
void input_scrub_close PARAMS ((void));
void input_scrub_end PARAMS ((void));
void new_logical_line PARAMS ((char *fname, int line_number));
void subsegs_begin PARAMS ((void));
void subseg_change PARAMS ((segT seg, int subseg));
segT subseg_new PARAMS ((const char *name, subsegT subseg));
segT subseg_force_new PARAMS ((const char *name, subsegT subseg));
void subseg_set PARAMS ((segT seg, subsegT subseg));
#ifdef BFD_ASSEMBLER
segT subseg_get PARAMS ((const char *, int));
#endif

struct expressionS;
struct fix;
struct symbol;
struct relax_type;

#ifdef BFD_ASSEMBLER
/* literal.c */
valueT add_to_literal_pool PARAMS ((struct symbol *, valueT, segT, int));
#endif

#include "expr.h"		/* Before targ-*.h */

/* this one starts the chain of target dependant headers */
#include "targ-env.h"

#include "struc-symbol.h"
#include "write.h"
#include "frags.h"
#include "hash.h"
#include "read.h"
#include "symbols.h"

#include "tc.h"
#include "obj.h"

#ifdef USE_EMULATIONS
#include "emul.h"
#endif
#include "listing.h"

#ifndef LOCAL_LABELS_DOLLAR
#define LOCAL_LABELS_DOLLAR 0
#endif

#ifndef LOCAL_LABELS_FB
#define LOCAL_LABELS_FB 0
#endif

#endif /* GAS */

/* end of as.h */
