#include "cg_arcs.h"
#include "call_graph.h"
#include "core.h"
#include "gmon_io.h"
#include "gmon_out.h"
#include "symtab.h"
#include "sym_ids.h"

extern void
DEFUN (cg_tally, (from_pc, self_pc, count),
       bfd_vma from_pc AND bfd_vma self_pc AND int count)
{
  Sym *parent;
  Sym *child;

  parent = sym_lookup (&symtab, from_pc);
  child = sym_lookup (&symtab, self_pc);

  /*
   * Keep arc if it is on INCL_ARCS table or if the INCL_ARCS table
   * is empty and it is not in the EXCL_ARCS table.
   */
  if (sym_id_arc_is_present (&syms[INCL_ARCS], parent, child)
      || (syms[INCL_ARCS].len == 0
	  && !sym_id_arc_is_present (&syms[EXCL_ARCS], parent, child)))
    {
      child->ncalls += count;
      DBG (TALLYDEBUG,
	   printf ("[cg_tally] arc from %s to %s traversed %d times\n",
		   parent->name, child->name, count));
      arc_add (parent, child, count);
    }
}


/*
 * Read a record from file IFP describing an arc in the function
 * call-graph and the count of how many times the arc has been
 * traversed.  FILENAME is the name of file IFP and is provided
 * for formatting error-messages only.
 */
void
DEFUN (cg_read_rec, (ifp, filename), FILE * ifp AND CONST char *filename)
{
  bfd_vma from_pc, self_pc;
  struct gmon_cg_arc_record arc;
  int count;

  if (fread (&arc, sizeof (arc), 1, ifp) != 1)
    {
      fprintf (stderr, "%s: %s: unexpected end of file\n",
	       whoami, filename);
      done (1);
    }
  from_pc = get_vma (core_bfd, (bfd_byte *) arc.from_pc);
  self_pc = get_vma (core_bfd, (bfd_byte *) arc.self_pc);
  count = bfd_get_32 (core_bfd, (bfd_byte *) arc.count);
  DBG (SAMPLEDEBUG,
       printf ("[cg_read_rec] frompc 0x%lx selfpc 0x%lx count %d\n",
	       from_pc, self_pc, count));
  /* add this arc: */
  cg_tally (from_pc, self_pc, count);
}


/*
 * Write all the arcs in the call-graph to file OFP.  FILENAME is
 * the name of OFP and is provided for formatting error-messages
 * only.
 */
void
DEFUN (cg_write_arcs, (ofp, filename), FILE * ofp AND const char *filename)
{
  const unsigned char tag = GMON_TAG_CG_ARC;
  struct gmon_cg_arc_record raw_arc;
  Arc *arc;
  Sym *sym;

  for (sym = symtab.base; sym < symtab.limit; sym++)
    {
      for (arc = sym->cg.children; arc; arc = arc->next_child)
	{
	  put_vma (core_bfd, arc->parent->addr, (bfd_byte *) raw_arc.from_pc);
	  put_vma (core_bfd, arc->child->addr, (bfd_byte *) raw_arc.self_pc);
	  bfd_put_32 (core_bfd, arc->count, (bfd_byte *) raw_arc.count);
	  if (fwrite (&tag, sizeof (tag), 1, ofp) != 1
	      || fwrite (&raw_arc, sizeof (raw_arc), 1, ofp) != 1)
	    {
	      perror (filename);
	      done (1);
	    }
	  DBG (SAMPLEDEBUG,
	     printf ("[cg_write_arcs] frompc 0x%lx selfpc 0x%lx count %d\n",
		     arc->parent->addr, arc->child->addr, arc->count));
	}
    }
}
