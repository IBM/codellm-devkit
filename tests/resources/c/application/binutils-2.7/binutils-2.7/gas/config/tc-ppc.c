/* tc-ppc.c -- Assemble for the PowerPC or POWER (RS/6000)
   Copyright (C) 1994, 1995, 1996 Free Software Foundation, Inc.
   Written by Ian Lance Taylor, Cygnus Support.

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

#include <stdio.h>
#include <ctype.h>
#include "as.h"
#include "subsegs.h"

#include "opcode/ppc.h"

#ifdef OBJ_ELF
#include "elf/ppc.h"
#endif

#ifdef TE_PE
#include "coff/pe.h"
#endif

/* This is the assembler for the PowerPC or POWER (RS/6000) chips.  */

/* Tell the main code what the endianness is.  */
extern int target_big_endian;

/* Whether or not, we've set target_big_endian.  */
static int set_target_endian = 0;

/* Whether to use user friendly register names.  */
#ifndef TARGET_REG_NAMES_P
#ifdef TE_PE
#define TARGET_REG_NAMES_P true
#else
#define TARGET_REG_NAMES_P false
#endif
#endif

static boolean reg_names_p = TARGET_REG_NAMES_P;

static void ppc_set_cpu PARAMS ((void));
static unsigned long ppc_insert_operand
  PARAMS ((unsigned long insn, const struct powerpc_operand *operand,
	   offsetT val, char *file, unsigned int line));
static void ppc_macro PARAMS ((char *str, const struct powerpc_macro *macro));
static void ppc_byte PARAMS ((int));
static int ppc_is_toc_sym PARAMS ((symbolS *sym));
static void ppc_tc PARAMS ((int));

#ifdef OBJ_XCOFF
static void ppc_comm PARAMS ((int));
static void ppc_bb PARAMS ((int));
static void ppc_bc PARAMS ((int));
static void ppc_bf PARAMS ((int));
static void ppc_biei PARAMS ((int));
static void ppc_bs PARAMS ((int));
static void ppc_eb PARAMS ((int));
static void ppc_ec PARAMS ((int));
static void ppc_ef PARAMS ((int));
static void ppc_es PARAMS ((int));
static void ppc_csect PARAMS ((int));
static void ppc_change_csect PARAMS ((symbolS *));
static void ppc_function PARAMS ((int));
static void ppc_extern PARAMS ((int));
static void ppc_lglobl PARAMS ((int));
static void ppc_section PARAMS ((int));
static void ppc_stabx PARAMS ((int));
static void ppc_rename PARAMS ((int));
static void ppc_toc PARAMS ((int));
#endif

#ifdef OBJ_ELF
static bfd_reloc_code_real_type ppc_elf_suffix PARAMS ((char **));
static void ppc_elf_cons PARAMS ((int));
static void ppc_elf_rdata PARAMS ((int));
static void ppc_elf_lcomm PARAMS ((int));
static void ppc_elf_validate_fix PARAMS ((fixS *, segT));
#endif

#ifdef TE_PE
static void ppc_set_current_section PARAMS ((segT));
static void ppc_previous PARAMS ((int));
static void ppc_pdata PARAMS ((int));
static void ppc_ydata PARAMS ((int));
static void ppc_reldata PARAMS ((int));
static void ppc_rdata PARAMS ((int));
static void ppc_ualong PARAMS ((int));
static void ppc_znop PARAMS ((int));
static void ppc_pe_comm PARAMS ((int));
static void ppc_pe_section PARAMS ((int));
static void ppc_pe_function PARAMS ((int));
static void ppc_pe_tocd PARAMS ((int));
#endif

/* Generic assembler global variables which must be defined by all
   targets.  */

/* Characters which always start a comment.  */
#ifdef TARGET_SOLARIS_COMMENT
const char comment_chars[] = "#!";
#else
const char comment_chars[] = "#";
#endif

/* Characters which start a comment at the beginning of a line.  */
const char line_comment_chars[] = "#";

/* Characters which may be used to separate multiple commands on a
   single line.  */
const char line_separator_chars[] = ";";

/* Characters which are used to indicate an exponent in a floating
   point number.  */
const char EXP_CHARS[] = "eE";

/* Characters which mean that a number is a floating point constant,
   as in 0d1.0.  */
const char FLT_CHARS[] = "dD";

/* The target specific pseudo-ops which we support.  */

const pseudo_typeS md_pseudo_table[] =
{
  /* Pseudo-ops which must be overridden.  */
  { "byte",	ppc_byte,	0 },

#ifdef OBJ_XCOFF
  /* Pseudo-ops specific to the RS/6000 XCOFF format.  Some of these
     legitimately belong in the obj-*.c file.  However, XCOFF is based
     on COFF, and is only implemented for the RS/6000.  We just use
     obj-coff.c, and add what we need here.  */
  { "comm",	ppc_comm,	0 },
  { "lcomm",	ppc_comm,	1 },
  { "bb",	ppc_bb,		0 },
  { "bc",	ppc_bc,		0 },
  { "bf",	ppc_bf,		0 },
  { "bi",	ppc_biei,	0 },
  { "bs",	ppc_bs,		0 },
  { "csect",	ppc_csect,	0 },
  { "data",	ppc_section,	'd' },
  { "eb",	ppc_eb,		0 },
  { "ec",	ppc_ec,		0 },
  { "ef",	ppc_ef,		0 },
  { "ei",	ppc_biei,	1 },
  { "es",	ppc_es,		0 },
  { "extern",	ppc_extern,	0 },
  { "function",	ppc_function,	0 },
  { "lglobl",	ppc_lglobl,	0 },
  { "rename",	ppc_rename,	0 },
  { "stabx",	ppc_stabx,	0 },
  { "text",	ppc_section,	't' },
  { "toc",	ppc_toc,	0 },
#endif

#ifdef OBJ_ELF
  { "long",	ppc_elf_cons,	4 },
  { "word",	ppc_elf_cons,	2 },
  { "short",	ppc_elf_cons,	2 },
  { "rdata",	ppc_elf_rdata,	0 },
  { "rodata",	ppc_elf_rdata,	0 },
  { "lcomm",	ppc_elf_lcomm,	0 },
#endif

#ifdef TE_PE
  /* Pseudo-ops specific to the Windows NT PowerPC PE (coff) format */
  { "previous", ppc_previous,   0 },
  { "pdata",    ppc_pdata,      0 },
  { "ydata",    ppc_ydata,      0 },
  { "reldata",  ppc_reldata,    0 },
  { "rdata",    ppc_rdata,      0 },
  { "ualong",   ppc_ualong,     0 },
  { "znop",     ppc_znop,       0 },
  { "comm",	ppc_pe_comm,	0 },
  { "lcomm",	ppc_pe_comm,	1 },
  { "section",  ppc_pe_section, 0 },
  { "function",	ppc_pe_function,0 },
  { "tocd",     ppc_pe_tocd,    0 },
#endif

  /* This pseudo-op is used even when not generating XCOFF output.  */
  { "tc",	ppc_tc,		0 },

  { NULL,	NULL,		0 }
};


/* Predefined register names if -mregnames (or default for Windows NT).  */
/* In general, there are lots of them, in an attempt to be compatible */
/* with a number of other Windows NT assemblers.                      */

/* Structure to hold information about predefined registers.  */
struct pd_reg
  {
    char *name;
    int value;
  };

/* List of registers that are pre-defined:

   Each general register has predefined names of the form:
   1. r<reg_num> which has the value <reg_num>.
   2. r.<reg_num> which has the value <reg_num>.


   Each floating point register has predefined names of the form:
   1. f<reg_num> which has the value <reg_num>.
   2. f.<reg_num> which has the value <reg_num>.

   Each condition register has predefined names of the form:
   1. cr<reg_num> which has the value <reg_num>.
   2. cr.<reg_num> which has the value <reg_num>.

   There are individual registers as well:
   sp or r.sp     has the value 1
   rtoc or r.toc  has the value 2
   fpscr          has the value 0
   xer            has the value 1
   lr             has the value 8
   ctr            has the value 9
   pmr            has the value 0
   dar            has the value 19
   dsisr          has the value 18
   dec            has the value 22
   sdr1           has the value 25
   srr0           has the value 26
   srr1           has the value 27

   The table is sorted. Suitable for searching by a binary search. */

static const struct pd_reg pre_defined_registers[] =
{
  { "cr.0", 0 },    /* Condition Registers */
  { "cr.1", 1 },
  { "cr.2", 2 },
  { "cr.3", 3 },
  { "cr.4", 4 },
  { "cr.5", 5 },
  { "cr.6", 6 },
  { "cr.7", 7 },

  { "cr0", 0 },
  { "cr1", 1 },
  { "cr2", 2 },
  { "cr3", 3 },
  { "cr4", 4 },
  { "cr5", 5 },
  { "cr6", 6 },
  { "cr7", 7 },

  { "ctr", 9 },

  { "dar", 19 },    /* Data Access Register */
  { "dec", 22 },    /* Decrementer */
  { "dsisr", 18 },  /* Data Storage Interrupt Status Register */

  { "f.0", 0 },     /* Floating point registers */
  { "f.1", 1 }, 
  { "f.10", 10 }, 
  { "f.11", 11 }, 
  { "f.12", 12 }, 
  { "f.13", 13 }, 
  { "f.14", 14 }, 
  { "f.15", 15 }, 
  { "f.16", 16 }, 
  { "f.17", 17 }, 
  { "f.18", 18 }, 
  { "f.19", 19 }, 
  { "f.2", 2 }, 
  { "f.20", 20 }, 
  { "f.21", 21 }, 
  { "f.22", 22 }, 
  { "f.23", 23 }, 
  { "f.24", 24 }, 
  { "f.25", 25 }, 
  { "f.26", 26 }, 
  { "f.27", 27 }, 
  { "f.28", 28 }, 
  { "f.29", 29 }, 
  { "f.3", 3 }, 
  { "f.30", 30 },
  { "f.31", 31 },
  { "f.4", 4 }, 
  { "f.5", 5 }, 
  { "f.6", 6 }, 
  { "f.7", 7 }, 
  { "f.8", 8 }, 
  { "f.9", 9 }, 

  { "f0", 0 }, 
  { "f1", 1 }, 
  { "f10", 10 }, 
  { "f11", 11 }, 
  { "f12", 12 }, 
  { "f13", 13 }, 
  { "f14", 14 }, 
  { "f15", 15 }, 
  { "f16", 16 }, 
  { "f17", 17 }, 
  { "f18", 18 }, 
  { "f19", 19 }, 
  { "f2", 2 }, 
  { "f20", 20 }, 
  { "f21", 21 }, 
  { "f22", 22 }, 
  { "f23", 23 }, 
  { "f24", 24 }, 
  { "f25", 25 }, 
  { "f26", 26 }, 
  { "f27", 27 }, 
  { "f28", 28 }, 
  { "f29", 29 }, 
  { "f3", 3 }, 
  { "f30", 30 },
  { "f31", 31 },
  { "f4", 4 }, 
  { "f5", 5 }, 
  { "f6", 6 }, 
  { "f7", 7 }, 
  { "f8", 8 }, 
  { "f9", 9 }, 

  { "fpscr", 0 },

  { "lr", 8 },     /* Link Register */

  { "pmr", 0 },

  { "r.0", 0 },    /* General Purpose Registers */
  { "r.1", 1 },
  { "r.10", 10 },
  { "r.11", 11 },
  { "r.12", 12 },
  { "r.13", 13 },
  { "r.14", 14 },
  { "r.15", 15 },
  { "r.16", 16 },
  { "r.17", 17 },
  { "r.18", 18 },
  { "r.19", 19 },
  { "r.2", 2 },
  { "r.20", 20 },
  { "r.21", 21 },
  { "r.22", 22 },
  { "r.23", 23 },
  { "r.24", 24 },
  { "r.25", 25 },
  { "r.26", 26 },
  { "r.27", 27 },
  { "r.28", 28 },
  { "r.29", 29 },
  { "r.3", 3 },
  { "r.30", 30 },
  { "r.31", 31 },
  { "r.4", 4 },
  { "r.5", 5 },
  { "r.6", 6 },
  { "r.7", 7 },
  { "r.8", 8 },
  { "r.9", 9 },

  { "r.sp", 1 },   /* Stack Pointer */

  { "r.toc", 2 },  /* Pointer to the table of contents */

  { "r0", 0 },     /* More general purpose registers */
  { "r1", 1 },
  { "r10", 10 },
  { "r11", 11 },
  { "r12", 12 },
  { "r13", 13 },
  { "r14", 14 },
  { "r15", 15 },
  { "r16", 16 },
  { "r17", 17 },
  { "r18", 18 },
  { "r19", 19 },
  { "r2", 2 },
  { "r20", 20 },
  { "r21", 21 },
  { "r22", 22 },
  { "r23", 23 },
  { "r24", 24 },
  { "r25", 25 },
  { "r26", 26 },
  { "r27", 27 },
  { "r28", 28 },
  { "r29", 29 },
  { "r3", 3 },
  { "r30", 30 },
  { "r31", 31 },
  { "r4", 4 },
  { "r5", 5 },
  { "r6", 6 },
  { "r7", 7 },
  { "r8", 8 },
  { "r9", 9 },

  { "rtoc", 2 },  /* Table of contents */

  { "sdr1", 25 }, /* Storage Description Register 1 */

  { "sp", 1 },

  { "srr0", 26 }, /* Machine Status Save/Restore Register 0 */
  { "srr1", 27 }, /* Machine Status Save/Restore Register 1 */

  { "xer", 1 },

};

#define REG_NAME_CNT	(sizeof(pre_defined_registers) / sizeof(struct pd_reg))

/* Given NAME, find the register number associated with that name, return
   the integer value associated with the given name or -1 on failure.  */

static int reg_name_search PARAMS ( (char * name) );

static int
reg_name_search (name)
     char *name;
{
  int middle, low, high;
  int cmp;

  low = 0;
  high = REG_NAME_CNT - 1;

  do
    {
      middle = (low + high) / 2;
      cmp = strcasecmp (name, pre_defined_registers[middle].name);
      if (cmp < 0)
	high = middle - 1;
      else if (cmp > 0)
	low = middle + 1;
      else
	return pre_defined_registers[middle].value;
    }
  while (low <= high);

  return -1;
}

/*
 * Summary of register_name().
 *
 * in:	Input_line_pointer points to 1st char of operand.
 *
 * out:	A expressionS.
 *      The operand may have been a register: in this case, X_op == O_register,
 *      X_add_number is set to the register number, and truth is returned.
 *	Input_line_pointer->(next non-blank) char after operand, or is in its
 *      original state.
 */

static boolean
register_name (expressionP)
     expressionS *expressionP;
{
  int reg_number;
  char *name;
  char *start;
  char c;

  /* Find the spelling of the operand */
  start = name = input_line_pointer;
  if (name[0] == '%' && isalpha (name[1]))
    name = ++input_line_pointer;

  else if (!reg_names_p || !isalpha (name[0]))
    return false;

  c = get_symbol_end ();
  reg_number = reg_name_search (name);

  /* look to see if it's in the register table */
  if (reg_number >= 0) 
    {
      expressionP->X_op = O_register;
      expressionP->X_add_number = reg_number;
      
      /* make the rest nice */
      expressionP->X_add_symbol = NULL;
      expressionP->X_op_symbol = NULL;
      *input_line_pointer = c;   /* put back the delimiting char */
      return true;
    }
  else
    {
      /* reset the line as if we had not done anything */
      *input_line_pointer = c;   /* put back the delimiting char */
      input_line_pointer = start; /* reset input_line pointer */
      return false;
    }
}


/* Local variables.  */

/* The type of processor we are assembling for.  This is one or more
   of the PPC_OPCODE flags defined in opcode/ppc.h.  */
static int ppc_cpu = 0;

/* The size of the processor we are assembling for.  This is either
   PPC_OPCODE_32 or PPC_OPCODE_64.  */
static int ppc_size = PPC_OPCODE_32;

/* Opcode hash table.  */
static struct hash_control *ppc_hash;

/* Macro hash table.  */
static struct hash_control *ppc_macro_hash;

#ifdef OBJ_ELF
/* Whether to warn about non PC relative relocations that aren't
   in the .got2 section. */
static boolean mrelocatable = false;

/* Flags to set in the elf header */
static flagword ppc_flags = 0;
#endif

#ifdef OBJ_XCOFF

/* The RS/6000 assembler uses the .csect pseudo-op to generate code
   using a bunch of different sections.  These assembler sections,
   however, are all encompassed within the .text or .data sections of
   the final output file.  We handle this by using different
   subsegments within these main segments.  */

/* Next subsegment to allocate within the .text segment.  */
static subsegT ppc_text_subsegment = 2;

/* Linked list of csects in the text section.  */
static symbolS *ppc_text_csects;

/* Next subsegment to allocate within the .data segment.  */
static subsegT ppc_data_subsegment = 2;

/* Linked list of csects in the data section.  */
static symbolS *ppc_data_csects;

/* The current csect.  */
static symbolS *ppc_current_csect;

/* The RS/6000 assembler uses a TOC which holds addresses of functions
   and variables.  Symbols are put in the TOC with the .tc pseudo-op.
   A special relocation is used when accessing TOC entries.  We handle
   the TOC as a subsegment within the .data segment.  We set it up if
   we see a .toc pseudo-op, and save the csect symbol here.  */
static symbolS *ppc_toc_csect;

/* The first frag in the TOC subsegment.  */
static fragS *ppc_toc_frag;

/* The first frag in the first subsegment after the TOC in the .data
   segment.  NULL if there are no subsegments after the TOC.  */
static fragS *ppc_after_toc_frag;

/* The current static block.  */
static symbolS *ppc_current_block;

/* The COFF debugging section; set by md_begin.  This is not the
   .debug section, but is instead the secret BFD section which will
   cause BFD to set the section number of a symbol to N_DEBUG.  */
static asection *ppc_coff_debug_section;

#endif /* OBJ_XCOFF */

#ifdef TE_PE

/* Various sections that we need for PE coff support.  */
static segT ydata_section;
static segT pdata_section;
static segT reldata_section;
static segT rdata_section;
static segT tocdata_section;

/* The current section and the previous section. See ppc_previous. */
static segT ppc_previous_section;
static segT ppc_current_section;

#endif /* TE_PE */

#ifdef OBJ_ELF
symbolS *GOT_symbol;		/* Pre-defined "_GLOBAL_OFFSET_TABLE" */
#endif /* OBJ_ELF */

#ifndef WORKING_DOT_WORD
const int md_short_jump_size = 4;
const int md_long_jump_size = 4;
#endif

#ifdef OBJ_ELF
CONST char *md_shortopts = "b:l:usm:K:VQ:";
#else
CONST char *md_shortopts = "um:";
#endif
struct option md_longopts[] = {
  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof(md_longopts);

int
md_parse_option (c, arg)
     int c;
     char *arg;
{
  switch (c)
    {
    case 'u':
      /* -u means that any undefined symbols should be treated as
	 external, which is the default for gas anyhow.  */
      break;

#ifdef OBJ_ELF
    case 'l':
      /* Solaris as takes -le (presumably for little endian).  For completeness
         sake, recognize -be also.  */
      if (strcmp (arg, "e") == 0)
	{
	  target_big_endian = 0;
	  set_target_endian = 1;
	}
      else
	return 0;

      break;

    case 'b':
      if (strcmp (arg, "e") == 0)
	{
	  target_big_endian = 1;
	  set_target_endian = 1;
	}
      else
	return 0;

      break;

    case 'K':
      /* Recognize -K PIC */
      if (strcmp (arg, "PIC") == 0)
	{
	  mrelocatable = true;
	  ppc_flags |= EF_PPC_RELOCATABLE_LIB;
	}
      else
	return 0;

      break;
#endif

    case 'm':
      /* -mpwrx and -mpwr2 mean to assemble for the IBM POWER/2
         (RIOS2).  */
      if (strcmp (arg, "pwrx") == 0 || strcmp (arg, "pwr2") == 0)
	ppc_cpu = PPC_OPCODE_POWER | PPC_OPCODE_POWER2;
      /* -mpwr means to assemble for the IBM POWER (RIOS1).  */
      else if (strcmp (arg, "pwr") == 0)
	ppc_cpu = PPC_OPCODE_POWER;
      /* -m601 means to assemble for the Motorola PowerPC 601, which includes
         instructions that are holdovers from the Power. */
      else if (strcmp (arg, "601") == 0)
	ppc_cpu = PPC_OPCODE_PPC | PPC_OPCODE_601;
      /* -mppc, -mppc32, -m603, and -m604 mean to assemble for the
         Motorola PowerPC 603/604.  */
      else if (strcmp (arg, "ppc") == 0
	       || strcmp (arg, "ppc32") == 0
	       || strcmp (arg, "403") == 0
	       || strcmp (arg, "603") == 0
	       || strcmp (arg, "604") == 0)
	ppc_cpu = PPC_OPCODE_PPC;
      /* -mppc64 and -m620 mean to assemble for the 64-bit PowerPC
         620.  */
      else if (strcmp (arg, "ppc64") == 0 || strcmp (arg, "620") == 0)
	{
	  ppc_cpu = PPC_OPCODE_PPC;
	  ppc_size = PPC_OPCODE_64;
	}
      /* -mcom means assemble for the common intersection between Power
	 and PowerPC.  At present, we just allow the union, rather
	 than the intersection.  */
      else if (strcmp (arg, "com") == 0)
	ppc_cpu = PPC_OPCODE_COMMON;
      /* -many means to assemble for any architecture (PWR/PWRX/PPC).  */
      else if (strcmp (arg, "any") == 0)
	ppc_cpu = PPC_OPCODE_ANY;

      else if (strcmp (arg, "regnames") == 0)
	reg_names_p = true;

      else if (strcmp (arg, "no-regnames") == 0)
	reg_names_p = false;

#ifdef OBJ_ELF
      /* -mrelocatable/-mrelocatable-lib -- warn about initializations that require relocation */
      else if (strcmp (arg, "relocatable") == 0)
	{
	  mrelocatable = true;
	  ppc_flags |= EF_PPC_RELOCATABLE;
	}

      else if (strcmp (arg, "relocatable-lib") == 0)
	{
	  mrelocatable = true;
	  ppc_flags |= EF_PPC_RELOCATABLE_LIB;
	}

      /* -memb, set embedded bit */
      else if (strcmp (arg, "emb") == 0)
	ppc_flags |= EF_PPC_EMB;

      /* -mlittle/-mbig set the endianess */
      else if (strcmp (arg, "little") == 0 || strcmp (arg, "little-endian") == 0)
	{
	  target_big_endian = 0;
	  set_target_endian = 1;
	}

      else if (strcmp (arg, "big") == 0 || strcmp (arg, "big-endian") == 0)
	{
	  target_big_endian = 1;
	  set_target_endian = 1;
	}
#endif
      else
	{
	  as_bad ("invalid switch -m%s", arg);
	  return 0;
	}
      break;

#ifdef OBJ_ELF
      /* -V: SVR4 argument to print version ID.  */
    case 'V':
      print_version_id ();
      break;

      /* -Qy, -Qn: SVR4 arguments controlling whether a .comment section
	 should be emitted or not.  FIXME: Not implemented.  */
    case 'Q':
      break;

      /* Solaris takes -s to specify that .stabs go in a .stabs section,
	 rather than .stabs.excl, which is ignored by the linker.
	 FIXME: Not implemented.  */
    case 's':
      if (arg)
	return 0;

      break;
#endif

    default:
      return 0;
    }

  return 1;
}

void
md_show_usage (stream)
     FILE *stream;
{
  fprintf(stream, "\
PowerPC options:\n\
-u			ignored\n\
-mpwrx, -mpwr2		generate code for IBM POWER/2 (RIOS2)\n\
-mpwr			generate code for IBM POWER (RIOS1)\n\
-m601			generate code for Motorola PowerPC 601\n\
-mppc, -mppc32, -m403, -m603, -m604\n\
			generate code for Motorola PowerPC 603/604\n\
-mppc64, -m620		generate code for Motorola PowerPC 620\n\
-mcom			generate code Power/PowerPC common instructions\n\
-many			generate code for any architecture (PWR/PWRX/PPC)\n\
-mregnames		Allow symbolic names for registers\n\
-mno-regnames		Do not allow symbolic names for registers\n");
#ifdef OBJ_ELF
  fprintf(stream, "\
-mrelocatable		support for GCC's -mrelocatble option\n\
-mrelocatable-lib	support for GCC's -mrelocatble-lib option\n\
-memb			set PPC_EMB bit in ELF flags\n\
-mlittle, -mlittle-endian\n\
			generate code for a little endian machine\n\
-mbig, -mbig-endian	generate code for a big endian machine\n\
-V			print assembler version number\n\
-Qy, -Qn		ignored\n");
#endif
}

/* Set ppc_cpu if it is not already set.  */

static void
ppc_set_cpu ()
{
  const char *default_os  = TARGET_OS;
  const char *default_cpu = TARGET_CPU;

  if (ppc_cpu == 0)
    {
      if (strncmp (default_os, "aix", 3) == 0
	  && default_os[3] >= '4' && default_os[3] <= '9')
	ppc_cpu = PPC_OPCODE_COMMON;
      else if (strncmp (default_os, "aix3", 4) == 0)
	ppc_cpu = PPC_OPCODE_POWER;
      else if (strcmp (default_cpu, "rs6000") == 0)
	ppc_cpu = PPC_OPCODE_POWER;
      else if (strcmp (default_cpu, "powerpc") == 0
	       || strcmp (default_cpu, "powerpcle") == 0)
	ppc_cpu = PPC_OPCODE_PPC;
      else
	as_fatal ("Unknown default cpu = %s, os = %s", default_cpu, default_os);
    }
}

/* Figure out the BFD architecture to use.  */

enum bfd_architecture
ppc_arch ()
{
  const char *default_cpu = TARGET_CPU;
  ppc_set_cpu ();

  if ((ppc_cpu & PPC_OPCODE_PPC) != 0)
    return bfd_arch_powerpc;
  else if ((ppc_cpu & PPC_OPCODE_POWER) != 0)
    return bfd_arch_rs6000;
  else if ((ppc_cpu & (PPC_OPCODE_COMMON | PPC_OPCODE_ANY)) != 0)
    {
      if (strcmp (default_cpu, "rs6000") == 0)
	return bfd_arch_rs6000;
      else if (strcmp (default_cpu, "powerpc") == 0
	       || strcmp (default_cpu, "powerpcle") == 0)
	return bfd_arch_powerpc;
    }

  as_fatal ("Neither Power nor PowerPC opcodes were selected.");
  return bfd_arch_unknown;
}

/* This function is called when the assembler starts up.  It is called
   after the options have been parsed and the output file has been
   opened.  */

void
md_begin ()
{
  register const struct powerpc_opcode *op;
  const struct powerpc_opcode *op_end;
  const struct powerpc_macro *macro;
  const struct powerpc_macro *macro_end;
  boolean dup_insn = false;

  ppc_set_cpu ();

#ifdef OBJ_ELF
  /* Set the ELF flags if desired. */
  if (ppc_flags)
    bfd_set_private_flags (stdoutput, ppc_flags);
#endif

  /* Insert the opcodes into a hash table.  */
  ppc_hash = hash_new ();

  op_end = powerpc_opcodes + powerpc_num_opcodes;
  for (op = powerpc_opcodes; op < op_end; op++)
    {
      know ((op->opcode & op->mask) == op->opcode);

      if ((op->flags & ppc_cpu) != 0
	  && ((op->flags & (PPC_OPCODE_32 | PPC_OPCODE_64)) == 0
	      || (op->flags & (PPC_OPCODE_32 | PPC_OPCODE_64)) == ppc_size))
	{
	  const char *retval;

	  retval = hash_insert (ppc_hash, op->name, (PTR) op);
	  if (retval != (const char *) NULL)
	    {
	      /* Ignore Power duplicates for -m601 */
	      if ((ppc_cpu & PPC_OPCODE_601) != 0
		  && (op->flags & PPC_OPCODE_POWER) != 0)
		continue;

	      as_bad ("Internal assembler error for instruction %s", op->name);
	      dup_insn = true;
	    }
	}
    }

  /* Insert the macros into a hash table.  */
  ppc_macro_hash = hash_new ();

  macro_end = powerpc_macros + powerpc_num_macros;
  for (macro = powerpc_macros; macro < macro_end; macro++)
    {
      if ((macro->flags & ppc_cpu) != 0)
	{
	  const char *retval;

	  retval = hash_insert (ppc_macro_hash, macro->name, (PTR) macro);
	  if (retval != (const char *) NULL)
	    {
	      as_bad ("Internal assembler error for macro %s", macro->name);
	      dup_insn = true;
	    }
	}
    }

  if (dup_insn)
    abort ();

  /* Tell the main code what the endianness is if it is not overidden by the user.  */
  if (!set_target_endian)
    {
      set_target_endian = 1;
      target_big_endian = PPC_BIG_ENDIAN;
    }

#ifdef OBJ_XCOFF
  ppc_coff_debug_section = coff_section_from_bfd_index (stdoutput, N_DEBUG);

  /* Create dummy symbols to serve as initial csects.  This forces the
     text csects to precede the data csects.  These symbols will not
     be output.  */
  ppc_text_csects = symbol_make ("dummy\001");
  ppc_text_csects->sy_tc.within = ppc_text_csects;
  ppc_data_csects = symbol_make ("dummy\001");
  ppc_data_csects->sy_tc.within = ppc_data_csects;
#endif

#ifdef TE_PE

  ppc_current_section = text_section;
  ppc_previous_section = 0;  

#endif
}

/* Insert an operand value into an instruction.  */

static unsigned long
ppc_insert_operand (insn, operand, val, file, line)
     unsigned long insn;
     const struct powerpc_operand *operand;
     offsetT val;
     char *file;
     unsigned int line;
{
  if (operand->bits != 32)
    {
      long min, max;
      offsetT test;

      if ((operand->flags & PPC_OPERAND_SIGNED) != 0)
	{
	  if ((operand->flags & PPC_OPERAND_SIGNOPT) != 0
	      && ppc_size == PPC_OPCODE_32)
	    max = (1 << operand->bits) - 1;
	  else
	    max = (1 << (operand->bits - 1)) - 1;
	  min = - (1 << (operand->bits - 1));
	}
      else
	{
	  max = (1 << operand->bits) - 1;
	  min = 0;
	}

      if ((operand->flags & PPC_OPERAND_NEGATIVE) != 0)
	test = - val;
      else
	test = val;

      if (test < (offsetT) min || test > (offsetT) max)
	{
	  const char *err =
	    "operand out of range (%s not between %ld and %ld)";
	  char buf[100];

	  sprint_value (buf, test);
	  if (file == (char *) NULL)
	    as_warn (err, buf, min, max);
	  else
	    as_warn_where (file, line, err, buf, min, max);
	}
    }

  if (operand->insert)
    {
      const char *errmsg;

      errmsg = NULL;
      insn = (*operand->insert) (insn, (long) val, &errmsg);
      if (errmsg != (const char *) NULL)
	as_warn (errmsg);
    }
  else
    insn |= (((long) val & ((1 << operand->bits) - 1))
	     << operand->shift);

  return insn;
}


#ifdef OBJ_ELF
/* Parse @got, etc. and return the desired relocation.  */
static bfd_reloc_code_real_type
ppc_elf_suffix (str_p)
     char **str_p;
{
  struct map_bfd {
    char *string;
    int length;
    bfd_reloc_code_real_type reloc;
  };

  char ident[20];
  char *str = *str_p;
  char *str2;
  int ch;
  int len;
  struct map_bfd *ptr;

#define MAP(str,reloc) { str, sizeof(str)-1, reloc }

  static struct map_bfd mapping[] = {
    MAP ("l",		BFD_RELOC_LO16),
    MAP ("h",		BFD_RELOC_HI16),
    MAP ("ha",		BFD_RELOC_HI16_S),
    MAP ("brtaken",	BFD_RELOC_PPC_B16_BRTAKEN),
    MAP ("brntaken",	BFD_RELOC_PPC_B16_BRNTAKEN),
    MAP ("got",		BFD_RELOC_16_GOTOFF),
    MAP ("got@l",	BFD_RELOC_LO16_GOTOFF),
    MAP ("got@h",	BFD_RELOC_HI16_GOTOFF),
    MAP ("got@ha",	BFD_RELOC_HI16_S_GOTOFF),
    MAP ("fixup",	BFD_RELOC_CTOR),		/* warnings with -mrelocatable */
    MAP ("pltrel24",	BFD_RELOC_24_PLT_PCREL),
    MAP ("copy",	BFD_RELOC_PPC_COPY),
    MAP ("globdat",	BFD_RELOC_PPC_GLOB_DAT),
    MAP ("local24pc",	BFD_RELOC_PPC_LOCAL24PC),
    MAP ("plt",		BFD_RELOC_32_PLTOFF),
    MAP ("pltrel",	BFD_RELOC_32_PLT_PCREL),
    MAP ("plt@l",	BFD_RELOC_LO16_PLTOFF),
    MAP ("plt@h",	BFD_RELOC_HI16_PLTOFF),
    MAP ("plt@ha",	BFD_RELOC_HI16_S_PLTOFF),
    MAP ("sdarel",	BFD_RELOC_GPREL16),
    MAP ("sectoff",	BFD_RELOC_32_BASEREL),
    MAP ("sectoff@l",	BFD_RELOC_LO16_BASEREL),
    MAP ("sectoff@h",	BFD_RELOC_HI16_BASEREL),
    MAP ("sectoff@ha",	BFD_RELOC_HI16_S_BASEREL),
    MAP ("naddr",	BFD_RELOC_PPC_EMB_NADDR32),
    MAP ("naddr16",	BFD_RELOC_PPC_EMB_NADDR16),
    MAP ("naddr@l",	BFD_RELOC_PPC_EMB_NADDR16_LO),
    MAP ("naddr@h",	BFD_RELOC_PPC_EMB_NADDR16_HI),
    MAP ("naddr@ha",	BFD_RELOC_PPC_EMB_NADDR16_HA),
    MAP ("sdai16",	BFD_RELOC_PPC_EMB_SDAI16),
    MAP ("sda2rel",	BFD_RELOC_PPC_EMB_SDA2REL),
    MAP ("sda2i16",	BFD_RELOC_PPC_EMB_SDA2I16),
    MAP ("sda21",	BFD_RELOC_PPC_EMB_SDA21),
    MAP ("mrkref",	BFD_RELOC_PPC_EMB_MRKREF),
    MAP ("relsect",	BFD_RELOC_PPC_EMB_RELSEC16),
    MAP ("relsect@l",	BFD_RELOC_PPC_EMB_RELST_LO),
    MAP ("relsect@h",	BFD_RELOC_PPC_EMB_RELST_HI),
    MAP ("relsect@ha",	BFD_RELOC_PPC_EMB_RELST_HA),
    MAP ("bitfld",	BFD_RELOC_PPC_EMB_BIT_FLD),
    MAP ("relsda",	BFD_RELOC_PPC_EMB_RELSDA),
    MAP ("xgot",	BFD_RELOC_PPC_TOC16),

    { (char *)0,	0,	BFD_RELOC_UNUSED }
  };

  if (*str++ != '@')
    return BFD_RELOC_UNUSED;

  for (ch = *str, str2 = ident;
       (str2 < ident + sizeof (ident) - 1
	&& (isalnum (ch) || ch == '@'));
       ch = *++str)
    {
      *str2++ = (islower (ch)) ? ch : tolower (ch);
    }

  *str2 = '\0';
  len = str2 - ident;

  ch = ident[0];
  for (ptr = &mapping[0]; ptr->length > 0; ptr++)
    if (ch == ptr->string[0] && len == ptr->length && memcmp (ident, ptr->string, ptr->length) == 0)
      {
	*str_p = str;
	return ptr->reloc;
      }

  return BFD_RELOC_UNUSED;
}

/* Like normal .long/.short/.word, except support @got, etc. */
/* clobbers input_line_pointer, checks */
/* end-of-line. */
static void
ppc_elf_cons (nbytes)
     register int nbytes;	/* 1=.byte, 2=.word, 4=.long */
{
  expressionS exp;
  bfd_reloc_code_real_type reloc;

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

  do
    {
      expression (&exp);
      if (exp.X_op == O_symbol
	  && *input_line_pointer == '@'
	  && (reloc = ppc_elf_suffix (&input_line_pointer)) != BFD_RELOC_UNUSED)
	{
	  reloc_howto_type *reloc_howto = bfd_reloc_type_lookup (stdoutput, reloc);
	  int size = bfd_get_reloc_size (reloc_howto);

	  if (size > nbytes)
	    as_bad ("%s relocations do not fit in %d bytes\n", reloc_howto->name, nbytes);

	  else
	    {
	      register char *p = frag_more ((int) nbytes);
	      int offset = nbytes - size;

	      fix_new_exp (frag_now, p - frag_now->fr_literal + offset, size, &exp, 0, reloc);
	    }
	}
      else
	emit_expr (&exp, (unsigned int) nbytes);
    }
  while (*input_line_pointer++ == ',');

  input_line_pointer--;		/* Put terminator back into stream. */
  demand_empty_rest_of_line ();
}

/* Solaris pseduo op to change to the .rodata section.  */
static void
ppc_elf_rdata (xxx)
     int xxx;
{
  char *save_line = input_line_pointer;
  static char section[] = ".rodata\n";

  /* Just pretend this is .section .rodata */
  input_line_pointer = section;
  obj_elf_section (xxx);

  input_line_pointer = save_line;
}

/* Pseudo op to make file scope bss items */
static void
ppc_elf_lcomm(xxx)
     int xxx;
{
  register char *name;
  register char c;
  register char *p;
  offsetT size;
  register symbolS *symbolP;
  offsetT align;
  segT old_sec;
  int old_subsec;
  char *pfrag;
  int align2;

  name = input_line_pointer;
  c = get_symbol_end ();

  /* just after name is now '\0' */
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad ("Expected comma after symbol-name: rest of line ignored.");
      ignore_rest_of_line ();
      return;
    }

  input_line_pointer++;		/* skip ',' */
  if ((size = get_absolute_expression ()) < 0)
    {
      as_warn (".COMMon length (%ld.) <0! Ignored.", (long) size);
      ignore_rest_of_line ();
      return;
    }

  /* The third argument to .lcomm is the alignment.  */
  if (*input_line_pointer != ',')
    align = 3;
  else
    {
      ++input_line_pointer;
      align = get_absolute_expression ();
      if (align <= 0)
	{
	  as_warn ("ignoring bad alignment");
	  align = 3;
	}
    }

  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;

  if (S_IS_DEFINED (symbolP))
    {
      as_bad ("Ignoring attempt to re-define symbol `%s'.",
	      S_GET_NAME (symbolP));
      ignore_rest_of_line ();
      return;
    }

  if (S_GET_VALUE (symbolP) && S_GET_VALUE (symbolP) != (valueT) size)
    {
      as_bad ("Length of .lcomm \"%s\" is already %ld. Not changed to %ld.",
	      S_GET_NAME (symbolP),
	      (long) S_GET_VALUE (symbolP),
	      (long) size);

      ignore_rest_of_line ();
      return;
    }

  /* allocate_bss: */
  old_sec = now_seg;
  old_subsec = now_subseg;
  if (align)
    {
      /* convert to a power of 2 alignment */
      for (align2 = 0; (align & 1) == 0; align >>= 1, ++align2);
      if (align != 1)
	{
	  as_bad ("Common alignment not a power of 2");
	  ignore_rest_of_line ();
	  return;
	}
    }
  else
    align2 = 0;

  record_alignment (bss_section, align2);
  subseg_set (bss_section, 0);
  if (align2)
    frag_align (align2, 0);
  if (S_GET_SEGMENT (symbolP) == bss_section)
    symbolP->sy_frag->fr_symbol = 0;
  symbolP->sy_frag = frag_now;
  pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP, size,
		    (char *) 0);
  *pfrag = 0;
  S_SET_SIZE (symbolP, size);
  S_SET_SEGMENT (symbolP, bss_section);
  S_CLEAR_EXTERNAL (symbolP);
  subseg_set (old_sec, old_subsec);
  demand_empty_rest_of_line ();
}

/* Validate any relocations emitted for -mrelocatable, possibly adding
   fixups for word relocations in writable segments, so we can adjust
   them at runtime.  */
static void
ppc_elf_validate_fix (fixp, seg)
     fixS *fixp;
     segT seg;
{
  if (mrelocatable
      && !fixp->fx_done
      && !fixp->fx_pcrel
      && fixp->fx_r_type <= BFD_RELOC_UNUSED
      && fixp->fx_r_type != BFD_RELOC_16_GOTOFF
      && fixp->fx_r_type != BFD_RELOC_HI16_GOTOFF
      && fixp->fx_r_type != BFD_RELOC_LO16_GOTOFF
      && fixp->fx_r_type != BFD_RELOC_HI16_S_GOTOFF
      && fixp->fx_r_type != BFD_RELOC_32_BASEREL
      && fixp->fx_r_type != BFD_RELOC_LO16_BASEREL
      && fixp->fx_r_type != BFD_RELOC_HI16_BASEREL
      && fixp->fx_r_type != BFD_RELOC_HI16_S_BASEREL
      && strcmp (segment_name (seg), ".got2") != 0
      && strcmp (segment_name (seg), ".dtors") != 0
      && strcmp (segment_name (seg), ".ctors") != 0
      && strcmp (segment_name (seg), ".fixup") != 0
      && strcmp (segment_name (seg), ".stab") != 0
      && strcmp (segment_name (seg), ".gcc_except_table") != 0
      && strcmp (segment_name (seg), ".ex_shared") != 0)
    {
      if ((seg->flags & (SEC_READONLY | SEC_CODE)) != 0
	  || fixp->fx_r_type != BFD_RELOC_CTOR)
	{
	  as_warn_where (fixp->fx_file, fixp->fx_line,
			 "Relocation cannot be done when using -mrelocatable");
	}
    }
}
#endif /* OBJ_ELF */

#ifdef TE_PE

/*
 * Summary of parse_toc_entry().
 *
 * in:	Input_line_pointer points to the '[' in one of:
 *
 *        [toc] [tocv] [toc32] [toc64]
 *
 *      Anything else is an error of one kind or another.
 *
 * out:	
 *   return value: success or failure
 *   toc_kind:     kind of toc reference
 *   input_line_pointer:
 *     success: first char after the ']'
 *     failure: unchanged
 *
 * settings:
 *
 *     [toc]   - rv == success, toc_kind = default_toc
 *     [tocv]  - rv == success, toc_kind = data_in_toc
 *     [toc32] - rv == success, toc_kind = must_be_32
 *     [toc64] - rv == success, toc_kind = must_be_64
 *
 */

enum toc_size_qualifier 
{ 
  default_toc, /* The toc cell constructed should be the system default size */
  data_in_toc, /* This is a direct reference to a toc cell                   */
  must_be_32,  /* The toc cell constructed must be 32 bits wide              */
  must_be_64   /* The toc cell constructed must be 64 bits wide              */
};

static int
parse_toc_entry(toc_kind)
     enum toc_size_qualifier *toc_kind;
{
  char *start;
  char *toc_spec;
  char c;
  enum toc_size_qualifier t;

  /* save the input_line_pointer */
  start = input_line_pointer;

  /* skip over the '[' , and whitespace */
  ++input_line_pointer;
  SKIP_WHITESPACE ();
  
  /* find the spelling of the operand */
  toc_spec = input_line_pointer;
  c = get_symbol_end ();

  if (strcmp(toc_spec, "toc") == 0) 
    {
      t = default_toc;
    }
  else if (strcmp(toc_spec, "tocv") == 0) 
    {
      t = data_in_toc;
    }
  else if (strcmp(toc_spec, "toc32") == 0) 
    {
      t = must_be_32;
    }
  else if (strcmp(toc_spec, "toc64") == 0) 
    {
      t = must_be_64;
    }
  else
    {
      as_bad ("syntax error: invalid toc specifier `%s'", toc_spec);
      *input_line_pointer = c;   /* put back the delimiting char */
      input_line_pointer = start; /* reset input_line pointer */
      return 0;
    }

  /* now find the ']' */
  *input_line_pointer = c;   /* put back the delimiting char */

  SKIP_WHITESPACE ();	     /* leading whitespace could be there. */
  c = *input_line_pointer++; /* input_line_pointer->past char in c. */

  if (c != ']')
    {
      as_bad ("syntax error: expected `]', found  `%c'", c);
      input_line_pointer = start; /* reset input_line pointer */
      return 0;
    }

  *toc_kind = t;             /* set return value */
  return 1;
}
#endif


/* We need to keep a list of fixups.  We can't simply generate them as
   we go, because that would require us to first create the frag, and
   that would screw up references to ``.''.  */

struct ppc_fixup
{
  expressionS exp;
  int opindex;
  bfd_reloc_code_real_type reloc;
};

#define MAX_INSN_FIXUPS (5)

/* This routine is called for each instruction to be assembled.  */

void
md_assemble (str)
     char *str;
{
  char *s;
  const struct powerpc_opcode *opcode;
  unsigned long insn;
  const unsigned char *opindex_ptr;
  int skip_optional;
  int need_paren;
  int next_opindex;
  struct ppc_fixup fixups[MAX_INSN_FIXUPS];
  int fc;
  char *f;
  int i;
#ifdef OBJ_ELF
  bfd_reloc_code_real_type reloc;
#endif

  /* Get the opcode.  */
  for (s = str; *s != '\0' && ! isspace (*s); s++)
    ;
  if (*s != '\0')
    *s++ = '\0';

  /* Look up the opcode in the hash table.  */
  opcode = (const struct powerpc_opcode *) hash_find (ppc_hash, str);
  if (opcode == (const struct powerpc_opcode *) NULL)
    {
      const struct powerpc_macro *macro;

      macro = (const struct powerpc_macro *) hash_find (ppc_macro_hash, str);
      if (macro == (const struct powerpc_macro *) NULL)
	as_bad ("Unrecognized opcode: `%s'", str);
      else
	ppc_macro (s, macro);

      return;
    }

  insn = opcode->opcode;

  str = s;
  while (isspace (*str))
    ++str;

  /* PowerPC operands are just expressions.  The only real issue is
     that a few operand types are optional.  All cases which might use
     an optional operand separate the operands only with commas (in
     some cases parentheses are used, as in ``lwz 1,0(1)'' but such
     cases never have optional operands).  There is never more than
     one optional operand for an instruction.  So, before we start
     seriously parsing the operands, we check to see if we have an
     optional operand, and, if we do, we count the number of commas to
     see whether the operand should be omitted.  */
  skip_optional = 0;
  for (opindex_ptr = opcode->operands; *opindex_ptr != 0; opindex_ptr++)
    {
      const struct powerpc_operand *operand;

      operand = &powerpc_operands[*opindex_ptr];
      if ((operand->flags & PPC_OPERAND_OPTIONAL) != 0)
	{
	  unsigned int opcount;

	  /* There is an optional operand.  Count the number of
	     commas in the input line.  */
	  if (*str == '\0')
	    opcount = 0;
	  else
	    {
	      opcount = 1;
	      s = str;
	      while ((s = strchr (s, ',')) != (char *) NULL)
		{
		  ++opcount;
		  ++s;
		}
	    }

	  /* If there are fewer operands in the line then are called
	     for by the instruction, we want to skip the optional
	     operand.  */
	  if (opcount < strlen (opcode->operands))
	    skip_optional = 1;

	  break;
	}
    }

  /* Gather the operands.  */
  need_paren = 0;
  next_opindex = 0;
  fc = 0;
  for (opindex_ptr = opcode->operands; *opindex_ptr != 0; opindex_ptr++)
    {
      const struct powerpc_operand *operand;
      const char *errmsg;
      char *hold;
      expressionS ex;
      char endc;

      if (next_opindex == 0)
	operand = &powerpc_operands[*opindex_ptr];
      else
	{
	  operand = &powerpc_operands[next_opindex];
	  next_opindex = 0;
	}

      errmsg = NULL;

      /* If this is a fake operand, then we do not expect anything
	 from the input.  */
      if ((operand->flags & PPC_OPERAND_FAKE) != 0)
	{
	  insn = (*operand->insert) (insn, 0L, &errmsg);
	  if (errmsg != (const char *) NULL)
	    as_warn (errmsg);
	  continue;
	}

      /* If this is an optional operand, and we are skipping it, just
	 insert a zero.  */
      if ((operand->flags & PPC_OPERAND_OPTIONAL) != 0
	  && skip_optional)
	{
	  if (operand->insert)
	    {
	      insn = (*operand->insert) (insn, 0L, &errmsg);
	      if (errmsg != (const char *) NULL)
		as_warn (errmsg);
	    }
	  if ((operand->flags & PPC_OPERAND_NEXT) != 0)
	    next_opindex = *opindex_ptr + 1;
	  continue;
	}

      /* Gather the operand.  */
      hold = input_line_pointer;
      input_line_pointer = str;

#ifdef TE_PE
      if (*input_line_pointer == '[') 
	{
	  /* We are expecting something like the second argument here:

	        lwz r4,[toc].GS.0.static_int(rtoc)
                       ^^^^^^^^^^^^^^^^^^^^^^^^^^^
	     The argument following the `]' must be a symbol name, and the 
             register must be the toc register: 'rtoc' or '2'

	     The effect is to 0 as the displacement field
	     in the instruction, and issue an IMAGE_REL_PPC_TOCREL16 (or
	     the appropriate variation) reloc against it based on the symbol.
	     The linker will build the toc, and insert the resolved toc offset.

	     Note:
	     o The size of the toc entry is currently assumed to be
	       32 bits. This should not be assumed to be a hard coded
	       number.
	     o In an effort to cope with a change from 32 to 64 bits,
	       there are also toc entries that are specified to be
	       either 32 or 64 bits:
                 lwz r4,[toc32].GS.0.static_int(rtoc)
	         lwz r4,[toc64].GS.0.static_int(rtoc)
	       These demand toc entries of the specified size, and the
	       instruction probably requires it.
          */

	  int valid_toc;
	  enum toc_size_qualifier toc_kind;
	  bfd_reloc_code_real_type toc_reloc;

	  /* go parse off the [tocXX] part */
	  valid_toc = parse_toc_entry(&toc_kind);

	  if (!valid_toc) 
	    {
	      /* Note: message has already been issued.     */
	      /* FIXME: what sort of recovery should we do? */
	      /*        demand_rest_of_line(); return; ?    */
	    }

	  /* Now get the symbol following the ']' */
	  expression(&ex);

	  switch (toc_kind)
	    {
	    case default_toc:
	      /* In this case, we may not have seen the symbol yet, since  */
	      /* it is allowed to appear on a .extern or .globl or just be */
	      /* a label in the .data section.                             */
	      toc_reloc = BFD_RELOC_PPC_TOC16;
	      break;
	    case data_in_toc:
	      /* 1. The symbol must be defined and either in the toc        */
	      /*    section, or a global.                                   */
	      /* 2. The reloc generated must have the TOCDEFN flag set in   */
	      /*    upper bit mess of the reloc type.                       */
	      /* FIXME: It's a little confusing what the tocv qualifier can */
	      /*        be used for. At the very least, I've seen three     */
	      /*        uses, only one of which I'm sure I can explain.     */
	      if (ex.X_op == O_symbol) 
		{		  
		  assert (ex.X_add_symbol != NULL);
		  if (ex.X_add_symbol->bsym->section != tocdata_section)
		    {
		      as_warn("[tocv] symbol is not a toc symbol");
		    }
		}

	      toc_reloc = BFD_RELOC_PPC_TOC16;
	      break;
	    case must_be_32:
	      /* FIXME: these next two specifically specify 32/64 bit toc   */
	      /*        entries. We don't support them today. Is this the   */
	      /*        right way to say that?                              */
	      toc_reloc = BFD_RELOC_UNUSED;
	      as_bad ("Unimplemented toc32 expression modifier");
	      break;
	    case must_be_64:
	      /* FIXME: see above */
	      toc_reloc = BFD_RELOC_UNUSED;
	      as_bad ("Unimplemented toc64 expression modifier");
	      break;
	    default:
	      fprintf(stderr, 
		      "Unexpected return value [%d] from parse_toc_entry!\n",
		      toc_kind);
	      abort();
	      break;
	    }

	  /* We need to generate a fixup for this expression.  */
	  if (fc >= MAX_INSN_FIXUPS)
	    as_fatal ("too many fixups");

	  fixups[fc].reloc = toc_reloc;
	  fixups[fc].exp = ex;
	  fixups[fc].opindex = *opindex_ptr;
	  ++fc;

          /* Ok. We've set up the fixup for the instruction. Now make it
	     look like the constant 0 was found here */
	  ex.X_unsigned = 1;
	  ex.X_op = O_constant;
	  ex.X_add_number = 0;
	  ex.X_add_symbol = NULL;
	  ex.X_op_symbol = NULL;
	}

      else
#endif		/* TE_PE */
	if (!register_name(&ex))
	  expression (&ex);

      str = input_line_pointer;
      input_line_pointer = hold;

      if (ex.X_op == O_illegal)
	as_bad ("illegal operand");
      else if (ex.X_op == O_absent)
	as_bad ("missing operand");
      else if (ex.X_op == O_register)
	{
	  insn = ppc_insert_operand (insn, operand, ex.X_add_number,
				     (char *) NULL, 0);
	}
      else if (ex.X_op == O_constant)
	{
#ifdef OBJ_ELF
	  /* Allow @HA, @L, @H on constants. */
	  char *orig_str = str;

	  if ((reloc = ppc_elf_suffix (&str)) != BFD_RELOC_UNUSED)
	    switch (reloc)
	      {
	      default:
		str = orig_str;
		break;

	      case BFD_RELOC_LO16:
		if (ex.X_unsigned)
		  ex.X_add_number &= 0xffff;
		else
		  ex.X_add_number = (((ex.X_add_number & 0xffff)
				      ^ 0x8000)
				     - 0x8000);
		break;

	      case BFD_RELOC_HI16:
		ex.X_add_number = (ex.X_add_number >> 16) & 0xffff;
		break;

	      case BFD_RELOC_HI16_S:
		ex.X_add_number = (((ex.X_add_number >> 16) & 0xffff)
				   + ((ex.X_add_number >> 15) & 1));
		break;
	      }
#endif
	  insn = ppc_insert_operand (insn, operand, ex.X_add_number,
				     (char *) NULL, 0);
	}
#ifdef OBJ_ELF
      else if ((reloc = ppc_elf_suffix (&str)) != BFD_RELOC_UNUSED)
	{
	  /* For the absoulte forms of branchs, convert the PC relative form back into
	     the absolute.  */
	  if ((operand->flags & PPC_OPERAND_ABSOLUTE) != 0)
	    {
	      switch (reloc)
		{
		case BFD_RELOC_PPC_B26:
		  reloc = BFD_RELOC_PPC_BA26;
		  break;
		case BFD_RELOC_PPC_B16:
		  reloc = BFD_RELOC_PPC_BA16;
		  break;
		case BFD_RELOC_PPC_B16_BRTAKEN:
		  reloc = BFD_RELOC_PPC_BA16_BRTAKEN;
		  break;
		case BFD_RELOC_PPC_B16_BRNTAKEN:
		  reloc = BFD_RELOC_PPC_BA16_BRNTAKEN;
		  break;
		default:
		  break;
		}
	    }

	  /* We need to generate a fixup for this expression.  */
	  if (fc >= MAX_INSN_FIXUPS)
	    as_fatal ("too many fixups");
	  fixups[fc].exp = ex;
	  fixups[fc].opindex = 0;
	  fixups[fc].reloc = reloc;
	  ++fc;
	}
#endif /* OBJ_ELF */

      else
	{
	  /* We need to generate a fixup for this expression.  */
	  if (fc >= MAX_INSN_FIXUPS)
	    as_fatal ("too many fixups");
	  fixups[fc].exp = ex;
	  fixups[fc].opindex = *opindex_ptr;
	  fixups[fc].reloc = BFD_RELOC_UNUSED;
	  ++fc;
	}

      if (need_paren)
	{
	  endc = ')';
	  need_paren = 0;
	}
      else if ((operand->flags & PPC_OPERAND_PARENS) != 0)
	{
	  endc = '(';
	  need_paren = 1;
	}
      else
	endc = ',';

      /* The call to expression should have advanced str past any
	 whitespace.  */
      if (*str != endc
	  && (endc != ',' || *str != '\0'))
	{
	  as_bad ("syntax error; found `%c' but expected `%c'", *str, endc);
	  break;
	}

      if (*str != '\0')
	++str;
    }

  while (isspace (*str))
    ++str;

  if (*str != '\0')
    as_bad ("junk at end of line: `%s'", str);

  /* Write out the instruction.  */
  f = frag_more (4);
  md_number_to_chars (f, insn, 4);

  /* Create any fixups.  At this point we do not use a
     bfd_reloc_code_real_type, but instead just use the
     BFD_RELOC_UNUSED plus the operand index.  This lets us easily
     handle fixups for any operand type, although that is admittedly
     not a very exciting feature.  We pick a BFD reloc type in
     md_apply_fix.  */
  for (i = 0; i < fc; i++)
    {
      const struct powerpc_operand *operand;

      operand = &powerpc_operands[fixups[i].opindex];
      if (fixups[i].reloc != BFD_RELOC_UNUSED)
	{
	  reloc_howto_type *reloc_howto = bfd_reloc_type_lookup (stdoutput, fixups[i].reloc);
	  int size;
	  int offset;
	  fixS *fixP;

	  if (!reloc_howto)
	    abort ();

	  size = bfd_get_reloc_size (reloc_howto);
	  offset = target_big_endian ? (4 - size) : 0;

	  if (size < 1 || size > 4)
	    abort();

	  fixP = fix_new_exp (frag_now, f - frag_now->fr_literal + offset, size,
			      &fixups[i].exp, reloc_howto->pc_relative,
			      fixups[i].reloc);

	  /* Turn off complaints that the addend is too large for things like
	     foo+100000@ha.  */
	  switch (fixups[i].reloc)
	    {
	    case BFD_RELOC_16_GOTOFF:
	    case BFD_RELOC_PPC_TOC16:
	    case BFD_RELOC_LO16:
	    case BFD_RELOC_HI16:
	    case BFD_RELOC_HI16_S:
	      fixP->fx_no_overflow = 1;
	      break;
	    default:
	      break;
	    }
	}
      else
	fix_new_exp (frag_now, f - frag_now->fr_literal, 4,
		     &fixups[i].exp,
		     (operand->flags & PPC_OPERAND_RELATIVE) != 0,
		     ((bfd_reloc_code_real_type)
		       (fixups[i].opindex + (int) BFD_RELOC_UNUSED)));
    }
}

#ifndef WORKING_DOT_WORD
/* Handle long and short jumps */
void
md_create_short_jump (ptr, from_addr, to_addr, frag, to_symbol)
     char *ptr;
     addressT from_addr, to_addr;
     fragS *frag;
     symbolS *to_symbol;
{
  abort ();
}

void
md_create_long_jump (ptr, from_addr, to_addr, frag, to_symbol)
     char *ptr;
     addressT from_addr, to_addr;
     fragS *frag;
     symbolS *to_symbol;
{
  abort ();
}
#endif

/* Handle a macro.  Gather all the operands, transform them as
   described by the macro, and call md_assemble recursively.  All the
   operands are separated by commas; we don't accept parentheses
   around operands here.  */

static void
ppc_macro (str, macro)
     char *str;
     const struct powerpc_macro *macro;
{
  char *operands[10];
  unsigned int count;
  char *s;
  unsigned int len;
  const char *format;
  int arg;
  char *send;
  char *complete;

  /* Gather the users operands into the operands array.  */
  count = 0;
  s = str;
  while (1)
    {
      if (count >= sizeof operands / sizeof operands[0])
	break;
      operands[count++] = s;
      s = strchr (s, ',');
      if (s == (char *) NULL)
	break;
      *s++ = '\0';
    }  

  if (count != macro->operands)
    {
      as_bad ("wrong number of operands");
      return;
    }

  /* Work out how large the string must be (the size is unbounded
     because it includes user input).  */
  len = 0;
  format = macro->format;
  while (*format != '\0')
    {
      if (*format != '%')
	{
	  ++len;
	  ++format;
	}
      else
	{
	  arg = strtol (format + 1, &send, 10);
	  know (send != format && arg >= 0 && arg < count);
	  len += strlen (operands[arg]);
	  format = send;
	}
    }

  /* Put the string together.  */
  complete = s = (char *) alloca (len + 1);
  format = macro->format;
  while (*format != '\0')
    {
      if (*format != '%')
	*s++ = *format++;
      else
	{
	  arg = strtol (format + 1, &send, 10);
	  strcpy (s, operands[arg]);
	  s += strlen (s);
	  format = send;
	}
    }
  *s = '\0';

  /* Assemble the constructed instruction.  */
  md_assemble (complete);
}  

#ifdef OBJ_ELF
/* For ELF, add support for SHF_EXCLUDE and SHT_ORDERED */

int
ppc_section_letter (letter, ptr_msg)
     int letter;
     char **ptr_msg;
{
  if (letter == 'e')
    return SHF_EXCLUDE;

  *ptr_msg = "Bad .section directive: want a,w,x,e in string";
  return 0;
}

int
ppc_section_word (ptr_str)
     char **ptr_str;
{
  if (strncmp (*ptr_str, "exclude", sizeof ("exclude")-1) == 0)
    {
      *ptr_str += sizeof ("exclude")-1;
      return SHF_EXCLUDE;
    }

  return 0;
}

int
ppc_section_type (ptr_str)
     char **ptr_str;
{
  if (strncmp (*ptr_str, "ordered", sizeof ("ordered")-1) == 0)
    {
      *ptr_str += sizeof ("ordered")-1;
      return SHT_ORDERED;
    }

  return 0;
}

int
ppc_section_flags (flags, attr, type)
     int flags;
     int attr;
     int type;
{
  if (type == SHT_ORDERED)
    flags |= SEC_ALLOC | SEC_LOAD | SEC_SORT_ENTRIES;

  if (attr & SHF_EXCLUDE)
    flags |= SEC_EXCLUDE;

  return flags;
}
#endif /* OBJ_ELF */


/* Pseudo-op handling.  */

/* The .byte pseudo-op.  This is similar to the normal .byte
   pseudo-op, but it can also take a single ASCII string.  */

static void
ppc_byte (ignore)
     int ignore;
{
  if (*input_line_pointer != '\"')
    {
      cons (1);
      return;
    }

  /* Gather characters.  A real double quote is doubled.  Unusual
     characters are not permitted.  */
  ++input_line_pointer;
  while (1)
    {
      char c;

      c = *input_line_pointer++;

      if (c == '\"')
	{
	  if (*input_line_pointer != '\"')
	    break;
	  ++input_line_pointer;
	}

      FRAG_APPEND_1_CHAR (c);
    }

  demand_empty_rest_of_line ();
}

#ifdef OBJ_XCOFF

/* XCOFF specific pseudo-op handling.  */

/* This is set if we are creating a .stabx symbol, since we don't want
   to handle symbol suffixes for such symbols.  */
static boolean ppc_stab_symbol;

/* The .comm and .lcomm pseudo-ops for XCOFF.  XCOFF puts common
   symbols in the .bss segment as though they were local common
   symbols, and uses a different smclas.  */

static void
ppc_comm (lcomm)
     int lcomm;
{
  asection *current_seg = now_seg;
  subsegT current_subseg = now_subseg;
  char *name;
  char endc;
  char *end_name;
  offsetT size;
  offsetT align;
  symbolS *lcomm_sym = NULL;
  symbolS *sym;
  char *pfrag;

  name = input_line_pointer;
  endc = get_symbol_end ();
  end_name = input_line_pointer;
  *end_name = endc;

  if (*input_line_pointer != ',')
    {
      as_bad ("missing size");
      ignore_rest_of_line ();
      return;
    }
  ++input_line_pointer;

  size = get_absolute_expression ();
  if (size < 0)
    {
      as_bad ("negative size");
      ignore_rest_of_line ();
      return;
    }

  if (! lcomm)
    {
      /* The third argument to .comm is the alignment.  */
      if (*input_line_pointer != ',')
	align = 3;
      else
	{
	  ++input_line_pointer;
	  align = get_absolute_expression ();
	  if (align <= 0)
	    {
	      as_warn ("ignoring bad alignment");
	      align = 3;
	    }
	}
    }
  else
    {
      char *lcomm_name;
      char lcomm_endc;

      if (size <= 1)
	align = 0;
      else if (size <= 2)
	align = 1;
      else if (size <= 4)
	align = 2;
      else
	align = 3;

      /* The third argument to .lcomm appears to be the real local
	 common symbol to create.  References to the symbol named in
	 the first argument are turned into references to the third
	 argument.  */
      if (*input_line_pointer != ',')
	{
	  as_bad ("missing real symbol name");
	  ignore_rest_of_line ();
	  return;
	}
      ++input_line_pointer;

      lcomm_name = input_line_pointer;
      lcomm_endc = get_symbol_end ();
      
      lcomm_sym = symbol_find_or_make (lcomm_name);

      *input_line_pointer = lcomm_endc;
    }

  *end_name = '\0';
  sym = symbol_find_or_make (name);
  *end_name = endc;

  if (S_IS_DEFINED (sym)
      || S_GET_VALUE (sym) != 0)
    {
      as_bad ("attempt to redefine symbol");
      ignore_rest_of_line ();
      return;
    }
    
  record_alignment (bss_section, align);
	  
  if (! lcomm
      || ! S_IS_DEFINED (lcomm_sym))
    {
      symbolS *def_sym;
      offsetT def_size;

      if (! lcomm)
	{
	  def_sym = sym;
	  def_size = size;
	  S_SET_EXTERNAL (sym);
	}
      else
	{
	  lcomm_sym->sy_tc.output = 1;
	  def_sym = lcomm_sym;
	  def_size = 0;
	}

      subseg_set (bss_section, 1);
      frag_align (align, 0);
  
      def_sym->sy_frag = frag_now;
      pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, def_sym,
			def_size, (char *) NULL);
      *pfrag = 0;
      S_SET_SEGMENT (def_sym, bss_section);
      def_sym->sy_tc.align = align;
    }
  else if (lcomm)
    {
      /* Align the size of lcomm_sym.  */
      lcomm_sym->sy_frag->fr_offset =
	((lcomm_sym->sy_frag->fr_offset + (1 << align) - 1)
	 &~ ((1 << align) - 1));
      if (align > lcomm_sym->sy_tc.align)
	lcomm_sym->sy_tc.align = align;
    }

  if (lcomm)
    {
      /* Make sym an offset from lcomm_sym.  */
      S_SET_SEGMENT (sym, bss_section);
      sym->sy_frag = lcomm_sym->sy_frag;
      S_SET_VALUE (sym, lcomm_sym->sy_frag->fr_offset);
      lcomm_sym->sy_frag->fr_offset += size;
    }

  subseg_set (current_seg, current_subseg);

  demand_empty_rest_of_line ();
}

/* The .csect pseudo-op.  This switches us into a different
   subsegment.  The first argument is a symbol whose value is the
   start of the .csect.  In COFF, csect symbols get special aux
   entries defined by the x_csect field of union internal_auxent.  The
   optional second argument is the alignment (the default is 2).  */

static void
ppc_csect (ignore)
     int ignore;
{
  char *name;
  char endc;
  symbolS *sym;

  name = input_line_pointer;
  endc = get_symbol_end ();
  
  sym = symbol_find_or_make (name);

  *input_line_pointer = endc;

  if (S_GET_NAME (sym)[0] == '\0')
    {
      /* An unnamed csect is assumed to be [PR].  */
      sym->sy_tc.class = XMC_PR;
    }

  ppc_change_csect (sym);

  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;
      sym->sy_tc.align = get_absolute_expression ();
    }

  demand_empty_rest_of_line ();
}

/* Change to a different csect.  */

static void
ppc_change_csect (sym)
     symbolS *sym;
{
  if (S_IS_DEFINED (sym))
    subseg_set (S_GET_SEGMENT (sym), sym->sy_tc.subseg);
  else
    {
      symbolS **list_ptr;
      int after_toc;
      symbolS *list;

      /* This is a new csect.  We need to look at the symbol class to
	 figure out whether it should go in the text section or the
	 data section.  */
      after_toc = 0;
      switch (sym->sy_tc.class)
	{
	case XMC_PR:
	case XMC_RO:
	case XMC_DB:
	case XMC_GL:
	case XMC_XO:
	case XMC_SV:
	case XMC_TI:
	case XMC_TB:
	  S_SET_SEGMENT (sym, text_section);
	  sym->sy_tc.subseg = ppc_text_subsegment;
	  ++ppc_text_subsegment;
	  list_ptr = &ppc_text_csects;
	  break;
	case XMC_RW:
	case XMC_TC0:
	case XMC_TC:
	case XMC_DS:
	case XMC_UA:
	case XMC_BS:
	case XMC_UC:
	  if (ppc_toc_csect != NULL
	      && ppc_toc_csect->sy_tc.subseg + 1 == ppc_data_subsegment)
	    after_toc = 1;
	  S_SET_SEGMENT (sym, data_section);
	  sym->sy_tc.subseg = ppc_data_subsegment;
	  ++ppc_data_subsegment;
	  list_ptr = &ppc_data_csects;
	  break;
	default:
	  abort ();
	}

      subseg_new (segment_name (S_GET_SEGMENT (sym)), sym->sy_tc.subseg);
      if (after_toc)
	ppc_after_toc_frag = frag_now;

      sym->sy_frag = frag_now;
      S_SET_VALUE (sym, (valueT) frag_now_fix ());

      sym->sy_tc.align = 2;
      sym->sy_tc.output = 1;
      sym->sy_tc.within = sym;
	  
      for (list = *list_ptr;
	   list->sy_tc.next != (symbolS *) NULL;
	   list = list->sy_tc.next)
	;
      list->sy_tc.next = sym;
	  
      symbol_remove (sym, &symbol_rootP, &symbol_lastP);
      symbol_append (sym, list->sy_tc.within, &symbol_rootP, &symbol_lastP);
    }

  ppc_current_csect = sym;
}

/* This function handles the .text and .data pseudo-ops.  These
   pseudo-ops aren't really used by XCOFF; we implement them for the
   convenience of people who aren't used to XCOFF.  */

static void
ppc_section (type)
     int type;
{
  const char *name;
  symbolS *sym;

  if (type == 't')
    name = ".text[PR]";
  else if (type == 'd')
    name = ".data[RW]";
  else
    abort ();

  sym = symbol_find_or_make (name);

  ppc_change_csect (sym);

  demand_empty_rest_of_line ();
}

/* The .extern pseudo-op.  We create an undefined symbol.  */

static void
ppc_extern (ignore)
     int ignore;
{
  char *name;
  char endc;

  name = input_line_pointer;
  endc = get_symbol_end ();

  (void) symbol_find_or_make (name);

  *input_line_pointer = endc;

  demand_empty_rest_of_line ();
}

/* The .lglobl pseudo-op.  Keep the symbol in the symbol table.  */

static void
ppc_lglobl (ignore)
     int ignore;
{
  char *name;
  char endc;
  symbolS *sym;

  name = input_line_pointer;
  endc = get_symbol_end ();

  sym = symbol_find_or_make (name);

  *input_line_pointer = endc;

  sym->sy_tc.output = 1;

  demand_empty_rest_of_line ();
}

/* The .rename pseudo-op.  The RS/6000 assembler can rename symbols,
   although I don't know why it bothers.  */

static void
ppc_rename (ignore)
     int ignore;
{
  char *name;
  char endc;
  symbolS *sym;
  int len;

  name = input_line_pointer;
  endc = get_symbol_end ();

  sym = symbol_find_or_make (name);

  *input_line_pointer = endc;

  if (*input_line_pointer != ',')
    {
      as_bad ("missing rename string");
      ignore_rest_of_line ();
      return;
    }
  ++input_line_pointer;

  sym->sy_tc.real_name = demand_copy_C_string (&len);

  demand_empty_rest_of_line ();
}

/* The .stabx pseudo-op.  This is similar to a normal .stabs
   pseudo-op, but slightly different.  A sample is
       .stabx "main:F-1",.main,142,0
   The first argument is the symbol name to create.  The second is the
   value, and the third is the storage class.  The fourth seems to be
   always zero, and I am assuming it is the type.  */

static void
ppc_stabx (ignore)
     int ignore;
{
  char *name;
  int len;
  symbolS *sym;
  expressionS exp;

  name = demand_copy_C_string (&len);

  if (*input_line_pointer != ',')
    {
      as_bad ("missing value");
      return;
    }
  ++input_line_pointer;

  ppc_stab_symbol = true;
  sym = symbol_make (name);
  ppc_stab_symbol = false;

  sym->sy_tc.real_name = name;

  (void) expression (&exp);

  switch (exp.X_op)
    {
    case O_illegal:
    case O_absent:
    case O_big:
      as_bad ("illegal .stabx expression; zero assumed");
      exp.X_add_number = 0;
      /* Fall through.  */
    case O_constant:
      S_SET_VALUE (sym, (valueT) exp.X_add_number);
      sym->sy_frag = &zero_address_frag;
      break;

    case O_symbol:
      if (S_GET_SEGMENT (exp.X_add_symbol) == undefined_section)
	sym->sy_value = exp;
      else
	{
	  S_SET_VALUE (sym,
		       exp.X_add_number + S_GET_VALUE (exp.X_add_symbol));
	  sym->sy_frag = exp.X_add_symbol->sy_frag;
	}
      break;

    default:
      /* The value is some complex expression.  This will probably
         fail at some later point, but this is probably the right
         thing to do here.  */
      sym->sy_value = exp;
      break;
    }

  S_SET_SEGMENT (sym, ppc_coff_debug_section);
  sym->bsym->flags |= BSF_DEBUGGING;

  if (*input_line_pointer != ',')
    {
      as_bad ("missing class");
      return;
    }
  ++input_line_pointer;

  S_SET_STORAGE_CLASS (sym, get_absolute_expression ());

  if (*input_line_pointer != ',')
    {
      as_bad ("missing type");
      return;
    }
  ++input_line_pointer;

  S_SET_DATA_TYPE (sym, get_absolute_expression ());

  sym->sy_tc.output = 1;

  if (S_GET_STORAGE_CLASS (sym) == C_STSYM)
    sym->sy_tc.within = ppc_current_block;

  if (exp.X_op != O_symbol
      || ! S_IS_EXTERNAL (exp.X_add_symbol)
      || S_GET_SEGMENT (exp.X_add_symbol) != bss_section)
    ppc_frob_label (sym);
  else
    {
      symbol_remove (sym, &symbol_rootP, &symbol_lastP);
      symbol_append (sym, exp.X_add_symbol, &symbol_rootP, &symbol_lastP);
      if (ppc_current_csect->sy_tc.within == exp.X_add_symbol)
	ppc_current_csect->sy_tc.within = sym;
    }

  demand_empty_rest_of_line ();
}

/* The .function pseudo-op.  This takes several arguments.  The first
   argument seems to be the external name of the symbol.  The second
   argment seems to be the label for the start of the function.  gcc
   uses the same name for both.  I have no idea what the third and
   fourth arguments are meant to be.  The optional fifth argument is
   an expression for the size of the function.  In COFF this symbol
   gets an aux entry like that used for a csect.  */

static void
ppc_function (ignore)
     int ignore;
{
  char *name;
  char endc;
  char *s;
  symbolS *ext_sym;
  symbolS *lab_sym;

  name = input_line_pointer;
  endc = get_symbol_end ();

  /* Ignore any [PR] suffix.  */
  name = ppc_canonicalize_symbol_name (name);
  s = strchr (name, '[');
  if (s != (char *) NULL
      && strcmp (s + 1, "PR]") == 0)
    *s = '\0';

  ext_sym = symbol_find_or_make (name);

  *input_line_pointer = endc;

  if (*input_line_pointer != ',')
    {
      as_bad ("missing symbol name");
      ignore_rest_of_line ();
      return;
    }
  ++input_line_pointer;

  name = input_line_pointer;
  endc = get_symbol_end ();

  lab_sym = symbol_find_or_make (name);

  *input_line_pointer = endc;

  if (ext_sym != lab_sym)
    {
      ext_sym->sy_value.X_op = O_symbol;
      ext_sym->sy_value.X_add_symbol = lab_sym;
      ext_sym->sy_value.X_op_symbol = NULL;
      ext_sym->sy_value.X_add_number = 0;
    }

  if (ext_sym->sy_tc.class == -1)
    ext_sym->sy_tc.class = XMC_PR;
  ext_sym->sy_tc.output = 1;

  if (*input_line_pointer == ',')
    {
      expressionS ignore;

      /* Ignore the third argument.  */
      ++input_line_pointer;
      expression (&ignore);
      if (*input_line_pointer == ',')
	{
	  /* Ignore the fourth argument.  */
	  ++input_line_pointer;
	  expression (&ignore);
	  if (*input_line_pointer == ',')
	    {
	      /* The fifth argument is the function size.  */
	      ++input_line_pointer;
	      ext_sym->sy_tc.size = symbol_new ("L0\001",
						absolute_section,
						(valueT) 0,
						&zero_address_frag);
	      pseudo_set (ext_sym->sy_tc.size);
	    }
	}
    }

  S_SET_DATA_TYPE (ext_sym, DT_FCN << N_BTSHFT);
  SF_SET_FUNCTION (ext_sym);
  SF_SET_PROCESS (ext_sym);
  coff_add_linesym (ext_sym);

  demand_empty_rest_of_line ();
}

/* The .bf pseudo-op.  This is just like a COFF C_FCN symbol named
   ".bf".  */

static void
ppc_bf (ignore)
     int ignore;
{
  symbolS *sym;

  sym = symbol_make (".bf");
  S_SET_SEGMENT (sym, text_section);
  sym->sy_frag = frag_now;
  S_SET_VALUE (sym, frag_now_fix ());
  S_SET_STORAGE_CLASS (sym, C_FCN);

  coff_line_base = get_absolute_expression ();

  S_SET_NUMBER_AUXILIARY (sym, 1);
  SA_SET_SYM_LNNO (sym, coff_line_base);

  sym->sy_tc.output = 1;

  ppc_frob_label (sym);

  demand_empty_rest_of_line ();
}

/* The .ef pseudo-op.  This is just like a COFF C_FCN symbol named
   ".ef", except that the line number is absolute, not relative to the
   most recent ".bf" symbol.  */

static void
ppc_ef (ignore)
     int ignore;
{
  symbolS *sym;

  sym = symbol_make (".ef");
  S_SET_SEGMENT (sym, text_section);
  sym->sy_frag = frag_now;
  S_SET_VALUE (sym, frag_now_fix ());
  S_SET_STORAGE_CLASS (sym, C_FCN);
  S_SET_NUMBER_AUXILIARY (sym, 1);
  SA_SET_SYM_LNNO (sym, get_absolute_expression ());
  sym->sy_tc.output = 1;

  ppc_frob_label (sym);

  demand_empty_rest_of_line ();
}

/* The .bi and .ei pseudo-ops.  These take a string argument and
   generates a C_BINCL or C_EINCL symbol, which goes at the start of
   the symbol list.  */

static void
ppc_biei (ei)
     int ei;
{
  char *name;
  int len;
  symbolS *sym;
  symbolS *look;

  name = demand_copy_C_string (&len);

  /* The value of these symbols is actually file offset.  Here we set
     the value to the index into the line number entries.  In
     ppc_frob_symbols we set the fix_line field, which will cause BFD
     to do the right thing.  */

  sym = symbol_make (name);
  /* obj-coff.c currently only handles line numbers correctly in the
     .text section.  */
  S_SET_SEGMENT (sym, text_section);
  S_SET_VALUE (sym, coff_n_line_nos);
  sym->bsym->flags |= BSF_DEBUGGING;

  S_SET_STORAGE_CLASS (sym, ei ? C_EINCL : C_BINCL);
  sym->sy_tc.output = 1;
  
  for (look = symbol_rootP;
       (look != (symbolS *) NULL
	&& (S_GET_STORAGE_CLASS (look) == C_FILE
	    || S_GET_STORAGE_CLASS (look) == C_BINCL
	    || S_GET_STORAGE_CLASS (look) == C_EINCL));
       look = symbol_next (look))
    ;
  if (look != (symbolS *) NULL)
    {
      symbol_remove (sym, &symbol_rootP, &symbol_lastP);
      symbol_insert (sym, look, &symbol_rootP, &symbol_lastP);
    }

  demand_empty_rest_of_line ();
}

/* The .bs pseudo-op.  This generates a C_BSTAT symbol named ".bs".
   There is one argument, which is a csect symbol.  The value of the
   .bs symbol is the index of this csect symbol.  */

static void
ppc_bs (ignore)
     int ignore;
{
  char *name;
  char endc;
  symbolS *csect;
  symbolS *sym;

  if (ppc_current_block != NULL)
    as_bad ("nested .bs blocks");

  name = input_line_pointer;
  endc = get_symbol_end ();

  csect = symbol_find_or_make (name);

  *input_line_pointer = endc;

  sym = symbol_make (".bs");
  S_SET_SEGMENT (sym, now_seg);
  S_SET_STORAGE_CLASS (sym, C_BSTAT);
  sym->bsym->flags |= BSF_DEBUGGING;
  sym->sy_tc.output = 1;

  sym->sy_tc.within = csect;

  ppc_frob_label (sym);

  ppc_current_block = sym;

  demand_empty_rest_of_line ();
}

/* The .es pseudo-op.  Generate a C_ESTART symbol named .es.  */

static void
ppc_es (ignore)
     int ignore;
{
  symbolS *sym;

  if (ppc_current_block == NULL)
    as_bad (".es without preceding .bs");

  sym = symbol_make (".es");
  S_SET_SEGMENT (sym, now_seg);
  S_SET_STORAGE_CLASS (sym, C_ESTAT);
  sym->bsym->flags |= BSF_DEBUGGING;
  sym->sy_tc.output = 1;

  ppc_frob_label (sym);

  ppc_current_block = NULL;

  demand_empty_rest_of_line ();
}

/* The .bb pseudo-op.  Generate a C_BLOCK symbol named .bb, with a
   line number.  */

static void
ppc_bb (ignore)
     int ignore;
{
  symbolS *sym;

  sym = symbol_make (".bb");
  S_SET_SEGMENT (sym, text_section);
  sym->sy_frag = frag_now;
  S_SET_VALUE (sym, frag_now_fix ());
  S_SET_STORAGE_CLASS (sym, C_BLOCK);

  S_SET_NUMBER_AUXILIARY (sym, 1);
  SA_SET_SYM_LNNO (sym, get_absolute_expression ());

  sym->sy_tc.output = 1;

  SF_SET_PROCESS (sym);

  ppc_frob_label (sym);

  demand_empty_rest_of_line ();
}

/* The .eb pseudo-op.  Generate a C_BLOCK symbol named .eb, with a
   line number.  */

static void
ppc_eb (ignore)
     int ignore;
{
  symbolS *sym;

  sym = symbol_make (".eb");
  S_SET_SEGMENT (sym, text_section);
  sym->sy_frag = frag_now;
  S_SET_VALUE (sym, frag_now_fix ());
  S_SET_STORAGE_CLASS (sym, C_BLOCK);
  S_SET_NUMBER_AUXILIARY (sym, 1);
  SA_SET_SYM_LNNO (sym, get_absolute_expression ());
  sym->sy_tc.output = 1;

  SF_SET_PROCESS (sym);

  ppc_frob_label (sym);

  demand_empty_rest_of_line ();
}

/* The .bc pseudo-op.  This just creates a C_BCOMM symbol with a
   specified name.  */

static void
ppc_bc (ignore)
     int ignore;
{
  char *name;
  int len;
  symbolS *sym;

  name = demand_copy_C_string (&len);
  sym = symbol_make (name);
  S_SET_SEGMENT (sym, ppc_coff_debug_section);
  sym->bsym->flags |= BSF_DEBUGGING;
  S_SET_STORAGE_CLASS (sym, C_BCOMM);
  S_SET_VALUE (sym, 0);
  sym->sy_tc.output = 1;

  ppc_frob_label (sym);

  demand_empty_rest_of_line ();
}

/* The .ec pseudo-op.  This just creates a C_ECOMM symbol.  */

static void
ppc_ec (ignore)
     int ignore;
{
  symbolS *sym;

  sym = symbol_make (".ec");
  S_SET_SEGMENT (sym, ppc_coff_debug_section);
  sym->bsym->flags |= BSF_DEBUGGING;
  S_SET_STORAGE_CLASS (sym, C_ECOMM);
  S_SET_VALUE (sym, 0);
  sym->sy_tc.output = 1;

  ppc_frob_label (sym);

  demand_empty_rest_of_line ();
}

/* The .toc pseudo-op.  Switch to the .toc subsegment.  */

static void
ppc_toc (ignore)
     int ignore;
{
  if (ppc_toc_csect != (symbolS *) NULL)
    subseg_set (data_section, ppc_toc_csect->sy_tc.subseg);
  else
    {
      subsegT subseg;
      symbolS *sym;
      symbolS *list;
    
      subseg = ppc_data_subsegment;
      ++ppc_data_subsegment;

      subseg_new (segment_name (data_section), subseg);
      ppc_toc_frag = frag_now;

      sym = symbol_find_or_make ("TOC[TC0]");
      sym->sy_frag = frag_now;
      S_SET_SEGMENT (sym, data_section);
      S_SET_VALUE (sym, (valueT) frag_now_fix ());
      sym->sy_tc.subseg = subseg;
      sym->sy_tc.output = 1;
      sym->sy_tc.within = sym;

      ppc_toc_csect = sym;
	  
      for (list = ppc_data_csects;
	   list->sy_tc.next != (symbolS *) NULL;
	   list = list->sy_tc.next)
	;
      list->sy_tc.next = sym;

      symbol_remove (sym, &symbol_rootP, &symbol_lastP);
      symbol_append (sym, list->sy_tc.within, &symbol_rootP, &symbol_lastP);
    }

  ppc_current_csect = ppc_toc_csect;

  demand_empty_rest_of_line ();
}

#endif /* OBJ_XCOFF */

/* The .tc pseudo-op.  This is used when generating either XCOFF or
   ELF.  This takes two or more arguments.

   When generating XCOFF output, the first argument is the name to
   give to this location in the toc; this will be a symbol with class
   TC.  The rest of the arguments are 4 byte values to actually put at
   this location in the TOC; often there is just one more argument, a
   relocateable symbol reference.

   When not generating XCOFF output, the arguments are the same, but
   the first argument is simply ignored.  */

static void
ppc_tc (ignore)
     int ignore;
{
#ifdef OBJ_XCOFF

  /* Define the TOC symbol name.  */
  {
    char *name;
    char endc;
    symbolS *sym;

    if (ppc_toc_csect == (symbolS *) NULL
	|| ppc_toc_csect != ppc_current_csect)
      {
	as_bad (".tc not in .toc section");
	ignore_rest_of_line ();
	return;
      }

    name = input_line_pointer;
    endc = get_symbol_end ();

    sym = symbol_find_or_make (name);

    *input_line_pointer = endc;

    if (S_IS_DEFINED (sym))
      {
	symbolS *label;

	label = ppc_current_csect->sy_tc.within;
	if (label->sy_tc.class != XMC_TC0)
	  {
	    as_warn (".tc with no label");
	    ignore_rest_of_line ();
	    return;
	  }

	S_SET_SEGMENT (label, S_GET_SEGMENT (sym));
	label->sy_frag = sym->sy_frag;
	S_SET_VALUE (label, S_GET_VALUE (sym));

	while (! is_end_of_line[(unsigned char) *input_line_pointer])
	  ++input_line_pointer;

	return;
      }

    S_SET_SEGMENT (sym, now_seg);
    sym->sy_frag = frag_now;
    S_SET_VALUE (sym, (valueT) frag_now_fix ());
    sym->sy_tc.class = XMC_TC;
    sym->sy_tc.output = 1;

    ppc_frob_label (sym);
  }

#else /* ! defined (OBJ_XCOFF) */

  /* Skip the TOC symbol name.  */
  while (is_part_of_name (*input_line_pointer)
	 || *input_line_pointer == '['
	 || *input_line_pointer == ']'
	 || *input_line_pointer == '{'
	 || *input_line_pointer == '}')
    ++input_line_pointer;

  /* Align to a four byte boundary.  */
  frag_align (2, 0);
  record_alignment (now_seg, 2);

#endif /* ! defined (OBJ_XCOFF) */

  if (*input_line_pointer != ',')
    demand_empty_rest_of_line ();
  else
    {
      ++input_line_pointer;
      cons (4);
    }
}

#ifdef TE_PE

/* Pseudo-ops specific to the Windows NT PowerPC PE (coff) format */

/* Set the current section.  */
static void
ppc_set_current_section (new)
     segT new;
{
  ppc_previous_section = ppc_current_section;
  ppc_current_section = new;
}

/* pseudo-op: .previous
   behaviour: toggles the current section with the previous section.
   errors:    None
   warnings:  "No previous section"
*/
static void
ppc_previous(ignore)
     int ignore;
{
  symbolS *tmp;

  if (ppc_previous_section == NULL) 
    {
      as_warn("No previous section to return to. Directive ignored.");
      return;
    }

  subseg_set(ppc_previous_section, 0);

  ppc_set_current_section(ppc_previous_section);
}

/* pseudo-op: .pdata
   behaviour: predefined read only data section
              double word aligned
   errors:    None
   warnings:  None
   initial:   .section .pdata "adr3"
              a - don't know -- maybe a misprint
	      d - initialized data
	      r - readable
	      3 - double word aligned (that would be 4 byte boundary)

   commentary:
   Tag index tables (also known as the function table) for exception
   handling, debugging, etc.

*/
static void
ppc_pdata(ignore)
     int ignore;
{
  if (pdata_section == 0) 
    {
      pdata_section = subseg_new (".pdata", 0);
      
      bfd_set_section_flags (stdoutput, pdata_section,
			     (SEC_ALLOC | SEC_LOAD | SEC_RELOC
			      | SEC_READONLY | SEC_DATA ));
      
      bfd_set_section_alignment (stdoutput, pdata_section, 2);
    }
  else
    {
      pdata_section = subseg_new(".pdata", 0);
    }
  ppc_set_current_section(pdata_section);
}

/* pseudo-op: .ydata
   behaviour: predefined read only data section
              double word aligned
   errors:    None
   warnings:  None
   initial:   .section .ydata "drw3"
              a - don't know -- maybe a misprint
	      d - initialized data
	      r - readable
	      3 - double word aligned (that would be 4 byte boundary)
   commentary:
   Tag tables (also known as the scope table) for exception handling,
   debugging, etc.
*/
static void
ppc_ydata(ignore)
     int ignore;
{
  if (ydata_section == 0) 
    {
      ydata_section = subseg_new (".ydata", 0);
      bfd_set_section_flags (stdoutput, ydata_section,
			 (SEC_ALLOC | SEC_LOAD | SEC_RELOC
				       | SEC_READONLY | SEC_DATA ));

      bfd_set_section_alignment (stdoutput, ydata_section, 3);
    }
  else
    {
      ydata_section = subseg_new (".ydata", 0);
    }
  ppc_set_current_section(ydata_section);
}

/* pseudo-op: .reldata
   behaviour: predefined read write data section
              double word aligned (4-byte)
	      FIXME: relocation is applied to it
	      FIXME: what's the difference between this and .data?
   errors:    None
   warnings:  None
   initial:   .section .reldata "drw3"
	      d - initialized data
	      r - readable
	      w - writeable
	      3 - double word aligned (that would be 8 byte boundary)

   commentary:
   Like .data, but intended to hold data subject to relocation, such as
   function descriptors, etc.
*/
static void
ppc_reldata(ignore)
     int ignore;
{
  if (reldata_section == 0)
    {
      reldata_section = subseg_new (".reldata", 0);

      bfd_set_section_flags (stdoutput, reldata_section,
			     ( SEC_ALLOC | SEC_LOAD | SEC_RELOC 
			      | SEC_DATA ));

      bfd_set_section_alignment (stdoutput, reldata_section, 2);
    }
  else
    {
      reldata_section = subseg_new (".reldata", 0);
    }
  ppc_set_current_section(reldata_section);
}

/* pseudo-op: .rdata
   behaviour: predefined read only data section
              double word aligned
   errors:    None
   warnings:  None
   initial:   .section .rdata "dr3"
	      d - initialized data
	      r - readable
	      3 - double word aligned (that would be 4 byte boundary)
*/
static void
ppc_rdata(ignore)
     int ignore;
{
  if (rdata_section == 0)
    {
      rdata_section = subseg_new (".rdata", 0);
      bfd_set_section_flags (stdoutput, rdata_section,
			     (SEC_ALLOC | SEC_LOAD | SEC_RELOC
			      | SEC_READONLY | SEC_DATA ));

      bfd_set_section_alignment (stdoutput, rdata_section, 2);
    }
  else
    {
      rdata_section = subseg_new (".rdata", 0);
    }
  ppc_set_current_section(rdata_section);
}

/* pseudo-op: .ualong
   behaviour: much like .int, with the exception that no alignment is 
              performed.
	      FIXME: test the alignment statement
   errors:    None
   warnings:  None
*/
static void
ppc_ualong(ignore)
     int ignore;
{
  /* try for long */
  cons ( 4 );
}

/* pseudo-op: .znop  <symbol name>
   behaviour: Issue a nop instruction
              Issue a IMAGE_REL_PPC_IFGLUE relocation against it, using
	      the supplied symbol name.
   errors:    None
   warnings:  Missing symbol name
*/
static void
ppc_znop(ignore)
     int ignore;
{
  unsigned long insn;
  const struct powerpc_opcode *opcode;
  expressionS ex;
  char *f;

  symbolS *sym;

  /* Strip out the symbol name */
  char *symbol_name;
  char c;
  char *name;
  unsigned int exp;
  flagword flags;
  asection *sec;

  symbol_name = input_line_pointer;
  c = get_symbol_end ();

  name = xmalloc (input_line_pointer - symbol_name + 1);
  strcpy (name, symbol_name);

  sym = symbol_find_or_make (name);

  *input_line_pointer = c;

  SKIP_WHITESPACE ();

  /* Look up the opcode in the hash table.  */
  opcode = (const struct powerpc_opcode *) hash_find (ppc_hash, "nop");

  /* stick in the nop */
  insn = opcode->opcode;

  /* Write out the instruction.  */
  f = frag_more (4);
  md_number_to_chars (f, insn, 4);
  fix_new (frag_now,
	   f - frag_now->fr_literal,
	   4,
	   sym,
	   0,
	   0,
	   BFD_RELOC_16_GOT_PCREL);

}

/* pseudo-op: 
   behaviour: 
   errors:    
   warnings:  
*/
static void
ppc_pe_comm(lcomm)
     int lcomm;
{
  register char *name;
  register char c;
  register char *p;
  offsetT temp;
  register symbolS *symbolP;
  offsetT align;

  name = input_line_pointer;
  c = get_symbol_end ();

  /* just after name is now '\0' */
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad ("Expected comma after symbol-name: rest of line ignored.");
      ignore_rest_of_line ();
      return;
    }

  input_line_pointer++;		/* skip ',' */
  if ((temp = get_absolute_expression ()) < 0)
    {
      as_warn (".COMMon length (%ld.) <0! Ignored.", (long) temp);
      ignore_rest_of_line ();
      return;
    }

  if (! lcomm)
    {
      /* The third argument to .comm is the alignment.  */
      if (*input_line_pointer != ',')
	align = 3;
      else
	{
	  ++input_line_pointer;
	  align = get_absolute_expression ();
	  if (align <= 0)
	    {
	      as_warn ("ignoring bad alignment");
	      align = 3;
	    }
	}
    }

  *p = 0;
  symbolP = symbol_find_or_make (name);

  *p = c;
  if (S_IS_DEFINED (symbolP))
    {
      as_bad ("Ignoring attempt to re-define symbol `%s'.",
	      S_GET_NAME (symbolP));
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

  demand_empty_rest_of_line ();
}

/*
 * implement the .section pseudo op:
 *	.section name {, "flags"}
 *                ^         ^
 *                |         +--- optional flags: 'b' for bss
 *                |                              'i' for info
 *                +-- section name               'l' for lib
 *                                               'n' for noload
 *                                               'o' for over
 *                                               'w' for data
 *						 'd' (apparently m88k for data)
 *                                               'x' for text
 * But if the argument is not a quoted string, treat it as a
 * subsegment number.
 *
 * FIXME: this is a copy of the section processing from obj-coff.c, with
 * additions/changes for the moto-pas assembler support. There are three
 * categories:
 *
 * FIXME: I just noticed this. This doesn't work at all really. It it 
 *        setting bits that bfd probably neither understands or uses. The
 *        correct approach (?) will have to incorporate extra fields attached
 *        to the section to hold the system specific stuff. (krk)
 *
 * Section Contents:
 * 'a' - unknown - referred to in documentation, but no definition supplied
 * 'c' - section has code
 * 'd' - section has initialized data
 * 'u' - section has uninitialized data
 * 'i' - section contains directives (info)
 * 'n' - section can be discarded
 * 'R' - remove section at link time
 *
 * Section Protection:
 * 'r' - section is readable
 * 'w' - section is writeable
 * 'x' - section is executable
 * 's' - section is sharable
 *
 * Section Alignment:
 * '0' - align to byte boundary
 * '1' - align to halfword undary
 * '2' - align to word boundary
 * '3' - align to doubleword boundary
 * '4' - align to quadword boundary
 * '5' - align to 32 byte boundary
 * '6' - align to 64 byte boundary
 *
 */

void
ppc_pe_section (ignore)
     int ignore;
{
  /* Strip out the section name */
  char *section_name;
  char c;
  char *name;
  unsigned int exp;
  flagword flags;
  segT sec;
  int align;

  section_name = input_line_pointer;
  c = get_symbol_end ();

  name = xmalloc (input_line_pointer - section_name + 1);
  strcpy (name, section_name);

  *input_line_pointer = c;

  SKIP_WHITESPACE ();

  exp = 0;
  flags = SEC_NO_FLAGS;

  if (strcmp (name, ".idata$2") == 0)
    {
      align = 0;
    }
  else if (strcmp (name, ".idata$3") == 0)
    {
      align = 0;
    }
  else if (strcmp (name, ".idata$4") == 0)
    {
      align = 2;
    }
  else if (strcmp (name, ".idata$5") == 0)
    {
      align = 2;
    }
  else if (strcmp (name, ".idata$6") == 0)
    {
      align = 1;
    }
  else
    align = 4; /* default alignment to 16 byte boundary */

  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;
      SKIP_WHITESPACE ();
      if (*input_line_pointer != '"')
	exp = get_absolute_expression ();
      else
	{
	  ++input_line_pointer;
	  while (*input_line_pointer != '"'
		 && ! is_end_of_line[(unsigned char) *input_line_pointer])
	    {
	      switch (*input_line_pointer)
		{
		  /* Section Contents */
		case 'a': /* unknown */
		  as_warn ("Unsupported section attribute -- 'a'");
		  break;
		case 'c': /* code section */
		  flags |= SEC_CODE; 
		  break;
		case 'd': /* section has initialized data */
		  flags |= SEC_DATA;
		  break;
		case 'u': /* section has uninitialized data */
		  /* FIXME: This is IMAGE_SCN_CNT_UNINITIALIZED_DATA
		     in winnt.h */
		  flags |= SEC_ROM;
		  break;
		case 'i': /* section contains directives (info) */
		  /* FIXME: This is IMAGE_SCN_LNK_INFO
		     in winnt.h */
		  flags |= SEC_HAS_CONTENTS;
		  break;
		case 'n': /* section can be discarded */
		  flags &=~ SEC_LOAD; 
		  break;
		case 'R': /* Remove section at link time */
		  flags |= SEC_NEVER_LOAD;
		  break;

		  /* Section Protection */
		case 'r': /* section is readable */
		  flags |= IMAGE_SCN_MEM_READ;
		  break;
		case 'w': /* section is writeable */
		  flags |= IMAGE_SCN_MEM_WRITE;
		  break;
		case 'x': /* section is executable */
		  flags |= IMAGE_SCN_MEM_EXECUTE;
		  break;
		case 's': /* section is sharable */
		  flags |= IMAGE_SCN_MEM_SHARED;
		  break;

		  /* Section Alignment */
		case '0': /* align to byte boundary */
		  flags |= IMAGE_SCN_ALIGN_1BYTES;
		  align = 0;
		  break;
		case '1':  /* align to halfword boundary */
		  flags |= IMAGE_SCN_ALIGN_2BYTES;
		  align = 1;
		  break;
		case '2':  /* align to word boundary */
		  flags |= IMAGE_SCN_ALIGN_4BYTES;
		  align = 2;
		  break;
		case '3':  /* align to doubleword boundary */
		  flags |= IMAGE_SCN_ALIGN_8BYTES;
		  align = 3;
		  break;
		case '4':  /* align to quadword boundary */
		  flags |= IMAGE_SCN_ALIGN_16BYTES;
		  align = 4;
		  break;
		case '5':  /* align to 32 byte boundary */
		  flags |= IMAGE_SCN_ALIGN_32BYTES;
		  align = 5;
		  break;
		case '6':  /* align to 64 byte boundary */
		  flags |= IMAGE_SCN_ALIGN_64BYTES;
		  align = 6;
		  break;

		default:
		  as_warn("unknown section attribute '%c'",
			  *input_line_pointer);
		  break;
		}
	      ++input_line_pointer;
	    }
	  if (*input_line_pointer == '"')
	    ++input_line_pointer;
	}
    }

  sec = subseg_new (name, (subsegT) exp);

  ppc_set_current_section(sec);

  if (flags != SEC_NO_FLAGS)
    {
      if (! bfd_set_section_flags (stdoutput, sec, flags))
	as_warn ("error setting flags for \"%s\": %s",
		 bfd_section_name (stdoutput, sec),
		 bfd_errmsg (bfd_get_error ()));
    }

  bfd_set_section_alignment(stdoutput, sec, align);

}

static void
ppc_pe_function (ignore)
     int ignore;
{
  char *name;
  char endc;
  symbolS *ext_sym;

  name = input_line_pointer;
  endc = get_symbol_end ();

  ext_sym = symbol_find_or_make (name);

  *input_line_pointer = endc;

  S_SET_DATA_TYPE (ext_sym, DT_FCN << N_BTSHFT);
  SF_SET_FUNCTION (ext_sym);
  SF_SET_PROCESS (ext_sym);
  coff_add_linesym (ext_sym);

  demand_empty_rest_of_line ();
}

static void
ppc_pe_tocd (ignore)
     int ignore;
{
  if (tocdata_section == 0)
    {
      tocdata_section = subseg_new (".tocd", 0);
      /* FIXME: section flags won't work */
      bfd_set_section_flags (stdoutput, tocdata_section,
			     (SEC_ALLOC | SEC_LOAD | SEC_RELOC
			      | SEC_READONLY | SEC_DATA ));

      bfd_set_section_alignment (stdoutput, tocdata_section, 2);
    }
  else
    {
      rdata_section = subseg_new (".tocd", 0);
    }

  ppc_set_current_section(tocdata_section);

  demand_empty_rest_of_line ();
}

/* Don't adjust TOC relocs to use the section symbol.  */

int
ppc_pe_fix_adjustable (fix)
     fixS *fix;
{
  return fix->fx_r_type != BFD_RELOC_PPC_TOC16;
}

#endif

#ifdef OBJ_XCOFF

/* XCOFF specific symbol and file handling.  */

/* Canonicalize the symbol name.  We use the to force the suffix, if
   any, to use square brackets, and to be in upper case.  */

char *
ppc_canonicalize_symbol_name (name)
     char *name;
{
  char *s;

  if (ppc_stab_symbol)
    return name;

  for (s = name; *s != '\0' && *s != '{' && *s != '['; s++)
    ;
  if (*s != '\0')
    {
      char brac;

      if (*s == '[')
	brac = ']';
      else
	{
	  *s = '[';
	  brac = '}';
	}

      for (s++; *s != '\0' && *s != brac; s++)
	if (islower (*s))
	  *s = toupper (*s);

      if (*s == '\0' || s[1] != '\0')
	as_bad ("bad symbol suffix");

      *s = ']';
    }

  return name;
}

/* Set the class of a symbol based on the suffix, if any.  This is
   called whenever a new symbol is created.  */

void
ppc_symbol_new_hook (sym)
     symbolS *sym;
{
  const char *s;

  sym->sy_tc.next = NULL;
  sym->sy_tc.output = 0;
  sym->sy_tc.class = -1;
  sym->sy_tc.real_name = NULL;
  sym->sy_tc.subseg = 0;
  sym->sy_tc.align = 0;
  sym->sy_tc.size = NULL;
  sym->sy_tc.within = NULL;

  if (ppc_stab_symbol)
    return;

  s = strchr (S_GET_NAME (sym), '[');
  if (s == (const char *) NULL)
    {
      /* There is no suffix.  */
      return;
    }

  ++s;

  switch (s[0])
    {
    case 'B':
      if (strcmp (s, "BS]") == 0)
	sym->sy_tc.class = XMC_BS;
      break;
    case 'D':
      if (strcmp (s, "DB]") == 0)
	sym->sy_tc.class = XMC_DB;
      else if (strcmp (s, "DS]") == 0)
	sym->sy_tc.class = XMC_DS;
      break;
    case 'G':
      if (strcmp (s, "GL]") == 0)
	sym->sy_tc.class = XMC_GL;
      break;
    case 'P':
      if (strcmp (s, "PR]") == 0)
	sym->sy_tc.class = XMC_PR;
      break;
    case 'R':
      if (strcmp (s, "RO]") == 0)
	sym->sy_tc.class = XMC_RO;
      else if (strcmp (s, "RW]") == 0)
	sym->sy_tc.class = XMC_RW;
      break;
    case 'S':
      if (strcmp (s, "SV]") == 0)
	sym->sy_tc.class = XMC_SV;
      break;
    case 'T':
      if (strcmp (s, "TC]") == 0)
	sym->sy_tc.class = XMC_TC;
      else if (strcmp (s, "TI]") == 0)
	sym->sy_tc.class = XMC_TI;
      else if (strcmp (s, "TB]") == 0)
	sym->sy_tc.class = XMC_TB;
      else if (strcmp (s, "TC0]") == 0 || strcmp (s, "T0]") == 0)
	sym->sy_tc.class = XMC_TC0;
      break;
    case 'U':
      if (strcmp (s, "UA]") == 0)
	sym->sy_tc.class = XMC_UA;
      else if (strcmp (s, "UC]") == 0)
	sym->sy_tc.class = XMC_UC;
      break;
    case 'X':
      if (strcmp (s, "XO]") == 0)
	sym->sy_tc.class = XMC_XO;
      break;
    }

  if (sym->sy_tc.class == -1)
    as_bad ("Unrecognized symbol suffix");
}

/* Set the class of a label based on where it is defined.  This
   handles symbols without suffixes.  Also, move the symbol so that it
   follows the csect symbol.  */

void
ppc_frob_label (sym)
     symbolS *sym;
{
  if (ppc_current_csect != (symbolS *) NULL)
    {
      if (sym->sy_tc.class == -1)
	sym->sy_tc.class = ppc_current_csect->sy_tc.class;

      symbol_remove (sym, &symbol_rootP, &symbol_lastP);
      symbol_append (sym, ppc_current_csect->sy_tc.within, &symbol_rootP,
		     &symbol_lastP);
      ppc_current_csect->sy_tc.within = sym;
    }
}

/* This variable is set by ppc_frob_symbol if any absolute symbols are
   seen.  It tells ppc_adjust_symtab whether it needs to look through
   the symbols.  */

static boolean ppc_saw_abs;

/* Change the name of a symbol just before writing it out.  Set the
   real name if the .rename pseudo-op was used.  Otherwise, remove any
   class suffix.  Return 1 if the symbol should not be included in the
   symbol table.  */

int
ppc_frob_symbol (sym)
     symbolS *sym;
{
  static symbolS *ppc_last_function;
  static symbolS *set_end;

  /* Discard symbols that should not be included in the output symbol
     table.  */
  if (! sym->sy_used_in_reloc
      && ((sym->bsym->flags & BSF_SECTION_SYM) != 0
	  || (! S_IS_EXTERNAL (sym)
	      && ! sym->sy_tc.output
	      && S_GET_STORAGE_CLASS (sym) != C_FILE)))
    return 1;

  if (sym->sy_tc.real_name != (char *) NULL)
    S_SET_NAME (sym, sym->sy_tc.real_name);
  else
    {
      const char *name;
      const char *s;

      name = S_GET_NAME (sym);
      s = strchr (name, '[');
      if (s != (char *) NULL)
	{
	  unsigned int len;
	  char *snew;

	  len = s - name;
	  snew = xmalloc (len + 1);
	  memcpy (snew, name, len);
	  snew[len] = '\0';

	  S_SET_NAME (sym, snew);
	}
    }

  if (set_end != (symbolS *) NULL)
    {
      SA_SET_SYM_ENDNDX (set_end, sym);
      set_end = NULL;
    }

  if (SF_GET_FUNCTION (sym))
    {
      if (ppc_last_function != (symbolS *) NULL)
	as_warn ("two .function pseudo-ops with no intervening .ef");
      ppc_last_function = sym;
      if (sym->sy_tc.size != (symbolS *) NULL)
	{
	  resolve_symbol_value (sym->sy_tc.size);
	  SA_SET_SYM_FSIZE (sym, (long) S_GET_VALUE (sym->sy_tc.size));
	}
    }
  else if (S_GET_STORAGE_CLASS (sym) == C_FCN
	   && strcmp (S_GET_NAME (sym), ".ef") == 0)
    {
      if (ppc_last_function == (symbolS *) NULL)
	as_warn (".ef with no preceding .function");
      else
	{
	  set_end = ppc_last_function;
	  ppc_last_function = NULL;

	  /* We don't have a C_EFCN symbol, but we need to force the
	     COFF backend to believe that it has seen one.  */
	  coff_last_function = NULL;
	}
    }

  if (! S_IS_EXTERNAL (sym)
      && (sym->bsym->flags & BSF_SECTION_SYM) == 0
      && S_GET_STORAGE_CLASS (sym) != C_FILE
      && S_GET_STORAGE_CLASS (sym) != C_FCN
      && S_GET_STORAGE_CLASS (sym) != C_BLOCK
      && S_GET_STORAGE_CLASS (sym) != C_BSTAT
      && S_GET_STORAGE_CLASS (sym) != C_ESTAT
      && S_GET_STORAGE_CLASS (sym) != C_BINCL
      && S_GET_STORAGE_CLASS (sym) != C_EINCL
      && S_GET_SEGMENT (sym) != ppc_coff_debug_section)
    S_SET_STORAGE_CLASS (sym, C_HIDEXT);

  if (S_GET_STORAGE_CLASS (sym) == C_EXT
      || S_GET_STORAGE_CLASS (sym) == C_HIDEXT)
    {
      int i;
      union internal_auxent *a;

      /* Create a csect aux.  */
      i = S_GET_NUMBER_AUXILIARY (sym);
      S_SET_NUMBER_AUXILIARY (sym, i + 1);
      a = &coffsymbol (sym->bsym)->native[i + 1].u.auxent;
      if (sym->sy_tc.class == XMC_TC0)
	{
	  /* This is the TOC table.  */
	  know (strcmp (S_GET_NAME (sym), "TOC") == 0);
	  a->x_csect.x_scnlen.l = 0;
	  a->x_csect.x_smtyp = (2 << 3) | XTY_SD;
	}
      else if (sym->sy_tc.subseg != 0)
	{
	  /* This is a csect symbol.  x_scnlen is the size of the
	     csect.  */
	  if (sym->sy_tc.next == (symbolS *) NULL)
	    a->x_csect.x_scnlen.l = (bfd_section_size (stdoutput,
						       S_GET_SEGMENT (sym))
				     - S_GET_VALUE (sym));
	  else
	    {
	      resolve_symbol_value (sym->sy_tc.next);
	      a->x_csect.x_scnlen.l = (S_GET_VALUE (sym->sy_tc.next)
				       - S_GET_VALUE (sym));
	    }
	  a->x_csect.x_smtyp = (sym->sy_tc.align << 3) | XTY_SD;
	}
      else if (S_GET_SEGMENT (sym) == bss_section)
	{
	  /* This is a common symbol.  */
	  a->x_csect.x_scnlen.l = sym->sy_frag->fr_offset;
	  a->x_csect.x_smtyp = (sym->sy_tc.align << 3) | XTY_CM;
	  if (S_IS_EXTERNAL (sym))
	    sym->sy_tc.class = XMC_RW;
	  else
	    sym->sy_tc.class = XMC_BS;
	}
      else if (S_GET_SEGMENT (sym) == absolute_section)
	{
	  /* This is an absolute symbol.  The csect will be created by
             ppc_adjust_symtab.  */
	  ppc_saw_abs = true;
	  a->x_csect.x_smtyp = XTY_LD;
	  if (sym->sy_tc.class == -1)
	    sym->sy_tc.class = XMC_XO;
	}
      else if (! S_IS_DEFINED (sym))
	{
	  /* This is an external symbol.  */
	  a->x_csect.x_scnlen.l = 0;
	  a->x_csect.x_smtyp = XTY_ER;
	}
      else if (sym->sy_tc.class == XMC_TC)
	{
	  symbolS *next;

	  /* This is a TOC definition.  x_scnlen is the size of the
	     TOC entry.  */
	  next = symbol_next (sym);
	  while (next->sy_tc.class == XMC_TC0)
	    next = symbol_next (next);
	  if (next == (symbolS *) NULL
	      || next->sy_tc.class != XMC_TC)
	    {
	      if (ppc_after_toc_frag == (fragS *) NULL)
		a->x_csect.x_scnlen.l = (bfd_section_size (stdoutput,
							   data_section)
					 - S_GET_VALUE (sym));
	      else
		a->x_csect.x_scnlen.l = (ppc_after_toc_frag->fr_address
					 - S_GET_VALUE (sym));
	    }
	  else
	    {
	      resolve_symbol_value (next);
	      a->x_csect.x_scnlen.l = (S_GET_VALUE (next)
				       - S_GET_VALUE (sym));
	    }
	  a->x_csect.x_smtyp = (2 << 3) | XTY_SD;
	}
      else
	{
	  symbolS *csect;

	  /* This is a normal symbol definition.  x_scnlen is the
	     symbol index of the containing csect.  */
	  if (S_GET_SEGMENT (sym) == text_section)
	    csect = ppc_text_csects;
	  else if (S_GET_SEGMENT (sym) == data_section)
	    csect = ppc_data_csects;
	  else
	    abort ();

	  /* Skip the initial dummy symbol.  */
	  csect = csect->sy_tc.next;

	  if (csect == (symbolS *) NULL)
	    {
	      as_warn ("warning: symbol %s has no csect", S_GET_NAME (sym));
	      a->x_csect.x_scnlen.l = 0;
	    }
	  else
	    {
	      while (csect->sy_tc.next != (symbolS *) NULL)
		{
		  resolve_symbol_value (csect->sy_tc.next);
		  if (S_GET_VALUE (csect->sy_tc.next) > S_GET_VALUE (sym))
		    break;
		  csect = csect->sy_tc.next;
		}

	      a->x_csect.x_scnlen.p = coffsymbol (csect->bsym)->native;
	      coffsymbol (sym->bsym)->native[i + 1].fix_scnlen = 1;
	    }
	  a->x_csect.x_smtyp = XTY_LD;
	}
	
      a->x_csect.x_parmhash = 0;
      a->x_csect.x_snhash = 0;
      if (sym->sy_tc.class == -1)
	a->x_csect.x_smclas = XMC_PR;
      else
	a->x_csect.x_smclas = sym->sy_tc.class;
      a->x_csect.x_stab = 0;
      a->x_csect.x_snstab = 0;

      /* Don't let the COFF backend resort these symbols.  */
      sym->bsym->flags |= BSF_NOT_AT_END;
    }
  else if (S_GET_STORAGE_CLASS (sym) == C_BSTAT)
    {
      /* We want the value to be the symbol index of the referenced
	 csect symbol.  BFD will do that for us if we set the right
	 flags.  */
      S_SET_VALUE (sym,
		   (valueT) coffsymbol (sym->sy_tc.within->bsym)->native);
      coffsymbol (sym->bsym)->native->fix_value = 1;
    }
  else if (S_GET_STORAGE_CLASS (sym) == C_STSYM)
    {
      symbolS *block;
      symbolS *csect;

      /* The value is the offset from the enclosing csect.  */
      block = sym->sy_tc.within;
      csect = block->sy_tc.within;
      resolve_symbol_value (csect);
      S_SET_VALUE (sym, S_GET_VALUE (sym) - S_GET_VALUE (csect));
    }
  else if (S_GET_STORAGE_CLASS (sym) == C_BINCL
	   || S_GET_STORAGE_CLASS (sym) == C_EINCL)
    {
      /* We want the value to be a file offset into the line numbers.
         BFD will do that for us if we set the right flags.  We have
         already set the value correctly.  */
      coffsymbol (sym->bsym)->native->fix_line = 1;
    }

  return 0;
}

/* Adjust the symbol table.  This creates csect symbols for all
   absolute symbols.  */

void
ppc_adjust_symtab ()
{
  symbolS *sym;

  if (! ppc_saw_abs)
    return;

  for (sym = symbol_rootP; sym != NULL; sym = symbol_next (sym))
    {
      symbolS *csect;
      int i;
      union internal_auxent *a;

      if (S_GET_SEGMENT (sym) != absolute_section)
	continue;

      csect = symbol_create (".abs[XO]", absolute_section,
			     S_GET_VALUE (sym), &zero_address_frag);
      csect->bsym->value = S_GET_VALUE (sym);
      S_SET_STORAGE_CLASS (csect, C_HIDEXT);
      i = S_GET_NUMBER_AUXILIARY (csect);
      S_SET_NUMBER_AUXILIARY (csect, i + 1);
      a = &coffsymbol (csect->bsym)->native[i + 1].u.auxent;
      a->x_csect.x_scnlen.l = 0;
      a->x_csect.x_smtyp = XTY_SD;
      a->x_csect.x_parmhash = 0;
      a->x_csect.x_snhash = 0;
      a->x_csect.x_smclas = XMC_XO;
      a->x_csect.x_stab = 0;
      a->x_csect.x_snstab = 0;

      symbol_insert (csect, sym, &symbol_rootP, &symbol_lastP);

      i = S_GET_NUMBER_AUXILIARY (sym);
      a = &coffsymbol (sym->bsym)->native[i].u.auxent;
      a->x_csect.x_scnlen.p = coffsymbol (csect->bsym)->native;
      coffsymbol (sym->bsym)->native[i].fix_scnlen = 1;
    }

  ppc_saw_abs = false;
}

/* Set the VMA for a section.  This is called on all the sections in
   turn.  */

void
ppc_frob_section (sec)
     asection *sec;
{
  static bfd_size_type vma = 0;

  bfd_set_section_vma (stdoutput, sec, vma);
  vma += bfd_section_size (stdoutput, sec);
}

#endif /* OBJ_XCOFF */

/* Turn a string in input_line_pointer into a floating point constant
   of type type, and store the appropriate bytes in *litp.  The number
   of LITTLENUMS emitted is stored in *sizep .  An error message is
   returned, or NULL on OK.  */

char *
md_atof (type, litp, sizep)
     int type;
     char *litp;
     int *sizep;
{
  int prec;
  LITTLENUM_TYPE words[4];
  char *t;
  int i;

  switch (type)
    {
    case 'f':
      prec = 2;
      break;

    case 'd':
      prec = 4;
      break;

    default:
      *sizep = 0;
      return "bad call to md_atof";
    }

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;

  *sizep = prec * 2;

  if (target_big_endian)
    {
      for (i = 0; i < prec; i++)
	{
	  md_number_to_chars (litp, (valueT) words[i], 2);
	  litp += 2;
	}
    }
  else
    {
      for (i = prec - 1; i >= 0; i--)
	{
	  md_number_to_chars (litp, (valueT) words[i], 2);
	  litp += 2;
	}
    }
     
  return NULL;
}

/* Write a value out to the object file, using the appropriate
   endianness.  */

void
md_number_to_chars (buf, val, n)
     char *buf;
     valueT val;
     int n;
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

/* Align a section (I don't know why this is machine dependent).  */

valueT
md_section_align (seg, addr)
     asection *seg;
     valueT addr;
{
  int align = bfd_get_section_alignment (stdoutput, seg);

  return ((addr + (1 << align) - 1) & (-1 << align));
}

/* We don't have any form of relaxing.  */

int
md_estimate_size_before_relax (fragp, seg)
     fragS *fragp;
     asection *seg;
{
  abort ();
  return 0;
}

/* Convert a machine dependent frag.  We never generate these.  */

void
md_convert_frag (abfd, sec, fragp)
     bfd *abfd;
     asection *sec;
     fragS *fragp;
{
  abort ();
}

/* We have no need to default values of symbols.  */

/*ARGSUSED*/
symbolS *
md_undefined_symbol (name)
     char *name;
{
  return 0;
}

/* Functions concerning relocs.  */

/* The location from which a PC relative jump should be calculated,
   given a PC relative reloc.  */

long
md_pcrel_from_section (fixp, sec)
     fixS *fixp;
     segT sec;
{
#ifdef OBJ_ELF
  if (fixp->fx_addsy != (symbolS *) NULL
      && (! S_IS_DEFINED (fixp->fx_addsy)
	  || TC_FORCE_RELOCATION_SECTION (fixp, sec)))
    return 0;
#endif

  return fixp->fx_frag->fr_address + fixp->fx_where;
}

#ifdef OBJ_XCOFF

/* This is called to see whether a fixup should be adjusted to use a
   section symbol.  We take the opportunity to change a fixup against
   a symbol in the TOC subsegment into a reloc against the
   corresponding .tc symbol.  */

int
ppc_fix_adjustable (fix)
     fixS *fix;
{
  valueT val;

  resolve_symbol_value (fix->fx_addsy);
  val = S_GET_VALUE (fix->fx_addsy);
  if (ppc_toc_csect != (symbolS *) NULL
      && fix->fx_addsy != (symbolS *) NULL
      && fix->fx_addsy != ppc_toc_csect
      && S_GET_SEGMENT (fix->fx_addsy) == data_section
      && val >= ppc_toc_frag->fr_address
      && (ppc_after_toc_frag == (fragS *) NULL
	  || val < ppc_after_toc_frag->fr_address))
    {
      symbolS *sy;

      for (sy = symbol_next (ppc_toc_csect);
	   sy != (symbolS *) NULL;
	   sy = symbol_next (sy))
	{
	  if (sy->sy_tc.class == XMC_TC0)
	    continue;
	  if (sy->sy_tc.class != XMC_TC)
	    break;
	  resolve_symbol_value (sy);
	  if (val == S_GET_VALUE (sy))
	    {
	      fix->fx_addsy = sy;
	      fix->fx_addnumber = val - ppc_toc_frag->fr_address;
	      return 0;
	    }
	}

      as_bad_where (fix->fx_file, fix->fx_line,
		    "symbol in .toc does not match any .tc");
    }

  /* Possibly adjust the reloc to be against the csect.  */
  if (fix->fx_addsy != (symbolS *) NULL
      && fix->fx_addsy->sy_tc.subseg == 0
      && fix->fx_addsy->sy_tc.class != XMC_TC0
      && fix->fx_addsy->sy_tc.class != XMC_TC
      && S_GET_SEGMENT (fix->fx_addsy) != bss_section)
    {
      symbolS *csect;

      if (S_GET_SEGMENT (fix->fx_addsy) == text_section)
	csect = ppc_text_csects;
      else if (S_GET_SEGMENT (fix->fx_addsy) == data_section)
	csect = ppc_data_csects;
      else
	abort ();

      /* Skip the initial dummy symbol.  */
      csect = csect->sy_tc.next;

      if (csect != (symbolS *) NULL)
	{
	  while (csect->sy_tc.next != (symbolS *) NULL
		 && (csect->sy_tc.next->sy_frag->fr_address
		     <= fix->fx_addsy->sy_frag->fr_address))
	    csect = csect->sy_tc.next;

	  fix->fx_offset += (S_GET_VALUE (fix->fx_addsy)
			     - csect->sy_frag->fr_address);
	  fix->fx_addsy = csect;
	}
    }

  /* Adjust a reloc against a .lcomm symbol to be against the base
     .lcomm.  */
  if (fix->fx_addsy != (symbolS *) NULL
      && S_GET_SEGMENT (fix->fx_addsy) == bss_section
      && ! S_IS_EXTERNAL (fix->fx_addsy))
    {
      resolve_symbol_value (fix->fx_addsy->sy_frag->fr_symbol);
      fix->fx_offset += (S_GET_VALUE (fix->fx_addsy)
			 - S_GET_VALUE (fix->fx_addsy->sy_frag->fr_symbol));
      fix->fx_addsy = fix->fx_addsy->sy_frag->fr_symbol;
    }

  return 0;
}

#endif

/* See whether a symbol is in the TOC section.  */

static int
ppc_is_toc_sym (sym)
     symbolS *sym;
{
#ifdef OBJ_XCOFF
  return sym->sy_tc.class == XMC_TC;
#else
  return strcmp (segment_name (S_GET_SEGMENT (sym)), ".got") == 0;
#endif
}

/* Apply a fixup to the object code.  This is called for all the
   fixups we generated by the call to fix_new_exp, above.  In the call
   above we used a reloc code which was the largest legal reloc code
   plus the operand index.  Here we undo that to recover the operand
   index.  At this point all symbol values should be fully resolved,
   and we attempt to completely resolve the reloc.  If we can not do
   that, we determine the correct reloc code and put it back in the
   fixup.  */

int
md_apply_fix3 (fixp, valuep, seg)
     fixS *fixp;
     valueT *valuep;
     segT seg;
{
  valueT value;

  /* FIXME FIXME FIXME: The value we are passed in *valuep includes
     the symbol values.  Since we are using BFD_ASSEMBLER, if we are
     doing this relocation the code in write.c is going to call
     bfd_perform_relocation, which is also going to use the symbol
     value.  That means that if the reloc is fully resolved we want to
     use *valuep since bfd_perform_relocation is not being used.
     However, if the reloc is not fully resolved we do not want to use
     *valuep, and must use fx_offset instead.  However, if the reloc
     is PC relative, we do want to use *valuep since it includes the
     result of md_pcrel_from.  This is confusing.  */

  if (fixp->fx_addsy == (symbolS *) NULL)
    {
      value = *valuep;
      fixp->fx_done = 1;
    }
  else if (fixp->fx_pcrel)
    value = *valuep;
  else
    {
      value = fixp->fx_offset;
      if (fixp->fx_subsy != (symbolS *) NULL)
	{
	  if (S_GET_SEGMENT (fixp->fx_subsy) == absolute_section)
	    value -= S_GET_VALUE (fixp->fx_subsy);
	  else
	    {
	      /* We can't actually support subtracting a symbol.  */
	      as_bad_where (fixp->fx_file, fixp->fx_line,
			    "expression too complex");
	    }
	}
    }

  if ((int) fixp->fx_r_type >= (int) BFD_RELOC_UNUSED)
    {
      int opindex;
      const struct powerpc_operand *operand;
      char *where;
      unsigned long insn;

      opindex = (int) fixp->fx_r_type - (int) BFD_RELOC_UNUSED;

      operand = &powerpc_operands[opindex];

#ifdef OBJ_XCOFF
      /* It appears that an instruction like
	     l 9,LC..1(30)
	 when LC..1 is not a TOC symbol does not generate a reloc.  It
	 uses the offset of LC..1 within its csect.  However, .long
	 LC..1 will generate a reloc.  I can't find any documentation
	 on how these cases are to be distinguished, so this is a wild
	 guess.  These cases are generated by gcc -mminimal-toc.  */
      if ((operand->flags & PPC_OPERAND_PARENS) != 0
	  && operand->bits == 16
	  && operand->shift == 0
	  && operand->insert == NULL
	  && fixp->fx_addsy != NULL
	  && fixp->fx_addsy->sy_tc.subseg != 0
	  && fixp->fx_addsy->sy_tc.class != XMC_TC
	  && fixp->fx_addsy->sy_tc.class != XMC_TC0
	  && S_GET_SEGMENT (fixp->fx_addsy) != bss_section)
	{
	  value = fixp->fx_offset;
	  fixp->fx_done = 1;
	}
#endif

      /* Fetch the instruction, insert the fully resolved operand
	 value, and stuff the instruction back again.  */
      where = fixp->fx_frag->fr_literal + fixp->fx_where;
      if (target_big_endian)
	insn = bfd_getb32 ((unsigned char *) where);
      else
	insn = bfd_getl32 ((unsigned char *) where);
      insn = ppc_insert_operand (insn, operand, (offsetT) value,
				 fixp->fx_file, fixp->fx_line);
      if (target_big_endian)
	bfd_putb32 ((bfd_vma) insn, (unsigned char *) where);
      else
	bfd_putl32 ((bfd_vma) insn, (unsigned char *) where);

      if (fixp->fx_done)
	{
	  /* Nothing else to do here.  */
	  return 1;
	}

      /* Determine a BFD reloc value based on the operand information.
	 We are only prepared to turn a few of the operands into
	 relocs.
	 FIXME: We need to handle the DS field at the very least.
	 FIXME: Selecting the reloc type is a bit haphazard; perhaps
	 there should be a new field in the operand table.  */
      if ((operand->flags & PPC_OPERAND_RELATIVE) != 0
	  && operand->bits == 26
	  && operand->shift == 0)
	fixp->fx_r_type = BFD_RELOC_PPC_B26;
      else if ((operand->flags & PPC_OPERAND_RELATIVE) != 0
	  && operand->bits == 16
	  && operand->shift == 0)
	fixp->fx_r_type = BFD_RELOC_PPC_B16;
      else if ((operand->flags & PPC_OPERAND_ABSOLUTE) != 0
	       && operand->bits == 26
	       && operand->shift == 0)
	fixp->fx_r_type = BFD_RELOC_PPC_BA26;
      else if ((operand->flags & PPC_OPERAND_ABSOLUTE) != 0
	       && operand->bits == 16
	       && operand->shift == 0)
	fixp->fx_r_type = BFD_RELOC_PPC_BA16;
      else if ((operand->flags & PPC_OPERAND_PARENS) != 0
	       && operand->bits == 16
	       && operand->shift == 0
	       && operand->insert == NULL
	       && fixp->fx_addsy != NULL
	       && ppc_is_toc_sym (fixp->fx_addsy))
	{
	  fixp->fx_size = 2;
	  if (target_big_endian)
	    fixp->fx_where += 2;
	  fixp->fx_r_type = BFD_RELOC_PPC_TOC16;
	}
      else
	{
	  as_bad_where (fixp->fx_file, fixp->fx_line,
			"unresolved expression that must be resolved");
	  fixp->fx_done = 1;
	  return 1;
	}
    }
  else
    {
#ifdef OBJ_ELF
      ppc_elf_validate_fix (fixp, seg);
#endif
      switch (fixp->fx_r_type)
	{
	case BFD_RELOC_32:
	case BFD_RELOC_CTOR:
	  if (fixp->fx_pcrel)
	    fixp->fx_r_type = BFD_RELOC_32_PCREL;
					/* fall through */

	case BFD_RELOC_RVA:
	case BFD_RELOC_32_PCREL:
	case BFD_RELOC_32_BASEREL:
	case BFD_RELOC_PPC_EMB_NADDR32:
	  md_number_to_chars (fixp->fx_frag->fr_literal + fixp->fx_where,
			      value, 4);
	  break;

	case BFD_RELOC_LO16:
	case BFD_RELOC_HI16:
	case BFD_RELOC_HI16_S:
	case BFD_RELOC_16:
	case BFD_RELOC_GPREL16:
	case BFD_RELOC_16_GOT_PCREL:
	case BFD_RELOC_16_GOTOFF:
	case BFD_RELOC_LO16_GOTOFF:
	case BFD_RELOC_HI16_GOTOFF:
	case BFD_RELOC_HI16_S_GOTOFF:
	case BFD_RELOC_LO16_BASEREL:
	case BFD_RELOC_HI16_BASEREL:
	case BFD_RELOC_HI16_S_BASEREL:
	case BFD_RELOC_PPC_EMB_NADDR16:
	case BFD_RELOC_PPC_EMB_NADDR16_LO:
	case BFD_RELOC_PPC_EMB_NADDR16_HI:
	case BFD_RELOC_PPC_EMB_NADDR16_HA:
	case BFD_RELOC_PPC_EMB_SDAI16:
	case BFD_RELOC_PPC_EMB_SDA2REL:
	case BFD_RELOC_PPC_EMB_SDA2I16:
	case BFD_RELOC_PPC_EMB_RELSEC16:
	case BFD_RELOC_PPC_EMB_RELST_LO:
	case BFD_RELOC_PPC_EMB_RELST_HI:
	case BFD_RELOC_PPC_EMB_RELST_HA:
	case BFD_RELOC_PPC_EMB_RELSDA:
	case BFD_RELOC_PPC_TOC16:
	  if (fixp->fx_pcrel)
	    as_bad_where (fixp->fx_file, fixp->fx_line,
			  "cannot emit PC relative %s relocation%s%s",
			  bfd_get_reloc_code_name (fixp->fx_r_type),
			  fixp->fx_addsy != NULL ? " against " : "",
			  (fixp->fx_addsy != NULL
			   ? S_GET_NAME (fixp->fx_addsy)
			   : ""));

	  md_number_to_chars (fixp->fx_frag->fr_literal + fixp->fx_where,
			      value, 2);
	  break;

	  /* Because SDA21 modifies the register field, the size is set to 4
	     bytes, rather than 2, so offset it here appropriately */
	case BFD_RELOC_PPC_EMB_SDA21:
	  if (fixp->fx_pcrel)
	    abort ();

	  md_number_to_chars (fixp->fx_frag->fr_literal + fixp->fx_where
			      + ((target_big_endian) ? 2 : 0),
			      value, 2);
	  break;

	case BFD_RELOC_8:
	  if (fixp->fx_pcrel)
	    abort ();

	  md_number_to_chars (fixp->fx_frag->fr_literal + fixp->fx_where,
			      value, 1);
	  break;

	default:
	  fprintf(stderr,
		  "Gas failure, reloc value %d\n", fixp->fx_r_type);
	  fflush(stderr);
	  abort ();
	}
    }

#ifdef OBJ_ELF
  fixp->fx_addnumber = value;
#else
  if (fixp->fx_r_type != BFD_RELOC_PPC_TOC16)
    fixp->fx_addnumber = 0;
  else
    {
#ifdef TE_PE
      fixp->fx_addnumber = 0;
#else
      /* We want to use the offset within the data segment of the
	 symbol, not the actual VMA of the symbol.  */
      fixp->fx_addnumber =
	- bfd_get_section_vma (stdoutput, S_GET_SEGMENT (fixp->fx_addsy));
#endif
    }
#endif

  return 1;
}

/* Generate a reloc for a fixup.  */

arelent *
tc_gen_reloc (seg, fixp)
     asection *seg;
     fixS *fixp;
{
  arelent *reloc;

  reloc = (arelent *) bfd_alloc_by_size_t (stdoutput, sizeof (arelent));

  reloc->sym_ptr_ptr = &fixp->fx_addsy->bsym;
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == (reloc_howto_type *) NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    "reloc %d not supported by object file format", (int)fixp->fx_r_type);
      return NULL;
    }
  reloc->addend = fixp->fx_addnumber;

  return reloc;
}
