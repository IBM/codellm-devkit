/* alpha-opc.c -- Alpha AXP opcode list
   Copyright 1996 Free Software Foundation, Inc.
   Contributed by Richard Henderson <rth@tamu.edu>,
   patterned after the PPC opcode handling written by Ian Lance Taylor.

   This file is part of GDB, GAS, and the GNU binutils.

   GDB, GAS, and the GNU binutils are free software; you can redistribute
   them and/or modify them under the terms of the GNU General Public
   License as published by the Free Software Foundation; either version
   2, or (at your option) any later version.

   GDB, GAS, and the GNU binutils are distributed in the hope that they
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#include <stdio.h>
#include "ansidecl.h"
#include "opcode/alpha.h"

/* This file holds the Alpha AXP opcode table.  The opcode table
   includes almost all of the extended instruction mnemonics.  This
   permits the disassembler to use them, and simplifies the assembler
   logic, at the cost of increasing the table size.  The table is
   strictly constant data, so the compiler should be able to put it in
   the .text section.

   This file also holds the operand table.  All knowledge about
   inserting operands into instructions and vice-versa is kept in this
   file.  */

/* Local insertion and extraction functions */

static unsigned insert_rba PARAMS((unsigned, int, const char **));
static unsigned insert_rca PARAMS((unsigned, int, const char **));
static unsigned insert_za PARAMS((unsigned, int, const char **));
static unsigned insert_zb PARAMS((unsigned, int, const char **));
static unsigned insert_zc PARAMS((unsigned, int, const char **));
static unsigned insert_bdisp PARAMS((unsigned, int, const char **));
static unsigned insert_jhint PARAMS((unsigned, int, const char **));

static int extract_rba PARAMS((unsigned, int *));
static int extract_rca PARAMS((unsigned, int *));
static int extract_za PARAMS((unsigned, int *));
static int extract_zb PARAMS((unsigned, int *));
static int extract_zc PARAMS((unsigned, int *));
static int extract_bdisp PARAMS((unsigned, int *));
static int extract_jhint PARAMS((unsigned, int *));


/* The operands table  */

const struct alpha_operand alpha_operands[] = 
{
  /* The fields are bits, shift, insert, extract, flags */
  /* The zero index is used to indicate end-of-list */
#define UNUSED		0
  { 0, 0, BFD_RELOC_UNUSED, 0, 0 },

  /* The plain integer register fields */
#define RA		(UNUSED + 1)
  { 5, 21, BFD_RELOC_UNUSED, 0, 0, AXP_OPERAND_IR },
#define RB		(RA + 1)
  { 5, 16, BFD_RELOC_UNUSED, 0, 0, AXP_OPERAND_IR },
#define RC		(RB + 1)
  { 5, 0, BFD_RELOC_UNUSED, 0, 0, AXP_OPERAND_IR },

  /* The plain fp register fields */
#define FA		(RC + 1)
  { 5, 21, BFD_RELOC_UNUSED, 0, 0, AXP_OPERAND_FPR },
#define FB		(FA + 1)
  { 5, 16, BFD_RELOC_UNUSED, 0, 0, AXP_OPERAND_FPR },
#define FC		(FB + 1)
  { 5, 0, BFD_RELOC_UNUSED, 0, 0, AXP_OPERAND_FPR },

  /* The integer registers when they are ZERO */
#define ZA		(FC + 1)
  { 5, 21, BFD_RELOC_UNUSED, insert_za, extract_za, AXP_OPERAND_FAKE },
#define ZB		(ZA + 1)
  { 5, 16, BFD_RELOC_UNUSED, insert_zb, extract_zb, AXP_OPERAND_FAKE },
#define ZC		(ZB + 1)
  { 5, 0, BFD_RELOC_UNUSED, insert_zc, extract_zc, AXP_OPERAND_FAKE },

  /* The RB field when it needs parentheses */
#define PRB		(ZC + 1)
  { 5, 16, BFD_RELOC_UNUSED, 0, 0, AXP_OPERAND_IR|AXP_OPERAND_PARENS },

  /* The RB field when it needs parentheses _and_ a preceding comma */
#define CPRB		(PRB + 1)
  { 5, 16, BFD_RELOC_UNUSED, 0, 0,
    AXP_OPERAND_IR|AXP_OPERAND_PARENS|AXP_OPERAND_COMMA },

  /* The RB field when it must be the same as the RA field */
#define RBA		(CPRB + 1)
  { 5, 16, BFD_RELOC_UNUSED, insert_rba, extract_rba, AXP_OPERAND_FAKE },

  /* The RC field when it must be the same as the RB field */
#define RCA		(RBA + 1)
  { 5, 0, BFD_RELOC_UNUSED, insert_rca, extract_rca, AXP_OPERAND_FAKE },

  /* The RC field when it can *default* to RA */
#define DRC1		(RCA + 1)
  { 5, 0, BFD_RELOC_UNUSED, 0, 0,
    AXP_OPERAND_IR|AXP_OPERAND_DEFAULT_FIRST },

  /* The RC field when it can *default* to RB */
#define DRC2		(DRC1 + 1)
  { 5, 0, BFD_RELOC_UNUSED, 0, 0,
    AXP_OPERAND_IR|AXP_OPERAND_DEFAULT_SECOND },

  /* The FC field when it can *default* to RA */
#define DFC1		(DRC2 + 1)
  { 5, 0, BFD_RELOC_UNUSED, 0, 0,
    AXP_OPERAND_FPR|AXP_OPERAND_DEFAULT_FIRST },
  
  /* The FC field when it can *default* to RB */
#define DFC2		(DFC1 + 1)
  { 5, 0, BFD_RELOC_UNUSED, 0, 0,
    AXP_OPERAND_FPR|AXP_OPERAND_DEFAULT_SECOND },
  
  /* The unsigned 8-bit literal of Operate format insns */
#define LIT		(DFC2 + 1)
  { 8, 13, BFD_RELOC_UNUSED+LIT, 0, 0, AXP_OPERAND_UNSIGNED },

  /* The signed 16-bit displacement of Memory format insns.  From here
     we can't tell what relocation should be used, so don't use a default. */
#define MDISP		(LIT + 1)
  { 16, 0, BFD_RELOC_UNUSED+MDISP, 0, 0, AXP_OPERAND_SIGNED },

  /* The signed "23-bit" aligned displacement of Branch format insns */
#define BDISP		(MDISP + 1)
  { 21, 0, BFD_RELOC_23_PCREL_S2, insert_bdisp, extract_bdisp,
    AXP_OPERAND_RELATIVE },

  /* The 26-bit PALcode function */
#define PALFN		(BDISP + 1)
  { 26, 0, BFD_RELOC_UNUSED+PALFN, 0, 0, AXP_OPERAND_UNSIGNED },

  /* The optional signed "16-bit" aligned displacement of the JMP/JSR hint */
#define JMPHINT		(PALFN + 1)
  { 14, 0, BFD_RELOC_ALPHA_HINT, insert_jhint, extract_jhint,
    AXP_OPERAND_RELATIVE|AXP_OPERAND_DEFAULT_ZERO|AXP_OPERAND_NOOVERFLOW },

  /* The optional hint to RET/JSR_COROUTINE */
#define RETHINT		(JMPHINT + 1)
  { 14, 0, BFD_RELOC_UNUSED+RETHINT, 0, 0,
    AXP_OPERAND_UNSIGNED|AXP_OPERAND_DEFAULT_ZERO },

  /* The 12-bit displacement for the ev4 hw_{ld,st} (pal1b/pal1f) insns */
#define EV4HWDISP	(RETHINT + 1)
  { 12, 0, BFD_RELOC_UNUSED+EV4HWDISP, 0, 0, AXP_OPERAND_SIGNED },

  /* The 5-bit index for the ev4 hw_m[ft]pr (pal19/pal1d) insns */
#define EV4HWINDEX	(EV4HWDISP + 1)
  { 5, 0, BFD_RELOC_UNUSED+EV4HWINDEX, 0, 0, AXP_OPERAND_UNSIGNED },

  /* The 8-bit index for the oddly unqualified hw_m[tf]pr insns
     that occur in DEC PALcode.  */
#define EV4EXTHWINDEX	(EV4HWINDEX + 1)
  { 5, 0, BFD_RELOC_UNUSED+EV4EXTHWINDEX, 0, 0, AXP_OPERAND_UNSIGNED },

  /* The 10-bit displacement for the ev5 hw_{ld,st} (pal1b/pal1f) insns */
#define EV5HWDISP	(EV4EXTHWINDEX + 1)
  { 10, 0, BFD_RELOC_UNUSED+EV5HWDISP, 0, 0, AXP_OPERAND_SIGNED },

  /* The 16-bit index for the ev5 hw_m[ft]pr (pal19/pal1d) insns */
#define EV5HWINDEX	(EV5HWDISP + 1)
  { 16, 0, BFD_RELOC_UNUSED+EV5HWINDEX, 0, 0, AXP_OPERAND_UNSIGNED },
};

const int alpha_num_operands = sizeof(alpha_operands)/sizeof(*alpha_operands);

/* The RB field when it is the same as the RA field in the same insn.
   This operand is marked fake.  The insertion function just copies
   the RA field into the RB field, and the extraction function just
   checks that the fields are the same. */

/*ARGSUSED*/
static unsigned
insert_rba(insn, value, errmsg)
     unsigned insn;
     int value;
     const char **errmsg;
{
  return insn | (((insn >> 21) & 0x1f) << 16);
}

static int
extract_rba(insn, invalid)
     unsigned insn;
     int *invalid;
{
  if (invalid != (int *) NULL
      && ((insn >> 21) & 0x1f) != ((insn >> 16) & 0x1f))
    *invalid = 1;
  return 0;
}


/* The same for the RC field */

/*ARGSUSED*/
static unsigned
insert_rca(insn, value, errmsg)
     unsigned insn;
     int value;
     const char **errmsg;
{
  return insn | ((insn >> 21) & 0x1f);
}

static int
extract_rca(insn, invalid)
     unsigned insn;
     int *invalid;
{
  if (invalid != (int *) NULL
      && ((insn >> 21) & 0x1f) != (insn & 0x1f))
    *invalid = 1;
  return 0;
}


/* Fake arguments in which the registers must be set to ZERO */

/*ARGSUSED*/
static unsigned
insert_za(insn, value, errmsg)
     unsigned insn;
     int value;
     const char **errmsg;
{
  return insn | (31 << 21);
}

static int
extract_za(insn, invalid)
     unsigned insn;
     int *invalid;
{
  if (invalid != (int *) NULL && ((insn >> 21) & 0x1f) != 31)
    *invalid = 1;
  return 0;
}

/*ARGSUSED*/
static unsigned
insert_zb(insn, value, errmsg)
     unsigned insn;
     int value;
     const char **errmsg;
{
  return insn | (31 << 16);
}

static int
extract_zb(insn, invalid)
     unsigned insn;
     int *invalid;
{
  if (invalid != (int *) NULL && ((insn >> 16) & 0x1f) != 31)
    *invalid = 1;
  return 0;
}

/*ARGSUSED*/
static unsigned
insert_zc(insn, value, errmsg)
     unsigned insn;
     int value;
     const char **errmsg;
{
  return insn | 31;
}

static int
extract_zc(insn, invalid)
     unsigned insn;
     int *invalid;
{
  if (invalid != (int *) NULL && (insn & 0x1f) != 31)
    *invalid = 1;
  return 0;
}


/* The displacement field of a Branch format insn.  */

static unsigned
insert_bdisp(insn, value, errmsg)
     unsigned insn;
     int value;
     const char **errmsg;
{
  if (errmsg != (const char **)NULL && (value & 3))
    *errmsg = "branch operand unaligned";
  return insn | ((value / 4) & 0x1FFFFF);
}

/*ARGSUSED*/
static int
extract_bdisp(insn, invalid)
     unsigned insn;
     int *invalid;
{
  return 4 * (((insn & 0x1FFFFF) ^ 0x100000) - 0x100000);
}


/* The hint field of a JMP/JSR insn.  */

static unsigned
insert_jhint(insn, value, errmsg)
     unsigned insn;
     int value;
     const char **errmsg;
{
  if (errmsg != (const char **)NULL && (value & 3))
    *errmsg = "jump hint unaligned";
  return insn | ((value / 4) & 0xFFFF);
}

/*ARGSUSED*/
static int
extract_jhint(insn, invalid)
     unsigned insn;
     int *invalid;
{
  return 4 * (((insn & 0x3FFF) ^ 0x2000) - 0x2000);
}


/* Macros used to form opcodes */

/* The main opcode */
#define OP(x)		(((x) & 0x3F) << 26)
#define OP_MASK		0xFC000000

/* Branch format instructions */
#define BRA_(oo)	OP(oo)
#define BRA_MASK	OP_MASK
#define BRA(oo)		BRA_(oo), BRA_MASK

/* Floating point format instructions */
#define FP_(oo,fff)	(OP(oo) | (((fff) & 0x7FF) << 5))
#define FP_MASK		(OP_MASK | 0xFFE0)
#define FP(oo,fff)	FP_(oo,fff), FP_MASK

/* Memory format instructions */
#define MEM_(oo)	OP(oo)
#define MEM_MASK	OP_MASK
#define MEM(oo)		MEM_(oo), MEM_MASK

/* Memory/Func Code format instructions */
#define MFC_(oo,ffff)	(OP(oo) | ((ffff) & 0xFFFF))
#define MFC_MASK	(OP_MASK | 0xFFFF)
#define MFC(oo,ffff)	MFC_(oo,ffff), MFC_MASK

/* Memory/Branch format instructions */
#define MBR_(oo,h)	(OP(oo) | (((h) & 3) << 14))
#define MBR_MASK	(OP_MASK | 0xC000)
#define MBR(oo,h)	MBR_(oo,h), MBR_MASK

/* Operate format instructions.  The OPRL variant specifies a
   literal second argument. */
#define OPR_(oo,ff)	(OP(oo) | (((ff) & 0x7F) << 5))
#define OPRL_(oo,ff)	(OPR_((oo),(ff)) | 0x1000)
#define OPR_MASK	(OP_MASK | 0x1FE0)
#define OPR(oo,ff)	OPR_(oo,ff), OPR_MASK
#define OPRL(oo,ff)	OPRL_(oo,ff), OPR_MASK

/* Generic PALcode format instructions */
#define PCD_(oo)	OP(oo)
#define PCD_MASK	OP_MASK
#define PCD(oo)		PCD_(oo), PCD_MASK

/* Specific PALcode instructions */
#define SPCD_(oo,ffff)	(OP(oo) | ((ffff) & 0x3FFFFFF))
#define SPCD_MASK	0xFFFFFFFF
#define SPCD(oo,ffff)	SPCD_(oo,ffff), SPCD_MASK

/* Hardware memory (hw_{ld,st}) instructions */
#define EV4HWMEM_(oo,f)	(OP(oo) | (((f) & 0xF) << 12))
#define EV4HWMEM_MASK	(OP_MASK | 0xF000)
#define EV4HWMEM(oo,f)	EV4HWMEM_(oo,f), EV4HWMEM_MASK

#define EV5HWMEM_(oo,f)	(OP(oo) | (((f) & 0x3F) << 10))
#define EV5HWMEM_MASK	(OP_MASK | 0xF800)
#define EV5HWMEM(oo,f)	EV5HWMEM_(oo,f), EV5HWMEM_MASK

/* Common combinations of flags and arguments */
#define ALL			AXP_OPCODE_ALL
#define EV4			AXP_OPCODE_EV4
#define EV5			AXP_OPCODE_EV5
#define EV56			AXP_OPCODE_EV56
#define EV5x			EV5|EV56

#define ARG_NONE		{ 0 }
#define ARG_BRA			{ RA, BDISP }
#define ARG_FBRA		{ FA, BDISP }
#define ARG_FP			{ FA, FB, DFC1 }
#define ARG_FPZ1		{ ZA, FB, DFC1 }
#define ARG_MEM			{ RA, MDISP, PRB }
#define ARG_FMEM		{ FA, MDISP, PRB }
#define ARG_OPR			{ RA, RB, DRC1 }
#define ARG_OPRL		{ RA, LIT, DRC1 }
#define ARG_OPRZ1		{ ZA, RB, DRC1 }
#define ARG_OPRLZ1		{ ZA, LIT, RC }
#define ARG_PCD			{ PALFN }
#define ARG_EV4HWMEM		{ RA, EV4HWDISP, PRB }
#define ARG_EV4HWMPR		{ RA, RBA, EV4HWINDEX }
#define ARG_EV5HWMEM		{ RA, EV5HWDISP, PRB }


/* The opcode table.

   The format of the opcode table is:
 
   NAME OPCODE MASK { OPERANDS }

   NAME		is the name of the instruction.

   OPCODE	is the instruction opcode.

   MASK		is the opcode mask; this is used to tell the disassembler
            	which bits in the actual opcode must match OPCODE.

   OPERANDS	is the list of operands.

   The preceding macros merge the text of the OPCODE and MASK fields.

   The disassembler reads the table in order and prints the first
   instruction which matches, so this table is sorted to put more
   specific instructions before more general instructions. 

   Otherwise, it is sorted by major opcode and minor function code.

   There are three classes of not-really-instructions in this table:

   ALIAS	is another name for another instruction.  Some of
		these come from the Architecture Handbook, some
		come from the original gas opcode tables.  In all
		cases, the functionality of the opcode is unchanged.

   PSEUDO	a stylized code form endorsed by Chapter A.4 of the
		Architecture Handbook.

   EXTRA	a stylized code form found in the original gas tables.

   XXX: Can anyone shed light on the pal{19,1b,1d,1e,1f} opcodes?
   XXX: Do we want to conditionally compile complete sets of the
        PALcodes described in the Architecture Handbook?
*/

const struct alpha_opcode alpha_opcodes[] = {
  { "halt",		SPCD(0x00,0x0000), ALL, ARG_NONE },
  { "draina",		SPCD(0x00,0x0002), ALL, ARG_NONE },
  { "bpt",		SPCD(0x00,0x0080), ALL, ARG_NONE },
  { "callsys",		SPCD(0x00,0x0083), ALL, ARG_NONE },
  { "chmk", 		SPCD(0x00,0x0083), ALL, ARG_NONE },
  { "imb",		SPCD(0x00,0x0086), ALL, ARG_NONE },
  { "call_pal",		PCD(0x00), ALL, ARG_PCD },
  { "pal",		PCD(0x00), ALL, ARG_PCD },		/* alias */

  { "lda",		MEM(0x08), ALL, ARG_MEM },
  { "ldah",		MEM(0x09), ALL, ARG_MEM },
  { "unop",		MEM(0x0B), ALL, { ZA } },		/* pseudo */
  { "ldq_u",		MEM(0x0B), ALL, ARG_MEM },
  { "stq_u",		MEM(0x0F), ALL, ARG_MEM },

  { "sextl",		OPR(0x10,0x00), ALL, ARG_OPRZ1 },	/* pseudo */
  { "sextl",		OPRL(0x10,0x00), ALL, ARG_OPRLZ1 },	/* pseudo */
  { "addl",		OPR(0x10,0x00), ALL, ARG_OPR },
  { "addl",		OPRL(0x10,0x00), ALL, ARG_OPRL },
  { "s4addl",		OPR(0x10,0x02), ALL, ARG_OPR },
  { "s4addl",		OPRL(0x10,0x02), ALL, ARG_OPRL },
  { "negl",		OPR(0x10,0x09), ALL, ARG_OPRZ1 },	/* pseudo */
  { "negl",		OPRL(0x10,0x09), ALL, ARG_OPRLZ1 },	/* pseudo */
  { "subl",		OPR(0x10,0x09), ALL, ARG_OPR },
  { "subl",		OPRL(0x10,0x09), ALL, ARG_OPRL },
  { "s4subl",		OPR(0x10,0x0B), ALL, ARG_OPR },
  { "s4subl",		OPRL(0x10,0x0B), ALL, ARG_OPRL },
  { "cmpbge",		OPR(0x10,0x0F), ALL, ARG_OPR },
  { "cmpbge",		OPRL(0x10,0x0F), ALL, ARG_OPRL },
  { "s8addl",		OPR(0x10,0x12), ALL, ARG_OPR },
  { "s8addl",		OPRL(0x10,0x12), ALL, ARG_OPRL },
  { "s8subl",		OPR(0x10,0x1B), ALL, ARG_OPR },
  { "s8subl",		OPRL(0x10,0x1B), ALL, ARG_OPRL },
  { "cmpult",		OPR(0x10,0x1D), ALL, ARG_OPR },
  { "cmpult",		OPRL(0x10,0x1D), ALL, ARG_OPRL },
  { "addq",		OPR(0x10,0x20), ALL, ARG_OPR },
  { "addq",		OPRL(0x10,0x20), ALL, ARG_OPRL },
  { "s4addq",		OPR(0x10,0x22), ALL, ARG_OPR },
  { "s4addq",		OPRL(0x10,0x22), ALL, ARG_OPRL },
  { "negq", 		OPR(0x10,0x29), ALL, ARG_OPRZ1 },	/* pseudo */
  { "negq", 		OPRL(0x10,0x29), ALL, ARG_OPRLZ1 },	/* pseudo */
  { "subq",		OPR(0x10,0x29), ALL, ARG_OPR },
  { "subq",		OPRL(0x10,0x29), ALL, ARG_OPRL },
  { "s4subq",		OPR(0x10,0x2B), ALL, ARG_OPR },
  { "s4subq",		OPRL(0x10,0x2B), ALL, ARG_OPRL },
  { "cmpeq",		OPR(0x10,0x2D), ALL, ARG_OPR },
  { "cmpeq",		OPRL(0x10,0x2D), ALL, ARG_OPRL },
  { "s8addq",		OPR(0x10,0x32), ALL, ARG_OPR },
  { "s8addq",		OPRL(0x10,0x32), ALL, ARG_OPRL },
  { "s8subq",		OPR(0x10,0x3B), ALL, ARG_OPR },
  { "s8subq",		OPRL(0x10,0x3B), ALL, ARG_OPRL },
  { "cmpule",		OPR(0x10,0x3D), ALL, ARG_OPR },
  { "cmpule",		OPRL(0x10,0x3D), ALL, ARG_OPRL },
  { "addl/v",		OPR(0x10,0x40), ALL, ARG_OPR },
  { "addl/v",		OPRL(0x10,0x40), ALL, ARG_OPRL },
  { "negl/v",		OPR(0x10,0x49), ALL, ARG_OPRZ1 },	/* pseudo */
  { "negl/v",		OPRL(0x10,0x49), ALL, ARG_OPRLZ1 },	/* pseudo */
  { "subl/v",		OPR(0x10,0x49), ALL, ARG_OPR },
  { "subl/v",		OPRL(0x10,0x49), ALL, ARG_OPRL },
  { "cmplt",		OPR(0x10,0x4D), ALL, ARG_OPR },
  { "cmplt",		OPRL(0x10,0x4D), ALL, ARG_OPRL },
  { "addq/v",		OPR(0x10,0x60), ALL, ARG_OPR },
  { "addq/v",		OPRL(0x10,0x60), ALL, ARG_OPRL },
  { "negq/v",		OPR(0x10,0x69), ALL, ARG_OPRZ1 },	/* pseudo */
  { "negq/v",		OPRL(0x10,0x69), ALL, ARG_OPRLZ1 },	/* pseudo */
  { "subq/v",		OPR(0x10,0x69), ALL, ARG_OPR },
  { "subq/v",		OPRL(0x10,0x69), ALL, ARG_OPRL },
  { "cmple",		OPR(0x10,0x6D), ALL, ARG_OPR },
  { "cmple",		OPRL(0x10,0x6D), ALL, ARG_OPRL },

  { "and",		OPR(0x11,0x00), ALL, ARG_OPR },
  { "and",		OPRL(0x11,0x00), ALL, ARG_OPRL },
  { "andnot",		OPR(0x11,0x08), ALL, ARG_OPR },		/* alias */
  { "andnot",		OPRL(0x11,0x08), ALL, ARG_OPRL },	/* alias */
  { "bic",		OPR(0x11,0x08), ALL, ARG_OPR },
  { "bic",		OPRL(0x11,0x08), ALL, ARG_OPRL },
  { "cmovlbs",		OPR(0x11,0x14), ALL, ARG_OPR },
  { "cmovlbs",		OPRL(0x11,0x14), ALL, ARG_OPRL },
  { "cmovlbc",		OPR(0x11,0x16), ALL, ARG_OPR },
  { "cmovlbc",		OPRL(0x11,0x16), ALL, ARG_OPRL },
  { "nop",		OPR(0x11,0x20), ALL, { ZA, ZB, ZC } }, /* pseudo */
  { "clr",		OPR(0x11,0x20), ALL, { ZA, ZB, RC } }, /* pseudo */
  { "mov",		OPR(0x11,0x20), ALL, { ZA, RB, RC } }, /* pseudo */
  { "mov",		OPR(0x11,0x20), ALL, { RA, RBA, RC } }, /* pseudo */
  { "mov",		OPRL(0x11,0x20), ALL, { ZA, LIT, RC } }, /* pseudo */
  { "or",		OPR(0x11,0x20), ALL, ARG_OPR },		/* alias */
  { "or",		OPRL(0x11,0x20), ALL, ARG_OPRL },	/* alias */
  { "bis",		OPR(0x11,0x20), ALL, ARG_OPR },
  { "bis",		OPRL(0x11,0x20), ALL, ARG_OPRL },
  { "cmoveq",		OPR(0x11,0x24), ALL, ARG_OPR },
  { "cmoveq",		OPRL(0x11,0x24), ALL, ARG_OPRL },
  { "cmovne",		OPR(0x11,0x26), ALL, ARG_OPR },
  { "cmovne",		OPRL(0x11,0x26), ALL, ARG_OPRL },
  { "not",		OPR(0x11,0x28), ALL, ARG_OPRZ1 },	/* pseudo */
  { "not",		OPRL(0x11,0x28), ALL, ARG_OPRLZ1 },	/* pseudo */
  { "ornot",		OPR(0x11,0x28), ALL, ARG_OPR },
  { "ornot",		OPRL(0x11,0x28), ALL, ARG_OPRL },
  { "xor",		OPR(0x11,0x40), ALL, ARG_OPR },
  { "xor",		OPRL(0x11,0x40), ALL, ARG_OPRL },
  { "cmovlt",		OPR(0x11,0x44), ALL, ARG_OPR },
  { "cmovlt",		OPRL(0x11,0x44), ALL, ARG_OPRL },
  { "cmovge",		OPR(0x11,0x46), ALL, ARG_OPR },
  { "cmovge",		OPRL(0x11,0x46), ALL, ARG_OPRL },
  { "eqv",		OPR(0x11,0x48), ALL, ARG_OPR },
  { "eqv",		OPRL(0x11,0x48), ALL, ARG_OPRL },
  { "xornot",		OPR(0x11,0x48), ALL, ARG_OPR },		/* alias */
  { "xornot",		OPRL(0x11,0x48), ALL, ARG_OPRL },	/* alias */
  { "amask",		OPR(0x11,0x61), EV56, ARG_OPRZ1 },	/* ev56 */
  { "amask",		OPRL(0x11,0x61), EV56, ARG_OPRLZ1 },	/* ev56 */
  { "cmovle",		OPR(0x11,0x64), ALL, ARG_OPR },
  { "cmovle",		OPRL(0x11,0x64), ALL, ARG_OPRL },
  { "cmovgt",		OPR(0x11,0x66), ALL, ARG_OPR },
  { "cmovgt",		OPRL(0x11,0x66), ALL, ARG_OPRL },
  { "implver",		OPR(0x11,0x6C), ALL, ARG_OPRZ1 },	/* ev56 */
  { "implver",		OPRL(0x11,0x6C), ALL, ARG_OPRLZ1 },	/* ev56 */

  { "mskbl",		OPR(0x12,0x02), ALL, ARG_OPR },
  { "mskbl",		OPRL(0x12,0x02), ALL, ARG_OPRL },
  { "extbl",		OPR(0x12,0x06), ALL, ARG_OPR },
  { "extbl",		OPRL(0x12,0x06), ALL, ARG_OPRL },
  { "insbl",		OPR(0x12,0x0B), ALL, ARG_OPR },
  { "insbl",		OPRL(0x12,0x0B), ALL, ARG_OPRL },
  { "mskwl",		OPR(0x12,0x12), ALL, ARG_OPR },
  { "mskwl",		OPRL(0x12,0x12), ALL, ARG_OPRL },
  { "extwl",		OPR(0x12,0x16), ALL, ARG_OPR },
  { "extwl",		OPRL(0x12,0x16), ALL, ARG_OPRL },
  { "inswl",		OPR(0x12,0x1B), ALL, ARG_OPR },
  { "inswl",		OPRL(0x12,0x1B), ALL, ARG_OPRL },
  { "mskll",		OPR(0x12,0x22), ALL, ARG_OPR },
  { "mskll",		OPRL(0x12,0x22), ALL, ARG_OPRL },
  { "extll",		OPR(0x12,0x26), ALL, ARG_OPR },
  { "extll",		OPRL(0x12,0x26), ALL, ARG_OPRL },
  { "insll",		OPR(0x12,0x2B), ALL, ARG_OPR },
  { "insll",		OPRL(0x12,0x2B), ALL, ARG_OPRL },
  { "zap",		OPR(0x12,0x30), ALL, ARG_OPR },
  { "zap",		OPRL(0x12,0x30), ALL, ARG_OPRL },
  { "zapnot",		OPR(0x12,0x31), ALL, ARG_OPR },
  { "zapnot",		OPRL(0x12,0x31), ALL, ARG_OPRL },
  { "mskql",		OPR(0x12,0x32), ALL, ARG_OPR },
  { "mskql",		OPRL(0x12,0x32), ALL, ARG_OPRL },
  { "srl",		OPR(0x12,0x34), ALL, ARG_OPR },
  { "srl",		OPRL(0x12,0x34), ALL, ARG_OPRL },
  { "extql",		OPR(0x12,0x36), ALL, ARG_OPR },
  { "extql",		OPRL(0x12,0x36), ALL, ARG_OPRL },
  { "sll",		OPR(0x12,0x39), ALL, ARG_OPR },
  { "sll",		OPRL(0x12,0x39), ALL, ARG_OPRL },
  { "insql",		OPR(0x12,0x3B), ALL, ARG_OPR },
  { "insql",		OPRL(0x12,0x3B), ALL, ARG_OPRL },
  { "sra",		OPR(0x12,0x3C), ALL, ARG_OPR },
  { "sra",		OPRL(0x12,0x3C), ALL, ARG_OPRL },
  { "mskwh",		OPR(0x12,0x52), ALL, ARG_OPR },
  { "mskwh",		OPRL(0x12,0x52), ALL, ARG_OPRL },
  { "inswh",		OPR(0x12,0x57), ALL, ARG_OPR },
  { "inswh",		OPRL(0x12,0x57), ALL, ARG_OPRL },
  { "extwh",		OPR(0x12,0x5A), ALL, ARG_OPR },
  { "extwh",		OPRL(0x12,0x5A), ALL, ARG_OPRL },
  { "msklh",		OPR(0x12,0x62), ALL, ARG_OPR },
  { "msklh",		OPRL(0x12,0x62), ALL, ARG_OPRL },
  { "inslh",		OPR(0x12,0x67), ALL, ARG_OPR },
  { "inslh",		OPRL(0x12,0x67), ALL, ARG_OPRL },
  { "extlh",		OPR(0x12,0x6A), ALL, ARG_OPR },
  { "extlh",		OPRL(0x12,0x6A), ALL, ARG_OPRL },
  { "mskqh",		OPR(0x12,0x72), ALL, ARG_OPR },
  { "mskqh",		OPRL(0x12,0x72), ALL, ARG_OPRL },
  { "insqh",		OPR(0x12,0x77), ALL, ARG_OPR },
  { "insqh",		OPRL(0x12,0x77), ALL, ARG_OPRL },
  { "extqh",		OPR(0x12,0x7A), ALL, ARG_OPR },
  { "extqh",		OPRL(0x12,0x7A), ALL, ARG_OPRL },

  { "mull",		OPR(0x13,0x00), ALL, ARG_OPR },
  { "mull",		OPRL(0x13,0x00), ALL, ARG_OPRL },
  { "mulq",		OPR(0x13,0x20), ALL, ARG_OPR },
  { "mulq",		OPRL(0x13,0x20), ALL, ARG_OPRL },
  { "umulh",		OPR(0x13,0x30), ALL, ARG_OPR },
  { "umulh",		OPRL(0x13,0x30), ALL, ARG_OPRL },
  { "mull/v",		OPR(0x13,0x40), ALL, ARG_OPR },
  { "mull/v",		OPRL(0x13,0x40), ALL, ARG_OPRL },
  { "mulq/v",		OPR(0x13,0x60), ALL, ARG_OPR },
  { "mulq/v",		OPRL(0x13,0x60), ALL, ARG_OPRL },

  { "addf/c",		FP(0x15,0x000), ALL, ARG_FP },
  { "subf/c",		FP(0x15,0x001), ALL, ARG_FP },
  { "mulf/c",		FP(0x15,0x002), ALL, ARG_FP },
  { "divf/c",		FP(0x15,0x003), ALL, ARG_FP },
  { "cvtdg/c",		FP(0x15,0x01E), ALL, ARG_FPZ1 },
  { "addg/c",		FP(0x15,0x020), ALL, ARG_FP },
  { "subg/c",		FP(0x15,0x021), ALL, ARG_FP },
  { "mulg/c",		FP(0x15,0x022), ALL, ARG_FP },
  { "divg/c",		FP(0x15,0x023), ALL, ARG_FP },
  { "cvtgf/c",		FP(0x15,0x02C), ALL, ARG_FPZ1 },
  { "cvtgd/c",		FP(0x15,0x02D), ALL, ARG_FPZ1 },
  { "cvtgq/c",		FP(0x15,0x02F), ALL, ARG_FPZ1 },
  { "cvtqf/c",		FP(0x15,0x03C), ALL, ARG_FPZ1 },
  { "cvtqg/c",		FP(0x15,0x03E), ALL, ARG_FPZ1 },
  { "addf",		FP(0x15,0x080), ALL, ARG_FP },
  { "negf",		FP(0x15,0x081), ALL, ARG_FPZ1 },	/* pseudo */
  { "subf",		FP(0x15,0x081), ALL, ARG_FP },
  { "mulf",		FP(0x15,0x082), ALL, ARG_FP },
  { "divf",		FP(0x15,0x083), ALL, ARG_FP },
  { "cvtdg",		FP(0x15,0x09E), ALL, ARG_FPZ1 },
  { "addg",		FP(0x15,0x0A0), ALL, ARG_FP },
  { "negg",		FP(0x15,0x0A1), ALL, ARG_FPZ1 },	/* pseudo */
  { "subg",		FP(0x15,0x0A1), ALL, ARG_FP },
  { "mulg",		FP(0x15,0x0A2), ALL, ARG_FP },
  { "divg",		FP(0x15,0x0A3), ALL, ARG_FP },
  { "cmpgeq",		FP(0x15,0x0A5), ALL, ARG_FP },
  { "cmpglt",		FP(0x15,0x0A6), ALL, ARG_FP },
  { "cmpgle",		FP(0x15,0x0A7), ALL, ARG_FP },
  { "cvtgf",		FP(0x15,0x0AC), ALL, ARG_FPZ1 },
  { "cvtgd",		FP(0x15,0x0AD), ALL, ARG_FPZ1 },
  { "cvtgq",		FP(0x15,0x0AF), ALL, ARG_FPZ1 },
  { "cvtqf",		FP(0x15,0x0BC), ALL, ARG_FPZ1 },
  { "cvtqg",		FP(0x15,0x0BE), ALL, ARG_FPZ1 },
  { "addf/uc",		FP(0x15,0x100), ALL, ARG_FP },
  { "subf/uc",		FP(0x15,0x101), ALL, ARG_FP },
  { "mulf/uc",		FP(0x15,0x102), ALL, ARG_FP },
  { "divf/uc",		FP(0x15,0x103), ALL, ARG_FP },
  { "cvtdg/uc",		FP(0x15,0x11E), ALL, ARG_FPZ1 },
  { "addg/uc",		FP(0x15,0x120), ALL, ARG_FP },
  { "subg/uc",		FP(0x15,0x121), ALL, ARG_FP },
  { "mulg/uc",		FP(0x15,0x122), ALL, ARG_FP },
  { "divg/uc",		FP(0x15,0x123), ALL, ARG_FP },
  { "cvtgf/uc",		FP(0x15,0x12C), ALL, ARG_FPZ1 },
  { "cvtgd/uc",		FP(0x15,0x12D), ALL, ARG_FPZ1 },
  { "cvtgq/vc",		FP(0x15,0x12F), ALL, ARG_FPZ1 },
  { "addf/u",		FP(0x15,0x180), ALL, ARG_FP },
  { "subf/u",		FP(0x15,0x181), ALL, ARG_FP },
  { "mulf/u",		FP(0x15,0x182), ALL, ARG_FP },
  { "divf/u",		FP(0x15,0x183), ALL, ARG_FP },
  { "cvtdg/u",		FP(0x15,0x19E), ALL, ARG_FPZ1 },
  { "addg/u",		FP(0x15,0x1A0), ALL, ARG_FP },
  { "subg/u",		FP(0x15,0x1A1), ALL, ARG_FP },
  { "mulg/u",		FP(0x15,0x1A2), ALL, ARG_FP },
  { "divg/u",		FP(0x15,0x1A3), ALL, ARG_FP },
  { "cvtgf/u",		FP(0x15,0x1AC), ALL, ARG_FPZ1 },
  { "cvtgd/u",		FP(0x15,0x1AD), ALL, ARG_FPZ1 },
  { "cvtgq/v",		FP(0x15,0x1AF), ALL, ARG_FPZ1 },
  { "addf/sc",		FP(0x15,0x400), ALL, ARG_FP },
  { "subf/sc",		FP(0x15,0x401), ALL, ARG_FP },
  { "mulf/sc",		FP(0x15,0x402), ALL, ARG_FP },
  { "divf/sc",		FP(0x15,0x403), ALL, ARG_FP },
  { "cvtdg/sc",		FP(0x15,0x41E), ALL, ARG_FPZ1 },
  { "addg/sc",		FP(0x15,0x420), ALL, ARG_FP },
  { "subg/sc",		FP(0x15,0x421), ALL, ARG_FP },
  { "mulg/sc",		FP(0x15,0x422), ALL, ARG_FP },
  { "divg/sc",		FP(0x15,0x423), ALL, ARG_FP },
  { "cvtgf/sc",		FP(0x15,0x42C), ALL, ARG_FPZ1 },
  { "cvtgd/sc",		FP(0x15,0x42D), ALL, ARG_FPZ1 },
  { "cvtgq/sc",		FP(0x15,0x42F), ALL, ARG_FPZ1 },
  { "addf/s",		FP(0x15,0x480), ALL, ARG_FP },
  { "negf/s",		FP(0x15,0x481), ALL, ARG_FPZ1 },	/* pseudo */
  { "subf/s",		FP(0x15,0x481), ALL, ARG_FP },
  { "mulf/s",		FP(0x15,0x482), ALL, ARG_FP },
  { "divf/s",		FP(0x15,0x483), ALL, ARG_FP },
  { "cvtdg/s",		FP(0x15,0x49E), ALL, ARG_FPZ1 },
  { "addg/s",		FP(0x15,0x4A0), ALL, ARG_FP },
  { "negg/s",		FP(0x15,0x4A1), ALL, ARG_FPZ1 },	/* pseudo */
  { "subg/s",		FP(0x15,0x4A1), ALL, ARG_FP },
  { "mulg/s",		FP(0x15,0x4A2), ALL, ARG_FP },
  { "divg/s",		FP(0x15,0x4A3), ALL, ARG_FP },
  { "cmpgeq/s",		FP(0x15,0x4A5), ALL, ARG_FP },
  { "cmpglt/s",		FP(0x15,0x4A6), ALL, ARG_FP },
  { "cmpgle/s",		FP(0x15,0x4A7), ALL, ARG_FP },
  { "cvtgf/s",		FP(0x15,0x4AC), ALL, ARG_FPZ1 },
  { "cvtgd/s",		FP(0x15,0x4AD), ALL, ARG_FPZ1 },
  { "cvtgq/s",		FP(0x15,0x4AF), ALL, ARG_FPZ1 },
  { "addf/suc",		FP(0x15,0x500), ALL, ARG_FP },
  { "subf/suc",		FP(0x15,0x501), ALL, ARG_FP },
  { "mulf/suc",		FP(0x15,0x502), ALL, ARG_FP },
  { "divf/suc",		FP(0x15,0x503), ALL, ARG_FP },
  { "cvtdg/suc",	FP(0x15,0x51E), ALL, ARG_FPZ1 },
  { "addg/suc",		FP(0x15,0x520), ALL, ARG_FP },
  { "subg/suc",		FP(0x15,0x521), ALL, ARG_FP },
  { "mulg/suc",		FP(0x15,0x522), ALL, ARG_FP },
  { "divg/suc",		FP(0x15,0x523), ALL, ARG_FP },
  { "cvtgf/suc",	FP(0x15,0x52C), ALL, ARG_FPZ1 },
  { "cvtgd/suc",	FP(0x15,0x52D), ALL, ARG_FPZ1 },
  { "cvtgq/svc",	FP(0x15,0x52F), ALL, ARG_FPZ1 },
  { "addf/su",		FP(0x15,0x580), ALL, ARG_FP },
  { "subf/su",		FP(0x15,0x581), ALL, ARG_FP },
  { "mulf/su",		FP(0x15,0x582), ALL, ARG_FP },
  { "divf/su",		FP(0x15,0x583), ALL, ARG_FP },
  { "cvtdg/su",		FP(0x15,0x59E), ALL, ARG_FPZ1 },
  { "addg/su",		FP(0x15,0x5A0), ALL, ARG_FP },
  { "subg/su",		FP(0x15,0x5A1), ALL, ARG_FP },
  { "mulg/su",		FP(0x15,0x5A2), ALL, ARG_FP },
  { "divg/su",		FP(0x15,0x5A3), ALL, ARG_FP },
  { "cvtgf/su",		FP(0x15,0x5AC), ALL, ARG_FPZ1 },
  { "cvtgd/su",		FP(0x15,0x5AD), ALL, ARG_FPZ1 },
  { "cvtgq/sv",		FP(0x15,0x5AF), ALL, ARG_FPZ1 },

  { "adds/c",		FP(0x16,0x000), ALL, ARG_FP },
  { "subs/c",		FP(0x16,0x001), ALL, ARG_FP },
  { "muls/c",		FP(0x16,0x002), ALL, ARG_FP },
  { "divs/c",		FP(0x16,0x003), ALL, ARG_FP },
  { "addt/c",		FP(0x16,0x020), ALL, ARG_FP },
  { "subt/c",		FP(0x16,0x021), ALL, ARG_FP },
  { "mult/c",		FP(0x16,0x022), ALL, ARG_FP },
  { "divt/c",		FP(0x16,0x023), ALL, ARG_FP },
  { "cvtts/c",		FP(0x16,0x02C), ALL, ARG_FPZ1 },
  { "cvttq/c",		FP(0x16,0x02F), ALL, ARG_FPZ1 },
  { "cvtqs/c",		FP(0x16,0x03C), ALL, ARG_FPZ1 },
  { "cvtqt/c",		FP(0x16,0x03E), ALL, ARG_FPZ1 },
  { "adds/m",		FP(0x16,0x040), ALL, ARG_FP },
  { "subs/m",		FP(0x16,0x041), ALL, ARG_FP },
  { "muls/m",		FP(0x16,0x042), ALL, ARG_FP },
  { "divs/m",		FP(0x16,0x043), ALL, ARG_FP },
  { "addt/m",		FP(0x16,0x060), ALL, ARG_FP },
  { "subt/m",		FP(0x16,0x061), ALL, ARG_FP },
  { "mult/m",		FP(0x16,0x062), ALL, ARG_FP },
  { "divt/m",		FP(0x16,0x063), ALL, ARG_FP },
  { "cvtts/m",		FP(0x16,0x06C), ALL, ARG_FPZ1 },
  { "cvttq/m",		FP(0x16,0x06F), ALL, ARG_FPZ1 },
  { "cvtqs/m",		FP(0x16,0x07C), ALL, ARG_FPZ1 },
  { "cvtqt/m",		FP(0x16,0x07E), ALL, ARG_FPZ1 },
  { "adds",		FP(0x16,0x080), ALL, ARG_FP },
  { "negs", 		FP(0x16,0x081), ALL, ARG_FPZ1 },	/* pseudo */
  { "subs",		FP(0x16,0x081), ALL, ARG_FP },
  { "muls",		FP(0x16,0x082), ALL, ARG_FP },
  { "divs",		FP(0x16,0x083), ALL, ARG_FP },
  { "addt",		FP(0x16,0x0A0), ALL, ARG_FP },
  { "negt", 		FP(0x16,0x0A1), ALL, ARG_FPZ1 },	/* pseudo */
  { "subt",		FP(0x16,0x0A1), ALL, ARG_FP },
  { "mult",		FP(0x16,0x0A2), ALL, ARG_FP },
  { "divt",		FP(0x16,0x0A3), ALL, ARG_FP },
  { "cmptun",		FP(0x16,0x0A4), ALL, ARG_FP },
  { "cmpteq",		FP(0x16,0x0A5), ALL, ARG_FP },
  { "cmptlt",		FP(0x16,0x0A6), ALL, ARG_FP },
  { "cmptle",		FP(0x16,0x0A7), ALL, ARG_FP },
  { "cvtts",		FP(0x16,0x0AC), ALL, ARG_FPZ1 },
  { "cvttq",		FP(0x16,0x0AF), ALL, ARG_FPZ1 },
  { "cvtqs",		FP(0x16,0x0BC), ALL, ARG_FPZ1 },
  { "cvtqt",		FP(0x16,0x0BE), ALL, ARG_FPZ1 },
  { "adds/d",		FP(0x16,0x0C0), ALL, ARG_FP },
  { "subs/d",		FP(0x16,0x0C1), ALL, ARG_FP },
  { "muls/d",		FP(0x16,0x0C2), ALL, ARG_FP },
  { "divs/d",		FP(0x16,0x0C3), ALL, ARG_FP },
  { "addt/d",		FP(0x16,0x0E0), ALL, ARG_FP },
  { "subt/d",		FP(0x16,0x0E1), ALL, ARG_FP },
  { "mult/d",		FP(0x16,0x0E2), ALL, ARG_FP },
  { "divt/d",		FP(0x16,0x0E3), ALL, ARG_FP },
  { "cvtts/d",		FP(0x16,0x0EC), ALL, ARG_FPZ1 },
  { "cvttq/d",		FP(0x16,0x0EF), ALL, ARG_FPZ1 },
  { "cvtqs/d",		FP(0x16,0x0FC), ALL, ARG_FPZ1 },
  { "cvtqt/d",		FP(0x16,0x0FE), ALL, ARG_FPZ1 },
  { "adds/uc",		FP(0x16,0x100), ALL, ARG_FP },
  { "subs/uc",		FP(0x16,0x101), ALL, ARG_FP },
  { "muls/uc",		FP(0x16,0x102), ALL, ARG_FP },
  { "divs/uc",		FP(0x16,0x103), ALL, ARG_FP },
  { "addt/uc",		FP(0x16,0x120), ALL, ARG_FP },
  { "subt/uc",		FP(0x16,0x121), ALL, ARG_FP },
  { "mult/uc",		FP(0x16,0x122), ALL, ARG_FP },
  { "divt/uc",		FP(0x16,0x123), ALL, ARG_FP },
  { "cvtts/uc",		FP(0x16,0x12C), ALL, ARG_FPZ1 },
  { "cvttq/vc",		FP(0x16,0x12F), ALL, ARG_FPZ1 },
  { "adds/um",		FP(0x16,0x140), ALL, ARG_FP },
  { "subs/um",		FP(0x16,0x141), ALL, ARG_FP },
  { "muls/um",		FP(0x16,0x142), ALL, ARG_FP },
  { "divs/um",		FP(0x16,0x143), ALL, ARG_FP },
  { "addt/um",		FP(0x16,0x160), ALL, ARG_FP },
  { "subt/um",		FP(0x16,0x161), ALL, ARG_FP },
  { "mult/um",		FP(0x16,0x162), ALL, ARG_FP },
  { "divt/um",		FP(0x16,0x163), ALL, ARG_FP },
  { "cvtts/um",		FP(0x16,0x16C), ALL, ARG_FPZ1 },
  { "cvttq/um",		FP(0x16,0x16F), ALL, ARG_FPZ1 },
  { "cvtqs/um",		FP(0x16,0x17C), ALL, ARG_FPZ1 },
  { "adds/u",		FP(0x16,0x180), ALL, ARG_FP },
  { "subs/u",		FP(0x16,0x181), ALL, ARG_FP },
  { "muls/u",		FP(0x16,0x182), ALL, ARG_FP },
  { "divs/u",		FP(0x16,0x183), ALL, ARG_FP },
  { "addt/u",		FP(0x16,0x1A0), ALL, ARG_FP },
  { "subt/u",		FP(0x16,0x1A1), ALL, ARG_FP },
  { "mult/u",		FP(0x16,0x1A2), ALL, ARG_FP },
  { "divt/u",		FP(0x16,0x1A3), ALL, ARG_FP },
  { "cvtts/u",		FP(0x16,0x1AC), ALL, ARG_FPZ1 },
  { "cvttq/v",		FP(0x16,0x1AF), ALL, ARG_FPZ1 },
  { "adds/ud",		FP(0x16,0x1C0), ALL, ARG_FP },
  { "subs/ud",		FP(0x16,0x1C1), ALL, ARG_FP },
  { "muls/ud",		FP(0x16,0x1C2), ALL, ARG_FP },
  { "divs/ud",		FP(0x16,0x1C3), ALL, ARG_FP },
  { "addt/ud",		FP(0x16,0x1E0), ALL, ARG_FP },
  { "subt/ud",		FP(0x16,0x1E1), ALL, ARG_FP },
  { "mult/ud",		FP(0x16,0x1E2), ALL, ARG_FP },
  { "divt/ud",		FP(0x16,0x1E3), ALL, ARG_FP },
  { "cvtts/ud",		FP(0x16,0x1EC), ALL, ARG_FPZ1 },
  { "cvttq/ud",		FP(0x16,0x1EF), ALL, ARG_FPZ1 },
  { "cvtst",		FP(0x16,0x2AC), ALL, ARG_FPZ1 },
  { "adds/suc",		FP(0x16,0x500), ALL, ARG_FP },
  { "subs/suc",		FP(0x16,0x501), ALL, ARG_FP },
  { "muls/suc",		FP(0x16,0x502), ALL, ARG_FP },
  { "divs/suc",		FP(0x16,0x503), ALL, ARG_FP },
  { "addt/suc",		FP(0x16,0x520), ALL, ARG_FP },
  { "subt/suc",		FP(0x16,0x521), ALL, ARG_FP },
  { "mult/suc",		FP(0x16,0x522), ALL, ARG_FP },
  { "divt/suc",		FP(0x16,0x523), ALL, ARG_FP },
  { "cvtts/suc",	FP(0x16,0x52C), ALL, ARG_FPZ1 },
  { "cvttq/svc",	FP(0x16,0x52F), ALL, ARG_FPZ1 },
  { "adds/sum",		FP(0x16,0x540), ALL, ARG_FP },
  { "subs/sum",		FP(0x16,0x541), ALL, ARG_FP },
  { "muls/sum",		FP(0x16,0x542), ALL, ARG_FP },
  { "divs/sum",		FP(0x16,0x543), ALL, ARG_FP },
  { "addt/sum",		FP(0x16,0x560), ALL, ARG_FP },
  { "subt/sum",		FP(0x16,0x561), ALL, ARG_FP },
  { "mult/sum",		FP(0x16,0x562), ALL, ARG_FP },
  { "divt/sum",		FP(0x16,0x563), ALL, ARG_FP },
  { "cvtts/sum",	FP(0x16,0x56C), ALL, ARG_FPZ1 },
  { "cvttq/sum",	FP(0x16,0x56F), ALL, ARG_FPZ1 },
  { "cvtqs/sum",	FP(0x16,0x57C), ALL, ARG_FPZ1 },
  { "adds/su",		FP(0x16,0x580), ALL, ARG_FP },
  { "negs/su",		FP(0x16,0x581), ALL, ARG_FPZ1 },	/* pseudo */
  { "subs/su",		FP(0x16,0x581), ALL, ARG_FP },
  { "muls/su",		FP(0x16,0x582), ALL, ARG_FP },
  { "divs/su",		FP(0x16,0x583), ALL, ARG_FP },
  { "addt/su",		FP(0x16,0x5A0), ALL, ARG_FP },
  { "negt/su",		FP(0x16,0x5A1), ALL, ARG_FPZ1 },	/* pseudo */
  { "subt/su",		FP(0x16,0x5A1), ALL, ARG_FP },
  { "mult/su",		FP(0x16,0x5A2), ALL, ARG_FP },
  { "divt/su",		FP(0x16,0x5A3), ALL, ARG_FP },
  { "cmptun/su",	FP(0x16,0x5A4), ALL, ARG_FP },
  { "cmpteq/su",	FP(0x16,0x5A5), ALL, ARG_FP },
  { "cmptlt/su",	FP(0x16,0x5A6), ALL, ARG_FP },
  { "cmptle/su",	FP(0x16,0x5A7), ALL, ARG_FP },
  { "cvtts/su",		FP(0x16,0x5AC), ALL, ARG_FPZ1 },
  { "cvttq/sv",		FP(0x16,0x5AF), ALL, ARG_FPZ1 },
  { "adds/sud",		FP(0x16,0x5C0), ALL, ARG_FP },
  { "subs/sud",		FP(0x16,0x5C1), ALL, ARG_FP },
  { "muls/sud",		FP(0x16,0x5C2), ALL, ARG_FP },
  { "divs/sud",		FP(0x16,0x5C3), ALL, ARG_FP },
  { "addt/sud",		FP(0x16,0x5E0), ALL, ARG_FP },
  { "subt/sud",		FP(0x16,0x5E1), ALL, ARG_FP },
  { "mult/sud",		FP(0x16,0x5E2), ALL, ARG_FP },
  { "divt/sud",		FP(0x16,0x5E3), ALL, ARG_FP },
  { "cvtts/sud",	FP(0x16,0x5EC), ALL, ARG_FPZ1 },
  { "cvttq/sud",	FP(0x16,0x5EF), ALL, ARG_FPZ1 },
  { "cvtst/s",		FP(0x16,0x6AC), ALL, ARG_FPZ1 },
  { "adds/suic",	FP(0x16,0x700), ALL, ARG_FP },
  { "subs/suic",	FP(0x16,0x701), ALL, ARG_FP },
  { "muls/suic",	FP(0x16,0x702), ALL, ARG_FP },
  { "divs/suic",	FP(0x16,0x703), ALL, ARG_FP },
  { "addt/suic",	FP(0x16,0x720), ALL, ARG_FP },
  { "subt/suic",	FP(0x16,0x721), ALL, ARG_FP },
  { "mult/suic",	FP(0x16,0x722), ALL, ARG_FP },
  { "divt/suic",	FP(0x16,0x723), ALL, ARG_FP },
  { "cvtts/suic",	FP(0x16,0x72C), ALL, ARG_FPZ1 },
  { "cvttq/svic",	FP(0x16,0x72F), ALL, ARG_FPZ1 },
  { "cvtqs/suic",	FP(0x16,0x73C), ALL, ARG_FPZ1 },
  { "cvtqt/suic",	FP(0x16,0x73E), ALL, ARG_FPZ1 },
  { "adds/suim",	FP(0x16,0x740), ALL, ARG_FP },
  { "subs/suim",	FP(0x16,0x741), ALL, ARG_FP },
  { "muls/suim",	FP(0x16,0x742), ALL, ARG_FP },
  { "divs/suim",	FP(0x16,0x743), ALL, ARG_FP },
  { "addt/suim",	FP(0x16,0x760), ALL, ARG_FP },
  { "subt/suim",	FP(0x16,0x761), ALL, ARG_FP },
  { "mult/suim",	FP(0x16,0x762), ALL, ARG_FP },
  { "divt/suim",	FP(0x16,0x763), ALL, ARG_FP },
  { "cvtts/suim",	FP(0x16,0x76C), ALL, ARG_FPZ1 },
  { "cvttq/suim",	FP(0x16,0x76F), ALL, ARG_FPZ1 },
  { "cvtqs/suim",	FP(0x16,0x77C), ALL, ARG_FPZ1 },
  { "cvtqt/suim",	FP(0x16,0x77E), ALL, ARG_FPZ1 },
  { "adds/sui",		FP(0x16,0x780), ALL, ARG_FP },
  { "negs/sui", 	FP(0x16,0x781), ALL, ARG_FPZ1 },	/* pseudo */
  { "subs/sui",		FP(0x16,0x781), ALL, ARG_FP },
  { "muls/sui",		FP(0x16,0x782), ALL, ARG_FP },
  { "divs/sui",		FP(0x16,0x783), ALL, ARG_FP },
  { "addt/sui",		FP(0x16,0x7A0), ALL, ARG_FP },
  { "negt/sui", 	FP(0x16,0x7A1), ALL, ARG_FPZ1 },	/* pseudo */
  { "subt/sui",		FP(0x16,0x7A1), ALL, ARG_FP },
  { "mult/sui",		FP(0x16,0x7A2), ALL, ARG_FP },
  { "divt/sui",		FP(0x16,0x7A3), ALL, ARG_FP },
  { "cvtts/sui",	FP(0x16,0x7AC), ALL, ARG_FPZ1 },
  { "cvttq/svi",	FP(0x16,0x7AF), ALL, ARG_FPZ1 },
  { "cvtqs/sui",	FP(0x16,0x7BC), ALL, ARG_FPZ1 },
  { "cvtqt/sui",	FP(0x16,0x7BE), ALL, ARG_FPZ1 },
  { "adds/suid",	FP(0x16,0x7C0), ALL, ARG_FP },
  { "subs/suid",	FP(0x16,0x7C1), ALL, ARG_FP },
  { "muls/suid",	FP(0x16,0x7C2), ALL, ARG_FP },
  { "divs/suid",	FP(0x16,0x7C3), ALL, ARG_FP },
  { "addt/suid",	FP(0x16,0x7E0), ALL, ARG_FP },
  { "subt/suid",	FP(0x16,0x7E1), ALL, ARG_FP },
  { "mult/suid",	FP(0x16,0x7E2), ALL, ARG_FP },
  { "divt/suid",	FP(0x16,0x7E3), ALL, ARG_FP },
  { "cvtts/suid",	FP(0x16,0x7EC), ALL, ARG_FPZ1 },
  { "cvttq/suid",	FP(0x16,0x7EF), ALL, ARG_FPZ1 },
  { "cvtqs/suid",	FP(0x16,0x7FC), ALL, ARG_FPZ1 },
  { "cvtqt/suid",	FP(0x16,0x7FE), ALL, ARG_FPZ1 },

  { "cvtlq",		FP(0x17,0x010), ALL, ARG_FPZ1 },
  { "fnop",		FP(0x17,0x020), ALL, { ZA, ZB, ZC } },	/* pseudo */
  { "fclr",		FP(0x17,0x020), ALL, { ZA, ZB, FC } },	/* pseudo */
  { "fabs",		FP(0x17,0x020), ALL, ARG_FPZ1 },	/* pseudo */
  { "fmov",		FP(0x17,0x020), ALL, { FA, RBA, FC } }, /* pseudo */
  { "cpys",		FP(0x17,0x020), ALL, ARG_FP },
  { "fneg",		FP(0x17,0x021), ALL, { FA, RBA, FC } }, /* pseudo */
  { "cpysn",		FP(0x17,0x021), ALL, ARG_FP },
  { "cpyse",		FP(0x17,0x022), ALL, ARG_FP },
  { "mt_fpcr",		FP(0x17,0x024), ALL, { FA, RBA, RCA } },
  { "mf_fpcr",		FP(0x17,0x025), ALL, { FA, RBA, RCA } },
  { "fcmoveq",		FP(0x17,0x02A), ALL, ARG_FP },
  { "fcmovne",		FP(0x17,0x02B), ALL, ARG_FP },
  { "fcmovlt",		FP(0x17,0x02C), ALL, ARG_FP },
  { "fcmovge",		FP(0x17,0x02D), ALL, ARG_FP },
  { "fcmovle",		FP(0x17,0x02E), ALL, ARG_FP },
  { "fcmovgt",		FP(0x17,0x02F), ALL, ARG_FP },
  { "cvtql",		FP(0x17,0x030), ALL, ARG_FPZ1 },
  { "cvtql/v",		FP(0x17,0x130), ALL, ARG_FPZ1 },
  { "cvtql/sv",		FP(0x17,0x530), ALL, ARG_FPZ1 },

  { "trapb",		MFC(0x18,0x0000), ALL, ARG_NONE },
  { "draint",		MFC(0x18,0x0000), ALL, ARG_NONE },	/* alias */
  { "excb",		MFC(0x18,0x0400), ALL, ARG_NONE },
  { "mb",		MFC(0x18,0x4000), ALL, ARG_NONE },
  { "wmb",		MFC(0x18,0x4400), ALL, ARG_NONE },
  { "fetch",		MFC(0x18,0x8000), ALL, { MDISP, RB } },
  { "fetch_m",		MFC(0x18,0xA000), ALL, { MDISP, RB } },
  { "rpcc",		MFC(0x18,0xC000), ALL, { RA } },
  { "rc",		MFC(0x18,0xE000), ALL, { RA } },
  { "rs",		MFC(0x18,0xF000), ALL, { RA } },

  { "hw_mfpr",		OPR(0x19,0x00), EV4, { RA, RBA, EV4EXTHWINDEX } },
  { "hw_mfpr",		OP(0x19), OP_MASK, EV5x, { RA, RBA, EV5HWINDEX } },
  { "hw_mfpr/i",	OPR(0x19,0x01), EV4, ARG_EV4HWMPR },
  { "hw_mfpr/a",	OPR(0x19,0x02), EV4, ARG_EV4HWMPR },
  { "hw_mfpr/ai",	OPR(0x19,0x03), EV4, ARG_EV4HWMPR },
  { "hw_mfpr/p",	OPR(0x19,0x04), EV4, ARG_EV4HWMPR },
  { "hw_mfpr/pi",	OPR(0x19,0x05), EV4, ARG_EV4HWMPR },
  { "hw_mfpr/pa",	OPR(0x19,0x06), EV4, ARG_EV4HWMPR },
  { "hw_mfpr/pai",	OPR(0x19,0x07), EV4, ARG_EV4HWMPR },
  { "pal19",		PCD(0x19), ALL, ARG_PCD },

  { "jmp",		MBR(0x1A,0), ALL, { RA, CPRB, JMPHINT } },
  { "jsr",		MBR(0x1A,1), ALL, { RA, CPRB, JMPHINT } },
  { "ret",		MBR(0x1A,2), ALL, { RA, CPRB, RETHINT } },
  { "jcr",		MBR(0x1A,3), ALL, { RA, CPRB, RETHINT } }, /* alias */
  { "jsr_coroutine",	MBR(0x1A,3), ALL, { RA, CPRB, RETHINT } },

  { "hw_ldl",		EV4HWMEM(0x1B,0x0), EV4, ARG_EV4HWMEM },
  { "hw_ldl",		EV5HWMEM(0x1B,0x00), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/a",		EV4HWMEM(0x1B,0x4), EV4, ARG_EV4HWMEM },
  { "hw_ldl/a",		EV5HWMEM(0x1B,0x10), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/al",	EV5HWMEM(0x1B,0x11), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/ar",	EV4HWMEM(0x1B,0x6), EV4, ARG_EV4HWMEM },
  { "hw_ldl/av",	EV5HWMEM(0x1B,0x12), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/avl",	EV5HWMEM(0x1B,0x13), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/aw",	EV5HWMEM(0x1B,0x18), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/awl",	EV5HWMEM(0x1B,0x19), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/awv",	EV5HWMEM(0x1B,0x1a), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/awvl",	EV5HWMEM(0x1B,0x1b), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/l",		EV5HWMEM(0x1B,0x01), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/p",		EV4HWMEM(0x1B,0x8), EV4, ARG_EV4HWMEM },
  { "hw_ldl/p",		EV5HWMEM(0x1B,0x20), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pa",	EV4HWMEM(0x1B,0xC), EV4, ARG_EV4HWMEM },
  { "hw_ldl/pa",	EV5HWMEM(0x1B,0x30), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pal",	EV5HWMEM(0x1B,0x31), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/par",	EV4HWMEM(0x1B,0xE), EV4, ARG_EV4HWMEM },
  { "hw_ldl/pav",	EV5HWMEM(0x1B,0x32), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pavl",	EV5HWMEM(0x1B,0x33), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/paw",	EV5HWMEM(0x1B,0x38), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pawl",	EV5HWMEM(0x1B,0x39), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pawv",	EV5HWMEM(0x1B,0x3a), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pawvl",	EV5HWMEM(0x1B,0x3b), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pl",	EV5HWMEM(0x1B,0x21), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pr",	EV4HWMEM(0x1B,0xA), EV4, ARG_EV4HWMEM },
  { "hw_ldl/pv",	EV5HWMEM(0x1B,0x22), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pvl",	EV5HWMEM(0x1B,0x23), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pw",	EV5HWMEM(0x1B,0x28), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pwl",	EV5HWMEM(0x1B,0x29), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pwv",	EV5HWMEM(0x1B,0x2a), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/pwvl",	EV5HWMEM(0x1B,0x2b), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/r",		EV4HWMEM(0x1B,0x2), EV4, ARG_EV4HWMEM },
  { "hw_ldl/v",		EV5HWMEM(0x1B,0x02), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/vl",	EV5HWMEM(0x1B,0x03), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/w",		EV5HWMEM(0x1B,0x08), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/wl",	EV5HWMEM(0x1B,0x09), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/wv",	EV5HWMEM(0x1B,0x0a), EV5x, ARG_EV5HWMEM },
  { "hw_ldl/wvl",	EV5HWMEM(0x1B,0x0b), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l",		EV5HWMEM(0x1B,0x01), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/a",	EV5HWMEM(0x1B,0x11), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/av",	EV5HWMEM(0x1B,0x13), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/aw",	EV5HWMEM(0x1B,0x19), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/awv",	EV5HWMEM(0x1B,0x1b), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/p",	EV5HWMEM(0x1B,0x21), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/pa",	EV5HWMEM(0x1B,0x31), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/pav",	EV5HWMEM(0x1B,0x33), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/paw",	EV5HWMEM(0x1B,0x39), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/pawv",	EV5HWMEM(0x1B,0x3b), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/pv",	EV5HWMEM(0x1B,0x23), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/pw",	EV5HWMEM(0x1B,0x29), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/pwv",	EV5HWMEM(0x1B,0x2b), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/v",	EV5HWMEM(0x1B,0x03), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/w",	EV5HWMEM(0x1B,0x09), EV5x, ARG_EV5HWMEM },
  { "hw_ldl_l/wv",	EV5HWMEM(0x1B,0x0b), EV5x, ARG_EV5HWMEM },
  { "hw_ldq",		EV4HWMEM(0x1B,0x1), EV4, ARG_EV4HWMEM },
  { "hw_ldq",		EV5HWMEM(0x1B,0x04), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/a",		EV4HWMEM(0x1B,0x5), EV4, ARG_EV4HWMEM },
  { "hw_ldq/a",		EV5HWMEM(0x1B,0x14), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/al",	EV5HWMEM(0x1B,0x15), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/ar",	EV4HWMEM(0x1B,0x7), EV4, ARG_EV4HWMEM },
  { "hw_ldq/av",	EV5HWMEM(0x1B,0x16), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/avl",	EV5HWMEM(0x1B,0x17), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/aw",	EV5HWMEM(0x1B,0x1c), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/awl",	EV5HWMEM(0x1B,0x1d), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/awv",	EV5HWMEM(0x1B,0x1e), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/awvl",	EV5HWMEM(0x1B,0x1f), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/l",		EV5HWMEM(0x1B,0x05), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/p",		EV4HWMEM(0x1B,0x9), EV4, ARG_EV4HWMEM },
  { "hw_ldq/p",		EV5HWMEM(0x1B,0x24), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pa",	EV4HWMEM(0x1B,0xD), EV4, ARG_EV4HWMEM },
  { "hw_ldq/pa",	EV5HWMEM(0x1B,0x34), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pal",	EV5HWMEM(0x1B,0x35), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/par",	EV4HWMEM(0x1B,0xF), EV4, ARG_EV4HWMEM },
  { "hw_ldq/pav",	EV5HWMEM(0x1B,0x36), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pavl",	EV5HWMEM(0x1B,0x37), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/paw",	EV5HWMEM(0x1B,0x3c), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pawl",	EV5HWMEM(0x1B,0x3d), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pawv",	EV5HWMEM(0x1B,0x3e), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pawvl",	EV5HWMEM(0x1B,0x3f), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pl",	EV5HWMEM(0x1B,0x25), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pr",	EV4HWMEM(0x1B,0xB), EV4, ARG_EV4HWMEM },
  { "hw_ldq/pv",	EV5HWMEM(0x1B,0x26), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pvl",	EV5HWMEM(0x1B,0x27), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pw",	EV5HWMEM(0x1B,0x2c), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pwl",	EV5HWMEM(0x1B,0x2d), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pwv",	EV5HWMEM(0x1B,0x2e), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/pwvl",	EV5HWMEM(0x1B,0x2f), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/r",		EV4HWMEM(0x1B,0x3), EV4, ARG_EV4HWMEM },
  { "hw_ldq/v",		EV5HWMEM(0x1B,0x06), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/vl",	EV5HWMEM(0x1B,0x07), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/w",		EV5HWMEM(0x1B,0x0c), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/wl",	EV5HWMEM(0x1B,0x0d), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/wv",	EV5HWMEM(0x1B,0x0e), EV5x, ARG_EV5HWMEM },
  { "hw_ldq/wvl",	EV5HWMEM(0x1B,0x0f), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l",		EV5HWMEM(0x1B,0x05), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/a",	EV5HWMEM(0x1B,0x15), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/av",	EV5HWMEM(0x1B,0x17), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/aw",	EV5HWMEM(0x1B,0x1d), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/awv",	EV5HWMEM(0x1B,0x1f), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/p",	EV5HWMEM(0x1B,0x25), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/pa",	EV5HWMEM(0x1B,0x35), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/pav",	EV5HWMEM(0x1B,0x37), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/paw",	EV5HWMEM(0x1B,0x3d), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/pawv",	EV5HWMEM(0x1B,0x3f), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/pv",	EV5HWMEM(0x1B,0x27), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/pw",	EV5HWMEM(0x1B,0x2d), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/pwv",	EV5HWMEM(0x1B,0x2f), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/v",	EV5HWMEM(0x1B,0x07), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/w",	EV5HWMEM(0x1B,0x0d), EV5x, ARG_EV5HWMEM },
  { "hw_ldq_l/wv",	EV5HWMEM(0x1B,0x0f), EV5x, ARG_EV5HWMEM },
  { "hw_ld",		EV4HWMEM(0x1B,0x0), EV4, ARG_EV4HWMEM },
  { "hw_ld",		EV5HWMEM(0x1B,0x00), EV5x, ARG_EV5HWMEM },
  { "hw_ld/a",		EV4HWMEM(0x1B,0x4), EV4, ARG_EV4HWMEM },
  { "hw_ld/a",		EV5HWMEM(0x1B,0x10), EV5x, ARG_EV5HWMEM },
  { "hw_ld/al",		EV5HWMEM(0x1B,0x11), EV5x, ARG_EV5HWMEM },
  { "hw_ld/aq",		EV4HWMEM(0x1B,0x5), EV4, ARG_EV4HWMEM },
  { "hw_ld/aq",		EV5HWMEM(0x1B,0x14), EV5x, ARG_EV5HWMEM },
  { "hw_ld/aql",	EV5HWMEM(0x1B,0x15), EV5x, ARG_EV5HWMEM },
  { "hw_ld/aqv",	EV5HWMEM(0x1B,0x16), EV5x, ARG_EV5HWMEM },
  { "hw_ld/aqvl",	EV5HWMEM(0x1B,0x17), EV5x, ARG_EV5HWMEM },
  { "hw_ld/ar",		EV4HWMEM(0x1B,0x6), EV4, ARG_EV4HWMEM },
  { "hw_ld/arq",	EV4HWMEM(0x1B,0x7), EV4, ARG_EV4HWMEM },
  { "hw_ld/av",		EV5HWMEM(0x1B,0x12), EV5x, ARG_EV5HWMEM },
  { "hw_ld/avl",	EV5HWMEM(0x1B,0x13), EV5x, ARG_EV5HWMEM },
  { "hw_ld/aw",		EV5HWMEM(0x1B,0x18), EV5x, ARG_EV5HWMEM },
  { "hw_ld/awl",	EV5HWMEM(0x1B,0x19), EV5x, ARG_EV5HWMEM },
  { "hw_ld/awq",	EV5HWMEM(0x1B,0x1c), EV5x, ARG_EV5HWMEM },
  { "hw_ld/awql",	EV5HWMEM(0x1B,0x1d), EV5x, ARG_EV5HWMEM },
  { "hw_ld/awqv",	EV5HWMEM(0x1B,0x1e), EV5x, ARG_EV5HWMEM },
  { "hw_ld/awqvl",	EV5HWMEM(0x1B,0x1f), EV5x, ARG_EV5HWMEM },
  { "hw_ld/awv",	EV5HWMEM(0x1B,0x1a), EV5x, ARG_EV5HWMEM },
  { "hw_ld/awvl",	EV5HWMEM(0x1B,0x1b), EV5x, ARG_EV5HWMEM },
  { "hw_ld/l",		EV5HWMEM(0x1B,0x01), EV5x, ARG_EV5HWMEM },
  { "hw_ld/p",		EV4HWMEM(0x1B,0x8), EV4, ARG_EV4HWMEM },
  { "hw_ld/p",		EV5HWMEM(0x1B,0x20), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pa",		EV4HWMEM(0x1B,0xC), EV4, ARG_EV4HWMEM },
  { "hw_ld/pa",		EV5HWMEM(0x1B,0x30), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pal",	EV5HWMEM(0x1B,0x31), EV5x, ARG_EV5HWMEM },
  { "hw_ld/paq",	EV4HWMEM(0x1B,0xD), EV4, ARG_EV4HWMEM },
  { "hw_ld/paq",	EV5HWMEM(0x1B,0x34), EV5x, ARG_EV5HWMEM },
  { "hw_ld/paql",	EV5HWMEM(0x1B,0x35), EV5x, ARG_EV5HWMEM },
  { "hw_ld/paqv",	EV5HWMEM(0x1B,0x36), EV5x, ARG_EV5HWMEM },
  { "hw_ld/paqvl",	EV5HWMEM(0x1B,0x37), EV5x, ARG_EV5HWMEM },
  { "hw_ld/par",	EV4HWMEM(0x1B,0xE), EV4, ARG_EV4HWMEM },
  { "hw_ld/parq",	EV4HWMEM(0x1B,0xF), EV4, ARG_EV4HWMEM },
  { "hw_ld/pav",	EV5HWMEM(0x1B,0x32), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pavl",	EV5HWMEM(0x1B,0x33), EV5x, ARG_EV5HWMEM },
  { "hw_ld/paw",	EV5HWMEM(0x1B,0x38), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pawl",	EV5HWMEM(0x1B,0x39), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pawq",	EV5HWMEM(0x1B,0x3c), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pawql",	EV5HWMEM(0x1B,0x3d), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pawqv",	EV5HWMEM(0x1B,0x3e), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pawqvl",	EV5HWMEM(0x1B,0x3f), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pawv",	EV5HWMEM(0x1B,0x3a), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pawvl",	EV5HWMEM(0x1B,0x3b), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pl",		EV5HWMEM(0x1B,0x21), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pq",		EV4HWMEM(0x1B,0x9), EV4, ARG_EV4HWMEM },
  { "hw_ld/pq",		EV5HWMEM(0x1B,0x24), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pql",	EV5HWMEM(0x1B,0x25), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pqv",	EV5HWMEM(0x1B,0x26), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pqvl",	EV5HWMEM(0x1B,0x27), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pr",		EV4HWMEM(0x1B,0xA), EV4, ARG_EV4HWMEM },
  { "hw_ld/prq",	EV4HWMEM(0x1B,0xB), EV4, ARG_EV4HWMEM },
  { "hw_ld/pv",		EV5HWMEM(0x1B,0x22), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pvl",	EV5HWMEM(0x1B,0x23), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pw",		EV5HWMEM(0x1B,0x28), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pwl",	EV5HWMEM(0x1B,0x29), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pwq",	EV5HWMEM(0x1B,0x2c), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pwql",	EV5HWMEM(0x1B,0x2d), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pwqv",	EV5HWMEM(0x1B,0x2e), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pwqvl",	EV5HWMEM(0x1B,0x2f), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pwv",	EV5HWMEM(0x1B,0x2a), EV5x, ARG_EV5HWMEM },
  { "hw_ld/pwvl",	EV5HWMEM(0x1B,0x2b), EV5x, ARG_EV5HWMEM },
  { "hw_ld/q",		EV4HWMEM(0x1B,0x1), EV4, ARG_EV4HWMEM },
  { "hw_ld/q",		EV5HWMEM(0x1B,0x04), EV5x, ARG_EV5HWMEM },
  { "hw_ld/ql",		EV5HWMEM(0x1B,0x05), EV5x, ARG_EV5HWMEM },
  { "hw_ld/qv",		EV5HWMEM(0x1B,0x06), EV5x, ARG_EV5HWMEM },
  { "hw_ld/qvl",	EV5HWMEM(0x1B,0x07), EV5x, ARG_EV5HWMEM },
  { "hw_ld/r",		EV4HWMEM(0x1B,0x2), EV4, ARG_EV4HWMEM },
  { "hw_ld/rq",		EV4HWMEM(0x1B,0x3), EV4, ARG_EV4HWMEM },
  { "hw_ld/v",		EV5HWMEM(0x1B,0x02), EV5x, ARG_EV5HWMEM },
  { "hw_ld/vl",		EV5HWMEM(0x1B,0x03), EV5x, ARG_EV5HWMEM },
  { "hw_ld/w",		EV5HWMEM(0x1B,0x08), EV5x, ARG_EV5HWMEM },
  { "hw_ld/wl",		EV5HWMEM(0x1B,0x09), EV5x, ARG_EV5HWMEM },
  { "hw_ld/wq",		EV5HWMEM(0x1B,0x0c), EV5x, ARG_EV5HWMEM },
  { "hw_ld/wql",	EV5HWMEM(0x1B,0x0d), EV5x, ARG_EV5HWMEM },
  { "hw_ld/wqv",	EV5HWMEM(0x1B,0x0e), EV5x, ARG_EV5HWMEM },
  { "hw_ld/wqvl",	EV5HWMEM(0x1B,0x0f), EV5x, ARG_EV5HWMEM },
  { "hw_ld/wv",		EV5HWMEM(0x1B,0x0a), EV5x, ARG_EV5HWMEM },
  { "hw_ld/wvl",	EV5HWMEM(0x1B,0x0b), EV5x, ARG_EV5HWMEM },
  { "pal1b",		PCD(0x1B), ALL, ARG_PCD },

  { "hw_mtpr",		OPR(0x1D,0x00), EV4, { RA, RBA, EV4EXTHWINDEX } },
  { "hw_mtpr",		OP(0x1D), OP_MASK, EV5x, { RA, RBA, EV5HWINDEX } },
  { "hw_mtpr/i", 	OPR(0x1D,0x01), EV4, ARG_EV4HWMPR },
  { "hw_mtpr/a", 	OPR(0x1D,0x02), EV4, ARG_EV4HWMPR },
  { "hw_mtpr/ai",	OPR(0x1D,0x03), EV4, ARG_EV4HWMPR },
  { "hw_mtpr/p", 	OPR(0x1D,0x04), EV4, ARG_EV4HWMPR },
  { "hw_mtpr/pi",	OPR(0x1D,0x05), EV4, ARG_EV4HWMPR },
  { "hw_mtpr/pa",	OPR(0x1D,0x06), EV4, ARG_EV4HWMPR },
  { "hw_mtpr/pai",	OPR(0x1D,0x07), EV4, ARG_EV4HWMPR },
  { "pal1d",		PCD(0x1D), ALL, ARG_PCD },

  { "hw_rei",		SPCD(0x1E,0x3FF8000), EV4|EV5x, ARG_NONE },
  { "hw_rei_stall",	SPCD(0x1E,0x3FFC000), EV5x, ARG_NONE },
  { "pal1e",		PCD(0x1E), ALL, ARG_PCD },

  { "hw_stl",		EV4HWMEM(0x1F,0x0), EV4, ARG_EV4HWMEM },
  { "hw_stl",		EV5HWMEM(0x1F,0x00), EV5x, ARG_EV5HWMEM },
  { "hw_stl/a",		EV4HWMEM(0x1F,0x4), EV4, ARG_EV4HWMEM },
  { "hw_stl/a",		EV5HWMEM(0x1F,0x10), EV5x, ARG_EV5HWMEM },
  { "hw_stl/ac",	EV5HWMEM(0x1F,0x11), EV5x, ARG_EV5HWMEM },
  { "hw_stl/ar",	EV4HWMEM(0x1F,0x6), EV4, ARG_EV4HWMEM },
  { "hw_stl/av",	EV5HWMEM(0x1F,0x12), EV5x, ARG_EV5HWMEM },
  { "hw_stl/avc",	EV5HWMEM(0x1F,0x13), EV5x, ARG_EV5HWMEM },
  { "hw_stl/c",		EV5HWMEM(0x1F,0x01), EV5x, ARG_EV5HWMEM },
  { "hw_stl/p",		EV4HWMEM(0x1F,0x8), EV4, ARG_EV4HWMEM },
  { "hw_stl/p",		EV5HWMEM(0x1F,0x20), EV5x, ARG_EV5HWMEM },
  { "hw_stl/pa",	EV4HWMEM(0x1F,0xC), EV4, ARG_EV4HWMEM },
  { "hw_stl/pa",	EV5HWMEM(0x1F,0x30), EV5x, ARG_EV5HWMEM },
  { "hw_stl/pac",	EV5HWMEM(0x1F,0x31), EV5x, ARG_EV5HWMEM },
  { "hw_stl/pav",	EV5HWMEM(0x1F,0x32), EV5x, ARG_EV5HWMEM },
  { "hw_stl/pavc",	EV5HWMEM(0x1F,0x33), EV5x, ARG_EV5HWMEM },
  { "hw_stl/pc",	EV5HWMEM(0x1F,0x21), EV5x, ARG_EV5HWMEM },
  { "hw_stl/pr",	EV4HWMEM(0x1F,0xA), EV4, ARG_EV4HWMEM },
  { "hw_stl/pv",	EV5HWMEM(0x1F,0x22), EV5x, ARG_EV5HWMEM },
  { "hw_stl/pvc",	EV5HWMEM(0x1F,0x23), EV5x, ARG_EV5HWMEM },
  { "hw_stl/r",		EV4HWMEM(0x1F,0x2), EV4, ARG_EV4HWMEM },
  { "hw_stl/v",		EV5HWMEM(0x1F,0x02), EV5x, ARG_EV5HWMEM },
  { "hw_stl/vc",	EV5HWMEM(0x1F,0x03), EV5x, ARG_EV5HWMEM },
  { "hw_stl_c",		EV5HWMEM(0x1F,0x01), EV5x, ARG_EV5HWMEM },
  { "hw_stl_c/a",	EV5HWMEM(0x1F,0x11), EV5x, ARG_EV5HWMEM },
  { "hw_stl_c/av",	EV5HWMEM(0x1F,0x13), EV5x, ARG_EV5HWMEM },
  { "hw_stl_c/p",	EV5HWMEM(0x1F,0x21), EV5x, ARG_EV5HWMEM },
  { "hw_stl_c/pa",	EV5HWMEM(0x1F,0x31), EV5x, ARG_EV5HWMEM },
  { "hw_stl_c/pav",	EV5HWMEM(0x1F,0x33), EV5x, ARG_EV5HWMEM },
  { "hw_stl_c/pv",	EV5HWMEM(0x1F,0x23), EV5x, ARG_EV5HWMEM },
  { "hw_stl_c/v",	EV5HWMEM(0x1F,0x03), EV5x, ARG_EV5HWMEM },
  { "hw_stq",		EV4HWMEM(0x1F,0x1), EV4, ARG_EV4HWMEM },
  { "hw_stq",		EV5HWMEM(0x1F,0x04), EV5x, ARG_EV5HWMEM },
  { "hw_stq/a",		EV4HWMEM(0x1F,0x5), EV4, ARG_EV4HWMEM },
  { "hw_stq/a",		EV5HWMEM(0x1F,0x14), EV5x, ARG_EV5HWMEM },
  { "hw_stq/ac",	EV5HWMEM(0x1F,0x15), EV5x, ARG_EV5HWMEM },
  { "hw_stq/ar",	EV4HWMEM(0x1F,0x7), EV4, ARG_EV4HWMEM },
  { "hw_stq/av",	EV5HWMEM(0x1F,0x16), EV5x, ARG_EV5HWMEM },
  { "hw_stq/avc",	EV5HWMEM(0x1F,0x17), EV5x, ARG_EV5HWMEM },
  { "hw_stq/c",		EV5HWMEM(0x1F,0x05), EV5x, ARG_EV5HWMEM },
  { "hw_stq/p",		EV4HWMEM(0x1F,0x9), EV4, ARG_EV4HWMEM },
  { "hw_stq/p",		EV5HWMEM(0x1F,0x24), EV5x, ARG_EV5HWMEM },
  { "hw_stq/pa",	EV4HWMEM(0x1F,0xD), EV4, ARG_EV4HWMEM },
  { "hw_stq/pa",	EV5HWMEM(0x1F,0x34), EV5x, ARG_EV5HWMEM },
  { "hw_stq/pac",	EV5HWMEM(0x1F,0x35), EV5x, ARG_EV5HWMEM },
  { "hw_stq/par",	EV4HWMEM(0x1F,0xE), EV4, ARG_EV4HWMEM },
  { "hw_stq/par",	EV4HWMEM(0x1F,0xF), EV4, ARG_EV4HWMEM },
  { "hw_stq/pav",	EV5HWMEM(0x1F,0x36), EV5x, ARG_EV5HWMEM },
  { "hw_stq/pavc",	EV5HWMEM(0x1F,0x37), EV5x, ARG_EV5HWMEM },
  { "hw_stq/pc",	EV5HWMEM(0x1F,0x25), EV5x, ARG_EV5HWMEM },
  { "hw_stq/pr",	EV4HWMEM(0x1F,0xB), EV4, ARG_EV4HWMEM },
  { "hw_stq/pv",	EV5HWMEM(0x1F,0x26), EV5x, ARG_EV5HWMEM },
  { "hw_stq/pvc",	EV5HWMEM(0x1F,0x27), EV5x, ARG_EV5HWMEM },
  { "hw_stq/r",		EV4HWMEM(0x1F,0x3), EV4, ARG_EV4HWMEM },
  { "hw_stq/v",		EV5HWMEM(0x1F,0x06), EV5x, ARG_EV5HWMEM },
  { "hw_stq/vc",	EV5HWMEM(0x1F,0x07), EV5x, ARG_EV5HWMEM },
  { "hw_stq_c",		EV5HWMEM(0x1F,0x05), EV5x, ARG_EV5HWMEM },
  { "hw_stq_c/a",	EV5HWMEM(0x1F,0x15), EV5x, ARG_EV5HWMEM },
  { "hw_stq_c/av",	EV5HWMEM(0x1F,0x17), EV5x, ARG_EV5HWMEM },
  { "hw_stq_c/p",	EV5HWMEM(0x1F,0x25), EV5x, ARG_EV5HWMEM },
  { "hw_stq_c/pa",	EV5HWMEM(0x1F,0x35), EV5x, ARG_EV5HWMEM },
  { "hw_stq_c/pav",	EV5HWMEM(0x1F,0x37), EV5x, ARG_EV5HWMEM },
  { "hw_stq_c/pv",	EV5HWMEM(0x1F,0x27), EV5x, ARG_EV5HWMEM },
  { "hw_stq_c/v",	EV5HWMEM(0x1F,0x07), EV5x, ARG_EV5HWMEM },
  { "hw_st",		EV4HWMEM(0x1F,0x0), EV4, ARG_EV4HWMEM },
  { "hw_st",		EV5HWMEM(0x1F,0x00), EV5x, ARG_EV5HWMEM },
  { "hw_st/a",		EV4HWMEM(0x1F,0x4), EV4, ARG_EV4HWMEM },
  { "hw_st/a",		EV5HWMEM(0x1F,0x10), EV5x, ARG_EV5HWMEM },
  { "hw_st/ac",		EV5HWMEM(0x1F,0x11), EV5x, ARG_EV5HWMEM },
  { "hw_st/aq",		EV4HWMEM(0x1F,0x5), EV4, ARG_EV4HWMEM },
  { "hw_st/aq",		EV5HWMEM(0x1F,0x14), EV5x, ARG_EV5HWMEM },
  { "hw_st/aqc",	EV5HWMEM(0x1F,0x15), EV5x, ARG_EV5HWMEM },
  { "hw_st/aqv",	EV5HWMEM(0x1F,0x16), EV5x, ARG_EV5HWMEM },
  { "hw_st/aqvc",	EV5HWMEM(0x1F,0x17), EV5x, ARG_EV5HWMEM },
  { "hw_st/ar",		EV4HWMEM(0x1F,0x6), EV4, ARG_EV4HWMEM },
  { "hw_st/arq",	EV4HWMEM(0x1F,0x7), EV4, ARG_EV4HWMEM },
  { "hw_st/av",		EV5HWMEM(0x1F,0x12), EV5x, ARG_EV5HWMEM },
  { "hw_st/avc",	EV5HWMEM(0x1F,0x13), EV5x, ARG_EV5HWMEM },
  { "hw_st/c",		EV5HWMEM(0x1F,0x01), EV5x, ARG_EV5HWMEM },
  { "hw_st/p",		EV4HWMEM(0x1F,0x8), EV4, ARG_EV4HWMEM },
  { "hw_st/p",		EV5HWMEM(0x1F,0x20), EV5x, ARG_EV5HWMEM },
  { "hw_st/pa",		EV4HWMEM(0x1F,0xC), EV4, ARG_EV4HWMEM },
  { "hw_st/pa",		EV5HWMEM(0x1F,0x30), EV5x, ARG_EV5HWMEM },
  { "hw_st/pac",	EV5HWMEM(0x1F,0x31), EV5x, ARG_EV5HWMEM },
  { "hw_st/paq",	EV4HWMEM(0x1F,0xD), EV4, ARG_EV4HWMEM },
  { "hw_st/paq",	EV5HWMEM(0x1F,0x34), EV5x, ARG_EV5HWMEM },
  { "hw_st/paqc",	EV5HWMEM(0x1F,0x35), EV5x, ARG_EV5HWMEM },
  { "hw_st/paqv",	EV5HWMEM(0x1F,0x36), EV5x, ARG_EV5HWMEM },
  { "hw_st/paqvc",	EV5HWMEM(0x1F,0x37), EV5x, ARG_EV5HWMEM },
  { "hw_st/par",	EV4HWMEM(0x1F,0xE), EV4, ARG_EV4HWMEM },
  { "hw_st/parq",	EV4HWMEM(0x1F,0xF), EV4, ARG_EV4HWMEM },
  { "hw_st/pav",	EV5HWMEM(0x1F,0x32), EV5x, ARG_EV5HWMEM },
  { "hw_st/pavc",	EV5HWMEM(0x1F,0x33), EV5x, ARG_EV5HWMEM },
  { "hw_st/pc",		EV5HWMEM(0x1F,0x21), EV5x, ARG_EV5HWMEM },
  { "hw_st/pq",		EV4HWMEM(0x1F,0x9), EV4, ARG_EV4HWMEM },
  { "hw_st/pq",		EV5HWMEM(0x1F,0x24), EV5x, ARG_EV5HWMEM },
  { "hw_st/pqc",	EV5HWMEM(0x1F,0x25), EV5x, ARG_EV5HWMEM },
  { "hw_st/pqv",	EV5HWMEM(0x1F,0x26), EV5x, ARG_EV5HWMEM },
  { "hw_st/pqvc",	EV5HWMEM(0x1F,0x27), EV5x, ARG_EV5HWMEM },
  { "hw_st/pr",		EV4HWMEM(0x1F,0xA), EV4, ARG_EV4HWMEM },
  { "hw_st/prq",	EV4HWMEM(0x1F,0xB), EV4, ARG_EV4HWMEM },
  { "hw_st/pv",		EV5HWMEM(0x1F,0x22), EV5x, ARG_EV5HWMEM },
  { "hw_st/pvc",	EV5HWMEM(0x1F,0x23), EV5x, ARG_EV5HWMEM },
  { "hw_st/q",		EV4HWMEM(0x1F,0x1), EV4, ARG_EV4HWMEM },
  { "hw_st/q",		EV5HWMEM(0x1F,0x04), EV5x, ARG_EV5HWMEM },
  { "hw_st/qc",		EV5HWMEM(0x1F,0x05), EV5x, ARG_EV5HWMEM },
  { "hw_st/qv",		EV5HWMEM(0x1F,0x06), EV5x, ARG_EV5HWMEM },
  { "hw_st/qvc",	EV5HWMEM(0x1F,0x07), EV5x, ARG_EV5HWMEM },
  { "hw_st/r",		EV4HWMEM(0x1F,0x2), EV4, ARG_EV4HWMEM },
  { "hw_st/v",		EV5HWMEM(0x1F,0x02), EV5x, ARG_EV5HWMEM },
  { "hw_st/vc",		EV5HWMEM(0x1F,0x03), EV5x, ARG_EV5HWMEM },
  { "pal1f",		PCD(0x1F), ALL, ARG_PCD },

  { "ldf",		MEM(0x20), ALL, ARG_FMEM },
  { "ldg",		MEM(0x21), ALL, ARG_FMEM },
  { "lds",		MEM(0x22), ALL, ARG_FMEM },
  { "ldt",		MEM(0x23), ALL, ARG_FMEM },
  { "stf",		MEM(0x24), ALL, ARG_FMEM },
  { "stg",		MEM(0x25), ALL, ARG_FMEM },
  { "sts",		MEM(0x26), ALL, ARG_FMEM },
  { "stt",		MEM(0x27), ALL, ARG_FMEM },

  { "ldl",		MEM(0x28), ALL, ARG_MEM },
  { "ldq",		MEM(0x29), ALL, ARG_MEM },
  { "ldl_l",		MEM(0x2A), ALL, ARG_MEM },
  { "ldq_l",		MEM(0x2B), ALL, ARG_MEM },
  { "stl",		MEM(0x2C), ALL, ARG_MEM },
  { "stq",		MEM(0x2D), ALL, ARG_MEM },
  { "stl_c",		MEM(0x2E), ALL, ARG_MEM },
  { "stq_c",		MEM(0x2F), ALL, ARG_MEM },

  { "br",		BRA(0x30), ALL, { ZA, BDISP } },	/* pseudo */
  { "br",		BRA(0x30), ALL, ARG_BRA },
  { "fbeq",		BRA(0x31), ALL, ARG_FBRA },
  { "fblt",		BRA(0x32), ALL, ARG_FBRA },
  { "fble",		BRA(0x33), ALL, ARG_FBRA },
  { "bsr",		BRA(0x34), ALL, ARG_BRA },
  { "fbne",		BRA(0x35), ALL, ARG_FBRA },
  { "fbge",		BRA(0x36), ALL, ARG_FBRA },
  { "fbgt",		BRA(0x37), ALL, ARG_FBRA },
  { "blbc",		BRA(0x38), ALL, ARG_BRA },
  { "beq",		BRA(0x39), ALL, ARG_BRA },
  { "blt",		BRA(0x3A), ALL, ARG_BRA },
  { "ble",		BRA(0x3B), ALL, ARG_BRA },
  { "blbs",		BRA(0x3C), ALL, ARG_BRA },
  { "bne",		BRA(0x3D), ALL, ARG_BRA },
  { "bge",		BRA(0x3E), ALL, ARG_BRA },
  { "bgt",		BRA(0x3F), ALL, ARG_BRA },
};

const int alpha_num_opcodes = sizeof(alpha_opcodes)/sizeof(*alpha_opcodes);
