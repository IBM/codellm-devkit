/*
 * Keeps track of source files.
 */
#include "gprof.h"
#include "libiberty.h"
#include "search_list.h"
#include "source.h"

#define EXT_ANNO "-ann"		/* postfix of annotated files */

/*
 * Default option values:
 */
bool create_annotation_files = FALSE;

Search_List src_search_list =
{0, 0};
Source_File *first_src_file = 0;


Source_File *
DEFUN (source_file_lookup_path, (path), const char *path)
{
  Source_File *sf;

  for (sf = first_src_file; sf; sf = sf->next)
    {
      if (strcmp (path, sf->name) == 0)
	{
	  break;
	}
    }
  if (!sf)
    {
      /* create a new source file descriptor: */

      sf = (Source_File *) xmalloc (sizeof (*sf));
      memset (sf, 0, sizeof (*sf));
      sf->name = xstrdup (path);
      sf->next = first_src_file;
      first_src_file = sf;
    }
  return sf;
}


Source_File *
DEFUN (source_file_lookup_name, (filename), const char *filename)
{
  const char *fname;
  Source_File *sf;
  /*
   * The user cannot know exactly how a filename will be stored in
   * the debugging info (e.g., ../include/foo.h
   * vs. /usr/include/foo.h).  So we simply compare the filename
   * component of a path only:
   */
  for (sf = first_src_file; sf; sf = sf->next)
    {
      fname = strrchr (sf->name, '/');
      if (fname)
	{
	  ++fname;
	}
      else
	{
	  fname = sf->name;
	}
      if (strcmp (filename, fname) == 0)
	{
	  break;
	}
    }
  return sf;
}


FILE *
DEFUN (annotate_source, (sf, max_width, annote, arg),
       Source_File * sf AND int max_width
       AND void (*annote) PARAMS ((char *buf, int w, int l, void *arg))
       AND void *arg)
{
  static bool first_file = TRUE;
  int i, line_num, nread;
  bool new_line;
  char buf[8192];
  char fname[PATH_MAX];
  char *annotation, *name_only;
  FILE *ifp, *ofp;
  Search_List_Elem *sle = src_search_list.head;

  /*
   * Open input file.  If open fails, walk along search-list until
   * open succeeds or reaching end of list:
   */
  strcpy (fname, sf->name);
  if (sf->name[0] == '/')
    {
      sle = 0;			/* don't use search list for absolute paths */
    }
  name_only = 0;
  while (TRUE)
    {
      DBG (SRCDEBUG, printf ("[annotate_source]: looking for %s, trying %s\n",
			     sf->name, fname));
      ifp = fopen (fname, FOPEN_RB);
      if (ifp)
	{
	  break;
	}
      if (!sle && !name_only)
	{
	  name_only = strrchr (sf->name, '/');
	  if (name_only)
	    {
	      /* try search-list again, but this time with name only: */
	      ++name_only;
	      sle = src_search_list.head;
	    }
	}
      if (sle)
	{
	  strcpy (fname, sle->path);
	  strcat (fname, "/");
	  if (name_only)
	    {
	      strcat (fname, name_only);
	    }
	  else
	    {
	      strcat (fname, sf->name);
	    }
	  sle = sle->next;
	}
      else
	{
	  if (errno == ENOENT)
	    {
	      fprintf (stderr, "%s: could not locate `%s'\n",
		       whoami, sf->name);
	    }
	  else
	    {
	      perror (sf->name);
	    }
	  return 0;
	}
    }

  ofp = stdout;
  if (create_annotation_files)
    {
      /* try to create annotated source file: */
      const char *filename;

      /* create annotation files in the current working directory: */
      filename = strrchr (sf->name, '/');
      if (filename)
	{
	  ++filename;
	}
      else
	{
	  filename = sf->name;
	}

      strcpy (fname, filename);
      strcat (fname, EXT_ANNO);
      ofp = fopen (fname, "w");
      if (!ofp)
	{
	  perror (fname);
	  return 0;
	}
    }

  /*
   * Print file names if output goes to stdout and there are
   * more than one source file:
   */
  if (ofp == stdout)
    {
      if (first_file)
	{
	  first_file = FALSE;
	}
      else
	{
	  fputc ('\n', ofp);
	}
      if (first_output)
	{
	  first_output = FALSE;
	}
      else
	{
	  fprintf (ofp, "\f\n");
	}
      fprintf (ofp, "*** File %s:\n", sf->name);
    }

  annotation = xmalloc (max_width + 1);
  line_num = 1;
  new_line = TRUE;
  while ((nread = fread (buf, 1, sizeof (buf), ifp)) > 0)
    {
      for (i = 0; i < nread; ++i)
	{
	  if (new_line)
	    {
	      (*annote) (annotation, max_width, line_num, arg);
	      fputs (annotation, ofp);
	      ++line_num;
	      new_line = FALSE;
	    }
	  new_line = (buf[i] == '\n');
	  fputc (buf[i], ofp);
	}
    }
  free (annotation);
  return ofp;
}
