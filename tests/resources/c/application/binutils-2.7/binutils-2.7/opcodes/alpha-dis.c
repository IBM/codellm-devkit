/* alpha-dis.c -- Disassemble Alpha AXP instructions
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
along with this file; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <stdlib.h>
#include <stdio.h>
#include "ansidecl.h"
#include "sysdep.h"
#include "dis-asm.h"
#include "opcode/alpha.h"

int print_insn_alpha PARAMS ((bfd_vma, struct disassemble_info *));

static const char * const alpha_ir_regnames[32] = {
  "v0", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
  "s0", "s1", "s2", "s3", "s4", "s5", "fp", "a0", "a1",
  "a2", "a3", "a4", "a5", "t8", "t9", "t10", "t11", 
  "ra", "t12", "at", "gp", "sp", "zero"
};

int
print_insn_alpha(memaddr, info)
     bfd_vma memaddr;
     struct disassemble_info *info;
{
  static const struct alpha_opcode *opcode_index[AXP_NOPS+1];
  static int option_regnames = 1, option_printhex = 1;
  static unsigned option_target = ~0U;

  const struct alpha_opcode *opcode, *opcode_end;
  unsigned insn, op;
  const char *leader = "\t";

  /* Initialize the majorop table the first time through */
  if (!opcode_index[0])
    {
      char *p, *q;

      opcode = alpha_opcodes;
      opcode_end = opcode + alpha_num_opcodes;

      for (op = 0; op < AXP_NOPS; ++op)
        {
          opcode_index[op] = opcode;
          while (opcode < opcode_end && op == AXP_OP (opcode->opcode))
	    ++opcode;
        }
      opcode_index[op] = opcode;

      p = getenv("BFD_ALPHA_DISASSEMBLE_OPTIONS");
      if (p)
	{
	  do {
	    int set, len;
	    if (*p == '!')
	      set = 0, ++p;
	    else
	      set = 1;

	    q = strchr(p, ',');
	    if (!q) q = strchr(p, '\0');

	    switch (len = q - p)
	      {
	      case 3:
		if (strncmp("ev4", p, len) == 0)
		  option_target = AXP_OPCODE_EV4|AXP_OPCODE_ALL;
		else if (strncmp("ev5", p, len) == 0)
		  option_target = AXP_OPCODE_EV5|AXP_OPCODE_ALL;
		break;
	      case 4:
		if (strncmp("ev56", p, len) == 0)
		  option_target = AXP_OPCODE_EV56|AXP_OPCODE_ALL;
		break;
	      case 5:
		if (strncmp("21064", p, len) == 0)
		  option_target = AXP_OPCODE_EV4|AXP_OPCODE_ALL;
		else if (strncmp("21066", p, len) == 0)
		  option_target = AXP_OPCODE_EV4|AXP_OPCODE_ALL;
		else if (strncmp("21164", p, len) == 0)
		  option_target = AXP_OPCODE_EV5|AXP_OPCODE_ALL;
		break;
	      case 6:
		if (strncmp("21164a", p, len) == 0)
		  option_target = AXP_OPCODE_EV56|AXP_OPCODE_ALL;
		break;
	      case 8:
		if (strncmp("regnames", p, len) == 0)
		  option_regnames = set;
		else if (strncmp("printhex", p, len) == 0)
		  option_printhex = set;
		break;
	      }

	    p = q+1;
	  } while (*q);
	}
    }

  /* Read the insn into a host word */
  {
    bfd_byte buffer[4];
    int status = (*info->read_memory_func) (memaddr, buffer, 4, info);
    if (status != 0)
      {
        (*info->memory_error_func) (status, memaddr, info);
        return -1;
      }
    insn = bfd_getl32 (buffer);
  }

  if (option_printhex)
    {
      (*info->fprintf_func) (info->stream, "%s%08x", leader, insn);
      leader = "  ";
    }

  /* Get the major opcode of the instruction.  */
  op = AXP_OP (insn);

  /* Find the first match in the opcode table.  */
  opcode = opcode_index[op];
  opcode_end = opcode_index[op+1];
  for (; opcode < opcode_end; ++opcode)
    {
      const unsigned char *opindex;
      int need_comma;

      if ((insn & opcode->mask) != opcode->opcode)
	continue;

      if (!(opcode->flags & option_target))
	continue;

      /* Make two passes over the operands.  First see if any of them
	 have extraction functions, and, if they do, make sure the
	 instruction is valid.  */
      {
        int invalid = 0;
        for (opindex = opcode->operands; *opindex != 0; opindex++)
	  {
            const struct alpha_operand *operand = alpha_operands + *opindex;
	    if (operand->extract)
	      (*operand->extract) (insn, &invalid);
	  }
        if (invalid)
	  continue;
      }

      /* The instruction is valid.  */
      (*info->fprintf_func) (info->stream, "%s%-10s ", leader, opcode->name);

      /* Now extract and print the operands.  */
      need_comma = 0;
      for (opindex = opcode->operands; *opindex != 0; opindex++)
	{
          const struct alpha_operand *operand = alpha_operands + *opindex;
	  int value;

	  /* Operands that are marked FAKE are simply ignored.  We
	     already made sure that the extract function considered
	     the instruction to be valid.  */
	  if ((operand->flags & AXP_OPERAND_FAKE) != 0)
	    continue;

	  /* Extract the value from the instruction.  */
	  if (operand->extract)
	    value = (*operand->extract) (insn, (int *) NULL);
	  else
	    {
	      value = (insn >> operand->shift) & ((1 << operand->bits) - 1);
	      if (operand->flags & AXP_OPERAND_SIGNED)
		{
		  int signbit = 1 << (operand->bits - 1);
		  value = (value ^ signbit) - signbit;
		}
	    }

	  if (need_comma &&
	      ((operand->flags & (AXP_OPERAND_PARENS|AXP_OPERAND_COMMA))
	       != AXP_OPERAND_PARENS))
	    {
	      (*info->fprintf_func) (info->stream, ",");
	    }
	  if (operand->flags & AXP_OPERAND_PARENS)
	    (*info->fprintf_func) (info->stream, "(");

	  /* Print the operand as directed by the flags.  */
	  if (operand->flags & AXP_OPERAND_IR)
	    {
	      if (option_regnames)
		(*info->fprintf_func) (info->stream, alpha_ir_regnames[value]);
	      else
	        (*info->fprintf_func) (info->stream, "$%d", value);
	    }
	  else if (operand->flags & AXP_OPERAND_FPR)
	    (*info->fprintf_func) (info->stream, "$f%d", value);
	  else if (operand->flags & AXP_OPERAND_RELATIVE)
	    (*info->print_address_func) (memaddr + 4 + value, info);
	  else if (operand->flags & AXP_OPERAND_SIGNED)
	    (*info->fprintf_func) (info->stream, "%d", value);
	  else
	    (*info->fprintf_func) (info->stream, "%#x", value);

	  if (operand->flags & AXP_OPERAND_PARENS)
	    (*info->fprintf_func) (info->stream, ")");
	  need_comma = 1;
	}

      return 4;
    }

  /* No instruction found */
  if (!option_printhex)
    (*info->fprintf_func) (info->stream, ".long %#08x", insn);
    
  return 4;
}
