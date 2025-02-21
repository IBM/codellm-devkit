/* POWER/PowerPC XCOFF linker support.
   Copyright 1995, 1996 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <ian@cygnus.com>, Cygnus Support.

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
#include "coff/internal.h"
#include "libcoff.h"

/* This file holds the XCOFF linker code.  */

#define STRING_SIZE_SIZE (4)

/* In order to support linking different object file formats into an
   XCOFF format, we need to be able to determine whether a particular
   bfd_target is an XCOFF vector.  FIXME: We need to rethink this
   whole approach.  */
#define XCOFF_XVECP(xv) \
  (strcmp ((xv)->name, "aixcoff-rs6000") == 0 \
   || strcmp ((xv)->name, "xcoff-powermac") == 0)

/* Get the XCOFF hash table entries for a BFD.  */
#define obj_xcoff_sym_hashes(bfd) \
  ((struct xcoff_link_hash_entry **) obj_coff_sym_hashes (bfd))

/* XCOFF relocation types.  These probably belong in a header file
   somewhere.  The relocations are described in the function
   _bfd_ppc_xcoff_relocate_section in this file.  */

#define R_POS   (0x00)
#define R_NEG   (0x01)
#define R_REL   (0x02)
#define R_TOC   (0x03)
#define R_RTB   (0x04)
#define R_GL    (0x05)
#define R_TCL   (0x06)
#define R_BA    (0x08)
#define R_BR    (0x0a)
#define R_RL    (0x0c)
#define R_RLA   (0x0d)
#define R_REF   (0x0f)
#define R_TRL   (0x12)
#define R_TRLA  (0x13)
#define R_RRTBI (0x14)
#define R_RRTBA (0x15)
#define R_CAI   (0x16)
#define R_CREL  (0x17)
#define R_RBA   (0x18)
#define R_RBAC  (0x19)
#define R_RBR   (0x1a)
#define R_RBRC  (0x1b)

/* The first word of global linkage code.  This must be modified by
   filling in the correct TOC offset.  */

#define XCOFF_GLINK_FIRST (0x81820000)	/* lwz r12,0(r2) */

/* The remaining words of global linkage code.  */

static unsigned long xcoff_glink_code[] =
{
  0x90410014,	/* stw r2,20(r1) */
  0x800c0000,	/* lwz r0,0(r12) */
  0x804c0004,	/* lwz r2,4(r12) */
  0x7c0903a6,	/* mtctr r0 */
  0x4e800420,	/* bctr */
  0x0,		/* start of traceback table */
  0x000c8000,	/* traceback table */
  0x0		/* traceback table */
};

#define XCOFF_GLINK_SIZE \
  (((sizeof xcoff_glink_code / sizeof xcoff_glink_code[0]) * 4) + 4)

/* We reuse the SEC_ROM flag as a mark flag for garbage collection.
   This flag will only be used on input sections.  */

#define SEC_MARK (SEC_ROM)

/* The ldhdr structure.  This appears at the start of the .loader
   section.  */

struct internal_ldhdr
{
  /* The version number: currently always 1.  */
  unsigned long l_version;
  /* The number of symbol table entries.  */
  bfd_size_type l_nsyms;
  /* The number of relocation table entries.  */
  bfd_size_type l_nreloc;
  /* The length of the import file string table.  */
  bfd_size_type l_istlen;
  /* The number of import files.  */
  bfd_size_type l_nimpid;
  /* The offset from the start of the .loader section to the first
     entry in the import file table.  */
  bfd_size_type l_impoff;
  /* The length of the string table.  */
  bfd_size_type l_stlen;
  /* The offset from the start of the .loader section to the first
     entry in the string table.  */
  bfd_size_type l_stoff;
};

struct external_ldhdr
{
  bfd_byte l_version[4];
  bfd_byte l_nsyms[4];
  bfd_byte l_nreloc[4];
  bfd_byte l_istlen[4];
  bfd_byte l_nimpid[4];
  bfd_byte l_impoff[4];
  bfd_byte l_stlen[4];
  bfd_byte l_stoff[4];
};

#define LDHDRSZ (8 * 4)

/* The ldsym structure.  This is used to represent a symbol in the
   .loader section.  */

struct internal_ldsym
{
  union
    {
      /* The symbol name if <= SYMNMLEN characters.  */
      char _l_name[SYMNMLEN];
      struct
	{
	  /* Zero if the symbol name is more than SYMNMLEN characters.  */
	  long _l_zeroes;
	  /* The offset in the string table if the symbol name is more
             than SYMNMLEN characters.  */
	  long _l_offset;
	} _l_l;
    } _l;
  /* The symbol value.  */
  bfd_vma l_value;
  /* The symbol section number.  */
  short l_scnum;
  /* The symbol type and flags.  */
  char l_smtype;
  /* The symbol storage class.  */
  char l_smclas;
  /* The import file ID.  */
  bfd_size_type l_ifile;
  /* Offset to the parameter type check string.  */
  bfd_size_type l_parm;
};

struct external_ldsym
{
  union
    {
      bfd_byte _l_name[SYMNMLEN];
      struct
	{
	  bfd_byte _l_zeroes[4];
	  bfd_byte _l_offset[4];
	} _l_l;
    } _l;
  bfd_byte l_value[4];
  bfd_byte l_scnum[2];
  bfd_byte l_smtype[1];
  bfd_byte l_smclas[1];
  bfd_byte l_ifile[4];
  bfd_byte l_parm[4];
};

#define LDSYMSZ (8 + 3 * 4 + 2 + 2)

/* These flags are for the l_smtype field (the lower three bits are an
   XTY_* value).  */

/* Imported symbol.  */
#define L_IMPORT (0x40)
/* Entry point.  */
#define L_ENTRY (0x20)
/* Exported symbol.  */
#define L_EXPORT (0x10)

/* The ldrel structure.  This is used to represent a reloc in the
   .loader section.  */

struct internal_ldrel
{
  /* The reloc address.  */
  bfd_vma l_vaddr;
  /* The symbol table index in the .loader section symbol table.  */
  bfd_size_type l_symndx;
  /* The relocation type and size.  */
  short l_rtype;
  /* The section number this relocation applies to.  */
  short l_rsecnm;
};

struct external_ldrel
{
  bfd_byte l_vaddr[4];
  bfd_byte l_symndx[4];
  bfd_byte l_rtype[2];
  bfd_byte l_rsecnm[2];
};

#define LDRELSZ (2 * 4 + 2 * 2)

/* The list of import files.  */

struct xcoff_import_file
{
  /* The next entry in the list.  */
  struct xcoff_import_file *next;
  /* The path.  */
  const char *path;
  /* The file name.  */
  const char *file;
  /* The member name.  */
  const char *member;
};

/* An entry in the XCOFF linker hash table.  */

struct xcoff_link_hash_entry
{
  struct bfd_link_hash_entry root;

  /* Symbol index in output file.  Set to -1 initially.  Set to -2 if
     there is a reloc against this symbol.  */
  long indx;

  /* If we have created a TOC entry for this symbol, this is the .tc
     section which holds it.  */
  asection *toc_section;

  union
    {
      /* If we have created a TOC entry (the XCOFF_SET_TOC flag is
	 set), this is the offset in toc_section.  */
      bfd_vma toc_offset;
      /* If the TOC entry comes from an input file, this is set to the
         symbo lindex of the C_HIDEXT XMC_TC symbol.  */
      long toc_indx;
    } u;

  /* If this symbol is a function entry point which is called, this
     field holds a pointer to the function descriptor.  If this symbol
     is a function descriptor, this field holds a pointer to the
     function entry point.  */
  struct xcoff_link_hash_entry *descriptor;

  /* The .loader symbol table entry, if there is one.  */
  struct internal_ldsym *ldsym;

  /* The .loader symbol table index.  */
  long ldindx;

  /* Some linker flags.  */
  unsigned short flags;
  /* Symbol is referenced by a regular object.  */
#define XCOFF_REF_REGULAR (01)
  /* Symbol is defined by a regular object.  */
#define XCOFF_DEF_REGULAR (02)
  /* Symbol is defined by a dynamic object.  */
#define XCOFF_DEF_DYNAMIC (04)
  /* Symbol is used in a reloc being copied into the .loader section.  */
#define XCOFF_LDREL (010)
  /* Symbol is the entry point.  */
#define XCOFF_ENTRY (020)
  /* Symbol is called; this is, it appears in a R_BR reloc.  */
#define XCOFF_CALLED (040)
  /* Symbol needs the TOC entry filled in.  */
#define XCOFF_SET_TOC (0100)
  /* Symbol is explicitly imported.  */
#define XCOFF_IMPORT (0200)
  /* Symbol is explicitly exported.  */
#define XCOFF_EXPORT (0400)
  /* Symbol has been processed by xcoff_build_ldsyms.  */
#define XCOFF_BUILT_LDSYM (01000)
  /* Symbol is mentioned by a section which was not garbage collected.  */
#define XCOFF_MARK (02000)
  /* Symbol size is recorded in size_list list from hash table.  */
#define XCOFF_HAS_SIZE (04000)
  /* Symbol is a function descriptor.  */
#define XCOFF_DESCRIPTOR (010000)

  /* The storage mapping class.  */
  unsigned char smclas;
};

/* The XCOFF linker hash table.  */

struct xcoff_link_hash_table
{
  struct bfd_link_hash_table root;

  /* The .debug string hash table.  We need to compute this while
     reading the input files, so that we know how large the .debug
     section will be before we assign section positions.  */
  struct bfd_strtab_hash *debug_strtab;

  /* The .debug section we will use for the final output.  */
  asection *debug_section;

  /* The .loader section we will use for the final output.  */
  asection *loader_section;

  /* A count of non TOC relative relocs which will need to be
     allocated in the .loader section.  */
  size_t ldrel_count;

  /* The .loader section header.  */
  struct internal_ldhdr ldhdr;

  /* The .gl section we use to hold global linkage code.  */
  asection *linkage_section;

  /* The .tc section we use to hold toc entries we build for global
     linkage code.  */
  asection *toc_section;

  /* The .ds section we use to hold function descriptors which we
     create for exported symbols.  */
  asection *descriptor_section;

  /* The list of import files.  */
  struct xcoff_import_file *imports;

  /* Required alignment of sections within the output file.  */
  unsigned long file_align;

  /* Whether the .text section must be read-only.  */
  boolean textro;

  /* Whether garbage collection was done.  */
  boolean gc;

  /* A linked list of symbols for which we have size information.  */
  struct xcoff_link_size_list
    {
      struct xcoff_link_size_list *next;
      struct xcoff_link_hash_entry *h;
      bfd_size_type size;
    } *size_list;

  /* Magic sections: _text, _etext, _data, _edata, _end, end.  */
  asection *special_sections[6];
};

/* Information we keep for each section in the output file during the
   final link phase.  */

struct xcoff_link_section_info
{
  /* The relocs to be output.  */
  struct internal_reloc *relocs;
  /* For each reloc against a global symbol whose index was not known
     when the reloc was handled, the global hash table entry.  */
  struct xcoff_link_hash_entry **rel_hashes;
  /* If there is a TOC relative reloc against a global symbol, and the
     index of the TOC symbol is not known when the reloc was handled,
     an entry is added to this linked list.  This is not an array,
     like rel_hashes, because this case is quite uncommon.  */
  struct xcoff_toc_rel_hash
    {
      struct xcoff_toc_rel_hash *next;
      struct xcoff_link_hash_entry *h;
      struct internal_reloc *rel;
    } *toc_rel_hashes;
};

/* Information that we pass around while doing the final link step.  */

struct xcoff_final_link_info
{
  /* General link information.  */
  struct bfd_link_info *info;
  /* Output BFD.  */
  bfd *output_bfd;
  /* Hash table for long symbol names.  */
  struct bfd_strtab_hash *strtab;
  /* Array of information kept for each output section, indexed by the
     target_index field.  */
  struct xcoff_link_section_info *section_info;
  /* Symbol index of last C_FILE symbol (-1 if none).  */
  long last_file_index;
  /* Contents of last C_FILE symbol.  */
  struct internal_syment last_file;
  /* Symbol index of TOC symbol.  */
  long toc_symindx;
  /* Start of .loader symbols.  */
  struct external_ldsym *ldsym;
  /* Next .loader reloc to swap out.  */
  struct external_ldrel *ldrel;
  /* File position of start of line numbers.  */
  file_ptr line_filepos;
  /* Buffer large enough to hold swapped symbols of any input file.  */
  struct internal_syment *internal_syms;
  /* Buffer large enough to hold output indices of symbols of any
     input file.  */
  long *sym_indices;
  /* Buffer large enough to hold output symbols for any input file.  */
  bfd_byte *outsyms;
  /* Buffer large enough to hold external line numbers for any input
     section.  */
  bfd_byte *linenos;
  /* Buffer large enough to hold any input section.  */
  bfd_byte *contents;
  /* Buffer large enough to hold external relocs of any input section.  */
  bfd_byte *external_relocs;
};

static void xcoff_swap_ldhdr_in
  PARAMS ((bfd *, const struct external_ldhdr *, struct internal_ldhdr *));
static void xcoff_swap_ldhdr_out
  PARAMS ((bfd *, const struct internal_ldhdr *, struct external_ldhdr *));
static void xcoff_swap_ldsym_in
  PARAMS ((bfd *, const struct external_ldsym *, struct internal_ldsym *));
static void xcoff_swap_ldsym_out
  PARAMS ((bfd *, const struct internal_ldsym *, struct external_ldsym *));
static void xcoff_swap_ldrel_in
  PARAMS ((bfd *, const struct external_ldrel *, struct internal_ldrel *));
static void xcoff_swap_ldrel_out
  PARAMS ((bfd *, const struct internal_ldrel *, struct external_ldrel *));
static struct bfd_hash_entry *xcoff_link_hash_newfunc
  PARAMS ((struct bfd_hash_entry *, struct bfd_hash_table *, const char *));
static struct internal_reloc *xcoff_read_internal_relocs
  PARAMS ((bfd *, asection *, boolean, bfd_byte *, boolean,
	   struct internal_reloc *));
static boolean xcoff_link_add_object_symbols
  PARAMS ((bfd *, struct bfd_link_info *));
static boolean xcoff_link_check_archive_element
  PARAMS ((bfd *, struct bfd_link_info *, boolean *));
static boolean xcoff_link_check_ar_symbols
  PARAMS ((bfd *, struct bfd_link_info *, boolean *));
static boolean xcoff_link_check_dynamic_ar_symbols
  PARAMS ((bfd *, struct bfd_link_info *, boolean *));
static bfd_size_type xcoff_find_reloc
  PARAMS ((struct internal_reloc *, bfd_size_type, bfd_vma));
static boolean xcoff_link_add_symbols PARAMS ((bfd *, struct bfd_link_info *));
static boolean xcoff_link_add_dynamic_symbols
  PARAMS ((bfd *, struct bfd_link_info *));
static boolean xcoff_mark PARAMS ((struct bfd_link_info *, asection *));
static void xcoff_sweep PARAMS ((struct bfd_link_info *));
static boolean xcoff_build_ldsyms
  PARAMS ((struct xcoff_link_hash_entry *, PTR));
static boolean xcoff_link_input_bfd
  PARAMS ((struct xcoff_final_link_info *, bfd *));
static boolean xcoff_write_global_symbol
  PARAMS ((struct xcoff_link_hash_entry *, PTR));
static boolean xcoff_reloc_link_order
  PARAMS ((bfd *, struct xcoff_final_link_info *, asection *,
	   struct bfd_link_order *));
static int xcoff_sort_relocs PARAMS ((const PTR, const PTR));

/* Routines to swap information in the XCOFF .loader section.  If we
   ever need to write an XCOFF loader, this stuff will need to be
   moved to another file shared by the linker (which XCOFF calls the
   ``binder'') and the loader.  */

/* Swap in the ldhdr structure.  */

static void
xcoff_swap_ldhdr_in (abfd, src, dst)
     bfd *abfd;
     const struct external_ldhdr *src;
     struct internal_ldhdr *dst;
{
  dst->l_version = bfd_get_32 (abfd, src->l_version);
  dst->l_nsyms = bfd_get_32 (abfd, src->l_nsyms);
  dst->l_nreloc = bfd_get_32 (abfd, src->l_nreloc);
  dst->l_istlen = bfd_get_32 (abfd, src->l_istlen);
  dst->l_nimpid = bfd_get_32 (abfd, src->l_nimpid);
  dst->l_impoff = bfd_get_32 (abfd, src->l_impoff);
  dst->l_stlen = bfd_get_32 (abfd, src->l_stlen);
  dst->l_stoff = bfd_get_32 (abfd, src->l_stoff);
}

/* Swap out the ldhdr structure.  */

static void
xcoff_swap_ldhdr_out (abfd, src, dst)
     bfd *abfd;
     const struct internal_ldhdr *src;
     struct external_ldhdr *dst;
{
  bfd_put_32 (abfd, src->l_version, dst->l_version);
  bfd_put_32 (abfd, src->l_nsyms, dst->l_nsyms);
  bfd_put_32 (abfd, src->l_nreloc, dst->l_nreloc);
  bfd_put_32 (abfd, src->l_istlen, dst->l_istlen);
  bfd_put_32 (abfd, src->l_nimpid, dst->l_nimpid);
  bfd_put_32 (abfd, src->l_impoff, dst->l_impoff);
  bfd_put_32 (abfd, src->l_stlen, dst->l_stlen);
  bfd_put_32 (abfd, src->l_stoff, dst->l_stoff);
}

/* Swap in the ldsym structure.  */

static void
xcoff_swap_ldsym_in (abfd, src, dst)
     bfd *abfd;
     const struct external_ldsym *src;
     struct internal_ldsym *dst;
{
  if (bfd_get_32 (abfd, src->_l._l_l._l_zeroes) != 0)
    memcpy (dst->_l._l_name, src->_l._l_name, SYMNMLEN);
  else
    {
      dst->_l._l_l._l_zeroes = 0;
      dst->_l._l_l._l_offset = bfd_get_32 (abfd, src->_l._l_l._l_offset);
    }
  dst->l_value = bfd_get_32 (abfd, src->l_value);
  dst->l_scnum = bfd_get_16 (abfd, src->l_scnum);
  dst->l_smtype = bfd_get_8 (abfd, src->l_smtype);
  dst->l_smclas = bfd_get_8 (abfd, src->l_smclas);
  dst->l_ifile = bfd_get_32 (abfd, src->l_ifile);
  dst->l_parm = bfd_get_32 (abfd, src->l_parm);
}

/* Swap out the ldsym structure.  */

static void
xcoff_swap_ldsym_out (abfd, src, dst)
     bfd *abfd;
     const struct internal_ldsym *src;
     struct external_ldsym *dst;
{
  if (src->_l._l_l._l_zeroes != 0)
    memcpy (dst->_l._l_name, src->_l._l_name, SYMNMLEN);
  else
    {
      bfd_put_32 (abfd, 0, dst->_l._l_l._l_zeroes);
      bfd_put_32 (abfd, src->_l._l_l._l_offset, dst->_l._l_l._l_offset);
    }
  bfd_put_32 (abfd, src->l_value, dst->l_value);
  bfd_put_16 (abfd, src->l_scnum, dst->l_scnum);
  bfd_put_8 (abfd, src->l_smtype, dst->l_smtype);
  bfd_put_8 (abfd, src->l_smclas, dst->l_smclas);
  bfd_put_32 (abfd, src->l_ifile, dst->l_ifile);
  bfd_put_32 (abfd, src->l_parm, dst->l_parm);
}

/* Swap in the ldrel structure.  */

static void
xcoff_swap_ldrel_in (abfd, src, dst)
     bfd *abfd;
     const struct external_ldrel *src;
     struct internal_ldrel *dst;
{
  dst->l_vaddr = bfd_get_32 (abfd, src->l_vaddr);
  dst->l_symndx = bfd_get_32 (abfd, src->l_symndx);
  dst->l_rtype = bfd_get_16 (abfd, src->l_rtype);
  dst->l_rsecnm = bfd_get_16 (abfd, src->l_rsecnm);
}

/* Swap out the ldrel structure.  */

static void
xcoff_swap_ldrel_out (abfd, src, dst)
     bfd *abfd;
     const struct internal_ldrel *src;
     struct external_ldrel *dst;
{
  bfd_put_32 (abfd, src->l_vaddr, dst->l_vaddr);
  bfd_put_32 (abfd, src->l_symndx, dst->l_symndx);
  bfd_put_16 (abfd, src->l_rtype, dst->l_rtype);
  bfd_put_16 (abfd, src->l_rsecnm, dst->l_rsecnm);
}

/* Routines to read XCOFF dynamic information.  This don't really
   belong here, but we already have the ldsym manipulation routines
   here.  */

/* Read the contents of a section.  */

static boolean
xcoff_get_section_contents (abfd, sec)
     bfd *abfd;
     asection *sec;
{
  if (coff_section_data (abfd, sec) == NULL)
    {
      sec->used_by_bfd = bfd_zalloc (abfd,
				     sizeof (struct coff_section_tdata));
      if (sec->used_by_bfd == NULL)
	return false;
    }

  if (coff_section_data (abfd, sec)->contents == NULL)
    {
      coff_section_data (abfd, sec)->contents = bfd_malloc (sec->_raw_size);
      if (coff_section_data (abfd, sec)->contents == NULL)
	return false;

      if (! bfd_get_section_contents (abfd, sec,
				      coff_section_data (abfd, sec)->contents,
				      (file_ptr) 0, sec->_raw_size))
	return false;
    }

  return true;
}

/* Get the size required to hold the dynamic symbols.  */

long
_bfd_xcoff_get_dynamic_symtab_upper_bound (abfd)
     bfd *abfd;
{
  asection *lsec;
  bfd_byte *contents;
  struct internal_ldhdr ldhdr;

  if ((abfd->flags & DYNAMIC) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  lsec = bfd_get_section_by_name (abfd, ".loader");
  if (lsec == NULL)
    {
      bfd_set_error (bfd_error_no_symbols);
      return -1;
    }

  if (! xcoff_get_section_contents (abfd, lsec))
    return -1;
  contents = coff_section_data (abfd, lsec)->contents;

  xcoff_swap_ldhdr_in (abfd, (struct external_ldhdr *) contents, &ldhdr);

  return (ldhdr.l_nsyms + 1) * sizeof (asymbol *);
}

/* Get the dynamic symbols.  */

long
_bfd_xcoff_canonicalize_dynamic_symtab (abfd, psyms)
     bfd *abfd;
     asymbol **psyms;
{
  asection *lsec;
  bfd_byte *contents;
  struct internal_ldhdr ldhdr;
  const char *strings;
  struct external_ldsym *elsym, *elsymend;
  coff_symbol_type *symbuf;

  if ((abfd->flags & DYNAMIC) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  lsec = bfd_get_section_by_name (abfd, ".loader");
  if (lsec == NULL)
    {
      bfd_set_error (bfd_error_no_symbols);
      return -1;
    }

  if (! xcoff_get_section_contents (abfd, lsec))
    return -1;
  contents = coff_section_data (abfd, lsec)->contents;

  coff_section_data (abfd, lsec)->keep_contents = true;

  xcoff_swap_ldhdr_in (abfd, (struct external_ldhdr *) contents, &ldhdr);

  strings = (char *) contents + ldhdr.l_stoff;

  symbuf = ((coff_symbol_type *)
	    bfd_zalloc (abfd, ldhdr.l_nsyms * sizeof (coff_symbol_type)));
  if (symbuf == NULL)
    return -1;

  elsym = (struct external_ldsym *) (contents + LDHDRSZ);
  elsymend = elsym + ldhdr.l_nsyms;
  for (; elsym < elsymend; elsym++, symbuf++, psyms++)
    {
      struct internal_ldsym ldsym;

      xcoff_swap_ldsym_in (abfd, elsym, &ldsym);

      symbuf->symbol.the_bfd = abfd;

      if (ldsym._l._l_l._l_zeroes == 0)
	symbuf->symbol.name = strings + ldsym._l._l_l._l_offset;
      else
	{
	  int i;

	  for (i = 0; i < SYMNMLEN; i++)
	    if (ldsym._l._l_name[i] == '\0')
	      break;
	  if (i < SYMNMLEN)
	    symbuf->symbol.name = elsym->_l._l_name;
	  else
	    {
	      char *c;

	      c = bfd_alloc (abfd, SYMNMLEN + 1);
	      if (c == NULL)
		return -1;
	      memcpy (c, ldsym._l._l_name, SYMNMLEN);
	      c[SYMNMLEN] = '\0';
	      symbuf->symbol.name = c;
	    }
	}

      if (ldsym.l_smclas == XMC_XO)
	symbuf->symbol.section = bfd_abs_section_ptr;
      else
	symbuf->symbol.section = coff_section_from_bfd_index (abfd,
							      ldsym.l_scnum);
      symbuf->symbol.value = ldsym.l_value - symbuf->symbol.section->vma;

      symbuf->symbol.flags = BSF_NO_FLAGS;
      if ((ldsym.l_smtype & L_EXPORT) != 0)
	symbuf->symbol.flags |= BSF_GLOBAL;

      /* FIXME: We have no way to record the other information stored
         with the loader symbol.  */

      *psyms = (asymbol *) symbuf;
    }

  *psyms = NULL;

  return ldhdr.l_nsyms;
}

/* Get the size required to hold the dynamic relocs.  */

long
_bfd_xcoff_get_dynamic_reloc_upper_bound (abfd)
     bfd *abfd;
{
  asection *lsec;
  bfd_byte *contents;
  struct internal_ldhdr ldhdr;

  if ((abfd->flags & DYNAMIC) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  lsec = bfd_get_section_by_name (abfd, ".loader");
  if (lsec == NULL)
    {
      bfd_set_error (bfd_error_no_symbols);
      return -1;
    }

  if (! xcoff_get_section_contents (abfd, lsec))
    return -1;
  contents = coff_section_data (abfd, lsec)->contents;

  xcoff_swap_ldhdr_in (abfd, (struct external_ldhdr *) contents, &ldhdr);

  return (ldhdr.l_nreloc + 1) * sizeof (arelent *);
}

/* The typical dynamic reloc.  */

static reloc_howto_type xcoff_dynamic_reloc =
  HOWTO (0,	                /* type */                                 
	 0,	                /* rightshift */                           
	 2,	                /* size (0 = byte, 1 = short, 2 = long) */ 
	 32,	                /* bitsize */                   
	 false,	                /* pc_relative */                          
	 0,	                /* bitpos */                               
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,		        /* special_function */                     
	 "R_POS",               /* name */                                 
	 true,	                /* partial_inplace */                      
	 0xffffffff,            /* src_mask */                             
	 0xffffffff,            /* dst_mask */                             
	 false);                /* pcrel_offset */

/* Get the dynamic relocs.  */

long
_bfd_xcoff_canonicalize_dynamic_reloc (abfd, prelocs, syms)
     bfd *abfd;
     arelent **prelocs;
     asymbol **syms;
{
  asection *lsec;
  bfd_byte *contents;
  struct internal_ldhdr ldhdr;
  arelent *relbuf;
  struct external_ldrel *elrel, *elrelend;

  if ((abfd->flags & DYNAMIC) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  lsec = bfd_get_section_by_name (abfd, ".loader");
  if (lsec == NULL)
    {
      bfd_set_error (bfd_error_no_symbols);
      return -1;
    }

  if (! xcoff_get_section_contents (abfd, lsec))
    return -1;
  contents = coff_section_data (abfd, lsec)->contents;

  xcoff_swap_ldhdr_in (abfd, (struct external_ldhdr *) contents, &ldhdr);

  relbuf = (arelent *) bfd_alloc (abfd, ldhdr.l_nreloc * sizeof (arelent));
  if (relbuf == NULL)
    return -1;

  elrel = ((struct external_ldrel *)
	   (contents + LDHDRSZ + ldhdr.l_nsyms * LDSYMSZ));
  elrelend = elrel + ldhdr.l_nreloc;
  for (; elrel < elrelend; elrel++, relbuf++, prelocs++)
    {
      struct internal_ldrel ldrel;

      xcoff_swap_ldrel_in (abfd, elrel, &ldrel);

      if (ldrel.l_symndx >= 3)
	relbuf->sym_ptr_ptr = syms + (ldrel.l_symndx - 3);
      else
	{
	  const char *name;
	  asection *sec;

	  switch (ldrel.l_symndx)
	    {
	    case 0:
	      name = ".text";
	      break;
	    case 1:
	      name = ".data";
	      break;
	    case 2:
	      name = ".bss";
	      break;
	    default:
	      abort ();
	      break;
	    }

	  sec = bfd_get_section_by_name (abfd, name);
	  if (sec == NULL)
	    {
	      bfd_set_error (bfd_error_bad_value);
	      return -1;
	    }

	  relbuf->sym_ptr_ptr = sec->symbol_ptr_ptr;
	}

      relbuf->address = ldrel.l_vaddr;
      relbuf->addend = 0;

      /* Most dynamic relocs have the same type.  FIXME: This is only
         correct if ldrel.l_rtype == 0.  In other cases, we should use
         a different howto.  */
      relbuf->howto = &xcoff_dynamic_reloc;

      /* FIXME: We have no way to record the l_rsecnm field.  */

      *prelocs = relbuf;
    }

  *prelocs = NULL;

  return ldhdr.l_nreloc;
}

/* Routine to create an entry in an XCOFF link hash table.  */

static struct bfd_hash_entry *
xcoff_link_hash_newfunc (entry, table, string)
     struct bfd_hash_entry *entry;
     struct bfd_hash_table *table;
     const char *string;
{
  struct xcoff_link_hash_entry *ret = (struct xcoff_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == (struct xcoff_link_hash_entry *) NULL)
    ret = ((struct xcoff_link_hash_entry *)
	   bfd_hash_allocate (table, sizeof (struct xcoff_link_hash_entry)));
  if (ret == (struct xcoff_link_hash_entry *) NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = ((struct xcoff_link_hash_entry *)
	 _bfd_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				 table, string));
  if (ret != NULL)
    {
      /* Set local fields.  */
      ret->indx = -1;
      ret->toc_section = NULL;
      ret->u.toc_indx = -1;
      ret->descriptor = NULL;
      ret->ldsym = NULL;
      ret->ldindx = -1;
      ret->flags = 0;
      ret->smclas = XMC_UA;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Create a XCOFF link hash table.  */

struct bfd_link_hash_table *
_bfd_xcoff_bfd_link_hash_table_create (abfd)
     bfd *abfd;
{
  struct xcoff_link_hash_table *ret;

  ret = ((struct xcoff_link_hash_table *)
	 bfd_alloc (abfd, sizeof (struct xcoff_link_hash_table)));
  if (ret == (struct xcoff_link_hash_table *) NULL)
    return (struct bfd_link_hash_table *) NULL;
  if (! _bfd_link_hash_table_init (&ret->root, abfd, xcoff_link_hash_newfunc))
    {
      bfd_release (abfd, ret);
      return (struct bfd_link_hash_table *) NULL;
    }

  ret->debug_strtab = _bfd_xcoff_stringtab_init ();
  ret->debug_section = NULL;
  ret->loader_section = NULL;
  ret->ldrel_count = 0;
  memset (&ret->ldhdr, 0, sizeof (struct internal_ldhdr));
  ret->linkage_section = NULL;
  ret->toc_section = NULL;
  ret->descriptor_section = NULL;
  ret->imports = NULL;
  ret->file_align = 0;
  ret->textro = false;
  ret->gc = false;
  memset (ret->special_sections, 0, sizeof ret->special_sections);

  /* The linker will always generate a full a.out header.  We need to
     record that fact now, before the sizeof_headers routine could be
     called.  */
  xcoff_data (abfd)->full_aouthdr = true;

  return &ret->root;
}

/* Look up an entry in an XCOFF link hash table.  */

#define xcoff_link_hash_lookup(table, string, create, copy, follow) \
  ((struct xcoff_link_hash_entry *) \
   bfd_link_hash_lookup (&(table)->root, (string), (create), (copy),\
			 (follow)))

/* Traverse an XCOFF link hash table.  */

#define xcoff_link_hash_traverse(table, func, info)			\
  (bfd_link_hash_traverse						\
   (&(table)->root,							\
    (boolean (*) PARAMS ((struct bfd_link_hash_entry *, PTR))) (func),	\
    (info)))

/* Get the XCOFF link hash table from the info structure.  This is
   just a cast.  */

#define xcoff_hash_table(p) ((struct xcoff_link_hash_table *) ((p)->hash))

/* Read internal relocs for an XCOFF csect.  This is a wrapper around
   _bfd_coff_read_internal_relocs which tries to take advantage of any
   relocs which may have been cached for the enclosing section.  */

static struct internal_reloc *
xcoff_read_internal_relocs (abfd, sec, cache, external_relocs,
			    require_internal, internal_relocs)
     bfd *abfd;
     asection *sec;
     boolean cache;
     bfd_byte *external_relocs;
     boolean require_internal;
     struct internal_reloc *internal_relocs;
{
  if (coff_section_data (abfd, sec) != NULL
      && coff_section_data (abfd, sec)->relocs == NULL
      && xcoff_section_data (abfd, sec) != NULL)
    {
      asection *enclosing;

      enclosing = xcoff_section_data (abfd, sec)->enclosing;

      if (enclosing != NULL
	  && (coff_section_data (abfd, enclosing) == NULL
	      || coff_section_data (abfd, enclosing)->relocs == NULL)
	  && cache
	  && enclosing->reloc_count > 0)
	{
	  if (_bfd_coff_read_internal_relocs (abfd, enclosing, true,
					      external_relocs, false,
					      (struct internal_reloc *) NULL)
	      == NULL)
	    return NULL;
	}

      if (enclosing != NULL
	  && coff_section_data (abfd, enclosing) != NULL
	  && coff_section_data (abfd, enclosing)->relocs != NULL)
	{
	  size_t off;

	  off = ((sec->rel_filepos - enclosing->rel_filepos)
		 / bfd_coff_relsz (abfd));
	  if (! require_internal)
	    return coff_section_data (abfd, enclosing)->relocs + off;
	  memcpy (internal_relocs,
		  coff_section_data (abfd, enclosing)->relocs + off,
		  sec->reloc_count * sizeof (struct internal_reloc));
	  return internal_relocs;
	}
    }

  return _bfd_coff_read_internal_relocs (abfd, sec, cache, external_relocs,
					 require_internal, internal_relocs);
}

/* Given an XCOFF BFD, add symbols to the global hash table as
   appropriate.  */

boolean
_bfd_xcoff_bfd_link_add_symbols (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  switch (bfd_get_format (abfd))
    {
    case bfd_object:
      return xcoff_link_add_object_symbols (abfd, info);
    case bfd_archive:
      /* We need to look through the archive for stripped dynamic
         objects, because they will not appear in the archive map even
         though they should, perhaps, be included.  Also, if the
         linker has no map, we just consider each object file in turn,
         since that apparently is what the AIX native linker does.  */
      {
	bfd *member;

	member = bfd_openr_next_archived_file (abfd, (bfd *) NULL);
	while (member != NULL)
	  {
	    if (bfd_check_format (member, bfd_object)
		&& (! bfd_has_map (abfd)
		    || ((member->flags & DYNAMIC) != 0
			&& (member->flags & HAS_SYMS) == 0)))
	      {
		boolean needed;

		if (! xcoff_link_check_archive_element (member, info, &needed))
		  return false;
		if (needed)
		  member->archive_pass = -1;
	      }
	    member = bfd_openr_next_archived_file (abfd, member);
	  }

	if (! bfd_has_map (abfd))
	  return true;

	/* Now do the usual search.  */
	return (_bfd_generic_link_add_archive_symbols
		(abfd, info, xcoff_link_check_archive_element));
      }

    default:
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }
}

/* Add symbols from an XCOFF object file.  */

static boolean
xcoff_link_add_object_symbols (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  if (! _bfd_coff_get_external_symbols (abfd))
    return false;
  if (! xcoff_link_add_symbols (abfd, info))
    return false;
  if (! info->keep_memory)
    {
      if (! _bfd_coff_free_symbols (abfd))
	return false;
    }
  return true;
}

/* Check a single archive element to see if we need to include it in
   the link.  *PNEEDED is set according to whether this element is
   needed in the link or not.  This is called via
   _bfd_generic_link_add_archive_symbols.  */

static boolean
xcoff_link_check_archive_element (abfd, info, pneeded)
     bfd *abfd;
     struct bfd_link_info *info;
     boolean *pneeded;
{
  if (! _bfd_coff_get_external_symbols (abfd))
    return false;

  if (! xcoff_link_check_ar_symbols (abfd, info, pneeded))
    return false;

  if (*pneeded)
    {
      if (! xcoff_link_add_symbols (abfd, info))
	return false;
    }

  if (! info->keep_memory || ! *pneeded)
    {
      if (! _bfd_coff_free_symbols (abfd))
	return false;
    }

  return true;
}

/* Look through the symbols to see if this object file should be
   included in the link.  */

static boolean
xcoff_link_check_ar_symbols (abfd, info, pneeded)
     bfd *abfd;
     struct bfd_link_info *info;
     boolean *pneeded;
{
  bfd_size_type symesz;
  bfd_byte *esym;
  bfd_byte *esym_end;

  *pneeded = false;

  if ((abfd->flags & DYNAMIC) != 0
      && ! info->static_link
      && info->hash->creator == abfd->xvec)
    return xcoff_link_check_dynamic_ar_symbols (abfd, info, pneeded);

  symesz = bfd_coff_symesz (abfd);
  esym = (bfd_byte *) obj_coff_external_syms (abfd);
  esym_end = esym + obj_raw_syment_count (abfd) * symesz;
  while (esym < esym_end)
    {
      struct internal_syment sym;

      bfd_coff_swap_sym_in (abfd, (PTR) esym, (PTR) &sym);

      if (sym.n_sclass == C_EXT && sym.n_scnum != N_UNDEF)
	{
	  const char *name;
	  char buf[SYMNMLEN + 1];
	  struct bfd_link_hash_entry *h;

	  /* This symbol is externally visible, and is defined by this
             object file.  */

	  name = _bfd_coff_internal_syment_name (abfd, &sym, buf);
	  if (name == NULL)
	    return false;
	  h = bfd_link_hash_lookup (info->hash, name, false, false, true);

	  /* We are only interested in symbols that are currently
	     undefined.  If a symbol is currently known to be common,
	     XCOFF linkers do not bring in an object file which
	     defines it.  We also don't bring in symbols to satisfy
	     undefined references in shared objects.  */
	  if (h != (struct bfd_link_hash_entry *) NULL
	      && h->type == bfd_link_hash_undefined
	      && (info->hash->creator != abfd->xvec
		  || (((struct xcoff_link_hash_entry *) h)->flags
		      & XCOFF_DEF_DYNAMIC) == 0))
	    {
	      if (! (*info->callbacks->add_archive_element) (info, abfd, name))
		return false;
	      *pneeded = true;
	      return true;
	    }
	}

      esym += (sym.n_numaux + 1) * symesz;
    }

  /* We do not need this object file.  */
  return true;
}

/* Look through the loader symbols to see if this dynamic object
   should be included in the link.  The native linker uses the loader
   symbols, not the normal symbol table, so we do too.  */

static boolean
xcoff_link_check_dynamic_ar_symbols (abfd, info, pneeded)
     bfd *abfd;
     struct bfd_link_info *info;
     boolean *pneeded;
{
  asection *lsec;
  bfd_byte *buf;
  struct internal_ldhdr ldhdr;
  const char *strings;
  struct external_ldsym *elsym, *elsymend;

  *pneeded = false;

  lsec = bfd_get_section_by_name (abfd, ".loader");
  if (lsec == NULL)
    {
      /* There are no symbols, so don't try to include it.  */
      return true;
    }

  if (! xcoff_get_section_contents (abfd, lsec))
    return false;
  buf = coff_section_data (abfd, lsec)->contents;

  xcoff_swap_ldhdr_in (abfd, (struct external_ldhdr *) buf, &ldhdr);

  strings = (char *) buf + ldhdr.l_stoff;

  elsym = (struct external_ldsym *) (buf + LDHDRSZ);
  elsymend = elsym + ldhdr.l_nsyms;
  for (; elsym < elsymend; elsym++)
    {
      struct internal_ldsym ldsym;
      char nambuf[SYMNMLEN + 1];
      const char *name;
      struct bfd_link_hash_entry *h;

      xcoff_swap_ldsym_in (abfd, elsym, &ldsym);

      /* We are only interested in exported symbols.  */
      if ((ldsym.l_smtype & L_EXPORT) == 0)
	continue;

      if (ldsym._l._l_l._l_zeroes == 0)
	name = strings + ldsym._l._l_l._l_offset;
      else
	{
	  memcpy (nambuf, ldsym._l._l_name, SYMNMLEN);
	  nambuf[SYMNMLEN] = '\0';
	  name = nambuf;
	}

      h = bfd_link_hash_lookup (info->hash, name, false, false, true);

      /* We are only interested in symbols that are currently
         undefined.  At this point we know that we are using an XCOFF
         hash table.  */
      if (h != NULL
	  && h->type == bfd_link_hash_undefined
	  && (((struct xcoff_link_hash_entry *) h)->flags
	      & XCOFF_DEF_DYNAMIC) == 0)
	{
	  if (! (*info->callbacks->add_archive_element) (info, abfd, name))
	    return false;
	  *pneeded = true;
	  return true;
	}
    }

  /* We do not need this shared object.  */

  if (buf != NULL && ! coff_section_data (abfd, lsec)->keep_contents)
    {
      free (coff_section_data (abfd, lsec)->contents);
      coff_section_data (abfd, lsec)->contents = NULL;
    }

  return true;
}

/* Returns the index of reloc in RELOCS with the least address greater
   than or equal to ADDRESS.  The relocs are sorted by address.  */

static bfd_size_type
xcoff_find_reloc (relocs, count, address)
     struct internal_reloc *relocs;
     bfd_size_type count;
     bfd_vma address;
{
  bfd_size_type min, max, this;

  if (count < 2)
    {
      if (count == 1 && relocs[0].r_vaddr < address)
	return 1;
      else
	return 0;
    }

  min = 0;
  max = count;

  /* Do a binary search over (min,max].  */
  while (min + 1 < max)
    {
      bfd_vma raddr;

      this = (max + min) / 2;
      raddr = relocs[this].r_vaddr;
      if (raddr > address)
	max = this;
      else if (raddr < address)
	min = this;
      else
	{
	  min = this;
	  break;
	}
    }

  if (relocs[min].r_vaddr < address)
    return min + 1;

  while (min > 0
	 && relocs[min - 1].r_vaddr == address)
    --min;

  return min;
}

/* Add all the symbols from an object file to the hash table.

   XCOFF is a weird format.  A normal XCOFF .o files will have three
   COFF sections--.text, .data, and .bss--but each COFF section will
   contain many csects.  These csects are described in the symbol
   table.  From the linker's point of view, each csect must be
   considered a section in its own right.  For example, a TOC entry is
   handled as a small XMC_TC csect.  The linker must be able to merge
   different TOC entries together, which means that it must be able to
   extract the XMC_TC csects from the .data section of the input .o
   file.

   From the point of view of our linker, this is, of course, a hideous
   nightmare.  We cope by actually creating sections for each csect,
   and discarding the original sections.  We then have to handle the
   relocation entries carefully, since the only way to tell which
   csect they belong to is to examine the address.  */

static boolean
xcoff_link_add_symbols (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  unsigned int n_tmask;
  unsigned int n_btshft;
  boolean default_copy;
  bfd_size_type symcount;
  struct xcoff_link_hash_entry **sym_hash;
  asection **csect_cache;
  bfd_size_type linesz;
  asection *o;
  asection *last_real;
  boolean keep_syms;
  asection *csect;
  unsigned int csect_index;
  asection *first_csect;
  bfd_size_type symesz;
  bfd_byte *esym;
  bfd_byte *esym_end;
  struct reloc_info_struct
    {
      struct internal_reloc *relocs;
      asection **csects;
      bfd_byte *linenos;
    } *reloc_info = NULL;

  if ((abfd->flags & DYNAMIC) != 0
      && ! info->static_link)
    {
      if (! xcoff_link_add_dynamic_symbols (abfd, info))
	return false;
    }

  if (info->hash->creator == abfd->xvec)
    {
      /* We need to build a .loader section, so we do it here.  This
	 won't work if we're producing an XCOFF output file with no
	 XCOFF input files.  FIXME.  */
      if (xcoff_hash_table (info)->loader_section == NULL)
	{
	  asection *lsec;

	  lsec = bfd_make_section_anyway (abfd, ".loader");
	  if (lsec == NULL)
	    goto error_return;
	  xcoff_hash_table (info)->loader_section = lsec;
	  lsec->flags |= SEC_HAS_CONTENTS | SEC_IN_MEMORY;
	}
      /* Likewise for the linkage section.  */
      if (xcoff_hash_table (info)->linkage_section == NULL)
	{
	  asection *lsec;

	  lsec = bfd_make_section_anyway (abfd, ".gl");
	  if (lsec == NULL)
	    goto error_return;
	  xcoff_hash_table (info)->linkage_section = lsec;
	  lsec->flags |= (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS
			  | SEC_IN_MEMORY);
	  lsec->alignment_power = 2;
	}
      /* Likewise for the TOC section.  */
      if (xcoff_hash_table (info)->toc_section == NULL)
	{
	  asection *tsec;

	  tsec = bfd_make_section_anyway (abfd, ".tc");
	  if (tsec == NULL)
	    goto error_return;
	  xcoff_hash_table (info)->toc_section = tsec;
	  tsec->flags |= (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS
			  | SEC_IN_MEMORY);
	  tsec->alignment_power = 2;
	}
      /* Likewise for the descriptor section.  */
      if (xcoff_hash_table (info)->descriptor_section == NULL)
	{
	  asection *dsec;

	  dsec = bfd_make_section_anyway (abfd, ".ds");
	  if (dsec == NULL)
	    goto error_return;
	  xcoff_hash_table (info)->descriptor_section = dsec;
	  dsec->flags |= (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS
			  | SEC_IN_MEMORY);
	  dsec->alignment_power = 2;
	}
      /* Likewise for the .debug section.  */
      if (xcoff_hash_table (info)->debug_section == NULL)
	{
	  asection *dsec;

	  dsec = bfd_make_section_anyway (abfd, ".debug");
	  if (dsec == NULL)
	    goto error_return;
	  xcoff_hash_table (info)->debug_section = dsec;
	  dsec->flags |= SEC_HAS_CONTENTS | SEC_IN_MEMORY;
	}
    }

  if ((abfd->flags & DYNAMIC) != 0
      && ! info->static_link)
    return true;

  n_tmask = coff_data (abfd)->local_n_tmask;
  n_btshft = coff_data (abfd)->local_n_btshft;

  /* Define macros so that ISFCN, et. al., macros work correctly.  */
#define N_TMASK n_tmask
#define N_BTSHFT n_btshft

  if (info->keep_memory)
    default_copy = false;
  else
    default_copy = true;

  symcount = obj_raw_syment_count (abfd);

  /* We keep a list of the linker hash table entries that correspond
     to each external symbol.  */
  sym_hash = ((struct xcoff_link_hash_entry **)
	      bfd_alloc (abfd,
			 (symcount
			  * sizeof (struct xcoff_link_hash_entry *))));
  if (sym_hash == NULL && symcount != 0)
    goto error_return;
  coff_data (abfd)->sym_hashes = (struct coff_link_hash_entry **) sym_hash;
  memset (sym_hash, 0,
	  (size_t) symcount * sizeof (struct xcoff_link_hash_entry *));

  /* Because of the weird stuff we are doing with XCOFF csects, we can
     not easily determine which section a symbol is in, so we store
     the information in the tdata for the input file.  */
  csect_cache = ((asection **)
		 bfd_alloc (abfd, symcount * sizeof (asection *)));
  if (csect_cache == NULL && symcount != 0)
    goto error_return;
  xcoff_data (abfd)->csects = csect_cache;
  memset (csect_cache, 0, (size_t) symcount * sizeof (asection *));

  /* While splitting sections into csects, we need to assign the
     relocs correctly.  The relocs and the csects must both be in
     order by VMA within a given section, so we handle this by
     scanning along the relocs as we process the csects.  We index
     into reloc_info using the section target_index.  */
  reloc_info = ((struct reloc_info_struct *)
		bfd_malloc ((abfd->section_count + 1)
			    * sizeof (struct reloc_info_struct)));
  if (reloc_info == NULL)
    goto error_return;
  memset ((PTR) reloc_info, 0,
	  (abfd->section_count + 1) * sizeof (struct reloc_info_struct));

  /* Read in the relocs and line numbers for each section.  */
  linesz = bfd_coff_linesz (abfd);
  last_real = NULL;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      last_real = o;
      if ((o->flags & SEC_RELOC) != 0)
	{
	  reloc_info[o->target_index].relocs =
	    xcoff_read_internal_relocs (abfd, o, true, (bfd_byte *) NULL,
					false, (struct internal_reloc *) NULL);
	  reloc_info[o->target_index].csects =
	    (asection **) bfd_malloc (o->reloc_count * sizeof (asection *));
	  if (reloc_info[o->target_index].csects == NULL)
	    goto error_return;
	  memset (reloc_info[o->target_index].csects, 0,
		  o->reloc_count * sizeof (asection *));
	}

      if ((info->strip == strip_none || info->strip == strip_some)
	  && o->lineno_count > 0)
	{
	  bfd_byte *linenos;

	  linenos = (bfd_byte *) bfd_malloc (o->lineno_count * linesz);
	  if (linenos == NULL)
	    goto error_return;
	  reloc_info[o->target_index].linenos = linenos;
	  if (bfd_seek (abfd, o->line_filepos, SEEK_SET) != 0
	      || (bfd_read (linenos, linesz, o->lineno_count, abfd)
		  != linesz * o->lineno_count))
	    goto error_return;
	}
    }

  /* Don't let the linker relocation routines discard the symbols.  */
  keep_syms = obj_coff_keep_syms (abfd);
  obj_coff_keep_syms (abfd) = true;

  csect = NULL;
  csect_index = 0;
  first_csect = NULL;

  symesz = bfd_coff_symesz (abfd);
  BFD_ASSERT (symesz == bfd_coff_auxesz (abfd));
  esym = (bfd_byte *) obj_coff_external_syms (abfd);
  esym_end = esym + symcount * symesz;
  while (esym < esym_end)
    {
      struct internal_syment sym;
      union internal_auxent aux;
      const char *name;
      char buf[SYMNMLEN + 1];
      int smtyp;
      flagword flags;
      asection *section;
      bfd_vma value;
      struct xcoff_link_hash_entry *set_toc;

      bfd_coff_swap_sym_in (abfd, (PTR) esym, (PTR) &sym);

      /* In this pass we are only interested in symbols with csect
         information.  */
      if (sym.n_sclass != C_EXT && sym.n_sclass != C_HIDEXT)
	{
	  if (sym.n_sclass == C_FILE && csect != NULL)
	    {
	      xcoff_section_data (abfd, csect)->last_symndx =
		((esym
		  - (bfd_byte *) obj_coff_external_syms (abfd))
		 / symesz);
	      csect = NULL;
	    }

	  if (csect != NULL)
	    *csect_cache = csect;
	  else if (first_csect == NULL || sym.n_sclass == C_FILE)
	    *csect_cache = coff_section_from_bfd_index (abfd, sym.n_scnum);
	  else
	    *csect_cache = NULL;
	  esym += (sym.n_numaux + 1) * symesz;
	  sym_hash += sym.n_numaux + 1;
	  csect_cache += sym.n_numaux + 1;
	  continue;
	}

      name = _bfd_coff_internal_syment_name (abfd, &sym, buf);
      if (name == NULL)
	goto error_return;

      /* If this symbol has line number information attached to it,
         and we're not stripping it, count the number of entries and
         add them to the count for this csect.  In the final link pass
         we are going to attach line number information by symbol,
         rather than by section, in order to more easily handle
         garbage collection.  */
      if ((info->strip == strip_none || info->strip == strip_some)
	  && sym.n_numaux > 1
	  && csect != NULL
	  && ISFCN (sym.n_type))
	{
	  union internal_auxent auxlin;

	  bfd_coff_swap_aux_in (abfd, (PTR) (esym + symesz),
				sym.n_type, sym.n_sclass,
				0, sym.n_numaux, (PTR) &auxlin);
	  if (auxlin.x_sym.x_fcnary.x_fcn.x_lnnoptr != 0)
	    {
	      asection *enclosing;
	      bfd_size_type linoff;

	      enclosing = xcoff_section_data (abfd, csect)->enclosing;
	      if (enclosing == NULL)
		{
		  (*_bfd_error_handler)
		    ("%s: `%s' has line numbers but no enclosing section",
		     bfd_get_filename (abfd), name);
		  bfd_set_error (bfd_error_bad_value);
		  goto error_return;
		}
	      linoff = (auxlin.x_sym.x_fcnary.x_fcn.x_lnnoptr
			- enclosing->line_filepos);
	      if (linoff < enclosing->lineno_count * linesz)
		{
		  struct internal_lineno lin;
		  bfd_byte *linpstart;

		  linpstart = (reloc_info[enclosing->target_index].linenos
			       + linoff);
		  bfd_coff_swap_lineno_in (abfd, (PTR) linpstart, (PTR) &lin);
		  if (lin.l_lnno == 0
		      && ((bfd_size_type) lin.l_addr.l_symndx
			  == ((esym
			       - (bfd_byte *) obj_coff_external_syms (abfd))
			      / symesz)))
		    {
		      bfd_byte *linpend, *linp;

		      linpend = (reloc_info[enclosing->target_index].linenos
				 + enclosing->lineno_count * linesz);
		      for (linp = linpstart + linesz;
			   linp < linpend;
			   linp += linesz)
			{
			  bfd_coff_swap_lineno_in (abfd, (PTR) linp,
						   (PTR) &lin);
			  if (lin.l_lnno == 0)
			    break;
			}
		      csect->lineno_count += (linp - linpstart) / linesz;
		      /* The setting of line_filepos will only be
                         useful if all the line number entries for a
                         csect are contiguous; this only matters for
                         error reporting.  */
		      if (csect->line_filepos == 0)
			csect->line_filepos =
			  auxlin.x_sym.x_fcnary.x_fcn.x_lnnoptr;
		    }
		}
	    }
	}

      /* Pick up the csect auxiliary information.  */

      if (sym.n_numaux == 0)
	{
	  (*_bfd_error_handler)
	    ("%s: class %d symbol `%s' has no aux entries",
	     bfd_get_filename (abfd), sym.n_sclass, name);
	  bfd_set_error (bfd_error_bad_value);
	  goto error_return;
	}

      bfd_coff_swap_aux_in (abfd,
			    (PTR) (esym + symesz * sym.n_numaux),
			    sym.n_type, sym.n_sclass,
			    sym.n_numaux - 1, sym.n_numaux,
			    (PTR) &aux);

      smtyp = SMTYP_SMTYP (aux.x_csect.x_smtyp);

      flags = BSF_GLOBAL;
      section = NULL;
      value = 0;
      set_toc = NULL;

      switch (smtyp)
	{
	default:
	  (*_bfd_error_handler)
	    ("%s: symbol `%s' has unrecognized csect type %d",
	     bfd_get_filename (abfd), name, smtyp);
	  bfd_set_error (bfd_error_bad_value);
	  goto error_return;

	case XTY_ER:
	  /* This is an external reference.  */
	  if (sym.n_sclass == C_HIDEXT
	      || sym.n_scnum != N_UNDEF
	      || aux.x_csect.x_scnlen.l != 0)
	    {
	      (*_bfd_error_handler)
		("%s: bad XTY_ER symbol `%s': class %d scnum %d scnlen %d",
		 bfd_get_filename (abfd), name, sym.n_sclass, sym.n_scnum,
		 aux.x_csect.x_scnlen.l);
	      bfd_set_error (bfd_error_bad_value);
	      goto error_return;
	    }

	  /* An XMC_XO external reference is actually a reference to
             an absolute location.  */
	  if (aux.x_csect.x_smclas != XMC_XO)
	    section = bfd_und_section_ptr;
	  else
	    {
	      section = bfd_abs_section_ptr;
	      value = sym.n_value;
	    }
	  break;

	case XTY_SD:
	  /* This is a csect definition.  */

	  if (csect != NULL)
	    {
	      xcoff_section_data (abfd, csect)->last_symndx =
		((esym
		  - (bfd_byte *) obj_coff_external_syms (abfd))
		 / symesz);
	    }

	  csect = NULL;
	  csect_index = -1;

	  /* When we see a TOC anchor, we record the TOC value.  */
	  if (aux.x_csect.x_smclas == XMC_TC0)
	    {
	      if (sym.n_sclass != C_HIDEXT
		  || aux.x_csect.x_scnlen.l != 0)
		{
		  (*_bfd_error_handler)
		    ("%s: XMC_TC0 symbol `%s' is class %d scnlen %d",
		     bfd_get_filename (abfd), name, sym.n_sclass,
		     aux.x_csect.x_scnlen.l);
		  bfd_set_error (bfd_error_bad_value);
		  goto error_return;
		}
	      xcoff_data (abfd)->toc = sym.n_value;
	    }

	  /* We must merge TOC entries for the same symbol.  We can
	     merge two TOC entries if they are both C_HIDEXT, they
	     both have the same name, they are both 4 bytes long, and
	     they both have a relocation table entry for an external
	     symbol with the same name.  Unfortunately, this means
	     that we must look through the relocations.  Ick.  */
	  if (aux.x_csect.x_smclas == XMC_TC
	      && sym.n_sclass == C_HIDEXT
	      && aux.x_csect.x_scnlen.l == 4
	      && info->hash->creator == abfd->xvec)
	    {
	      asection *enclosing;
	      struct internal_reloc *relocs;
	      bfd_size_type relindx;
	      struct internal_reloc *rel;

	      enclosing = coff_section_from_bfd_index (abfd, sym.n_scnum);
	      if (enclosing == NULL)
		goto error_return;

	      relocs = reloc_info[enclosing->target_index].relocs;
	      relindx = xcoff_find_reloc (relocs, enclosing->reloc_count,
					  sym.n_value);
	      rel = relocs + relindx;
	      if (relindx < enclosing->reloc_count
		  && rel->r_vaddr == (bfd_vma) sym.n_value
		  && rel->r_size == 31
		  && rel->r_type == R_POS)
		{
		  bfd_byte *erelsym;
		  struct internal_syment relsym;

		  erelsym = ((bfd_byte *) obj_coff_external_syms (abfd)
			     + rel->r_symndx * symesz);
		  bfd_coff_swap_sym_in (abfd, (PTR) erelsym, (PTR) &relsym);
		  if (relsym.n_sclass == C_EXT)
		    {
		      const char *relname;
		      char relbuf[SYMNMLEN + 1];
		      boolean copy;
		      struct xcoff_link_hash_entry *h;

		      /* At this point we know that the TOC entry is
			 for an externally visible symbol.  */
		      relname = _bfd_coff_internal_syment_name (abfd, &relsym,
								relbuf);
		      if (relname == NULL)
			goto error_return;

		      /* We only merge TOC entries if the TC name is
                         the same as the symbol name.  This handles
                         the normal case, but not common cases like
                         SYM.P4 which gcc generates to store SYM + 4
                         in the TOC.  FIXME.  */
		      if (strcmp (name, relname) == 0)
			{
			  copy = (! info->keep_memory
				  || relsym._n._n_n._n_zeroes != 0
				  || relsym._n._n_n._n_offset == 0);
			  h = xcoff_link_hash_lookup (xcoff_hash_table (info),
						      relname, true, copy,
						      false);
			  if (h == NULL)
			    goto error_return;

			  /* At this point h->root.type could be
			     bfd_link_hash_new.  That should be OK,
			     since we know for sure that we will come
			     across this symbol as we step through the
			     file.  */

			  /* We store h in *sym_hash for the
			     convenience of the relocate_section
			     function.  */
			  *sym_hash = h;

			  if (h->toc_section != NULL)
			    {
			      asection **rel_csects;

			      /* We already have a TOC entry for this
				 symbol, so we can just ignore this
				 one.  */
			      rel_csects =
				reloc_info[enclosing->target_index].csects;
			      rel_csects[relindx] = bfd_und_section_ptr;
			      break;
			    }

			  /* We are about to create a TOC entry for
			     this symbol.  */
			  set_toc = h;
			}
		    }
		}
	    }

	  /* We need to create a new section.  We get the name from
	     the csect storage mapping class, so that the linker can
	     accumulate similar csects together.  */
	  {
	    static const char *csect_name_by_class[] =
	      {
		".pr", ".ro", ".db", ".tc", ".ua", ".rw", ".gl", ".xo",
		".sv", ".bs", ".ds", ".uc", ".ti", ".tb", NULL, ".tc0",
		".td"
	      };
	    const char *csect_name;
	    asection *enclosing;

	    if ((aux.x_csect.x_smclas >=
		 sizeof csect_name_by_class / sizeof csect_name_by_class[0])
		|| csect_name_by_class[aux.x_csect.x_smclas] == NULL)
	      {
		(*_bfd_error_handler)
		  ("%s: symbol `%s' has unrecognized smclas %d",
		   bfd_get_filename (abfd), name, aux.x_csect.x_smclas);
		bfd_set_error (bfd_error_bad_value);
		goto error_return;
	      }

	    csect_name = csect_name_by_class[aux.x_csect.x_smclas];
	    csect = bfd_make_section_anyway (abfd, csect_name);
	    if (csect == NULL)
	      goto error_return;
	    enclosing = coff_section_from_bfd_index (abfd, sym.n_scnum);
	    if (enclosing == NULL)
	      goto error_return;
	    if (! bfd_is_abs_section (enclosing)
		&& ((bfd_vma) sym.n_value < enclosing->vma
		    || ((bfd_vma) sym.n_value + aux.x_csect.x_scnlen.l
			> enclosing->vma + enclosing->_raw_size)))
	      {
		(*_bfd_error_handler)
		  ("%s: csect `%s' not in enclosing section",
		   bfd_get_filename (abfd), name);
		bfd_set_error (bfd_error_bad_value);
		goto error_return;
	      }
	    csect->vma = sym.n_value;
	    csect->filepos = (enclosing->filepos
			      + sym.n_value
			      - enclosing->vma);
	    csect->_raw_size = aux.x_csect.x_scnlen.l;
	    csect->flags |= SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS;
	    csect->alignment_power = SMTYP_ALIGN (aux.x_csect.x_smtyp);

	    /* Record the enclosing section in the tdata for this new
	       section.  */
	    csect->used_by_bfd =
	      ((struct coff_section_tdata *)
	       bfd_zalloc (abfd, sizeof (struct coff_section_tdata)));
	    if (csect->used_by_bfd == NULL)
	      goto error_return;
	    coff_section_data (abfd, csect)->tdata =
	      bfd_zalloc (abfd, sizeof (struct xcoff_section_tdata));
	    if (coff_section_data (abfd, csect)->tdata == NULL)
	      goto error_return;
	    xcoff_section_data (abfd, csect)->enclosing = enclosing;
	    xcoff_section_data (abfd, csect)->lineno_count =
	      enclosing->lineno_count;

	    if (enclosing->owner == abfd)
	      {
		struct internal_reloc *relocs;
		bfd_size_type relindx;
		struct internal_reloc *rel;
		asection **rel_csect;

		relocs = reloc_info[enclosing->target_index].relocs;
		relindx = xcoff_find_reloc (relocs, enclosing->reloc_count,
					    csect->vma);
		rel = relocs + relindx;
		rel_csect = (reloc_info[enclosing->target_index].csects
			     + relindx);
		csect->rel_filepos = (enclosing->rel_filepos
				      + relindx * bfd_coff_relsz (abfd));
		while (relindx < enclosing->reloc_count
		       && *rel_csect == NULL
		       && rel->r_vaddr < csect->vma + csect->_raw_size)
		  {
		    *rel_csect = csect;
		    csect->flags |= SEC_RELOC;
		    ++csect->reloc_count;
		    ++relindx;
		    ++rel;
		    ++rel_csect;
		  }
	      }

	    /* There are a number of other fields and section flags
	       which we do not bother to set.  */

	    csect_index = ((esym
			    - (bfd_byte *) obj_coff_external_syms (abfd))
			   / symesz);

	    xcoff_section_data (abfd, csect)->first_symndx = csect_index;

	    if (first_csect == NULL)
	      first_csect = csect;

	    /* If this symbol is C_EXT, we treat it as starting at the
	       beginning of the newly created section.  */
	    if (sym.n_sclass == C_EXT)
	      {
		section = csect;
		value = 0;
	      }

	    /* If this is a TOC section for a symbol, record it.  */
	    if (set_toc != NULL)
	      set_toc->toc_section = csect;
	  }
	  break;

	case XTY_LD:
	  /* This is a label definition.  The x_scnlen field is the
             symbol index of the csect.  I believe that this must
             always follow the appropriate XTY_SD symbol, so I will
             insist on it.  */
	  {
	    boolean bad;

	    bad = false;
	    if (aux.x_csect.x_scnlen.l < 0
		|| (aux.x_csect.x_scnlen.l
		    >= esym - (bfd_byte *) obj_coff_external_syms (abfd)))
	      bad = true;
	    if (! bad)
	      {
		section = xcoff_data (abfd)->csects[aux.x_csect.x_scnlen.l];
		if (section == NULL
		    || (section->flags & SEC_HAS_CONTENTS) == 0)
		  bad = true;
	      }
	    if (bad)
	      {
		(*_bfd_error_handler)
		  ("%s: misplaced XTY_LD `%s'",
		   bfd_get_filename (abfd), name);
		bfd_set_error (bfd_error_bad_value);
		goto error_return;
	      }

	    value = sym.n_value - csect->vma;
	  }
	  break;

	case XTY_CM:
	  /* This is an unitialized csect.  We could base the name on
             the storage mapping class, but we don't bother.  If this
             csect is externally visible, it is a common symbol.  */

	  if (csect != NULL)
	    {
	      xcoff_section_data (abfd, csect)->last_symndx =
		((esym
		  - (bfd_byte *) obj_coff_external_syms (abfd))
		 / symesz);
	    }

	  csect = bfd_make_section_anyway (abfd, ".bss");
	  if (csect == NULL)
	    goto error_return;
	  csect->vma = sym.n_value;
	  csect->_raw_size = aux.x_csect.x_scnlen.l;
	  csect->flags |= SEC_ALLOC;
	  csect->alignment_power = SMTYP_ALIGN (aux.x_csect.x_smtyp);
	  /* There are a number of other fields and section flags
	     which we do not bother to set.  */

	  csect_index = ((esym
			  - (bfd_byte *) obj_coff_external_syms (abfd))
			 / symesz);

	  csect->used_by_bfd =
	    ((struct coff_section_tdata *)
	     bfd_zalloc (abfd, sizeof (struct coff_section_tdata)));
	  if (csect->used_by_bfd == NULL)
	    goto error_return;
	  coff_section_data (abfd, csect)->tdata =
	    bfd_zalloc (abfd, sizeof (struct xcoff_section_tdata));
	  if (coff_section_data (abfd, csect)->tdata == NULL)
	    goto error_return;
	  xcoff_section_data (abfd, csect)->first_symndx = csect_index;

	  if (first_csect == NULL)
	    first_csect = csect;

	  if (sym.n_sclass == C_EXT)
	    {
	      csect->flags |= SEC_IS_COMMON;
	      csect->_raw_size = 0;
	      section = csect;
	      value = aux.x_csect.x_scnlen.l;
	    }

	  break;
	}

      /* Check for magic symbol names.  */
      if ((smtyp == XTY_SD || smtyp == XTY_CM)
	  && aux.x_csect.x_smclas != XMC_TC)
	{
	  int i;

	  i = -1;
	  if (name[0] == '_')
	    {
	      if (strcmp (name, "_text") == 0)
		i = 0;
	      else if (strcmp (name, "_etext") == 0)
		i = 1;
	      else if (strcmp (name, "_data") == 0)
		i = 2;
	      else if (strcmp (name, "_edata") == 0)
		i = 3;
	      else if (strcmp (name, "_end") == 0)
		i = 4;
	    }
	  else if (name[0] == 'e' && strcmp (name, "end") == 0)
	    i = 5;

	  if (i != -1)
	    xcoff_hash_table (info)->special_sections[i] = csect;
	}

      /* Now we have enough information to add the symbol to the
         linker hash table.  */

      if (sym.n_sclass == C_EXT)
	{
	  boolean copy;

	  BFD_ASSERT (section != NULL);

	  /* We must copy the name into memory if we got it from the
             syment itself, rather than the string table.  */
	  copy = default_copy;
	  if (sym._n._n_n._n_zeroes != 0
	      || sym._n._n_n._n_offset == 0)
	    copy = true;

	  if (info->hash->creator == abfd->xvec)
	    {
	      /* If we are statically linking a shared object, it is
                 OK for symbol redefinitions to occur.  I can't figure
                 out just what the XCOFF linker is doing, but
                 something like this is required for -bnso to work.  */
	      if (! bfd_is_und_section (section))
		*sym_hash = xcoff_link_hash_lookup (xcoff_hash_table (info),
						    name, true, copy, false);
	      else
		*sym_hash = ((struct xcoff_link_hash_entry *)
			     bfd_wrapped_link_hash_lookup (abfd, info, name,
							   true, copy, false));
	      if (*sym_hash == NULL)
		goto error_return;
	      if (((*sym_hash)->root.type == bfd_link_hash_defined
		   || (*sym_hash)->root.type == bfd_link_hash_defweak)
		  && ! bfd_is_und_section (section)
		  && ! bfd_is_com_section (section))
		{
		  /* If the existing symbol is to global linkage code,
                     and this symbol is not global linkage code, then
                     replace the existing symbol.  */
		  if ((abfd->flags & DYNAMIC) != 0
		      && ((*sym_hash)->smclas != XMC_GL
			  || aux.x_csect.x_smclas == XMC_GL
			  || ((*sym_hash)->root.u.def.section->owner->flags
			      & DYNAMIC) == 0))
		    {
		      section = bfd_und_section_ptr;
		      value = 0;
		    }
		  else if (((*sym_hash)->root.u.def.section->owner->flags
			    & DYNAMIC) != 0)
		    {
		      (*sym_hash)->root.type = bfd_link_hash_undefined;
		      (*sym_hash)->root.u.undef.abfd =
			(*sym_hash)->root.u.def.section->owner;
		    }
		}
	    }

	  /* _bfd_generic_link_add_one_symbol may call the linker to
	     generate an error message, and the linker may try to read
	     the symbol table to give a good error.  Right now, the
	     line numbers are in an inconsistent state, since they are
	     counted both in the real sections and in the new csects.
	     We need to leave the count in the real sections so that
	     the linker can report the line number of the error
	     correctly, so temporarily clobber the link to the csects
	     so that the linker will not try to read the line numbers
	     a second time from the csects.  */
	  BFD_ASSERT (last_real->next == first_csect);
	  last_real->next = NULL;
	  if (! (_bfd_generic_link_add_one_symbol
		 (info, abfd, name, flags, section, value,
		  (const char *) NULL, copy, true,
		  (struct bfd_link_hash_entry **) sym_hash)))
	    goto error_return;
	  last_real->next = first_csect;

	  if (smtyp == XTY_CM)
	    {
	      if ((*sym_hash)->root.type != bfd_link_hash_common
		  || (*sym_hash)->root.u.c.p->section != csect)
		{
		  /* We don't need the common csect we just created.  */
		  csect->_raw_size = 0;
		}
	      else
		{
		  (*sym_hash)->root.u.c.p->alignment_power
		     = csect->alignment_power;
		}
	    }

	  if (info->hash->creator == abfd->xvec)
	    {
	      int flag;

	      if (smtyp == XTY_ER || smtyp == XTY_CM)
		flag = XCOFF_REF_REGULAR;
	      else
		flag = XCOFF_DEF_REGULAR;
	      (*sym_hash)->flags |= flag;

	      if ((*sym_hash)->smclas == XMC_UA
		  || flag == XCOFF_DEF_REGULAR)
		(*sym_hash)->smclas = aux.x_csect.x_smclas;
	    }
	}

      *csect_cache = csect;

      esym += (sym.n_numaux + 1) * symesz;
      sym_hash += sym.n_numaux + 1;
      csect_cache += sym.n_numaux + 1;
    }

  BFD_ASSERT (last_real == NULL || last_real->next == first_csect);

  /* Make sure that we have seen all the relocs.  */
  for (o = abfd->sections; o != first_csect; o = o->next)
    {
      /* Reset the section size and the line number count, since the
	 data is now attached to the csects.  Don't reset the size of
	 the .debug section, since we need to read it below in
	 bfd_xcoff_size_dynamic_sections.  */
      if (strcmp (bfd_get_section_name (abfd, o), ".debug") != 0)
	o->_raw_size = 0;
      o->lineno_count = 0;

      if ((o->flags & SEC_RELOC) != 0)
	{
	  bfd_size_type i;
	  struct internal_reloc *rel;
	  asection **rel_csect;

	  rel = reloc_info[o->target_index].relocs;
	  rel_csect = reloc_info[o->target_index].csects;
	  for (i = 0; i < o->reloc_count; i++, rel++, rel_csect++)
	    {
	      if (*rel_csect == NULL)
		{
		  (*_bfd_error_handler)
		    ("%s: reloc %s:%d not in csect",
		     bfd_get_filename (abfd), o->name, i);
		  bfd_set_error (bfd_error_bad_value);
		  goto error_return;
		}

	      /* We identify all symbols which are called, so that we
		 can create glue code for calls to functions imported
		 from dynamic objects.  */
	      if (info->hash->creator == abfd->xvec
		  && *rel_csect != bfd_und_section_ptr
		  && (rel->r_type == R_BR
		      || rel->r_type == R_RBR)
		  && obj_xcoff_sym_hashes (abfd)[rel->r_symndx] != NULL)
		{
		  struct xcoff_link_hash_entry *h;

		  h = obj_xcoff_sym_hashes (abfd)[rel->r_symndx];
		  h->flags |= XCOFF_CALLED;
		  /* If the symbol name starts with a period, it is
                     the code of a function.  If the symbol is
                     currently undefined, then add an undefined symbol
                     for the function descriptor.  This should do no
                     harm, because any regular object that defines the
                     function should also define the function
                     descriptor.  It helps, because it means that we
                     will identify the function descriptor with a
                     dynamic object if a dynamic object defines it.  */
		  if (h->root.root.string[0] == '.'
		      && h->descriptor == NULL)
		    {
		      struct xcoff_link_hash_entry *hds;

		      hds = xcoff_link_hash_lookup (xcoff_hash_table (info),
						    h->root.root.string + 1,
						    true, false, true);
		      if (hds == NULL)
			goto error_return;
		      if (hds->root.type == bfd_link_hash_new)
			{
			  if (! (_bfd_generic_link_add_one_symbol
				 (info, abfd, hds->root.root.string,
				  (flagword) 0, bfd_und_section_ptr,
				  (bfd_vma) 0, (const char *) NULL, false,
				  true,
				  (struct bfd_link_hash_entry **) &hds)))
			    goto error_return;
			}
		      hds->flags |= XCOFF_DESCRIPTOR;
		      BFD_ASSERT ((hds->flags & XCOFF_CALLED) == 0
				  && (h->flags & XCOFF_DESCRIPTOR) == 0);
		      hds->descriptor = h;
		      h->descriptor = hds;
		    }
		}
	    }

	  free (reloc_info[o->target_index].csects);
	  reloc_info[o->target_index].csects = NULL;

	  /* Reset SEC_RELOC and the reloc_count, since the reloc
	     information is now attached to the csects.  */
	  o->flags &=~ SEC_RELOC;
	  o->reloc_count = 0;

	  /* If we are not keeping memory, free the reloc information.  */
	  if (! info->keep_memory
	      && coff_section_data (abfd, o) != NULL
	      && coff_section_data (abfd, o)->relocs != NULL
	      && ! coff_section_data (abfd, o)->keep_relocs)
	    {
	      free (coff_section_data (abfd, o)->relocs);
	      coff_section_data (abfd, o)->relocs = NULL;
	    }
	}

      /* Free up the line numbers.  FIXME: We could cache these
         somewhere for the final link, to avoid reading them again.  */
      if (reloc_info[o->target_index].linenos != NULL)
	{
	  free (reloc_info[o->target_index].linenos);
	  reloc_info[o->target_index].linenos = NULL;
	}
    }

  free (reloc_info);

  obj_coff_keep_syms (abfd) = keep_syms;

  return true;

 error_return:
  if (reloc_info != NULL)
    {
      for (o = abfd->sections; o != NULL; o = o->next)
	{
	  if (reloc_info[o->target_index].csects != NULL)
	    free (reloc_info[o->target_index].csects);
	  if (reloc_info[o->target_index].linenos != NULL)
	    free (reloc_info[o->target_index].linenos);
	}
    free (reloc_info);
    }
  obj_coff_keep_syms (abfd) = keep_syms;
  return false;
}

#undef N_TMASK
#undef N_BTSHFT

/* This function is used to add symbols from a dynamic object to the
   global symbol table.  */

static boolean
xcoff_link_add_dynamic_symbols (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  asection *lsec;
  bfd_byte *buf;
  struct internal_ldhdr ldhdr;
  const char *strings;
  struct external_ldsym *elsym, *elsymend;
  struct xcoff_import_file *n;
  const char *bname;
  const char *mname;
  const char *s;
  unsigned int c;
  struct xcoff_import_file **pp;

  /* We can only handle a dynamic object if we are generating an XCOFF
     output file.  */
  if (info->hash->creator != abfd->xvec)
    {
      (*_bfd_error_handler)
	("%s: XCOFF shared object when not producing XCOFF output",
	 bfd_get_filename (abfd));
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  /* The symbols we use from a dynamic object are not the symbols in
     the normal symbol table, but, rather, the symbols in the export
     table.  If there is a global symbol in a dynamic object which is
     not in the export table, the loader will not be able to find it,
     so we don't want to find it either.  Also, on AIX 4.1.3, shr.o in
     libc.a has symbols in the export table which are not in the
     symbol table.  */

  /* Read in the .loader section.  FIXME: We should really use the
     o_snloader field in the a.out header, rather than grabbing the
     section by name.  */
  lsec = bfd_get_section_by_name (abfd, ".loader");
  if (lsec == NULL)
    {
      (*_bfd_error_handler)
	("%s: dynamic object with no .loader section",
	 bfd_get_filename (abfd));
      bfd_set_error (bfd_error_no_symbols);
      return false;
    }

  if (! xcoff_get_section_contents (abfd, lsec))
    return false;
  buf = coff_section_data (abfd, lsec)->contents;

  /* Remove the sections from this object, so that they do not get
     included in the link.  */
  abfd->sections = NULL;

  xcoff_swap_ldhdr_in (abfd, (struct external_ldhdr *) buf, &ldhdr);

  strings = (char *) buf + ldhdr.l_stoff;

  elsym = (struct external_ldsym *) (buf + LDHDRSZ);
  elsymend = elsym + ldhdr.l_nsyms;
  BFD_ASSERT (sizeof (struct external_ldsym) == LDSYMSZ);
  for (; elsym < elsymend; elsym++)
    {
      struct internal_ldsym ldsym;
      char nambuf[SYMNMLEN + 1];
      const char *name;
      struct xcoff_link_hash_entry *h;

      xcoff_swap_ldsym_in (abfd, elsym, &ldsym);

      /* We are only interested in exported symbols.  */
      if ((ldsym.l_smtype & L_EXPORT) == 0)
	continue;

      if (ldsym._l._l_l._l_zeroes == 0)
	name = strings + ldsym._l._l_l._l_offset;
      else
	{
	  memcpy (nambuf, ldsym._l._l_name, SYMNMLEN);
	  nambuf[SYMNMLEN] = '\0';
	  name = nambuf;
	}

      /* Normally we could not call xcoff_link_hash_lookup in an add
	 symbols routine, since we might not be using an XCOFF hash
	 table.  However, we verified above that we are using an XCOFF
	 hash table.  */

      h = xcoff_link_hash_lookup (xcoff_hash_table (info), name, true,
				  true, true);
      if (h == NULL)
	return false;

      h->flags |= XCOFF_DEF_DYNAMIC;

      /* If the symbol is undefined, and the BFD it was found in is
	 not a dynamic object, change the BFD to this dynamic object,
	 so that we can get the correct import file ID.  */
      if ((h->root.type == bfd_link_hash_undefined
	   || h->root.type == bfd_link_hash_undefweak)
	  && (h->root.u.undef.abfd == NULL
	      || (h->root.u.undef.abfd->flags & DYNAMIC) == 0))
	h->root.u.undef.abfd = abfd;

      if (h->root.type == bfd_link_hash_new)
	{
	  h->root.type = bfd_link_hash_undefined;
	  h->root.u.undef.abfd = abfd;
	  /* We do not want to add this to the undefined symbol list.  */
	}

      if (h->smclas == XMC_UA
	  || h->root.type == bfd_link_hash_undefined
	  || h->root.type == bfd_link_hash_undefweak)
	h->smclas = ldsym.l_smclas;

      /* Unless this is an XMC_XO symbol, we don't bother to actually
         define it, since we don't have a section to put it in anyhow.
         Instead, the relocation routines handle the DEF_DYNAMIC flag
         correctly.  */

      if (h->smclas == XMC_XO
	  && (h->root.type == bfd_link_hash_undefined
	      || h->root.type == bfd_link_hash_undefweak))
	{
	  /* This symbol has an absolute value.  */
	  h->root.type = bfd_link_hash_defined;
	  h->root.u.def.section = bfd_abs_section_ptr;
	  h->root.u.def.value = ldsym.l_value;
	}
    }

  if (buf != NULL && ! coff_section_data (abfd, lsec)->keep_contents)
    {
      free (coff_section_data (abfd, lsec)->contents);
      coff_section_data (abfd, lsec)->contents = NULL;
    }

  /* Record this file in the import files.  */

  n = ((struct xcoff_import_file *)
       bfd_alloc (abfd, sizeof (struct xcoff_import_file)));
  if (n == NULL)
    return false;
  n->next = NULL;

  /* For some reason, the path entry in the import file list for a
     shared object appears to always be empty.  The file name is the
     base name.  */
  n->path = "";
  if (abfd->my_archive == NULL)
    {
      bname = bfd_get_filename (abfd);
      mname = "";
    }
  else
    {
      bname = bfd_get_filename (abfd->my_archive);
      mname = bfd_get_filename (abfd);
    }
  s = strrchr (bname, '/');
  if (s != NULL)
    bname = s + 1;
  n->file = bname;
  n->member = mname;

  /* We start c at 1 because the first import file number is reserved
     for LIBPATH.  */
  for (pp = &xcoff_hash_table (info)->imports, c = 1;
       *pp != NULL;
       pp = &(*pp)->next, ++c)
    ;
  *pp = n;

  xcoff_data (abfd)->import_file_id = c;

  return true;
}

/* Routines that are called after all the input files have been
   handled, but before the sections are laid out in memory.  */

/* Mark a symbol as not being garbage, including the section in which
   it is defined.  */

static INLINE boolean
xcoff_mark_symbol (info, h)
     struct bfd_link_info *info;
     struct xcoff_link_hash_entry *h;
{
  if ((h->flags & XCOFF_MARK) != 0)
    return true;

  h->flags |= XCOFF_MARK;
  if (h->root.type == bfd_link_hash_defined
      || h->root.type == bfd_link_hash_defweak)
    {
      asection *hsec;

      hsec = h->root.u.def.section;
      if ((hsec->flags & SEC_MARK) == 0)
	{
	  if (! xcoff_mark (info, hsec))
	    return false;
	}
    }

  if (h->toc_section != NULL
      && (h->toc_section->flags & SEC_MARK) == 0)
    {
      if (! xcoff_mark (info, h->toc_section))
	return false;
    }

  return true;
}

/* The mark phase of garbage collection.  For a given section, mark
   it, and all the sections which define symbols to which it refers.
   Because this function needs to look at the relocs, we also count
   the number of relocs which need to be copied into the .loader
   section.  */

static boolean
xcoff_mark (info, sec)
     struct bfd_link_info *info;
     asection *sec;
{
  if ((sec->flags & SEC_MARK) != 0)
    return true;

  sec->flags |= SEC_MARK;

  if (sec->owner->xvec == info->hash->creator
      && coff_section_data (sec->owner, sec) != NULL
      && xcoff_section_data (sec->owner, sec) != NULL)
    {
      register struct xcoff_link_hash_entry **hp, **hpend;
      struct internal_reloc *rel, *relend;

      /* Mark all the symbols in this section.  */

      hp = (obj_xcoff_sym_hashes (sec->owner)
	    + xcoff_section_data (sec->owner, sec)->first_symndx);
      hpend = (obj_xcoff_sym_hashes (sec->owner)
	       + xcoff_section_data (sec->owner, sec)->last_symndx);
      for (; hp < hpend; hp++)
	{
	  register struct xcoff_link_hash_entry *h;

	  h = *hp;
	  if (h != NULL
	      && (h->flags & XCOFF_MARK) == 0)
	    {
	      if (! xcoff_mark_symbol (info, h))
		return false;
	    }
	}

      /* Look through the section relocs.  */

      if ((sec->flags & SEC_RELOC) != 0
	  && sec->reloc_count > 0)
	{
	  rel = xcoff_read_internal_relocs (sec->owner, sec, true,
					    (bfd_byte *) NULL, false,
					    (struct internal_reloc *) NULL);
	  if (rel == NULL)
	    return false;
	  relend = rel + sec->reloc_count;
	  for (; rel < relend; rel++)
	    {
	      asection *rsec;
	      struct xcoff_link_hash_entry *h;

	      if ((unsigned int) rel->r_symndx
		  > obj_raw_syment_count (sec->owner))
		continue;

	      h = obj_xcoff_sym_hashes (sec->owner)[rel->r_symndx];
	      if (h != NULL
		  && (h->flags & XCOFF_MARK) == 0)
		{
		  if (! xcoff_mark_symbol (info, h))
		    return false;
		}

	      rsec = xcoff_data (sec->owner)->csects[rel->r_symndx];
	      if (rsec != NULL
		  && (rsec->flags & SEC_MARK) == 0)
		{
		  if (! xcoff_mark (info, rsec))
		    return false;
		}

	      /* See if this reloc needs to be copied into the .loader
                 section.  */
	      switch (rel->r_type)
		{
		default:
		  if (h == NULL
		      || h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak
		      || h->root.type == bfd_link_hash_common
		      || ((h->flags & XCOFF_CALLED) != 0
			  && (h->root.type == bfd_link_hash_undefined
			      || h->root.type == bfd_link_hash_undefweak)
			  && h->root.root.string[0] == '.'
			  && h->descriptor != NULL
			  && ((h->descriptor->flags & XCOFF_DEF_DYNAMIC) != 0
			      || info->shared
			      || ((h->descriptor->flags & XCOFF_IMPORT) != 0
				  && (h->descriptor->flags
				      & XCOFF_DEF_REGULAR) == 0))))
		    break;
		  /* Fall through.  */
		case R_POS:
		case R_NEG:
		case R_RL:
		case R_RLA:
		  ++xcoff_hash_table (info)->ldrel_count;
		  if (h != NULL)
		    h->flags |= XCOFF_LDREL;
		  break;
		case R_TOC:
		case R_GL:
		case R_TCL:
		case R_TRL:
		case R_TRLA:
		  /* We should never need a .loader reloc for a TOC
		     relative reloc.  */
		  break;
		}
	    }

	  if (! info->keep_memory
	      && coff_section_data (sec->owner, sec) != NULL
	      && coff_section_data (sec->owner, sec)->relocs != NULL
	      && ! coff_section_data (sec->owner, sec)->keep_relocs)
	    {
	      free (coff_section_data (sec->owner, sec)->relocs);
	      coff_section_data (sec->owner, sec)->relocs = NULL;
	    }
	}
    }

  return true;
}

/* The sweep phase of garbage collection.  Remove all garbage
   sections.  */

static void
xcoff_sweep (info)
     struct bfd_link_info *info;
{
  bfd *sub;

  for (sub = info->input_bfds; sub != NULL; sub = sub->link_next)
    {
      asection *o;

      for (o = sub->sections; o != NULL; o = o->next)
	{
	  if ((o->flags & SEC_MARK) == 0)
	    {
	      /* Keep all sections from non-XCOFF input files.  Keep
                 special sections.  Keep .debug sections for the
                 moment.  */
	      if (sub->xvec != info->hash->creator
		  || o == xcoff_hash_table (info)->debug_section
		  || o == xcoff_hash_table (info)->loader_section
		  || o == xcoff_hash_table (info)->linkage_section
		  || o == xcoff_hash_table (info)->toc_section
		  || o == xcoff_hash_table (info)->descriptor_section
		  || strcmp (o->name, ".debug") == 0)
		o->flags |= SEC_MARK;
	      else
		{
		  o->_raw_size = 0;
		  o->reloc_count = 0;
		  o->lineno_count = 0;
		}
	    }
	}
    }
}

/* Record the number of elements in a set.  This is used to output the
   correct csect length.  */

boolean
bfd_xcoff_link_record_set (output_bfd, info, harg, size)
     bfd *output_bfd;
     struct bfd_link_info *info;
     struct bfd_link_hash_entry *harg;
     bfd_size_type size;
{
  struct xcoff_link_hash_entry *h = (struct xcoff_link_hash_entry *) harg;
  struct xcoff_link_size_list *n;

  if (! XCOFF_XVECP (output_bfd->xvec))
    return true;

  /* This will hardly ever be called.  I don't want to burn four bytes
     per global symbol, so instead the size is kept on a linked list
     attached to the hash table.  */

  n = ((struct xcoff_link_size_list *)
       bfd_alloc (output_bfd, sizeof (struct xcoff_link_size_list)));
  if (n == NULL)
    return false;
  n->next = xcoff_hash_table (info)->size_list;
  n->h = h;
  n->size = size;
  xcoff_hash_table (info)->size_list = n;

  h->flags |= XCOFF_HAS_SIZE;

  return true;
}

/* Import a symbol.  */

boolean
bfd_xcoff_import_symbol (output_bfd, info, harg, val, imppath, impfile,
			 impmember)
     bfd *output_bfd;
     struct bfd_link_info *info;
     struct bfd_link_hash_entry *harg;
     bfd_vma val;
     const char *imppath;
     const char *impfile;
     const char *impmember;
{
  struct xcoff_link_hash_entry *h = (struct xcoff_link_hash_entry *) harg;

  if (! XCOFF_XVECP (output_bfd->xvec))
    return true;

  h->flags |= XCOFF_IMPORT;

  if (val != (bfd_vma) -1)
    {
      if (h->root.type == bfd_link_hash_defined
	  && (! bfd_is_abs_section (h->root.u.def.section)
	      || h->root.u.def.value != val))
	{
	  if (! ((*info->callbacks->multiple_definition)
		 (info, h->root.root.string, h->root.u.def.section->owner,
		  h->root.u.def.section, h->root.u.def.value,
		  output_bfd, bfd_abs_section_ptr, val)))
	    return false;
	}

      h->root.type = bfd_link_hash_defined;
      h->root.u.def.section = bfd_abs_section_ptr;
      h->root.u.def.value = val;
    }

  if (h->ldsym == NULL)
    {
      h->ldsym = ((struct internal_ldsym *)
		  bfd_zalloc (output_bfd, sizeof (struct internal_ldsym)));
      if (h->ldsym == NULL)
	return false;
    }

  if (imppath == NULL)
    h->ldsym->l_ifile = (bfd_size_type) -1;
  else
    {
      unsigned int c;
      struct xcoff_import_file **pp;

      /* We start c at 1 because the first entry in the import list is
         reserved for the library search path.  */
      for (pp = &xcoff_hash_table (info)->imports, c = 1;
	   *pp != NULL;
	   pp = &(*pp)->next, ++c)
	{
	  if (strcmp ((*pp)->path, imppath) == 0
	      && strcmp ((*pp)->file, impfile) == 0
	      && strcmp ((*pp)->member, impmember) == 0)
	    break;
	}

      if (*pp == NULL)
	{
	  struct xcoff_import_file *n;

	  n = ((struct xcoff_import_file *)
	       bfd_alloc (output_bfd, sizeof (struct xcoff_import_file)));
	  if (n == NULL)
	    return false;
	  n->next = NULL;
	  n->path = imppath;
	  n->file = impfile;
	  n->member = impmember;
	  *pp = n;
	}

      h->ldsym->l_ifile = c;
    }

  return true;
}

/* Export a symbol.  */

boolean
bfd_xcoff_export_symbol (output_bfd, info, harg, syscall)
     bfd *output_bfd;
     struct bfd_link_info *info;
     struct bfd_link_hash_entry *harg;
     boolean syscall;
{
  struct xcoff_link_hash_entry *h = (struct xcoff_link_hash_entry *) harg;

  if (! XCOFF_XVECP (output_bfd->xvec))
    return true;

  h->flags |= XCOFF_EXPORT;

  /* FIXME: I'm not at all sure what syscall is supposed to mean, so
     I'm just going to ignore it until somebody explains it.  */

  /* See if this is a function descriptor.  It may be one even though
     it is not so marked.  */
  if ((h->flags & XCOFF_DESCRIPTOR) == 0
      && h->root.root.string[0] != '.')
    {
      char *fnname;
      struct xcoff_link_hash_entry *hfn;

      fnname = (char *) bfd_malloc (strlen (h->root.root.string) + 2);
      if (fnname == NULL)
	return false;
      fnname[0] = '.';
      strcpy (fnname + 1, h->root.root.string);
      hfn = xcoff_link_hash_lookup (xcoff_hash_table (info),
				    fnname, false, false, true);
      free (fnname);
      if (hfn != NULL
	  && hfn->smclas == XMC_PR
	  && (hfn->root.type == bfd_link_hash_defined
	      || hfn->root.type == bfd_link_hash_defweak))
	{
	  h->flags |= XCOFF_DESCRIPTOR;
	  h->descriptor = hfn;
	  hfn->descriptor = h;
	}
    }

  /* Make sure we don't garbage collect this symbol.  */
  if (! xcoff_mark_symbol (info, h))
    return false;

  /* If this is a function descriptor, make sure we don't garbage
     collect the associated function code.  We normally don't have to
     worry about this, because the descriptor will be attached to a
     section with relocs, but if we are creating the descriptor
     ourselves those relocs will not be visible to the mark code.  */
  if ((h->flags & XCOFF_DESCRIPTOR) != 0)
    {
      if (! xcoff_mark_symbol (info, h->descriptor))
	return false;
    }

  return true;
}

/* Count a reloc against a symbol.  This is called for relocs
   generated by the linker script, typically for global constructors
   and destructors.  */

boolean
bfd_xcoff_link_count_reloc (output_bfd, info, name)
     bfd *output_bfd;
     struct bfd_link_info *info;
     const char *name;
{
  struct xcoff_link_hash_entry *h;

  if (! XCOFF_XVECP (output_bfd->xvec))
    return true;

  h = ((struct xcoff_link_hash_entry *)
       bfd_wrapped_link_hash_lookup (output_bfd, info, name, false, false,
				     false));
  if (h == NULL)
    {
      (*_bfd_error_handler) ("%s: no such symbol", name);
      bfd_set_error (bfd_error_no_symbols);
      return false;
    }

  h->flags |= XCOFF_REF_REGULAR | XCOFF_LDREL;
  ++xcoff_hash_table (info)->ldrel_count;
  
  /* Mark the symbol to avoid garbage collection.  */
  if (! xcoff_mark_symbol (info, h))
    return false;

  return true;
}

/* This function is called for each symbol to which the linker script
   assigns a value.  */

boolean
bfd_xcoff_record_link_assignment (output_bfd, info, name)
     bfd *output_bfd;
     struct bfd_link_info *info;
     const char *name;
{
  struct xcoff_link_hash_entry *h;

  if (! XCOFF_XVECP (output_bfd->xvec))
    return true;

  h = xcoff_link_hash_lookup (xcoff_hash_table (info), name, true, true,
			      false);
  if (h == NULL)
    return false;

  h->flags |= XCOFF_DEF_REGULAR;

  return true;
}

/* This structure is used to pass information through
   xcoff_link_hash_traverse.  */

struct xcoff_loader_info
{
  /* Set if a problem occurred.  */
  boolean failed;
  /* Output BFD.  */
  bfd *output_bfd;
  /* Link information structure.  */
  struct bfd_link_info *info;
  /* Whether all defined symbols should be exported.  */
  boolean export_defineds;
  /* Number of ldsym structures.  */
  size_t ldsym_count;
  /* Size of string table.  */
  size_t string_size;
  /* String table.  */
  bfd_byte *strings;
  /* Allocated size of string table.  */
  size_t string_alc;
};

/* Build the .loader section.  This is called by the XCOFF linker
   emulation before_allocation routine.  We must set the size of the
   .loader section before the linker lays out the output file.
   LIBPATH is the library path to search for shared objects; this is
   normally built from the -L arguments passed to the linker.  ENTRY
   is the name of the entry point symbol (the -e linker option).
   FILE_ALIGN is the alignment to use for sections within the file
   (the -H linker option).  MAXSTACK is the maximum stack size (the
   -bmaxstack linker option).  MAXDATA is the maximum data size (the
   -bmaxdata linker option).  GC is whether to do garbage collection
   (the -bgc linker option).  MODTYPE is the module type (the
   -bmodtype linker option).  TEXTRO is whether the text section must
   be read only (the -btextro linker option).  EXPORT_DEFINEDS is
   whether all defined symbols should be exported (the -unix linker
   option).  SPECIAL_SECTIONS is set by this routine to csects with
   magic names like _end.  */

boolean
bfd_xcoff_size_dynamic_sections (output_bfd, info, libpath, entry,
				 file_align, maxstack, maxdata, gc,
				 modtype, textro, export_defineds,
				 special_sections)
     bfd *output_bfd;
     struct bfd_link_info *info;
     const char *libpath;
     const char *entry;
     unsigned long file_align;
     unsigned long maxstack;
     unsigned long maxdata;
     boolean gc;
     int modtype;
     boolean textro;
     boolean export_defineds;
     asection **special_sections;
{
  struct xcoff_link_hash_entry *hentry;
  asection *lsec;
  struct xcoff_loader_info ldinfo;
  int i;
  size_t impsize, impcount;
  struct xcoff_import_file *fl;
  struct internal_ldhdr *ldhdr;
  bfd_size_type stoff;
  register char *out;
  asection *sec;
  bfd *sub;
  struct bfd_strtab_hash *debug_strtab;
  bfd_byte *debug_contents = NULL;

  if (! XCOFF_XVECP (output_bfd->xvec))
    {
      for (i = 0; i < 6; i++)
	special_sections[i] = NULL;
      return true;
    }

  ldinfo.failed = false;
  ldinfo.output_bfd = output_bfd;
  ldinfo.info = info;
  ldinfo.export_defineds = export_defineds;
  ldinfo.ldsym_count = 0;
  ldinfo.string_size = 0;
  ldinfo.strings = NULL;
  ldinfo.string_alc = 0;

  xcoff_data (output_bfd)->maxstack = maxstack;
  xcoff_data (output_bfd)->maxdata = maxdata;
  xcoff_data (output_bfd)->modtype = modtype;

  xcoff_hash_table (info)->file_align = file_align;
  xcoff_hash_table (info)->textro = textro;

  hentry = xcoff_link_hash_lookup (xcoff_hash_table (info), entry,
				   false, false, true);
  if (hentry != NULL)
    hentry->flags |= XCOFF_ENTRY;

  /* Garbage collect unused sections.  */
  if (info->relocateable
      || ! gc
      || hentry == NULL
      || (hentry->root.type != bfd_link_hash_defined
	  && hentry->root.type != bfd_link_hash_defweak))
    {
      gc = false;
      xcoff_hash_table (info)->gc = false;

      /* We still need to call xcoff_mark, in order to set ldrel_count
         correctly.  */
      for (sub = info->input_bfds; sub != NULL; sub = sub->link_next)
	{
	  asection *o;

	  for (o = sub->sections; o != NULL; o = o->next)
	    {
	      if ((o->flags & SEC_MARK) == 0)
		{
		  if (! xcoff_mark (info, o))
		    goto error_return;
		}
	    }
	}
    }
  else
    {
      if (! xcoff_mark (info, hentry->root.u.def.section))
	goto error_return;
      xcoff_sweep (info);
      xcoff_hash_table (info)->gc = true;
    }

  /* Return special sections to the caller.  */
  for (i = 0; i < 6; i++)
    {
      asection *sec;

      sec = xcoff_hash_table (info)->special_sections[i];
      if (sec != NULL
	  && gc
	  && (sec->flags & SEC_MARK) == 0)
	sec = NULL;
      special_sections[i] = sec;
    }

  if (info->input_bfds == NULL)
    {
      /* I'm not sure what to do in this bizarre case.  */
      return true;
    }

  xcoff_link_hash_traverse (xcoff_hash_table (info), xcoff_build_ldsyms,
			    (PTR) &ldinfo);
  if (ldinfo.failed)
    goto error_return;

  /* Work out the size of the import file names.  Each import file ID
     consists of three null terminated strings: the path, the file
     name, and the archive member name.  The first entry in the list
     of names is the path to use to find objects, which the linker has
     passed in as the libpath argument.  For some reason, the path
     entry in the other import file names appears to always be empty.  */
  impsize = strlen (libpath) + 3;
  impcount = 1;
  for (fl = xcoff_hash_table (info)->imports; fl != NULL; fl = fl->next)
    {
      ++impcount;
      impsize += (strlen (fl->path)
		  + strlen (fl->file)
		  + strlen (fl->member)
		  + 3);
    }

  /* Set up the .loader section header.  */
  ldhdr = &xcoff_hash_table (info)->ldhdr;
  ldhdr->l_version = 1;
  ldhdr->l_nsyms = ldinfo.ldsym_count;
  ldhdr->l_nreloc = xcoff_hash_table (info)->ldrel_count;
  ldhdr->l_istlen = impsize;
  ldhdr->l_nimpid = impcount;
  ldhdr->l_impoff = (LDHDRSZ
		     + ldhdr->l_nsyms * LDSYMSZ
		     + ldhdr->l_nreloc * LDRELSZ);
  ldhdr->l_stlen = ldinfo.string_size;
  stoff = ldhdr->l_impoff + impsize;
  if (ldinfo.string_size == 0)
    ldhdr->l_stoff = 0;
  else
    ldhdr->l_stoff = stoff;

  /* We now know the final size of the .loader section.  Allocate
     space for it.  */
  lsec = xcoff_hash_table (info)->loader_section;
  lsec->_raw_size = stoff + ldhdr->l_stlen;
  lsec->contents = (bfd_byte *) bfd_zalloc (output_bfd, lsec->_raw_size);
  if (lsec->contents == NULL)
    goto error_return;

  /* Set up the header.  */
  xcoff_swap_ldhdr_out (output_bfd, ldhdr,
			(struct external_ldhdr *) lsec->contents);

  /* Set up the import file names.  */
  out = (char *) lsec->contents + ldhdr->l_impoff;
  strcpy (out, libpath);
  out += strlen (libpath) + 1;
  *out++ = '\0';
  *out++ = '\0';
  for (fl = xcoff_hash_table (info)->imports; fl != NULL; fl = fl->next)
    {
      register const char *s;

      s = fl->path;
      while ((*out++ = *s++) != '\0')
	;
      s = fl->file;
      while ((*out++ = *s++) != '\0')
	;
      s = fl->member;
      while ((*out++ = *s++) != '\0')
	;
    }

  BFD_ASSERT ((bfd_size_type) ((bfd_byte *) out - lsec->contents) == stoff);

  /* Set up the symbol string table.  */
  if (ldinfo.string_size > 0)
    {
      memcpy (out, ldinfo.strings, ldinfo.string_size);
      free (ldinfo.strings);
      ldinfo.strings = NULL;
    }

  /* We can't set up the symbol table or the relocs yet, because we
     don't yet know the final position of the various sections.  The
     .loader symbols are written out when the corresponding normal
     symbols are written out in xcoff_link_input_bfd or
     xcoff_write_global_symbol.  The .loader relocs are written out
     when the corresponding normal relocs are handled in
     xcoff_link_input_bfd.  */

  /* Allocate space for the magic sections.  */
  sec = xcoff_hash_table (info)->linkage_section;
  if (sec->_raw_size > 0)
    {
      sec->contents = (bfd_byte *) bfd_zalloc (output_bfd, sec->_raw_size);
      if (sec->contents == NULL)
	goto error_return;
    }
  sec = xcoff_hash_table (info)->toc_section;
  if (sec->_raw_size > 0)
    {
      sec->contents = (bfd_byte *) bfd_zalloc (output_bfd, sec->_raw_size);
      if (sec->contents == NULL)
	goto error_return;
    }
  sec = xcoff_hash_table (info)->descriptor_section;
  if (sec->_raw_size > 0)
    {
      sec->contents = (bfd_byte *) bfd_zalloc (output_bfd, sec->_raw_size);
      if (sec->contents == NULL)
	goto error_return;
    }

  /* Now that we've done garbage collection, figure out the contents
     of the .debug section.  */
  debug_strtab = xcoff_hash_table (info)->debug_strtab;

  for (sub = info->input_bfds; sub != NULL; sub = sub->link_next)
    {
      asection *subdeb;
      bfd_size_type symcount;
      unsigned long *debug_index;
      asection **csectpp;
      bfd_byte *esym, *esymend;
      bfd_size_type symesz;

      if (sub->xvec != info->hash->creator)
	continue;
      subdeb = bfd_get_section_by_name (sub, ".debug");
      if (subdeb == NULL || subdeb->_raw_size == 0)
	continue;

      if (info->strip == strip_all
	  || info->strip == strip_debugger
	  || info->discard == discard_all)
	{
	  subdeb->_raw_size = 0;
	  continue;
	}

      if (! _bfd_coff_get_external_symbols (sub))
	goto error_return;

      symcount = obj_raw_syment_count (sub);
      debug_index = ((unsigned long *)
		     bfd_zalloc (sub, symcount * sizeof (unsigned long)));
      if (debug_index == NULL)
	goto error_return;
      xcoff_data (sub)->debug_indices = debug_index;

      /* Grab the contents of the .debug section.  We use malloc and
	 copy the neams into the debug stringtab, rather than
	 bfd_alloc, because I expect that, when linking many files
	 together, many of the strings will be the same.  Storing the
	 strings in the hash table should save space in this case.  */
      debug_contents = (bfd_byte *) bfd_malloc (subdeb->_raw_size);
      if (debug_contents == NULL)
	goto error_return;
      if (! bfd_get_section_contents (sub, subdeb, (PTR) debug_contents,
				      (file_ptr) 0, subdeb->_raw_size))
	goto error_return;

      csectpp = xcoff_data (sub)->csects;

      symesz = bfd_coff_symesz (sub);
      esym = (bfd_byte *) obj_coff_external_syms (sub);
      esymend = esym + symcount * symesz;
      while (esym < esymend)
	{
	  struct internal_syment sym;

	  bfd_coff_swap_sym_in (sub, (PTR) esym, (PTR) &sym);

	  *debug_index = (unsigned long) -1;

	  if (sym._n._n_n._n_zeroes == 0
	      && *csectpp != NULL
	      && (! gc
		  || ((*csectpp)->flags & SEC_MARK) != 0
		  || *csectpp == bfd_abs_section_ptr)
	      && bfd_coff_symname_in_debug (sub, &sym))
	    {
	      char *name;
	      bfd_size_type indx;

	      name = (char *) debug_contents + sym._n._n_n._n_offset;
	      indx = _bfd_stringtab_add (debug_strtab, name, true, true);
	      if (indx == (bfd_size_type) -1)
		goto error_return;
	      *debug_index = indx;
	    }

	  esym += (sym.n_numaux + 1) * symesz;
	  csectpp += sym.n_numaux + 1;
	  debug_index += sym.n_numaux + 1;
	}

      free (debug_contents);
      debug_contents = NULL;

      /* Clear the size of subdeb, so that it is not included directly
	 in the output file.  */
      subdeb->_raw_size = 0;

      if (! info->keep_memory)
	{
	  if (! _bfd_coff_free_symbols (sub))
	    goto error_return;
	}
    }

  xcoff_hash_table (info)->debug_section->_raw_size =
    _bfd_stringtab_size (debug_strtab);

  return true;

 error_return:
  if (ldinfo.strings != NULL)
    free (ldinfo.strings);
  if (debug_contents != NULL)
    free (debug_contents);
  return false;
}

/* Add a symbol to the .loader symbols, if necessary.  */

static boolean
xcoff_build_ldsyms (h, p)
     struct xcoff_link_hash_entry *h;
     PTR p;
{
  struct xcoff_loader_info *ldinfo = (struct xcoff_loader_info *) p;
  size_t len;

  /* If this is a final link, and the symbol was defined as a common
     symbol in a regular object file, and there was no definition in
     any dynamic object, then the linker will have allocated space for
     the symbol in a common section but the XCOFF_DEF_REGULAR flag
     will not have been set.  */
  if (h->root.type == bfd_link_hash_defined
      && (h->flags & XCOFF_DEF_REGULAR) == 0
      && (h->flags & XCOFF_REF_REGULAR) != 0
      && (h->flags & XCOFF_DEF_DYNAMIC) == 0
      && (h->root.u.def.section->owner->flags & DYNAMIC) == 0)
    h->flags |= XCOFF_DEF_REGULAR;

  /* If all defined symbols should be exported, mark them now.  We
     don't want to export the actual functions, just the function
     descriptors.  */
  if (ldinfo->export_defineds
      && (h->flags & XCOFF_DEF_REGULAR) != 0
      && h->root.root.string[0] != '.')
    h->flags |= XCOFF_EXPORT;

  /* We don't want to garbage collect symbols which are not defined in
     XCOFF files.  This is a convenient place to mark them.  */
  if (xcoff_hash_table (ldinfo->info)->gc
      && (h->flags & XCOFF_MARK) == 0
      && (h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak)
      && (h->root.u.def.section->owner == NULL
	  || (h->root.u.def.section->owner->xvec
	      != ldinfo->info->hash->creator)))
    h->flags |= XCOFF_MARK;

  /* If this symbol is called and defined in a dynamic object, or not
     defined at all when building a shared object, or imported, then
     we need to set up global linkage code for it.  (Unless we did
     garbage collection and we didn't need this symbol.)  */
  if ((h->flags & XCOFF_CALLED) != 0
      && (h->root.type == bfd_link_hash_undefined
	  || h->root.type == bfd_link_hash_undefweak)
      && h->root.root.string[0] == '.'
      && h->descriptor != NULL
      && ((h->descriptor->flags & XCOFF_DEF_DYNAMIC) != 0
	  || ldinfo->info->shared
	  || ((h->descriptor->flags & XCOFF_IMPORT) != 0
	      && (h->descriptor->flags & XCOFF_DEF_REGULAR) == 0))
      && (! xcoff_hash_table (ldinfo->info)->gc
	  || (h->flags & XCOFF_MARK) != 0))
    {
      asection *sec;
      struct xcoff_link_hash_entry *hds;

      sec = xcoff_hash_table (ldinfo->info)->linkage_section;
      h->root.type = bfd_link_hash_defined;
      h->root.u.def.section = sec;
      h->root.u.def.value = sec->_raw_size;
      h->smclas = XMC_GL;
      h->flags |= XCOFF_DEF_REGULAR;
      sec->_raw_size += XCOFF_GLINK_SIZE;

      /* The global linkage code requires a TOC entry for the
         descriptor.  */
      hds = h->descriptor;
      BFD_ASSERT ((hds->root.type == bfd_link_hash_undefined
		   || hds->root.type == bfd_link_hash_undefweak)
		  && (hds->flags & XCOFF_DEF_REGULAR) == 0);
      hds->flags |= XCOFF_MARK;
      if (hds->toc_section == NULL)
	{
	  hds->toc_section = xcoff_hash_table (ldinfo->info)->toc_section;
	  hds->u.toc_offset = hds->toc_section->_raw_size;
	  hds->toc_section->_raw_size += 4;
	  ++xcoff_hash_table (ldinfo->info)->ldrel_count;
	  ++hds->toc_section->reloc_count;
	  hds->indx = -2;
	  hds->flags |= XCOFF_SET_TOC | XCOFF_LDREL;

	  /* We need to call xcoff_build_ldsyms recursively here,
             because we may already have passed hds on the traversal.  */
	  xcoff_build_ldsyms (hds, p);
	}
    }

  /* If this symbol is exported, but not defined, we need to try to
     define it.  */
  if ((h->flags & XCOFF_EXPORT) != 0
      && (h->flags & XCOFF_IMPORT) == 0
      && (h->flags & XCOFF_DEF_REGULAR) == 0
      && (h->flags & XCOFF_DEF_DYNAMIC) == 0
      && (h->root.type == bfd_link_hash_undefined
	  || h->root.type == bfd_link_hash_undefweak))
    {
      if ((h->flags & XCOFF_DESCRIPTOR) != 0
	  && (h->descriptor->root.type == bfd_link_hash_defined
	      || h->descriptor->root.type == bfd_link_hash_defweak))
	{
	  asection *sec;

	  /* This is an undefined function descriptor associated with
             a defined entry point.  We can build up a function
             descriptor ourselves.  Believe it or not, the AIX linker
             actually does this, and there are cases where we need to
             do it as well.  */
	  sec = xcoff_hash_table (ldinfo->info)->descriptor_section;
	  h->root.type = bfd_link_hash_defined;
	  h->root.u.def.section = sec;
	  h->root.u.def.value = sec->_raw_size;
	  h->smclas = XMC_DS;
	  h->flags |= XCOFF_DEF_REGULAR;
	  sec->_raw_size += 12;

	  /* A function descriptor uses two relocs: one for the
             associated code, and one for the TOC address.  */
	  xcoff_hash_table (ldinfo->info)->ldrel_count += 2;
	  sec->reloc_count += 2;

	  /* We handle writing out the contents of the descriptor in
             xcoff_write_global_symbol.  */
	}
      else
	{
	  (*_bfd_error_handler)
	    ("attempt to export undefined symbol `%s'",
	     h->root.root.string);
	  ldinfo->failed = true;
	  bfd_set_error (bfd_error_invalid_operation);
	  return false;
	}
    }

  /* If this is still a common symbol, and it wasn't garbage
     collected, we need to actually allocate space for it in the .bss
     section.  */
  if (h->root.type == bfd_link_hash_common
      && (! xcoff_hash_table (ldinfo->info)->gc
	  || (h->flags & XCOFF_MARK) != 0)
      && h->root.u.c.p->section->_raw_size == 0)
    {
      BFD_ASSERT (bfd_is_com_section (h->root.u.c.p->section));
      h->root.u.c.p->section->_raw_size = h->root.u.c.size;
    }

  /* We need to add a symbol to the .loader section if it is mentioned
     in a reloc which we are copying to the .loader section and it was
     not defined or common, or if it is the entry point, or if it is
     being exported.  */

  if (((h->flags & XCOFF_LDREL) == 0
       || h->root.type == bfd_link_hash_defined
       || h->root.type == bfd_link_hash_defweak
       || h->root.type == bfd_link_hash_common)
      && (h->flags & XCOFF_ENTRY) == 0
      && (h->flags & XCOFF_EXPORT) == 0)
    {
      h->ldsym = NULL;
      return true;
    }

  /* We don't need to add this symbol if we did garbage collection and
     we did not mark this symbol.  */
  if (xcoff_hash_table (ldinfo->info)->gc
      && (h->flags & XCOFF_MARK) == 0)
    {
      h->ldsym = NULL;
      return true;
    }

  /* We may have already processed this symbol due to the recursive
     call above.  */
  if ((h->flags & XCOFF_BUILT_LDSYM) != 0)
    return true;

  /* We need to add this symbol to the .loader symbols.  */

  /* h->ldsym will already have been allocated for an explicitly
     imported symbol.  */
  if (h->ldsym == NULL)
    {
      h->ldsym = ((struct internal_ldsym *)
		  bfd_zalloc (ldinfo->output_bfd,
			      sizeof (struct internal_ldsym)));
      if (h->ldsym == NULL)
	{
	  ldinfo->failed = true;
	  return false;
	}
    }

  /* The first 3 symbol table indices are reserved to indicate the
     sections.  */
  h->ldindx = ldinfo->ldsym_count + 3;

  ++ldinfo->ldsym_count;

  len = strlen (h->root.root.string);
  if (len <= SYMNMLEN)
    strncpy (h->ldsym->_l._l_name, h->root.root.string, SYMNMLEN);
  else
    {
      if (ldinfo->string_size + len + 3 > ldinfo->string_alc)
	{
	  size_t newalc;
	  bfd_byte *newstrings;

	  newalc = ldinfo->string_alc * 2;
	  if (newalc == 0)
	    newalc = 32;
	  while (ldinfo->string_size + len + 3 > newalc)
	    newalc *= 2;

	  newstrings = ((bfd_byte *)
			bfd_realloc ((PTR) ldinfo->strings, newalc));
	  if (newstrings == NULL)
	    {
	      ldinfo->failed = true;
	      return false;
	    }
	  ldinfo->string_alc = newalc;
	  ldinfo->strings = newstrings;
	}

      bfd_put_16 (ldinfo->output_bfd, len + 1,
		  ldinfo->strings + ldinfo->string_size);
      strcpy (ldinfo->strings + ldinfo->string_size + 2, h->root.root.string);
      h->ldsym->_l._l_l._l_zeroes = 0;
      h->ldsym->_l._l_l._l_offset = ldinfo->string_size + 2;
      ldinfo->string_size += len + 3;
    }

  h->flags |= XCOFF_BUILT_LDSYM;

  return true;
}

/* Do the final link step.  */

boolean
_bfd_xcoff_bfd_final_link (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  bfd_size_type symesz;
  struct xcoff_final_link_info finfo;
  asection *o;
  struct bfd_link_order *p;
  size_t max_contents_size;
  size_t max_sym_count;
  size_t max_lineno_count;
  size_t max_reloc_count;
  size_t max_output_reloc_count;
  file_ptr rel_filepos;
  unsigned int relsz;
  file_ptr line_filepos;
  unsigned int linesz;
  bfd *sub;
  bfd_byte *external_relocs = NULL;
  char strbuf[STRING_SIZE_SIZE];

  if (info->shared)
    abfd->flags |= DYNAMIC;

  symesz = bfd_coff_symesz (abfd);

  finfo.info = info;
  finfo.output_bfd = abfd;
  finfo.strtab = NULL;
  finfo.section_info = NULL;
  finfo.last_file_index = -1;
  finfo.toc_symindx = -1;
  finfo.internal_syms = NULL;
  finfo.sym_indices = NULL;
  finfo.outsyms = NULL;
  finfo.linenos = NULL;
  finfo.contents = NULL;
  finfo.external_relocs = NULL;

  finfo.ldsym = ((struct external_ldsym *)
		 (xcoff_hash_table (info)->loader_section->contents
		  + LDHDRSZ));
  finfo.ldrel = ((struct external_ldrel *)
		 (xcoff_hash_table (info)->loader_section->contents
		  + LDHDRSZ
		  + xcoff_hash_table (info)->ldhdr.l_nsyms * LDSYMSZ));

  xcoff_data (abfd)->coff.link_info = info;

  finfo.strtab = _bfd_stringtab_init ();
  if (finfo.strtab == NULL)
    goto error_return;

  /* Count the line number and relocation entries required for the
     output file.  Determine a few maximum sizes.  */
  max_contents_size = 0;
  max_lineno_count = 0;
  max_reloc_count = 0;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      o->reloc_count = 0;
      o->lineno_count = 0;
      for (p = o->link_order_head; p != NULL; p = p->next)
	{
	  if (p->type == bfd_indirect_link_order)
	    {
	      asection *sec;

	      sec = p->u.indirect.section;

	      /* Mark all sections which are to be included in the
		 link.  This will normally be every section.  We need
		 to do this so that we can identify any sections which
		 the linker has decided to not include.  */
	      sec->linker_mark = true;

	      if (info->strip == strip_none
		  || info->strip == strip_some)
		o->lineno_count += sec->lineno_count;

	      o->reloc_count += sec->reloc_count;

	      if (sec->_raw_size > max_contents_size)
		max_contents_size = sec->_raw_size;
	      if (sec->lineno_count > max_lineno_count)
		max_lineno_count = sec->lineno_count;
	      if (coff_section_data (sec->owner, sec) != NULL
		  && xcoff_section_data (sec->owner, sec) != NULL
		  && (xcoff_section_data (sec->owner, sec)->lineno_count
		      > max_lineno_count))
		max_lineno_count =
		  xcoff_section_data (sec->owner, sec)->lineno_count;
	      if (sec->reloc_count > max_reloc_count)
		max_reloc_count = sec->reloc_count;
	    }
	  else if (p->type == bfd_section_reloc_link_order
		   || p->type == bfd_symbol_reloc_link_order)
	    ++o->reloc_count;
	}
    }

  /* Compute the file positions for all the sections.  */
  if (abfd->output_has_begun)
    {
      if (xcoff_hash_table (info)->file_align != 0)
	abort ();
    }
  else
    {
      bfd_vma file_align;

      file_align = xcoff_hash_table (info)->file_align;
      if (file_align != 0)
	{
	  boolean saw_contents;
	  int indx;
	  asection **op;
	  file_ptr sofar;

	  /* Insert .pad sections before every section which has
             contents and is loaded, if it is preceded by some other
             section which has contents and is loaded.  */
	  saw_contents = true;
	  for (op = &abfd->sections; *op != NULL; op = &(*op)->next)
	    {
	      (*op)->target_index = indx;
	      if (strcmp ((*op)->name, ".pad") == 0)
		saw_contents = false;
	      else if (((*op)->flags & SEC_HAS_CONTENTS) != 0
		       && ((*op)->flags & SEC_LOAD) != 0)
		{
		  if (! saw_contents)
		    saw_contents = true;
		  else
		    {
		      asection *n, *hold;

		      hold = *op;
		      *op = NULL;
		      n = bfd_make_section_anyway (abfd, ".pad");
		      BFD_ASSERT (*op == n);
		      n->next = hold;
		      n->flags = SEC_HAS_CONTENTS;
		      n->alignment_power = 0;
		      saw_contents = false;
		    }
		}
	    }

	  /* Reset the section indices after inserting the new
             sections.  */
	  indx = 0;
	  for (o = abfd->sections; o != NULL; o = o->next)
	    {
	      ++indx;
	      o->target_index = indx;
	    }
	  BFD_ASSERT ((unsigned int) indx == abfd->section_count);

	  /* Work out appropriate sizes for the .pad sections to force
             each section to land on a page boundary.  This bit of
             code knows what compute_section_file_positions is going
             to do.  */
	  sofar = bfd_coff_filhsz (abfd);
	  sofar += bfd_coff_aoutsz (abfd);
	  sofar += abfd->section_count * bfd_coff_scnhsz (abfd);
	  for (o = abfd->sections; o != NULL; o = o->next)
	    if (o->reloc_count >= 0xffff || o->lineno_count >= 0xffff)
	      sofar += bfd_coff_scnhsz (abfd);

	  for (o = abfd->sections; o != NULL; o = o->next)
	    {
	      if (strcmp (o->name, ".pad") == 0)
		{
		  bfd_vma pageoff;

		  BFD_ASSERT (o->_raw_size == 0);
		  pageoff = sofar & (file_align - 1);
		  if (pageoff != 0)
		    {
		      o->_raw_size = file_align - pageoff;
		      sofar += file_align - pageoff;
		      o->flags |= SEC_HAS_CONTENTS;
		    }
		}
	      else
		{
		  if ((o->flags & SEC_HAS_CONTENTS) != 0)
		    sofar += BFD_ALIGN (o->_raw_size,
					1 << o->alignment_power);
		}
	    }
	}

      bfd_coff_compute_section_file_positions (abfd);
    }

  /* Allocate space for the pointers we need to keep for the relocs.  */
  {
    unsigned int i;

    /* We use section_count + 1, rather than section_count, because
       the target_index fields are 1 based.  */
    finfo.section_info =
      ((struct xcoff_link_section_info *)
       bfd_malloc ((abfd->section_count + 1)
		   * sizeof (struct xcoff_link_section_info)));
    if (finfo.section_info == NULL)
      goto error_return;
    for (i = 0; i <= abfd->section_count; i++)
      {
	finfo.section_info[i].relocs = NULL;
	finfo.section_info[i].rel_hashes = NULL;
	finfo.section_info[i].toc_rel_hashes = NULL;
      }
  }

  /* Set the file positions for the relocs.  */
  rel_filepos = obj_relocbase (abfd);
  relsz = bfd_coff_relsz (abfd);
  max_output_reloc_count = 0;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      if (o->reloc_count == 0)
	o->rel_filepos = 0;
      else
	{
	  o->flags |= SEC_RELOC;
	  o->rel_filepos = rel_filepos;
	  rel_filepos += o->reloc_count * relsz;

	  /* We don't know the indices of global symbols until we have
             written out all the local symbols.  For each section in
             the output file, we keep an array of pointers to hash
             table entries.  Each entry in the array corresponds to a
             reloc.  When we find a reloc against a global symbol, we
             set the corresponding entry in this array so that we can
             fix up the symbol index after we have written out all the
             local symbols.

	     Because of this problem, we also keep the relocs in
	     memory until the end of the link.  This wastes memory.
	     We could backpatch the file later, I suppose, although it
	     would be slow.  */
	  finfo.section_info[o->target_index].relocs =
	    ((struct internal_reloc *)
	     bfd_malloc (o->reloc_count * sizeof (struct internal_reloc)));
	  finfo.section_info[o->target_index].rel_hashes =
	    ((struct xcoff_link_hash_entry **)
	     bfd_malloc (o->reloc_count
		     * sizeof (struct xcoff_link_hash_entry *)));
	  if (finfo.section_info[o->target_index].relocs == NULL
	      || finfo.section_info[o->target_index].rel_hashes == NULL)
	    goto error_return;

	  if (o->reloc_count > max_output_reloc_count)
	    max_output_reloc_count = o->reloc_count;
	}
    }

  /* We now know the size of the relocs, so we can determine the file
     positions of the line numbers.  */
  line_filepos = rel_filepos;
  finfo.line_filepos = line_filepos;
  linesz = bfd_coff_linesz (abfd);
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      if (o->lineno_count == 0)
	o->line_filepos = 0;
      else
	{
	  o->line_filepos = line_filepos;
	  line_filepos += o->lineno_count * linesz;
	}

      /* Reset the reloc and lineno counts, so that we can use them to
	 count the number of entries we have output so far.  */
      o->reloc_count = 0;
      o->lineno_count = 0;
    }

  obj_sym_filepos (abfd) = line_filepos;

  /* Figure out the largest number of symbols in an input BFD.  Take
     the opportunity to clear the output_has_begun fields of all the
     input BFD's.  We want at least 4 symbols, since that is the
     number which xcoff_write_global_symbol may need.  */
  max_sym_count = 4;
  for (sub = info->input_bfds; sub != NULL; sub = sub->link_next)
    {
      size_t sz;

      sub->output_has_begun = false;
      sz = obj_raw_syment_count (sub);
      if (sz > max_sym_count)
	max_sym_count = sz;
    }

  /* Allocate some buffers used while linking.  */
  finfo.internal_syms = ((struct internal_syment *)
			 bfd_malloc (max_sym_count
				     * sizeof (struct internal_syment)));
  finfo.sym_indices = (long *) bfd_malloc (max_sym_count * sizeof (long));
  finfo.outsyms = ((bfd_byte *)
		   bfd_malloc ((size_t) ((max_sym_count + 1) * symesz)));
  finfo.linenos = (bfd_byte *) bfd_malloc (max_lineno_count
					   * bfd_coff_linesz (abfd));
  finfo.contents = (bfd_byte *) bfd_malloc (max_contents_size);
  finfo.external_relocs = (bfd_byte *) bfd_malloc (max_reloc_count * relsz);
  if ((finfo.internal_syms == NULL && max_sym_count > 0)
      || (finfo.sym_indices == NULL && max_sym_count > 0)
      || finfo.outsyms == NULL
      || (finfo.linenos == NULL && max_lineno_count > 0)
      || (finfo.contents == NULL && max_contents_size > 0)
      || (finfo.external_relocs == NULL && max_reloc_count > 0))
    goto error_return;

  obj_raw_syment_count (abfd) = 0;
  xcoff_data (abfd)->toc = (bfd_vma) -1;

  /* We now know the position of everything in the file, except that
     we don't know the size of the symbol table and therefore we don't
     know where the string table starts.  We just build the string
     table in memory as we go along.  We process all the relocations
     for a single input file at once.  */
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      for (p = o->link_order_head; p != NULL; p = p->next)
	{
	  if (p->type == bfd_indirect_link_order
	      && p->u.indirect.section->owner->xvec == abfd->xvec)
	    {
	      sub = p->u.indirect.section->owner;
	      if (! sub->output_has_begun)
		{
		  if (! xcoff_link_input_bfd (&finfo, sub))
		    goto error_return;
		  sub->output_has_begun = true;
		}
	    }
	  else if (p->type == bfd_section_reloc_link_order
		   || p->type == bfd_symbol_reloc_link_order)
	    {
	      if (! xcoff_reloc_link_order (abfd, &finfo, o, p))
		goto error_return;
	    }
	  else
	    {
	      if (! _bfd_default_link_order (abfd, info, o, p))
		goto error_return;
	    }
	}
    }

  /* Free up the buffers used by xcoff_link_input_bfd.  */

  if (finfo.internal_syms != NULL)
    {
      free (finfo.internal_syms);
      finfo.internal_syms = NULL;
    }
  if (finfo.sym_indices != NULL)
    {
      free (finfo.sym_indices);
      finfo.sym_indices = NULL;
    }
  if (finfo.linenos != NULL)
    {
      free (finfo.linenos);
      finfo.linenos = NULL;
    }
  if (finfo.contents != NULL)
    {
      free (finfo.contents);
      finfo.contents = NULL;
    }
  if (finfo.external_relocs != NULL)
    {
      free (finfo.external_relocs);
      finfo.external_relocs = NULL;
    }

  /* The value of the last C_FILE symbol is supposed to be -1.  Write
     it out again.  */
  if (finfo.last_file_index != -1)
    {
      finfo.last_file.n_value = -1;
      bfd_coff_swap_sym_out (abfd, (PTR) &finfo.last_file,
			     (PTR) finfo.outsyms);
      if (bfd_seek (abfd,
		    (obj_sym_filepos (abfd)
		     + finfo.last_file_index * symesz),
		    SEEK_SET) != 0
	  || bfd_write (finfo.outsyms, symesz, 1, abfd) != symesz)
	goto error_return;
    }

  /* Write out all the global symbols which do not come from XCOFF
     input files.  */
  xcoff_link_hash_traverse (xcoff_hash_table (info),
			    xcoff_write_global_symbol,
			    (PTR) &finfo);

  if (finfo.outsyms != NULL)
    {
      free (finfo.outsyms);
      finfo.outsyms = NULL;
    }

  /* Now that we have written out all the global symbols, we know the
     symbol indices to use for relocs against them, and we can finally
     write out the relocs.  */
  external_relocs = (bfd_byte *) malloc (max_output_reloc_count * relsz);
  if (external_relocs == NULL && max_output_reloc_count != 0)
    {
      bfd_set_error (bfd_error_no_memory);
      goto error_return;
    }

  for (o = abfd->sections; o != NULL; o = o->next)
    {
      struct internal_reloc *irel;
      struct internal_reloc *irelend;
      struct xcoff_link_hash_entry **rel_hash;
      struct xcoff_toc_rel_hash *toc_rel_hash;
      bfd_byte *erel;

      if (o->reloc_count == 0)
	continue;

      irel = finfo.section_info[o->target_index].relocs;
      irelend = irel + o->reloc_count;
      rel_hash = finfo.section_info[o->target_index].rel_hashes;
      for (; irel < irelend; irel++, rel_hash++, erel += relsz)
	{
	  if (*rel_hash != NULL)
	    {
	      if ((*rel_hash)->indx < 0)
		{
		  if (! ((*info->callbacks->unattached_reloc)
			 (info, (*rel_hash)->root.root.string,
			  (bfd *) NULL, o, irel->r_vaddr)))
		    goto error_return;
		  (*rel_hash)->indx = 0;
		}
	      irel->r_symndx = (*rel_hash)->indx;
	    }
	}

      for (toc_rel_hash = finfo.section_info[o->target_index].toc_rel_hashes;
	   toc_rel_hash != NULL;
	   toc_rel_hash = toc_rel_hash->next)
	{
	  if (toc_rel_hash->h->u.toc_indx < 0)
	    {
	      if (! ((*info->callbacks->unattached_reloc)
		     (info, toc_rel_hash->h->root.root.string,
		      (bfd *) NULL, o, toc_rel_hash->rel->r_vaddr)))
		goto error_return;
	      toc_rel_hash->h->u.toc_indx = 0;
	    }
	  toc_rel_hash->rel->r_symndx = toc_rel_hash->h->u.toc_indx;
	}

      /* XCOFF requires that the relocs be sorted by address.  We tend
         to produce them in the order in which their containing csects
         appear in the symbol table, which is not necessarily by
         address.  So we sort them here.  There may be a better way to
         do this.  */
      qsort ((PTR) finfo.section_info[o->target_index].relocs,
	     o->reloc_count, sizeof (struct internal_reloc),
	     xcoff_sort_relocs);

      irel = finfo.section_info[o->target_index].relocs;
      irelend = irel + o->reloc_count;
      erel = external_relocs;
      for (; irel < irelend; irel++, rel_hash++, erel += relsz)
	bfd_coff_swap_reloc_out (abfd, (PTR) irel, (PTR) erel);

      if (bfd_seek (abfd, o->rel_filepos, SEEK_SET) != 0
	  || bfd_write ((PTR) external_relocs, relsz, o->reloc_count,
			abfd) != relsz * o->reloc_count)
	goto error_return;
    }

  if (external_relocs != NULL)
    {
      free (external_relocs);
      external_relocs = NULL;
    }

  /* Free up the section information.  */
  if (finfo.section_info != NULL)
    {
      unsigned int i;

      for (i = 0; i < abfd->section_count; i++)
	{
	  if (finfo.section_info[i].relocs != NULL)
	    free (finfo.section_info[i].relocs);
	  if (finfo.section_info[i].rel_hashes != NULL)
	    free (finfo.section_info[i].rel_hashes);
	}
      free (finfo.section_info);
      finfo.section_info = NULL;
    }

  /* Write out the loader section contents.  */
  BFD_ASSERT ((bfd_byte *) finfo.ldrel
	      == (xcoff_hash_table (info)->loader_section->contents
		  + xcoff_hash_table (info)->ldhdr.l_impoff));
  o = xcoff_hash_table (info)->loader_section;
  if (! bfd_set_section_contents (abfd, o->output_section,
				  o->contents, o->output_offset,
				  o->_raw_size))
    goto error_return;

  /* Write out the magic sections.  */
  o = xcoff_hash_table (info)->linkage_section;
  if (o->_raw_size > 0
      && ! bfd_set_section_contents (abfd, o->output_section, o->contents,
				     o->output_offset, o->_raw_size))
    goto error_return;
  o = xcoff_hash_table (info)->toc_section;
  if (o->_raw_size > 0
      && ! bfd_set_section_contents (abfd, o->output_section, o->contents,
				     o->output_offset, o->_raw_size))
    goto error_return;
  o = xcoff_hash_table (info)->descriptor_section;
  if (o->_raw_size > 0
      && ! bfd_set_section_contents (abfd, o->output_section, o->contents,
				     o->output_offset, o->_raw_size))
    goto error_return;

  /* Write out the string table.  */
  if (bfd_seek (abfd,
		(obj_sym_filepos (abfd)
		 + obj_raw_syment_count (abfd) * symesz),
		SEEK_SET) != 0)
    goto error_return;
  bfd_h_put_32 (abfd,
		_bfd_stringtab_size (finfo.strtab) + STRING_SIZE_SIZE,
		(bfd_byte *) strbuf);
  if (bfd_write (strbuf, 1, STRING_SIZE_SIZE, abfd) != STRING_SIZE_SIZE)
    goto error_return;
  if (! _bfd_stringtab_emit (abfd, finfo.strtab))
    goto error_return;

  _bfd_stringtab_free (finfo.strtab);

  /* Write out the debugging string table.  */
  o = xcoff_hash_table (info)->debug_section;
  if (o != NULL)
    {
      struct bfd_strtab_hash *debug_strtab;

      debug_strtab = xcoff_hash_table (info)->debug_strtab;
      BFD_ASSERT (o->output_section->_raw_size - o->output_offset
		  >= _bfd_stringtab_size (debug_strtab));
      if (bfd_seek (abfd,
		    o->output_section->filepos + o->output_offset,
		    SEEK_SET) != 0)
	goto error_return;
      if (! _bfd_stringtab_emit (abfd, debug_strtab))
	goto error_return;
    }

  /* Setting bfd_get_symcount to 0 will cause write_object_contents to
     not try to write out the symbols.  */
  bfd_get_symcount (abfd) = 0;

  return true;

 error_return:
  if (finfo.strtab != NULL)
    _bfd_stringtab_free (finfo.strtab);
  if (finfo.section_info != NULL)
    {
      unsigned int i;

      for (i = 0; i < abfd->section_count; i++)
	{
	  if (finfo.section_info[i].relocs != NULL)
	    free (finfo.section_info[i].relocs);
	  if (finfo.section_info[i].rel_hashes != NULL)
	    free (finfo.section_info[i].rel_hashes);
	}
      free (finfo.section_info);
    }
  if (finfo.internal_syms != NULL)
    free (finfo.internal_syms);
  if (finfo.sym_indices != NULL)
    free (finfo.sym_indices);
  if (finfo.outsyms != NULL)
    free (finfo.outsyms);
  if (finfo.linenos != NULL)
    free (finfo.linenos);
  if (finfo.contents != NULL)
    free (finfo.contents);
  if (finfo.external_relocs != NULL)
    free (finfo.external_relocs);
  if (external_relocs != NULL)
    free (external_relocs);
  return false;
}

/* Link an input file into the linker output file.  This function
   handles all the sections and relocations of the input file at once.  */

static boolean
xcoff_link_input_bfd (finfo, input_bfd)
     struct xcoff_final_link_info *finfo;
     bfd *input_bfd;
{
  bfd *output_bfd;
  const char *strings;
  bfd_size_type syment_base;
  unsigned int n_tmask;
  unsigned int n_btshft;
  boolean copy, hash;
  bfd_size_type isymesz;
  bfd_size_type osymesz;
  bfd_size_type linesz;
  bfd_byte *esym;
  bfd_byte *esym_end;
  struct xcoff_link_hash_entry **sym_hash;
  struct internal_syment *isymp;
  asection **csectpp;
  unsigned long *debug_index;
  long *indexp;
  unsigned long output_index;
  bfd_byte *outsym;
  unsigned int incls;
  asection *oline;
  boolean keep_syms;
  asection *o;

  /* We can just skip DYNAMIC files, unless this is a static link.  */
  if ((input_bfd->flags & DYNAMIC) != 0
      && ! finfo->info->static_link)
    return true;

  /* Move all the symbols to the output file.  */

  output_bfd = finfo->output_bfd;
  strings = NULL;
  syment_base = obj_raw_syment_count (output_bfd);
  isymesz = bfd_coff_symesz (input_bfd);
  osymesz = bfd_coff_symesz (output_bfd);
  linesz = bfd_coff_linesz (input_bfd);
  BFD_ASSERT (linesz == bfd_coff_linesz (output_bfd));

  n_tmask = coff_data (input_bfd)->local_n_tmask;
  n_btshft = coff_data (input_bfd)->local_n_btshft;

  /* Define macros so that ISFCN, et. al., macros work correctly.  */
#define N_TMASK n_tmask
#define N_BTSHFT n_btshft

  copy = false;
  if (! finfo->info->keep_memory)
    copy = true;
  hash = true;
  if ((output_bfd->flags & BFD_TRADITIONAL_FORMAT) != 0)
    hash = false;

  if (! _bfd_coff_get_external_symbols (input_bfd))
    return false;

  esym = (bfd_byte *) obj_coff_external_syms (input_bfd);
  esym_end = esym + obj_raw_syment_count (input_bfd) * isymesz;
  sym_hash = obj_xcoff_sym_hashes (input_bfd);
  csectpp = xcoff_data (input_bfd)->csects;
  debug_index = xcoff_data (input_bfd)->debug_indices;
  isymp = finfo->internal_syms;
  indexp = finfo->sym_indices;
  output_index = syment_base;
  outsym = finfo->outsyms;
  incls = 0;
  oline = NULL;

  while (esym < esym_end)
    {
      struct internal_syment isym;
      union internal_auxent aux;
      int smtyp = 0;
      boolean skip;
      boolean require;
      int add;

      bfd_coff_swap_sym_in (input_bfd, (PTR) esym, (PTR) isymp);

      /* If this is a C_EXT or C_HIDEXT symbol, we need the csect
         information.  */
      if (isymp->n_sclass == C_EXT || isymp->n_sclass == C_HIDEXT)
	{
	  BFD_ASSERT (isymp->n_numaux > 0);
	  bfd_coff_swap_aux_in (input_bfd,
				(PTR) (esym + isymesz * isymp->n_numaux),
				isymp->n_type, isymp->n_sclass,
				isymp->n_numaux - 1, isymp->n_numaux,
				(PTR) &aux);
	  smtyp = SMTYP_SMTYP (aux.x_csect.x_smtyp);
	}

      /* Make a copy of *isymp so that the relocate_section function
	 always sees the original values.  This is more reliable than
	 always recomputing the symbol value even if we are stripping
	 the symbol.  */
      isym = *isymp;

      /* If this symbol is in the .loader section, swap out the
         .loader symbol information.  If this is an external symbol
         reference to a defined symbol, though, then wait until we get
         to the definition.  */
      if (isym.n_sclass == C_EXT
	  && *sym_hash != NULL
	  && (*sym_hash)->ldsym != NULL
	  && (smtyp != XTY_ER
	      || (*sym_hash)->root.type == bfd_link_hash_undefined))
	{
	  struct xcoff_link_hash_entry *h;
	  struct internal_ldsym *ldsym;

	  h = *sym_hash;
	  ldsym = h->ldsym;
	  if (isym.n_scnum > 0)
	    {
	      ldsym->l_scnum = (*csectpp)->output_section->target_index;
	      ldsym->l_value = (isym.n_value
				+ (*csectpp)->output_section->vma
				+ (*csectpp)->output_offset
				- (*csectpp)->vma);
	    }
	  else
	    {
	      ldsym->l_scnum = isym.n_scnum;
	      ldsym->l_value = isym.n_value;
	    }

	  ldsym->l_smtype = smtyp;
	  if (((h->flags & XCOFF_DEF_REGULAR) == 0
	       && (h->flags & XCOFF_DEF_DYNAMIC) != 0)
	      || (h->flags & XCOFF_IMPORT) != 0)
	    ldsym->l_smtype |= L_IMPORT;
	  if (((h->flags & XCOFF_DEF_REGULAR) != 0
	       && (h->flags & XCOFF_DEF_DYNAMIC) != 0)
	      || (h->flags & XCOFF_EXPORT) != 0)
	    ldsym->l_smtype |= L_EXPORT;
	  if ((h->flags & XCOFF_ENTRY) != 0)
	    ldsym->l_smtype |= L_ENTRY;

	  ldsym->l_smclas = aux.x_csect.x_smclas;

	  if (ldsym->l_ifile == (bfd_size_type) -1)
	    ldsym->l_ifile = 0;
	  else if (ldsym->l_ifile == 0)
	    {
	      if ((ldsym->l_smtype & L_IMPORT) == 0)
		ldsym->l_ifile = 0;
	      else
		{
		  bfd *impbfd;

		  if (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak)
		    impbfd = h->root.u.def.section->owner;
		  else if (h->root.type == bfd_link_hash_undefined
			   || h->root.type == bfd_link_hash_undefweak)
		    impbfd = h->root.u.undef.abfd;
		  else
		    impbfd = NULL;

		  if (impbfd == NULL)
		    ldsym->l_ifile = 0;
		  else
		    {
		      BFD_ASSERT (impbfd->xvec == finfo->output_bfd->xvec);
		      ldsym->l_ifile = xcoff_data (impbfd)->import_file_id;
		    }
		}
	    }

	  ldsym->l_parm = 0;

	  BFD_ASSERT (h->ldindx >= 0);
	  BFD_ASSERT (LDSYMSZ == sizeof (struct external_ldsym));
	  xcoff_swap_ldsym_out (finfo->output_bfd, ldsym,
				finfo->ldsym + h->ldindx - 3);
	  h->ldsym = NULL;

	  /* Fill in snentry now that we know the target_index.  */
	  if ((h->flags & XCOFF_ENTRY) != 0
	      && (h->root.type == bfd_link_hash_defined
		  || h->root.type == bfd_link_hash_defweak))
	    xcoff_data (output_bfd)->snentry =
	      h->root.u.def.section->output_section->target_index;
	}

      *indexp = -1;

      skip = false;
      require = false;
      add = 1 + isym.n_numaux;

      /* If we are skipping this csect, we want to skip this symbol.  */
      if (*csectpp == NULL)
	skip = true;

      /* If we garbage collected this csect, we want to skip this
         symbol.  */
      if (! skip
	  && xcoff_hash_table (finfo->info)->gc
	  && ((*csectpp)->flags & SEC_MARK) == 0
	  && *csectpp != bfd_abs_section_ptr)
	skip = true;

      /* An XCOFF linker always skips C_STAT symbols.  */
      if (! skip
	  && isymp->n_sclass == C_STAT)
	skip = true;

      /* We skip all but the first TOC anchor.  */
      if (! skip
	  && isymp->n_sclass == C_HIDEXT
	  && aux.x_csect.x_smclas == XMC_TC0)
	{
	  if (finfo->toc_symindx != -1)
	    skip = true;
	  else
	    {
	      bfd_vma tocval, tocend;

	      tocval = ((*csectpp)->output_section->vma
			+ (*csectpp)->output_offset
			+ isym.n_value
			- (*csectpp)->vma);
	      /* We want to find out if tocval is a good value to use
                 as the TOC anchor--that is, whether we can access all
                 of the TOC using a 16 bit offset from tocval.  This
                 test assumes that the TOC comes at the end of the
                 output section, as it does in the default linker
                 script.  */

	      tocend = ((*csectpp)->output_section->vma
			+ (*csectpp)->output_section->_raw_size);

	      if (tocval + 0x10000 < tocend)
		{
		  (*_bfd_error_handler)
		    ("TOC overflow: 0x%lx > 0x10000; try -mminimal-toc when compiling",
		     (unsigned long) (tocend - tocval));
		  bfd_set_error (bfd_error_file_too_big);
		  return false;
		}

	      if (tocval + 0x8000 < tocend)
		{
		  bfd_vma tocadd;

		  tocadd = tocend - (tocval + 0x8000);
		  tocval += tocadd;
		  isym.n_value += tocadd;
		}

	      finfo->toc_symindx = output_index;
	      xcoff_data (finfo->output_bfd)->toc = tocval;
	      xcoff_data (finfo->output_bfd)->sntoc =
		(*csectpp)->output_section->target_index;
	      require = true;
	    }
	}

      /* If we are stripping all symbols, we want to skip this one.  */
      if (! skip
	  && finfo->info->strip == strip_all)
	skip = true;

      /* We can skip resolved external references.  */
      if (! skip
	  && isym.n_sclass == C_EXT
	  && smtyp == XTY_ER
	  && (*sym_hash)->root.type != bfd_link_hash_undefined)
	skip = true;

      /* We can skip common symbols if they got defined somewhere
         else.  */
      if (! skip
	  && isym.n_sclass == C_EXT
	  && smtyp == XTY_CM
	  && ((*sym_hash)->root.type != bfd_link_hash_common
	      || (*sym_hash)->root.u.c.p->section != *csectpp)
	  && ((*sym_hash)->root.type != bfd_link_hash_defined
	      || (*sym_hash)->root.u.def.section != *csectpp))
	skip = true;

      /* Skip local symbols if we are discarding them.  */
      if (! skip
	  && finfo->info->discard == discard_all
	  && isym.n_sclass != C_EXT
	  && (isym.n_sclass != C_HIDEXT
	      || smtyp != XTY_SD))
	skip = true;

      /* If we stripping debugging symbols, and this is a debugging
         symbol, then skip it.  */
      if (! skip
	  && finfo->info->strip == strip_debugger
	  && isym.n_scnum == N_DEBUG)
	skip = true;

      /* If some symbols are stripped based on the name, work out the
	 name and decide whether to skip this symbol.  We don't handle
	 this correctly for symbols whose names are in the .debug
	 section; to get it right we would need a new bfd_strtab_hash
	 function to return the string given the index.  */
      if (! skip
	  && (finfo->info->strip == strip_some
	      || finfo->info->discard == discard_l)
	  && (debug_index == NULL || *debug_index == (unsigned long) -1))
	{
	  const char *name;
	  char buf[SYMNMLEN + 1];

	  name = _bfd_coff_internal_syment_name (input_bfd, &isym, buf);
	  if (name == NULL)
	    return false;

	  if ((finfo->info->strip == strip_some
	       && (bfd_hash_lookup (finfo->info->keep_hash, name, false,
				    false) == NULL))
	      || (finfo->info->discard == discard_l
		  && (isym.n_sclass != C_EXT
		      && (isym.n_sclass != C_HIDEXT
			  || smtyp != XTY_SD))
		  && strncmp (name, finfo->info->lprefix,
			      finfo->info->lprefix_len) == 0))
	    skip = true;
	}

      /* We can not skip the first TOC anchor.  */
      if (skip
	  && require
	  && finfo->info->strip != strip_all)
	skip = false;

      /* We now know whether we are to skip this symbol or not.  */
      if (! skip)
	{
	  /* Adjust the symbol in order to output it.  */

	  if (isym._n._n_n._n_zeroes == 0
	      && isym._n._n_n._n_offset != 0)
	    {
	      /* This symbol has a long name.  Enter it in the string
		 table we are building.  If *debug_index != -1, the
		 name has already been entered in the .debug section.  */
	      if (debug_index != NULL && *debug_index != (unsigned long) -1)
		isym._n._n_n._n_offset = *debug_index;
	      else
		{
		  const char *name;
		  bfd_size_type indx;

		  name = _bfd_coff_internal_syment_name (input_bfd, &isym,
							 (char *) NULL);
		  if (name == NULL)
		    return false;
		  indx = _bfd_stringtab_add (finfo->strtab, name, hash, copy);
		  if (indx == (bfd_size_type) -1)
		    return false;
		  isym._n._n_n._n_offset = STRING_SIZE_SIZE + indx;
		}
	    }

	  if (isym.n_sclass != C_BSTAT
	      && isym.n_sclass != C_ESTAT
	      && isym.n_sclass != C_DECL
	      && isym.n_scnum > 0)
	    {
	      isym.n_scnum = (*csectpp)->output_section->target_index;
	      isym.n_value += ((*csectpp)->output_section->vma
			       + (*csectpp)->output_offset
			       - (*csectpp)->vma);
	    }

	  /* The value of a C_FILE symbol is the symbol index of the
	     next C_FILE symbol.  The value of the last C_FILE symbol
	     is -1.  We try to get this right, below, just before we
	     write the symbols out, but in the general case we may
	     have to write the symbol out twice.  */
	  if (isym.n_sclass == C_FILE)
	    {
	      if (finfo->last_file_index != -1
		  && finfo->last_file.n_value != (long) output_index)
		{
		  /* We must correct the value of the last C_FILE entry.  */
		  finfo->last_file.n_value = output_index;
		  if ((bfd_size_type) finfo->last_file_index >= syment_base)
		    {
		      /* The last C_FILE symbol is in this input file.  */
		      bfd_coff_swap_sym_out (output_bfd,
					     (PTR) &finfo->last_file,
					     (PTR) (finfo->outsyms
						    + ((finfo->last_file_index
							- syment_base)
						       * osymesz)));
		    }
		  else
		    {
		      /* We have already written out the last C_FILE
			 symbol.  We need to write it out again.  We
			 borrow *outsym temporarily.  */
		      bfd_coff_swap_sym_out (output_bfd,
					     (PTR) &finfo->last_file,
					     (PTR) outsym);
		      if (bfd_seek (output_bfd,
				    (obj_sym_filepos (output_bfd)
				     + finfo->last_file_index * osymesz),
				    SEEK_SET) != 0
			  || (bfd_write (outsym, osymesz, 1, output_bfd)
			      != osymesz))
			return false;
		    }
		}

	      finfo->last_file_index = output_index;
	      finfo->last_file = isym;
	    }

	  /* The value of a C_BINCL or C_EINCL symbol is a file offset
             into the line numbers.  We update the symbol values when
             we handle the line numbers.  */
	  if (isym.n_sclass == C_BINCL
	      || isym.n_sclass == C_EINCL)
	    {
	      isym.n_value = finfo->line_filepos;
	      ++incls;
	    }

	  /* Output the symbol.  */

	  bfd_coff_swap_sym_out (output_bfd, (PTR) &isym, (PTR) outsym);

	  *indexp = output_index;

	  if (isym.n_sclass == C_EXT)
	    {
	      long indx;
	      struct xcoff_link_hash_entry *h;

	      indx = ((esym - (bfd_byte *) obj_coff_external_syms (input_bfd))
		      / isymesz);
	      h = obj_xcoff_sym_hashes (input_bfd)[indx];
	      BFD_ASSERT (h != NULL);
	      h->indx = output_index;
	    }

	  /* If this is a symbol in the TOC which we may have merged
             (class XMC_TC), remember the symbol index of the TOC
             symbol.  */
	  if (isym.n_sclass == C_HIDEXT
	      && aux.x_csect.x_smclas == XMC_TC
	      && *sym_hash != NULL)
	    {
	      BFD_ASSERT (((*sym_hash)->flags & XCOFF_SET_TOC) == 0);
	      BFD_ASSERT ((*sym_hash)->toc_section != NULL);
	      (*sym_hash)->u.toc_indx = output_index;
	    }

	  output_index += add;
	  outsym += add * osymesz;
	}

      esym += add * isymesz;
      isymp += add;
      csectpp += add;
      sym_hash += add;
      if (debug_index != NULL)
	debug_index += add;
      ++indexp;
      for (--add; add > 0; --add)
	*indexp++ = -1;
    }

  /* Fix up the aux entries and the C_BSTAT symbols.  This must be
     done in a separate pass, because we don't know the correct symbol
     indices until we have already decided which symbols we are going
     to keep.  */

  esym = (bfd_byte *) obj_coff_external_syms (input_bfd);
  esym_end = esym + obj_raw_syment_count (input_bfd) * isymesz;
  isymp = finfo->internal_syms;
  indexp = finfo->sym_indices;
  csectpp = xcoff_data (input_bfd)->csects;
  outsym = finfo->outsyms;
  while (esym < esym_end)
    {
      int add;

      add = 1 + isymp->n_numaux;

      if (*indexp < 0)
	esym += add * isymesz;
      else
	{
	  int i;

	  if (isymp->n_sclass == C_BSTAT)
	    {
	      struct internal_syment isym;
	      unsigned long indx;

	      /* The value of a C_BSTAT symbol is the symbol table
                 index of the containing csect.  */
	      bfd_coff_swap_sym_in (output_bfd, (PTR) outsym, (PTR) &isym);
	      indx = isym.n_value;
	      if (indx < obj_raw_syment_count (input_bfd))
		{
		  long symindx;

		  symindx = finfo->sym_indices[indx];
		  if (symindx < 0)
		    isym.n_value = 0;
		  else
		    isym.n_value = symindx;
		  bfd_coff_swap_sym_out (output_bfd, (PTR) &isym,
					 (PTR) outsym);
		}
	    }

	  esym += isymesz;
	  outsym += osymesz;

	  for (i = 0; i < isymp->n_numaux && esym < esym_end; i++)
	    {
	      union internal_auxent aux;

	      bfd_coff_swap_aux_in (input_bfd, (PTR) esym, isymp->n_type,
				    isymp->n_sclass, i, isymp->n_numaux,
				    (PTR) &aux);

	      if (isymp->n_sclass == C_FILE)
		{
		  /* This is the file name (or some comment put in by
		     the compiler).  If it is long, we must put it in
		     the string table.  */
		  if (aux.x_file.x_n.x_zeroes == 0
		      && aux.x_file.x_n.x_offset != 0)
		    {
		      const char *filename;
		      bfd_size_type indx;

		      BFD_ASSERT (aux.x_file.x_n.x_offset
				  >= STRING_SIZE_SIZE);
		      if (strings == NULL)
			{
			  strings = _bfd_coff_read_string_table (input_bfd);
			  if (strings == NULL)
			    return false;
			}
		      filename = strings + aux.x_file.x_n.x_offset;
		      indx = _bfd_stringtab_add (finfo->strtab, filename,
						 hash, copy);
		      if (indx == (bfd_size_type) -1)
			return false;
		      aux.x_file.x_n.x_offset = STRING_SIZE_SIZE + indx;
		    }
		}
	      else if ((isymp->n_sclass == C_EXT
			|| isymp->n_sclass == C_HIDEXT)
		       && i + 1 == isymp->n_numaux)
		{
		  /* We don't support type checking.  I don't know if
                     anybody does.  */
		  aux.x_csect.x_parmhash = 0;
		  /* I don't think anybody uses these fields, but we'd
                     better clobber them just in case.  */
		  aux.x_csect.x_stab = 0;
		  aux.x_csect.x_snstab = 0;
		  if (SMTYP_SMTYP (aux.x_csect.x_smtyp) == XTY_LD)
		    {
		      unsigned long indx;

		      indx = aux.x_csect.x_scnlen.l;
		      if (indx < obj_raw_syment_count (input_bfd))
			{
			  long symindx;

			  symindx = finfo->sym_indices[indx];
			  if (symindx < 0)
			    aux.x_sym.x_tagndx.l = 0;
			  else
			    aux.x_sym.x_tagndx.l = symindx;
			}
		    }
		}
	      else if (isymp->n_sclass != C_STAT || isymp->n_type != T_NULL)
		{
		  unsigned long indx;

		  if (ISFCN (isymp->n_type)
		      || ISTAG (isymp->n_sclass)
		      || isymp->n_sclass == C_BLOCK
		      || isymp->n_sclass == C_FCN)
		    {
		      indx = aux.x_sym.x_fcnary.x_fcn.x_endndx.l;
		      if (indx > 0
			  && indx < obj_raw_syment_count (input_bfd))
			{
			  /* We look forward through the symbol for
                             the index of the next symbol we are going
                             to include.  I don't know if this is
                             entirely right.  */
			  while (finfo->sym_indices[indx] < 0
				 && indx < obj_raw_syment_count (input_bfd))
			    ++indx;
			  if (indx >= obj_raw_syment_count (input_bfd))
			    indx = output_index;
			  else
			    indx = finfo->sym_indices[indx];
			  aux.x_sym.x_fcnary.x_fcn.x_endndx.l = indx;
			}
		    }

		  indx = aux.x_sym.x_tagndx.l;
		  if (indx > 0 && indx < obj_raw_syment_count (input_bfd))
		    {
		      long symindx;

		      symindx = finfo->sym_indices[indx];
		      if (symindx < 0)
			aux.x_sym.x_tagndx.l = 0;
		      else
			aux.x_sym.x_tagndx.l = symindx;
		    }
		}

	      /* Copy over the line numbers, unless we are stripping
		 them.  We do this on a symbol by symbol basis in
		 order to more easily handle garbage collection.  */
	      if ((isymp->n_sclass == C_EXT
		   || isymp->n_sclass == C_HIDEXT)
		  && i == 0
		  && isymp->n_numaux > 1
		  && ISFCN (isymp->n_type)
		  && aux.x_sym.x_fcnary.x_fcn.x_lnnoptr != 0)
		{
		  if (finfo->info->strip != strip_none
		      && finfo->info->strip != strip_some)
		    aux.x_sym.x_fcnary.x_fcn.x_lnnoptr = 0;
		  else
		    {
		      asection *enclosing;
		      unsigned int enc_count;
		      bfd_size_type linoff;
		      struct internal_lineno lin;

		      o = *csectpp;
		      enclosing = xcoff_section_data (abfd, o)->enclosing;
		      enc_count = xcoff_section_data (abfd, o)->lineno_count;
		      if (oline != enclosing)
			{
			  if (bfd_seek (input_bfd,
					enclosing->line_filepos,
					SEEK_SET) != 0
			      || (bfd_read (finfo->linenos, linesz,
					    enc_count, input_bfd)
				  != linesz * enc_count))
			    return false;
			  oline = enclosing;
			}

		      linoff = (aux.x_sym.x_fcnary.x_fcn.x_lnnoptr
				- enclosing->line_filepos);

		      bfd_coff_swap_lineno_in (input_bfd,
					       (PTR) (finfo->linenos + linoff),
					       (PTR) &lin);
		      if (lin.l_lnno != 0
			  || ((bfd_size_type) lin.l_addr.l_symndx
			      != ((esym
				   - isymesz
				   - ((bfd_byte *)
				      obj_coff_external_syms (input_bfd)))
				  / isymesz)))
			aux.x_sym.x_fcnary.x_fcn.x_lnnoptr = 0;
		      else
			{
			  bfd_byte *linpend, *linp;
			  bfd_vma offset;
			  bfd_size_type count;

			  lin.l_addr.l_symndx = *indexp;
			  bfd_coff_swap_lineno_out (output_bfd, (PTR) &lin,
						    (PTR) (finfo->linenos
							   + linoff));

			  linpend = (finfo->linenos
				     + enc_count * linesz);
			  offset = (o->output_section->vma
				    + o->output_offset
				    - o->vma);
			  for (linp = finfo->linenos + linoff + linesz;
			       linp < linpend;
			       linp += linesz)
			    {
			      bfd_coff_swap_lineno_in (input_bfd, (PTR) linp,
						       (PTR) &lin);
			      if (lin.l_lnno == 0)
				break;
			      lin.l_addr.l_paddr += offset;
			      bfd_coff_swap_lineno_out (output_bfd,
							(PTR) &lin,
							(PTR) linp);
			    }

			  count = (linp - (finfo->linenos + linoff)) / linesz;

			  aux.x_sym.x_fcnary.x_fcn.x_lnnoptr =
			    (o->output_section->line_filepos
			     + o->output_section->lineno_count * linesz);

			  if (bfd_seek (output_bfd,
					aux.x_sym.x_fcnary.x_fcn.x_lnnoptr,
					SEEK_SET) != 0
			      || (bfd_write (finfo->linenos + linoff,
					     linesz, count, output_bfd)
				  != linesz * count))
			    return false;

			  o->output_section->lineno_count += count;

			  if (incls > 0)
			    {
			      struct internal_syment *iisp, *iispend;
			      long *iindp;
			      bfd_byte *oos;
			      int iiadd;

			      /* Update any C_BINCL or C_EINCL symbols
                                 that refer to a line number in the
                                 range we just output.  */
			      iisp = finfo->internal_syms;
			      iispend = (iisp
					 + obj_raw_syment_count (input_bfd));
			      iindp = finfo->sym_indices;
			      oos = finfo->outsyms;
			      while (iisp < iispend)
				{
				  if (*iindp >= 0
				      && (iisp->n_sclass == C_BINCL
					  || iisp->n_sclass == C_EINCL)
				      && ((bfd_size_type) iisp->n_value
					  >= enclosing->line_filepos + linoff)
				      && ((bfd_size_type) iisp->n_value
					  < (enclosing->line_filepos
					     + enc_count * linesz)))
				    {
				      struct internal_syment iis;

				      bfd_coff_swap_sym_in (output_bfd,
							    (PTR) oos,
							    (PTR) &iis);
				      iis.n_value =
					(iisp->n_value
					 - enclosing->line_filepos
					 - linoff
					 + aux.x_sym.x_fcnary.x_fcn.x_lnnoptr);
				      bfd_coff_swap_sym_out (output_bfd,
							     (PTR) &iis,
							     (PTR) oos);
				      --incls;
				    }

				  iiadd = 1 + iisp->n_numaux;
				  if (*iindp >= 0)
				    oos += iiadd * osymesz;
				  iisp += iiadd;
				  iindp += iiadd;
				}
			    }
			}
		    }
		}

	      bfd_coff_swap_aux_out (output_bfd, (PTR) &aux, isymp->n_type,
				     isymp->n_sclass, i, isymp->n_numaux,
				     (PTR) outsym);
	      outsym += osymesz;
	      esym += isymesz;
	    }
	}

      indexp += add;
      isymp += add;
      csectpp += add;
    }

  /* If we swapped out a C_FILE symbol, guess that the next C_FILE
     symbol will be the first symbol in the next input file.  In the
     normal case, this will save us from writing out the C_FILE symbol
     again.  */
  if (finfo->last_file_index != -1
      && (bfd_size_type) finfo->last_file_index >= syment_base)
    {
      finfo->last_file.n_value = output_index;
      bfd_coff_swap_sym_out (output_bfd, (PTR) &finfo->last_file,
			     (PTR) (finfo->outsyms
 				    + ((finfo->last_file_index - syment_base)
 				       * osymesz)));
    }

  /* Write the modified symbols to the output file.  */
  if (outsym > finfo->outsyms)
    {
      if (bfd_seek (output_bfd,
		    obj_sym_filepos (output_bfd) + syment_base * osymesz,
		    SEEK_SET) != 0
	  || (bfd_write (finfo->outsyms, outsym - finfo->outsyms, 1,
			output_bfd)
	      != (bfd_size_type) (outsym - finfo->outsyms)))
	return false;

      BFD_ASSERT ((obj_raw_syment_count (output_bfd)
		   + (outsym - finfo->outsyms) / osymesz)
		  == output_index);

      obj_raw_syment_count (output_bfd) = output_index;
    }

  /* Don't let the linker relocation routines discard the symbols.  */
  keep_syms = obj_coff_keep_syms (input_bfd);
  obj_coff_keep_syms (input_bfd) = true;

  /* Relocate the contents of each section.  */
  for (o = input_bfd->sections; o != NULL; o = o->next)
    {
      bfd_byte *contents;

      if (! o->linker_mark)
	{
	  /* This section was omitted from the link.  */
	  continue;
	}

      if ((o->flags & SEC_HAS_CONTENTS) == 0
	  || o->_raw_size == 0
	  || (o->flags & SEC_IN_MEMORY) != 0)
	continue;

      /* We have set filepos correctly for the sections we created to
         represent csects, so bfd_get_section_contents should work.  */
      if (coff_section_data (input_bfd, o) != NULL
	  && coff_section_data (input_bfd, o)->contents != NULL)
	contents = coff_section_data (input_bfd, o)->contents;
      else
	{
	  if (! bfd_get_section_contents (input_bfd, o, finfo->contents,
					  (file_ptr) 0, o->_raw_size))
	    return false;
	  contents = finfo->contents;
	}

      if ((o->flags & SEC_RELOC) != 0)
	{
	  int target_index;
	  struct internal_reloc *internal_relocs;
	  struct internal_reloc *irel;
	  bfd_vma offset;
	  struct internal_reloc *irelend;
	  struct xcoff_link_hash_entry **rel_hash;
	  long r_symndx;

	  /* Read in the relocs.  */
	  target_index = o->output_section->target_index;
	  internal_relocs = (xcoff_read_internal_relocs
			     (input_bfd, o, false, finfo->external_relocs,
			      true,
			      (finfo->section_info[target_index].relocs
			       + o->output_section->reloc_count)));
	  if (internal_relocs == NULL)
	    return false;

	  /* Call processor specific code to relocate the section
             contents.  */
	  if (! bfd_coff_relocate_section (output_bfd, finfo->info,
					   input_bfd, o,
					   contents,
					   internal_relocs,
					   finfo->internal_syms,
					   xcoff_data (input_bfd)->csects))
	    return false;

	  offset = o->output_section->vma + o->output_offset - o->vma;
	  irel = internal_relocs;
	  irelend = irel + o->reloc_count;
	  rel_hash = (finfo->section_info[target_index].rel_hashes
		      + o->output_section->reloc_count);
	  for (; irel < irelend; irel++, rel_hash++)
	    {
	      struct xcoff_link_hash_entry *h = NULL;
	      struct internal_ldrel ldrel;

	      *rel_hash = NULL;

	      /* Adjust the reloc address and symbol index.  */

	      irel->r_vaddr += offset;

	      r_symndx = irel->r_symndx;

	      if (r_symndx != -1)
		{
		  h = obj_xcoff_sym_hashes (input_bfd)[r_symndx];
		  if (h != NULL
		      && (irel->r_type == R_TOC
			  || irel->r_type == R_GL
			  || irel->r_type == R_TCL
			  || irel->r_type == R_TRL
			  || irel->r_type == R_TRLA))
		    {
		      /* This is a TOC relative reloc with a symbol
                         attached.  The symbol should be the one which
                         this reloc is for.  We want to make this
                         reloc against the TOC address of the symbol,
                         not the symbol itself.  */
		      BFD_ASSERT (h->toc_section != NULL);
		      BFD_ASSERT ((h->flags & XCOFF_SET_TOC) == 0);
		      if (h->u.toc_indx != -1)
			irel->r_symndx = h->u.toc_indx;
		      else
			{
			  struct xcoff_toc_rel_hash *n;
			  struct xcoff_link_section_info *si;

			  n = ((struct xcoff_toc_rel_hash *)
			       bfd_alloc (finfo->output_bfd,
					  sizeof (struct xcoff_toc_rel_hash)));
			  if (n == NULL)
			    return false;
			  si = finfo->section_info + target_index;
			  n->next = si->toc_rel_hashes;
			  n->h = h;
			  n->rel = irel;
			  si->toc_rel_hashes = n;
			}
		    }
		  else if (h != NULL)
		    {
		      /* This is a global symbol.  */
		      if (h->indx >= 0)
			irel->r_symndx = h->indx;
		      else
			{
			  /* This symbol is being written at the end
			     of the file, and we do not yet know the
			     symbol index.  We save the pointer to the
			     hash table entry in the rel_hash list.
			     We set the indx field to -2 to indicate
			     that this symbol must not be stripped.  */
			  *rel_hash = h;
			  h->indx = -2;
			}
		    }
		  else
		    {
		      long indx;

		      indx = finfo->sym_indices[r_symndx];

		      if (indx == -1)
			{
			  struct internal_syment *is;

			  /* Relocations against a TC0 TOC anchor are
			     automatically transformed to be against
			     the TOC anchor in the output file.  */
			  is = finfo->internal_syms + r_symndx;
			  if (is->n_sclass == C_HIDEXT
			      && is->n_numaux > 0)
			    {
			      PTR auxptr;
			      union internal_auxent aux;

			      auxptr = ((PTR)
					(((bfd_byte *)
					  obj_coff_external_syms (input_bfd))
					 + ((r_symndx + is->n_numaux)
					    * isymesz)));
			      bfd_coff_swap_aux_in (input_bfd, auxptr,
						    is->n_type, is->n_sclass,
						    is->n_numaux - 1,
						    is->n_numaux,
						    (PTR) &aux);
			      if (SMTYP_SMTYP (aux.x_csect.x_smtyp) == XTY_SD
				  && aux.x_csect.x_smclas == XMC_TC0)
				indx = finfo->toc_symindx;
			    }
			}

		      if (indx != -1)
			irel->r_symndx = indx;
		      else
			{
			  struct internal_syment *is;
			  const char *name;
			  char buf[SYMNMLEN + 1];

			  /* This reloc is against a symbol we are
			     stripping.  It would be possible to handle
			     this case, but I don't think it's worth it.  */
			  is = finfo->internal_syms + r_symndx;

			  name = (_bfd_coff_internal_syment_name
				  (input_bfd, is, buf));
			  if (name == NULL)
			    return false;

			  if (! ((*finfo->info->callbacks->unattached_reloc)
				 (finfo->info, name, input_bfd, o,
				  irel->r_vaddr)))
			    return false;
			}
		    }
		}

	      switch (irel->r_type)
		{
		default:
		  if (h == NULL
		      || h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak
		      || h->root.type == bfd_link_hash_common)
		    break;
		  /* Fall through.  */
		case R_POS:
		case R_NEG:
		case R_RL:
		case R_RLA:
		  /* This reloc needs to be copied into the .loader
		     section.  */
		  ldrel.l_vaddr = irel->r_vaddr;
		  if (r_symndx == -1)
		    ldrel.l_symndx = -1;
		  else if (h == NULL
			   || (h->root.type == bfd_link_hash_defined
			       || h->root.type == bfd_link_hash_defweak
			       || h->root.type == bfd_link_hash_common))
		    {
		      asection *sec;

		      if (h == NULL)
			sec = xcoff_data (input_bfd)->csects[r_symndx];
		      else if (h->root.type == bfd_link_hash_common)
			sec = h->root.u.c.p->section;
		      else
			sec = h->root.u.def.section;
		      sec = sec->output_section;

		      if (strcmp (sec->name, ".text") == 0)
			ldrel.l_symndx = 0;
		      else if (strcmp (sec->name, ".data") == 0)
			ldrel.l_symndx = 1;
		      else if (strcmp (sec->name, ".bss") == 0)
			ldrel.l_symndx = 2;
		      else
			{
			  (*_bfd_error_handler)
			    ("%s: loader reloc in unrecognized section `%s'",
			     bfd_get_filename (input_bfd),
			     sec->name);
			  bfd_set_error (bfd_error_nonrepresentable_section);
			  return false;
			}
		    }
		  else
		    {
		      if (h->ldindx < 0)
			{
			  (*_bfd_error_handler)
			    ("%s: `%s' in loader reloc but not loader sym",
			     bfd_get_filename (input_bfd),
			     h->root.root.string);
			  bfd_set_error (bfd_error_bad_value);
			  return false;
			}
		      ldrel.l_symndx = h->ldindx;
		    }
		  ldrel.l_rtype = (irel->r_size << 8) | irel->r_type;
		  ldrel.l_rsecnm = o->output_section->target_index;
		  if (xcoff_hash_table (finfo->info)->textro
		      && strcmp (o->output_section->name, ".text") == 0)
		    {
		      (*_bfd_error_handler)
			("%s: loader reloc in read-only section %s",
			 bfd_get_filename (input_bfd),
			 bfd_get_section_name (finfo->output_bfd,
					       o->output_section));
		      bfd_set_error (bfd_error_invalid_operation);
		      return false;
		    }
		  xcoff_swap_ldrel_out (output_bfd, &ldrel,
					finfo->ldrel);
		  BFD_ASSERT (sizeof (struct external_ldrel) == LDRELSZ);
		  ++finfo->ldrel;
		  break;

		case R_TOC:
		case R_GL:
		case R_TCL:
		case R_TRL:
		case R_TRLA:
		  /* We should never need a .loader reloc for a TOC
		     relative reloc.  */
		  break;
		}
	    }

	  o->output_section->reloc_count += o->reloc_count;
	}

      /* Write out the modified section contents.  */
      if (! bfd_set_section_contents (output_bfd, o->output_section,
				      contents, o->output_offset,
				      (o->_cooked_size != 0
				       ? o->_cooked_size
				       : o->_raw_size)))
	return false;
    }

  obj_coff_keep_syms (input_bfd) = keep_syms;

  if (! finfo->info->keep_memory)
    {
      if (! _bfd_coff_free_symbols (input_bfd))
	return false;
    }

  return true;
}

#undef N_TMASK
#undef N_BTSHFT

/* Write out a non-XCOFF global symbol.  */

static boolean
xcoff_write_global_symbol (h, p)
     struct xcoff_link_hash_entry *h;
     PTR p;
{
  struct xcoff_final_link_info *finfo = (struct xcoff_final_link_info *) p;
  bfd *output_bfd;
  bfd_byte *outsym;
  struct internal_syment isym;
  union internal_auxent aux;

  output_bfd = finfo->output_bfd;

  /* If this symbol was garbage collected, just skip it.  */
  if (xcoff_hash_table (finfo->info)->gc
      && (h->flags & XCOFF_MARK) == 0)
    return true;

  /* If we need a .loader section entry, write it out.  */
  if (h->ldsym != NULL)
    {
      struct internal_ldsym *ldsym;
      bfd *impbfd;

      ldsym = h->ldsym;

      if (h->root.type == bfd_link_hash_undefined
	  || h->root.type == bfd_link_hash_undefweak)
	{
	  ldsym->l_value = 0;
	  ldsym->l_scnum = N_UNDEF;
	  ldsym->l_smtype = XTY_ER;
	  impbfd = h->root.u.undef.abfd;
	}
      else if (h->root.type == bfd_link_hash_defined
	       || h->root.type == bfd_link_hash_defweak)
	{
	  asection *sec;

	  sec = h->root.u.def.section;
	  ldsym->l_value = (sec->output_section->vma
			    + sec->output_offset
			    + h->root.u.def.value);
	  ldsym->l_scnum = sec->output_section->target_index;
	  ldsym->l_smtype = XTY_SD;
	  impbfd = sec->owner;
	}
      else
	abort ();

      if (((h->flags & XCOFF_DEF_REGULAR) == 0
	   && (h->flags & XCOFF_DEF_DYNAMIC) != 0)
	  || (h->flags & XCOFF_IMPORT) != 0)
	ldsym->l_smtype |= L_IMPORT;
      if (((h->flags & XCOFF_DEF_REGULAR) != 0
	   && (h->flags & XCOFF_DEF_DYNAMIC) != 0)
	  || (h->flags & XCOFF_EXPORT) != 0)
	ldsym->l_smtype |= L_EXPORT;
      if ((h->flags & XCOFF_ENTRY) != 0)
	ldsym->l_smtype |= L_ENTRY;

      ldsym->l_smclas = h->smclas;

      if (ldsym->l_ifile == (bfd_size_type) -1)
	ldsym->l_ifile = 0;
      else if (ldsym->l_ifile == 0)
	{
	  if ((ldsym->l_smtype & L_IMPORT) == 0)
	    ldsym->l_ifile = 0;
	  else if (impbfd == NULL)
	    ldsym->l_ifile = 0;
	  else
	    {
	      BFD_ASSERT (impbfd->xvec == output_bfd->xvec);
	      ldsym->l_ifile = xcoff_data (impbfd)->import_file_id;
	    }
	}

      ldsym->l_parm = 0;

      BFD_ASSERT (h->ldindx >= 0);
      BFD_ASSERT (LDSYMSZ == sizeof (struct external_ldsym));
      xcoff_swap_ldsym_out (output_bfd, ldsym, finfo->ldsym + h->ldindx - 3);
      h->ldsym = NULL;
    }

  /* If this symbol needs global linkage code, write it out.  */
  if (h->root.type == bfd_link_hash_defined
      && (h->root.u.def.section
	  == xcoff_hash_table (finfo->info)->linkage_section))
    {
      bfd_byte *p;
      bfd_vma tocoff;
      unsigned int i;

      p = h->root.u.def.section->contents + h->root.u.def.value;

      /* The first instruction in the global linkage code loads a
         specific TOC element.  */
      tocoff = (h->descriptor->toc_section->output_section->vma
		+ h->descriptor->toc_section->output_offset
		- xcoff_data (output_bfd)->toc);
      if ((h->descriptor->flags & XCOFF_SET_TOC) != 0)
	tocoff += h->descriptor->u.toc_offset;
      bfd_put_32 (output_bfd, XCOFF_GLINK_FIRST | (tocoff & 0xffff), p);
      for (i = 0, p += 4;
	   i < sizeof xcoff_glink_code / sizeof xcoff_glink_code[0];
	   i++, p += 4)
	bfd_put_32 (output_bfd, xcoff_glink_code[i], p);
    }

  /* If we created a TOC entry for this symbol, write out the required
     relocs.  */
  if ((h->flags & XCOFF_SET_TOC) != 0)
    {
      asection *tocsec;
      asection *osec;
      int oindx;
      struct internal_reloc *irel;
      struct internal_ldrel ldrel;

      tocsec = h->toc_section;
      osec = tocsec->output_section;
      oindx = osec->target_index;
      irel = finfo->section_info[oindx].relocs + osec->reloc_count;
      irel->r_vaddr = (osec->vma
		       + tocsec->output_offset
		       + h->u.toc_offset);
      if (h->indx >= 0)
	irel->r_symndx = h->indx;
      else
	{
	  h->indx = -2;
	  irel->r_symndx = obj_raw_syment_count (output_bfd);
	}
      irel->r_type = R_POS;
      irel->r_size = 31;
      finfo->section_info[oindx].rel_hashes[osec->reloc_count] = NULL;
      ++osec->reloc_count;

      BFD_ASSERT (h->ldindx >= 0);
      ldrel.l_vaddr = irel->r_vaddr;
      ldrel.l_symndx = h->ldindx;
      ldrel.l_rtype = (31 << 8) | R_POS;
      ldrel.l_rsecnm = oindx;
      xcoff_swap_ldrel_out (output_bfd, &ldrel, finfo->ldrel);
      ++finfo->ldrel;
    }

  /* If this symbol is a specially defined function descriptor, write
     it out.  The first word is the address of the function code
     itself, the second word is the address of the TOC, and the third
     word is zero.  */
  if ((h->flags & XCOFF_DESCRIPTOR) != 0
      && h->root.type == bfd_link_hash_defined
      && (h->root.u.def.section
	  == xcoff_hash_table (finfo->info)->descriptor_section))
    {
      asection *sec;
      asection *osec;
      int oindx;
      bfd_byte *p;
      struct xcoff_link_hash_entry *hentry;
      asection *esec;
      struct internal_reloc *irel;
      struct internal_ldrel ldrel;
      asection *tsec;

      sec = h->root.u.def.section;
      osec = sec->output_section;
      oindx = osec->target_index;
      p = sec->contents + h->root.u.def.value;

      hentry = h->descriptor;
      BFD_ASSERT (hentry != NULL
		  && (hentry->root.type == bfd_link_hash_defined
		      || hentry->root.type == bfd_link_hash_defweak));
      esec = hentry->root.u.def.section;
      bfd_put_32 (output_bfd,
		  (esec->output_section->vma
		   + esec->output_offset
		   + hentry->root.u.def.value),
		  p);

      irel = finfo->section_info[oindx].relocs + osec->reloc_count;
      irel->r_vaddr = (osec->vma
		       + sec->output_offset
		       + h->root.u.def.value);
      irel->r_symndx = esec->output_section->target_index;
      irel->r_type = R_POS;
      irel->r_size = 31;
      finfo->section_info[oindx].rel_hashes[osec->reloc_count] = NULL;
      ++osec->reloc_count;

      ldrel.l_vaddr = irel->r_vaddr;
      if (strcmp (esec->output_section->name, ".text") == 0)
	ldrel.l_symndx = 0;
      else if (strcmp (esec->output_section->name, ".data") == 0)
	ldrel.l_symndx = 1;
      else if (strcmp (esec->output_section->name, ".bss") == 0)
	ldrel.l_symndx = 2;
      else
	{
	  (*_bfd_error_handler)
	    ("%s: loader reloc in unrecognized section `%s'",
	     bfd_get_filename (output_bfd),
	     esec->output_section->name);
	  bfd_set_error (bfd_error_nonrepresentable_section);
	  return false;
	}
      ldrel.l_rtype = (31 << 8) | R_POS;
      ldrel.l_rsecnm = oindx;
      xcoff_swap_ldrel_out (output_bfd, &ldrel, finfo->ldrel);
      ++finfo->ldrel;

      bfd_put_32 (output_bfd, xcoff_data (output_bfd)->toc, p + 4);

      tsec = coff_section_from_bfd_index (output_bfd,
					  xcoff_data (output_bfd)->sntoc);

      ++irel;
      irel->r_vaddr = (osec->vma
		       + sec->output_offset
		       + h->root.u.def.value
		       + 4);
      irel->r_symndx = tsec->output_section->target_index;
      irel->r_type = R_POS;
      irel->r_size = 31;
      finfo->section_info[oindx].rel_hashes[osec->reloc_count] = NULL;
      ++osec->reloc_count;

      ldrel.l_vaddr = irel->r_vaddr;
      if (strcmp (tsec->output_section->name, ".text") == 0)
	ldrel.l_symndx = 0;
      else if (strcmp (tsec->output_section->name, ".data") == 0)
	ldrel.l_symndx = 1;
      else if (strcmp (tsec->output_section->name, ".bss") == 0)
	ldrel.l_symndx = 2;
      else
	{
	  (*_bfd_error_handler)
	    ("%s: loader reloc in unrecognized section `%s'",
	     bfd_get_filename (output_bfd),
	     tsec->output_section->name);
	  bfd_set_error (bfd_error_nonrepresentable_section);
	  return false;
	}
      ldrel.l_rtype = (31 << 8) | R_POS;
      ldrel.l_rsecnm = oindx;
      xcoff_swap_ldrel_out (output_bfd, &ldrel, finfo->ldrel);
      ++finfo->ldrel;
    }

  if (h->indx >= 0)
    return true;

  if (h->indx != -2
      && (finfo->info->strip == strip_all
	  || (finfo->info->strip == strip_some
	      && (bfd_hash_lookup (finfo->info->keep_hash,
				   h->root.root.string, false, false)
		  == NULL))))
    return true;

  if (h->indx != -2
      && (h->flags & (XCOFF_REF_REGULAR | XCOFF_DEF_REGULAR)) == 0)
    return true;

  outsym = finfo->outsyms;

  memset (&aux, 0, sizeof aux);

  h->indx = obj_raw_syment_count (output_bfd);

  if (strlen (h->root.root.string) <= SYMNMLEN)
    strncpy (isym._n._n_name, h->root.root.string, SYMNMLEN);
  else
    {
      boolean hash;
      bfd_size_type indx;

      hash = true;
      if ((output_bfd->flags & BFD_TRADITIONAL_FORMAT) != 0)
	hash = false;
      indx = _bfd_stringtab_add (finfo->strtab, h->root.root.string, hash,
				 false);
      if (indx == (bfd_size_type) -1)
	return false;
      isym._n._n_n._n_zeroes = 0;
      isym._n._n_n._n_offset = STRING_SIZE_SIZE + indx;
    }

  if (h->root.type == bfd_link_hash_undefined
      || h->root.type == bfd_link_hash_undefweak)
    {
      isym.n_value = 0;
      isym.n_scnum = N_UNDEF;
      isym.n_sclass = C_EXT;
      aux.x_csect.x_smtyp = XTY_ER;
    }
  else if (h->root.type == bfd_link_hash_defined
	   || h->root.type == bfd_link_hash_defweak)
    {
      struct xcoff_link_size_list *l;

      isym.n_value = (h->root.u.def.section->output_section->vma
		      + h->root.u.def.section->output_offset
		      + h->root.u.def.value);
      isym.n_scnum = h->root.u.def.section->output_section->target_index;
      isym.n_sclass = C_HIDEXT;
      aux.x_csect.x_smtyp = XTY_SD;

      if ((h->flags & XCOFF_HAS_SIZE) != 0)
	{
	  for (l = xcoff_hash_table (finfo->info)->size_list;
	       l != NULL;
	       l = l->next)
	    {
	      if (l->h == h)
		{
		  aux.x_csect.x_scnlen.l = l->size;
		  break;
		}
	    }
	}
    }
  else if (h->root.type == bfd_link_hash_common)
    {
      isym.n_value = (h->root.u.c.p->section->output_section->vma
		      + h->root.u.c.p->section->output_offset);
      isym.n_scnum = h->root.u.c.p->section->output_section->target_index;
      isym.n_sclass = C_EXT;
      aux.x_csect.x_smtyp = XTY_CM;
      aux.x_csect.x_scnlen.l = h->root.u.c.size;
    }
  else
    abort ();

  isym.n_type = T_NULL;
  isym.n_numaux = 1;

  bfd_coff_swap_sym_out (output_bfd, (PTR) &isym, (PTR) outsym);
  outsym += bfd_coff_symesz (output_bfd);

  aux.x_csect.x_smclas = h->smclas;

  bfd_coff_swap_aux_out (output_bfd, (PTR) &aux, T_NULL, isym.n_sclass, 0, 1,
			 (PTR) outsym);
  outsym += bfd_coff_auxesz (output_bfd);

  if (h->root.type == bfd_link_hash_defined
      || h->root.type == bfd_link_hash_defweak)
    {
      /* We just output an SD symbol.  Now output an LD symbol.  */

      h->indx += 2;

      isym.n_sclass = C_EXT;
      bfd_coff_swap_sym_out (output_bfd, (PTR) &isym, (PTR) outsym);
      outsym += bfd_coff_symesz (output_bfd);

      aux.x_csect.x_smtyp = XTY_LD;
      aux.x_csect.x_scnlen.l = obj_raw_syment_count (output_bfd);

      bfd_coff_swap_aux_out (output_bfd, (PTR) &aux, T_NULL, C_EXT, 0, 1,
			     (PTR) outsym);
      outsym += bfd_coff_auxesz (output_bfd);
    }

  if (bfd_seek (output_bfd,
		(obj_sym_filepos (output_bfd)
		 + (obj_raw_syment_count (output_bfd)
		    * bfd_coff_symesz (output_bfd))),
		SEEK_SET) != 0
      || (bfd_write (finfo->outsyms, outsym - finfo->outsyms, 1, output_bfd)
	  != (bfd_size_type) (outsym - finfo->outsyms)))
    return false;
  obj_raw_syment_count (output_bfd) +=
    (outsym - finfo->outsyms) / bfd_coff_symesz (output_bfd);

  return true;
}

/* Handle a link order which is supposed to generate a reloc.  */

static boolean
xcoff_reloc_link_order (output_bfd, finfo, output_section, link_order)
     bfd *output_bfd;
     struct xcoff_final_link_info *finfo;
     asection *output_section;
     struct bfd_link_order *link_order;
{
  reloc_howto_type *howto;
  struct xcoff_link_hash_entry *h;
  asection *hsec;
  bfd_vma hval;
  bfd_vma addend;
  struct internal_reloc *irel;
  struct xcoff_link_hash_entry **rel_hash_ptr;
  struct internal_ldrel ldrel;

  if (link_order->type == bfd_section_reloc_link_order)
    {
      /* We need to somehow locate a symbol in the right section.  The
         symbol must either have a value of zero, or we must adjust
         the addend by the value of the symbol.  FIXME: Write this
         when we need it.  The old linker couldn't handle this anyhow.  */
      abort ();
    }

  howto = bfd_reloc_type_lookup (output_bfd, link_order->u.reloc.p->reloc);
  if (howto == NULL)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  h = ((struct xcoff_link_hash_entry *)
       bfd_wrapped_link_hash_lookup (output_bfd, finfo->info,
				     link_order->u.reloc.p->u.name,
				     false, false, true));
  if (h == NULL)
    {
      if (! ((*finfo->info->callbacks->unattached_reloc)
	     (finfo->info, link_order->u.reloc.p->u.name, (bfd *) NULL,
	      (asection *) NULL, (bfd_vma) 0)))
	return false;
      return true;
    }

  if (h->root.type == bfd_link_hash_common)
    {
      hsec = h->root.u.c.p->section;
      hval = 0;
    }
  else if (h->root.type == bfd_link_hash_defined
	   || h->root.type == bfd_link_hash_defweak)
    {
      hsec = h->root.u.def.section;
      hval = h->root.u.def.value;
    }
  else
    {
      hsec = NULL;
      hval = 0;
    }

  addend = link_order->u.reloc.p->addend;
  if (hsec != NULL)
    addend += (hsec->output_section->vma
	       + hsec->output_offset
	       + hval);

  if (addend != 0)
    {
      bfd_size_type size;
      bfd_byte *buf;
      bfd_reloc_status_type rstat;
      boolean ok;

      size = bfd_get_reloc_size (howto);
      buf = (bfd_byte *) bfd_zmalloc (size);
      if (buf == NULL)
	return false;

      rstat = _bfd_relocate_contents (howto, output_bfd, addend, buf);
      switch (rstat)
	{
	case bfd_reloc_ok:
	  break;
	default:
	case bfd_reloc_outofrange:
	  abort ();
	case bfd_reloc_overflow:
	  if (! ((*finfo->info->callbacks->reloc_overflow)
		 (finfo->info, link_order->u.reloc.p->u.name,
		  howto->name, addend, (bfd *) NULL, (asection *) NULL,
		  (bfd_vma) 0)))
	    {
	      free (buf);
	      return false;
	    }
	  break;
	}
      ok = bfd_set_section_contents (output_bfd, output_section, (PTR) buf,
				     (file_ptr) link_order->offset, size);
      free (buf);
      if (! ok)
	return false;
    }

  /* Store the reloc information in the right place.  It will get
     swapped and written out at the end of the final_link routine.  */

  irel = (finfo->section_info[output_section->target_index].relocs
	  + output_section->reloc_count);
  rel_hash_ptr = (finfo->section_info[output_section->target_index].rel_hashes
		  + output_section->reloc_count);

  memset (irel, 0, sizeof (struct internal_reloc));
  *rel_hash_ptr = NULL;

  irel->r_vaddr = output_section->vma + link_order->offset;

  if (h->indx >= 0)
    irel->r_symndx = h->indx;
  else
    {
      /* Set the index to -2 to force this symbol to get written out.  */
      h->indx = -2;
      *rel_hash_ptr = h;
      irel->r_symndx = 0;
    }

  irel->r_type = howto->type;
  irel->r_size = howto->bitsize - 1;
  if (howto->complain_on_overflow == complain_overflow_signed)
    irel->r_size |= 0x80;

  ++output_section->reloc_count;

  /* Now output the reloc to the .loader section.  */

  ldrel.l_vaddr = irel->r_vaddr;

  if (hsec != NULL)
    {
      const char *secname;

      secname = hsec->output_section->name;

      if (strcmp (secname, ".text") == 0)
	ldrel.l_symndx = 0;
      else if (strcmp (secname, ".data") == 0)
	ldrel.l_symndx = 1;
      else if (strcmp (secname, ".bss") == 0)
	ldrel.l_symndx = 2;
      else
	{
	  (*_bfd_error_handler)
	    ("%s: loader reloc in unrecognized section `%s'",
	     bfd_get_filename (output_bfd), secname);
	  bfd_set_error (bfd_error_nonrepresentable_section);
	  return false;
	}
    }
  else
    {
      if (h->ldindx < 0)
	{
	  (*_bfd_error_handler)
	    ("%s: `%s' in loader reloc but not loader sym",
	     bfd_get_filename (output_bfd),
	     h->root.root.string);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      ldrel.l_symndx = h->ldindx;
    }

  ldrel.l_rtype = (irel->r_size << 8) | irel->r_type;
  ldrel.l_rsecnm = output_section->target_index;
  xcoff_swap_ldrel_out (output_bfd, &ldrel, finfo->ldrel);
  ++finfo->ldrel;

  return true;
}

/* Sort relocs by VMA.  This is called via qsort.  */

static int
xcoff_sort_relocs (p1, p2)
     const PTR p1;
     const PTR p2;
{
  const struct internal_reloc *r1 = (const struct internal_reloc *) p1;
  const struct internal_reloc *r2 = (const struct internal_reloc *) p2;

  if (r1->r_vaddr > r2->r_vaddr)
    return 1;
  else if (r1->r_vaddr < r2->r_vaddr)
    return -1;
  else
    return 0;
}

/* This is the relocation function for the RS/6000/POWER/PowerPC.
   This is currently the only processor which uses XCOFF; I hope that
   will never change.  */

boolean
_bfd_ppc_xcoff_relocate_section (output_bfd, info, input_bfd,
				 input_section, contents, relocs, syms,
				 sections)
     bfd *output_bfd;
     struct bfd_link_info *info;
     bfd *input_bfd;
     asection *input_section;
     bfd_byte *contents;
     struct internal_reloc *relocs;
     struct internal_syment *syms;
     asection **sections;
{
  struct internal_reloc *rel;
  struct internal_reloc *relend;

  rel = relocs;
  relend = rel + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      long symndx;
      struct xcoff_link_hash_entry *h;
      struct internal_syment *sym;
      bfd_vma addend;
      bfd_vma val;
      struct reloc_howto_struct howto;
      bfd_reloc_status_type rstat;

      /* Relocation type R_REF is a special relocation type which is
         merely used to prevent garbage collection from occurring for
         the csect including the symbol which it references.  */
      if (rel->r_type == R_REF)
	continue;

      symndx = rel->r_symndx;

      if (symndx == -1)
	{
	  h = NULL;
	  sym = NULL;
	  addend = 0;
	}
      else
	{    
	  h = obj_xcoff_sym_hashes (input_bfd)[symndx];
	  sym = syms + symndx;
	  addend = - sym->n_value;
	}

      /* We build the howto information on the fly.  */

      howto.type = rel->r_type;
      howto.rightshift = 0;
      howto.size = 2;
      howto.bitsize = (rel->r_size & 0x1f) + 1;
      howto.pc_relative = false;
      howto.bitpos = 0;
      if ((rel->r_size & 0x80) != 0)
	howto.complain_on_overflow = complain_overflow_signed;
      else
	howto.complain_on_overflow = complain_overflow_bitfield;
      howto.special_function = NULL;
      howto.name = "internal";
      howto.partial_inplace = true;
      if (howto.bitsize == 32)
	howto.src_mask = howto.dst_mask = 0xffffffff;
      else
	{
	  howto.src_mask = howto.dst_mask = (1 << howto.bitsize) - 1;
	  if (howto.bitsize == 16)
	    howto.size = 1;
	}
      howto.pcrel_offset = false;

      val = 0;

      if (h == NULL)
	{
	  asection *sec;

	  if (symndx == -1)
	    {
	      sec = bfd_abs_section_ptr;
	      val = 0;
	    }
	  else
	    {
	      sec = sections[symndx];
	      /* Hack to make sure we use the right TOC anchor value
                 if this reloc is against the TOC anchor.  */
	      if (sec->name[3] == '0'
		  && strcmp (sec->name, ".tc0") == 0)
		val = xcoff_data (output_bfd)->toc;
	      else
		val = (sec->output_section->vma
		       + sec->output_offset
		       + sym->n_value
		       - sec->vma);
	    }
	}
      else
	{
	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    {
	      asection *sec;

	      sec = h->root.u.def.section;
	      val = (h->root.u.def.value
		     + sec->output_section->vma
		     + sec->output_offset);
	    }
	  else if (h->root.type == bfd_link_hash_common)
	    {
	      asection *sec;

	      sec = h->root.u.c.p->section;
	      val = (sec->output_section->vma
		     + sec->output_offset);
	    }
	  else if ((h->flags & XCOFF_DEF_DYNAMIC) != 0
		   || (h->flags & XCOFF_IMPORT) != 0)
	    {
	      /* Every symbol in a shared object is defined somewhere.  */
	      val = 0;
	    }
	  else if (! info->relocateable)
	    {
	      if (! ((*info->callbacks->undefined_symbol)
		     (info, h->root.root.string, input_bfd, input_section,
		      rel->r_vaddr - input_section->vma)))
		return false;
	    }
	}

      /* I took the relocation type definitions from two documents:
	 the PowerPC AIX Version 4 Application Binary Interface, First
	 Edition (April 1992), and the PowerOpen ABI, Big-Endian
	 32-Bit Hardware Implementation (June 30, 1994).  Differences
	 between the documents are noted below.  */

      switch (rel->r_type)
	{
	case R_RTB:
	case R_RRTBI:
	case R_RRTBA:
	  /* These relocs are defined by the PowerPC ABI to be
             relative branches which use half of the difference
             between the symbol and the program counter.  I can't
             quite figure out when this is useful.  These relocs are
             not defined by the PowerOpen ABI.  */
	default:
	  (*_bfd_error_handler)
	    ("%s: unsupported relocation type 0x%02x",
	     bfd_get_filename (input_bfd), (unsigned int) rel->r_type);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	case R_POS:
	  /* Simple positive relocation.  */
	  break;
	case R_NEG:
	  /* Simple negative relocation.  */
	  val = - val;
	  break;
	case R_REL:
	  /* Simple PC relative relocation.  */
	  howto.pc_relative = true;
	  break;
	case R_TOC:
	  /* TOC relative relocation.  The value in the instruction in
             the input file is the offset from the input file TOC to
             the desired location.  We want the offset from the final
             TOC to the desired location.  We have:
	         isym = iTOC + in
		 iinsn = in + o
		 osym = oTOC + on
		 oinsn = on + o
	     so we must change insn by on - in.
	     */
	case R_GL:
	  /* Global linkage relocation.  The value of this relocation
             is the address of the entry in the TOC section.  */
	case R_TCL:
	  /* Local object TOC address.  I can't figure out the
             difference between this and case R_GL.  */
	case R_TRL:
	  /* TOC relative relocation.  A TOC relative load instruction
             which may be changed to a load address instruction.
             FIXME: We don't currently implement this optimization.  */
	case R_TRLA:
	  /* TOC relative relocation.  This is a TOC relative load
             address instruction which may be changed to a load
             instruction.  FIXME: I don't know if this is the correct
             implementation.  */
	  if (h != NULL && h->toc_section == NULL)
	    {
	      (*_bfd_error_handler)
		("%s: TOC reloc at 0x%x to symbol `%s' with no TOC entry",
		 bfd_get_filename (input_bfd), rel->r_vaddr,
		 h->root.root.string);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  if (h != NULL)
	    {
	      BFD_ASSERT ((h->flags & XCOFF_SET_TOC) == 0);
	      val = (h->toc_section->output_section->vma
		     + h->toc_section->output_offset);
	    }
	  val = ((val - xcoff_data (output_bfd)->toc)
		 - (sym->n_value - xcoff_data (input_bfd)->toc));
	  addend = 0;
	  break;
	case R_BA:
	  /* Absolute branch.  We don't want to mess with the lower
             two bits of the instruction.  */
	case R_CAI:
	  /* The PowerPC ABI defines this as an absolute call which
             may be modified to become a relative call.  The PowerOpen
             ABI does not define this relocation type.  */
	case R_RBA:
	  /* Absolute branch which may be modified to become a
             relative branch.  */
	case R_RBAC:
	  /* The PowerPC ABI defines this as an absolute branch to a
             fixed address which may be modified to an absolute branch
             to a symbol.  The PowerOpen ABI does not define this
             relocation type.  */
	case R_RBRC:
	  /* The PowerPC ABI defines this as an absolute branch to a
             fixed address which may be modified to a relative branch.
             The PowerOpen ABI does not define this relocation type.  */
	  howto.src_mask &= ~3;
	  howto.dst_mask = howto.src_mask;
	  break;
	case R_BR:
	  /* Relative branch.  We don't want to mess with the lower
             two bits of the instruction.  */
	case R_CREL:
	  /* The PowerPC ABI defines this as a relative call which may
             be modified to become an absolute call.  The PowerOpen
             ABI does not define this relocation type.  */
	case R_RBR:
	  /* A relative branch which may be modified to become an
             absolute branch.  FIXME: We don't implement this,
             although we should for symbols of storage mapping class
             XMC_XO.  */
	  howto.pc_relative = true;
	  howto.src_mask &= ~3;
	  howto.dst_mask = howto.src_mask;
	  break;
	case R_RL:
	  /* The PowerPC AIX ABI describes this as a load which may be
             changed to a load address.  The PowerOpen ABI says this
             is the same as case R_POS.  */
	  break;
	case R_RLA:
	  /* The PowerPC AIX ABI describes this as a load address
             which may be changed to a load.  The PowerOpen ABI says
             this is the same as R_POS.  */
	  break;
	}

      /* If we see an R_BR or R_RBR reloc which is jumping to global
         linkage code, and it is followed by an appropriate cror nop
         instruction, we replace the cror with lwz r2,20(r1).  This
         restores the TOC after the glink code.  Contrariwise, if the
         call is followed by a lwz r2,20(r1), but the call is not
         going to global linkage code, we can replace the load with a
         cror.  */
      if ((rel->r_type == R_BR || rel->r_type == R_RBR)
	  && h != NULL
	  && h->root.type == bfd_link_hash_defined
	  && (rel->r_vaddr - input_section->vma + 8
	      <= input_section->_cooked_size))
	{
	  bfd_byte *pnext;
	  unsigned long next;

	  pnext = contents + (rel->r_vaddr - input_section->vma) + 4;
	  next = bfd_get_32 (input_bfd, pnext);

	  /* The _ptrgl function is magic.  It is used by the AIX
             compiler to call a function through a pointer.  */
	  if (h->smclas == XMC_GL
	      || strcmp (h->root.root.string, "._ptrgl") == 0)
	    {
	      if (next == 0x4def7b82		/* cror 15,15,15 */
		  || next == 0x4ffffb82)	/* cror 31,31,31 */
		bfd_put_32 (input_bfd, 0x80410014, pnext); /* lwz r1,20(r1) */
	    }
	  else
	    {
	      if (next == 0x80410014)		/* lwz r1,20(r1) */
		bfd_put_32 (input_bfd, 0x4ffffb82, pnext); /* cror 31,31,31 */
	    }
	}

      /* A PC relative reloc includes the section address.  */
      if (howto.pc_relative)
	addend += input_section->vma;

      rstat = _bfd_final_link_relocate (&howto, input_bfd, input_section,
					contents,
					rel->r_vaddr - input_section->vma,
					val, addend);

      switch (rstat)
	{
	default:
	  abort ();
	case bfd_reloc_ok:
	  break;
	case bfd_reloc_overflow:
	  {
	    const char *name;
	    char buf[SYMNMLEN + 1];
	    char howto_name[10];

	    if (symndx == -1)
	      name = "*ABS*";
	    else if (h != NULL)
	      name = h->root.root.string;
	    else
	      {
		name = _bfd_coff_internal_syment_name (input_bfd, sym, buf);
		if (name == NULL)
		  return false;
	      }
	    sprintf (howto_name, "0x%02x", rel->r_type);

	    if (! ((*info->callbacks->reloc_overflow)
		   (info, name, howto_name, (bfd_vma) 0, input_bfd,
		    input_section, rel->r_vaddr - input_section->vma)))
	      return false;
	  }
	}
    }

  return true;
}
