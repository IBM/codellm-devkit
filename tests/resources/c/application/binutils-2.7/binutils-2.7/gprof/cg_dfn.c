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
#include <stdio.h>
#include "gprof.h"
#include "cg_arcs.h"
#include "cg_dfn.h"
#include "symtab.h"
#include "utils.h"

#define	DFN_DEPTH	100

typedef struct
  {
    Sym *sym;
    int cycle_top;
  }
DFN_Stack;

DFN_Stack dfn_stack[DFN_DEPTH];
int dfn_depth = 0;
int dfn_counter = DFN_NAN;


/*
 * Is CHILD already numbered?
 */
static bool
DEFUN (is_numbered, (child), Sym * child)
{
  return child->cg.top_order != DFN_NAN && child->cg.top_order != DFN_BUSY;
}


/*
 * Is CHILD already busy?
 */
static bool
DEFUN (is_busy, (child), Sym * child)
{
  if (child->cg.top_order == DFN_NAN)
    {
      return FALSE;
    }
  return TRUE;
}


/*
 * CHILD is part of a cycle.  Find the top caller into this cycle
 * that is not part of the cycle and make all functions in cycle
 * members of that cycle (top caller == caller with smallest
 * depth-first number).
 */
static void
DEFUN (find_cycle, (child), Sym * child)
{
  Sym *head = 0;
  Sym *tail;
  int cycle_top;
  int index;

  for (cycle_top = dfn_depth; cycle_top > 0; --cycle_top)
    {
      head = dfn_stack[cycle_top].sym;
      if (child == head)
	{
	  break;
	}
      if (child->cg.cyc.head != child && child->cg.cyc.head == head)
	{
	  break;
	}
    }
  if (cycle_top <= 0)
    {
      fprintf (stderr, "[find_cycle] couldn't find head of cycle\n");
      done (1);
    }
#ifdef DEBUG
  if (debug_level & DFNDEBUG)
    {
      printf ("[find_cycle] dfn_depth %d cycle_top %d ",
	      dfn_depth, cycle_top);
      if (head)
	{
	  print_name (head);
	}
      else
	{
	  printf ("<unknown>");
	}
      printf ("\n");
    }
#endif
  if (cycle_top == dfn_depth)
    {
      /*
       * This is previous function, e.g. this calls itself.  Sort of
       * boring.
       *
       * Since we are taking out self-cycles elsewhere no need for
       * the special case, here.
       */
      DBG (DFNDEBUG,
	   printf ("[find_cycle] ");
	   print_name (child);
	   printf ("\n"));
    }
  else
    {
      /*
       * Glom intervening functions that aren't already glommed into
       * this cycle.  Things have been glommed when their cyclehead
       * field points to the head of the cycle they are glommed
       * into.
       */
      for (tail = head; tail->cg.cyc.next; tail = tail->cg.cyc.next)
	{
	  /* void: chase down to tail of things already glommed */
	  DBG (DFNDEBUG,
	       printf ("[find_cycle] tail ");
	       print_name (tail);
	       printf ("\n"));
	}
      /*
       * If what we think is the top of the cycle has a cyclehead
       * field, then it's not really the head of the cycle, which is
       * really what we want.
       */
      if (head->cg.cyc.head != head)
	{
	  head = head->cg.cyc.head;
	  DBG (DFNDEBUG, printf ("[find_cycle] new cyclehead ");
	       print_name (head);
	       printf ("\n"));
	}
      for (index = cycle_top + 1; index <= dfn_depth; ++index)
	{
	  child = dfn_stack[index].sym;
	  if (child->cg.cyc.head == child)
	    {
	      /*
	       * Not yet glommed anywhere, glom it and fix any
	       * children it has glommed.
	       */
	      tail->cg.cyc.next = child;
	      child->cg.cyc.head = head;
	      DBG (DFNDEBUG, printf ("[find_cycle] glomming ");
		   print_name (child);
		   printf (" onto ");
		   print_name (head);
		   printf ("\n"));
	      for (tail = child; tail->cg.cyc.next; tail = tail->cg.cyc.next)
		{
		  tail->cg.cyc.next->cg.cyc.head = head;
		  DBG (DFNDEBUG, printf ("[find_cycle] and its tail ");
		       print_name (tail->cg.cyc.next);
		       printf (" onto ");
		       print_name (head);
		       printf ("\n"));
		}
	    }
	  else if (child->cg.cyc.head != head /* firewall */ )
	    {
	      fprintf (stderr, "[find_cycle] glommed, but not to head\n");
	      done (1);
	    }
	}
    }
}


/*
 * Prepare for visiting the children of PARENT.  Push a parent onto
 * the stack and mark it busy.
 */
static void
DEFUN (pre_visit, (parent), Sym * parent)
{
  ++dfn_depth;
  if (dfn_depth >= DFN_DEPTH)
    {
      fprintf (stderr, "[pre_visit] dfn_stack overflow\n");
      done (1);
    }
  dfn_stack[dfn_depth].sym = parent;
  dfn_stack[dfn_depth].cycle_top = dfn_depth;
  parent->cg.top_order = DFN_BUSY;
  DBG (DFNDEBUG, printf ("[pre_visit]\t\t%d:", dfn_depth);
       print_name (parent);
       printf ("\n"));
}


/*
 * Done with visiting node PARENT.  Pop PARENT off dfn_stack
 * and number functions if PARENT is head of a cycle.
 */
static void
DEFUN (post_visit, (parent), Sym * parent)
{
  Sym *member;

  DBG (DFNDEBUG, printf ("[post_visit]\t%d: ", dfn_depth);
       print_name (parent);
       printf ("\n"));
  /*
   * Number functions and things in their cycles unless the function
   * is itself part of a cycle:
   */
  if (parent->cg.cyc.head == parent)
    {
      ++dfn_counter;
      for (member = parent; member; member = member->cg.cyc.next)
	{
	  member->cg.top_order = dfn_counter;
	  DBG (DFNDEBUG, printf ("[post_visit]\t\tmember ");
	       print_name (member);
	       printf ("-> cg.top_order = %d\n", dfn_counter));
	}
    }
  else
    {
      DBG (DFNDEBUG, printf ("[post_visit]\t\tis part of a cycle\n"));
    }
  --dfn_depth;
}


/*
 * Given this PARENT, depth first number its children.
 */
void
DEFUN (cg_dfn, (parent), Sym * parent)
{
  Arc *arc;

  DBG (DFNDEBUG, printf ("[dfn] dfn( ");
       print_name (parent);
       printf (")\n"));
  /*
   * If we're already numbered, no need to look any further:
   */
  if (is_numbered (parent))
    {
      return;
    }
  /*
   * If we're already busy, must be a cycle:
   */
  if (is_busy (parent))
    {
      find_cycle (parent);
      return;
    }
  pre_visit (parent);
  /*
   * Recursively visit children:
   */
  for (arc = parent->cg.children; arc; arc = arc->next_child)
    {
      cg_dfn (arc->child);
    }
  post_visit (parent);
}
