/* tc-sh.c -- Assemble code for the Hitachi Super-H

   Copyright (C) 1993, 94, 95, 1996 Free Software Foundation.

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

/*
   Written By Steve Chamberlain
   sac@cygnus.com
 */

#include <stdio.h>
#include "as.h"
#include "bfd.h"
#include "subsegs.h"
#define DEFINE_TABLE
#include "opcodes/sh-opc.h"
#include <ctype.h>
const char comment_chars[] = "!";
const char line_separator_chars[] = ";";
const char line_comment_chars[] = "!#";

static void s_uses PARAMS ((int));

/* This table describes all the machine specific pseudo-ops the assembler
   has to support.  The fields are:
   pseudo-op name without dot
   function to call to execute this pseudo-op
   Integer arg to pass to the function
 */

void cons ();
void s_align_bytes ();

int shl = 0;

static void
little (ignore)
     int ignore;
{
  shl = 1;
  target_big_endian = 0;
}

const pseudo_typeS md_pseudo_table[] =
{
  {"int", cons, 4},
  {"word", cons, 2},
  {"form", listing_psize, 0},
  {"little", little, 0},
  {"heading", listing_title, 0},
  {"import", s_ignore, 0},
  {"page", listing_eject, 0},
  {"program", s_ignore, 0},
  {"uses", s_uses, 0},
  {0, 0, 0}
};

/*int md_reloc_size; */

int sh_relax;		/* set if -relax seen */

const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant */
/* As in 0f12.456 */
/* or    0d1.2345e12 */
const char FLT_CHARS[] = "rRsSfFdDxXpP";

#define C(a,b) ENCODE_RELAX(a,b)

#define JREG 14			/* Register used as a temp when relaxing */
#define ENCODE_RELAX(what,length) (((what) << 4) + (length))
#define GET_WHAT(x) ((x>>4))

/* These are the two types of relaxable instrction */
#define COND_JUMP 1
#define UNCOND_JUMP  2

#define UNDEF_DISP 0
#define COND8  1
#define COND12 2
#define COND32 3
#define UNCOND12 1
#define UNCOND32 2
#define UNDEF_WORD_DISP 4
#define END 5

#define UNCOND12 1
#define UNCOND32 2

/* Branch displacements are from the address of the branch plus
   four, thus all minimum and maximum values have 4 added to them.  */
#define COND8_F 258
#define COND8_M -252
#define COND8_LENGTH 2

/* There is one extra instruction before the branch, so we must add
   two more bytes to account for it.  */
#define COND12_F 4100
#define COND12_M -4090
#define COND12_LENGTH 6

/* ??? The minimum and maximum values are wrong, but this does not matter
   since this relocation type is not supported yet.  */
#define COND32_F (1<<30)
#define COND32_M -(1<<30)
#define COND32_LENGTH 14

#define UNCOND12_F 4098
#define UNCOND12_M -4092
#define UNCOND12_LENGTH 2

/* ??? The minimum and maximum values are wrong, but this does not matter
   since this relocation type is not supported yet.  */
#define UNCOND32_F (1<<30)
#define UNCOND32_M -(1<<30)
#define UNCOND32_LENGTH 14

const relax_typeS md_relax_table[C (END, 0)] = {
  { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 },
  { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 },

  { 0 },
  /* C (COND_JUMP, COND8) */
  { COND8_F, COND8_M, COND8_LENGTH, C (COND_JUMP, COND12) },
  /* C (COND_JUMP, COND12) */
  { COND12_F, COND12_M, COND12_LENGTH, C (COND_JUMP, COND32), },
  /* C (COND_JUMP, COND32) */
  { COND32_F, COND32_M, COND32_LENGTH, 0, },
  { 0 }, { 0 }, { 0 }, { 0 },
  { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 },

  { 0 },
  /* C (UNCOND_JUMP, UNCOND12) */
  { UNCOND12_F, UNCOND12_M, UNCOND12_LENGTH, C (UNCOND_JUMP, UNCOND32), },
  /* C (UNCOND_JUMP, UNCOND32) */
  { UNCOND32_F, UNCOND32_M, UNCOND32_LENGTH, 0, },
  { 0 }, { 0 }, { 0 }, { 0 }, { 0 },
  { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 },
};

static struct hash_control *opcode_hash_control;	/* Opcode mnemonics */

/*
   This function is called once, at assembler startup time.  This should
   set up all the tables, etc that the MD part of the assembler needs
 */

void
md_begin ()
{
  sh_opcode_info *opcode;
  char *prev_name = "";

  if (! shl)
    target_big_endian = 1;

  opcode_hash_control = hash_new ();

  /* Insert unique names into hash table */
  for (opcode = sh_table; opcode->name; opcode++)
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
}

static int reg_m;
static int reg_n;
static int reg_b;

static expressionS immediate;	/* absolute expression */

typedef struct
  {
    sh_arg_type type;
    int reg;
  }

sh_operand_info;

/* try and parse a reg name, returns number of chars consumed */
static int
parse_reg (src, mode, reg)
     char *src;
     int *mode;
     int *reg;
{
  /* We use !isalnum for the next character after the register name, to
     make sure that we won't accidentally recognize a symbol name such as
     'sram' as being a reference to the register 'sr'.  */

  if (src[0] == 'r')
    {
      if (src[1] >= '0' && src[1] <= '7' && strncmp(&src[2], "_bank", 5) == 0
	  && ! isalnum (src[7]))
	{
	  *mode = A_REG_B;
	  *reg  = (src[1] - '0');
	  return 7;
	}
    }

  if (src[0] == 'r')
    {
      if (src[1] == '1')
	{
	  if (src[2] >= '0' && src[2] <= '5' && ! isalnum (src[3]))
	    {
	      *mode = A_REG_N;
	      *reg = 10 + src[2] - '0';
	      return 3;
	    }
	}
      if (src[1] >= '0' && src[1] <= '9' && ! isalnum (src[2]))
	{
	  *mode = A_REG_N;
	  *reg = (src[1] - '0');
	  return 2;
	}
    }

  if (src[0] == 's' && src[1] == 's' && src[2] == 'r' && ! isalnum (src[3]))
    {
      *mode = A_SSR;
      return 3;
    }

  if (src[0] == 's' && src[1] == 'p' && src[2] == 'c' && ! isalnum (src[3]))
    {
      *mode = A_SPC;
      return 3;
    }

  if (src[0] == 's' && src[1] == 'r' && ! isalnum (src[2]))
    {
      *mode = A_SR;
      return 2;
    }

  if (src[0] == 's' && src[1] == 'p' && ! isalnum (src[2]))
    {
      *mode = A_REG_N;
      *reg = 15;
      return 2;
    }

  if (src[0] == 'p' && src[1] == 'r' && ! isalnum (src[2]))
    {
      *mode = A_PR;
      return 2;
    }
  if (src[0] == 'p' && src[1] == 'c' && ! isalnum (src[2]))
    {
      *mode = A_DISP_PC;
      return 2;
    }
  if (src[0] == 'g' && src[1] == 'b' && src[2] == 'r' && ! isalnum (src[3]))
    {
      *mode = A_GBR;
      return 3;
    }
  if (src[0] == 'v' && src[1] == 'b' && src[2] == 'r' && ! isalnum (src[3]))
    {
      *mode = A_VBR;
      return 3;
    }

  if (src[0] == 'm' && src[1] == 'a' && src[2] == 'c' && ! isalnum (src[4]))
    {
      if (src[3] == 'l')
	{
	  *mode = A_MACL;
	  return 4;
	}
      if (src[3] == 'h')
	{
	  *mode = A_MACH;
	  return 4;
	}
    }
  if (src[0] == 'f' && src[1] == 'r')
    {
      if (src[2] == '1')
	{
	  if (src[3] >= '0' && src[3] <= '5' && ! isalnum (src[4]))
	    {
	      *mode = F_REG_N;
	      *reg = 10 + src[3] - '0';
	      return 4;
	    }
	}
      if (src[2] >= '0' && src[2] <= '9' && ! isalnum (src[3]))
	{
	  *mode = F_REG_N;
	  *reg = (src[2] - '0');
	  return 3;
	}
    }
  if (src[0] == 'f' && src[1] == 'p' && src[2] == 'u' && src[3] == 'l'
      && ! isalnum (src[4]))
    {
      *mode = FPUL_N;
      return 4;
    }

  if (src[0] == 'f' && src[1] == 'p' && src[2] == 's' && src[3] == 'c'
      && src[4] == 'r' && ! isalnum (src[5]))
    {
      *mode = FPSCR_N;
      return 5;
    }

  return 0;
}

static symbolS *dot()
{
  const char *fake;

  /* JF: '.' is pseudo symbol with value of current location
     in current segment.  */
  fake = FAKE_LABEL_NAME;
  return  symbol_new (fake,
		      now_seg,
		      (valueT) frag_now_fix (),
		      frag_now);

}


static
char *
parse_exp (s)
     char *s;
{
  char *save;
  char *new;

  save = input_line_pointer;
  input_line_pointer = s;
  expression (&immediate);
  if (immediate.X_op == O_absent)
    as_bad ("missing operand");
  new = input_line_pointer;
  input_line_pointer = save;
  return new;
}


/* The many forms of operand:

   Rn                   Register direct
   @Rn                  Register indirect
   @Rn+                 Autoincrement
   @-Rn                 Autodecrement
   @(disp:4,Rn)
   @(disp:8,GBR)
   @(disp:8,PC)

   @(R0,Rn)
   @(R0,GBR)

   disp:8
   disp:12
   #imm8
   pr, gbr, vbr, macl, mach

 */

static
char *
parse_at (src, op)
     char *src;
     sh_operand_info *op;
{
  int len;
  int mode;
  src++;
  if (src[0] == '-')
    {
      /* Must be predecrement */
      src++;

      len = parse_reg (src, &mode, &(op->reg));
      if (mode != A_REG_N)
	as_bad ("illegal register after @-");

      op->type = A_DEC_N;
      src += len;
    }
  else if (src[0] == '(')
    {
      /* Could be @(disp, rn), @(disp, gbr), @(disp, pc),  @(r0, gbr) or
         @(r0, rn) */
      src++;
      len = parse_reg (src, &mode, &(op->reg));
      if (len && mode == A_REG_N)
	{
	  src += len;
	  if (op->reg != 0)
	    {
	      as_bad ("must be @(r0,...)");
	    }
	  if (src[0] == ',')
	    src++;
	  /* Now can be rn or gbr */
	  len = parse_reg (src, &mode, &(op->reg));
	  if (mode == A_GBR)
	    {
	      op->type = A_R0_GBR;
	    }
	  else if (mode == A_REG_N)
	    {
	      op->type = A_IND_R0_REG_N;
	    }
	  else
	    {
	      as_bad ("syntax error in @(r0,...)");
	    }
	}
      else
	{
	  /* Must be an @(disp,.. thing) */
	  src = parse_exp (src);
	  if (src[0] == ',')
	    src++;
	  /* Now can be rn, gbr or pc */
	  len = parse_reg (src, &mode, &op->reg);
	  if (len)
	    {
	      if (mode == A_REG_N)
		{
		  op->type = A_DISP_REG_N;
		}
	      else if (mode == A_GBR)
		{
		  op->type = A_DISP_GBR;
		}
	      else if (mode == A_DISP_PC)
		{
		  /* Turn a plain @(4,pc) into @(.+4,pc) */
		  if (immediate.X_op == O_constant) { 
		    immediate.X_add_symbol = dot();
		    immediate.X_op = O_symbol;
		  }
		  op->type = A_DISP_PC;
		}
	      else
		{
		  as_bad ("syntax error in @(disp,[Rn, gbr, pc])");
		}
	    }
	  else
	    {
	      as_bad ("syntax error in @(disp,[Rn, gbr, pc])");
	    }
	}
      src += len;
      if (src[0] != ')')
	as_bad ("expecting )");
      else
	src++;
    }
  else
    {
      src += parse_reg (src, &mode, &(op->reg));
      if (mode != A_REG_N)
	{
	  as_bad ("illegal register after @");
	}
      if (src[0] == '+')
	{
	  op->type = A_INC_N;
	  src++;
	}
      else
	{
	  op->type = A_IND_N;
	}
    }
  return src;
}

static void
get_operand (ptr, op)
     char **ptr;
     sh_operand_info *op;
{
  char *src = *ptr;
  int mode = -1;
  unsigned int len;

  if (src[0] == '#')
    {
      src++;
      *ptr = parse_exp (src);
      op->type = A_IMM;
      return;
    }

  else if (src[0] == '@')
    {
      *ptr = parse_at (src, op);
      return;
    }
  len = parse_reg (src, &mode, &(op->reg));
  if (len)
    {
      *ptr = src + len;
      op->type = mode;
      return;
    }
  else
    {
      /* Not a reg, the only thing left is a displacement */
      *ptr = parse_exp (src);
      op->type = A_DISP_PC;
      return;
    }
}

static
char *
get_operands (info, args, operand)
     sh_opcode_info *info;
     char *args;
     sh_operand_info *operand;

{
  char *ptr = args;
  if (info->arg[0])
    {
      ptr++;

      get_operand (&ptr, operand + 0);
      if (info->arg[1])
	{
	  if (*ptr == ',')
	    {
	      ptr++;
	    }
	  get_operand (&ptr, operand + 1);
	  if (info->arg[2])
	    {
	      if (*ptr == ',')
		{
		  ptr++;
		}
	      get_operand (&ptr, operand + 2);
	    }
	  else
	    {
	      operand[2].type = 0;
	    }
	}
      else
	{
	  operand[1].type = 0;
	  operand[2].type = 0;
	}
    }
  else
    {
      operand[0].type = 0;
      operand[1].type = 0;
      operand[2].type = 0;
    }
  return ptr;
}

/* Passed a pointer to a list of opcodes which use different
   addressing modes, return the opcode which matches the opcodes
   provided
 */

static
sh_opcode_info *
get_specific (opcode, operands)
     sh_opcode_info *opcode;
     sh_operand_info *operands;
{
  sh_opcode_info *this_try = opcode;
  char *name = opcode->name;
  int n = 0;
  while (opcode->name)
    {
      this_try = opcode++;
      if (this_try->name != name)
	{
	  /* We've looked so far down the table that we've run out of
	     opcodes with the same name */
	  return 0;
	}
      /* look at both operands needed by the opcodes and provided by
         the user - since an arg test will often fail on the same arg
         again and again, we'll try and test the last failing arg the
         first on each opcode try */

      for (n = 0; this_try->arg[n]; n++)
	{
	  sh_operand_info *user = operands + n;
	  sh_arg_type arg = this_try->arg[n];
	  switch (arg)
	    {
	    case A_IMM:
	    case A_BDISP12:
	    case A_BDISP8:
	    case A_DISP_GBR:
	    case A_DISP_PC:
	    case A_MACH:
	    case A_PR:
	    case A_MACL:
	      if (user->type != arg)
		goto fail;
	      break;
	    case A_R0:
	      /* opcode needs r0 */
	      if (user->type != A_REG_N || user->reg != 0)
		goto fail;
	      break;
	    case A_R0_GBR:
	      if (user->type != A_R0_GBR || user->reg != 0)
		goto fail;
	      break;
	    case F_FR0:
	      if (user->type != F_REG_N || user->reg != 0)
		goto fail;
	      break;

	    case A_REG_N:
	    case A_INC_N:
	    case A_DEC_N:
	    case A_IND_N:
	    case A_IND_R0_REG_N:
	    case A_DISP_REG_N:
	    case F_REG_N:
	    case FPUL_N:
	    case FPSCR_N:
	      /* Opcode needs rn */
	      if (user->type != arg)
		goto fail;
	      reg_n = user->reg;
	      break;
	    case A_GBR:
	    case A_SR:
	    case A_VBR:
	    case A_SSR:
	    case A_SPC:
	      if (user->type != arg)
		goto fail;
	      break;

            case A_REG_B:
	      if (user->type != arg)
		goto fail;
	      reg_b = user->reg;
	      break;

	    case A_REG_M:
	    case A_INC_M:
	    case A_DEC_M:
	    case A_IND_M:
	    case A_IND_R0_REG_M:
	    case A_DISP_REG_M:
	      /* Opcode needs rn */
	      if (user->type != arg - A_REG_M + A_REG_N)
		goto fail;
	      reg_m = user->reg;
	      break;

	    case F_REG_M:
	    case FPUL_M:
	    case FPSCR_M:
	      /* Opcode needs rn */
	      if (user->type != arg - F_REG_M + F_REG_N)
		goto fail;
	      reg_m = user->reg;
	      break;
	
	    default:
	      printf ("unhandled %d\n", arg);
	      goto fail;
	    }
	}
      return this_try;
    fail:;
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


static void
insert (where, how, pcrel)
     char *where;
     int how;
     int pcrel;
{
  fix_new_exp (frag_now,
	       where - frag_now->fr_literal,
	       2,
	       &immediate,
	       pcrel,
	       how);
}

static void
build_relax (opcode)
     sh_opcode_info *opcode;
{
  int high_byte = target_big_endian ? 0 : 1;
  char *p;

  if (opcode->arg[0] == A_BDISP8)
    {
      p = frag_var (rs_machine_dependent,
		    md_relax_table[C (COND_JUMP, COND32)].rlx_length,
		    md_relax_table[C (COND_JUMP, COND8)].rlx_length,
		    C (COND_JUMP, 0),
		    immediate.X_add_symbol,
		    immediate.X_add_number,
		    0);
      p[high_byte] = (opcode->nibbles[0] << 4) | (opcode->nibbles[1]);
    }
  else if (opcode->arg[0] == A_BDISP12)
    {
      p = frag_var (rs_machine_dependent,
		    md_relax_table[C (UNCOND_JUMP, UNCOND32)].rlx_length,
		    md_relax_table[C (UNCOND_JUMP, UNCOND12)].rlx_length,
		    C (UNCOND_JUMP, 0),
		    immediate.X_add_symbol,
		    immediate.X_add_number,
		    0);
      p[high_byte] = (opcode->nibbles[0] << 4);
    }

}

/* Now we know what sort of opcodes it is, lets build the bytes -
 */
static void
build_Mytes (opcode, operand)
     sh_opcode_info *opcode;
     sh_operand_info *operand;

{
  int index;
  char nbuf[4];
  char *output = frag_more (2);
  int low_byte = target_big_endian ? 1 : 0;
  nbuf[0] = 0;
  nbuf[1] = 0;
  nbuf[2] = 0;
  nbuf[3] = 0;

  for (index = 0; index < 4; index++)
    {
      sh_nibble_type i = opcode->nibbles[index];
      if (i < 16)
	{
	  nbuf[index] = i;
	}
      else
	{
	  switch (i)
	    {
	    case REG_N:
	      nbuf[index] = reg_n;
	      break;
	    case REG_M:
	      nbuf[index] = reg_m;
	      break;
            case REG_B:
	      nbuf[index] = reg_b | 0x08;
	      break;
	    case DISP_4:
	      insert (output + low_byte, R_SH_IMM4, 0);
	      break;
	    case IMM_4BY4:
	      insert (output + low_byte, R_SH_IMM4BY4, 0);
	      break;
	    case IMM_4BY2:
	      insert (output + low_byte, R_SH_IMM4BY2, 0);
	      break;
	    case IMM_4:
	      insert (output + low_byte, R_SH_IMM4, 0);
	      break;
	    case IMM_8BY4:
	      insert (output + low_byte, R_SH_IMM8BY4, 0);
	      break;
	    case IMM_8BY2:
	      insert (output + low_byte, R_SH_IMM8BY2, 0);
	      break;
	    case IMM_8:
	      insert (output + low_byte, R_SH_IMM8, 0);
	      break;
	    case PCRELIMM_8BY4:
	      insert (output, R_SH_PCRELIMM8BY4, 1);
	      break;
	    case PCRELIMM_8BY2:
	      insert (output, R_SH_PCRELIMM8BY2, 1);
	      break;
	    default:
	      printf ("failed for %d\n", i);
	    }
	}
    }
  if (! target_big_endian) {
    output[1] = (nbuf[0] << 4) | (nbuf[1]);
    output[0] = (nbuf[2] << 4) | (nbuf[3]);
  }
  else {
    output[0] = (nbuf[0] << 4) | (nbuf[1]);
    output[1] = (nbuf[2] << 4) | (nbuf[3]);
  }
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
  sh_operand_info operand[3];
  sh_opcode_info *opcode;
  char name[20];
  int nlen = 0;
  /* Drop leading whitespace */
  while (*str == ' ')
    str++;

  /* find the op code end */
  for (op_start = op_end = (unsigned char *) (str);
       *op_end
       && nlen < 20
       && !is_end_of_line[*op_end] && *op_end != ' ';
       op_end++)
    {
      name[nlen] = op_start[nlen];
      nlen++;
    }
  name[nlen] = 0;

  if (nlen == 0)
    {
      as_bad ("can't find opcode ");
    }

  opcode = (sh_opcode_info *) hash_find (opcode_hash_control, name);

  if (opcode == NULL)
    {
      as_bad ("unknown opcode");
      return;
    }

  if (sh_relax
      && ! seg_info (now_seg)->tc_segment_info_data.in_code)
    {
      /* Output a CODE reloc to tell the linker that the following
         bytes are instructions, not data.  */
      fix_new (frag_now, frag_now_fix (), 2, &abs_symbol, 0, 0, R_SH_CODE);
      seg_info (now_seg)->tc_segment_info_data.in_code = 1;
    }

  if (opcode->arg[0] == A_BDISP12
      || opcode->arg[0] == A_BDISP8)
    {
      parse_exp (op_end + 1);
      build_relax (opcode);
    }
  else
    {
      if (opcode->arg[0] != A_END)
	{
	  get_operands (opcode, op_end, operand);
	}
      opcode = get_specific (opcode, operand);

      if (opcode == 0)
	{
	  /* Couldn't find an opcode which matched the operands */
	  char *where = frag_more (2);

	  where[0] = 0x0;
	  where[1] = 0x0;
	  as_bad ("invalid operands for opcode");
	  return;
	}

      build_Mytes (opcode, operand);
    }

}

/* This routine is called each time a label definition is seen.  It
   emits a R_SH_LABEL reloc if necessary.  */

void
sh_frob_label ()
{
  static fragS *last_label_frag;
  static int last_label_offset;

  if (sh_relax
      && seg_info (now_seg)->tc_segment_info_data.in_code)
    {
      int offset;

      offset = frag_now_fix ();
      if (frag_now != last_label_frag
	  || offset != last_label_offset)
	{	
	  fix_new (frag_now, offset, 2, &abs_symbol, 0, 0, R_SH_LABEL);
	  last_label_frag = frag_now;
	  last_label_offset = offset;
	}
    }
}

/* This routine is called when the assembler is about to output some
   data.  It emits a R_SH_DATA reloc if necessary.  */

void
sh_flush_pending_output ()
{
  if (sh_relax
      && seg_info (now_seg)->tc_segment_info_data.in_code)
    {
      fix_new (frag_now, frag_now_fix (), 2, &abs_symbol, 0, 0, R_SH_DATA);
      seg_info (now_seg)->tc_segment_info_data.in_code = 0;
    }
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
     int type;
     char *litP;
     int *sizeP;
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
      *sizeP = 0;
      return "bad call to md_atof";
    }

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;

  *sizeP = prec * 2;

  if (! target_big_endian)
    {
      for (i = prec - 1; i >= 0; i--)
	{
	  md_number_to_chars (litP, (valueT) words[i], 2);
	  litP += 2;
	}
    }
  else
    {
      for (i = 0; i < prec; i++)
	{
	  md_number_to_chars (litP, (valueT) words[i], 2);
	  litP += 2;
	}
    }
     
  return NULL;
}

/* Handle the .uses pseudo-op.  This pseudo-op is used just before a
   call instruction.  It refers to a label of the instruction which
   loads the register which the call uses.  We use it to generate a
   special reloc for the linker.  */

static void
s_uses (ignore)
     int ignore;
{
  expressionS ex;

  if (! sh_relax)
    as_warn (".uses pseudo-op seen when not relaxing");

  expression (&ex);

  if (ex.X_op != O_symbol || ex.X_add_number != 0)
    {
      as_bad ("bad .uses format");
      ignore_rest_of_line ();
      return;
    }

  fix_new_exp (frag_now, frag_now_fix (), 2, &ex, 1, R_SH_USES);

  demand_empty_rest_of_line ();
}

CONST char *md_shortopts = "";
struct option md_longopts[] = {

#define OPTION_RELAX  (OPTION_MD_BASE)
#define OPTION_LITTLE (OPTION_MD_BASE+1)

  {"relax", no_argument, NULL, OPTION_RELAX},
  {"little", no_argument, NULL, OPTION_LITTLE},
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
    case OPTION_RELAX:
      sh_relax = 1;
      break;
    case OPTION_LITTLE:
      shl = 1;
      target_big_endian = 0;
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
  fprintf(stream, "\
SH options:\n\
-little			generate little endian code\n\
-relax			alter jump instructions for long displacements\n");
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

/* This is function is called after the symbol table has been
   completed, but before md_convert_frag has been called.  If we have
   seen any .uses pseudo-ops, they point to an instruction which loads
   a register with the address of a function.  We look through the
   fixups to find where the function address is being loaded from.  We
   then generate a COUNT reloc giving the number of times that
   function address is referred to.  The linker uses this information
   when doing relaxing, to decide when it can eliminate the stored
   function address entirely.  */

void
sh_coff_frob_file ()
{
  int iseg;

  if (! sh_relax)
    return;

  for (iseg = SEG_E0; iseg < SEG_UNKNOWN; iseg++)
    {
      fixS *fix;

      for (fix = segment_info[iseg].fix_root; fix != NULL; fix = fix->fx_next)
	{
	  symbolS *sym;
	  bfd_vma val;
	  fixS *fscan;
	  int iscan;
	  int count;

	  if (fix->fx_r_type != R_SH_USES)
	    continue;

	  /* The R_SH_USES reloc should refer to a defined local
             symbol in the same section.  */
	  sym = fix->fx_addsy;
	  if (sym == NULL
	      || fix->fx_subsy != NULL
	      || fix->fx_addnumber != 0
	      || S_GET_SEGMENT (sym) != iseg
	      || S_GET_STORAGE_CLASS (sym) == C_EXT)
	    {
	      as_warn_where (fix->fx_file, fix->fx_line,
			     ".uses does not refer to a local symbol in the same section");
	      continue;
	    }

	  /* Look through the fixups again, this time looking for one
             at the same location as sym.  */
	  val = S_GET_VALUE (sym);
	  for (fscan = segment_info[iseg].fix_root;
	       fscan != NULL;
	       fscan = fscan->fx_next)
	    if (val == fscan->fx_frag->fr_address + fscan->fx_where
		&& fscan->fx_r_type != R_SH_ALIGN
		&& fscan->fx_r_type != R_SH_CODE
		&& fscan->fx_r_type != R_SH_DATA
		&& fscan->fx_r_type != R_SH_LABEL)
	      break;
	  if (fscan == NULL)
	    {
	      as_warn_where (fix->fx_file, fix->fx_line,
			     "can't find fixup pointed to by .uses");
	      continue;
	    }

	  if (fscan->fx_tcbit)
	    {
	      /* We've already done this one.  */
	      continue;
	    }

	  /* fscan should also be a fixup to a local symbol in the same
             section.  */
	  sym = fscan->fx_addsy;
	  if (sym == NULL
	      || fscan->fx_subsy != NULL
	      || fscan->fx_addnumber != 0
	      || S_GET_SEGMENT (sym) != iseg
	      || S_GET_STORAGE_CLASS (sym) == C_EXT)
	    {
	      as_warn_where (fix->fx_file, fix->fx_line,
			     ".uses target does not refer to a local symbol in the same section");
	      continue;
	    }

	  /* Now we look through all the fixups of all the sections,
	     counting the number of times we find a reference to sym.  */
	  count = 0;
	  for (iscan = SEG_E0; iscan < SEG_UNKNOWN; iscan++)
	    {
	      for (fscan = segment_info[iscan].fix_root;
		   fscan != NULL;
		   fscan = fscan->fx_next)
		{
		  if (fscan->fx_addsy == sym)
		    {
		      ++count;
		      fscan->fx_tcbit = 1;
		    }
		}
	    }

	  if (count < 1)
	    abort ();

	  /* Generate a R_SH_COUNT fixup at the location of sym.  We
             have already adjusted the value of sym to include the
             fragment address, so we undo that adjustment here.  */
	  subseg_change (iseg, 0);
	  fix_new (sym->sy_frag, S_GET_VALUE (sym) - sym->sy_frag->fr_address,
		   4, &abs_symbol, count, 0, R_SH_COUNT);
	}
    }
}

/* Called after relaxing.  Set the correct sizes of the fragments, and
   create relocs so that md_apply_fix will fill in the correct values.  */

void
md_convert_frag (headers, seg, fragP)
     object_headers *headers;
     segT seg;
     fragS *fragP;
{
  int donerelax = 0;

  switch (fragP->fr_subtype)
    {
    case C (COND_JUMP, COND8):
      subseg_change (seg, 0);
      fix_new (fragP, fragP->fr_fix, 2, fragP->fr_symbol, fragP->fr_offset,
	       1, R_SH_PCDISP8BY2);
      fragP->fr_fix += 2;
      fragP->fr_var = 0;
      break;

    case C (UNCOND_JUMP, UNCOND12):
      subseg_change (seg, 0);
      fix_new (fragP, fragP->fr_fix, 2, fragP->fr_symbol, fragP->fr_offset,
	       1, R_SH_PCDISP);
      fragP->fr_fix += 2;
      fragP->fr_var = 0;
      break;

    case C (UNCOND_JUMP, UNCOND32):
    case C (UNCOND_JUMP, UNDEF_WORD_DISP):
      if (fragP->fr_symbol == NULL)
	as_bad ("at 0x%lx, displacement overflows 12-bit field",
		(unsigned long) fragP->fr_address);
      else
	as_bad ("at 0x%lx, displacement to %sdefined symbol %s overflows 12-bit field",
		(unsigned long) fragP->fr_address,		
		S_IS_DEFINED (fragP->fr_symbol) ? "" : "un",
		S_GET_NAME (fragP->fr_symbol));

#if 0				/* This code works, but generates poor code and the compiler
				   should never produce a sequence that requires it to be used.  */

      /* A jump wont fit in 12 bits, make code which looks like
	 bra foo
	 mov.w @(0, PC), r14
	 .long disp
	 foo: bra @r14
	 */
      int t = buffer[0] & 0x10;

      buffer[highbyte] = 0xa0;	/* branch over move and disp */
      buffer[lowbyte] = 3;
      buffer[highbyte+2] = 0xd0 | JREG;	/* Build mov insn */
      buffer[lowbyte+2] = 0x00;

      buffer[highbyte+4] = 0;	/* space for 32 bit jump disp */
      buffer[lowbyte+4] = 0;
      buffer[highbyte+6] = 0;
      buffer[lowbyte+6] = 0;

      buffer[highbyte+8] = 0x40 | JREG;	/* Build jmp @JREG */
      buffer[lowbyte+8] = t ? 0xb : 0x2b;

      buffer[highbyte+10] = 0x20; /* build nop */
      buffer[lowbyte+10] = 0x0b;

      /* Make reloc for the long disp */
      fix_new (fragP,
	       fragP->fr_fix + 4,
	       4,
	       fragP->fr_symbol,
	       fragP->fr_offset,
	       0,
	       R_SH_IMM32);
      fragP->fr_fix += UNCOND32_LENGTH;
      fragP->fr_var = 0;
      donerelax = 1;
#endif

      break;

    case C (COND_JUMP, COND12):
      /* A bcond won't fit, so turn it into a b!cond; bra disp; nop */
      {
	unsigned char *buffer =
	  (unsigned char *) (fragP->fr_fix + fragP->fr_literal);
	int highbyte = target_big_endian ? 0 : 1;
	int lowbyte = target_big_endian ? 1 : 0;

	/* Toggle the true/false bit of the bcond.  */
	buffer[highbyte] ^= 0x2;

	/* Build a relocation to six bytes farther on.  */
	subseg_change (seg, 0);
	fix_new (fragP, fragP->fr_fix, 2,
		 segment_info[seg].dot,
		 fragP->fr_address + fragP->fr_fix + 6,
		 1, R_SH_PCDISP8BY2);

	/* Set up a jump instruction.  */
	buffer[highbyte + 2] = 0xa0;
	buffer[lowbyte + 2] = 0;
	fix_new (fragP, fragP->fr_fix + 2, 2, fragP->fr_symbol,
		 fragP->fr_offset, 1, R_SH_PCDISP);

	/* Fill in a NOP instruction.  */
	buffer[highbyte + 4] = 0x0;
	buffer[lowbyte + 4] = 0x9;

	fragP->fr_fix += 6;
	fragP->fr_var = 0;
	donerelax = 1;
      }
      break;

    case C (COND_JUMP, COND32):
    case C (COND_JUMP, UNDEF_WORD_DISP):
      if (fragP->fr_symbol == NULL)
	as_bad ("at 0x%lx, displacement overflows 8-bit field", 
		(unsigned long) fragP->fr_address);
      else  
	as_bad ("at 0x%lx, displacement to %sdefined symbol %s overflows 8-bit field ",
		(unsigned long) fragP->fr_address,		
		S_IS_DEFINED (fragP->fr_symbol) ? "" : "un",
		S_GET_NAME (fragP->fr_symbol));

#if 0				/* This code works, but generates poor code, and the compiler
				   should never produce a sequence that requires it to be used.  */

      /* A bcond won't fit and it won't go into a 12 bit
	 displacement either, the code sequence looks like:
	 b!cond foop
	 mov.w @(n, PC), r14
	 jmp  @r14
	 nop
	 .long where
	 foop:
	 */

      buffer[0] ^= 0x2;		/* Toggle T/F bit */
#define JREG 14
      buffer[1] = 5;		/* branch over mov, jump, nop and ptr */
      buffer[2] = 0xd0 | JREG;	/* Build mov insn */
      buffer[3] = 0x2;
      buffer[4] = 0x40 | JREG;	/* Build jmp @JREG */
      buffer[5] = 0x0b;
      buffer[6] = 0x20;		/* build nop */
      buffer[7] = 0x0b;
      buffer[8] = 0;		/* space for 32 bit jump disp */
      buffer[9] = 0;
      buffer[10] = 0;
      buffer[11] = 0;
      buffer[12] = 0;
      buffer[13] = 0;
      /* Make reloc for the long disp */
      fix_new (fragP,
	       fragP->fr_fix + 8,
	       4,
	       fragP->fr_symbol,
	       fragP->fr_offset,
	       0,
	       R_SH_IMM32);
      fragP->fr_fix += COND32_LENGTH;
      fragP->fr_var = 0;
      donerelax = 1;
#endif

      break;

    default:
      abort ();
    }

  if (donerelax && !sh_relax)
    as_warn ("Offset doesn't fit at 0x%lx, trying to get to %s+0x%lx",
	     (unsigned long) fragP->fr_address,
	     fragP->fr_symbol ? S_GET_NAME(fragP->fr_symbol): "",
	     (unsigned long) fragP->fr_offset);
}

valueT
DEFUN (md_section_align, (seg, size),
       segT seg AND
       valueT size)
{
  return ((size + (1 << section_alignment[(int) seg]) - 1)
	  & (-1 << section_alignment[(int) seg]));

}

/* When relaxing, we need to output a reloc for any .align directive
   that requests alignment to a four byte boundary or larger.  */

void
sh_handle_align (frag)
     fragS *frag;
{
  if (sh_relax
      && frag->fr_type == rs_align
      && frag->fr_address + frag->fr_fix > 0
      && frag->fr_offset > 1
      && now_seg != bss_section)
    fix_new (frag, frag->fr_fix, 2, &abs_symbol, frag->fr_offset, 0,
	     R_SH_ALIGN);
}

/* This macro decides whether a particular reloc is an entry in a
   switch table.  It is used when relaxing, because the linker needs
   to know about all such entries so that it can adjust them if
   necessary.  */

#define SWITCH_TABLE(fix)				\
  ((fix)->fx_addsy != NULL				\
   && (fix)->fx_subsy != NULL				\
   && S_GET_SEGMENT ((fix)->fx_addsy) == text_section	\
   && S_GET_SEGMENT ((fix)->fx_subsy) == text_section	\
   && ((fix)->fx_r_type == R_SH_IMM32			\
       || (fix)->fx_r_type == R_SH_IMM16		\
       || ((fix)->fx_r_type == 0			\
	   && ((fix)->fx_size == 2			\
	       || (fix)->fx_size == 4))))

/* See whether we need to force a relocation into the output file.
   This is used to force out switch and PC relative relocations when
   relaxing.  */

int
sh_force_relocation (fix)
     fixS *fix;
{
  if (! sh_relax)
    return 0;

  return (fix->fx_pcrel
	  || SWITCH_TABLE (fix)
	  || fix->fx_r_type == R_SH_COUNT
	  || fix->fx_r_type == R_SH_ALIGN
	  || fix->fx_r_type == R_SH_CODE
	  || fix->fx_r_type == R_SH_DATA
	  || fix->fx_r_type == R_SH_LABEL);
}

/* Apply a fixup to the object file.  */

void
md_apply_fix (fixP, val)
     fixS *fixP;
     long val;
{
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  int lowbyte = target_big_endian ? 1 : 0;
  int highbyte = target_big_endian ? 0 : 1;

  if (fixP->fx_r_type == 0)
    {
      if (fixP->fx_size == 2)
	fixP->fx_r_type = R_SH_IMM16;
      else if (fixP->fx_size == 4)
	fixP->fx_r_type = R_SH_IMM32;
      else if (fixP->fx_size == 1)
	fixP->fx_r_type = R_SH_IMM8;
      else
	abort ();
    }

  switch (fixP->fx_r_type)
    {
    case R_SH_IMM4:
      *buf = (*buf & 0xf0) | (val & 0xf);
      break;

    case R_SH_IMM4BY2:
      *buf = (*buf & 0xf0) | ((val >> 1) & 0xf);
      break;

    case R_SH_IMM4BY4:
      *buf = (*buf & 0xf0) | ((val >> 2) & 0xf);
      break;

    case R_SH_IMM8BY2:
      *buf = val >> 1;
      break;

    case R_SH_IMM8BY4:
      *buf = val >> 2;
      break;

    case R_SH_IMM8:
      *buf++ = val;
      break;

    case R_SH_PCRELIMM8BY4:
      /* The lower two bits of the PC are cleared before the
         displacement is added in.  We can assume that the destination
         is on a 4 byte bounday.  If this instruction is also on a 4
         byte boundary, then we want
	   (target - here) / 4
	 and target - here is a multiple of 4.
	 Otherwise, we are on a 2 byte boundary, and we want
	   (target - (here - 2)) / 4
	 and target - here is not a multiple of 4.  Computing
	   (target - (here - 2)) / 4 == (target - here + 2) / 4
	 works for both cases, since in the first case the addition of
	 2 will be removed by the division.  target - here is in the
	 variable val.  */
      val = (val + 2) / 4;
      if (val & ~0xff)
	as_bad_where (fixP->fx_file, fixP->fx_line, "pcrel too far");
      buf[lowbyte] = val;
      break;

    case R_SH_PCRELIMM8BY2:
      val /= 2;
      if (val & ~0xff)
	as_bad_where (fixP->fx_file, fixP->fx_line, "pcrel too far");
      buf[lowbyte] = val;
      break;

    case R_SH_PCDISP8BY2:
      val /= 2;
      if (val < -0x80 || val > 0x7f)
	as_bad_where (fixP->fx_file, fixP->fx_line, "pcrel too far");
      buf[lowbyte] = val;
      break;

    case R_SH_PCDISP:
      val /= 2;
      if (val < -0x800 || val >= 0x7ff)
	as_bad_where (fixP->fx_file, fixP->fx_line, "pcrel too far");
      buf[lowbyte] = val & 0xff;
      buf[highbyte] |= (val >> 8) & 0xf;
      break;

    case R_SH_IMM32:
      if (! target_big_endian) 
	{
	  *buf++ = val >> 0;
	  *buf++ = val >> 8;
	  *buf++ = val >> 16;
	  *buf++ = val >> 24;
	}
      else 
	{
	  *buf++ = val >> 24;
	  *buf++ = val >> 16;
	  *buf++ = val >> 8;
	  *buf++ = val >> 0;
	}
      break;

    case R_SH_IMM16:
      if (! target_big_endian)
	{
	  *buf++ = val >> 0;
	  *buf++ = val >> 8;
	} 
      else 
	{
	  *buf++ = val >> 8;
	  *buf++ = val >> 0;
	}
      break;

    case R_SH_USES:
      /* Pass the value into sh_coff_reloc_mangle.  */
      fixP->fx_addnumber = val;
      break;

    case R_SH_COUNT:
    case R_SH_ALIGN:
    case R_SH_CODE:
    case R_SH_DATA:
    case R_SH_LABEL:
      /* Nothing to do here.  */
      break;

    default:
      abort ();
    }
}

int md_long_jump_size;

/* Called just before address relaxation.  Return the length
   by which a fragment must grow to reach it's destination.  */

int
md_estimate_size_before_relax (fragP, segment_type)
     register fragS *fragP;
     register segT segment_type;
{
  switch (fragP->fr_subtype)
    {
    case C (UNCOND_JUMP, UNDEF_DISP):
      /* used to be a branch to somewhere which was unknown */
      if (!fragP->fr_symbol)
	{
	  fragP->fr_subtype = C (UNCOND_JUMP, UNCOND12);
	  fragP->fr_var = md_relax_table[C (UNCOND_JUMP, UNCOND12)].rlx_length;
	}
      else if (S_GET_SEGMENT (fragP->fr_symbol) == segment_type)
	{
	  fragP->fr_subtype = C (UNCOND_JUMP, UNCOND12);
	  fragP->fr_var = md_relax_table[C (UNCOND_JUMP, UNCOND12)].rlx_length;
	}
      else
	{
	  fragP->fr_subtype = C (UNCOND_JUMP, UNDEF_WORD_DISP);
	  fragP->fr_var = md_relax_table[C (UNCOND_JUMP, UNCOND32)].rlx_length;
	  return md_relax_table[C (UNCOND_JUMP, UNCOND32)].rlx_length;
	}
      break;

    default:
      abort ();
    case C (COND_JUMP, UNDEF_DISP):
      /* used to be a branch to somewhere which was unknown */
      if (fragP->fr_symbol
	  && S_GET_SEGMENT (fragP->fr_symbol) == segment_type)
	{
	  /* Got a symbol and it's defined in this segment, become byte
	     sized - maybe it will fix up */
	  fragP->fr_subtype = C (COND_JUMP, COND8);
	  fragP->fr_var = md_relax_table[C (COND_JUMP, COND8)].rlx_length;
	}
      else if (fragP->fr_symbol)
	{
	  /* Its got a segment, but its not ours, so it will always be long */
	  fragP->fr_subtype = C (COND_JUMP, UNDEF_WORD_DISP);
	  fragP->fr_var = md_relax_table[C (COND_JUMP, COND32)].rlx_length;
	  return md_relax_table[C (COND_JUMP, COND32)].rlx_length;
	}
      else
	{
	  /* We know the abs value */
	  fragP->fr_subtype = C (COND_JUMP, COND8);
	  fragP->fr_var = md_relax_table[C (COND_JUMP, COND8)].rlx_length;
	}

      break;
    }
  return fragP->fr_var;
}

/* Put number into target byte order */

void
md_number_to_chars (ptr, use, nbytes)
     char *ptr;
     valueT use;
     int nbytes;
{
  if (! target_big_endian)
    number_to_chars_littleendian (ptr, use, nbytes);
  else
    number_to_chars_bigendian (ptr, use, nbytes);
}

long
md_pcrel_from (fixP)
     fixS *fixP;
{
  return fixP->fx_size + fixP->fx_where + fixP->fx_frag->fr_address + 2;
}

int
tc_coff_sizemachdep (frag)
     fragS *frag;
{
  return md_relax_table[frag->fr_subtype].rlx_length;
}

/* When we align the .text section, insert the correct NOP pattern.  */

int
sh_do_align (n, fill, len)
     int n;
     const char *fill;
     int len;
{
  if ((fill == NULL || (*fill == 0 && len == 1))
      && (now_seg == text_section
#ifdef BFD_ASSEMBLER
	  || (now_seg->flags & SEC_CODE) != 0
#endif
	  || strcmp (obj_segment_name (now_seg), ".init") == 0)
      && n > 1)
    {
      static const unsigned char big_nop_pattern[] = { 0x00, 0x09 };
      static const unsigned char little_nop_pattern[] = { 0x09, 0x00 };

      /* First align to a 2 byte boundary, in case there is an odd
         .byte.  */
      frag_align (1, 0);
      if (target_big_endian)
	frag_align_pattern (n, big_nop_pattern, sizeof big_nop_pattern);
      else
	frag_align_pattern (n, little_nop_pattern, sizeof little_nop_pattern);
      return 1;
    }

  return 0;
}

#ifdef OBJ_COFF

/* Adjust a reloc for the SH.  This is similar to the generic code,
   but does some minor tweaking.  */

void
sh_coff_reloc_mangle (seg, fix, intr, paddr)
     segment_info_type *seg;
     fixS *fix;
     struct internal_reloc *intr;
     unsigned int paddr;
{
  symbolS *symbol_ptr = fix->fx_addsy;
  symbolS *dot;

  intr->r_vaddr = paddr + fix->fx_frag->fr_address + fix->fx_where;

  if (! SWITCH_TABLE (fix))
    {
      intr->r_type = fix->fx_r_type;
      intr->r_offset = 0;
    }
  else
    {
      know (sh_relax);

      if (fix->fx_r_type == R_SH_IMM16)
	intr->r_type = R_SH_SWITCH16;
      else if (fix->fx_r_type == R_SH_IMM32)
	intr->r_type = R_SH_SWITCH32;
      else
	abort ();

      /* For a switch reloc, we set r_offset to the difference between
         the reloc address and the subtrahend.  When the linker is
         doing relaxing, it can use the determine the starting and
         ending points of the switch difference expression.  */
      intr->r_offset = intr->r_vaddr - S_GET_VALUE (fix->fx_subsy);
    }

  /* PC relative relocs are always against the current section.  */
  if (symbol_ptr == NULL)
    {
      switch (fix->fx_r_type)
	{
	case R_SH_PCRELIMM8BY2:
	case R_SH_PCRELIMM8BY4:
	case R_SH_PCDISP8BY2:
	case R_SH_PCDISP:
	case R_SH_USES:
	  symbol_ptr = seg->dot;
	  break;
	default:
	  break;
	}
    }

  if (fix->fx_r_type == R_SH_USES)
    {
      /* We can't store the offset in the object file, since this
	 reloc does not take up any space, so we store it in r_offset.
	 The fx_addnumber field was set in md_apply_fix.  */
      intr->r_offset = fix->fx_addnumber;
    }
  else if (fix->fx_r_type == R_SH_COUNT)
    {
      /* We can't store the count in the object file, since this reloc
         does not take up any space, so we store it in r_offset.  The
         fx_offset field was set when the fixup was created in
         sh_coff_frob_file.  */
      intr->r_offset = fix->fx_offset;
      /* This reloc is always absolute.  */
      symbol_ptr = NULL;
    }
  else if (fix->fx_r_type == R_SH_ALIGN)
    {
      /* Store the alignment in the r_offset field.  */
      intr->r_offset = fix->fx_offset;
      /* This reloc is always absolute.  */
      symbol_ptr = NULL;
    }
  else if (fix->fx_r_type == R_SH_CODE
	   || fix->fx_r_type == R_SH_DATA
	   || fix->fx_r_type == R_SH_LABEL)
    {
      /* These relocs are always absolute.  */
      symbol_ptr = NULL;
    }

  /* Turn the segment of the symbol into an offset.  */
  if (symbol_ptr != NULL)
    {
      dot = segment_info[S_GET_SEGMENT (symbol_ptr)].dot;
      if (dot != NULL)
	intr->r_symndx = dot->sy_number;
      else
	intr->r_symndx = symbol_ptr->sy_number;
    }
  else
    intr->r_symndx = -1;
}

#endif
