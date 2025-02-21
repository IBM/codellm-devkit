/* tc-w65.c -- Assemble code for the W65816

   Copyright (C) 1995 Free Software Foundation.

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

/*
   Written By Steve Chamberlain
   sac@cygnus.com
 */

#include <stdio.h>
#include "as.h"
#include "bfd.h"
#include "subsegs.h"
#define DEFINE_TABLE
#include "../opcodes/w65-opc.h"
#include <ctype.h>

const char comment_chars[] = "!";
CONST char line_separator_chars[] = ";";
const char line_comment_chars[] = "!#";

/* This table describes all the machine specific pseudo-ops the assembler
   has to support.  The fields are:
   pseudo-op name without dot
   function to call to execute this pseudo-op
   Integer arg to pass to the function
 */

#define OP_BCC	0x90
#define OP_BCS	0xB0
#define OP_BEQ	0xF0
#define OP_BMI	0x30
#define OP_BNE	0xD0
#define OP_BPL	0x10
#define OP_BRA	0x80
#define OP_BRL	0x82
#define OP_BVC	0x50
#define OP_BVS	0x70

void s_longa ();
const pseudo_typeS md_pseudo_table[] =
{
  {"int", cons, 2},
  {"word", cons, 2},
  {"longa", s_longa, 0},
  {"longi", s_longa, 1},
  {0, 0, 0}
};


void cons ();
void s_align_bytes ();


/*int md_reloc_size; */

static int relax;		/* set if -relax seen */

const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant */
/* As in 0f12.456 */
/* or    0d1.2345e12 */
const char FLT_CHARS[] = "rRsSfFdDxXpP";



static struct hash_control *opcode_hash_control;	/* Opcode mnemonics */

int M;				/* M flag */
int X;				/* X flag */




#define C(a,b) ENCODE_RELAX(a,b)
#define ENCODE_RELAX(what,length) (((what) << 2) + (length))

#define GET_WHAT(x) ((x>>2))

#define BYTE_DISP 1
#define WORD_DISP 2
#define UNDEF_BYTE_DISP 0
#define UNDEF_WORD_DISP 3

#define COND_BRANCH 	1
#define UNCOND_BRANCH   2
#define END 	3

#define BYTE_F 127		/* How far we can branch forwards */
#define BYTE_B -126		/* How far we can branch backwards */
#define WORD_F 32767
#define WORD_B 32768

relax_typeS md_relax_table[C (END, 0)];

/*
   This function is called once, at assembler startup time.  This should
   set up all the tables, etc that the MD part of the assembler needs
 */


void
s_longa (xmode)
{
  int *p = xmode ? &X : &M;
  while (*input_line_pointer == ' ')
    input_line_pointer++;
  if (strncmp (input_line_pointer, "on", 2) == 0)
    {
      input_line_pointer += 2;
      *p = 0;
    }
  else if (strncmp (input_line_pointer, "off", 3) == 0)
    {
      *p = 1;
      input_line_pointer += 3;
    }
  else
    as_bad ("need on or off.");
  demand_empty_rest_of_line ();
}
void
md_begin ()
{
  relax_typeS *table;
  struct opinfo *opcode;
  char *prev_name = "";

  opcode_hash_control = hash_new ();

  /* Insert unique names into hash table */
  for (opcode = optable; opcode->name; opcode++)
    {
      if (strcmp (prev_name, opcode->name))
	{
	  prev_name = opcode->name;
	  hash_insert (opcode_hash_control, opcode->name, (char *) opcode);
	}
      else
	{
	  /* Make all the opcodes with the same name point to the same
	     string */
	  opcode->name = prev_name;
	}
    }


  /* Initialize the relax table.  We use a local variable to avoid
     warnings about modifying a supposedly const data structure.  */
  table = (relax_typeS *) md_relax_table;
  table[C (COND_BRANCH, BYTE_DISP)].rlx_forward = BYTE_F;
  table[C (COND_BRANCH, BYTE_DISP)].rlx_backward = BYTE_B;
  table[C (COND_BRANCH, BYTE_DISP)].rlx_length = 2;
  table[C (COND_BRANCH, BYTE_DISP)].rlx_more = C (COND_BRANCH, WORD_DISP);

  table[C (COND_BRANCH, WORD_DISP)].rlx_forward = WORD_F;
  table[C (COND_BRANCH, WORD_DISP)].rlx_backward = WORD_B;
  table[C (COND_BRANCH, WORD_DISP)].rlx_length = 5;
  table[C (COND_BRANCH, WORD_DISP)].rlx_more = 0;

  table[C (UNCOND_BRANCH, BYTE_DISP)].rlx_forward = BYTE_F;
  table[C (UNCOND_BRANCH, BYTE_DISP)].rlx_backward = BYTE_B;
  table[C (UNCOND_BRANCH, BYTE_DISP)].rlx_length = 2;
  table[C (UNCOND_BRANCH, BYTE_DISP)].rlx_more = C (UNCOND_BRANCH, WORD_DISP);

  table[C (UNCOND_BRANCH, WORD_DISP)].rlx_forward = WORD_F;
  table[C (UNCOND_BRANCH, WORD_DISP)].rlx_backward = WORD_B;
  table[C (UNCOND_BRANCH, WORD_DISP)].rlx_length = 3;
  table[C (UNCOND_BRANCH, WORD_DISP)].rlx_more = 0;

  flag_signed_overflow_ok = 1;
}

static expressionS immediate;	/* absolute expression */
static expressionS immediate1;	/* absolute expression */


static symbolS *
dot ()
{
  const char *fake;

  /* JF: '.' is pseudo symbol with value of current location
     in current segment.  */
  fake = FAKE_LABEL_NAME;
  return symbol_new (fake,
		     now_seg,
		     (valueT) frag_now_fix (),
		     frag_now);

}

int expr_size;
int expr_shift;
int tc_cons_reloc;
void
w65_expression (dest, bytes)
     expressionS *dest;
     unsigned int bytes;
{
  expr_size = 0;
  expr_shift = 0;
  tc_cons_reloc = 0;
  while (*input_line_pointer == ' ')
    input_line_pointer++;

  if (*input_line_pointer == '<')
    {
      expr_size = 1;
      input_line_pointer++;
    }
  else if (*input_line_pointer == '>')
    {
      expr_shift = 1;
      input_line_pointer++;
    }
  else if (*input_line_pointer == '^')
    {
      expr_shift = 2;
      input_line_pointer++;
    }

  expr (0, dest);
}

int amode;
static
char *
parse_exp (s, bytes)
     char *s;
     int bytes;
{
  char *save;
  char *new;

  save = input_line_pointer;
  input_line_pointer = s;
  w65_expression (&immediate, bytes);
  if (immediate.X_op == O_absent)
    as_bad ("missing operand");
  new = input_line_pointer;
  input_line_pointer = save;
  return new;
}


static
char *
get_operands (info, ptr)
     struct opinfo *info;
     char *ptr;
{
  register int override_len = 0;
  register int bytes = 0;
  while (*ptr == ' ')
    ptr++;

  if (ptr[0] == '#')
    {
      ptr++;
      switch (info->amode)
	{
	case ADDR_IMMTOI:
	  bytes = X ? 1 : 2;
	  amode = ADDR_IMMTOI;
	  break;
	case ADDR_IMMTOA:
	  bytes = M ? 1 : 2;
	  amode = ADDR_IMMTOA;
	  break;
	case ADDR_IMMCOP:
	  bytes = 1;
	  amode = ADDR_IMMCOP;
	  break;
	case ADDR_DIR:
	  bytes = 2;
	  amode = ADDR_ABS;
	  break;
	default:
	  abort ();
	  break;
	}
      ptr = parse_exp (ptr);
    }
  else if (ptr[0] == '!')
    {
      ptr = parse_exp (ptr + 1);
      if (ptr[0] == ',')
	{
	  if (ptr[1] == 'y')
	    {
	      amode = ADDR_ABS_IDX_Y;
	      bytes = 2;
	      ptr += 2;
	    }
	  else if (ptr[1] == 'x')
	    {
	      amode = ADDR_ABS_IDX_X;
	      bytes = 2;
	      ptr += 2;
	    }
	  else
	    {
	      as_bad ("syntax error after <exp");
	    }
	}
      else
	{
	  amode = ADDR_ABS;
	  bytes = 2;
	}
    }
  else if (ptr[0] == '>')
    {
      ptr = parse_exp (ptr + 1);
      if (ptr[0] == ',' && ptr[1] == 'x')
	{
	  amode = ADDR_ABS_LONG_IDX_X;
	  bytes = 3;
	  ptr += 2;
	}
      else
	{
	  amode = ADDR_ABS_LONG;
	  bytes = 3;
	}
    }
  else if (ptr[0] == '<')
    {
      ptr = parse_exp (ptr + 1);
      if (ptr[0] == ',')
	{
	  if (ptr[1] == 'y')
	    {
	      amode = ADDR_DIR_IDX_Y;
	      ptr += 2;
	      bytes = 2;
	    }
	  else if (ptr[1] == 'x')
	    {
	      amode = ADDR_DIR_IDX_X;
	      ptr += 2;
	      bytes = 2;
	    }
	  else
	    {
	      as_bad ("syntax error after <exp");
	    }
	}
      else
	{
	  amode = ADDR_DIR;
	  bytes = 1;
	}
    }
  else if (ptr[0] == 'a')
    {
      amode = ADDR_ACC;
    }
  else if (ptr[0] == '(')
    {
      /* Look for (exp),y
	 (<exp),y
	 (exp,x)
	 (<exp,x)
	 (exp)
	 (!exp)
	 (exp)
	 (<exp)
	 (exp,x)
	 (!exp,x)
	 (exp,s)
	 (exp,s),y */

      ptr++;
      if (ptr[0] == '<')
	{
	  override_len = 1;
	  ptr++;
	}
      else if (ptr[0] == '!')
	{
	  override_len = 2;
	  ptr++;
	}
      else if (ptr[0] == '>')
	{
	  override_len = 3;
	  ptr++;
	}
      else
	{
	  override_len = 0;
	}
      ptr = parse_exp (ptr);

      if (ptr[0] == ',')
	{
	  ptr++;
	  if (ptr[0] == 'x' && ptr[1] == ')')
	    {
	      ptr += 2;

	      if (override_len == 1)
		{
		  amode = ADDR_DIR_IDX_IND_X;
		  bytes = 2;
		}
	      else
		{
		  amode = ADDR_ABS_IND_IDX;
		  bytes = 2;
		}
	    }
	  else if (ptr[0] == 's' && ptr[1] == ')' && ptr[2] == ',' && ptr[3] == 'y')
	    {
	      amode = ADDR_STACK_REL_INDX_IDX;
	      bytes = 1;
	      ptr += 4;
	    }
	}
      else if (ptr[0] == ')')
	{
	  if (ptr[1] == ',' && ptr[2] == 'y')
	    {
	      amode = ADDR_DIR_IND_IDX_Y;
	      ptr += 3;
	      bytes = 2;
	    }
	  else
	    {
	      if (override_len == 1)
		{
		  amode = ADDR_DIR_IND;
		  bytes = 1;
		}
	      else
		{
		  amode = ADDR_ABS_IND;
		  bytes = 2;
		}
	      ptr++;

	    }
	}

    }
  else if (ptr[0] == '[')
    {
      ptr = parse_exp (ptr + 1);
      if (ptr[0] == ']')
	{
	  ptr++;
	  if (ptr[0] == ',' && ptr[1] == 'y')
	    {
	      bytes = 1;
	      amode = ADDR_DIR_IND_IDX_Y_LONG;
	      ptr += 2;
	    }
	  else
	    {
	      if (info->code == O_jmp) 
		{
		bytes = 2;
		amode = ADDR_ABS_IND_LONG;
	      } 
	      else 
{
		bytes = 1;
	      
		amode = ADDR_DIR_IND_LONG;
	      }
	    }
	}
    }
  else
    {
      ptr = parse_exp (ptr, 2);
      if (ptr[0] == ',')
	{
	  if (ptr[1] == 'y')
	    {
	      if (override_len == 1)
		{
		  bytes = 1;
		  amode = ADDR_DIR_IDX_Y;
		}
	      else
		{
		  amode = ADDR_ABS_IDX_Y;
		  bytes = 2;
		}
	      ptr += 2;
	    }
	  else if (ptr[1] == 'x')
	    {
	      if (override_len == 1)
		{
		  amode = ADDR_DIR_IDX_X;
		  bytes = 1;
		}
	      else
		{
		  amode = ADDR_ABS_IDX_X;
		  bytes = 2;
		}
	      ptr += 2;
	    }
	  else if (ptr[1] == 's')
	    {
	      bytes = 1;
	      amode = ADDR_STACK_REL;
	      ptr += 2;
	    }
	  else
	    {
	      bytes = 1;
	      immediate1 = immediate;
	      ptr = parse_exp (ptr + 1);
	      amode = ADDR_BLOCK_MOVE;
	    }
	}
      else
	{
	  switch (info->amode)
	    {
	    case ADDR_PC_REL:
	      amode = ADDR_PC_REL;
	      bytes = 1;
	      break;
	    case ADDR_PC_REL_LONG:
	      amode = ADDR_PC_REL_LONG;
	      bytes = 2;
	      break;
	    default:
	      if (override_len == 1)
		{
		  amode = ADDR_DIR;
		  bytes = 1;
		}
	      else if (override_len == 3)
		{
		  bytes = 3;
		  amode = ADDR_ABS_LONG;
		}
	      else
		{
		  amode = ADDR_ABS;
		  bytes = 2;
		}
	    }
	}
    }

  switch (bytes)
    {
    case 1:
      switch (expr_shift)
	{
	case 0:
	  if (amode == ADDR_DIR)
	tc_cons_reloc = R_W65_DP;
	  else
tc_cons_reloc = R_W65_ABS8;
	  break;
	case 1:
	  tc_cons_reloc = R_W65_ABS8S8;
	  break;
	case 2:
	  tc_cons_reloc = R_W65_ABS8S16;
	  break;
	}
      break;
    case 2:
      switch (expr_shift)
	{
	case 0:
	  tc_cons_reloc = R_W65_ABS16;
	  break;
	case 1:
	  tc_cons_reloc = R_W65_ABS16S8;
	  break;
	case 2:
	  tc_cons_reloc = R_W65_ABS16S16;
	  break;
	}
    }
  return ptr;
}

/* Passed a pointer to a list of opcodes which use different
   addressing modes, return the opcode which matches the opcodes
   provided
 */

static
struct opinfo *
get_specific (opcode)
     struct opinfo *opcode;
{
  int ocode = opcode->code;

  for (; opcode->code == ocode; opcode++)
    {
      if (opcode->amode == amode)
	return opcode;
    }
  return 0;
}

int
check (operand, low, high)
     expressionS *operand;
     int low;
     int high;
{
  if (operand->X_op != O_constant
      || operand->X_add_number < low
      || operand->X_add_number > high)
    {
      as_bad ("operand must be absolute in range %d..%d", low, high);
    }
  return operand->X_add_number;
}


static int log2[] =
{0, 0, 1, 0, 2};

/* Now we know what sort of opcodes it is, lets build the bytes -
 */
static void
build_Mytes (opcode)
     struct opinfo *opcode;
{
  int size;
  int type;
  int pcrel;
  char *output;

  if (opcode->amode == ADDR_IMPLIED)
    {
      output = frag_more (1);
    }
  else if (opcode->amode == ADDR_PC_REL)
    {
      int type;
      /* This is a relaxable insn, so we do some special handling */
      type = opcode->val == OP_BRA ? UNCOND_BRANCH : COND_BRANCH;
      output = frag_var (rs_machine_dependent,
			 md_relax_table[C (type, WORD_DISP)].rlx_length,
			 md_relax_table[C (type, BYTE_DISP)].rlx_length,
			 C (type, UNDEF_BYTE_DISP),
			 immediate.X_add_symbol,
			 immediate.X_add_number,
			 0);
    }
  else
    {
      switch (opcode->amode)
	{
	  GETINFO (size, type, pcrel);
	}

      /* If something special was done in the
	 expression modify the reloc type */
      if (tc_cons_reloc)
	{
	  type = tc_cons_reloc;
	}



      /* 1 byte for the opcode + the bytes for the addrmode */
      output = frag_more (size + 1);

      if (opcode->amode == ADDR_BLOCK_MOVE)
	{
	  /* Two relocs for this one */
	  fix_new_exp (frag_now,
		       output + 1 - frag_now->fr_literal,
		       1,
		       &immediate,
		       0,
		       R_W65_ABS8S16);

	  fix_new_exp (frag_now,
		       output + 2 - frag_now->fr_literal,
		       1,
		       &immediate1,
		       0,
		       R_W65_ABS8S16);
	}
      else if (type >= 0
	       && opcode->amode != ADDR_IMPLIED
	       && opcode->amode != ADDR_ACC
	       && opcode->amode != ADDR_STACK)
	{
	  fix_new_exp (frag_now,
		       output + 1 - frag_now->fr_literal,
		       size,
		       &immediate,
		       pcrel,
		       type);
	}
    }
  output[0] = opcode->val;
}

/* This is the guts of the machine-dependent assembler.  STR points to a
   machine dependent instruction.  This function is supposed to emit
   the frags/bytes it assembles to.
 */

void
md_assemble (str)
     char *str;
{
  unsigned char *op_start;
  unsigned char *op_end;
  struct opinfo *opcode;
  char name[20];
  int nlen = 0;
  char *p;

  /* Drop leading whitespace */
  while (*str == ' ')
    str++;

  /* all opcodes are three letters */
  name[0] = str[0];
  name[1] = str[1];
  name[2] = str[2];
  name[3] = 0;

  tc_cons_reloc = 0;
  str += 3;
  opcode = (struct opinfo *) hash_find (opcode_hash_control, name);

  if (opcode == NULL)
    {
      as_bad ("unknown opcode");
      return;
    }

  if (opcode->amode != ADDR_IMPLIED
      && opcode->amode != ADDR_STACK)
    {
      get_operands (opcode, str);
      opcode = get_specific (opcode);
    }

  if (opcode == 0)
    {
      /* Couldn't find an opcode which matched the operands */


      char *where = frag_more (1);

      where[0] = 0x0;
      where[1] = 0x0;
      as_bad ("invalid operands for opcode");
      return;
    }

  build_Mytes (opcode);
}


void
DEFUN (tc_crawl_symbol_chain, (headers),
       object_headers * headers)
{
  printf ("call to tc_crawl_symbol_chain \n");
}

symbolS *
DEFUN (md_undefined_symbol, (name),
       char *name)
{
  return 0;
}

void
DEFUN (tc_headers_hook, (headers),
       object_headers * headers)
{
  printf ("call to tc_headers_hook \n");
}

/* Various routines to kill one day */
/* Equal to MAX_PRECISION in atof-ieee.c */
#define MAX_LITTLENUMS 6

/* Turn a string in input_line_pointer into a floating point constant of type
   type, and store the appropriate bytes in *litP.  The number of LITTLENUMS
   emitted is stored in *sizeP .  An error message is returned, or NULL on OK.
 */
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
  char *atof_ieee ();

  switch (type)
    {
    case 'f':
    case 'F':
    case 's':
    case 'S':
      prec = 2;
      break;

    case 'd':
    case 'D':
    case 'r':
    case 'R':
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
      return "Bad call to MD_NTOF()";
    }
  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;

  *sizeP = prec * sizeof (LITTLENUM_TYPE);
  for (wordP = words + prec - 1; prec--;)
    {
      md_number_to_chars (litP, (valueT) (*wordP--), sizeof (LITTLENUM_TYPE));
      litP += sizeof (LITTLENUM_TYPE);
    }
  return 0;
}

int
md_parse_option (c,a)
    int c;
    char *a;

{
  return 1;
}

int md_short_jump_size;

void
tc_Nout_fix_to_chars ()
{
  printf ("call to tc_Nout_fix_to_chars \n");
  abort ();
}

void
md_create_short_jump (ptr, from_Nddr, to_Nddr, frag, to_symbol)
     char *ptr;
     addressT from_Nddr;
     addressT to_Nddr;
     fragS *frag;
     symbolS *to_symbol;
{
  as_fatal ("failed sanity check.");
}

void
md_create_long_jump (ptr, from_Nddr, to_Nddr, frag, to_symbol)
     char *ptr;
     addressT from_Nddr, to_Nddr;
     fragS *frag;
     symbolS *to_symbol;
{
  as_fatal ("failed sanity check.");
}


/*
called after relaxing, change the frags so they know how big they are
*/
void
md_convert_frag (headers, seg, fragP)
     object_headers *headers;
     segT seg;
     fragS *fragP;
{
  int disp_size = 0;
  int inst_size = 0;
  unsigned char *buffer = (unsigned char *) (fragP->fr_fix + fragP->fr_literal);

  switch (fragP->fr_subtype)
    {
    case C (COND_BRANCH, BYTE_DISP):
    case C (UNCOND_BRANCH, BYTE_DISP):
      disp_size = 1;
      inst_size = 1;
      break;

      /* cond branches to a known 16 bit displacement */
    case C (COND_BRANCH, WORD_DISP):
      switch (buffer[0])
	{
	case OP_BCC:
	case OP_BCS:
	case OP_BEQ:
	case OP_BMI:
	case OP_BNE:
	case OP_BPL:
	case OP_BVS:
	case OP_BVC:
	  /* Invert the sense of the test */
	  buffer[0] ^= 0x20;
	  buffer[1] = 3;	/* Jump over following brl */
	  buffer[2] = OP_BRL;
	  buffer[3] = 0;
	  buffer[4] = 0;
	  disp_size = 2;
	  inst_size = 3;
	  break;
	default:
	  abort ();
	}
      break;
    case C (UNCOND_BRANCH, WORD_DISP):
      /* Unconditional branches to a known 16 bit displacement */

      switch (buffer[0])
	{
	case OP_BRA:
	  buffer[0] = OP_BRL;
	  disp_size = 2;
	  inst_size = 1;
	  break;
	default:
	  abort ();
	}
      break;
      /* got to create a branch over a reloc here */
    case C (COND_BRANCH, UNDEF_WORD_DISP):
      buffer[0] ^= 0x20;	/* invert test */
      buffer[1] = 3;
      buffer[2] = OP_BRL;
      buffer[3] = 0;
      buffer[4] = 0;
      fix_new (fragP,
	       fragP->fr_fix + 3,
	       4,
	       fragP->fr_symbol,
	       fragP->fr_offset,
	       0,
	       R_W65_PCR16);

      fragP->fr_fix += disp_size + inst_size;
      fragP->fr_var = 0;
      break;
    case C (UNCOND_BRANCH, UNDEF_WORD_DISP):
      buffer[0] = OP_BRL;
      buffer[1] = 0;
      buffer[2] = 0;
      fix_new (fragP,
	       fragP->fr_fix + 1,
	       4,
	       fragP->fr_symbol,
	       fragP->fr_offset,
	       0,
	       R_W65_PCR16);

      fragP->fr_fix += disp_size + inst_size;
      fragP->fr_var = 0;
      break;
    default:
      abort ();
    }
  if (inst_size)
    {
      /* Get the address of the end of the instruction */
      int next_inst = fragP->fr_fix + fragP->fr_address + disp_size + inst_size;
      int targ_addr = (S_GET_VALUE (fragP->fr_symbol) +
		       fragP->fr_offset);
      int disp = targ_addr - next_inst;

      md_number_to_chars (buffer + inst_size, disp, disp_size);
      fragP->fr_fix += disp_size + inst_size;
      fragP->fr_var = 0;
    }
}


valueT
DEFUN (md_section_align, (seg, size),
       segT seg AND
       valueT size)
{
  return ((size + (1 << section_alignment[(int) seg]) - 1)
	  & (-1 << section_alignment[(int) seg]));

}

void
md_apply_fix (fixP, val)
     fixS *fixP;
     long val;
{
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  int addr = fixP->fx_frag->fr_address + fixP->fx_where;

  if (fixP->fx_r_type == 0)
    {
      if (fixP->fx_size == 1)
	fixP->fx_r_type = R_W65_ABS8;
      else
	fixP->fx_r_type = R_W65_ABS16;
    }

  switch (fixP->fx_r_type)
    {
    case R_W65_ABS8S16:
      val >>= 8;
    case R_W65_ABS8S8:
      val >>= 8;
    case R_W65_ABS8:
      *buf++ = val;
      break;
    case R_W65_ABS16S16:
      val >>= 8;
    case R_W65_ABS16S8:
      val >>= 8;
    case R_W65_ABS16:
      *buf++ = val >> 0;
      *buf++ = val >> 8;
      break;
    case R_W65_ABS24:
      *buf++ = val >> 0;
      *buf++ = val >> 8;
      *buf++ = val >> 16;
      break;
    case R_W65_PCR8:
      *buf++ = val - addr - 1;
      break;
    case R_W65_PCR16:
      val = val - addr - 1;
      *buf++ = val;
      *buf++ = val >> 8;
      break;
    case R_W65_DP:
      *buf++ = val;
      break;

    default:
      abort ();
    }
}

int md_long_jump_size;

/* Put number into target byte order */

void
md_number_to_chars (ptr, use, nbytes)
     char *ptr;
     valueT use;
     int nbytes;
{
  number_to_chars_littleendian (ptr, use, nbytes);
}

long
md_pcrel_from (fixP)
     fixS *fixP;
{
  int gap = fixP->fx_size + fixP->fx_where + fixP->fx_frag->fr_address - 1;
  return gap;
}

void
tc_coff_symbol_emit_hook (x)
    struct symbol *x;
{
}

short
tc_coff_fix2rtype (fix_ptr)
     fixS *fix_ptr;
{
  return fix_ptr->fx_r_type;
}

void
tc_reloc_mangle (fix_ptr, intr, base)
     fixS *fix_ptr;
     struct internal_reloc *intr;
     bfd_vma base;

{
  symbolS *symbol_ptr;

  symbol_ptr = fix_ptr->fx_addsy;

  /* If this relocation is attached to a symbol then it's ok
     to output it */
  if (fix_ptr->fx_r_type == RELOC_32)
    {
      /* cons likes to create reloc32's whatever the size of the reloc..
       */
      switch (fix_ptr->fx_size)
	{
	case 2:
	  intr->r_type = R_IMM16;
	  break;
	case 1:
	  intr->r_type = R_IMM8;
	  break;
	default:
	  abort ();
	}
    }
  else
    {
      if (fix_ptr->fx_size == 4)
	intr->r_type = R_W65_ABS24;
      else
	intr->r_type = fix_ptr->fx_r_type;
    }

  intr->r_vaddr = fix_ptr->fx_frag->fr_address + fix_ptr->fx_where + base;
  intr->r_offset = fix_ptr->fx_offset;

  /* Turn the segment of the symbol into an offset.  */
  if (symbol_ptr)
    {
      symbolS *dot;

      dot = segment_info[S_GET_SEGMENT (symbol_ptr)].dot;
      if (dot)
	{
	  intr->r_offset += S_GET_VALUE (symbol_ptr);
	  intr->r_symndx = dot->sy_number;
	}
      else
	{
	  intr->r_symndx = symbol_ptr->sy_number;
	}
    }
  else
    {
      intr->r_symndx = -1;
    }
}

int
tc_coff_sizemachdep (frag)
     fragS *frag;
{
  return md_relax_table[frag->fr_subtype].rlx_length;
}





/*
called just before address relaxation, return the length
by which a fragment must grow to reach it's destination
*/
int
md_estimate_size_before_relax (fragP, segment_type)
     register fragS *fragP;
     register segT segment_type;
{
  int what = GET_WHAT (fragP->fr_subtype);

  switch (fragP->fr_subtype)
    {
    default:
      abort ();
    case C (COND_BRANCH, UNDEF_BYTE_DISP):
    case C (UNCOND_BRANCH, UNDEF_BYTE_DISP):
      /* used to be a branch to somewhere which was unknown */
      if (S_GET_SEGMENT (fragP->fr_symbol) == segment_type)
	{
	  /* Got a symbol and it's defined in this segment, become byte
	 sized - maybe it will fix up */
	  fragP->fr_subtype = C (what, BYTE_DISP);
	  fragP->fr_var = md_relax_table[C (what, BYTE_DISP)].rlx_length;
	}
      else
	{
	  /* Its got a segment, but its not ours, so it will always be long */
	  fragP->fr_subtype = C (what, UNDEF_WORD_DISP);
	  fragP->fr_var = md_relax_table[C (what, WORD_DISP)].rlx_length;
	  return md_relax_table[C (what, WORD_DISP)].rlx_length;
	}
    }
  return fragP->fr_var;
}



CONST char *md_shortopts = "";
struct option md_longopts[] = {
#define OPTION_RELAX (OPTION_MD_BASE)
  {NULL, no_argument, NULL, 0}
};

void
md_show_usage (stream)
     FILE *stream;
{

}

size_t md_longopts_size = sizeof(md_longopts);
