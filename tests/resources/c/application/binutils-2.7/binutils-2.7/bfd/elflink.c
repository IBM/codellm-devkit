/* ELF linking support for BFD.
   Copyright 1995 Free Software Foundation, Inc.

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

#include "bfd.h"
#include "sysdep.h"
#include "bfdlink.h"
#include "libbfd.h"
#define ARCH_SIZE 0
#include "elf-bfd.h"

boolean
_bfd_elf_create_got_section (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  flagword flags;
  register asection *s;
  struct elf_link_hash_entry *h;
  struct elf_backend_data *bed = get_elf_backend_data (abfd);

  /* This function may be called more than once.  */
  if (bfd_get_section_by_name (abfd, ".got") != NULL)
    return true;

  flags = SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY;

  s = bfd_make_section (abfd, ".got");
  if (s == NULL
      || !bfd_set_section_flags (abfd, s, flags)
      || !bfd_set_section_alignment (abfd, s, 2))
    return false;

  if (bed->want_got_plt)
    {
      s = bfd_make_section (abfd, ".got.plt");
      if (s == NULL
	  || !bfd_set_section_flags (abfd, s, flags)
	  || !bfd_set_section_alignment (abfd, s, 2))
	return false;
    }

  /* Define the symbol _GLOBAL_OFFSET_TABLE_ at the start of the .got
     (or .got.plt) section.  We don't do this in the linker script
     because we don't want to define the symbol if we are not creating
     a global offset table.  */
  h = NULL;
  if (!(_bfd_generic_link_add_one_symbol
	(info, abfd, "_GLOBAL_OFFSET_TABLE_", BSF_GLOBAL, s, (bfd_vma) 0,
	 (const char *) NULL, false, get_elf_backend_data (abfd)->collect,
	 (struct bfd_link_hash_entry **) &h)))
    return false;
  h->elf_link_hash_flags |= ELF_LINK_HASH_DEF_REGULAR;
  h->type = STT_OBJECT;

  if (info->shared
      && ! _bfd_elf_link_record_dynamic_symbol (info, h))
    return false;

  elf_hash_table (info)->hgot = h;

  /* The first three global offset table entries are reserved.  */
  s->_raw_size += 3 * 4;

  return true;
}


/* Create dynamic sections when linking against a dynamic object.  */

boolean
_bfd_elf_create_dynamic_sections (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  flagword flags;
  register asection *s;
  struct elf_backend_data *bed = get_elf_backend_data (abfd);

  /* We need to create .plt, .rel[a].plt, .got, .got.plt, .dynbss, and
     .rel[a].bss sections.  */

  flags = SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY;

  s = bfd_make_section (abfd, ".plt");
  if (s == NULL
      || ! bfd_set_section_flags (abfd, s,
				  (flags | SEC_CODE
				   | (bed->plt_readonly ? SEC_READONLY : 0)))
      || ! bfd_set_section_alignment (abfd, s, 2))
    return false;

  if (bed->want_plt_sym)
    {
      /* Define the symbol _PROCEDURE_LINKAGE_TABLE_ at the start of the
	 .plt section.  */
      struct elf_link_hash_entry *h = NULL;
      if (! (_bfd_generic_link_add_one_symbol
	     (info, abfd, "_PROCEDURE_LINKAGE_TABLE_", BSF_GLOBAL, s,
	      (bfd_vma) 0, (const char *) NULL, false,
	      get_elf_backend_data (abfd)->collect,
	      (struct bfd_link_hash_entry **) &h)))
	return false;
      h->elf_link_hash_flags |= ELF_LINK_HASH_DEF_REGULAR;
      h->type = STT_OBJECT;

      if (info->shared
	  && ! _bfd_elf_link_record_dynamic_symbol (info, h))
	return false;
    }

  s = bfd_make_section (abfd, bed->use_rela_p ? ".rela.plt" : ".rel.plt");
  if (s == NULL
      || ! bfd_set_section_flags (abfd, s, flags | SEC_READONLY)
      || ! bfd_set_section_alignment (abfd, s, 2))
    return false;

  if (! _bfd_elf_create_got_section (abfd, info))
    return false;

  /* The .dynbss section is a place to put symbols which are defined
     by dynamic objects, are referenced by regular objects, and are
     not functions.  We must allocate space for them in the process
     image and use a R_*_COPY reloc to tell the dynamic linker to
     initialize them at run time.  The linker script puts the .dynbss
     section into the .bss section of the final image.  */
  s = bfd_make_section (abfd, ".dynbss");
  if (s == NULL
      || ! bfd_set_section_flags (abfd, s, SEC_ALLOC))
    return false;

  /* The .rel[a].bss section holds copy relocs.  This section is not
     normally needed.  We need to create it here, though, so that the
     linker will map it to an output section.  We can't just create it
     only if we need it, because we will not know whether we need it
     until we have seen all the input files, and the first time the
     main linker code calls BFD after examining all the input files
     (size_dynamic_sections) the input sections have already been
     mapped to the output sections.  If the section turns out not to
     be needed, we can discard it later.  We will never need this
     section when generating a shared object, since they do not use
     copy relocs.  */
  if (! info->shared)
    {
      s = bfd_make_section (abfd, bed->use_rela_p ? ".rela.bss" : ".rel.bss");
      if (s == NULL
	  || ! bfd_set_section_flags (abfd, s, flags | SEC_READONLY)
	  || ! bfd_set_section_alignment (abfd, s, 2))
	return false;
    }

  return true;
}


/* Record a new dynamic symbol.  We record the dynamic symbols as we
   read the input files, since we need to have a list of all of them
   before we can determine the final sizes of the output sections.
   Note that we may actually call this function even though we are not
   going to output any dynamic symbols; in some cases we know that a
   symbol should be in the dynamic symbol table, but only if there is
   one.  */

boolean
_bfd_elf_link_record_dynamic_symbol (info, h)
     struct bfd_link_info *info;
     struct elf_link_hash_entry *h;
{
  if (h->dynindx == -1)
    {
      struct bfd_strtab_hash *dynstr;

      h->dynindx = elf_hash_table (info)->dynsymcount;
      ++elf_hash_table (info)->dynsymcount;

      dynstr = elf_hash_table (info)->dynstr;
      if (dynstr == NULL)
	{
	  /* Create a strtab to hold the dynamic symbol names.  */
	  elf_hash_table (info)->dynstr = dynstr = _bfd_elf_stringtab_init ();
	  if (dynstr == NULL)
	    return false;
	}

      h->dynstr_index = ((unsigned long)
			 _bfd_stringtab_add (dynstr, h->root.root.string,
					     true, false));
      if (h->dynstr_index == (unsigned long) -1)
	return false;
    }

  return true;
}

/* Create a special linker section, or return a pointer to a linker section already created  */

elf_linker_section_t *
_bfd_elf_create_linker_section (abfd, info, which, defaults)
     bfd *abfd;
     struct bfd_link_info *info;
     enum elf_linker_section_enum which;
     elf_linker_section_t *defaults;
{
  bfd *dynobj = elf_hash_table (info)->dynobj;
  elf_linker_section_t *lsect;

  /* Record the first bfd section that needs the special section */
  if (!dynobj)
    dynobj = elf_hash_table (info)->dynobj = abfd;

  /* If this is the first time, create the section */
  lsect = elf_linker_section (dynobj, which);
  if (!lsect)
    {
      asection *s;

      lsect = (elf_linker_section_t *)
	bfd_alloc (dynobj, sizeof (elf_linker_section_t));

      *lsect = *defaults;
      elf_linker_section (dynobj, which) = lsect;
      lsect->which = which;
      lsect->hole_written_p = false;

      /* See if the sections already exist */
      lsect->section = s = bfd_get_section_by_name (dynobj, lsect->name);
      if (!s)
	{
	  lsect->section = s = bfd_make_section (dynobj, lsect->name);

	  if (s == NULL)
	    return (elf_linker_section_t *)0;

	  bfd_set_section_flags (dynobj, s, defaults->flags);
	  bfd_set_section_alignment (dynobj, s, lsect->alignment);
	}
      else if (bfd_get_section_alignment (dynobj, s) < lsect->alignment)
	bfd_set_section_alignment (dynobj, s, lsect->alignment);

      s->_raw_size = align_power (s->_raw_size, lsect->alignment);

      /* Is there a hole we have to provide?  If so check whether the segment is
	 too big already */
      if (lsect->hole_size)
	{
	  lsect->hole_offset = s->_raw_size;
	  s->_raw_size += lsect->hole_size;
	  if (lsect->hole_offset > lsect->max_hole_offset)
	    {
	      (*_bfd_error_handler) ("%s: Section %s is already to large to put hole of %ld bytes in",
				     bfd_get_filename (abfd),
				     lsect->name,
				     (long)lsect->hole_size);

	      bfd_set_error (bfd_error_bad_value);
	      return (elf_linker_section_t *)0;
	    }
	}

#ifdef DEBUG
      fprintf (stderr, "Creating section %s, current size = %ld\n",
	       lsect->name, (long)s->_raw_size);
#endif

      if (lsect->sym_name)
	{
	  struct elf_link_hash_entry *h = NULL;
#ifdef DEBUG
	  fprintf (stderr, "Adding %s to section %s\n",
		   lsect->sym_name,
		   lsect->name);
#endif
	  h = (struct elf_link_hash_entry *)
	    bfd_link_hash_lookup (info->hash, lsect->sym_name, false, false, false);

	  if ((h == NULL || h->root.type == bfd_link_hash_undefined)
	      && !(_bfd_generic_link_add_one_symbol (info,
						     abfd,
						     lsect->sym_name,
						     BSF_GLOBAL,
						     s,
						     ((lsect->hole_size)
						      ? s->_raw_size - lsect->hole_size + lsect->sym_offset
						      : lsect->sym_offset),
						     (const char *) NULL,
						     false,
						     get_elf_backend_data (abfd)->collect,
						     (struct bfd_link_hash_entry **) &h)))
	    return (elf_linker_section_t *)0;

	  if ((defaults->which != LINKER_SECTION_SDATA)
	      && (defaults->which != LINKER_SECTION_SDATA2))
	    h->elf_link_hash_flags |= ELF_LINK_HASH_DEF_DYNAMIC;

	  h->type = STT_OBJECT;
	  lsect->sym_hash = h;

	  if (info->shared
	      && ! _bfd_elf_link_record_dynamic_symbol (info, h))
	    return (elf_linker_section_t *)0;
	}
    }

  /* Find the related sections if they have been created */
  if (lsect->bss_name && !lsect->bss_section)
    lsect->bss_section = bfd_get_section_by_name (dynobj, lsect->bss_name);

  if (lsect->rel_name && !lsect->rel_section)
    lsect->rel_section = bfd_get_section_by_name (dynobj, lsect->rel_name);

  return lsect;
}


/* Find a linker generated pointer with a given addend and type.  */

elf_linker_section_pointers_t *
_bfd_elf_find_pointer_linker_section (linker_pointers, addend, which)
     elf_linker_section_pointers_t *linker_pointers;
     bfd_signed_vma addend;
     elf_linker_section_enum_t which;
{
  for ( ; linker_pointers != NULL; linker_pointers = linker_pointers->next)
    {
      if (which == linker_pointers->which && addend == linker_pointers->addend)
	return linker_pointers;
    }

  return (elf_linker_section_pointers_t *)0;
}


/* Make the .rela section corresponding to the generated linker section.  */

boolean
_bfd_elf_make_linker_section_rela (dynobj, lsect, alignment)
     bfd *dynobj;
     elf_linker_section_t *lsect;
     int alignment;
{
  if (lsect->rel_section)
    return true;

  lsect->rel_section = bfd_get_section_by_name (dynobj, lsect->rel_name);
  if (lsect->rel_section == NULL)
    {
      lsect->rel_section = bfd_make_section (dynobj, lsect->rel_name);
      if (lsect->rel_section == NULL
	  || ! bfd_set_section_flags (dynobj,
				      lsect->rel_section,
				      (SEC_ALLOC
				       | SEC_LOAD
				       | SEC_HAS_CONTENTS
				       | SEC_IN_MEMORY
				       | SEC_READONLY))
	  || ! bfd_set_section_alignment (dynobj, lsect->rel_section, alignment))
	return false;
    }

  return true;
}

