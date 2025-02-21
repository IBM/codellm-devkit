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
#include "getopt.h"
#include "libiberty.h"
#include "gprof.h"
#include "basic_blocks.h"
#include "call_graph.h"
#include "cg_arcs.h"
#include "cg_print.h"
#include "core.h"
#include "gmon_io.h"
#include "hertz.h"
#include "hist.h"
#include "source.h"
#include "sym_ids.h"

#define VERSION "2.7"

const char *whoami;
const char *function_mapping_file;
const char *a_out_name = A_OUTNAME;
long hz = HZ_WRONG;

/*
 * Default options values:
 */
int debug_level = 0;
int output_style = 0;
int output_width = 80;
bool bsd_style_output = FALSE;
bool discard_underscores = TRUE;
bool ignore_direct_calls = FALSE;
bool ignore_static_funcs = FALSE;
bool ignore_zeros = TRUE;
bool line_granularity = FALSE;
bool print_descriptions = TRUE;
bool print_path = FALSE;
bool ignore_non_functions = FALSE;
File_Format file_format = FF_AUTO;

bool first_output = TRUE;

char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";

static char *gmon_name = GMONNAME;	/* profile filename */

bfd *abfd;

/*
 * Functions that get excluded by default:
 */
static char *default_excluded_list[] =
{
  "_gprof_mcount", "mcount", "_mcount", "__mcount", "__mcleanup",
  "<locore>", "<hicore>",
  0
};

static struct option long_options[] =
{
  {"line", no_argument, 0, 'l'},
  {"no-static", no_argument, 0, 'a'},
  {"ignore-non-functions", no_argument, 0, 'D'},

    /* output styles: */

  {"annotated-source", optional_argument, 0, 'A'},
  {"no-annotated-source", optional_argument, 0, 'J'},
  {"flat-profile", optional_argument, 0, 'p'},
  {"no-flat-profile", optional_argument, 0, 'P'},
  {"graph", optional_argument, 0, 'q'},
  {"no-graph", optional_argument, 0, 'Q'},
  {"exec-counts", optional_argument, 0, 'C'},
  {"no-exec-counts", optional_argument, 0, 'Z'},
  {"function-ordering", no_argument, 0, 'r'},
  {"file-ordering", required_argument, 0, 'R'},
  {"file-info", no_argument, 0, 'i'},
  {"sum", no_argument, 0, 's'},

    /* various options to affect output: */

  {"all-lines", no_argument, 0, 'x'},
  {"directory-path", required_argument, 0, 'I'},
  {"display-unused-functions", no_argument, 0, 'z'},
  {"min-count", required_argument, 0, 'm'},
  {"print-path", no_argument, 0, 'L'},
  {"separate-files", no_argument, 0, 'y'},
  {"static-call-graph", no_argument, 0, 'c'},
  {"table-length", required_argument, 0, 't'},
  {"time", required_argument, 0, 'n'},
  {"no-time", required_argument, 0, 'N'},
  {"width", required_argument, 0, 'w'},
    /*
     * These are for backwards-compatibility only.  Their functionality
     * is provided by the output style options already:
     */
  {"", required_argument, 0, 'e'},
  {"", required_argument, 0, 'E'},
  {"", required_argument, 0, 'f'},
  {"", required_argument, 0, 'F'},
  {"", required_argument, 0, 'k'},

    /* miscellaneous: */

  {"brief", no_argument, 0, 'b'},
  {"debug", optional_argument, 0, 'd'},
  {"help", no_argument, 0, 'h'},
  {"file-format", required_argument, 0, 'O'},
  {"traditional", no_argument, 0, 'T'},
  {"version", no_argument, 0, 'v'},
  {0, no_argument, 0, 0}
};


static void
DEFUN (usage, (stream, status), FILE * stream AND int status)
{
  fprintf (stream, "\
Usage: %s [-[abcDhilLsTvwxyz]] [-[ACeEfFJnNOpPqQZ][name]] [-I dirs]\n\
	[-d[num]] [-k from/to] [-m min-count] [-t table-length]\n\
	[--[no-]annotated-source[=name]] [--[no-]exec-counts[=name]]\n\
	[--[no-]flat-profile[=name]] [--[no-]graph[=name]]\n\
	[--[no-]time=name] [--all-lines] [--brief] [--debug[=level]]\n\
	[--function-ordering] [--file-ordering]\n\
	[--directory-path=dirs] [--display-unused-functions]\n\
	[--file-format=name] [--file-info] [--help] [--line] [--min-count=n]\n\
	[--no-static] [--print-path] [--separate-files]\n\
	[--static-call-graph] [--sum] [--table-length=len] [--traditional]\n\
	[--version] [--width=n] [--ignore-non-functions]\n\
	[image-file] [profile-file...]\n",
	   whoami);
  done (status);
}


int
DEFUN (main, (argc, argv), int argc AND char **argv)
{
  char **sp, *str;
  Sym **cg = 0;
  int ch, user_specified = 0;

  whoami = argv[0];
  xmalloc_set_program_name (whoami);

  while ((ch = getopt_long (argc, argv,
	"aA::bBcCdD::e:E:f:F:hiI:J::k:lLm:n::N::O:p::P::q::Q::st:Tvw:xyzZ::",
			    long_options, 0))
	 != EOF)
    {
      switch (ch)
	{
	case 'a':
	  ignore_static_funcs = TRUE;
	  break;
	case 'A':
	  if (optarg)
	    {
	      sym_id_add (optarg, INCL_ANNO);
	    }
	  output_style |= STYLE_ANNOTATED_SOURCE;
	  user_specified |= STYLE_ANNOTATED_SOURCE;
	  break;
	case 'b':
	  print_descriptions = FALSE;
	  break;
	case 'B':
	  output_style |= STYLE_CALL_GRAPH;
	  user_specified |= STYLE_CALL_GRAPH;
	  break;
	case 'c':
	  ignore_direct_calls = TRUE;
	  break;
	case 'C':
	  if (optarg)
	    {
	      sym_id_add (optarg, INCL_EXEC);
	    }
	  output_style |= STYLE_EXEC_COUNTS;
	  user_specified |= STYLE_EXEC_COUNTS;
	  break;
	case 'd':
	  if (optarg)
	    {
	      debug_level |= atoi (optarg);
	      debug_level |= ANYDEBUG;
	    }
	  else
	    {
	      debug_level = ~0;
	    }
	  DBG (ANYDEBUG, printf ("[main] debug-level=0x%x\n", debug_level));
#ifndef DEBUG
	  printf ("%s: debugging not supported; -d ignored\n", whoami);
#endif	/* DEBUG */
	  break;
	case 'D':
	  ignore_non_functions = TRUE;
	  break;
	case 'E':
	  sym_id_add (optarg, EXCL_TIME);
	case 'e':
	  sym_id_add (optarg, EXCL_GRAPH);
	  break;
	case 'F':
	  sym_id_add (optarg, INCL_TIME);
	case 'f':
	  sym_id_add (optarg, INCL_GRAPH);
	  break;
	case 'g':
	  sym_id_add (optarg, EXCL_FLAT);
	  break;
	case 'G':
	  sym_id_add (optarg, INCL_FLAT);
	  break;
	case 'h':
	  usage (stdout, 0);
	case 'i':
	  output_style |= STYLE_GMON_INFO;
	  user_specified |= STYLE_GMON_INFO;
	  break;
	case 'I':
	  search_list_append (&src_search_list, optarg);
	  break;
	case 'J':
	  if (optarg)
	    {
	      sym_id_add (optarg, EXCL_ANNO);
	      output_style |= STYLE_ANNOTATED_SOURCE;
	    }
	  else
	    {
	      output_style &= ~STYLE_ANNOTATED_SOURCE;
	    }
	  user_specified |= STYLE_ANNOTATED_SOURCE;
	  break;
	case 'k':
	  sym_id_add (optarg, EXCL_ARCS);
	  break;
	case 'l':
	  line_granularity = TRUE;
	  break;
	case 'L':
	  print_path = TRUE;
	  break;
	case 'm':
	  bb_min_calls = atoi (optarg);
	  break;
	case 'n':
	  sym_id_add (optarg, INCL_TIME);
	  break;
	case 'N':
	  sym_id_add (optarg, EXCL_TIME);
	  break;
	case 'O':
	  switch (optarg[0])
	    {
	    case 'a':
	      file_format = FF_AUTO;
	      break;
	    case 'm':
	      file_format = FF_MAGIC;
	      break;
	    case 'b':
	      file_format = FF_BSD;
	      break;
	    case 'p':
	      file_format = FF_PROF;
	      break;
	    default:
	      fprintf (stderr, "%s: unknown file format %s\n",
		       optarg, whoami);
	      done (1);
	    }
	  break;
	case 'p':
	  if (optarg)
	    {
	      sym_id_add (optarg, INCL_FLAT);
	    }
	  output_style |= STYLE_FLAT_PROFILE;
	  user_specified |= STYLE_FLAT_PROFILE;
	  break;
	case 'P':
	  if (optarg)
	    {
	      sym_id_add (optarg, EXCL_FLAT);
	      output_style |= STYLE_FLAT_PROFILE;
	    }
	  else
	    {
	      output_style &= ~STYLE_FLAT_PROFILE;
	    }
	  user_specified |= STYLE_FLAT_PROFILE;
	  break;
	case 'q':
	  if (optarg)
	    {
	      if (strchr (optarg, '/'))
		{
		  sym_id_add (optarg, INCL_ARCS);
		}
	      else
		{
		  sym_id_add (optarg, INCL_GRAPH);
		}
	    }
	  output_style |= STYLE_CALL_GRAPH;
	  user_specified |= STYLE_CALL_GRAPH;
	  break;
	case 'r':
	  output_style |= STYLE_FUNCTION_ORDER;
	  user_specified |= STYLE_FUNCTION_ORDER;
	  break;
	case 'R':
	  output_style |= STYLE_FILE_ORDER;
	  user_specified |= STYLE_FILE_ORDER;
	  function_mapping_file = optarg;
	  break;
	case 'Q':
	  if (optarg)
	    {
	      if (strchr (optarg, '/'))
		{
		  sym_id_add (optarg, EXCL_ARCS);
		}
	      else
		{
		  sym_id_add (optarg, EXCL_GRAPH);
		}
	      output_style |= STYLE_CALL_GRAPH;
	    }
	  else
	    {
	      output_style &= ~STYLE_CALL_GRAPH;
	    }
	  user_specified |= STYLE_CALL_GRAPH;
	  break;
	case 's':
	  output_style |= STYLE_SUMMARY_FILE;
	  user_specified |= STYLE_SUMMARY_FILE;
	  break;
	case 't':
	  bb_table_length = atoi (optarg);
	  if (bb_table_length < 0)
	    {
	      bb_table_length = 0;
	    }
	  break;
	case 'T':
	  bsd_style_output = TRUE;
	  break;
	case 'v':
	  printf ("%s version %s\n", whoami, VERSION);
	  done (0);
	case 'w':
	  output_width = atoi (optarg);
	  if (output_width < 1)
	    {
	      output_width = 1;
	    }
	  break;
	case 'x':
	  bb_annotate_all_lines = TRUE;
	  break;
	case 'y':
	  create_annotation_files = TRUE;
	  break;
	case 'z':
	  ignore_zeros = FALSE;
	  break;
	case 'Z':
	  if (optarg)
	    {
	      sym_id_add (optarg, EXCL_EXEC);
	      output_style |= STYLE_EXEC_COUNTS;
	    }
	  else
	    {
	      output_style &= ~STYLE_EXEC_COUNTS;
	    }
	  user_specified |= STYLE_ANNOTATED_SOURCE;
	  break;
	default:
	  usage (stderr, 1);
	}
    }

  /* Don't allow both ordering options, they modify the arc data in-place.  */
  if ((user_specified & STYLE_FUNCTION_ORDER)
      && (user_specified & STYLE_FILE_ORDER))
    {
      fprintf (stderr,"\
%s: Only one of --function-ordering and --file-ordering may be specified.\n",
	       whoami);
      done (1);
    }

  /* append value of GPROF_PATH to source search list if set: */
  str = (char *) getenv ("GPROF_PATH");
  if (str)
    {
      search_list_append (&src_search_list, str);
    }

  if (optind < argc)
    {
      a_out_name = argv[optind++];
    }
  if (optind < argc)
    {
      gmon_name = argv[optind++];
    }

  /*
   * Turn off default functions:
   */
  for (sp = &default_excluded_list[0]; *sp; sp++)
    {
      sym_id_add (*sp, EXCL_TIME);
      sym_id_add (*sp, EXCL_GRAPH);
#ifdef __alpha__
      sym_id_add (*sp, EXCL_FLAT);
#endif
    }

  /*
   * For line-by-line profiling, also want to keep those
   * functions off the flat profile:
   */
  if (line_granularity)
    {
      for (sp = &default_excluded_list[0]; *sp; sp++)
	{
	  sym_id_add (*sp, EXCL_FLAT);
	}
    }

  /*
   * Read symbol table from core file:
   */
  core_init (a_out_name);

  /*
   * If we should ignore direct function calls, we need to load
   * to core's text-space:
   */
  if (ignore_direct_calls)
    {
      core_get_text_space (core_bfd);
    }

  /*
   * Create symbols from core image:
   */
  if (line_granularity)
    {
      core_create_line_syms (core_bfd);
    }
  else
    {
      core_create_function_syms (core_bfd);
    }

  /*
   * Translate sym specs into syms:
   */
  sym_id_parse ();

  if (file_format == FF_PROF)
    {
#ifdef PROF_SUPPORT_IMPLEMENTED
      /*
       * Get information about mon.out file(s):
       */
      do
	{
	  mon_out_read (gmon_name);
	  if (optind < argc)
	    {
	      gmon_name = argv[optind];
	    }
	}
      while (optind++ < argc);
#else
      fprintf (stderr,
	       "%s: sorry, file format `prof' is not yet supported\n",
	       whoami);
      done (1);
#endif
    }
  else
    {
      /*
       * Get information about gmon.out file(s):
       */
      do
	{
	  gmon_out_read (gmon_name);
	  if (optind < argc)
	    {
	      gmon_name = argv[optind];
	    }
	}
      while (optind++ < argc);
    }

  /*
   * If user did not specify output style, try to guess something
   * reasonable:
   */
  if (output_style == 0)
    {
      if (gmon_input & (INPUT_HISTOGRAM | INPUT_CALL_GRAPH))
	{
	  output_style = STYLE_FLAT_PROFILE | STYLE_CALL_GRAPH;
	}
      else
	{
	  output_style = STYLE_EXEC_COUNTS;
	}
      output_style &= ~user_specified;
    }

  /*
   * Dump a gmon.sum file if requested (before any other processing!):
   */
  if (output_style & STYLE_SUMMARY_FILE)
    {
      gmon_out_write (GMONSUM);
    }

  if (gmon_input & INPUT_HISTOGRAM)
    {
      hist_assign_samples ();
    }

  if (gmon_input & INPUT_CALL_GRAPH)
    {
      cg = cg_assemble ();
    }

  /* do some simple sanity checks: */

  if ((output_style & STYLE_FLAT_PROFILE)
      && !(gmon_input & INPUT_HISTOGRAM))
    {
      fprintf (stderr, "%s: gmon.out file is missing histogram\n", whoami);
      done (1);
    }

  if ((output_style & STYLE_CALL_GRAPH) && !(gmon_input & INPUT_CALL_GRAPH))
    {
      fprintf (stderr,
	       "%s: gmon.out file is missing call-graph data\n", whoami);
      done (1);
    }

  /* output whatever user whishes to see: */

  if (cg && (output_style & STYLE_CALL_GRAPH) && bsd_style_output)
    {
      cg_print (cg);		/* print the dynamic profile */
    }

  if (output_style & STYLE_FLAT_PROFILE)
    {
      hist_print ();		/* print the flat profile */
    }

  if (cg && (output_style & STYLE_CALL_GRAPH))
    {
      if (!bsd_style_output)
	{
	  cg_print (cg);	/* print the dynamic profile */
	}
      cg_print_index ();
    }

  if (output_style & STYLE_EXEC_COUNTS)
    {
      print_exec_counts ();
    }

  if (output_style & STYLE_ANNOTATED_SOURCE)
    {
      print_annotated_source ();
    }
  if (output_style & STYLE_FUNCTION_ORDER)
    {
      cg_print_function_ordering ();
    }
  if (output_style & STYLE_FILE_ORDER)
    {
      cg_print_file_ordering ();
    }
  return 0;
}

void
done (status)
     int status;
{
  exit (status);
}
