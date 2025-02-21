/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "gprof.h"
#include "cg_arcs.h"
#include "core.h"
#include "hist.h"
#include "symtab.h"

/*
 * A symbol to be the child of indirect calls:
 */
Sym indirectchild;


static operandenum
operandmode (modep)
     struct modebyte *modep;
{
  long usesreg = modep->regfield;

  switch (modep->modefield)
    {
    case 0:
    case 1:
    case 2:
    case 3:
      return literal;
    case 4:
      return indexed;
    case 5:
      return reg;
    case 6:
      return regdef;
    case 7:
      return autodec;
    case 8:
      return usesreg != PC ? autoinc : immediate;
    case 9:
      return usesreg != PC ? autoincdef : absolute;
    case 10:
      return usesreg != PC ? bytedisp : byterel;
    case 11:
      return usesreg != PC ? bytedispdef : bytereldef;
    case 12:
      return usesreg != PC ? worddisp : wordrel;
    case 13:
      return usesreg != PC ? worddispdef : wordreldef;
    case 14:
      return usesreg != PC ? longdisp : longrel;
    case 15:
      return usesreg != PC ? longdispdef : longreldef;
    }
  /* NOTREACHED */
}

static char *
operandname (mode)
     operandenum mode;
{

  switch (mode)
    {
    case literal:
      return "literal";
    case indexed:
      return "indexed";
    case reg:
      return "register";
    case regdef:
      return "register deferred";
    case autodec:
      return "autodecrement";
    case autoinc:
      return "autoincrement";
    case autoincdef:
      return "autoincrement deferred";
    case bytedisp:
      return "byte displacement";
    case bytedispdef:
      return "byte displacement deferred";
    case byterel:
      return "byte relative";
    case bytereldef:
      return "byte relative deferred";
    case worddisp:
      return "word displacement";
    case worddispdef:
      return "word displacement deferred";
    case wordrel:
      return "word relative";
    case wordreldef:
      return "word relative deferred";
    case immediate:
      return "immediate";
    case absolute:
      return "absolute";
    case longdisp:
      return "long displacement";
    case longdispdef:
      return "long displacement deferred";
    case longrel:
      return "long relative";
    case longreldef:
      return "long relative deferred";
    }
  /* NOTREACHED */
}

static long
operandlength (modep)
     struct modebyte *modep;
{

  switch (operandmode (modep))
    {
    case literal:
    case reg:
    case regdef:
    case autodec:
    case autoinc:
    case autoincdef:
      return 1;
    case bytedisp:
    case bytedispdef:
    case byterel:
    case bytereldef:
      return 2;
    case worddisp:
    case worddispdef:
    case wordrel:
    case wordreldef:
      return 3;
    case immediate:
    case absolute:
    case longdisp:
    case longdispdef:
    case longrel:
    case longreldef:
      return 5;
    case indexed:
      return 1 + operandlength ((struct modebyte *) ((char *) modep) + 1);
    }
  /* NOTREACHED */
}

static bfd_vma
reladdr (modep)
     struct modebyte *modep;
{
  operandenum mode = operandmode (modep);
  char *cp;
  short *sp;
  long *lp;

  cp = (char *) modep;
  ++cp;				/* skip over the mode */
  switch (mode)
    {
    default:
      fprintf (stderr, "[reladdr] not relative address\n");
      return (bfd_vma) modep;
    case byterel:
      return (bfd_vma) (cp + sizeof *cp + *cp);
    case wordrel:
      sp = (short *) cp;
      return (bfd_vma) (cp + sizeof *sp + *sp);
    case longrel:
      lp = (long *) cp;
      return (bfd_vma) (cp + sizeof *lp + *lp);
    }
}


void
find_call (parent, p_lowpc, p_highpc)
     Sym *parent;
     bfd_vma p_lowpc;
     bfd_vma p_highpc;
{
  unsigned char *instructp;
  long length;
  Sym *child;
  operandenum mode;
  operandenum firstmode;
  bfd_vma destpc;
  static bool inited = FALSE;

  if (!inited)
    {
      inited = TRUE;
      sym_init (&indirectchild);
      indirectchild.cg.prop.fract = 1.0;
      indirectchild.cg.cyc.head = &indirectchild;
    }

  if (core_text_space == 0)
    {
      return;
    }
  if (p_lowpc < s_lowpc)
    {
      p_lowpc = s_lowpc;
    }
  if (p_highpc > s_highpc)
    {
      p_highpc = s_highpc;
    }
  DBG (CALLDEBUG, printf ("[findcall] %s: 0x%lx to 0x%lx\n",
			  parent->name, p_lowpc, p_highpc));
  for (instructp = (unsigned char *) core_text_space + p_lowpc;
       instructp < (unsigned char *) core_text_space + p_highpc;
       instructp += length)
    {
      length = 1;
      if (*instructp == CALLS)
	{
	  /*
	   *    maybe a calls, better check it out.
	   *      skip the count of the number of arguments.
	   */
	  DBG (CALLDEBUG,
	       printf ("[findcall]\t0x%x:calls",
		       instructp - (unsigned char *) core_text_space));
	  firstmode = operandmode ((struct modebyte *) (instructp + length));
	  switch (firstmode)
	    {
	    case literal:
	    case immediate:
	      break;
	    default:
	      goto botched;
	    }
	  length += operandlength ((struct modebyte *) (instructp + length));
	  mode = operandmode ((struct modebyte *) (instructp + length));
	  DBG (CALLDEBUG,
	       printf ("\tfirst operand is %s", operandname (firstmode));
	       printf ("\tsecond operand is %s\n", operandname (mode)));
	  switch (mode)
	    {
	    case regdef:
	    case bytedispdef:
	    case worddispdef:
	    case longdispdef:
	    case bytereldef:
	    case wordreldef:
	    case longreldef:
	      /*
	       *    indirect call: call through pointer
	       *      either  *d(r)   as a parameter or local
	       *              (r)     as a return value
	       *              *f      as a global pointer
	       *      [are there others that we miss?,
	       *       e.g. arrays of pointers to functions???]
	       */
	      arc_add (parent, &indirectchild, (long) 0);
	      length += operandlength (
				  (struct modebyte *) (instructp + length));
	      continue;
	    case byterel:
	    case wordrel:
	    case longrel:
	      /*
	       *    regular pc relative addressing
	       *      check that this is the address of 
	       *      a function.
	       */
	      destpc = reladdr ((struct modebyte *) (instructp + length))
		- (bfd_vma) core_text_space;
	      if (destpc >= s_lowpc && destpc <= s_highpc)
		{
		  child = sym_lookup (&symtab, destpc);
		  DBG (CALLDEBUG,
		       printf ("[findcall]\tdestpc 0x%lx", destpc);
		       printf (" child->name %s", child->name);
		       printf (" child->addr 0x%lx\n", child->addr);
		    );
		  if (child->addr == destpc)
		    {
		      /*
		       *    a hit
		       */
		      arc_add (parent, child, (long) 0);
		      length += operandlength ((struct modebyte *)
					       (instructp + length));
		      continue;
		    }
		  goto botched;
		}
	      /*
	       *    else:
	       *      it looked like a calls,
	       *      but it wasn't to anywhere.
	       */
	      goto botched;
	    default:
	    botched:
	      /*
	       *    something funny going on.
	       */
	      DBG (CALLDEBUG, printf ("[findcall]\tbut it's a botch\n"));
	      length = 1;
	      continue;
	    }
	}
    }
}
