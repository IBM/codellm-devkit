/* Stabs in sections linking support.
   Copyright 1996 Free Software Foundation, Inc.
   Written by Ian Lance Taylor, Cygnus Support.

This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* This file contains support for linking stabs in sections, as used
   on COFF and ELF.  */

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"
#include "aout/stab_gnu.h"

#include <ctype.h>

/* Stabs entries use a 12 byte format:
     4 byte string table index
     1 byte stab type
     1 byte stab other field
     2 byte stab desc field
     4 byte stab value
   FIXME: This will have to change for a 64 bit object format.

   The stabs symbols are divided into compilation units.  For the
   first entry in each unit, the type of 0, the value is the length of
   the string table for this unit, and the desc field is the number of
   stabs symbols for this unit.  */

#define STRDXOFF (0)
#define TYPEOFF (4)
#define OTHEROFF (5)
#define DESCOFF (6)
#define VALOFF (8)
#define STABSIZE (12)

/* A hash table used for header files with N_BINCL entries.  */

struct stab_link_includes_table
{
  struct bfd_hash_table root;
};

/* A linked list of totals that we have found for a particular header
   file.  */

struct stab_link_includes_totals
{
  struct stab_link_includes_totals *next;
  bfd_vma total;
};

/* An entry in the header file hash table.  */

struct stab_link_includes_entry
{
  struct bfd_hash_entry root;
  /* List of totals we have found for this file.  */
  struct stab_link_includes_totals *totals;
};

/* Look up an entry in an the header file hash table.  */

#define stab_link_includes_lookup(table, string, create, copy) \
  ((struct stab_link_includes_entry *) \
   bfd_hash_lookup (&(table)->root, (string), (create), (copy)))

/* This structure is used to hold a list of N_BINCL symbols, some of
   which might be converted into N_EXCL symbols.  */

struct stab_excl_list
{
  /* The next symbol to convert.  */
  struct stab_excl_list *next;
  /* The offset to this symbol in the section contents.  */
  bfd_size_type offset;
  /* The value to use for the symbol.  */
  bfd_vma val;
  /* The type of this symbol (N_BINCL or N_EXCL).  */
  int type;
};

/* This structure is stored with each .stab section.  */

struct stab_section_info
{
  /* This is a linked list of N_BINCL symbols which should be
     converted into N_EXCL symbols.  */
  struct stab_excl_list *excls;
  /* This is an array of string indices.  For each stab symbol, we
     store the string index here.  If a stab symbol should not be
     included in the final output, the string index is -1.  */
  bfd_size_type stridxs[1];
};

/* This structure is used to keep track of stabs in sections
   information while linking.  */

struct stab_info
{
  /* A hash table used to hold stabs strings.  */
  struct bfd_strtab_hash *strings;
  /* The header file hash table.  */
  struct stab_link_includes_table includes;
  /* The first .stabstr section.  */
  asection *stabstr;
};

static struct bfd_hash_entry *stab_link_includes_newfunc
  PARAMS ((struct bfd_hash_entry *, struct bfd_hash_table *, const char *));

/* The function to create a new entry in the header file hash table.  */

static struct bfd_hash_entry *
stab_link_includes_newfunc (entry, table, string)
     struct bfd_hash_entry *entry;
     struct bfd_hash_table *table;
     const char *string;
{
  struct stab_link_includes_entry *ret =
    (struct stab_link_includes_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == (struct stab_link_includes_entry *) NULL)
    ret = ((struct stab_link_includes_entry *)
	   bfd_hash_allocate (table,
			      sizeof (struct stab_link_includes_entry)));
  if (ret == (struct stab_link_includes_entry *) NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = ((struct stab_link_includes_entry *)
	 bfd_hash_newfunc ((struct bfd_hash_entry *) ret, table, string));
  if (ret)
    {
      /* Set local fields.  */
      ret->totals = NULL;
    }

  return (struct bfd_hash_entry *) ret;
}

/* This function is called for each input file from the add_symbols
   pass of the linker.  */

boolean
_bfd_link_section_stabs (abfd, psinfo, stabsec, stabstrsec, psecinfo)
     bfd *abfd;
     PTR *psinfo;
     asection *stabsec;
     asection *stabstrsec;
     PTR *psecinfo;
{
  struct stab_info *sinfo;
  bfd_size_type count;
  struct stab_section_info *secinfo;
  bfd_byte *stabbuf = NULL;
  bfd_byte *stabstrbuf = NULL;
  bfd_byte *sym, *symend;
  bfd_size_type stroff, next_stroff, skip;
  bfd_size_type *pstridx;

  if (stabsec->_raw_size == 0
      || stabstrsec->_raw_size == 0)
    {
      /* This file does not contain stabs debugging information.  */
      return true;
    }

  if (stabsec->_raw_size % STABSIZE != 0)
    {
      /* Something is wrong with the format of these stab symbols.
         Don't try to optimize them.  */
      return true;
    }

  if ((stabstrsec->flags & SEC_RELOC) != 0)
    {
      /* We shouldn't see relocations in the strings, and we aren't
         prepared to handle them.  */
      return true;
    }

  if (*psinfo == NULL)
    {
      /* Initialize the stabs information we need to keep track of.  */
      *psinfo = (PTR) bfd_alloc (abfd, sizeof (struct stab_info));
      sinfo = (struct stab_info *) *psinfo;
      sinfo->strings = _bfd_stringtab_init ();
      if (sinfo->strings == NULL)
	goto error_return;
      if (! bfd_hash_table_init_n (&sinfo->includes.root,
				   stab_link_includes_newfunc,
				   251))
	goto error_return;
      sinfo->stabstr = bfd_make_section_anyway (abfd, ".stabstr");
      sinfo->stabstr->flags |= SEC_HAS_CONTENTS | SEC_READONLY | SEC_DEBUGGING;
    }

  sinfo = (struct stab_info *) *psinfo;

  /* Initialize the information we are going to store for this .stab
     section.  */

  count = stabsec->_raw_size / STABSIZE;

  *psecinfo = bfd_alloc (abfd,
			 (sizeof (struct stab_section_info)
			  + (count - 1) * sizeof (bfd_size_type)));
  if (*psecinfo == NULL)
    goto error_return;

  secinfo = (struct stab_section_info *) *psecinfo;
  secinfo->excls = NULL;
  memset (secinfo->stridxs, 0, count * sizeof (bfd_size_type));

  /* Read the stabs information from abfd.  */

  stabbuf = (bfd_byte *) bfd_malloc (stabsec->_raw_size);
  stabstrbuf = (bfd_byte *) bfd_malloc (stabstrsec->_raw_size);
  if (stabbuf == NULL || stabstrbuf == NULL)
    goto error_return;

  if (! bfd_get_section_contents (abfd, stabsec, stabbuf, 0,
				  stabsec->_raw_size)
      || ! bfd_get_section_contents (abfd, stabstrsec, stabstrbuf, 0,
				     stabstrsec->_raw_size))
    goto error_return;

  /* Look through the stabs symbols, work out the new string indices,
     and identify N_BINCL symbols which can be eliminated.  */

  stroff = 0;
  next_stroff = 0;
  skip = 0;

  symend = stabbuf + stabsec->_raw_size;
  for (sym = stabbuf, pstridx = secinfo->stridxs;
       sym < symend;
       sym += STABSIZE, ++pstridx)
    {
      int type;
      const char *string;

      if (*pstridx != 0)
	{
	  /* This symbol has already been handled by an N_BINCL pass.  */
	  continue;
	}

      type = sym[TYPEOFF];

      if (type == 0)
	{
	  /* Special type 0 stabs indicate the offset to the next
             string table.  We don't copy these into the output file.  */
	  stroff = next_stroff;
	  next_stroff += bfd_get_32 (abfd, sym + 8);
	  *pstridx = (bfd_size_type) -1;
	  ++skip;
	  continue;
	}

      /* Store the string in the hash table, and record the index.  */
      string = ((char *) stabstrbuf
		+ stroff
		+ bfd_get_32 (abfd, sym + STRDXOFF));
      *pstridx = _bfd_stringtab_add (sinfo->strings, string, true, true);

      /* An N_BINCL symbol indicates the start of the stabs entries
	 for a header file.  We need to scan ahead to the next N_EINCL
	 symbol, ignoring nesting, adding up all the characters in the
	 symbol names, not including the file numbers in types (the
	 first number after an open parenthesis).  */
      if (type == N_BINCL)
	{
	  bfd_vma val;
	  int nest;
	  bfd_byte *incl_sym;
	  struct stab_link_includes_entry *incl_entry;
	  struct stab_link_includes_totals *t;
	  struct stab_excl_list *ne;

	  val = 0;
	  nest = 0;
	  for (incl_sym = sym + STABSIZE;
	       incl_sym < symend;
	       incl_sym += STABSIZE)
	    {
	      int incl_type;

	      incl_type = incl_sym[TYPEOFF];
	      if (incl_type == 0)
		break;
	      else if (incl_type == N_EINCL)
		{
		  if (nest == 0)
		    break;
		  --nest;
		}
	      else if (incl_type == N_BINCL)
		++nest;
	      else if (nest == 0)
		{
		  const char *str;

		  str = ((char *) stabstrbuf
			 + stroff
			 + bfd_get_32 (abfd, incl_sym + STRDXOFF));
		  for (; *str != '\0'; str++)
		    {
		      val += *str;
		      if (*str == '(')
			{
			  /* Skip the file number.  */
			  ++str;
			  while (isdigit ((unsigned char) *str))
			    ++str;
			  --str;
			}
		    }
		}
	    }

	  /* If we have already included a header file with the same
	     value, then replaced this one with an N_EXCL symbol.  */
	  incl_entry = stab_link_includes_lookup (&sinfo->includes, string,
						  true, true);
	  if (incl_entry == NULL)
	    goto error_return;

	  for (t = incl_entry->totals; t != NULL; t = t->next)
	    if (t->total == val)
	      break;

	  /* Record this symbol, so that we can set the value
             correctly.  */
	  ne = (struct stab_excl_list *) bfd_alloc (abfd, sizeof *ne);
	  ne->offset = sym - stabbuf;
	  ne->val = val;
	  ne->type = N_BINCL;
	  ne->next = secinfo->excls;
	  secinfo->excls = ne;

	  if (t == NULL)
	    {
	      /* This is the first time we have seen this header file
		 with this set of stabs strings.  */
	      t = ((struct stab_link_includes_totals *)
		   bfd_hash_allocate (&sinfo->includes.root, sizeof *t));
	      if (t == NULL)
		goto error_return;
	      t->total = val;
	      t->next = incl_entry->totals;
	      incl_entry->totals = t;
	    }
	  else
	    {
	      bfd_size_type *incl_pstridx;

	      /* We have seen this header file before.  Tell the final
		 pass to change the type to N_EXCL.  */
	      ne->type = N_EXCL;

	      /* Mark the skipped symbols.  */

	      nest = 0;
	      for (incl_sym = sym + STABSIZE, incl_pstridx = pstridx + 1;
		   incl_sym < symend;
		   incl_sym += STABSIZE, ++incl_pstridx)
		{
		  int incl_type;

		  incl_type = incl_sym[TYPEOFF];

		  if (incl_type == N_EINCL)
		    {
		      if (nest == 0)
			{
			  *incl_pstridx = (bfd_size_type) -1;
			  ++skip;
			  break;
			}
		      --nest;
		    }
		  else if (incl_type == N_BINCL)
		    ++nest;
		  else if (nest == 0)
		    {
		      *incl_pstridx = (bfd_size_type) -1;
		      ++skip;
		    }
		}
	    }
	}
    }

  free (stabbuf);
  free (stabstrbuf);

  /* We need to set the section sizes such that the linker will
     compute the output section sizes correctly.  We set the .stab
     size to not include the entries we don't want.  We set
     SEC_EXCLUDE for the .stabstr section, so that it will be dropped
     from the link.  We record the size of the strtab in the first
     .stabstr section we saw, and make sure we don't set SEC_EXCLUDE
     for that section.  */
  stabsec->_cooked_size = (count - skip) * STABSIZE;
  if (stabsec->_cooked_size == 0)
    stabsec->flags |= SEC_EXCLUDE;
  stabstrsec->flags |= SEC_EXCLUDE;
  sinfo->stabstr->_cooked_size = _bfd_stringtab_size (sinfo->strings);

  return true;

 error_return:
  if (stabbuf != NULL)
    free (stabbuf);
  if (stabstrbuf != NULL)
    free (stabstrbuf);
  return false;
}

/* Write out the stab section.  This is called with the relocated
   contents.  */

boolean
_bfd_write_section_stabs (output_bfd, stabsec, psecinfo, contents)
     bfd *output_bfd;
     asection *stabsec;
     PTR *psecinfo;
     bfd_byte *contents;
{
  struct stab_section_info *secinfo;
  struct stab_excl_list *e;
  bfd_byte *sym, *tosym, *symend;
  bfd_size_type *pstridx;

  secinfo = (struct stab_section_info *) *psecinfo;

  if (secinfo == NULL)
    return bfd_set_section_contents (output_bfd, stabsec->output_section,
				     contents, stabsec->output_offset,
				     stabsec->_raw_size);

  /* Handle each N_BINCL entry.  */
  for (e = secinfo->excls; e != NULL; e = e->next)
    {
      bfd_byte *excl_sym;

      BFD_ASSERT (e->offset < stabsec->_raw_size);
      excl_sym = contents + e->offset;
      bfd_put_32 (output_bfd, e->val, excl_sym + VALOFF);
      excl_sym[TYPEOFF] = e->type;
    }

  /* Copy over all the stabs symbols, omitting the ones we don't want,
     and correcting the string indices for those we do want.  */
  tosym = contents;
  symend = contents + stabsec->_raw_size;
  for (sym = contents, pstridx = secinfo->stridxs;
       sym < symend;
       sym += STABSIZE, ++pstridx)
    {
      if (*pstridx != (bfd_size_type) -1)
	{
	  if (tosym != sym)
	    memcpy (tosym, sym, STABSIZE);
	  bfd_put_32 (output_bfd, *pstridx, tosym + STRDXOFF);
	  tosym += STABSIZE;
	}
    }

  BFD_ASSERT (tosym - contents == stabsec->_cooked_size);

  return bfd_set_section_contents (output_bfd, stabsec->output_section,
				   contents, stabsec->output_offset,
				   stabsec->_cooked_size);
}

/* Write out the .stabstr section.  */

boolean
_bfd_write_stab_strings (output_bfd, psinfo)
     bfd *output_bfd;
     PTR *psinfo;
{
  struct stab_info *sinfo;

  sinfo = (struct stab_info *) *psinfo;

  if (sinfo == NULL)
    return true;

  BFD_ASSERT ((sinfo->stabstr->output_offset
	       + _bfd_stringtab_size (sinfo->strings))
	      <= sinfo->stabstr->output_section->_raw_size);

  if (bfd_seek (output_bfd,
		(sinfo->stabstr->output_section->filepos
		 + sinfo->stabstr->output_offset),
		SEEK_SET) != 0)
    return false;

  if (! _bfd_stringtab_emit (output_bfd, sinfo->strings))
    return false;

  /* We no longer need the stabs information.  */
  _bfd_stringtab_free (sinfo->strings);
  bfd_hash_table_free (&sinfo->includes.root);

  return true;
}
