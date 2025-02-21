/* evax-alpha.c -- BFD back-end for ALPHA EVAX (openVMS/AXP) files.
   Copyright 1996 Free Software Foundation, Inc.
   Written by Klaus K�mpf (kkaempf@progis.de)
   of proGIS Softwareentwicklung, Aachen, Germany

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


#include <stdio.h>

#include "bfd.h"
#include "sysdep.h"
#include "bfdlink.h"
#include "libbfd.h"

#include "evax.h"

static boolean evax_initialize PARAMS ((bfd *));
static boolean fill_section_ptr PARAMS ((struct bfd_hash_entry *, PTR));
static boolean evax_fixup_sections PARAMS ((bfd *));
static boolean copy_symbols PARAMS ((struct bfd_hash_entry *, PTR));
static bfd_reloc_status_type reloc_nil
  PARAMS ((bfd *, arelent *, asymbol *, PTR, asection *, bfd *, char **));
static const struct bfd_target *evax_object_p PARAMS ((bfd *abfd));
static const struct bfd_target *evax_archive_p PARAMS ((bfd *abfd));
static boolean evax_mkobject PARAMS ((bfd *abfd));
static boolean evax_write_object_contents PARAMS ((bfd *abfd));
static boolean evax_close_and_cleanup PARAMS ((bfd *abfd));
static boolean evax_bfd_free_cached_info PARAMS ((bfd *abfd));
static boolean evax_new_section_hook PARAMS ((bfd *abfd, asection *section));
static boolean evax_get_section_contents
  PARAMS ((bfd *abfd, asection *section, PTR x1, file_ptr x2,
	   bfd_size_type x3));
static boolean evax_get_section_contents_in_window
  PARAMS ((bfd *abfd, asection *section, bfd_window *w, file_ptr offset,
	   bfd_size_type count));
static boolean evax_bfd_copy_private_bfd_data PARAMS ((bfd *src, bfd *dest));
static boolean evax_bfd_copy_private_section_data
  PARAMS ((bfd *srcbfd, asection *srcsec, bfd *dstbfd, asection *dstsec));
static boolean evax_bfd_copy_private_symbol_data
  PARAMS ((bfd *ibfd, asymbol *isym, bfd *obfd, asymbol *osym));
static boolean evax_bfd_print_private_bfd_data
  PARAMS ((bfd *abfd, void *file));
static char *evax_core_file_failing_command PARAMS ((bfd *abfd));
static int evax_core_file_failing_signal PARAMS ((bfd *abfd));
static boolean evax_core_file_matches_executable_p
  PARAMS ((bfd *abfd, bfd *bbfd));
static boolean evax_slurp_armap PARAMS ((bfd *abfd));
static boolean evax_slurp_extended_name_table PARAMS ((bfd *abfd));
static boolean evax_construct_extended_name_table
  PARAMS ((bfd *abfd, char **tabloc, bfd_size_type *tablen,
	   const char **name));
static void evax_truncate_arname
  PARAMS ((bfd *abfd, CONST char *pathname, char *arhdr));
static boolean evax_write_armap
  PARAMS ((bfd *arch, unsigned int elength, struct orl *map,
	   unsigned int orl_count, int stridx));
static PTR evax_read_ar_hdr PARAMS ((bfd *abfd));
static bfd *evax_get_elt_at_index PARAMS ((bfd *abfd, symindex index));
static bfd *evax_openr_next_archived_file PARAMS ((bfd *arch, bfd *prev));
static boolean evax_update_armap_timestamp PARAMS ((bfd *abfd));
static int evax_generic_stat_arch_elt PARAMS ((bfd *abfd, struct stat *stat));
static long evax_get_symtab_upper_bound PARAMS ((bfd *abfd));
static long evax_get_symtab PARAMS ((bfd *abfd, asymbol **symbols));
static void evax_print_symbol
  PARAMS ((bfd *abfd, PTR file, asymbol *symbol, bfd_print_symbol_type how));
static void evax_get_symbol_info
  PARAMS ((bfd *abfd, asymbol *symbol, symbol_info *ret));
static boolean evax_bfd_is_local_label PARAMS ((bfd *abfd, asymbol *symbol));
static alent *evax_get_lineno PARAMS ((bfd *abfd, asymbol *symbol));
static boolean evax_find_nearest_line
  PARAMS ((bfd *abfd, asection *section, asymbol **symbols, bfd_vma offset,
	   const char **file, const char **func, unsigned int *line));
static asymbol *evax_bfd_make_debug_symbol
  PARAMS ((bfd *abfd, void *ptr, unsigned long size));
static long evax_read_minisymbols
  PARAMS ((bfd *abfd, boolean dynamic, PTR *minisymsp, unsigned int *sizep));
static asymbol *evax_minisymbol_to_symbol
  PARAMS ((bfd *abfd, boolean dynamic, const PTR minisym, asymbol *sym));
static long evax_get_reloc_upper_bound PARAMS ((bfd *abfd, asection *sect));
static long evax_canonicalize_reloc
  PARAMS ((bfd *abfd, asection *srcsec, arelent **location,
	   asymbol **symbols));
static const struct reloc_howto_struct *evax_bfd_reloc_type_lookup
  PARAMS ((bfd *abfd, bfd_reloc_code_real_type code));
static boolean evax_set_arch_mach
  PARAMS ((bfd *abfd, enum bfd_architecture arch, unsigned long mach));
static boolean evax_set_section_contents
  PARAMS ((bfd *abfd, asection *section, PTR location, file_ptr offset,
	   bfd_size_type count));
static int evax_sizeof_headers PARAMS ((bfd *abfd, boolean reloc));
static bfd_byte *evax_bfd_get_relocated_section_contents
  PARAMS ((bfd *abfd, struct bfd_link_info *link_info,
	   struct bfd_link_order *link_order, bfd_byte *data,
	   boolean relocateable, asymbol **symbols));
static boolean evax_bfd_relax_section
  PARAMS ((bfd *abfd, asection *section, struct bfd_link_info *link_info,
	   boolean *again));
static struct bfd_link_hash_table *evax_bfd_link_hash_table_create
  PARAMS ((bfd *abfd));
static boolean evax_bfd_link_add_symbols
  PARAMS ((bfd *abfd, struct bfd_link_info *link_info));
static boolean evax_bfd_final_link
  PARAMS ((bfd *abfd, struct bfd_link_info *link_info));
static boolean evax_bfd_link_split_section
  PARAMS ((bfd *abfd, asection *section));
static long evax_get_dynamic_symtab_upper_bound PARAMS ((bfd *abfd));
static long evax_canonicalize_dynamic_symtab
  PARAMS ((bfd *abfd, asymbol **symbols));
static long evax_get_dynamic_reloc_upper_bound PARAMS ((bfd *abfd));
static long evax_canonicalize_dynamic_reloc
  PARAMS ((bfd *abfd, arelent **arel, asymbol **symbols));
static boolean evax_bfd_merge_private_bfd_data PARAMS ((bfd *ibfd, bfd *obfd));
static boolean evax_bfd_set_private_flags PARAMS ((bfd *abfd, flagword flags));

#define evax_make_empty_symbol _bfd_evax_make_empty_symbol

/*===========================================================================*/

const bfd_target evax_alpha_vec =
{

  "evax-alpha",			/* name */
  bfd_target_evax_flavour,
  false,			/* data byte order is little */
  false,			/* header byte order is little */

  (HAS_RELOC |			/* object flags */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | WP_TEXT | D_PAGED),

  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC), /* sect flags */
  0,				/* symbol_leading_char */
  ' ',				/* ar_pad_char */
  15,				/* ar_max_namelen */
  bfd_getl64,			/* bfd_getx64 */
  bfd_getl_signed_64,		/* bfd_getx_signed_64 */
  bfd_putl64,			/* bfd_putx64 */
  bfd_getl32,			/* bfd_getx32 */
  bfd_getl_signed_32,		/* bfd_getx_signed_32 */
  bfd_putl32,			/* bfd_putx32 */
  bfd_getl16,			/* bfd_getx16 */
  bfd_getl_signed_16,		/* bfd_getx_signed_16 */
  bfd_putl16,			/* bfd_putx16 */
  bfd_getl64,			/* bfd_h_getx64 */
  bfd_getl_signed_64,		/* bfd_h_getx_signed_64 */
  bfd_putl64,			/* bfd_h_putx64 */
  bfd_getl32,			/* bfd_h_getx32 */
  bfd_getl_signed_32,		/* bfd_h_getx_signed_32 */
  bfd_putl32,			/* bfd_h_putx32 */
  bfd_getl16,			/* bfd_h_getx16 */
  bfd_getl_signed_16,		/* bfd_h_getx_signed_16 */
  bfd_putl16,			/* bfd_h_putx16 */

  {_bfd_dummy_target, evax_object_p,		/* bfd_check_format */
   evax_archive_p, _bfd_dummy_target},
  {bfd_false, evax_mkobject,			/* bfd_set_format */
   _bfd_generic_mkarchive, bfd_false},
  {bfd_false, evax_write_object_contents,	/* bfd_write_contents */
   _bfd_write_archive_contents, bfd_false},

  BFD_JUMP_TABLE_GENERIC (evax),
  BFD_JUMP_TABLE_COPY (evax),
  BFD_JUMP_TABLE_CORE (evax),
  BFD_JUMP_TABLE_ARCHIVE (evax),
  BFD_JUMP_TABLE_SYMBOLS (evax),
  BFD_JUMP_TABLE_RELOCS (evax),
  BFD_JUMP_TABLE_WRITE (evax),
  BFD_JUMP_TABLE_LINK (evax),
  BFD_JUMP_TABLE_DYNAMIC (evax),

  (PTR) 0
};


/*===========================================================================*/

/* Initialize private data  */

static boolean
evax_initialize (abfd)
     bfd *abfd;
{
  int i;

  if (abfd->tdata.any != 0)
    return true;

  bfd_set_start_address (abfd, (bfd_vma)-1);

  abfd->tdata.any = ((struct evax_private_data_struct*)
		     bfd_malloc (sizeof (struct evax_private_data_struct)));
  if (abfd->tdata.any == 0)
    return false;
  PRIV(evax_buf) = 0;
  PRIV(buf_size) = 0;
  PRIV(rec_length) = 0;
  PRIV(file_format) = FF_UNKNOWN;
  PRIV(filename) = NULL;
  PRIV(fixup_done) = false;
  PRIV(sections) = NULL;

  PRIV(stack) = ((struct stack_struct *)
		 bfd_malloc (sizeof (struct stack_struct) * STACKSIZE));
  if (PRIV(stack) == 0)
    {
     evax_init_no_mem1:
      free (abfd->tdata.any);
      abfd->tdata.any = 0;
      return false;
    }
  PRIV(stackptr) = 0;

  PRIV(evax_symbol_table) = ((struct bfd_hash_table *)
			     bfd_malloc (sizeof (struct bfd_hash_table)));
  if (PRIV(evax_symbol_table) == 0)
    {
     evax_init_no_mem2:
      free (PRIV(stack));
      PRIV(stack) = 0;
      goto evax_init_no_mem1;
    }

  if (!bfd_hash_table_init (PRIV(evax_symbol_table), _bfd_evax_hash_newfunc))
    return false;

  PRIV(location_stack) = ((struct location_struct *)
			  bfd_malloc (sizeof (struct location_struct)
				      * LOCATION_SAVE_SIZE));
  if (PRIV(location_stack) == 0)
    {
     evax_init_no_mem3:
      free (PRIV(evax_symbol_table));
      PRIV(evax_symbol_table) = 0;
      goto evax_init_no_mem2;
    }

  for (i=0; i<EVAX_SECTION_COUNT; i++)
    {
      PRIV(evax_section_table)[i] = NULL;
      PRIV(evax_reloc_table)[i] = NULL;
    }

  PRIV(output_buf) = (unsigned char *) malloc (MAX_OUTREC_SIZE);
  if (PRIV(output_buf) == 0)
    {
      free (PRIV(location_stack));
      PRIV(location_stack) = 0;
      goto evax_init_no_mem3;
    }
  PRIV(push_level) = 0;
  PRIV(pushed_size) = 0;
  PRIV(length_pos) = 2;
  PRIV(output_size) = 0;
  PRIV(output_alignment) = 1;

  return true;
}


/* Fill symbol->section with section ptr
   symbol->section is filled with the section index for defined symbols
   during reading the EGSD section. But we need the pointer to the
   bfd section later.

   It has the correct value for referenced (undefined section) symbols

   called from bfd_hash_traverse in evax_fixup_sections  */

static boolean
fill_section_ptr (entry, sections)
     struct bfd_hash_entry *entry;
     PTR sections;
{
  asection *sec;
  asymbol *sym;

  sym =  ((evax_symbol_entry *)entry)->symbol;
  sec = sym->section;

  if (!bfd_is_und_section (sec))
    {
      sec = ((evax_symbol_entry *)entry)->symbol->section =
	((asection **)sections)[(int)sec];
    }

  if (strcmp (sym->name, sec->name) == 0)
    sym->flags |= BSF_SECTION_SYM;

  return true;
}


/* Fixup sections
   set up all pointers and arrays, counters and sizes are fixed now

   we build a private sections vector for easy access since sections
   are always referenced by an index number.

   alloc PRIV(sections) according to abfd->section_count
	copy abfd->sections to PRIV(sections)  */

static boolean
evax_fixup_sections (abfd)
     bfd *abfd;
{
  asection *s;

  if (PRIV(fixup_done))
    return true;

  PRIV(sections) = ((asection **)
		    bfd_malloc (abfd->section_count * sizeof (asection *)));
  if (PRIV(sections) == 0)
    return false;
  PRIV(egsd_sec_count) = abfd->section_count;
  s = abfd->sections;
  while (s)
    {
      PRIV(sections)[s->index] = s;
      s = s->next;
    }

  /*
   * traverse symbol table and fill in all section pointers
   */

  bfd_hash_traverse (PRIV(evax_symbol_table), fill_section_ptr,
		    (PTR)(PRIV(sections)));

  PRIV(fixup_done) = true;

  return true;
}

/*===========================================================================*/

/* Check the format for a file being read.
   Return a (bfd_target *) if it's an object file or zero if not.  */

static const struct bfd_target *
evax_object_p (abfd)
     bfd *abfd;
{
  int err = 0;
  int prev_type;
#if EVAX_DEBUG
  evax_debug (1, "evax_object_p(%p)\n", abfd);
#endif
  if (bfd_seek (abfd, 0L, SEEK_SET))
    {
      bfd_set_error (bfd_error_file_truncated);
      return 0;
    }

  prev_type = -1;

  do
    {
#if EVAX_DEBUG
      evax_debug (3, "reading at %08lx\n", bfd_tell(abfd));
#endif
      if (_bfd_evax_next_record (abfd) < 0)
	{
#if EVAX_DEBUG
	  evax_debug (2, "next_record failed\n");      
#endif
	  bfd_set_error (bfd_error_wrong_format);
	  return 0;
	}

      if ((prev_type == EOBJ_S_C_EGSD) && (PRIV(rec_type) != EOBJ_S_C_EGSD))
	{
	  if (evax_fixup_sections (abfd) == false)
	    {
#if EVAX_DEBUG
	      evax_debug (2, "evax_fixup_sections failed\n");
#endif
	      bfd_set_error (bfd_error_wrong_format);
	      return 0;
	    }
	}

      prev_type = PRIV(rec_type);

      switch (PRIV(rec_type))
	{
	  case EOBJ_S_C_EMH:
	    err = _bfd_evax_slurp_emh (abfd);
	    break;
	  case EOBJ_S_C_EEOM:
	    err = _bfd_evax_slurp_eeom (abfd);
	    break;
	  case EOBJ_S_C_EGSD:
	    err = _bfd_evax_slurp_egsd (abfd);
	    break;
	  case EOBJ_S_C_ETIR:
	    err = _bfd_evax_slurp_etir (abfd);
	    break;
	  case EOBJ_S_C_EDBG:
	    err = _bfd_evax_slurp_edbg (abfd);
	    break;
	  case EOBJ_S_C_ETBT:
	    err = _bfd_evax_slurp_etbt (abfd);
	    break;
	  default:
	    err = -1;
	}
      if (err != 0)
	{
#if EVAX_DEBUG
	  evax_debug (2, "slurp type %d failed with %d\n", PRIV(rec_type), err);
#endif
	  bfd_set_error (bfd_error_wrong_format);
	  return 0;
	}
    }
  while (prev_type != EOBJ_S_C_EEOM);

  /* set arch_info to alpha  */
 
  {
    const bfd_arch_info_type *arch = bfd_scan_arch ("alpha");
    if (arch == 0)
      {
#if EVAX_DEBUG
	evax_debug (2, "arch not found\n");
#endif
	bfd_set_error (bfd_error_wrong_format);
	return 0;
      }
    abfd->arch_info = arch;
  }

  return &evax_alpha_vec;
}


/* Check the format for a file being read.
   Return a (bfd_target *) if it's an archive file or zero.  */

static const struct bfd_target *
evax_archive_p (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_archive_p(%p)\n", abfd);
#endif

  if (!evax_initialize (abfd))
    return 0;

  return 0;
}


/* Set the format of a file being written.  */

static boolean
evax_mkobject (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_mkobject(%p)\n", abfd);
#endif

  if (!evax_initialize (abfd))
    return 0;

  {
    const bfd_arch_info_type *arch = bfd_scan_arch ("alpha");
    if (arch == 0)
      {
	bfd_set_error(bfd_error_wrong_format);
	return 0;
      }
    abfd->arch_info = arch;
  }

  return true;
}


/* Write cached information into a file being written, at bfd_close.  */

static boolean
evax_write_object_contents (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_write_object_contents(%p)\n", abfd);
#endif

  if (abfd->section_count > 0)			/* we have sections */
    {
      if (_bfd_evax_write_emh (abfd) != 0)
	return false;
      if (_bfd_evax_write_egsd (abfd) != 0)
	return false;
      if (_bfd_evax_write_etir (abfd) != 0)
	return false;
      if (_bfd_evax_write_etbt (abfd) != 0)
	return false;
      if (_bfd_evax_write_eeom (abfd) != 0)
	return false;
    }
  return true;
}

/*-- 4.1, generic -----------------------------------------------------------*/

/* Called when the BFD is being closed to do any necessary cleanup.  */

static boolean
evax_close_and_cleanup (abfd)
     bfd *abfd;
{
  asection *sec;
  evax_section *es, *es1;
  evax_reloc *er, *er1;
  int i;

#if EVAX_DEBUG
  evax_debug (1, "evax_close_and_cleanup(%p)\n", abfd);
#endif
  if (abfd == 0)
    return true;

  if (PRIV(evax_buf) != NULL)
    {
      free (PRIV(evax_buf));
      PRIV(evax_buf) = NULL;
    }
  PRIV(buf_size) = 0;

  if (PRIV(filename) != NULL)
    {
      free (PRIV(filename));
      PRIV(filename) = NULL;
    }

  if (PRIV(output_buf) != 0)
    {
      free (PRIV(output_buf));
      PRIV(output_buf) = 0;
    }

  sec = abfd->sections;
  while (sec != NULL)
    {
      if (sec->contents)
	free (sec->contents);
      sec = sec->next;
    }

  if (PRIV(sections) != NULL)
    {
      free (PRIV(sections));
      PRIV(sections) = NULL;
    }

  if (PRIV(evax_symbol_table))
    {
      bfd_hash_table_free (PRIV(evax_symbol_table));
      PRIV(evax_symbol_table) = 0;
    }

  if (PRIV(stack))
    {
      free (PRIV(stack));
      PRIV(stack) = 0;
    }

  if (PRIV(location_stack))
    {
      free (PRIV(location_stack));
      PRIV(location_stack) = 0;
    }

  for (i = 0; i < EVAX_SECTION_COUNT; i++)
    {
      es = PRIV(evax_section_table)[i];
      while (es != NULL)
	{
	  es1 = es->next;
	  free (es);
	  es = es1;
	}
      PRIV(evax_section_table)[i] = NULL;

      er = PRIV(evax_reloc_table)[i];
      while (er != NULL)
	{
	  er1 = er->next;
	  free (er);
	  er = er1;
	}
      PRIV(evax_reloc_table)[i] = NULL;
   }

  free (abfd->tdata.any);
  abfd->tdata.any = NULL;

  return true;
}


/* Ask the BFD to free all cached information.  */
static boolean
evax_bfd_free_cached_info (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_free_cached_info(%p)\n", abfd);
#endif
  return true;
}


/* Called when a new section is created.  */

static boolean
evax_new_section_hook (abfd, section)
     bfd *abfd;
     asection *section;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_new_section_hook(%p, %s)\n", abfd, section->name);
#endif
  bfd_set_section_alignment(abfd, section, 4);
  return true;
}


/* Read the contents of a section.
   buf points to a buffer of buf_size bytes to be filled with
   section data (starting at offset into section)  */

static boolean
evax_get_section_contents (abfd, section, buf, offset, buf_size)
     bfd *abfd;
     asection *section;
     PTR buf;
     file_ptr offset;
     bfd_size_type buf_size;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_get_section_contents(%p, %s, %p, off %ld, size %d)\n",
		 abfd, section->name, buf, offset, (int)buf_size);
#endif

  /* shouldn't be called, since all sections are IN_MEMORY  */

  return false;
}

/* Read the contents of a section.
   buf points to a buffer of buf_size bytes to be filled with
   section data (starting at offset into section)  */

static boolean
evax_get_section_contents_in_window (abfd, section, w, offset, count)
     bfd *abfd;
     asection *section;
     bfd_window *w;
     file_ptr offset;
     bfd_size_type count;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_get_section_contents_in_window(%p, %s, %p, off %ld, count %d)\n",
		 abfd, section->name, w, offset, (int)count);
#endif

  /* shouldn't be called, since all sections are IN_MEMORY  */

  return false;
}

/*-- Part 4.2, copy private data --------------------------------------------*/

/* Called to copy BFD general private data from one object file
   to another.  */

static boolean
evax_bfd_copy_private_bfd_data (src, dest)
     bfd *src;
     bfd *dest;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_copy_private_bfd_data(%p, %p)\n", src, dest);
#endif
  return true;
}


/* Merge private BFD information from the BFD @var{ibfd} to the
   the output file BFD @var{obfd} when linking.  Return <<true>>
   on success, <<false>> on error.  Possible error returns are:

   o <<bfd_error_no_memory>> -
     Not enough memory exists to create private data for @var{obfd}.  */

static boolean
evax_bfd_merge_private_bfd_data (ibfd, obfd)
     bfd *ibfd;
     bfd *obfd;
{
#if EVAX_DEBUG
  evax_debug (1,"evax_bfd_merge_private_bfd_data(%p, %p)\n", ibfd, obfd);
#endif
  return true;
}


/* Set private BFD flag information in the BFD @var{abfd}.
   Return <<true>> on success, <<false>> on error.  Possible error
   returns are:

   o <<bfd_error_no_memory>> -
     Not enough memory exists to create private data for @var{obfd}.  */

static boolean
evax_bfd_set_private_flags (abfd, flags)
     bfd *abfd;
     flagword flags;
{
#if EVAX_DEBUG
  evax_debug (1,"evax_bfd_set_private_flags(%p, %lx)\n", abfd, (long)flags);
#endif
  return true;
}


/* Called to copy BFD private section data from one object file
   to another.  */

static boolean
evax_bfd_copy_private_section_data (srcbfd, srcsec, dstbfd, dstsec)
     bfd *srcbfd;
     asection *srcsec;
     bfd *dstbfd;
     asection *dstsec;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_copy_private_section_data(%p, %s, %p, %s)\n",
		 srcbfd, srcsec->name, dstbfd, dstsec->name);
#endif
  return true;
}

/* Called to copy BFD private symbol data from one object file
   to another.  */

static boolean 
evax_bfd_copy_private_symbol_data (ibfd, isym, obfd, osym)
     bfd *ibfd;
     asymbol *isym;
     bfd *obfd;
     asymbol *osym;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_copy_private_symbol_data(%p, %s, %p, %s)\n",
		 ibfd, isym->name, obfd, osym->name);
#endif
  return true;
}

/*-- Part 4.3, core file ----------------------------------------------------*/

/* Return a read-only string explaining which program was running
   when it failed and produced the core file abfd.  */

static char *
evax_core_file_failing_command (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_core_file_failing_command(%p)\n", abfd);
#endif
  return 0;
}


/* Returns the signal number which caused the core dump which
   generated the file the BFD abfd is attached to.  */

static int
evax_core_file_failing_signal (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_core_file_failing_signal(%p)\n", abfd);
#endif
  return 0;
}


/* Return true if the core file attached to core_bfd was generated
   by a run of the executable file attached to exec_bfd, false otherwise.  */

static boolean
evax_core_file_matches_executable_p (abfd, bbfd)
     bfd *abfd;
     bfd *bbfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_core_file_matches_executable_p(%p, %p)\n", abfd, bbfd);
#endif
  return false;
}

/*-- Part 4.4, archive ------------------------------------------------------*/

/* ???	do something with an archive map.
   Return false on error, true otherwise.  */

static boolean
evax_slurp_armap (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_slurp_armap(%p)\n", abfd);
#endif
  return false;
}


/* ???	do something with an extended name table.
   Return false on error, true otherwise.  */

static boolean
evax_slurp_extended_name_table (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_slurp_extended_name_table(%p)\n", abfd);
#endif
  return false;
}


/* ???	do something with an extended name table.
   Return false on error, true otherwise.  */

static boolean
evax_construct_extended_name_table (abfd, tabloc, tablen, name)
     bfd *abfd;
     char **tabloc;
     bfd_size_type *tablen;
     const char **name;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_construct_extended_name_table(%p)\n", abfd);
#endif
  return false;
}


/* Truncate the name of an archive to match system-dependent restrictions  */

static void
evax_truncate_arname (abfd, pathname, arhdr)
     bfd *abfd;
     CONST char *pathname;
     char *arhdr;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_truncate_arname(%p, %s, %s)\n", abfd, pathname, arhdr);
#endif
  return;
}


/* ???	write archive map  */

static boolean
evax_write_armap (arch, elength, map, orl_count, stridx)
     bfd *arch;
     unsigned int elength;
     struct orl *map;
     unsigned int orl_count;
     int stridx;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_write_armap(%p, %d, %p, %d %d)\n",
	arch, elength, map, orl_count, stridx);
#endif
  return true;
}

/* Read archive header ???  */

static PTR
evax_read_ar_hdr (abfd)
    bfd * abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_read_ar_hdr(%p)\n", abfd);
#endif
  return (PTR)0;
}


/* Provided a BFD, @var{archive}, containing an archive and NULL, open
   an input BFD on the first contained element and returns that.
   Subsequent calls should pass the archive and the previous return value
   to return a created BFD to the next contained element.
   NULL is returned when there are no more.  */

static bfd *
evax_openr_next_archived_file (arch, prev)
     bfd *arch;
     bfd *prev;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_openr_next_archived_file(%p, %p)\n", arch, prev);
#endif
  return false;
}


/* Return the BFD which is referenced by the symbol in ABFD indexed by
   INDEX.  INDEX should have been returned by bfd_get_next_mapent.  */

static bfd *
evax_get_elt_at_index (abfd, index)
     bfd *abfd;
     symindex index;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_get_elt_at_index(%p, %p)\n", abfd, index);
#endif
  return _bfd_generic_get_elt_at_index(abfd, index);
}


/* ???
   -> bfd_generic_stat_arch_elt  */

static int
evax_generic_stat_arch_elt (abfd, stat)
     bfd *abfd;
     struct stat *stat;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_generic_stat_arch_elt(%p, %p)\n", abfd, stat);
#endif
  return bfd_generic_stat_arch_elt(abfd, stat);
}


/* This is a new function in bfd 2.5  */

static boolean
evax_update_armap_timestamp (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_update_armap_timestamp(%p)\n", abfd);
#endif
  return true;
}

/*-- Part 4.5, symbols --------------------------------------------------------*/

/* Return the number of bytes required to store a vector of pointers
   to asymbols for all the symbols in the BFD abfd, including a
   terminal NULL pointer. If there are no symbols in the BFD,
   then return 0.  If an error occurs, return -1.  */

static long
evax_get_symtab_upper_bound (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_get_symtab_upper_bound(%p), %d symbols\n", abfd, PRIV(egsd_sym_count));
#endif
  return (PRIV(egsd_sym_count)+1) * sizeof(asymbol *);
}


/* Copy symbols from hash table to symbol vector

   called from bfd_hash_traverse in evax_get_symtab
   init counter to 0 if entry == 0  */

static boolean
copy_symbols (entry, arg)
     struct bfd_hash_entry *entry;
     PTR arg;
{
  bfd *abfd = (bfd *) arg;

  if (entry == NULL)	/* init counter */
    PRIV(symnum) = 0;
  else			/* fill vector, inc counter */
    PRIV(symcache)[PRIV(symnum)++] = ((evax_symbol_entry *)entry)->symbol;

  return true;
}


/* Read the symbols from the BFD abfd, and fills in the vector
   location with pointers to the symbols and a trailing NULL.

   return # of symbols read  */

static long
evax_get_symtab (abfd, symbols)
     bfd *abfd;
     asymbol **symbols;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_get_symtab(%p, <ret>)\n", abfd);
#endif

	/* init counter */
  (void)copy_symbols((struct bfd_hash_entry *)0, abfd);

	/* traverse table and fill symbols vector */

  PRIV(symcache) = symbols;
  bfd_hash_traverse(PRIV(evax_symbol_table), copy_symbols, (PTR)abfd);

  symbols[bfd_get_symcount(abfd)] = NULL;

  return bfd_get_symcount(abfd);
}


/* Create a new asymbol structure for the BFD abfd and return a pointer
   to it.
   This routine is necessary because each back end has private information
   surrounding the asymbol. Building your own asymbol and pointing to it
   will not create the private information, and will cause problems later on.  */

asymbol *
_bfd_evax_make_empty_symbol (abfd)
     bfd *abfd;
{
  asymbol *symbol = (asymbol *)bfd_zalloc(abfd, sizeof(asymbol));

#if EVAX_DEBUG
  evax_debug (1, "_bfd_evax_make_empty_symbol(%p)\n", abfd);
#endif

  if (symbol == 0)
    {
      bfd_set_error (bfd_error_no_memory);
      return 0;
    }
  symbol->the_bfd = abfd;

  return symbol;
}


/* Print symbol to file according to how. how is one of
   bfd_print_symbol_name	just print the name
   bfd_print_symbol_more	print more (???)
   bfd_print_symbol_all	print all we know, which is not much right now :-)  */

static void
evax_print_symbol (abfd, file, symbol, how)
     bfd *abfd;
     PTR file;
     asymbol *symbol;
     bfd_print_symbol_type how;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_print_symbol(%p, %p, %p, %d)\n", abfd, file, symbol, how);
#endif

  switch (how)
    {
      case bfd_print_symbol_name:
      case bfd_print_symbol_more:
	fprintf((FILE *)file," %s", symbol->name);
      break;

      break;

      case bfd_print_symbol_all:
	{
	  CONST char *section_name = symbol->section->name;

	  bfd_print_symbol_vandf((PTR)file,symbol);

	  fprintf((FILE *)file," %-8s %s", section_name, symbol->name);
        }
      break;
    }
  return;
}


/* Return information about symbol in ret.

   fill type, value and name
   type:
	A	absolute
	B	bss segment symbol
	C	common symbol
	D	data segment symbol
	f	filename
	t	a static function symbol
	T	text segment symbol
	U	undefined
	-	debug  */

static void
evax_get_symbol_info (abfd, symbol, ret)
     bfd *abfd;
     asymbol *symbol;
     symbol_info *ret;
{
  asection *sec;

#if EVAX_DEBUG
  evax_debug (1, "evax_get_symbol_info(%p, %p, <ret>)\n", abfd, symbol);
#endif

  sec = symbol->section;

  if (bfd_is_com_section (sec))
    ret->type = 'C';
  else if (bfd_is_abs_section (sec))
    ret->type = 'A';
  else if (bfd_is_und_section (sec))
    ret->type = 'U';
  else if (bfd_is_abs_section (sec))
    ret->type = 'A';
  else if (bfd_is_ind_section (sec))
    ret->type = 'I';
  else if (bfd_get_section_flags (abfd, sec) & SEC_CODE)
    ret->type = 'T';
  else if (bfd_get_section_flags (abfd, sec) & SEC_DATA)
    ret->type = 'D';
  else if (bfd_get_section_flags (abfd, sec) & SEC_ALLOC)
    ret->type = 'B';
  else
    ret->type = '-';

  if (ret->type != 'U')
    ret->value = symbol->value + symbol->section->vma;
  else
    ret->value = 0;
  ret->name = symbol->name;

  return;
}


/* Return true if the given symbol sym in the BFD abfd is
   a compiler generated local label, else return false.  */

static boolean
evax_bfd_is_local_label (abfd, symbol)
     bfd *abfd;
     asymbol *symbol;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_is_local_label(%p, %p)\n", abfd, symbol);
#endif
  return false;
}


/* Get source line number for symbol  */

static alent *
evax_get_lineno (abfd, symbol)
     bfd *abfd;
     asymbol *symbol;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_get_lineno(%p, %p)\n", abfd, symbol);
#endif
  return 0;
}


/* Provided a BFD, a section and an offset into the section, calculate and
   return the name of the source file and the line nearest to the wanted
   location.  */

static boolean
evax_find_nearest_line (abfd, section, symbols, offset, file, func, line)
     bfd *abfd;
     asection *section;
     asymbol **symbols;
     bfd_vma offset;
     CONST char **file;
     CONST char **func;
     unsigned int *line;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_find_nearest_line(%p, %s, %p, %ld, <ret>, <ret>, <ret>)\n",
	      abfd, section->name, symbols, (long int)offset);
#endif
  return false;
}


/* Back-door to allow format-aware applications to create debug symbols
   while using BFD for everything else.  Currently used by the assembler
   when creating COFF files.  */

static asymbol *
evax_bfd_make_debug_symbol (abfd, ptr, size)
     bfd *abfd;
     void *ptr;
     unsigned long size;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_make_debug_symbol(%p, %p, %ld)\n", abfd, ptr, size);
#endif
  return 0;
}


/* Read minisymbols.  For minisymbols, we use the unmodified a.out
   symbols.  The minisymbol_to_symbol function translates these into
   BFD asymbol structures.  */

static long
evax_read_minisymbols (abfd, dynamic, minisymsp, sizep)
     bfd *abfd;
     boolean dynamic;
     PTR *minisymsp;
     unsigned int *sizep;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_read_minisymbols(%p, %d, %p, %d)\n", abfd, dynamic, minisymsp, *sizep);
#endif
  return _bfd_generic_read_minisymbols (abfd, dynamic, minisymsp, sizep);
}

/* Convert a minisymbol to a BFD asymbol.  A minisymbol is just an
   unmodified a.out symbol.  The SYM argument is a structure returned
   by bfd_make_empty_symbol, which we fill in here.  */

static asymbol *
evax_minisymbol_to_symbol (abfd, dynamic, minisym, sym)
     bfd *abfd;
     boolean dynamic;
     const PTR minisym;
     asymbol *sym;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_minisymbol_to_symbol(%p, %d, %p, %p)\n", abfd, dynamic, minisym, sym);
#endif
  return _bfd_generic_minisymbol_to_symbol (abfd, dynamic, minisym, sym);
}

/*-- Part 4.6, relocations --------------------------------------------------*/

/* Return the number of bytes required to store the relocation information
   associated with section sect attached to bfd abfd.
   If an error occurs, return -1.  */

static long
evax_get_reloc_upper_bound (abfd, section)
     bfd *abfd;
     asection *section;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_get_reloc_upper_bound(%p, %s)\n", abfd, section->name);
#endif
  return -1L;
}


/* Call the back end associated with the open BFD abfd and translate the
   external form of the relocation information attached to sec into the
   internal canonical form.  Place the table into memory at loc, which has
   been preallocated, usually by a call to bfd_get_reloc_upper_bound.
   Returns the number of relocs, or -1 on error.  */

static long
evax_canonicalize_reloc (abfd, section, location, symbols)
     bfd *abfd;
     asection *section;
     arelent **location;
     asymbol **symbols;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_canonicalize_reloc(%p, %s, <ret>, <ret>)\n", abfd, section->name);
#endif
  return false;
}

/*---------------------------------------------------------------------------*/
/* this is just copied from ecoff-alpha, needs to be fixed probably */

/* How to process the various reloc types.  */

static bfd_reloc_status_type
reloc_nil (abfd, reloc, sym, data, sec, output_bfd, error_message)
     bfd *abfd;
     arelent *reloc;
     asymbol *sym;
     PTR data;
     asection *sec;
     bfd *output_bfd;
     char **error_message;
{
  evax_reloc *er;

#if EVAX_DEBUG
  evax_debug (1, "reloc_nil(abfd %p, output_bfd %p)\n", abfd, output_bfd);
  evax_debug (2, "In section %s, symbol %s\n",
	sec->name, sym->name);
  evax_debug (2, "reloc sym %s, addr %08lx, addend %08lx, reloc is a %s\n",
		reloc->sym_ptr_ptr[0]->name,
		(unsigned long)reloc->address,
		(unsigned long)reloc->addend, reloc->howto->name);
  evax_debug (2, "data at %p\n", data);
/*  _bfd_hexdump (2, data, bfd_get_reloc_size(reloc->howto),0); */
#endif

  er = (evax_reloc *)malloc (sizeof(evax_reloc));
  if (er == NULL)
    {
      bfd_set_error (bfd_error_no_memory);
      return bfd_reloc_notsupported;	/* FIXME */
    }
  er->section = sec;
  er->reloc = reloc;
  er->next = NULL;
  if (PRIV(evax_reloc_table)[sec->index] == NULL)
    {
      PRIV(evax_reloc_table)[sec->index] = er;
    }
  else
    {
      er->next = PRIV(evax_reloc_table)[sec->index];
    }
  return bfd_reloc_ok;
}

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value
   from smaller values.  Start with zero, widen, *then* decrement.  */
#define MINUS_ONE	(((bfd_vma)0) - 1)

static reloc_howto_type alpha_howto_table[] =
{
  HOWTO (ALPHA_R_IGNORE,	/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "IGNORE",		/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 64 bit reference to a symbol.  */
  HOWTO (ALPHA_R_REFQUAD,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "REFQUAD",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 21 bit branch.  The native assembler generates these for
     branches within the text segment, and also fills in the PC
     relative offset in the instruction.  */
  HOWTO (ALPHA_R_BRADDR,	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 21,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "BRADDR",		/* name */
	 true,			/* partial_inplace */
	 0x1fffff,		/* src_mask */
	 0x1fffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A hint for a jump to a register.  */
  HOWTO (ALPHA_R_HINT,		/* type */
	 2,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 14,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "HINT",		/* name */
	 true,			/* partial_inplace */
	 0x3fff,		/* src_mask */
	 0x3fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 16 bit PC relative offset.  */
  HOWTO (ALPHA_R_SREL16,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "SREL16",		/* name */
	 true,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 32 bit PC relative offset.  */
  HOWTO (ALPHA_R_SREL32,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "SREL32",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 64 bit PC relative offset.  */
  HOWTO (ALPHA_R_SREL64,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "SREL64",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Push a value on the reloc evaluation stack.  */
  HOWTO (ALPHA_R_OP_PUSH,	/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "OP_PUSH",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* Store the value from the stack at the given address.  Store it in
     a bitfield of size r_size starting at bit position r_offset.  */
  HOWTO (ALPHA_R_OP_STORE,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "OP_STORE",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Subtract the reloc address from the value on the top of the
     relocation stack.  */
  HOWTO (ALPHA_R_OP_PSUB,	/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "OP_PSUB",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* Shift the value on the top of the relocation stack right by the
     given value.  */
  HOWTO (ALPHA_R_OP_PRSHIFT,	/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "OP_PRSHIFT",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* Hack. Linkage is done by linker.  */
  HOWTO (ALPHA_R_LINKAGE,	/* type */
	 0,			/* rightshift */
	 8,			/* size (0 = byte, 1 = short, 2 = long) */
	 256,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "LINKAGE",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* Switch table, 32bit relocation.  */
  HOWTO (ALPHA_R_SWREL32,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "SWREL32",		/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Switch table, 64bit relocation.  */
  HOWTO (ALPHA_R_SWREL64,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "SWREL64",		/* name */
	 false,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),			/* pcrel_offset */

  /* A 32 bit reference to a symbol.  */
  HOWTO (ALPHA_R_REFLONG,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 reloc_nil,		/* special_function */
	 "REFLONG",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

};

/* Return a pointer to a howto structure which, when invoked, will perform
   the relocation code on data from the architecture noted.  */

static const struct reloc_howto_struct *
evax_bfd_reloc_type_lookup (abfd, code)
     bfd *abfd;
     bfd_reloc_code_real_type code;
{
  int alpha_type;

#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_reloc_type_lookup(%p, %d)\t", abfd, code);
#endif

  switch (code)
    {
      case BFD_RELOC_16:		alpha_type = ALPHA_R_SREL16;	break;
      case BFD_RELOC_32:		alpha_type = ALPHA_R_REFLONG;	break;
      case BFD_RELOC_64:		alpha_type = ALPHA_R_REFQUAD;	break;
      case BFD_RELOC_CTOR:		alpha_type = ALPHA_R_REFQUAD;	break;
      case BFD_RELOC_23_PCREL_S2:	alpha_type = ALPHA_R_BRADDR;	break;
      case BFD_RELOC_ALPHA_HINT:	alpha_type = ALPHA_R_HINT;	break;
      case BFD_RELOC_16_PCREL:		alpha_type = ALPHA_R_SREL16;	break;
      case BFD_RELOC_32_PCREL:		alpha_type = ALPHA_R_SREL32;	break;
      case BFD_RELOC_64_PCREL:		alpha_type = ALPHA_R_SREL64;	break;
      case BFD_RELOC_ALPHA_LINKAGE:	alpha_type = ALPHA_R_LINKAGE;	break;
      case BFD_RELOC_SWREL32:		alpha_type = ALPHA_R_SWREL32;	break;
      case BFD_RELOC_SWREL64:		alpha_type = ALPHA_R_SWREL64;	break;
#if 0
      case ???:				alpha_type = ALPHA_R_OP_PUSH;	break;
      case ???:				alpha_type = ALPHA_R_OP_STORE;	break;
      case ???:				alpha_type = ALPHA_R_OP_PSUB;	break;
      case ???:				alpha_type = ALPHA_R_OP_PRSHIFT;break;
      case ???:				alpha_type = ALPHA_R_GPVALUE;	break;
#endif
      default:
	(*_bfd_error_handler) ("reloc (%d) is *UNKNOWN*", code);
	return (const struct reloc_howto_struct *) NULL;
    }
#if EVAX_DEBUG
  evax_debug (2, "reloc is %s\n", alpha_howto_table[alpha_type].name);
#endif
  return &alpha_howto_table[alpha_type];
}


/*-- Part 4.7, writing an object file ---------------------------------------*/

/* Set the architecture and machine type in BFD abfd to arch and mach.
   Find the correct pointer to a structure and insert it into the arch_info
   pointer.  */

static boolean
evax_set_arch_mach (abfd, arch, mach)
     bfd *abfd;
     enum bfd_architecture arch;
     unsigned long mach;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_set_arch_mach(%p, %d, %ld)\n", abfd, arch, mach);
#endif
  abfd->arch_info = bfd_scan_arch("alpha");

  return true;
}


/* Sets the contents of the section section in BFD abfd to the data starting
   in memory at data. The data is written to the output section starting at
   offset offset for count bytes.

   Normally true is returned, else false. Possible error returns are:
   o bfd_error_no_contents - The output section does not have the
	SEC_HAS_CONTENTS attribute, so nothing can be written to it.
   o and some more too  */

static boolean
evax_set_section_contents (abfd, section, location, offset, count)
     bfd *abfd;
     asection *section;
     PTR location;
     file_ptr offset;
     bfd_size_type count;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_set_section_contents(%p, sec %s, loc %p, off %ld, count %d)\n",
					abfd, section->name, location, (long int)offset, (int)count);
#endif
  return _bfd_save_evax_section(abfd, section, location, offset, count);
}


/*-- Part 4.8, linker -------------------------------------------------------*/

/* Get the size of the section headers.  */

static int
evax_sizeof_headers (abfd, reloc)
     bfd *abfd;
     boolean reloc;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_sizeof_headers(%p, %s)\n", abfd, (reloc)?"True":"False");
#endif
  return 0;
}


/* Provides default handling of relocation effort for back ends
   which can't be bothered to do it efficiently.  */

static bfd_byte *
evax_bfd_get_relocated_section_contents (abfd, link_info, link_order, data,
					 relocateable, symbols)
     bfd *abfd;
     struct bfd_link_info *link_info;
     struct bfd_link_order *link_order;
     bfd_byte *data;
     boolean relocateable;
     asymbol **symbols;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_get_relocated_section_contents(%p, %p, %p, %p, %s, %p)\n",
			abfd, link_info, link_order, data, (relocateable)?"True":"False", symbols);
#endif
  return 0;
}


/* ???  */

static boolean
evax_bfd_relax_section (abfd, section, link_info, again)
     bfd *abfd;
     asection *section;
     struct bfd_link_info *link_info;
     boolean *again;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_relax_section(%p, %s, %p, <ret>)\n",
					abfd, section->name, link_info);
#endif
  return true;
}


/* Create a hash table for the linker.  Different backends store
   different information in this table.  */

static struct bfd_link_hash_table *
evax_bfd_link_hash_table_create (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_link_hash_table_create(%p)\n", abfd);
#endif
  return 0;
}


/* Add symbols from this object file into the hash table.  */

static boolean
evax_bfd_link_add_symbols (abfd, link_info)
     bfd *abfd;
     struct bfd_link_info *link_info;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_link_add_symbols(%p, %p)\n", abfd, link_info);
#endif
  return false;
}


/* Do a link based on the link_order structures attached to each
   section of the BFD.  */

static boolean
evax_bfd_final_link (abfd, link_info)
     bfd *abfd;
     struct bfd_link_info *link_info;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_final_link(%p, %p)\n", abfd, link_info);
#endif
  return true;
}

/* Should this section be split up into smaller pieces during linking.  */

static boolean
evax_bfd_link_split_section (abfd, section)
     bfd *abfd;
     asection *section;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_link_split_section(%p, %s)\n", abfd, section->name);
#endif
  return false;
}

/*-- Part 4.9, dynamic symbols and relocations ------------------------------*/

/* Get the amount of memory required to hold the dynamic symbols.  */

static long
evax_get_dynamic_symtab_upper_bound (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_get_dynamic_symtab_upper_bound(%p)\n", abfd);
#endif
  return 0;
}

static boolean
evax_bfd_print_private_bfd_data (abfd, file)
    bfd *abfd;
    void *file;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_bfd_print_private_bfd_data(%p)\n", abfd);
#endif
  return 0;
}


/* Read in the dynamic symbols.  */

static long
evax_canonicalize_dynamic_symtab (abfd, symbols)
     bfd *abfd;
     asymbol **symbols;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_canonicalize_dynamic_symtab(%p, <ret>)\n", abfd);
#endif
  return 0L;
}


/* Get the amount of memory required to hold the dynamic relocs.  */

static long
evax_get_dynamic_reloc_upper_bound (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_get_dynamic_reloc_upper_bound(%p)\n", abfd);
#endif
  return 0L;
}


/* Read in the dynamic relocs.  */

static long
evax_canonicalize_dynamic_reloc (abfd, arel, symbols)
     bfd *abfd;
     arelent **arel;
     asymbol **symbols;
{
#if EVAX_DEBUG
  evax_debug (1, "evax_canonicalize_dynamic_reloc(%p)\n", abfd);
#endif
  return 0L;
}
