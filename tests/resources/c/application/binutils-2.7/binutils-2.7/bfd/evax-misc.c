/* evax-misc.c -- Miscellaneous functions for ALPHA EVAX (openVMS/AXP) files.
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


#if __STDC__
#include <stdarg.h>
#endif
#include <stdio.h>

#include "bfd.h"
#include "sysdep.h"
#include "bfdlink.h"
#include "libbfd.h"

#include "evax.h"

/*-----------------------------------------------------------------------------*/
#if EVAX_DEBUG
/* debug functions */

/* debug function for all evax extensions
   evaluates environment variable EVAX_DEBUG for a
   numerical value on the first call
   all error levels below this value are printed
  
   levels:
   1	toplevel bfd calls (functions from the bfd vector)
   2	functions called by bfd calls
   ...
   9	almost everything

   level is also identation level. Indentation is performed
   if level > 0
	*/

#if __STDC__
void
_bfd_evax_debug (int level, char *format, ...)
{
  static int min_level = -1;
  static FILE *output = NULL;
  char *eptr;
  va_list args;
  int abslvl = (level > 0)?level:-level;

  if (min_level == -1)
    {
      if ((eptr = getenv("EVAX_DEBUG")) != NULL)
	{
	  min_level = atoi(eptr);
	  output = stderr;
	}
      else
	min_level = 0;
    }
  if (output == NULL)
    return;
  if (abslvl > min_level)
    return;

  while(--level>0)
    fprintf(output, " ");
  va_start(args, format);
  vfprintf(output, format, args);
  fflush(output);
  va_end(args);

  return;
}

#else /* not __STDC__ */

void
_bfd_evax_debug (level, format, a1, a2, a3, a4, a5, a6)
     int level;
     char *format;
     long a1; long a2; long a3;
     long a4; long a5; long a6;
{
  static int min_level = -1;
  static FILE *output = NULL;
  char *eptr;

  if (min_level == -1)
    {
      if ((eptr = getenv("EVAX_DEBUG")) != NULL)
	{
	  min_level = atoi(eptr);
	  output = stderr;
	}
      else
	min_level = 0;
    }
  if (output == NULL)
    return;
  if (level > min_level)
    return;

  while(--level>0)
    fprintf(output, " ");
  fprintf(output, format, a1, a2, a3, a4, a5, a6);
  fflush(output);

  return;
}
#endif /* __STDC__ */


/* a debug function
   hex dump 'size' bytes starting at 'ptr'  */

void
_bfd_hexdump (level, ptr, size, offset)
     int level;
     unsigned char *ptr;
     int size;
     int offset;
{
  unsigned char *lptr = ptr;
  int count = 0;
  long start = offset;

  while (size-- > 0)
    {
      if ((count%16) == 0)
	evax_debug (level, "%08lx:", start);
      evax_debug (-level, " %02x", *ptr++);
      count++;
      start++;
      if (size == 0)
	{
	  while ((count%16) != 0)
	    {
	      evax_debug (-level, "   ");
	      count++;
	    }
	}
      if ((count%16) == 0)
	{
	  evax_debug (-level, " ");
	  while (lptr < ptr)
	    {
	      evax_debug (-level, "%c", (*lptr < 32)?'.':*lptr);
	      lptr++;
	    }
	  evax_debug (-level, "\n");
	}
    }
  if ((count%16) != 0)
    evax_debug (-level, "\n");

  return;
}
#endif


/* hash functions

   These are needed when reading an object file.  */

/* allocate new evax_hash_entry
   keep the symbol name and a pointer to the bfd symbol in the table  */

struct bfd_hash_entry *
_bfd_evax_hash_newfunc (entry, table, string)
     struct bfd_hash_entry *entry;
     struct bfd_hash_table *table;
     const char *string;
{
  evax_symbol_entry *ret = (evax_symbol_entry *)entry;

#if EVAX_DEBUG
  evax_debug (5, "_bfd_evax_hash_newfunc(%p, %p, %s)\n", entry, table, string);
#endif

  if (ret == (evax_symbol_entry *)NULL)
    ret = ((evax_symbol_entry *) bfd_hash_allocate (table, sizeof (evax_symbol_entry)));
  if (ret == (evax_symbol_entry *)NULL)
    {
      bfd_set_error (bfd_error_no_memory);
      return (struct bfd_hash_entry *)NULL;
    }
  ret = (evax_symbol_entry *) bfd_hash_newfunc ((struct bfd_hash_entry *)ret, table, string);

  ret->symbol = (asymbol *)NULL;

  return (struct bfd_hash_entry *)ret;
}


/* object file input functions */

/* Return type and length from record header (buf)  */

void
_bfd_evax_get_header_values (abfd, buf, type, length)
     bfd *abfd;
     unsigned char *buf;
     int *type;
     int *length;
{
  if (type != 0)
    *type = bfd_getl16 (buf);
  buf += 2;
  if (length != 0)
    *length = bfd_getl16 (buf);

  return;
}


/* Get next record from object file to evax_buf
   set PRIV(buf_size) and return it
  
   this is a little tricky since it should be portable.
  
   the openVMS/AXP object file has 'variable length' which means that
   read() returns data in chunks of (hopefully) correct and expected
   size. The linker (and other tools on vms) depend on that. Unix doesn't
   know about 'formatted' files, so reading and writing such an object
   file in a unix environment is not trivial.
  
   With the tool 'file' (available on all vms ftp sites), one
   can view and change the attributes of a file. Changing from
   'variable length' to 'fixed length, 512 bytes' reveals the
   record length at the first 2 bytes of every record. The same
   happens during the transfer of object files from vms to unix,
   at least with ucx, dec's implementation of tcp/ip.
  
   The EVAX format repeats the length at bytes 2 & 3 of every record.
  
   On the first call (file_format == FF_UNKNOWN) we check if
   the first and the third byte pair (!) of the record match.
   If they do it's an object file in an unix environment or with
   wrong attributes (FF_FOREIGN), else we should be in a vms
   environment where read() returns the record size (FF_NATIVE).
  
   reading is always done in 2 steps.
   first just the record header is read and the length extracted
   by get_header_values
   then the read buffer is adjusted and the remaining bytes are
   read in.
  
   all file i/o is always done on even file positions  */

int
_bfd_evax_get_record (abfd)
     bfd *abfd;
{
  int test_len, test_start, remaining;
  unsigned char *evax_buf;

#if EVAX_DEBUG
  evax_debug (8, "_bfd_evax_get_record\n");
#endif

  /* minimum is 6 bytes
     (2 bytes length, 2 bytes record id, 2 bytes length repeated)  */

  if (PRIV(buf_size) == 0)
    {
      PRIV(evax_buf) = (unsigned char *) malloc (6);
#if EVAX_DEBUG
      evax_debug (9, "PRIV(evax_buf) %p\n", PRIV(evax_buf));
#endif
    }

  evax_buf = PRIV(evax_buf);

  if (evax_buf == 0)
    {
#if EVAX_DEBUG
      evax_debug (9, "can't alloc evax_buf\n");
#endif
      bfd_set_error (bfd_error_no_memory);
      return -1;
    }

  switch (PRIV(file_format))
    {
      case FF_UNKNOWN:
      case FF_FOREIGN:
	test_len = 6;				/* probe 6 bytes */
	test_start = 2;				/* where the record starts */
      break;

      case FF_NATIVE:
	test_len = 4;
	test_start = 0;
      break;
  }

  /* skip odd alignment byte  */
#if 0
  if (PRIV(file_format) == FF_FOREIGN)
    {
#endif
      if (bfd_tell (abfd) & 1)
	{
#if EVAX_DEBUG
	  evax_debug (10, "skip odd\n");
#endif
	  if (bfd_read (PRIV(evax_buf), 1, 1, abfd) != 1)
	    {
#if EVAX_DEBUG
	      evax_debug (9, "skip odd failed\n");
#endif
	      bfd_set_error (bfd_error_file_truncated);
	      return 0;
	    }
	}
#if 0
    }
#endif
  /* read the record header  */

  if (bfd_read (PRIV(evax_buf), 1, test_len, abfd) != test_len)
    {
#if EVAX_DEBUG
      evax_debug (9, "can't bfd_read test %d bytes\n", test_len);
#endif
      bfd_set_error (bfd_error_file_truncated);
      return 0;
    }

  /* check file format on first call  */

  if (PRIV(file_format) == FF_UNKNOWN)
    {						/* record length repeats ? */
      if ( (evax_buf[0] == evax_buf[4])
        && (evax_buf[1] == evax_buf[5]))
	{
	  PRIV(file_format) = FF_FOREIGN;	/* Y: foreign environment */
	  test_start = 2;
	}
      else
	{
	  PRIV(file_format) = FF_NATIVE;	/* N: native environment */
	  test_start = 0;
	}
#if EVAX_DEBUG
      evax_debug (10, "File format is %s\n", (PRIV(file_format) == FF_FOREIGN)?"foreign":"native");
#endif
    }

  /* extract evax record length  */

  _bfd_evax_get_header_values (abfd, evax_buf+test_start, NULL,
			       &PRIV(rec_length));

  if (PRIV(rec_length) <= 0)
    {
      bfd_set_error (bfd_error_file_truncated);
      return 0;
    }

  /* that's what the linker manual says  */

  if (PRIV(rec_length) > EOBJ_S_C_MAXRECSIZ)
    {
      bfd_set_error (bfd_error_file_truncated);
      return 0;
    }

  /* adjust the buffer  */

  if (PRIV(rec_length) > PRIV(buf_size))
    {
      PRIV(evax_buf) = (unsigned char *) realloc (evax_buf, PRIV(rec_length));
#if EVAX_DEBUG
      evax_debug (3, "adjusted the buffer (%p) from %d to %d\n", PRIV(evax_buf), PRIV(buf_size), PRIV(rec_length));
#endif
      evax_buf = PRIV(evax_buf);
      if (evax_buf == 0)
	{
#if EVAX_DEBUG
	  evax_debug (9, "can't realloc evax_buf to %d bytes\n", PRIV(rec_length));
#endif
	  bfd_set_error (bfd_error_no_memory);
	  return -1;
	}
      PRIV(buf_size) = PRIV(rec_length);
    }

  /* read the remaining record  */

  remaining = PRIV(rec_length) - test_len + test_start;

  if (bfd_read (evax_buf + test_len, 1, remaining, abfd) != remaining)
    {
#if EVAX_DEBUG
      evax_debug (9, "can't bfd_read remaining %d bytes\n", remaining);
#endif
      bfd_set_error (bfd_error_file_truncated);
      return 0;
    }

  PRIV(evax_rec) = evax_buf + test_start;

  return PRIV(rec_length);
}


/* get next EVAX record from file
   update evax_rec and rec_length to new (remaining) values  */

int
_bfd_evax_next_record (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (8, "_bfd_evax_next_record (len %d, size %d)\n",
	      PRIV(rec_length), PRIV(rec_size));
#endif

  if (PRIV(rec_length) > 0)
    {
      PRIV(evax_rec) += PRIV(rec_size);
    }
  else
    {
      if (_bfd_evax_get_record (abfd) <= 0)
	return -1;
    }
  _bfd_evax_get_header_values (abfd, PRIV(evax_rec), &PRIV(rec_type),
			       &PRIV(rec_size));
  PRIV(rec_length) -= PRIV(rec_size);

#if EVAX_DEBUG
  evax_debug (8, "_bfd_evax_next_record: rec %p, size %d, length %d, type %d\n",
	      PRIV(evax_rec), PRIV(rec_size),	PRIV(rec_length),
	      PRIV(rec_type));
#endif

  return PRIV(rec_type);
}



/* Copy sized string (string with fixed length) to new allocated area
   size is string length (size of record)  */

char *
_bfd_evax_save_sized_string (str, size)
     char *str;
     int size;
{
  char *newstr = bfd_malloc (size + 1);

  if (newstr == NULL)
    return 0;
  strncpy (newstr, str, size);
  newstr[size] = 0;

  return newstr;
}

/* Copy counted string (string with length at first byte) to new allocated area
   ptr points to length byte on entry  */

char *
_bfd_evax_save_counted_string (ptr)
     char *ptr;
{
  int len = *ptr++;

  return _bfd_evax_save_sized_string (ptr, len);
}


/* stack routines for EVAX ETIR commands */

/* Push value and section index  */

void
_bfd_evax_push (abfd, val, psect)
     bfd *abfd;
     uquad val;
     int psect;
{
  static int last_psect;

#if EVAX_DEBUG
  evax_debug (4, "<push %016lx(%d) at %d>\n", val, psect, PRIV(stackptr));
#endif

  if (psect >= 0)
    last_psect = psect;

  PRIV(stack[PRIV(stackptr)]).value = val;
  PRIV(stack[PRIV(stackptr)]).psect = last_psect;
  PRIV(stackptr)++;
  if (PRIV(stackptr) >= STACKSIZE)
    {
      bfd_set_error (bfd_error_bad_value);
      exit(1);
    }
  return;
}


/* Pop value and section index  */

uquad
_bfd_evax_pop (abfd, psect)
     bfd *abfd;
     int *psect;
{
  uquad value;

  if (PRIV(stackptr) == 0)
    {
      bfd_set_error (bfd_error_bad_value);
      exit(1);
    }
  PRIV(stackptr)--;
  value = PRIV(stack[PRIV(stackptr)]).value;
  if ((psect != NULL) && (PRIV(stack[PRIV(stackptr)]).psect >= 0))
    *psect = PRIV(stack[PRIV(stackptr)]).psect;

#if EVAX_DEBUG
  evax_debug (4, "<pop %016lx(%d)>\n", value, PRIV(stack[PRIV(stackptr)]).psect);
#endif

  return value;
}


/* object file output functions */

/* GAS tends to write sections in little chunks (bfd_set_section_contents)
   which we can't use directly. So we save the little chunks in linked
   lists (one per section) and write them later.  */

/* Add a new evax_section structure to evax_section_table
   - forward chaining -  */

static evax_section *
add_new_contents (abfd, section)
     bfd *abfd;
     sec_ptr section;
{
  evax_section *sptr, *newptr;

  newptr = (evax_section *) bfd_zalloc (abfd, sizeof (evax_section));
  if (newptr == (evax_section *)NULL)
    {
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }
  sptr = PRIV(evax_section_table)[section->index];
  if (sptr == NULL)
    {
      PRIV(evax_section_table)[section->index] = (evax_section *)newptr;
    }
  else
    {
      while (sptr->next != NULL)
	sptr = sptr->next;
      sptr->next = newptr;
    }
  return newptr;
}


/* Save section data & offset to an evax_section structure
   evax_section_table[] holds the evax_section chain  */

boolean
_bfd_save_evax_section (abfd, section, data, offset, count)
     bfd *abfd;
     sec_ptr section;
     PTR data;
     file_ptr offset;
     bfd_size_type count;
{
  evax_section *sptr;

  if (section->index >= EVAX_SECTION_COUNT)
    {
      bfd_set_error (bfd_error_nonrepresentable_section);
      return false;
    }
  if (count == (bfd_size_type)0)
    return true;
  sptr = add_new_contents (abfd, section);
  if (sptr == NULL)
    return false;
  sptr->contents = (unsigned char *) bfd_alloc (abfd, (int)count);
  if (sptr->contents == (unsigned char *)NULL)
    {
      bfd_set_error (bfd_error_no_memory);
      return false;
    }
  memcpy (sptr->contents, data, (int)count);
  sptr->offset = (bfd_vma)offset;
  sptr->size = count;

#if EVAX_DEBUG
  evax_debug (6, "_bfd_save_evax_section sptr = %08lx\n", sptr);
  _bfd_hexdump (6, data, count, (int)offset);
#endif

  return true;
}


/* Get evax_section pointer to saved contents for section # index  */

evax_section *
_bfd_get_evax_section (abfd, index)
     bfd *abfd;
     int index;
{
  if (index >=  EVAX_SECTION_COUNT)
    {
      bfd_set_error (bfd_error_nonrepresentable_section);
      return NULL;
    }
  return PRIV(evax_section_table)[index];
}


/* Object output routines  */

/* Begin new record or record header
   write 2 bytes rectype
   write 2 bytes record length (filled in at flush)
   write 2 bytes header type (ommitted if rechead == -1)  */

void
_bfd_evax_output_begin (abfd, rectype, rechead)
     bfd *abfd;
     int rectype;
     int rechead;
{
#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_begin(type %d, head %d)\n", rectype,
	      rechead);
#endif

  _bfd_evax_output_short (abfd,rectype);

  /* save current output position to fill in lenght later  */

  if (PRIV(push_level) > 0)
    PRIV(length_pos) = PRIV(output_size);

#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_begin: length_pos = %d\n",
	      PRIV(length_pos));
#endif

  _bfd_evax_output_short (abfd,0);		/* placeholder for length */

  if (rechead != -1)
    _bfd_evax_output_short (abfd,rechead);

  return;
}


/* Set record/subrecord alignment  */

void
_bfd_evax_output_alignment (abfd, alignto)
     bfd *abfd;
     int alignto;
{
#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_alignment(%d)\n", alignto);
#endif

  PRIV(output_alignment) = alignto;
  return;
}


/* Prepare for subrecord fields  */

void
_bfd_evax_output_push (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (6, "evax_output_push(pushed_size = %d)\n", PRIV(output_size));
#endif

  PRIV(push_level)++;
  PRIV(pushed_size) = PRIV(output_size);
  return;
}


/* End of subrecord fields  */

void
_bfd_evax_output_pop (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (6, "evax_output_pop(pushed_size = %d)\n", PRIV(pushed_size));
#endif

  _bfd_evax_output_flush (abfd);
  PRIV(length_pos) = 2;

#if EVAX_DEBUG
  evax_debug (6, "evax_output_pop: length_pos = %d\n", PRIV(length_pos));
#endif

  PRIV(pushed_size) = 0;
  PRIV(push_level)--;
  return;
}


/* Flush unwritten output, ends current record  */

void
_bfd_evax_output_flush (abfd)
     bfd *abfd;
{
  int real_size = PRIV(output_size);
  int aligncount;
  int length;

#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_flush(real_size = %d, pushed_size %d at lenpos %d)\n",
	      real_size, PRIV(pushed_size), PRIV(length_pos));
#endif

  if (PRIV(push_level) > 0)
    length = real_size - PRIV(pushed_size);
  else
    length = real_size;

  if (length == 0)
    return;
  aligncount = (PRIV(output_alignment)
		- (length % PRIV(output_alignment))) % PRIV(output_alignment);

#if EVAX_DEBUG
  evax_debug (6, "align: adding %d bytes\n", aligncount);
#endif

  while(aligncount-- > 0)
    {
      PRIV(output_buf)[real_size++] = 0;
#if 0
      /* this is why I *love* vms: inconsistency :-}
	 alignment is added to the subrecord length
	 but not to the record length  */
      if (PRIV(push_level) > 0)
#endif
	length++;
    }

  /* put length to buffer  */
  PRIV(output_size) = PRIV(length_pos);
  _bfd_evax_output_short (abfd, (unsigned int)length);

  if (PRIV(push_level) == 0)
    {
#ifndef VMS
	/* write length first, see FF_FOREIGN in the input routines */
      fwrite (PRIV(output_buf)+2, 2, 1, (FILE *)abfd->iostream);
#endif
      fwrite (PRIV(output_buf), real_size, 1, (FILE *)abfd->iostream);

      PRIV(output_size) = 0;
    }
  else
    {
      PRIV(output_size) = real_size;
      PRIV(pushed_size) = PRIV(output_size);
    }

  return;
}


/* End record output  */

void
_bfd_evax_output_end (abfd)
     bfd *abfd;
{
#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_end\n");
#endif

  _bfd_evax_output_flush (abfd);

  return;
}


/* check remaining buffer size

   return what's left.  */

int
_bfd_evax_output_check (abfd, size)
    bfd *abfd;
    int size;
{
#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_check(%d)\n", size);
#endif

  return (MAX_OUTREC_SIZE - (PRIV(output_size) + size + MIN_OUTREC_LUFT));
}


/* Output byte (8 bit) value  */

void
_bfd_evax_output_byte (abfd, value)
     bfd *abfd;
     unsigned int value;
{
#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_byte(%02x)\n", value);
#endif

  bfd_put_8 (abfd, value & 0xff, PRIV(output_buf) + PRIV(output_size));
  PRIV(output_size) += 1;
  return;
}


/* Output short (16 bit) value  */

void
_bfd_evax_output_short (abfd, value)
     bfd *abfd;
     unsigned int value;
{
#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_short (%04x)\n", value);
#endif

  bfd_put_16 (abfd, value & 0xffff, PRIV(output_buf) + PRIV(output_size));
  PRIV(output_size) += 2;
  return;
}


/* Output long (32 bit) value  */

void
_bfd_evax_output_long (abfd, value)
     bfd *abfd;
     unsigned long value;
{
#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_long (%08lx)\n", value);
#endif

  bfd_put_32 (abfd, value, PRIV(output_buf) + PRIV(output_size));
  PRIV(output_size) += 4;
  return;
}


/* Output quad (64 bit) value  */

void
_bfd_evax_output_quad (abfd, value)
     bfd *abfd;
     uquad value;
{
#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_quad(%016lx)\n", value);
#endif

  bfd_put_64(abfd, value, PRIV(output_buf) + PRIV(output_size));
  PRIV(output_size) += 8;
  return;
}


/* Output c-string as counted string  */

void
_bfd_evax_output_counted (abfd, value)
     bfd *abfd;
     char *value;
{
int len;

#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_counted(%s)\n", value);
#endif

  len = strlen (value);
  if (len == 0)
    {
      (*_bfd_error_handler) ("_bfd_evax_output_counted called with zero bytes");
      return;
    }
  if (len > 255)
    {
      (*_bfd_error_handler) ("_bfd_evax_output_counted called with too many bytes");
      return;
    }
  _bfd_evax_output_byte (abfd, len & 0xff);
  _bfd_evax_output_dump (abfd, (unsigned char *)value, len);
}


/* Output character area  */

void
_bfd_evax_output_dump (abfd, data, length)
     bfd *abfd;
     unsigned char *data;
     int length;
{
#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_dump(%d)\n", length);
#endif

  if (length == 0)
    return;

  memcpy (PRIV(output_buf) + PRIV(output_size), data, length);
  PRIV(output_size) += length;

  return;
}


/* Output count bytes of value  */

void
_bfd_evax_output_fill (abfd, value, count)
     bfd *abfd;
     int value;
     int count;
{
#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_output_fill(val %02x times %d)\n", value, count);
#endif

  if (count == 0)
    return;
  memset (PRIV(output_buf) + PRIV(output_size), value, count);
  PRIV(output_size) += count;

  return;
}

/*-----------------------------------------------------------------------------*/

/* Return basename (stripped of directory information) of filename  */

char *
_bfd_evax_basename (name)
     char *name;
{
  char *ptr;

#if EVAX_DEBUG
  evax_debug (6, "_bfd_evax_basename %s -> ", name);
#endif

#ifndef VMS
  /* assume unix host */
  ptr = strrchr (name, '.');
  if (ptr)
    *ptr = 0;
  ptr = strrchr (name, '/');
  if (ptr)
    *ptr++ = 0;
  else
    ptr = name;
#else
  /* assume vms host */
  ptr = strrchr (name, '.');
  if (ptr)
    {
      *ptr = 0;
      ptr = name;
    }
  else
    {
      ptr = strrchr (name, ']');
      if (ptr)
	*ptr++ = 0;
      else
	{
	  ptr = strrchr (name, ':');
	  if (ptr)
	    *ptr++ = 0;
	  else
	    ptr = name;
	}
    }
#endif

#if EVAX_DEBUG
  evax_debug (6, "%s\n", ptr);
#endif

  return ptr;
}


/* Manufacure a VMS like time on a unix based system.
   stolen from obj-vms.c  */

char *
_bfd_get_vms_time_string ()
{
  static char tbuf[18];
#ifndef VMS
#include <sys/types.h>
#include <time.h>

  char *pnt;
  time_t timeb;
  time (&timeb);
  pnt = ctime (&timeb);
  pnt[3] = 0;
  pnt[7] = 0;
  pnt[10] = 0;
  pnt[16] = 0;
  pnt[24] = 0;
  sprintf (tbuf, "%2s-%3s-%s %s", pnt + 8, pnt + 4, pnt + 20, pnt + 11);
#else
#include <starlet.h>
  struct
  {
    int Size;
    char *Ptr;
  } Descriptor;
  Descriptor.Size = 17;
  Descriptor.Ptr = tbuf;
  sys$asctim (0, &Descriptor, 0, 0);
#endif /* not VMS */

#if EVAX_DEBUG
  evax_debug (6, "vmstimestring:'%s'\n", tbuf);
#endif

  return tbuf;
}
