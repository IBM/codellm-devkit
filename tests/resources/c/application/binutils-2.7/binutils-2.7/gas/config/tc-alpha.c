/* tc-alpha.c - Processor-specific code for the DEC Alpha AXP CPU.
   Copyright (C) 1989, 93, 94, 95, 1996 Free Software Foundation, Inc.
   Contributed by Carnegie Mellon University, 1993.
   Written by Alessandro Forin, based on earlier gas-1.38 target CPU files.
   Modified by Ken Raeburn for gas-2.x and ECOFF support.
   Modified by Richard Henderson for ELF support.

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
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#include "as.h"
#include "subsegs.h"

#include "opcode/alpha.h"

#ifdef OBJ_ELF
#include "elf/alpha.h"
#endif

#include <ctype.h>


/* Local types */

#define MAX_INSN_FIXUPS 2
#define MAX_INSN_ARGS 5

struct alpha_fixup
{
  expressionS exp;
  bfd_reloc_code_real_type reloc;
};

struct alpha_insn
{
  unsigned insn;
  int nfixups;
  struct alpha_fixup fixups[MAX_INSN_FIXUPS];
};

enum alpha_macro_arg
{
  MACRO_EOA = 1, MACRO_IR, MACRO_PIR, MACRO_CPIR, MACRO_FPR, MACRO_EXP
};

struct alpha_macro
{
  const char *name;
  void (*emit) PARAMS((const expressionS *, int, void *));
  void *arg;
  enum alpha_macro_arg argsets[16];
};

/* Two extra symbols we want to see in our input.  This is a blatent
   misuse of the expressionS.X_op field.  */

#define O_pregister	(O_max+1)	/* O_register, but in parentheses */
#define O_cpregister	(O_pregister+1)	/* + a leading comma */

/* Macros for extracting the type and number of encoded register tokens */

#define is_ir_num(x)		(((x) & 32) == 0)
#define is_fpr_num(x)		(((x) & 32) != 0)
#define regno(x)		((x) & 31)

/* Something odd inherited from the old assembler */

#define note_gpreg(R)		(alpha_gprmask |= (1 << (R)))
#define note_fpreg(R)		(alpha_fprmask |= (1 << (R)))

/* Predicates for 16- and 32-bit ranges */

#define range_signed_16(x)	((offsetT)(x) >= -(offsetT)0x8000 &&	\
				 (offsetT)(x) <=  (offsetT)0x7FFF)
#define range_signed_32(x)	((offsetT)(x) >= -(offsetT)0x80000000 && \
				 (offsetT)(x) <=  (offsetT)0x7FFFFFFF)

/* Macros for sign extending from 16- and 32-bits.  */
/* XXX: The cast macros will work on all the systems that I care about,
   but really a predicate should be found to use the non-cast forms.  */

#if 1
#define sign_extend_16(x)	((short)(x))
#define sign_extend_32(x)	((int)(x))
#else
#define sign_extend_16(x)	((offsetT)(((x) & 0xFFFF) ^ 0x8000) - 0x8000)
#define sign_extend_32(x)	((offsetT)(((x) & 0xFFFFFFFF) \
					   ^ 0x80000000) - 0x80000000)
#endif

/* Macros to build tokens */

#define set_tok_reg(t, r)	(memset(&(t), 0, sizeof(t)),		\
				 (t).X_op = O_register,			\
				 (t).X_add_number = (r))
#define set_tok_preg(t, r)	(memset(&(t), 0, sizeof(t)),		\
				 (t).X_op = O_pregister,		\
				 (t).X_add_number = (r))
#define set_tok_cpreg(t, r)	(memset(&(t), 0, sizeof(t)),		\
				 (t).X_op = O_cpregister,		\
				 (t).X_add_number = (r))
#define set_tok_freg(t, r)	(memset(&(t), 0, sizeof(t)),		\
				 (t).X_op = O_register,			\
				 (t).X_add_number = (r)+32)
#define set_tok_sym(t, s, a)	(memset(&(t), 0, sizeof(t)),		\
				 (t).X_op = O_symbol,			\
				 (t).X_add_symbol = (s),		\
				 (t).X_add_number = (a))
#define set_tok_const(t, n)	(memset(&(t), 0, sizeof(t)),		\
				 (t).X_op = O_constant,			\
				 (t).X_add_number = (n))


/* Prototypes for all local functions */

static int tokenize_arguments PARAMS((char *, expressionS*, int));
static const struct alpha_opcode *find_opcode_match
	PARAMS((const struct alpha_opcode*, const expressionS*, int*, int*));
static const struct alpha_macro *find_macro_match
	PARAMS((const struct alpha_macro*, const expressionS*, int*));
static unsigned insert_operand PARAMS((unsigned, const struct alpha_operand*,
				      offsetT, char *, unsigned));
static void assemble_insn PARAMS((const struct alpha_opcode*,
				  const expressionS*, int,
				  struct alpha_insn*));
static void emit_insn PARAMS((struct alpha_insn *));
static void assemble_tokens_to_insn PARAMS((const char *, const expressionS*,
					    int, struct alpha_insn *));
static void assemble_tokens PARAMS((const char *, const expressionS*,
				    int, int));

static int load_expression PARAMS((int, const expressionS*, int *,
				   expressionS*));

static void emit_ldgp PARAMS((const expressionS*, int, void*));
static void emit_division PARAMS((const expressionS*, int, void*));
static void emit_lda PARAMS((const expressionS*, int, void*));
static void emit_ir_load PARAMS((const expressionS*, int, void*));
static void emit_loadstore PARAMS((const expressionS*, int, void*));
static void emit_jsrjmp PARAMS((const expressionS*, int, void*));

static void s_alpha_text PARAMS((int));
static void s_alpha_data PARAMS((int));
#ifndef OBJ_ELF
static void s_alpha_comm PARAMS((int));
#endif
static void s_alpha_rdata PARAMS((int));
static void s_alpha_sdata PARAMS((int));
static void s_alpha_gprel32 PARAMS((int));
static void s_alpha_float_cons PARAMS((int));
static void s_alpha_proc PARAMS((int));
static void s_alpha_set PARAMS((int));
static void s_alpha_base PARAMS((int));
static void s_alpha_align PARAMS((int));
static void s_alpha_cons PARAMS((int));

static void create_literal_section PARAMS((const char *, segT*, symbolS**));
#ifndef OBJ_ELF
static void select_gp_value PARAMS((void));
#endif
static void alpha_align PARAMS((int, char *, symbolS *));


/* Generic assembler global variables which must be defined by all
   targets.  */

/* These are exported to relaxing code, even though we don't do any
   relaxing on this processor currently.  */
int md_short_jump_size = 4;
int md_long_jump_size = 4;

/* Characters which always start a comment.  */
const char comment_chars[] = "#";

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
#if 0
const char FLT_CHARS[] = "dD";
#else
/* XXX: Do all of these really get used on the alpha??  */
char FLT_CHARS[] = "rRsSfFdDxXpP";
#endif

const char *md_shortopts = "Fm:g";

struct option md_longopts[] = {
#define OPTION_32ADDR (OPTION_MD_BASE)
  { "32addr", no_argument, NULL, OPTION_32ADDR },
  { NULL, no_argument, NULL, 0 }
};

size_t md_longopts_size = sizeof(md_longopts);


/* The cpu for which we are generating code */
static unsigned alpha_target = AXP_OPCODE_ALL;
static const char *alpha_target_name = "<all>";

/* Forward declaration of the table of macros */
static const struct alpha_macro alpha_macros[];
static const int alpha_num_macros;

/* The hash table of instruction opcodes */
static struct hash_control *alpha_opcode_hash;

/* The hash table of macro opcodes */
static struct hash_control *alpha_macro_hash;

#ifdef OBJ_ECOFF
/* The $gp relocation symbol */
static symbolS *alpha_gp_symbol;

/* XXX: what is this, and why is it exported? */
valueT alpha_gp_value;
#endif

/* The current $gp register */
static int alpha_gp_register = AXP_REG_GP;

/* A table of the register symbols */
static symbolS *alpha_register_table[64];

/* Constant sections, or sections of constants */
#ifdef OBJ_ECOFF
static segT alpha_lita_section;
static segT alpha_lit4_section;
#endif
static segT alpha_lit8_section;

/* Symbols referring to said sections. */
#ifdef OBJ_ECOFF
static symbolS *alpha_lita_symbol;
static symbolS *alpha_lit4_symbol;
#endif
static symbolS *alpha_lit8_symbol;

/* Is the assembler not allowed to use $at? */
static int alpha_noat_on = 0;

/* Are macros enabled? */
static int alpha_macros_on = 1;

/* Are floats disabled? */
static int alpha_nofloats_on = 0;

/* Are addresses 32 bit? */
static int alpha_addr32_on = 0;

/* Symbol labelling the current insn.  When the Alpha gas sees
     foo:
       .quad 0
   and the section happens to not be on an eight byte boundary, it
   will align both the symbol and the .quad to an eight byte boundary.  */
static symbolS *alpha_insn_label;

/* Whether we should automatically align data generation pseudo-ops.
   .align 0 will turn this off.  */
static int alpha_auto_align_on = 1;

/* These are exported to ECOFF code.  */
unsigned long alpha_gprmask, alpha_fprmask;


/* Public interface functions */

/*
 * This function is called once, at assembler startup time.  It sets up
 * all the tables, etc. that the MD part of the assembler will need,
 * that can be determined before arguments are parsed.
 */

void
md_begin ()
{
  unsigned int i = 0;

  /* Create the opcode hash table */

  alpha_opcode_hash = hash_new ();
  for (i = 0; i < alpha_num_opcodes; )
    {
      const char *name, *retval;

      name = alpha_opcodes[i].name;
      retval = hash_insert (alpha_opcode_hash, name, (PTR)&alpha_opcodes[i]);
      if (retval)
	as_fatal ("internal error: can't hash opcode `%s': %s", name, retval);

      while (++i < alpha_num_opcodes
	     && (alpha_opcodes[i].name == name
		 || !strcmp (alpha_opcodes[i].name, name)))
	continue;
    }

  /* Some opcodes include modifiers of various sorts with a "/mod" syntax,
     like the architecture manual suggests.  However, for use with gcc at
     least, we also need access to those same opcodes without the "/".  */
  for (i = 0; i < alpha_num_opcodes; )
    {
      const char *name, *slash;
      name = alpha_opcodes[i].name;
      if ((slash = strchr(name, '/')) != NULL)
	{
	  char *p = xmalloc (strlen (name));
	  memcpy(p, name, slash-name);
	  strcpy(p+(slash-name), slash+1);

	  (void)hash_insert(alpha_opcode_hash, p, (PTR)&alpha_opcodes[i]);
	  /* Ignore failures -- the opcode table does duplicate some
	     variants in different forms, like "hw_stq" and "hw_st/q".  */
	}

      while (++i < alpha_num_opcodes
	     && (alpha_opcodes[i].name == name
		 || !strcmp (alpha_opcodes[i].name, name)))
	continue;
    }

  /* Create the macro hash table */

  alpha_macro_hash = hash_new ();
  for (i = 0; i < alpha_num_macros; )
    {
      const char *name, *retval;

      name = alpha_macros[i].name;
      retval = hash_insert (alpha_macro_hash, name, (PTR)&alpha_macros[i]);
      if (retval)
	as_fatal ("internal error: can't hash macro `%s': %s", name, retval);

      while (++i < alpha_num_macros
	     && (alpha_macros[i].name == name
		 || !strcmp (alpha_macros[i].name, name)))
	continue;
    }

  /* Construct symbols for each of the registers */

  for (i = 0; i < 32; ++i)
    {
      char name[4];
      sprintf(name, "$%d", i);
      alpha_register_table[i] = symbol_create(name, reg_section, i,
					      &zero_address_frag);
    }
  for (; i < 64; ++i)
    {
      char name[5];
      sprintf(name, "$f%d", i-32);
      alpha_register_table[i] = symbol_create(name, reg_section, i,
					      &zero_address_frag);
    }

  /* Create the special symbols and sections we'll be using */

  /* So .sbss will get used for tiny objects.  */
  bfd_set_gp_size (stdoutput, 8);

#ifdef OBJ_ECOFF
  create_literal_section (".lita", &alpha_lita_section, &alpha_lita_symbol);

  /* For handling the GP, create a symbol that won't be output in the
     symbol table.  We'll edit it out of relocs later.  */
  alpha_gp_symbol = symbol_create ("<GP value>", alpha_lita_section, 0x8000,
				   &zero_address_frag);
#endif

#ifdef OBJ_ELF
  if (ECOFF_DEBUGGING)
    {
      segT sec;

      sec = subseg_new(".mdebug", (subsegT)0);
      bfd_set_section_flags(stdoutput, sec, SEC_HAS_CONTENTS|SEC_READONLY);
      bfd_set_section_alignment(stdoutput, sec, 3);

#ifdef ERIC_neverdef
      sec = subseg_new(".reginfo", (subsegT)0);
      /* The ABI says this section should be loaded so that the running
	 program can access it.  */
      bfd_set_section_flags(stdoutput, sec, 
			    SEC_ALLOC|SEC_LOAD|SEC_READONLY|SEC_DATA);
      bfd_set_section_alignement(stdoutput, sec, 3);
#endif
    }
#endif /* OBJ_ELF */

  subseg_set(text_section, 0);
}

/*
 * The public interface to the instruction assembler.
 */

void
md_assemble (str)
     char *str;
{
  char opname[32];			/* current maximum is 13 */
  expressionS tok[MAX_INSN_ARGS];
  int ntok, opnamelen, trunclen;

  /* split off the opcode */
  opnamelen = strspn (str, "abcdefghijklmnopqrstuvwxyz_/48");
  trunclen = (opnamelen < sizeof (opname) - 1
	      ? opnamelen
	      : sizeof (opname) - 1);
  memcpy (opname, str, trunclen);
  opname[trunclen] = '\0';

  /* tokenize the rest of the line */
  if ((ntok = tokenize_arguments (str + opnamelen, tok, MAX_INSN_ARGS)) < 0)
    {
      as_bad ("syntax error");
      return;
    }

  /* finish it off */
  assemble_tokens (opname, tok, ntok, alpha_macros_on);
}

valueT
md_section_align (seg, size)
     segT seg;
     valueT size;
{
  int align = bfd_get_section_alignment(stdoutput, seg);
  valueT mask = ((valueT)1 << align) - 1;

  return (size + mask) & ~mask;
}

/*
 * Turn a string in input_line_pointer into a floating point constant
 * of type type, and store the appropriate bytes in *litP.  The number
 * of LITTLENUMS emitted is stored in *sizeP.  An error message is
 * returned, or NULL on OK. 
 */

/* Equal to MAX_PRECISION in atof-ieee.c */
#define MAX_LITTLENUMS 6

char *
md_atof (type, litP, sizeP)
     char type;
     char *litP;
     int *sizeP;
{
  int prec;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  LITTLENUM_TYPE *wordP;
  char *t;
  char *atof_ieee (), *vax_md_atof ();

  switch (type)
    {
      /* VAX floats */
    case 'G':
      /* VAX md_atof doesn't like "G" for some reason.  */
      type = 'g';
    case 'F':
    case 'D':
      return vax_md_atof (type, litP, sizeP);

      /* IEEE floats */
    case 'f':
      prec = 2;
      break;

    case 'd':
      prec = 4;
      break;

    case 'x':
    case 'X':
      prec = 6;
      break;

    case 'p':
    case 'P':
      prec = 6;
      break;

    default:
      *sizeP = 0;
      return "Bad call to MD_ATOF()";
    }
  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;
  *sizeP = prec * sizeof (LITTLENUM_TYPE);

  for (wordP = words + prec - 1; prec--;)
    {
      md_number_to_chars (litP, (long) (*wordP--), sizeof (LITTLENUM_TYPE));
      litP += sizeof (LITTLENUM_TYPE);
    }

  return 0;
}

void
md_bignum_to_chars (buf, bignum, nchars)
     char *buf;
     LITTLENUM_TYPE *bignum;
     int nchars;
{
  while (nchars)
    {
      LITTLENUM_TYPE work = *bignum++;
      int nb = CHARS_PER_LITTLENUM;

      do
	{
	  *buf++ = work & ((1 << BITS_PER_CHAR) - 1);
	  if (--nchars == 0)
	    return;
	  work >>= BITS_PER_CHAR;
	}
      while (--nb);
    }
}

int
md_parse_option (c, arg)
     int c;
     char *arg;
{
  switch (c)
    {
    case 'F':
      alpha_nofloats_on = 1;
      break;

    case OPTION_32ADDR:
      alpha_addr32_on = 1;
      break;

    case 'g':
      /* Ignore `-g' so gcc can provide this option to the Digital
	 UNIX assembler, which otherwise would throw away info that
	 mips-tfile needs.  */
      break;

    case 'm':
      {
	static const struct machine
	{
	  const char *name;
	  unsigned flags;
	} *p, m[] =
	{
	  { "21064", AXP_OPCODE_EV4|AXP_OPCODE_ALL },
	  { "21066", AXP_OPCODE_EV4|AXP_OPCODE_ALL },
	  { "21164", AXP_OPCODE_EV5|AXP_OPCODE_ALL },
	  { "21164a", AXP_OPCODE_EV56|AXP_OPCODE_ALL },
	  { "ev4", AXP_OPCODE_EV4|AXP_OPCODE_ALL },
	  { "ev45", AXP_OPCODE_EV4|AXP_OPCODE_ALL },
	  { "ev5", AXP_OPCODE_EV5|AXP_OPCODE_ALL },
	  { "ev56", AXP_OPCODE_EV56|AXP_OPCODE_ALL },
	  { "all", AXP_OPCODE_ALL },
	  { 0 }
	};
	
	for (p = m; p->name; ++p)
	  if (strcmp(arg, p->name) == 0)
	    {
	      alpha_target_name = p->name, alpha_target = p->flags;
	      goto found;
	    }
	as_warn("Unknown CPU identifier `%s'", arg);
      found:;
      }
      break;

    default:
      return 0;
    }

  return 1;
}

void
md_show_usage (stream)
     FILE *stream;
{
  fputs("\
Alpha options:\n\
-32addr			treat addresses as 32-bit values\n\
-F			lack floating point instructions support\n\
-m21064 | -m21066 | -m21164 | -m21164a | -m21264\n\
			specify variant of Alpha architecture\n",
	stream);
}

/* FIXME (inherited): @@ Is this right?? */

long
md_pcrel_from (fixP)
     fixS *fixP;
{
  valueT addr = fixP->fx_where + fixP->fx_frag->fr_address;
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_ALPHA_GPDISP:
    case BFD_RELOC_ALPHA_GPDISP_HI16:
    case BFD_RELOC_ALPHA_GPDISP_LO16:
      return addr;
    default:
      return fixP->fx_size + addr;
    }
}

int
md_apply_fix (fixP, valueP)
     fixS *fixP;
     valueT *valueP;
{
  char * const fixpos = fixP->fx_frag->fr_literal + fixP->fx_where;
  valueT value = *valueP;
  unsigned image, size;

  switch (fixP->fx_r_type)
    {
      /* The GPDISP relocations are processed internally with a symbol
	 referring to the current function; we need to drop in a value
	 which, when added to the address of the start of the function,
	 gives the desired GP.  */
    case BFD_RELOC_ALPHA_GPDISP_HI16:
      {
	fixS *next = fixP->fx_next;
	assert (next->fx_r_type == BFD_RELOC_ALPHA_GPDISP_LO16);

	fixP->fx_offset = (next->fx_frag->fr_address + next->fx_where 
			   - fixP->fx_frag->fr_address - fixP->fx_where);

	value = (value - sign_extend_16 (value)) >> 16;
      }
#ifdef OBJ_ELF
      fixP->fx_r_type = BFD_RELOC_ALPHA_GPDISP;
#endif
      goto do_reloc_gp;

    case BFD_RELOC_ALPHA_GPDISP_LO16:
      value = sign_extend_16 (value);
      fixP->fx_offset = 0;
#ifdef OBJ_ELF
      fixP->fx_done = 1;
#endif

    do_reloc_gp:
      fixP->fx_addsy = section_symbol (absolute_section);
      md_number_to_chars (fixpos, value, 2);
      break;

    case BFD_RELOC_16:
      size = 2;
      goto do_reloc_xx;
    case BFD_RELOC_32:
      size = 4;
      goto do_reloc_xx;
    case BFD_RELOC_64:
      size = 8;
    do_reloc_xx:
      if (fixP->fx_pcrel == 0 && fixP->fx_addsy == 0)
	{
	  md_number_to_chars (fixpos, value, size);
	  goto done;
	}
      return 1;

#ifdef OBJ_ECOFF
    case BFD_RELOC_GPREL32:
      assert (fixP->fx_subsy == alpha_gp_symbol);
      fixP->fx_subsy = 0;
      /* FIXME: inherited this obliviousness of `value' -- why? */
      md_number_to_chars (fixpos, -alpha_gp_value, 4);
      break;
#endif
#ifdef OBJ_ELF
    case BFD_RELOC_GPREL32:
      return 1;
#endif

    case BFD_RELOC_23_PCREL_S2:
      if (fixP->fx_pcrel == 0 && fixP->fx_addsy == 0)
	{
	  image = bfd_getl32(fixpos);
	  image = (image & ~0x1FFFFF) | ((value >> 2) & 0x1FFFFF);
	  goto write_done;
	}
      return 1;

    case BFD_RELOC_ALPHA_HINT:
      if (fixP->fx_pcrel == 0 && fixP->fx_addsy == 0)
	{
	  image = bfd_getl32(fixpos);
	  image = (image & ~0x3FFF) | ((value >> 2) & 0x3FFF);
	  goto write_done;
	}
      return 1;

#ifdef OBJ_ECOFF
    case BFD_RELOC_ALPHA_LITERAL:
      md_number_to_chars (fixpos, value, 2);
      return 1;

    case BFD_RELOC_ALPHA_LITUSE:
      return 1;
#endif
#ifdef OBJ_ELF
    case BFD_RELOC_ALPHA_LITERAL:
    case BFD_RELOC_ALPHA_LITUSE:
      return 1;
#endif

    default:
      {
	const struct alpha_operand *operand;

	if (fixP->fx_r_type <= BFD_RELOC_UNUSED)
	  as_fatal ("unhandled relocation type %s",
		    bfd_get_reloc_code_name (fixP->fx_r_type));

	assert (fixP->fx_r_type < BFD_RELOC_UNUSED + alpha_num_operands);
	operand = &alpha_operands[fixP->fx_r_type - BFD_RELOC_UNUSED];

	/* The rest of these fixups only exist internally during symbol
	   resolution and have no representation in the object file.  
	   Therefore they must be completely resolved as constants.  */

	if (fixP->fx_addsy != 0
	    && fixP->fx_addsy->bsym->section != absolute_section)
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			"non-absolute expression in constant field");

	image = bfd_getl32(fixpos);
	image = insert_operand(image, operand, (offsetT)value,
			       fixP->fx_file, fixP->fx_line);
      }
      goto write_done;
    }

  if (fixP->fx_addsy != 0 || fixP->fx_pcrel != 0)
    return 1;
  else
    {
      as_warn_where(fixP->fx_file, fixP->fx_line,
		    "type %d reloc done?\n", fixP->fx_r_type);
      goto done;
    }
      
write_done:
  md_number_to_chars(fixpos, image, 4);

done:
  fixP->fx_done = 1;
  return 0;
}

/* 
 * Look for a register name in the given symbol.
 */

symbolS *
md_undefined_symbol(name)
     char *name;
{
  if (*name == '$')
    {
      int is_float = 0, num;

      switch (*++name)
	{
	case 'f':
	  if (name[1] == 'p' && name[2] == '\0')
	    return alpha_register_table[AXP_REG_FP];
	  is_float = 32;
	  /* FALLTHRU */

	case 'r':
	  if (!isdigit(*++name))
	    break;
	  /* FALLTHRU */

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	  if (name[1] == '\0')
	    num = name[0] - '0';
	  else if (name[0] != '0' && isdigit(name[1]) && name[2] == '\0')
	    {
	      num = (name[0] - '0')*10 + name[1] - '0';
	      if (num >= 32)
		break;
	    }
	  else
	    break;

	  if (!alpha_noat_on && num == AXP_REG_AT)
	    as_warn("Used $at without \".set noat\"");
	  return alpha_register_table[num + is_float];

	case 'a':
	  if (name[1] == 't' && name[2] == '\0')
	    {
	      if (!alpha_noat_on)
		as_warn("Used $at without \".set noat\"");
	      return alpha_register_table[AXP_REG_AT];
	    }
	  break;

	case 'g':
	  if (name[1] == 'p' && name[2] == '\0')
	    return alpha_register_table[alpha_gp_register];
	  break;

	case 's':
	  if (name[1] == 'p' && name[2] == '\0')
	    return alpha_register_table[AXP_REG_SP];
	  break;
	}
    }
  return NULL;
}

#ifdef OBJ_ECOFF
void
alpha_frob_ecoff_data ()
{
  select_gp_value ();
  /* $zero and $f31 are read-only */
  alpha_gprmask &= ~1;
  alpha_fprmask &= ~1;
}
#endif

void
alpha_flush_pending_output ()
{
  alpha_insn_label = NULL;
}

void
alpha_define_label (sym)
     symbolS *sym;
{
  alpha_insn_label = sym;
}

int
alpha_force_relocation (f)
     fixS *f;
{
  switch (f->fx_r_type)
    {
    case BFD_RELOC_ALPHA_GPDISP_HI16:
    case BFD_RELOC_ALPHA_GPDISP_LO16:
    case BFD_RELOC_ALPHA_GPDISP:
    case BFD_RELOC_ALPHA_LITERAL:
    case BFD_RELOC_ALPHA_LITUSE:
    case BFD_RELOC_GPREL32:
      return 1;

    case BFD_RELOC_23_PCREL_S2:
    case BFD_RELOC_32:
    case BFD_RELOC_64:
    case BFD_RELOC_ALPHA_HINT:
      return 0;

    default:
      assert(f->fx_r_type > BFD_RELOC_UNUSED &&
	     f->fx_r_type < BFD_RELOC_UNUSED + alpha_num_operands);
      return 0;
    }
}

int
alpha_fix_adjustable (f)
     fixS *f;
{
#ifdef OBJ_ELF
  /* Prevent all adjustments to global symbols */
  if (S_IS_EXTERN (f->fx_addsy))
    return 0;
#endif

  /* Are there any relocation types for which we must generate a reloc
     but we can adjust the values contained within it?  */
  switch (f->fx_r_type)
    {
    case BFD_RELOC_GPREL32:
      return 1;
    default:
      return !alpha_force_relocation (f);
    }
  /*NOTREACHED*/
}

arelent *
tc_gen_reloc (sec, fixp)
     asection *sec;
     fixS *fixp;
{
  arelent *reloc;

  reloc = (arelent *) bfd_alloc_by_size_t (stdoutput, sizeof (arelent));
  reloc->sym_ptr_ptr = &fixp->fx_addsy->bsym;
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  assert (fixp->fx_r_type < BFD_RELOC_UNUSED);
  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    "cannot represent `%s' relocation in object file",
		    bfd_get_reloc_code_name (fixp->fx_r_type));
      return NULL;
    }

  if (!fixp->fx_pcrel != !reloc->howto->pc_relative)
    {
      as_fatal ("internal error? cannot generate `%s' relocation",
		bfd_get_reloc_code_name (fixp->fx_r_type));
    }
  assert (!fixp->fx_pcrel == !reloc->howto->pc_relative);

#ifdef OBJ_ECOFF
  if (fixp->fx_r_type == BFD_RELOC_ALPHA_LITERAL)
    {
      /* fake out bfd_perform_relocation. sigh */
      reloc->addend = -alpha_gp_value;
    }
  else
#endif
    {
      reloc->addend = fixp->fx_offset;
#ifdef OBJ_ELF
      /* 
       * Ohhh, this is ugly.  The problem is that if this is a local global
       * symbol, the relocation will entirely be performed at link time, not
       * at assembly time.  bfd_perform_reloc doesn't know about this sort
       * of thing, and as a result we need to fake it out here.
       */
      if (S_IS_EXTERN (fixp->fx_addsy) && !S_IS_COMMON(fixp->fx_addsy))
	reloc->addend -= fixp->fx_addsy->bsym->value;
#endif
    }

  return reloc;
}

int
tc_get_register (frame)
     int frame;
{
  int framereg = AXP_REG_SP;

  SKIP_WHITESPACE ();
  if (*input_line_pointer == '$')
    {
      char *s = input_line_pointer;
      char c = get_symbol_end();
      symbolS *sym = md_undefined_symbol(s);

      *strchr(s, '\0') = c;
      if (sym && (framereg = S_GET_VALUE(sym)) <= 31)
	  goto found;
    }
  as_warn ("frame reg expected, using $%d.", framereg);

found:
  note_gpreg (framereg);
  return framereg;
}


/* 
 * Parse the arguments to an opcode 
 */

static int
tokenize_arguments (str, tok, ntok)
     char *str;
     expressionS tok[];
     int ntok;
{
  expressionS *end_tok = tok + ntok;
  char *old_input_line_pointer;
  int saw_comma = 0, saw_arg = 0;

  memset (tok, 0, sizeof(*tok)*ntok);

  /* Save and restore input_line_pointer around this function */ 
  old_input_line_pointer = input_line_pointer;
  input_line_pointer = str;
  
  while (tok < end_tok && *input_line_pointer)
    {
      SKIP_WHITESPACE ();
      switch (*input_line_pointer)
	{
	case '\0':
	  goto fini;

	case ',':
	  ++input_line_pointer;
	  if (saw_comma || !saw_arg)
	    goto err;
	  saw_comma = 1;
	  break;

	case '(':
	  {
	    char *hold = input_line_pointer++;

	    /* First try for parenthesized register ... */
	    expression (tok);
	    if (*input_line_pointer == ')' && tok->X_op == O_register)
	      {
		tok->X_op = (saw_comma ? O_cpregister : O_pregister);
		saw_comma = 0;
		saw_arg = 1;
		++input_line_pointer;
		++tok;
		break;
	      }

	    /* ... then fall through to plain expression */
	    input_line_pointer = hold;
	  }

	default:
	  if (saw_arg && !saw_comma)
	    goto err;
	  expression (tok);
	  if (tok->X_op == O_illegal || tok->X_op == O_absent)
	    goto err;

	  saw_comma = 0;
	  saw_arg = 1;
	  ++tok;
	  break;
	}
    }

fini:
  if (saw_comma)
    goto err;
  input_line_pointer = old_input_line_pointer;
  return ntok - (end_tok - tok);

err:
  input_line_pointer = old_input_line_pointer;
  return -1;
}

/*
 * Search forward through all variants of an opcode
 * looking for a syntax match.
 */

static const struct alpha_opcode *
find_opcode_match(first_opcode, tok, pntok, pcpumatch)
     const struct alpha_opcode *first_opcode;
     const expressionS *tok;
     int *pntok;
     int *pcpumatch;
{
  const struct alpha_opcode *opcode = first_opcode;
  int ntok = *pntok;
  int got_cpu_match = 0;

  do
    {
      const unsigned char *opidx;
      int tokidx = 0;

      /* Don't match opcodes that don't exist on this architecture */
      if (!(opcode->flags & alpha_target))
	goto match_failed;

      got_cpu_match = 1;

      for (opidx = opcode->operands; *opidx; ++opidx)
	{
	  const struct alpha_operand *operand = &alpha_operands[*opidx];

	  /* only take input from real operands */
	  if (operand->flags & AXP_OPERAND_FAKE)
	    continue;

	  /* when we expect input, make sure we have it */
	  if (tokidx >= ntok)
	    {
	      if ((operand->flags & AXP_OPERAND_OPTIONAL_MASK) == 0)
		goto match_failed;
	      continue;
	    }

	  /* match operand type with expression type */
	  switch (operand->flags & AXP_OPERAND_TYPECHECK_MASK)
	    {
	    case AXP_OPERAND_IR:
	      if (tok[tokidx].X_op != O_register
		  || !is_ir_num(tok[tokidx].X_add_number))
		goto match_failed;
	      break;
	    case AXP_OPERAND_FPR:
	      if (tok[tokidx].X_op != O_register
		  || !is_fpr_num(tok[tokidx].X_add_number))
		goto match_failed;
	      break;
	    case AXP_OPERAND_IR|AXP_OPERAND_PARENS:
	      if (tok[tokidx].X_op != O_pregister
		  || !is_ir_num(tok[tokidx].X_add_number))
		goto match_failed;
	      break;
	    case AXP_OPERAND_IR|AXP_OPERAND_PARENS|AXP_OPERAND_COMMA:
	      if (tok[tokidx].X_op != O_cpregister
		  || !is_ir_num(tok[tokidx].X_add_number))
		goto match_failed;
	      break;

	    case AXP_OPERAND_RELATIVE:
	    case AXP_OPERAND_SIGNED:
	    case AXP_OPERAND_UNSIGNED:
	      switch (tok[tokidx].X_op)
		{
		case O_illegal:
		case O_absent:
		case O_register:
		case O_pregister:
		case O_cpregister:
		  goto match_failed;
		}
	      break;

	    default:
	      /* everything else should have been fake */
	      abort();
	    }
	  ++tokidx;
	}

      /* possible match -- did we use all of our input? */
      if (tokidx == ntok)
	{
	  *pntok = ntok;
	  return opcode;
	}

    match_failed:;
    }
  while (++opcode-alpha_opcodes < alpha_num_opcodes 
	 && !strcmp(opcode->name, first_opcode->name));

  if (*pcpumatch)
      *pcpumatch = got_cpu_match;

  return NULL;
}

/*
 * Search forward through all variants of a macro
 * looking for a syntax match.
 */

static const struct alpha_macro *
find_macro_match(first_macro, tok, pntok)
     const struct alpha_macro *first_macro;
     const expressionS *tok;
     int *pntok;
{
  const struct alpha_macro *macro = first_macro;
  int ntok = *pntok;

  do
    {
      const enum alpha_macro_arg *arg = macro->argsets;
      int tokidx = 0;

      while (*arg)
	{
	  switch (*arg)
	    {
	    case MACRO_EOA:
	      if (tokidx == ntok)
		return macro;
	      else
		tokidx = 0;
	      break;

	    case MACRO_IR:
	      if (tokidx >= ntok || tok[tokidx].X_op != O_register
		  || !is_ir_num(tok[tokidx].X_add_number))
		goto match_failed;
	      ++tokidx;
	      break;
	    case MACRO_PIR:
	      if (tokidx >= ntok || tok[tokidx].X_op != O_pregister
		  || !is_ir_num(tok[tokidx].X_add_number))
		goto match_failed;
	      ++tokidx;
	      break;
	    case MACRO_CPIR:
	      if (tokidx >= ntok || tok[tokidx].X_op != O_cpregister
		  || !is_ir_num(tok[tokidx].X_add_number))
		goto match_failed;
	      ++tokidx;
	      break;
	    case MACRO_FPR:
	      if (tokidx >= ntok || tok[tokidx].X_op != O_register
		  || !is_fpr_num(tok[tokidx].X_add_number))
		goto match_failed;
	      ++tokidx;
	      break;

	    case MACRO_EXP:
	      if (tokidx >= ntok)
		goto match_failed;
	      switch (tok[tokidx].X_op)
		{
		case O_illegal:
		case O_absent:
		case O_register:
		case O_pregister:
		case O_cpregister:
		  goto match_failed;
		}
	      ++tokidx;
	      break;
		
	    match_failed:
	      while (*arg != MACRO_EOA)
		++arg;
	      tokidx = 0;
	      break;
	    }
	  ++arg;
	}
    }
  while (++macro-alpha_macros < alpha_num_macros 
	 && !strcmp(macro->name, first_macro->name));

  return NULL;
}

/*
 * Insert an operand value into an instruction.
 */

static unsigned
insert_operand(insn, operand, val, file, line)
     unsigned insn;
     const struct alpha_operand *operand;
     offsetT val;
     char *file;
     unsigned line;
{
  if (operand->bits != 32 && !(operand->flags & AXP_OPERAND_NOOVERFLOW))
    {
      offsetT min, max;

      if (operand->flags & AXP_OPERAND_SIGNED)
	{
	  max = (1 << (operand->bits - 1)) - 1;
	  min = -(1 << (operand->bits - 1));
	}
      else
	{
	  max = (1 << operand->bits) - 1;
	  min = 0;
	}

      if (val < min || val > max)
	{
	  const char *err = 
	    "operand out of range (%s not between %d and %d)";
	  char buf[sizeof(val)*3+2];

	  sprint_value(buf, val);
	  if (file)
	    as_warn_where(file, line, err, buf, min, max);
	  else
	    as_warn(err, buf, min, max);
	}
    }

  if (operand->insert)
    {
      const char *errmsg = NULL;

      insn = (*operand->insert)(insn, val, &errmsg);
      if (errmsg)
	as_warn(errmsg);
    }
  else
    insn |= ((val & ((1 << operand->bits) - 1)) << operand->shift);

  return insn;
}

/* 
 * Turn an opcode description and a set of arguments into
 * an instruction and a fixup.
 */

static void
assemble_insn(opcode, tok, ntok, insn)
     const struct alpha_opcode *opcode;
     const expressionS *tok;
     int ntok;
     struct alpha_insn *insn;
{
  const unsigned char *argidx;
  unsigned image;
  int tokidx = 0;

  memset(insn, 0, sizeof(*insn));
  image = opcode->opcode;

  for (argidx = opcode->operands; *argidx; ++argidx)
    {
      const struct alpha_operand *operand = &alpha_operands[*argidx];
      const expressionS *t;

      if (operand->flags & AXP_OPERAND_FAKE)
	{
	  /* fake operands take no value and generate no fixup */
	  image = insert_operand(image, operand, 0, NULL, 0);
	  continue;
	}

      if (tokidx >= ntok)
	{
	  switch (operand->flags & AXP_OPERAND_OPTIONAL_MASK)
	    {
	    case AXP_OPERAND_DEFAULT_FIRST:
	      t = &tok[0];
	      break;
	    case AXP_OPERAND_DEFAULT_SECOND:
	      t = &tok[1];
	      break;
	    case AXP_OPERAND_DEFAULT_ZERO:
	      {
		static const expressionS zero_exp = { 0, 0, 0, O_constant, 1 };
		t = &zero_exp;
	      }
	      break;
	    default:
	      abort();
	    }
	}
      else
	t = &tok[tokidx++];

      switch (t->X_op)
	{
	case O_register:
	case O_pregister:
	case O_cpregister:
	  image = insert_operand(image, operand, regno(t->X_add_number),
				 NULL, 0);
	  break;

	case O_constant:
	  image = insert_operand(image, operand, t->X_add_number, NULL, 0);
	  break;

	default:
	  {
	    struct alpha_fixup *fixup;

	    if (insn->nfixups >= MAX_INSN_FIXUPS)
	      as_fatal("too many fixups");

	    fixup = &insn->fixups[insn->nfixups++];

	    fixup->exp = *t;
	    fixup->reloc = operand->default_reloc;
	  }
	  break;
	}
    }

  insn->insn = image;
}

/* 
 * Actually output an instruction with its fixup.
 */

static void
emit_insn(insn)
    struct alpha_insn *insn;
{
  char *f;
  int i;

  /* Take care of alignment duties */
  if (alpha_auto_align_on)
    alpha_align (2, (char *) NULL, alpha_insn_label);
  alpha_insn_label = NULL;

  /* Write out the instruction.  */
  f = frag_more (4);
  md_number_to_chars (f, insn->insn, 4);

  /* Apply the fixups in order */
  for (i = 0; i < insn->nfixups; ++i)
    {
      struct alpha_fixup *fixup = &insn->fixups[i];
      int size, pcrel;
      fixS *fixP;

      /* Some fixups are only used internally and so have no howto */
      if (fixup->reloc > BFD_RELOC_UNUSED)
	size = 4, pcrel = 0;
#ifdef OBJ_ELF
      /* These relocation types are only used internally. */
      else if (fixup->reloc == BFD_RELOC_ALPHA_GPDISP_HI16
	       || fixup->reloc == BFD_RELOC_ALPHA_GPDISP_LO16)
	{
	  size = 2, pcrel = 0;
	}
#endif
      else
	{
	  reloc_howto_type *reloc_howto 
	    = bfd_reloc_type_lookup (stdoutput, fixup->reloc);
	  assert (reloc_howto);

	  size = bfd_get_reloc_size (reloc_howto);
	  pcrel = reloc_howto->pc_relative;
	}
      assert (size >= 1 && size <= 4);

      fixP = fix_new_exp (frag_now, f - frag_now->fr_literal, size,
			  &fixup->exp, pcrel, fixup->reloc);

      /* Turn off complaints that the addend is too large for some fixups */
      switch (fixup->reloc)
	{
	case BFD_RELOC_ALPHA_GPDISP_LO16:
	case BFD_RELOC_ALPHA_LITERAL:
	case BFD_RELOC_GPREL32:
	  fixP->fx_no_overflow = 1;
	  break;
	default:
	  break;
	}
    }
}

/*
 * Given an opcode name and a pre-tokenized set of arguments,
 * assemble the insn, but do not emit it. 
 */

static void
assemble_tokens_to_insn(opname, tok, ntok, insn)
     const char *opname;
     const expressionS *tok;
     int ntok;
     struct alpha_insn *insn;
{
  const struct alpha_opcode *opcode;

  /* search opcodes */
  opcode = (const struct alpha_opcode *) hash_find (alpha_opcode_hash, opname);
  if (opcode)
    {
      int cpumatch;
      opcode = find_opcode_match (opcode, tok, &ntok, &cpumatch);
      if (opcode)
	{
	  assemble_insn (opcode, tok, ntok, insn);
	  return;
	}
      else if (cpumatch)
	as_bad ("inappropriate arguments for opcode `%s'", opname);
      else
	as_bad ("opcode `%s' not supported for target %s", opname,
	       alpha_target_name);
    }
  else
    as_bad ("unknown opcode `%s'", opname);
}

/*
 * Given an opcode name and a pre-tokenized set of arguments,
 * take the opcode all the way through emission.
 */

static void
assemble_tokens (opname, tok, ntok, local_macros_on)
     const char *opname;
     const expressionS *tok;
     int ntok;
     int local_macros_on;
{
  int found_something = 0;
  const struct alpha_opcode *opcode;
  const struct alpha_macro *macro;
  int cpumatch = 1;

  /* search macros */
  if (local_macros_on)
    {
      macro = ((const struct alpha_macro *)
	       hash_find (alpha_macro_hash, opname));
      if (macro)
	{
	  found_something = 1;
	  macro = find_macro_match (macro, tok, &ntok);
	  if (macro)
	    {
	      (*macro->emit) (tok, ntok, macro->arg);
	      return;
	    }
	}
    }

  /* search opcodes */
  opcode = (const struct alpha_opcode *) hash_find (alpha_opcode_hash, opname);
  if (opcode)
    {
      found_something = 1;
      opcode = find_opcode_match (opcode, tok, &ntok, &cpumatch);
      if (opcode)
	{
	  struct alpha_insn insn;
	  assemble_insn (opcode, tok, ntok, &insn);
	  emit_insn (&insn);
	  return;
	}
    }
  
  if (found_something)
    if (cpumatch)
      as_bad ("inappropriate arguments for opcode `%s'", opname);
    else
      as_bad ("opcode `%s' not supported for target %s", opname,
	     alpha_target_name);
  else
    as_bad ("unknown opcode `%s'", opname);
}


/* Some instruction sets indexed by lg(size) */
static const char * const sextX_op[] = { "sextb", "sextw", "sextl", NULL };
static const char * const insXl_op[] = { "insbl", "inswl", "insll", "insql" };
static const char * const insXh_op[] = { NULL,    "inswh", "inslh", "insqh" };
static const char * const extXl_op[] = { "extbl", "extwl", "extll", "extql" };
static const char * const extXh_op[] = { NULL,    "extwh", "extlh", "extqh" };
static const char * const mskXl_op[] = { "mskbl", "mskwl", "mskll", "mskql" };
static const char * const mskXh_op[] = { NULL,    "mskwh", "msklh", "mskqh" };

static void 
emit_ldgp (tok, ntok, unused)
     const expressionS *tok;
     int ntok;
     void *unused;
{
#ifdef OBJ_AOUT
FIXME
#endif
#if defined(OBJ_ECOFF) || defined(OBJ_ELF)
  /* from "ldgp r1,n(r2)", generate "ldah r1,X(R2); lda r1,Y(r1)"
     with appropriate constants and relocations.  */
  struct alpha_insn insn;
  expressionS newtok[3];
  expressionS addend;

  /* We're going to need this symbol in md_apply_fix().  */
  (void) section_symbol (absolute_section);

#ifdef OBJ_ECOFF
  if (regno (tok[2].X_add_number) == AXP_REG_PV)
    ecoff_set_gp_prolog_size (0);
#endif

  newtok[0] = tok[0];
  set_tok_const (newtok[1], 0);
  newtok[2] = tok[2];

  assemble_tokens_to_insn ("ldah", newtok, 3, &insn);

  addend = tok[1];

#ifdef OBJ_ECOFF
  assert (addend.X_op == O_constant);
  addend.X_op = O_symbol;
  addend.X_add_symbol = alpha_gp_symbol;
#endif

  insn.nfixups = 1;
  insn.fixups[0].exp = addend;
  insn.fixups[0].reloc = BFD_RELOC_ALPHA_GPDISP_HI16;

  emit_insn (&insn);

  set_tok_preg (newtok[2], tok[0].X_add_number);
  
  assemble_tokens_to_insn ("lda", newtok, 3, &insn);

#ifdef OBJ_ECOFF
  addend.X_add_number += 4;
#endif

  insn.nfixups = 1;
  insn.fixups[0].exp = addend;
  insn.fixups[0].reloc = BFD_RELOC_ALPHA_GPDISP_LO16;

  emit_insn (&insn);
#endif /* OBJ_ECOFF || OBJ_ELF */
}

static int
load_expression (targreg, exp, pbasereg, poffset)
     int targreg;
     const expressionS *exp;
     int *pbasereg;
     expressionS *poffset;
{
  int emit_lituse = 0;
  offsetT addend = exp->X_add_number;
  int basereg = *pbasereg;
  struct alpha_insn insn;
  expressionS newtok[3];

  switch (exp->X_op)
    {
    case O_symbol:
      {
#ifdef OBJ_ECOFF
	offsetT lit;

	/* attempt to reduce .lit load by splitting the offset from
	   its symbol when possible, but don't create a situation in
	   which we'd fail.  */
	if (!range_signed_32 (addend) &&
	    (alpha_noat_on || targreg == AXP_REG_AT))
	  {
	    lit = add_to_literal_pool (exp->X_add_symbol, addend,
				       alpha_lita_section, 8);
	    addend = 0;
	  }
	else
	  {
	    lit = add_to_literal_pool (exp->X_add_symbol, 0,
				       alpha_lita_section, 8);
	  }

	if (lit >= 0x8000)
	  as_fatal ("overflow in literal (.lita) table");

	/* emit "ldq r, lit(gp)" */

	if (basereg != alpha_gp_register && targreg == basereg)
	  {
	    if (alpha_noat_on)
	      as_bad ("macro requires $at register while noat in effect");
	    if (targreg == AXP_REG_AT)
	      as_bad ("macro requires $at while $at in use");
	    
	    set_tok_reg (newtok[0], AXP_REG_AT);
	  }
	else
	  set_tok_reg (newtok[0], targreg);
	set_tok_sym (newtok[1], alpha_lita_symbol, lit);
	set_tok_preg (newtok[2], alpha_gp_register);

	assemble_tokens_to_insn ("ldq", newtok, 3, &insn);

	assert (insn.nfixups == 1);
	insn.fixups[0].reloc = BFD_RELOC_ALPHA_LITERAL;
#endif /* OBJ_ECOFF */
#ifdef OBJ_ELF
	/* emit "ldq r, gotoff(gp)" */

	if (basereg != alpha_gp_register && targreg == basereg)
	  {
	    if (alpha_noat_on)
	      as_bad ("macro requires $at register while noat in effect");
	    if (targreg == AXP_REG_AT)
	      as_bad ("macro requires $at while $at in use");
	    
	    set_tok_reg (newtok[0], AXP_REG_AT);
	  }
	else
	  set_tok_reg (newtok[0], targreg);

	if (!range_signed_32 (addend)
	    && (alpha_noat_on || targreg == AXP_REG_AT))
	  {
	    newtok[1] = *exp;
	    addend = 0;
	  }
	else
	  {
	    set_tok_sym (newtok[1], exp->X_add_symbol, 0);
	  }

	set_tok_preg (newtok[2], alpha_gp_register);

	assemble_tokens_to_insn ("ldq", newtok, 3, &insn);

	assert (insn.nfixups == 1);
	insn.fixups[0].reloc = BFD_RELOC_ALPHA_LITERAL;
#endif /* OBJ_ELF */

	emit_insn(&insn);
	emit_lituse = 1;

	if (basereg != alpha_gp_register && basereg != AXP_REG_ZERO)
	  {
	    /* emit "addq r, base, r" */

	    set_tok_reg (newtok[1], basereg);
	    set_tok_reg (newtok[2], targreg);
	    assemble_tokens ("addq", newtok, 3, 0);
	  }

	basereg = targreg;
      }
      break;

    case O_constant:
      break;

    case O_subtract:
      /* Assume that this difference expression will be resolved to an
	 absolute value and that that value will fit in 16 bits. */

      set_tok_reg (newtok[0], targreg);
      newtok[1] = *exp;
      set_tok_preg (newtok[2], basereg);
      assemble_tokens ("lda", newtok, 3, 0);

      if (poffset)
	set_tok_const (*poffset, 0);
      return 0;

    default:
      abort();
    }

  if (!range_signed_32 (addend))
    {
      offsetT lit;

      /* for 64-bit addends, just put it in the literal pool */

      if (alpha_lit8_section == NULL)
	{
	  create_literal_section (".lit8",
				  &alpha_lit8_section,
				  &alpha_lit8_symbol);
	}

      lit = add_to_literal_pool (NULL, addend, alpha_lit8_section, 8) - 0x8000;
      if (lit >= 0x8000)
	as_fatal ("overflow in literal (.lit8) table");

      /* emit "ldq litreg, .lit8+lit" */

      if (targreg == basereg)
	{
	  if (alpha_noat_on)
	    as_bad ("macro requires $at register while noat in effect");
	  if (targreg == AXP_REG_AT)
	    as_bad ("macro requires $at while $at in use");

	  set_tok_reg (newtok[0], AXP_REG_AT);
	}
      else
	set_tok_reg (newtok[0], targreg);
      set_tok_sym (newtok[1], alpha_lit8_symbol, lit);

      assemble_tokens ("ldq", newtok, 2, 1); /* note this does recurse */

      /* emit "addq litreg, base, target" */

      if (basereg != AXP_REG_ZERO)
	{
	  set_tok_reg (newtok[1], basereg);
	  set_tok_reg (newtok[2], targreg);
	  assemble_tokens ("addq", newtok, 3, 0);
	}

      if (poffset)
	set_tok_const (*poffset, 0);
      *pbasereg = targreg;
    }
  else
    {
      offsetT low, high, extra, tmp;

      /* for 32-bit operands, break up the addend */

      low = sign_extend_16 (addend);
      tmp = addend - low;
      high = sign_extend_16 (tmp >> 16);

      if (tmp - (high << 16))
	{
	  extra = 0x4000;
	  tmp -= 0x40000000;
	  high = sign_extend_16 (tmp >> 16);
	}
      else
	extra = 0;

      set_tok_reg (newtok[0], targreg);
      set_tok_preg (newtok[2], basereg);

      if (extra)
	{
	  /* emit "ldah r, extra(r) */
	  set_tok_const (newtok[1], extra);
	  assemble_tokens ("ldah", newtok, 3, 0);
	  set_tok_preg (newtok[2], basereg = targreg);
	}

      if (high)
	{
	  /* emit "ldah r, high(r) */
	  set_tok_const (newtok[1], high);
	  assemble_tokens ("ldah", newtok, 3, 0);
	  basereg = targreg;
	  set_tok_preg (newtok[2], basereg);
	}

      if ((low && !poffset) || (!poffset && basereg != targreg))
	{
	  /* emit "lda r, low(base)" */
	  set_tok_const (newtok[1], low);
	  assemble_tokens ("lda", newtok, 3, 0);
	  basereg = targreg;
	  low = 0;
	}

      if (poffset)
	set_tok_const (*poffset, low);
      *pbasereg = basereg;
    }

  return emit_lituse;
}

static void
emit_lda (tok, ntok, unused)
     const expressionS *tok;
     int ntok;
     void *unused;
{
  int basereg;

  if (ntok == 2)
    basereg = (tok[1].X_op == O_constant ? AXP_REG_ZERO : alpha_gp_register);
  else
    basereg = tok[2].X_add_number;

  (void) load_expression (tok[0].X_add_number, &tok[1], &basereg, NULL);
}

static void
emit_ldah (tok, ntok, unused)
     const expressionS *tok;
     int ntok;
     void *unused;
{
  expressionS newtok[3];

  newtok[0] = tok[0];
  newtok[1] = tok[1];
  set_tok_preg (newtok[2], AXP_REG_ZERO);

  assemble_tokens ("ldah", newtok, 3, 0);
}

static void
emit_ir_load (tok, ntok, opname)
     const expressionS *tok;
     int ntok;
     void *opname;
{
  int basereg, lituse;
  expressionS newtok[3];
  struct alpha_insn insn;

  if (ntok == 2)
    basereg = (tok[1].X_op == O_constant ? AXP_REG_ZERO : alpha_gp_register);
  else
    basereg = tok[2].X_add_number;

  lituse = load_expression (tok[0].X_add_number, &tok[1], &basereg,
			    &newtok[1]);

  newtok[0] = tok[0];
  set_tok_preg (newtok[2], basereg);

  assemble_tokens_to_insn ((const char *)opname, newtok, 3, &insn);

  if (lituse)
    {
      assert (insn.nfixups < MAX_INSN_FIXUPS);
      if (insn.nfixups > 0)
	{
	  memmove (&insn.fixups[1], &insn.fixups[0],
		   sizeof(struct alpha_fixup) * insn.nfixups);
	}
      insn.nfixups++;
      insn.fixups[0].reloc = BFD_RELOC_ALPHA_LITUSE;
      insn.fixups[0].exp.X_op = O_constant;
      insn.fixups[0].exp.X_add_number = 1;
    }

  emit_insn (&insn);
}

static void
emit_loadstore (tok, ntok, opname)
     const expressionS *tok;
     int ntok;
     void *opname;
{
  int basereg, lituse;
  expressionS newtok[3];
  struct alpha_insn insn;
  
  if (ntok == 2)
    basereg = (tok[1].X_op == O_constant ? AXP_REG_ZERO : alpha_gp_register);
  else
    basereg = tok[2].X_add_number;

  if (tok[1].X_op != O_constant || !range_signed_16(tok[1].X_add_number))
    {
      if (alpha_noat_on)
	as_bad ("macro requires $at register while noat in effect");

      lituse = load_expression (AXP_REG_AT, &tok[1], &basereg, &newtok[1]);
    }
  else
    {
      newtok[1] = tok[1];
      lituse = 0;
    }

  newtok[0] = tok[0];
  set_tok_preg (newtok[2], basereg);

  assemble_tokens_to_insn ((const char *)opname, newtok, 3, &insn);

  if (lituse)
    {
      assert (insn.nfixups < MAX_INSN_FIXUPS);
      if (insn.nfixups > 0)
	{
	  memmove (&insn.fixups[1], &insn.fixups[0],
		   sizeof(struct alpha_fixup) * insn.nfixups);
	}
      insn.nfixups++;
      insn.fixups[0].reloc = BFD_RELOC_ALPHA_LITUSE;
      insn.fixups[0].exp.X_op = O_constant;
      insn.fixups[0].exp.X_add_number = 1;
    }

  emit_insn (&insn);
}

static void 
emit_ldXu (tok, ntok, vlgsize)
     const expressionS *tok;
     int ntok;
     void *vlgsize;
{
  expressionS newtok[3];

  if (alpha_noat_on)
    as_bad ("macro requires $at register while noat in effect");
  
  /* emit "lda $at, exp" */

  memcpy (newtok, tok, sizeof(expressionS)*ntok);
  newtok[0].X_add_number = AXP_REG_AT;
  assemble_tokens ("lda", newtok, ntok, 1);

  /* emit "ldq_u targ, 0($at)" */

  newtok[0] = tok[0];
  set_tok_const (newtok[1], 0);
  set_tok_preg (newtok[2], AXP_REG_AT);
  assemble_tokens ("ldq_u", newtok, 3, 1);

  /* emit "extXl targ, $at, targ" */

  set_tok_reg (newtok[1], AXP_REG_AT);
  newtok[2] = newtok[0];
  assemble_tokens (extXl_op[(long)vlgsize], newtok, 3, 1);
}

static void 
emit_ldX (tok, ntok, vlgsize)
     const expressionS *tok;
     int ntok;
     void *vlgsize;
{
  emit_ldXu (tok, ntok, vlgsize);
  assemble_tokens (sextX_op[(long)vlgsize], tok, 1, 1);
}

static void
emit_uldXu (tok, ntok, vlgsize)
     const expressionS *tok;
     int ntok;
     void *vlgsize;
{
  long lgsize = (long)vlgsize;
  expressionS newtok[3];

  if (alpha_noat_on)
    as_bad ("macro requires $at register while noat in effect");

  /* emit "lda $at, exp" */

  memcpy (newtok, tok, sizeof(expressionS)*ntok);
  newtok[0].X_add_number = AXP_REG_AT;
  assemble_tokens ("lda", newtok, ntok, 1);

  /* emit "ldq_u $t9, 0($at)" */

  set_tok_reg (newtok[0], AXP_REG_T9);
  set_tok_const (newtok[1], 0);
  set_tok_preg (newtok[2], AXP_REG_AT);
  assemble_tokens ("ldq_u", newtok, 3, 1);

  /* emit "ldq_u $t10, size-1($at)" */

  set_tok_reg (newtok[0], AXP_REG_T10);
  set_tok_const (newtok[1], (1<<lgsize)-1);
  assemble_tokens ("ldq_u", newtok, 3, 1);

  /* emit "extXl $t9, $at, $t9" */

  set_tok_reg (newtok[0], AXP_REG_T9);
  set_tok_reg (newtok[1], AXP_REG_AT);
  set_tok_reg (newtok[2], AXP_REG_T9);
  assemble_tokens (extXl_op[lgsize], newtok, 3, 1);

  /* emit "extXh $t10, $at, $t10" */

  set_tok_reg (newtok[0], AXP_REG_T10);
  set_tok_reg (newtok[2], AXP_REG_T10);
  assemble_tokens (extXh_op[lgsize], newtok, 3, 1);

  /* emit "or $t9, $t10, targ" */

  set_tok_reg (newtok[0], AXP_REG_T9);
  set_tok_reg (newtok[1], AXP_REG_T10);
  newtok[2] = tok[0];
  assemble_tokens ("or", newtok, 3, 1);
}
  
static void
emit_uldX (tok, ntok, vlgsize)
     const expressionS *tok;
     int ntok;
     void *vlgsize;
{
  emit_uldXu (tok, ntok, vlgsize);
  assemble_tokens (sextX_op[(long)vlgsize], tok, 1, 1);
}

static void
emit_ldil (tok, ntok, unused)
     const expressionS *tok;
     int ntok;
     void *unused;
{
  expressionS newtok[2];

  memcpy (newtok, tok, sizeof(newtok));
  newtok[1].X_add_number = sign_extend_32 (tok[1].X_add_number);

  assemble_tokens ("lda", newtok, ntok, 1);
}

static void
emit_stX (tok, ntok, vlgsize)
     const expressionS *tok;
     void *vlgsize;
{
  int lgsize = (int)(long)vlgsize;
  expressionS newtok[3];

  if (alpha_noat_on)
    as_bad("macro requires $at register while noat in effect");

  /* emit "lda $at, exp" */

  memcpy (newtok, tok, sizeof(expressionS)*ntok);
  newtok[0].X_add_number = AXP_REG_AT;
  assemble_tokens ("lda", newtok, ntok, 1);

  /* emit "ldq_u $t9, 0($at)" */

  set_tok_reg (newtok[0], AXP_REG_T9);
  set_tok_const (newtok[1], 0);
  set_tok_preg (newtok[2], AXP_REG_AT);
  assemble_tokens ("ldq_u", newtok, 3, 1);

  /* emit "insXl src, $at, $t10" */

  newtok[0] = tok[0];
  set_tok_reg (newtok[1], AXP_REG_AT);
  set_tok_reg (newtok[2], AXP_REG_T10);
  assemble_tokens (insXl_op[lgsize], newtok, 3, 1);

  /* emit "mskXl $t9, $at, $t9" */

  set_tok_reg (newtok[0], AXP_REG_T9);
  newtok[2] = newtok[0];
  assemble_tokens (mskXl_op[lgsize], newtok, 3, 1);

  /* emit "or $t9, $t10, $t9" */

  set_tok_reg (newtok[1], AXP_REG_T10);
  assemble_tokens ("or", newtok, 3, 1);

  /* emit "stq_u $t9, 0($at) */

  set_tok_const (newtok[1], 0);
  set_tok_preg (newtok[2], AXP_REG_AT);
  assemble_tokens ("stq_u", newtok, 3, 1);
}

static void
emit_ustX (tok, ntok, vlgsize)
     const expressionS *tok;
     int ntok;
     void *vlgsize;
{
  int lgsize = (int)(long)vlgsize;
  expressionS newtok[3];

  /* emit "lda $at, exp" */

  memcpy (newtok, tok, sizeof(expressionS)*ntok);
  newtok[0].X_add_number = AXP_REG_AT;
  assemble_tokens ("lda", newtok, ntok, 1);

  /* emit "ldq_u $9, 0($at)" */

  set_tok_reg (newtok[0], AXP_REG_T9);
  set_tok_const (newtok[1], 0);
  set_tok_preg (newtok[2], AXP_REG_AT);
  assemble_tokens ("ldq_u", newtok, 3, 1);

  /* emit "ldq_u $10, size-1($at)" */

  set_tok_reg (newtok[0], AXP_REG_T10);
  set_tok_const (newtok[1], (1 << lgsize)-1);
  assemble_tokens ("ldq_u", newtok, 3, 1);

  /* emit "insXl src, $at, $t11" */

  newtok[0] = tok[0];
  set_tok_reg (newtok[1], AXP_REG_AT);
  set_tok_reg (newtok[2], AXP_REG_T11);
  assemble_tokens (insXl_op[lgsize], newtok, 3, 1);

  /* emit "insXh src, $at, $t12" */

  set_tok_reg (newtok[2], AXP_REG_T12);
  assemble_tokens (insXh_op[lgsize], newtok, 3, 1);

  /* emit "mskXl $t9, $at, $t9" */

  set_tok_reg (newtok[0], AXP_REG_T9);
  newtok[2] = newtok[0];
  assemble_tokens (mskXl_op[lgsize], newtok, 3, 1);

  /* emit "mskXh $t10, $at, $t10" */

  set_tok_reg (newtok[0], AXP_REG_T10);
  newtok[2] = newtok[0];
  assemble_tokens (mskXh_op[lgsize], newtok, 3, 1);

  /* emit "or $t9, $t11, $t9" */

  set_tok_reg (newtok[0], AXP_REG_T9);
  set_tok_reg (newtok[1], AXP_REG_T11);
  newtok[2] = newtok[0];
  assemble_tokens ("or", newtok, 3, 1);

  /* emit "or $t10, $t12, $t10" */

  set_tok_reg (newtok[0], AXP_REG_T10);
  set_tok_reg (newtok[1], AXP_REG_T12);
  newtok[2] = newtok[0];
  assemble_tokens ("or", newtok, 3, 1);

  /* emit "stq_u $t9, 0($at)" */

  set_tok_reg (newtok[0], AXP_REG_T9);
  set_tok_const (newtok[1], 0);
  set_tok_preg (newtok[2], AXP_REG_AT);
  assemble_tokens ("stq_u", newtok, 3, 1);

  /* emit "stq_u $t10, size-1($at)" */

  set_tok_reg (newtok[0], AXP_REG_T10);
  set_tok_const (newtok[1], (1 << lgsize)-1);
  assemble_tokens ("stq_u", newtok, 3, 1);
}

static void
emit_sextX (tok, ntok, vlgsize)
     const expressionS *tok;
     int ntok;
     void *vlgsize;
{
  int bitshift = 64 - 8*(1 << (long)vlgsize);
  expressionS newtok[3];

  /* emit "sll src,bits,dst" */

  newtok[0] = tok[0];
  set_tok_const (newtok[1], bitshift);
  newtok[2] = tok[ntok - 1];
  assemble_tokens ("sll", newtok, 3, 1);

  /* emit "sra dst,bits,dst" */

  newtok[0] = newtok[2];
  assemble_tokens ("sra", newtok, 3, 1);
}

static void 
emit_division (tok, ntok, symname)
     const expressionS *tok;
     int ntok;
     void *symname;
{
  /* DIVISION and MODULUS. Yech.
   * Convert
   *    OP x,y,result
   * to
   *    lda pv,__OP
   *    mov x,t10
   *    mov y,t11
   *    jsr t9,(pv),__OP
   *    mov t12,result
   *
   * with appropriate optimizations if t10,t11,t12 are the registers
   * specified by the compiler. 
   */

  int xr, yr, rr;
  symbolS *sym;
  expressionS newtok[3];

  xr = regno (tok[0].X_add_number);
  yr = regno (tok[1].X_add_number);
    
  if (ntok < 3)
    rr = xr;
  else
    rr = regno (tok[2].X_add_number);

  sym = symbol_find_or_make ((const char *)symname);

  /* Move the operands into the right place */
  if (yr == AXP_REG_T10 && xr == AXP_REG_T11)
    {
      /* They are in exactly the wrong order -- swap through AT */

      if (alpha_noat_on)
	as_bad ("macro requires $at register while noat in effect");

      set_tok_reg (newtok[0], AXP_REG_T10);
      set_tok_reg (newtok[1], AXP_REG_AT);
      assemble_tokens ("mov", newtok, 2, 1);

      set_tok_reg (newtok[0], AXP_REG_T11);
      set_tok_reg (newtok[1], AXP_REG_T10);
      assemble_tokens ("mov", newtok, 2, 1);

      set_tok_reg (newtok[0], AXP_REG_AT);
      set_tok_reg (newtok[1], AXP_REG_T11);
      assemble_tokens ("mov", newtok, 2, 1);
    }
  else
    {
      if (yr == AXP_REG_T10)
	{
	  set_tok_reg (newtok[0], AXP_REG_T10);
	  set_tok_reg (newtok[1], AXP_REG_T11);
	  assemble_tokens ("mov", newtok, 2, 1);
	}

      if (xr != AXP_REG_T10)
	{
	  set_tok_reg (newtok[0], xr);
	  set_tok_reg (newtok[1], AXP_REG_T10);
          assemble_tokens ("mov", newtok, 2, 1);
	}

      if (yr != AXP_REG_T10 && yr != AXP_REG_T11)
	{
	  set_tok_reg (newtok[0], yr);
	  set_tok_reg (newtok[1], AXP_REG_T11);
	  assemble_tokens ("mov", newtok, 2, 1);
	}
    }

  /* Call the division routine */
  set_tok_reg (newtok[0], AXP_REG_T9);
  set_tok_sym (newtok[1], sym, 0);
  assemble_tokens ("jsr", newtok, 2, 1);

  /* Reload the GP register */
#ifdef OBJ_AOUT
FIXME
#endif
#if defined(OBJ_ECOFF) || defined(OBJ_ELF)
  set_tok_reg (newtok[0], alpha_gp_register);
  set_tok_const (newtok[1], 0);
  set_tok_preg (newtok[2], AXP_REG_T9);
  assemble_tokens ("ldgp", newtok, 3, 1);
#endif

  /* Move the result to the right place */
  if (rr != AXP_REG_T12)
    {
      set_tok_reg (newtok[0], AXP_REG_T12);
      set_tok_reg (newtok[1], rr);
      assemble_tokens ("mov", newtok, 2, 1);
    }
}

static void
emit_jsrjmp (tok, ntok, vopname)
     const expressionS *tok;
     int ntok;
     void *vopname;
{
  const char *opname = (const char *) vopname;
  struct alpha_insn insn;
  expressionS newtok[3];
  int r, tokidx = 0, lituse = 0;

  if (tokidx < ntok && tok[tokidx].X_op == O_register)
    r = regno (tok[tokidx++].X_add_number);
  else
    r = strcmp (opname, "jmp") == 0 ? AXP_REG_ZERO : AXP_REG_RA;

  set_tok_reg (newtok[0], r);

  if (tokidx < ntok &&
      (tok[tokidx].X_op == O_pregister || tok[tokidx].X_op == O_cpregister))
    r = regno (tok[tokidx++].X_add_number);
  else
    {
      int basereg = alpha_gp_register;
      lituse = load_expression (r = AXP_REG_PV, &tok[tokidx], &basereg, NULL);
    }

  set_tok_cpreg (newtok[1], r);

  if (tokidx < ntok)
    newtok[2] = tok[tokidx];
  else
    set_tok_const (newtok[2], 0);

  assemble_tokens_to_insn (opname, newtok, 3, &insn);

  /* add the LITUSE fixup */
  if (lituse)
    {
      assert (insn.nfixups < MAX_INSN_FIXUPS);
      if (insn.nfixups > 0)
	{
	  memmove (&insn.fixups[1], &insn.fixups[0],
		   sizeof(struct alpha_fixup) * insn.nfixups);
	}
      insn.nfixups++;
      insn.fixups[0].reloc = BFD_RELOC_ALPHA_LITUSE;
      insn.fixups[0].exp.X_op = O_constant;
      insn.fixups[0].exp.X_add_number = 3;
    }

  emit_insn (&insn);
}

static void
emit_retjcr (tok, ntok, vopname)
     const expressionS *tok;
     int ntok;
     void *vopname;
{
  const char *opname = (const char *)vopname;
  expressionS newtok[3];
  int r, tokidx = 0;

  if (tokidx < ntok && tok[tokidx].X_op == O_register)
    r = regno (tok[tokidx++].X_add_number);
  else
    r = AXP_REG_ZERO;

  set_tok_reg (newtok[0], r);

  if (tokidx < ntok &&
      (tok[tokidx].X_op == O_pregister || tok[tokidx].X_op == O_cpregister))
    r = regno (tok[tokidx++].X_add_number);
  else
    r = AXP_REG_RA;

  set_tok_cpreg (newtok[1], r);

  if (tokidx < ntok)
    newtok[2] = tok[tokidx];
  else
    set_tok_const (newtok[2], strcmp(opname, "ret") == 0);

  assemble_tokens (opname, newtok, 3, 0);
}

/* Assembler directives */

/*
 * Handle the .text pseudo-op.  This is like the usual one, but it
 * clears alpha_insn_label and restores auto alignment.
 */

static void
s_alpha_text (i)
     int i;

{
  s_text (i);
  alpha_insn_label = NULL;
  alpha_auto_align_on = 1;
}  

/*
 * Handle the .data pseudo-op.  This is like the usual one, but it
 * clears alpha_insn_label and restores auto alignment. 
 */

static void
s_alpha_data (i)
     int i;
{
  s_data (i);
  alpha_insn_label = NULL;
  alpha_auto_align_on = 1;
}  

#ifndef OBJ_ELF

static void
s_alpha_comm (ignore)
     int ignore;
{
  register char *name;
  register char c;
  register char *p;
  offsetT temp;
  register symbolS *symbolP;

  name = input_line_pointer;
  c = get_symbol_end ();

  /* just after name is now '\0' */
  p = input_line_pointer;
  *p = c;

  SKIP_WHITESPACE ();

  /* Alpha OSF/1 compiler doesn't provide the comma, gcc does.  */
  if (*input_line_pointer == ',')
    {
      input_line_pointer++;
      SKIP_WHITESPACE ();
    }
  if ((temp = get_absolute_expression ()) < 0)
    {
      as_warn (".COMMon length (%ld.) <0! Ignored.", (long) temp);
      ignore_rest_of_line ();
      return;
    }

  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;

  if (S_IS_DEFINED (symbolP))
    {
      as_bad ("Ignoring attempt to re-define symbol");
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

  know (symbolP->sy_frag == &zero_address_frag);
  demand_empty_rest_of_line ();
}

#endif

static void
s_alpha_rdata (ignore)
     int ignore;
{
  int temp;

  temp = get_absolute_expression ();
  subseg_new (".rdata", 0);
  demand_empty_rest_of_line ();
  alpha_insn_label = NULL;
  alpha_auto_align_on = 1;
}

static void
s_alpha_sdata (ignore)
     int ignore;
{
  int temp;

  temp = get_absolute_expression ();
  subseg_new (".sdata", 0);
  demand_empty_rest_of_line ();
  alpha_insn_label = NULL;
  alpha_auto_align_on = 1;
}

static void
s_alpha_gprel32 (ignore)
     int ignore;
{
  expressionS e;
  char *p;

  SKIP_WHITESPACE ();
  expression (&e);

#ifdef OBJ_ELF
  switch (e.X_op)
    {
    case O_constant:
      e.X_add_symbol = section_symbol(absolute_section);
      e.X_op = O_symbol;
      /* FALLTHRU */
    case O_symbol:
      break;
    default:
      abort();
    }
#else
  switch (e.X_op)
    {
    case O_constant:
      e.X_add_symbol = section_symbol (absolute_section);
      /* fall through */
    case O_symbol:
      e.X_op = O_subtract;
      e.X_op_symbol = alpha_gp_symbol;
      break;
    default:
      abort ();
    }
#endif

  if (alpha_auto_align_on)
    alpha_align (2, (char *) NULL, alpha_insn_label);
  alpha_insn_label = NULL;

  p = frag_more (4);
  memset (p, 0, 4);
  fix_new_exp (frag_now, p-frag_now->fr_literal, 4,
	       &e, 0, BFD_RELOC_GPREL32);
}

/*
 * Handle floating point allocation pseudo-ops.  This is like the
 * generic vresion, but it makes sure the current label, if any, is
 * correctly aligned. 
 */

static void
s_alpha_float_cons (type)
     int type;
{
  if (alpha_auto_align_on)
    {
      int log_size;

      switch (type)
	{
	default:
	case 'f':
	case 'F':
	  log_size = 2;
	  break;

	case 'd':
	case 'D':
	case 'G':
	  log_size = 3;
	  break;

	case 'x':
	case 'X':
	case 'p':
	case 'P':
	  log_size = 4;
	  break;
	}

      alpha_align (log_size, (char *) NULL, alpha_insn_label);
    }

  alpha_insn_label = NULL;
  float_cons (type);
}

static void
s_alpha_proc (is_static)
     int is_static;
{
  /* XXXX Align to cache linesize XXXXX */
  char *name;
  char c;
  char *p;
  symbolS *symbolP;
  int temp;

  /* Takes ".proc name,nargs"  */
  name = input_line_pointer;
  c = get_symbol_end ();
  p = input_line_pointer;
  symbolP = symbol_find_or_make (name);
  *p = c;
  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      *p = 0;
      as_warn ("Expected comma after name \"%s\"", name);
      *p = c;
      temp = 0;
      ignore_rest_of_line ();
    }
  else
    {
      input_line_pointer++;
      temp = get_absolute_expression ();
    }
  /*  symbolP->sy_other = (signed char) temp; */
  as_warn ("unhandled: .proc %s,%d", name, temp);
  demand_empty_rest_of_line ();
}

static void
s_alpha_set (x)
     int x;
{
  char *name = input_line_pointer, ch, *s;
  int yesno = 1;

  while (!is_end_of_line[(unsigned char) *input_line_pointer])
    input_line_pointer++;
  ch = *input_line_pointer;
  *input_line_pointer = '\0';

  s = name;
  if (s[0] == 'n' && s[1] == 'o')
    {
      yesno = 0;
      s += 2;
    }
  if (!strcmp ("reorder", s))
    /* ignore */ ;
  else if (!strcmp ("at", s))
    alpha_noat_on = !yesno;
  else if (!strcmp ("macro", s))
    alpha_macros_on = yesno;
  else if (!strcmp ("move", s))
    /* ignore */ ;
  else if (!strcmp ("volatile", s))
    /* ignore */ ;
  else
    as_warn ("Tried to .set unrecognized mode `%s'", name);

  *input_line_pointer = ch;
  demand_empty_rest_of_line ();
}

static void
s_alpha_base (ignore)
     int ignore;
{
#if 0
  if (first_32bit_quadrant)
    {
      /* not fatal, but it might not work in the end */
      as_warn ("File overrides no-base-register option.");
      first_32bit_quadrant = 0;
    }
#endif

  SKIP_WHITESPACE ();
  if (*input_line_pointer == '$')
    {				/* $rNN form */
      input_line_pointer++;
      if (*input_line_pointer == 'r')
	input_line_pointer++;
    }

  alpha_gp_register = get_absolute_expression ();
  if (alpha_gp_register < 0 || alpha_gp_register > 31)
    {
      alpha_gp_register = AXP_REG_GP;
      as_warn ("Bad base register, using $%d.", alpha_gp_register);
    }

  demand_empty_rest_of_line ();
}

/*
 * Handle the .align pseudo-op.  This aligns to a power of two.  It
 * also adjusts any current instruction label.  We treat this the same
 * way the MIPS port does: .align 0 turns off auto alignment.
 */

static void
s_alpha_align (ignore)
     int ignore;
{
  int align;
  char fill, *pfill;
  long max_alignment = 15;

  align = get_absolute_expression ();
  if (align > max_alignment)
    {
      align = max_alignment;
      as_bad ("Alignment too large: %d. assumed", align);
    }
  else if (align < 0)
    {
      as_warn ("Alignment negative: 0 assumed");
      align = 0;
    }

  if (*input_line_pointer == ',')
    {
      input_line_pointer++;
      fill = get_absolute_expression ();
      pfill = &fill;
    }
  else
    pfill = NULL;

  if (align != 0)
    {
      alpha_auto_align_on = 1;
      alpha_align (align, pfill, alpha_insn_label);
    }
  else
    {
      alpha_auto_align_on = 0;
    }

  demand_empty_rest_of_line ();
}

/*
 * Handle data allocation pseudo-ops.  This is like the generic
 * version, but it makes sure the current label, if any, is correctly
 * aligned.
 */

static void
s_alpha_cons (log_size)
     int log_size;
{
  if (alpha_auto_align_on && log_size > 0)
    alpha_align (log_size, (char *) NULL, alpha_insn_label);
  alpha_insn_label = NULL;
  cons (1 << log_size);
}


/* The macro table */

const struct alpha_macro alpha_macros[] = {
/* Load/Store macros */
  { "lda",	emit_lda, NULL,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ldah",	emit_ldah, NULL,
    { MACRO_IR, MACRO_EXP, MACRO_EOA } },

  { "ldl",	emit_ir_load, "ldl",
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ldl_l",	emit_ir_load, "ldl_l",
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ldq",	emit_ir_load, "ldq",
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ldq_l",	emit_ir_load, "ldq_l",
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ldq_u",	emit_ir_load, "ldq_u",
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ldf",	emit_loadstore, "ldf",
    { MACRO_FPR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_FPR, MACRO_EXP, MACRO_EOA } },
  { "ldg",	emit_loadstore, "ldg",
    { MACRO_FPR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_FPR, MACRO_EXP, MACRO_EOA } },
  { "lds",	emit_loadstore, "lds",
    { MACRO_FPR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_FPR, MACRO_EXP, MACRO_EOA } },
  { "ldt",	emit_loadstore, "ldt",
    { MACRO_FPR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_FPR, MACRO_EXP, MACRO_EOA } },

  { "ldb",	emit_ldX, (void *)0,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ldbu",	emit_ldXu, (void *)0,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ldw",	emit_ldX, (void *)1,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ldwu",	emit_ldXu, (void *)1,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },

  { "uldw",	emit_uldX, (void*)1,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "uldwu",	emit_uldXu, (void*)1,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "uldl",	emit_uldX, (void*)2,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "uldlu",	emit_uldXu, (void*)2,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "uldq",	emit_uldXu, (void*)3,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },

  { "ldgp",	emit_ldgp, NULL,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA } },

  { "ldi",	emit_lda, NULL,
    { MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ldil",	emit_ldil, NULL,
    { MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ldiq",	emit_lda, NULL,
    { MACRO_IR, MACRO_EXP, MACRO_EOA } },
#if 0
  { "ldif"	emit_ldiq, NULL,
    { MACRO_FPR, MACRO_EXP, MACRO_EOA } },
  { "ldid"	emit_ldiq, NULL,
    { MACRO_FPR, MACRO_EXP, MACRO_EOA } },
  { "ldig"	emit_ldiq, NULL,
    { MACRO_FPR, MACRO_EXP, MACRO_EOA } },
  { "ldis"	emit_ldiq, NULL,
    { MACRO_FPR, MACRO_EXP, MACRO_EOA } },
  { "ldit"	emit_ldiq, NULL,
    { MACRO_FPR, MACRO_EXP, MACRO_EOA } },
#endif

  { "stl",	emit_loadstore, "stl",
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "stl_c",	emit_loadstore, "stl_c",
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "stq",	emit_loadstore, "stq",
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "stq_c",	emit_loadstore, "stq_c",
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "stq_u",	emit_loadstore, "stq_u",
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "stf",	emit_loadstore, "stf",
    { MACRO_FPR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_FPR, MACRO_EXP, MACRO_EOA } },
  { "stg",	emit_loadstore, "stg",
    { MACRO_FPR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_FPR, MACRO_EXP, MACRO_EOA } },
  { "sts",	emit_loadstore, "sts",
    { MACRO_FPR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_FPR, MACRO_EXP, MACRO_EOA } },
  { "stt",	emit_loadstore, "stt",
    { MACRO_FPR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_FPR, MACRO_EXP, MACRO_EOA } },

  { "stb",	emit_stX, (void*)0,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "stw",	emit_stX, (void*)1,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ustw",	emit_ustX, (void*)1,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ustl",	emit_ustX, (void*)2,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },
  { "ustq",	emit_ustX, (void*)3,
    { MACRO_IR, MACRO_EXP, MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA } },

/* Arithmetic macros */
#if 0
  { "absl"	emit_absl, 1, { IR } },
  { "absl"	emit_absl, 2, { IR, IR } },
  { "absl"	emit_absl, 2, { EXP, IR } },
  { "absq"	emit_absq, 1, { IR } },
  { "absq"	emit_absq, 2, { IR, IR } },
  { "absq"	emit_absq, 2, { EXP, IR } },
#endif

  { "sextb",	emit_sextX, (void *)0,
    { MACRO_IR, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_EOA,
      /* MACRO_EXP, MACRO_IR, MACRO_EOA */ } },
  { "sextw",	emit_sextX, (void *)1,
    { MACRO_IR, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_EOA,
      /* MACRO_EXP, MACRO_IR, MACRO_EOA */ } },

  { "divl",	emit_division, "__divl",
    { MACRO_IR, MACRO_IR, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_IR, MACRO_EOA,
      /* MACRO_IR, MACRO_EXP, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA */ } },
  { "divlu",	emit_division, "__divlu",
    { MACRO_IR, MACRO_IR, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_IR, MACRO_EOA,
      /* MACRO_IR, MACRO_EXP, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA */ } },
  { "divq",	emit_division, "__divq",
    { MACRO_IR, MACRO_IR, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_IR, MACRO_EOA,
      /* MACRO_IR, MACRO_EXP, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA */ } },
  { "divqu",	emit_division, "__divqu",
    { MACRO_IR, MACRO_IR, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_IR, MACRO_EOA,
      /* MACRO_IR, MACRO_EXP, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA */ } },
  { "reml",	emit_division, "__reml",
    { MACRO_IR, MACRO_IR, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_IR, MACRO_EOA,
      /* MACRO_IR, MACRO_EXP, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA */ } },
  { "remlu",	emit_division, "__remlu",
    { MACRO_IR, MACRO_IR, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_IR, MACRO_EOA,
      /* MACRO_IR, MACRO_EXP, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA */ } },
  { "remq",	emit_division, "__remq",
    { MACRO_IR, MACRO_IR, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_IR, MACRO_EOA,
      /* MACRO_IR, MACRO_EXP, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA */ } },
  { "remqu",	emit_division, "__remqu",
    { MACRO_IR, MACRO_IR, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_IR, MACRO_EOA,
      /* MACRO_IR, MACRO_EXP, MACRO_IR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA */ } },

  { "jsr",	emit_jsrjmp, "jsr",
    { MACRO_PIR, MACRO_EXP, MACRO_EOA,
      MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA,
      MACRO_EXP, MACRO_EOA } },
  { "jmp",	emit_jsrjmp, "jmp",
    { MACRO_PIR, MACRO_EXP, MACRO_EOA,
      MACRO_PIR, MACRO_EOA,
      MACRO_IR, MACRO_EXP, MACRO_EOA,
      MACRO_EXP, MACRO_EOA } },
  { "ret",	emit_retjcr, "ret",
    { MACRO_IR, MACRO_EXP, MACRO_EOA,
      MACRO_IR, MACRO_EOA,
      MACRO_PIR, MACRO_EXP, MACRO_EOA,
      MACRO_PIR, MACRO_EOA,
      MACRO_EXP, MACRO_EOA,
      MACRO_EOA } },
  { "jcr",	emit_retjcr, "jcr",
    { MACRO_IR, MACRO_EXP, MACRO_EOA,
      MACRO_IR, MACRO_EOA,
      MACRO_PIR, MACRO_EXP, MACRO_EOA,
      MACRO_PIR, MACRO_EOA,
      MACRO_EXP, MACRO_EOA,
      MACRO_EOA } },
  { "jsr_coroutine",	emit_retjcr, "jcr",
    { MACRO_IR, MACRO_EXP, MACRO_EOA,
      MACRO_IR, MACRO_EOA,
      MACRO_PIR, MACRO_EXP, MACRO_EOA,
      MACRO_PIR, MACRO_EOA,
      MACRO_EXP, MACRO_EOA,
      MACRO_EOA } },
};

static const int alpha_num_macros
  = sizeof(alpha_macros) / sizeof(*alpha_macros);

/* The target specific pseudo-ops which we support.  */

const pseudo_typeS md_pseudo_table[] =
{
  {"common", s_comm, 0},	/* is this used? */
#ifndef OBJ_ELF
  {"comm", s_alpha_comm, 0},	/* osf1 compiler does this */
#endif
  {"text", s_alpha_text, 0},
  {"data", s_alpha_data, 0},
  {"rdata", s_alpha_rdata, 0},
  {"sdata", s_alpha_sdata, 0},
  {"gprel32", s_alpha_gprel32, 0},
  {"t_floating", s_alpha_float_cons, 'd'},
  {"s_floating", s_alpha_float_cons, 'f'},
  {"f_floating", s_alpha_float_cons, 'F'},
  {"g_floating", s_alpha_float_cons, 'G'},
  {"d_floating", s_alpha_float_cons, 'D'},

  {"proc", s_alpha_proc, 0},
  {"aproc", s_alpha_proc, 1},
  {"set", s_alpha_set, 0},
  {"reguse", s_ignore, 0},
  {"livereg", s_ignore, 0},
  {"base", s_alpha_base, 0},		/*??*/
  {"option", s_ignore, 0},
  {"prologue", s_ignore, 0},
  {"aent", s_ignore, 0},
  {"ugen", s_ignore, 0},
  {"eflag", s_ignore, 0},

  {"align", s_alpha_align, 0},
  {"byte", s_alpha_cons, 0},
  {"hword", s_alpha_cons, 1},
  {"int", s_alpha_cons, 2},
  {"long", s_alpha_cons, 2},
  {"octa", s_alpha_cons, 4},
  {"quad", s_alpha_cons, 3},
  {"short", s_alpha_cons, 1},
  {"word", s_alpha_cons, 1},
  {"double", s_alpha_float_cons, 'd'},
  {"float", s_alpha_float_cons, 'f'},
  {"single", s_alpha_float_cons, 'f'},

/* We don't do any optimizing, so we can safely ignore these.  */
  {"noalias", s_ignore, 0},
  {"alias", s_ignore, 0},

  {NULL, 0, 0},
};



static void
create_literal_section (name, secp, symp)
     const char *name;
     segT *secp;
     symbolS **symp;
{
  segT current_section = now_seg;
  int current_subsec = now_subseg;
  segT new_sec;

  *secp = new_sec = subseg_new (name, 0);
  subseg_set (current_section, current_subsec);
  bfd_set_section_alignment (stdoutput, new_sec, 4);
  bfd_set_section_flags (stdoutput, new_sec,
			 SEC_RELOC | SEC_ALLOC | SEC_LOAD | SEC_READONLY
			 | SEC_DATA);

  S_CLEAR_EXTERNAL (*symp = section_symbol (new_sec));
}

#ifndef OBJ_ELF

static inline void
maybe_set_gp (sec)
     asection *sec;
{
  bfd_vma vma;
  if (!sec)
    return;
  vma = bfd_get_section_vma (foo, sec);
  if (vma && vma < alpha_gp_value)
    alpha_gp_value = vma;
}

static void
select_gp_value ()
{
  assert (alpha_gp_value == 0);

  /* Get minus-one in whatever width...  */
  alpha_gp_value = 0; alpha_gp_value--;

  /* Select the smallest VMA of these existing sections.  */
  maybe_set_gp (alpha_lita_section);
/* maybe_set_gp (sdata);   Was disabled before -- should we use it?  */
#if 0
  maybe_set_gp (lit8_sec);
  maybe_set_gp (lit4_sec);
#endif

/* @@ Will a simple 0x8000 work here?  If not, why not?  */
#define GP_ADJUSTMENT	(0x8000 - 0x10)

  alpha_gp_value += GP_ADJUSTMENT;

  S_SET_VALUE (alpha_gp_symbol, alpha_gp_value);

#ifdef DEBUG1
  printf ("Chose GP value of %lx\n", alpha_gp_value);
#endif
}
#endif /* !OBJ_ELF */

static void
alpha_align (n, pfill, label)
     int n;
     char *pfill;
     symbolS *label;
{
  if (pfill == NULL)
    {
      if (n > 2
	  && (bfd_get_section_flags (stdoutput, now_seg) & SEC_CODE) != 0)
	{
	  static char const nop[4] = { 0x1f, 0x04, 0xff, 0x47 };

	  /* First, make sure we're on a four-byte boundary, in case
	     someone has been putting .byte values into the text
	     section.  The DEC assembler silently fills with unaligned
	     no-op instructions.  This will zero-fill, then nop-fill
	     with proper alignment.  */
	  frag_align (2, 0);
	  frag_align_pattern (n, nop, sizeof nop);
	}
      else
	frag_align (n, 0);
    }
  else
    frag_align (n, *pfill);

  if (label != NULL)
    {
      assert (S_GET_SEGMENT (label) == now_seg);
      label->sy_frag = frag_now;
      S_SET_VALUE (label, (valueT) frag_now_fix ());
    }

  record_alignment(now_seg, n);
}

/* The Alpha has support for some VAX floating point types, as well as for
   IEEE floating point.  We consider IEEE to be the primary floating point
   format, and sneak in the VAX floating point support here.  */
#define md_atof vax_md_atof
#include "config/atof-vax.c"
