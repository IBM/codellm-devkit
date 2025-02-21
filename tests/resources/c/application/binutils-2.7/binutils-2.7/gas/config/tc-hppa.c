/* tc-hppa.c -- Assemble for the PA
   Copyright (C) 1989, 1996 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */


/* HP PA-RISC support was contributed by the Center for Software Science
   at the University of Utah.  */

#include <stdio.h>
#include <ctype.h>

#include "as.h"
#include "subsegs.h"

#include "bfd/libhppa.h"
#include "bfd/libbfd.h"

/* Be careful, this file includes data *declarations*.  */
#include "opcode/hppa.h"

/* A "convient" place to put object file dependencies which do
   not need to be seen outside of tc-hppa.c.  */
#ifdef OBJ_ELF
/* Names of various debugging spaces/subspaces.  */
#define GDB_DEBUG_SPACE_NAME ".stab"
#define GDB_STRINGS_SUBSPACE_NAME ".stabstr"
#define GDB_SYMBOLS_SUBSPACE_NAME ".stab"
#define UNWIND_SECTION_NAME ".PARISC.unwind"
/* Nonzero if CODE is a fixup code needing further processing.  */

/* Object file formats specify relocation types.  */
typedef elf32_hppa_reloc_type reloc_type;

/* Object file formats specify BFD symbol types.  */
typedef elf_symbol_type obj_symbol_type;

/* How to generate a relocation.  */
#define hppa_gen_reloc_type hppa_elf_gen_reloc_type

/* ELF objects can have versions, but apparently do not have anywhere
   to store a copyright string.  */
#define obj_version obj_elf_version
#define obj_copyright obj_elf_version

/* Use space aliases.  */
#define USE_ALIASES 1
#endif

#ifdef OBJ_SOM
/* Names of various debugging spaces/subspaces.  */
#define GDB_DEBUG_SPACE_NAME "$GDB_DEBUG$"
#define GDB_STRINGS_SUBSPACE_NAME "$GDB_STRINGS$"
#define GDB_SYMBOLS_SUBSPACE_NAME "$GDB_SYMBOLS$"
#define UNWIND_SECTION_NAME "$UNWIND$"

/* Object file formats specify relocation types.  */
typedef int reloc_type;

/* SOM objects can have both a version string and a copyright string.  */
#define obj_version obj_som_version
#define obj_copyright obj_som_copyright

/* Do not use space aliases.  */
#define USE_ALIASES 0

/* How to generate a relocation.  */
#define hppa_gen_reloc_type hppa_som_gen_reloc_type

/* Object file formats specify BFD symbol types.  */
typedef som_symbol_type obj_symbol_type;

/* This apparently isn't in older versions of hpux reloc.h.  */
#ifndef R_DLT_REL
#define R_DLT_REL 0x78
#endif
#endif

#ifndef R_N0SEL
#define R_N0SEL 0xd8
#endif

#ifndef R_N1SEL
#define R_N1SEL 0xd9
#endif

/* Various structures and types used internally in tc-hppa.c.  */

/* Unwind table and descriptor.  FIXME: Sync this with GDB version.  */

struct unwind_desc
  {
    unsigned int cannot_unwind:1;
    unsigned int millicode:1;
    unsigned int millicode_save_rest:1;
    unsigned int region_desc:2;
    unsigned int save_sr:2;
    unsigned int entry_fr:4;
    unsigned int entry_gr:5;
    unsigned int args_stored:1;
    unsigned int call_fr:5;
    unsigned int call_gr:5;
    unsigned int save_sp:1;
    unsigned int save_rp:1;
    unsigned int save_rp_in_frame:1;
    unsigned int extn_ptr_defined:1;
    unsigned int cleanup_defined:1;

    unsigned int hpe_interrupt_marker:1;
    unsigned int hpux_interrupt_marker:1;
    unsigned int reserved:3;
    unsigned int frame_size:27;
  };

struct unwind_table
  {
    /* Starting and ending offsets of the region described by
       descriptor.  */
    unsigned int start_offset;
    unsigned int end_offset;
    struct unwind_desc descriptor;
  };

/* This structure is used by the .callinfo, .enter, .leave pseudo-ops to
   control the entry and exit code they generate. It is also used in
   creation of the correct stack unwind descriptors.

   NOTE:  GAS does not support .enter and .leave for the generation of
   prologues and epilogues.  FIXME.

   The fields in structure roughly correspond to the arguments available on the
   .callinfo pseudo-op.  */

struct call_info
  {
    /* The unwind descriptor being built.  */
    struct unwind_table ci_unwind;

    /* Name of this function.  */
    symbolS *start_symbol;

    /* (temporary) symbol used to mark the end of this function.  */
    symbolS *end_symbol;

    /* Next entry in the chain.  */
    struct call_info *ci_next;
  };

/* Operand formats for FP instructions.   Note not all FP instructions
   allow all four formats to be used (for example fmpysub only allows
   SGL and DBL).  */
typedef enum
  {
    SGL, DBL, ILLEGAL_FMT, QUAD
  }
fp_operand_format;

/* This fully describes the symbol types which may be attached to
   an EXPORT or IMPORT directive.  Only SOM uses this formation
   (ELF has no need for it).  */
typedef enum
  {
    SYMBOL_TYPE_UNKNOWN,
    SYMBOL_TYPE_ABSOLUTE,
    SYMBOL_TYPE_CODE,
    SYMBOL_TYPE_DATA,
    SYMBOL_TYPE_ENTRY,
    SYMBOL_TYPE_MILLICODE,
    SYMBOL_TYPE_PLABEL,
    SYMBOL_TYPE_PRI_PROG,
    SYMBOL_TYPE_SEC_PROG,
  }
pa_symbol_type;

/* This structure contains information needed to assemble
   individual instructions.  */
struct pa_it
  {
    /* Holds the opcode after parsing by pa_ip.  */
    unsigned long opcode;

    /* Holds an expression associated with the current instruction.  */
    expressionS exp;

    /* Does this instruction use PC-relative addressing.  */
    int pcrel;

    /* Floating point formats for operand1 and operand2.  */
    fp_operand_format fpof1;
    fp_operand_format fpof2;

    /* Holds the field selector for this instruction
       (for example L%, LR%, etc).  */
    long field_selector;

    /* Holds any argument relocation bits associated with this
       instruction.  (instruction should be some sort of call).  */
    long arg_reloc;

    /* The format specification for this instruction.  */
    int format;

    /* The relocation (if any) associated with this instruction.  */
    reloc_type reloc;
  };

/* PA-89 floating point registers are arranged like this:


   +--------------+--------------+
   |   0 or 16L   |  16 or 16R   |
   +--------------+--------------+
   |   1 or 17L   |  17 or 17R   |
   +--------------+--------------+
   |              |              |

   .              .              .
   .              .              .
   .              .              .

   |              |              |
   +--------------+--------------+
   |  14 or 30L   |  30 or 30R   |
   +--------------+--------------+
   |  15 or 31L   |  31 or 31R   |
   +--------------+--------------+


   The following is a version of pa_parse_number that
   handles the L/R notation and returns the correct
   value to put into the instruction register field.
   The correct value to put into the instruction is
   encoded in the structure 'pa_11_fp_reg_struct'.  */

struct pa_11_fp_reg_struct
  {
    /* The register number.  */
    char number_part;

    /* L/R selector.  */
    char l_r_select;
  };

/* Additional information needed to build argument relocation stubs.  */
struct call_desc
  {
    /* The argument relocation specification.  */
    unsigned int arg_reloc;

    /* Number of arguments.  */
    unsigned int arg_count;
  };

/* This structure defines an entry in the subspace dictionary
   chain.  */

struct subspace_dictionary_chain
  {
    /* Nonzero if this space has been defined by the user code.  */
    unsigned int ssd_defined;

    /* Name of this subspace.  */
    char *ssd_name;

    /* GAS segment and subsegment associated with this subspace.  */
    asection *ssd_seg;
    int ssd_subseg;

    /* Next space in the subspace dictionary chain.  */
    struct subspace_dictionary_chain *ssd_next;
  };

typedef struct subspace_dictionary_chain ssd_chain_struct;

/* This structure defines an entry in the subspace dictionary
   chain.  */

struct space_dictionary_chain
  {
    /* Nonzero if this space has been defined by the user code or
       as a default space.  */
    unsigned int sd_defined;

    /* Nonzero if this spaces has been defined by the user code.  */
    unsigned int sd_user_defined;

    /* The space number (or index).  */
    unsigned int sd_spnum;

    /* The name of this subspace.  */
    char *sd_name;

    /* GAS segment to which this subspace corresponds.  */
    asection *sd_seg;

    /* Current subsegment number being used.  */
    int sd_last_subseg;

    /* The chain of subspaces contained within this space.  */
    ssd_chain_struct *sd_subspaces;

    /* The next entry in the space dictionary chain.  */
    struct space_dictionary_chain *sd_next;
  };

typedef struct space_dictionary_chain sd_chain_struct;

/* Structure for previous label tracking.  Needed so that alignments,
   callinfo declarations, etc can be easily attached to a particular
   label.  */
typedef struct label_symbol_struct
  {
    struct symbol *lss_label;
    sd_chain_struct *lss_space;
    struct label_symbol_struct *lss_next;
  }
label_symbol_struct;

/* This structure defines attributes of the default subspace
   dictionary entries.  */

struct default_subspace_dict
  {
    /* Name of the subspace.  */
    char *name;

    /* FIXME.  Is this still needed?  */
    char defined;

    /* Nonzero if this subspace is loadable.  */
    char loadable;

    /* Nonzero if this subspace contains only code.  */
    char code_only;

    /* Nonzero if this is a common subspace.  */
    char common;

    /* Nonzero if this is a common subspace which allows symbols
       to be multiply defined.  */
    char dup_common;

    /* Nonzero if this subspace should be zero filled.  */
    char zero;

    /* Sort key for this subspace.  */
    unsigned char sort;

    /* Access control bits for this subspace.  Can represent RWX access
       as well as privilege level changes for gateways.  */
    int access;

    /* Index of containing space.  */
    int space_index;

    /* Alignment (in bytes) of this subspace.  */
    int alignment;

    /* Quadrant within space where this subspace should be loaded.  */
    int quadrant;

    /* An index into the default spaces array.  */
    int def_space_index;

    /* An alias for this section (or NULL if no alias exists).  */
    char *alias;

    /* Subsegment associated with this subspace.  */
    subsegT subsegment;
  };

/* This structure defines attributes of the default space
   dictionary entries.  */

struct default_space_dict
  {
    /* Name of the space.  */
    char *name;

    /* Space number.  It is possible to identify spaces within
       assembly code numerically!  */
    int spnum;

    /* Nonzero if this space is loadable.  */
    char loadable;

    /* Nonzero if this space is "defined".  FIXME is still needed */
    char defined;

    /* Nonzero if this space can not be shared.  */
    char private;

    /* Sort key for this space.  */
    unsigned char sort;

    /* Segment associated with this space.  */
    asection *segment;

    /* An alias for this section (or NULL if no alias exists).  */
    char *alias;
  };

/* Extra information needed to perform fixups (relocations) on the PA.  */
struct hppa_fix_struct
  {
    /* The field selector.  */
    enum hppa_reloc_field_selector_type fx_r_field;

    /* Type of fixup.  */
    int fx_r_type;

    /* Format of fixup.  */
    int fx_r_format;

    /* Argument relocation bits.  */
    long fx_arg_reloc;

    /* The segment this fixup appears in.  */
    segT segment;
  };

/* Structure to hold information about predefined registers.  */

struct pd_reg
  {
    char *name;
    int value;
  };

/* This structure defines the mapping from a FP condition string
   to a condition number which can be recorded in an instruction.  */
struct fp_cond_map
  {
    char *string;
    int cond;
  };

/* This structure defines a mapping from a field selector
   string to a field selector type.  */
struct selector_entry
  {
    char *prefix;
    int field_selector;
  };

/* Prototypes for functions local to tc-hppa.c.  */

static void pa_check_current_space_and_subspace PARAMS ((void));
static fp_operand_format pa_parse_fp_format PARAMS ((char **s));
static void pa_cons PARAMS ((int));
static void pa_data PARAMS ((int));
static void pa_float_cons PARAMS ((int));
static void pa_fill PARAMS ((int));
static void pa_lcomm PARAMS ((int));
static void pa_lsym PARAMS ((int));
static void pa_stringer PARAMS ((int));
static void pa_text PARAMS ((int));
static void pa_version PARAMS ((int));
static int pa_parse_fp_cmp_cond PARAMS ((char **));
static int get_expression PARAMS ((char *));
static int pa_get_absolute_expression PARAMS ((struct pa_it *, char **));
static int evaluate_absolute PARAMS ((struct pa_it *));
static unsigned int pa_build_arg_reloc PARAMS ((char *));
static unsigned int pa_align_arg_reloc PARAMS ((unsigned int, unsigned int));
static int pa_parse_nullif PARAMS ((char **));
static int pa_parse_nonneg_cmpsub_cmpltr PARAMS ((char **, int));
static int pa_parse_neg_cmpsub_cmpltr PARAMS ((char **, int));
static int pa_parse_neg_add_cmpltr PARAMS ((char **, int));
static int pa_parse_nonneg_add_cmpltr PARAMS ((char **, int));
static void pa_align PARAMS ((int));
static void pa_block PARAMS ((int));
static void pa_brtab PARAMS ((int));
static void pa_try PARAMS ((int));
static void pa_call PARAMS ((int));
static void pa_call_args PARAMS ((struct call_desc *));
static void pa_callinfo PARAMS ((int));
static void pa_code PARAMS ((int));
static void pa_comm PARAMS ((int));
static void pa_copyright PARAMS ((int));
static void pa_end PARAMS ((int));
static void pa_enter PARAMS ((int));
static void pa_entry PARAMS ((int));
static void pa_equ PARAMS ((int));
static void pa_exit PARAMS ((int));
static void pa_export PARAMS ((int));
static void pa_type_args PARAMS ((symbolS *, int));
static void pa_import PARAMS ((int));
static void pa_label PARAMS ((int));
static void pa_leave PARAMS ((int));
static void pa_level PARAMS ((int));
static void pa_origin PARAMS ((int));
static void pa_proc PARAMS ((int));
static void pa_procend PARAMS ((int));
static void pa_space PARAMS ((int));
static void pa_spnum PARAMS ((int));
static void pa_subspace PARAMS ((int));
static void pa_param PARAMS ((int));
static void pa_undefine_label PARAMS ((void));
static int need_pa11_opcode PARAMS ((struct pa_it *,
				     struct pa_11_fp_reg_struct *));
static int pa_parse_number PARAMS ((char **, struct pa_11_fp_reg_struct *));
static label_symbol_struct *pa_get_label PARAMS ((void));
static sd_chain_struct *create_new_space PARAMS ((char *, int, int,
						  int, int, int,
						  asection *, int));
static ssd_chain_struct *create_new_subspace PARAMS ((sd_chain_struct *,
						      char *, int, int,
						      int, int, int,
						      int, int, int, int,
						      int, asection *));
static ssd_chain_struct *update_subspace PARAMS ((sd_chain_struct *,
						  char *, int, int, int,
						  int, int, int, int,
						  int, int, int,
						  asection *));
static sd_chain_struct *is_defined_space PARAMS ((char *));
static ssd_chain_struct *is_defined_subspace PARAMS ((char *));
static sd_chain_struct *pa_segment_to_space PARAMS ((asection *));
static ssd_chain_struct *pa_subsegment_to_subspace PARAMS ((asection *,
							    subsegT));
static sd_chain_struct *pa_find_space_by_number PARAMS ((int));
static unsigned int pa_subspace_start PARAMS ((sd_chain_struct *, int));
static void pa_ip PARAMS ((char *));
static void fix_new_hppa PARAMS ((fragS *, int, int, symbolS *,
				  long, expressionS *, int,
				  bfd_reloc_code_real_type,
				  enum hppa_reloc_field_selector_type,
				  int, long, int *));
static int is_end_of_statement PARAMS ((void));
static int reg_name_search PARAMS ((char *));
static int pa_chk_field_selector PARAMS ((char **));
static int is_same_frag PARAMS ((fragS *, fragS *));
static void pa_build_unwind_subspace PARAMS ((struct call_info *));
static void process_exit PARAMS ((void));
static sd_chain_struct *pa_parse_space_stmt PARAMS ((char *, int));
static int log2 PARAMS ((int));
static int pa_next_subseg PARAMS ((sd_chain_struct *));
static unsigned int pa_stringer_aux PARAMS ((char *));
static void pa_spaces_begin PARAMS ((void));
static void hppa_elf_mark_end_of_function PARAMS ((void));

/* File and gloally scoped variable declarations.  */

/* Root and final entry in the space chain.  */
static sd_chain_struct *space_dict_root;
static sd_chain_struct *space_dict_last;

/* The current space and subspace.  */
static sd_chain_struct *current_space;
static ssd_chain_struct *current_subspace;

/* Root of the call_info chain.  */
static struct call_info *call_info_root;

/* The last call_info (for functions) structure
   seen so it can be associated with fixups and
   function labels.  */
static struct call_info *last_call_info;

/* The last call description (for actual calls).  */
static struct call_desc last_call_desc;

/* Jumps are always the same size -- one instruction.  */
int md_short_jump_size = 4;
int md_long_jump_size = 4;

/* handle of the OPCODE hash table */
static struct hash_control *op_hash = NULL;

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful.  */
const char comment_chars[] = ";";

/* Table of pseudo ops for the PA.  FIXME -- how many of these
   are now redundant with the overall GAS and the object file
   dependent tables?  */
const pseudo_typeS md_pseudo_table[] =
{
  /* align pseudo-ops on the PA specify the actual alignment requested,
     not the log2 of the requested alignment.  */
  {"align", pa_align, 8},
  {"begin_brtab", pa_brtab, 1},
  {"begin_try", pa_try, 1},
  {"block", pa_block, 1},
  {"blockz", pa_block, 0},
  {"byte", pa_cons, 1},
  {"call", pa_call, 0},
  {"callinfo", pa_callinfo, 0},
  {"code", pa_code, 0},
  {"comm", pa_comm, 0},
  {"copyright", pa_copyright, 0},
  {"data", pa_data, 0},
  {"double", pa_float_cons, 'd'},
  {"end", pa_end, 0},
  {"end_brtab", pa_brtab, 0},
  {"end_try", pa_try, 0},
  {"enter", pa_enter, 0},
  {"entry", pa_entry, 0},
  {"equ", pa_equ, 0},
  {"exit", pa_exit, 0},
  {"export", pa_export, 0},
  {"fill", pa_fill, 0},
  {"float", pa_float_cons, 'f'},
  {"half", pa_cons, 2},
  {"import", pa_import, 0},
  {"int", pa_cons, 4},
  {"label", pa_label, 0},
  {"lcomm", pa_lcomm, 0},
  {"leave", pa_leave, 0},
  {"level", pa_level, 0},
  {"long", pa_cons, 4},
  {"lsym", pa_lsym, 0},
  {"nsubspa", pa_subspace, 1},
  {"octa", pa_cons, 16},
  {"org", pa_origin, 0},
  {"origin", pa_origin, 0},
  {"param", pa_param, 0},
  {"proc", pa_proc, 0},
  {"procend", pa_procend, 0},
  {"quad", pa_cons, 8},
  {"reg", pa_equ, 1},
  {"short", pa_cons, 2},
  {"single", pa_float_cons, 'f'},
  {"space", pa_space, 0},
  {"spnum", pa_spnum, 0},
  {"string", pa_stringer, 0},
  {"stringz", pa_stringer, 1},
  {"subspa", pa_subspace, 0},
  {"text", pa_text, 0},
  {"version", pa_version, 0},
  {"word", pa_cons, 4},
  {NULL, 0, 0}
};

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output.

   Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.

   Also note that C style comments will always work. */
const char line_comment_chars[] = "#";

/* This array holds the characters which act as line separators.  */
const char line_separator_chars[] = "!";

/* Chars that can be used to separate mant from exp in floating point nums.  */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant.
   As in 0f12.456 or 0d1.2345e12.

   Be aware that MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT may have to be
   changed in read.c.  Ideally it shouldn't hae to know abou it at
   all, but nothing is ideal around here.  */
const char FLT_CHARS[] = "rRsSfFdDxXpP";

static struct pa_it the_insn;

/* Points to the end of an expression just parsed by get_expressoin
   and friends.  FIXME.  This shouldn't be handled with a file-global
   variable.  */
static char *expr_end;

/* Nonzero if a .callinfo appeared within the current procedure.  */
static int callinfo_found;

/* Nonzero if the assembler is currently within a .entry/.exit pair.  */
static int within_entry_exit;

/* Nonzero if the assembler is currently within a procedure definition.  */
static int within_procedure;

/* Handle on strucutre which keep track of the last symbol
   seen in each subspace.  */
static label_symbol_struct *label_symbols_rootp = NULL;

/* Holds the last field selector.  */
static int hppa_field_selector;

/* A dummy bfd symbol so that all relocations have symbols of some kind.  */
static symbolS *dummy_symbol;

/* Nonzero if errors are to be printed.  */
static int print_errors = 1;

/* List of registers that are pre-defined:

   Each general register has one predefined name of the form
   %r<REGNUM> which has the value <REGNUM>.

   Space and control registers are handled in a similar manner,
   but use %sr<REGNUM> and %cr<REGNUM> as their predefined names.

   Likewise for the floating point registers, but of the form
   %fr<REGNUM>.  Floating point registers have additional predefined
   names with 'L' and 'R' suffixes (e.g. %fr19L, %fr19R) which
   again have the value <REGNUM>.

   Many registers also have synonyms:

   %r26 - %r23 have %arg0 - %arg3 as synonyms
   %r28 - %r29 have %ret0 - %ret1 as synonyms
   %r30 has %sp as a synonym
   %r27 has %dp as a synonym
   %r2  has %rp as a synonym

   Almost every control register has a synonym; they are not listed
   here for brevity.

   The table is sorted. Suitable for searching by a binary search. */

static const struct pd_reg pre_defined_registers[] =
{
  {"%arg0", 26},
  {"%arg1", 25},
  {"%arg2", 24},
  {"%arg3", 23},
  {"%cr0", 0},
  {"%cr10", 10},
  {"%cr11", 11},
  {"%cr12", 12},
  {"%cr13", 13},
  {"%cr14", 14},
  {"%cr15", 15},
  {"%cr16", 16},
  {"%cr17", 17},
  {"%cr18", 18},
  {"%cr19", 19},
  {"%cr20", 20},
  {"%cr21", 21},
  {"%cr22", 22},
  {"%cr23", 23},
  {"%cr24", 24},
  {"%cr25", 25},
  {"%cr26", 26},
  {"%cr27", 27},
  {"%cr28", 28},
  {"%cr29", 29},
  {"%cr30", 30},
  {"%cr31", 31},
  {"%cr8", 8},
  {"%cr9", 9},
  {"%dp", 27},
  {"%eiem", 15},
  {"%eirr", 23},
  {"%fr0", 0},
  {"%fr0l", 0},
  {"%fr0r", 0},
  {"%fr1", 1},
  {"%fr10", 10},
  {"%fr10l", 10},
  {"%fr10r", 10},
  {"%fr11", 11},
  {"%fr11l", 11},
  {"%fr11r", 11},
  {"%fr12", 12},
  {"%fr12l", 12},
  {"%fr12r", 12},
  {"%fr13", 13},
  {"%fr13l", 13},
  {"%fr13r", 13},
  {"%fr14", 14},
  {"%fr14l", 14},
  {"%fr14r", 14},
  {"%fr15", 15},
  {"%fr15l", 15},
  {"%fr15r", 15},
  {"%fr16", 16},
  {"%fr16l", 16},
  {"%fr16r", 16},
  {"%fr17", 17},
  {"%fr17l", 17},
  {"%fr17r", 17},
  {"%fr18", 18},
  {"%fr18l", 18},
  {"%fr18r", 18},
  {"%fr19", 19},
  {"%fr19l", 19},
  {"%fr19r", 19},
  {"%fr1l", 1},
  {"%fr1r", 1},
  {"%fr2", 2},
  {"%fr20", 20},
  {"%fr20l", 20},
  {"%fr20r", 20},
  {"%fr21", 21},
  {"%fr21l", 21},
  {"%fr21r", 21},
  {"%fr22", 22},
  {"%fr22l", 22},
  {"%fr22r", 22},
  {"%fr23", 23},
  {"%fr23l", 23},
  {"%fr23r", 23},
  {"%fr24", 24},
  {"%fr24l", 24},
  {"%fr24r", 24},
  {"%fr25", 25},
  {"%fr25l", 25},
  {"%fr25r", 25},
  {"%fr26", 26},
  {"%fr26l", 26},
  {"%fr26r", 26},
  {"%fr27", 27},
  {"%fr27l", 27},
  {"%fr27r", 27},
  {"%fr28", 28},
  {"%fr28l", 28},
  {"%fr28r", 28},
  {"%fr29", 29},
  {"%fr29l", 29},
  {"%fr29r", 29},
  {"%fr2l", 2},
  {"%fr2r", 2},
  {"%fr3", 3},
  {"%fr30", 30},
  {"%fr30l", 30},
  {"%fr30r", 30},
  {"%fr31", 31},
  {"%fr31l", 31},
  {"%fr31r", 31},
  {"%fr3l", 3},
  {"%fr3r", 3},
  {"%fr4", 4},
  {"%fr4l", 4},
  {"%fr4r", 4},
  {"%fr5", 5},
  {"%fr5l", 5},
  {"%fr5r", 5},
  {"%fr6", 6},
  {"%fr6l", 6},
  {"%fr6r", 6},
  {"%fr7", 7},
  {"%fr7l", 7},
  {"%fr7r", 7},
  {"%fr8", 8},
  {"%fr8l", 8},
  {"%fr8r", 8},
  {"%fr9", 9},
  {"%fr9l", 9},
  {"%fr9r", 9},
  {"%hta", 25},
  {"%iir", 19},
  {"%ior", 21},
  {"%ipsw", 22},
  {"%isr", 20},
  {"%itmr", 16},
  {"%iva", 14},
  {"%pcoq", 18},
  {"%pcsq", 17},
  {"%pidr1", 8},
  {"%pidr2", 9},
  {"%pidr3", 12},
  {"%pidr4", 13},
  {"%ppda", 24},
  {"%r0", 0},
  {"%r1", 1},
  {"%r10", 10},
  {"%r11", 11},
  {"%r12", 12},
  {"%r13", 13},
  {"%r14", 14},
  {"%r15", 15},
  {"%r16", 16},
  {"%r17", 17},
  {"%r18", 18},
  {"%r19", 19},
  {"%r2", 2},
  {"%r20", 20},
  {"%r21", 21},
  {"%r22", 22},
  {"%r23", 23},
  {"%r24", 24},
  {"%r25", 25},
  {"%r26", 26},
  {"%r27", 27},
  {"%r28", 28},
  {"%r29", 29},
  {"%r3", 3},
  {"%r30", 30},
  {"%r31", 31},
  {"%r4", 4},
  {"%r5", 5},
  {"%r6", 6},
  {"%r7", 7},
  {"%r8", 8},
  {"%r9", 9},
  {"%rctr", 0},
  {"%ret0", 28},
  {"%ret1", 29},
  {"%rp", 2},
  {"%sar", 11},
  {"%sp", 30},
  {"%sr0", 0},
  {"%sr1", 1},
  {"%sr2", 2},
  {"%sr3", 3},
  {"%sr4", 4},
  {"%sr5", 5},
  {"%sr6", 6},
  {"%sr7", 7},
  {"%tr0", 24},
  {"%tr1", 25},
  {"%tr2", 26},
  {"%tr3", 27},
  {"%tr4", 28},
  {"%tr5", 29},
  {"%tr6", 30},
  {"%tr7", 31}
};

/* This table is sorted by order of the length of the string. This is
   so we check for <> before we check for <. If we had a <> and checked
   for < first, we would get a false match.  */
static const struct fp_cond_map fp_cond_map[] =
{
  {"false?", 0},
  {"false", 1},
  {"true?", 30},
  {"true", 31},
  {"!<=>", 3},
  {"!?>=", 8},
  {"!?<=", 16},
  {"!<>", 7},
  {"!>=", 11},
  {"!?>", 12},
  {"?<=", 14},
  {"!<=", 19},
  {"!?<", 20},
  {"?>=", 22},
  {"!?=", 24},
  {"!=t", 27},
  {"<=>", 29},
  {"=t", 5},
  {"?=", 6},
  {"?<", 10},
  {"<=", 13},
  {"!>", 15},
  {"?>", 18},
  {">=", 21},
  {"!<", 23},
  {"<>", 25},
  {"!=", 26},
  {"!?", 28},
  {"?", 2},
  {"=", 4},
  {"<", 9},
  {">", 17}
};

static const struct selector_entry selector_table[] =
{
  {"f", e_fsel},
  {"l", e_lsel},
  {"ld", e_ldsel},
  {"lp", e_lpsel},
  {"lr", e_lrsel},
  {"ls", e_lssel},
  {"lt", e_ltsel},
  {"n", e_nsel},
  {"nl", e_nlsel},
  {"nlr", e_nlrsel},
  {"p", e_psel},
  {"r", e_rsel},
  {"rd", e_rdsel},
  {"rp", e_rpsel},
  {"rr", e_rrsel},
  {"rs", e_rssel},
  {"rt", e_rtsel},
  {"t", e_tsel},
};

/* default space and subspace dictionaries */

#define GDB_SYMBOLS          GDB_SYMBOLS_SUBSPACE_NAME
#define GDB_STRINGS          GDB_STRINGS_SUBSPACE_NAME

/* pre-defined subsegments (subspaces) for the HPPA.  */
#define SUBSEG_CODE   0
#define SUBSEG_LIT    1
#define SUBSEG_MILLI  2
#define SUBSEG_DATA   0
#define SUBSEG_BSS    2
#define SUBSEG_UNWIND 3
#define SUBSEG_GDB_STRINGS 0
#define SUBSEG_GDB_SYMBOLS 1

static struct default_subspace_dict pa_def_subspaces[] =
{
  {"$CODE$", 1, 1, 1, 0, 0, 0, 24, 0x2c, 0, 8, 0, 0, ".text", SUBSEG_CODE},
  {"$DATA$", 1, 1, 0, 0, 0, 0, 24, 0x1f, 1, 8, 1, 1, ".data", SUBSEG_DATA},
  {"$LIT$", 1, 1, 0, 0, 0, 0, 16, 0x2c, 0, 8, 0, 0, ".text", SUBSEG_LIT},
  {"$MILLICODE$", 1, 1, 0, 0, 0, 0, 8, 0x2c, 0, 8, 0, 0, ".text", SUBSEG_MILLI},
  {"$BSS$", 1, 1, 0, 0, 0, 1, 80, 0x1f, 1, 8, 1, 1, ".bss", SUBSEG_BSS},
#ifdef OBJ_ELF
  {"$UNWIND$", 1, 1, 0, 0, 0, 0, 64, 0x2c, 0, 4, 0, 0, ".PARISC.unwind", SUBSEG_UNWIND},
#endif
  {NULL, 0, 1, 0, 0, 0, 0, 255, 0x1f, 0, 4, 0, 0, 0}
};

static struct default_space_dict pa_def_spaces[] =
{
  {"$TEXT$", 0, 1, 1, 0, 8, ASEC_NULL, ".text"},
  {"$PRIVATE$", 1, 1, 1, 1, 16, ASEC_NULL, ".data"},
  {NULL, 0, 0, 0, 0, 0, ASEC_NULL, NULL}
};

/* Misc local definitions used by the assembler.  */

/* Return nonzero if the string pointed to by S potentially represents
   a right or left half of a FP register  */
#define IS_R_SELECT(S)   (*(S) == 'R' || *(S) == 'r')
#define IS_L_SELECT(S)   (*(S) == 'L' || *(S) == 'l')

/* These macros are used to maintain spaces/subspaces.  */
#define SPACE_DEFINED(space_chain)	(space_chain)->sd_defined
#define SPACE_USER_DEFINED(space_chain) (space_chain)->sd_user_defined
#define SPACE_SPNUM(space_chain)	(space_chain)->sd_spnum
#define SPACE_NAME(space_chain)		(space_chain)->sd_name

#define SUBSPACE_DEFINED(ss_chain)	(ss_chain)->ssd_defined
#define SUBSPACE_NAME(ss_chain)		(ss_chain)->ssd_name

/* Insert FIELD into OPCODE starting at bit START.  Continue pa_ip
   main loop after insertion.  */

#define INSERT_FIELD_AND_CONTINUE(OPCODE, FIELD, START) \
  { \
    ((OPCODE) |= (FIELD) << (START)); \
    continue; \
  }

/* Simple range checking for FIELD againt HIGH and LOW bounds.
   IGNORE is used to suppress the error message.  */

#define CHECK_FIELD(FIELD, HIGH, LOW, IGNORE) \
  { \
    if ((FIELD) > (HIGH) || (FIELD) < (LOW)) \
      { \
	if (! IGNORE) \
          as_bad ("Field out of range [%d..%d] (%d).", (LOW), (HIGH), \
		  (int) (FIELD));\
        break; \
      } \
  }

#define is_DP_relative(exp)			\
  ((exp).X_op == O_subtract			\
   && strcmp((exp).X_op_symbol->bsym->name, "$global$") == 0)

#define is_PC_relative(exp)			\
  ((exp).X_op == O_subtract			\
   && strcmp((exp).X_op_symbol->bsym->name, "$PIC_pcrel$0") == 0)

/* We need some complex handling for stabs (sym1 - sym2).  Luckily, we'll
   always be able to reduce the expression to a constant, so we don't
   need real complex handling yet.  */
#define is_complex(exp)				\
  ((exp).X_op != O_constant && (exp).X_op != O_symbol)

/* Actual functions to implement the PA specific code for the assembler.  */

/* Called before writing the object file.  Make sure entry/exit and
   proc/procend pairs match.  */

void
pa_check_eof ()
{
  if (within_entry_exit)
    as_fatal ("Missing .exit\n");

  if (within_procedure)
    as_fatal ("Missing .procend\n");
}

/* Check to make sure we have a valid space and subspace.  */

static void
pa_check_current_space_and_subspace ()
{
  if (current_space == NULL)
    as_fatal ("Not in a space.\n");

  if (current_subspace == NULL)
    as_fatal ("Not in a subspace.\n");
}

/* Returns a pointer to the label_symbol_struct for the current space.
   or NULL if no label_symbol_struct exists for the current space.  */

static label_symbol_struct *
pa_get_label ()
{
  label_symbol_struct *label_chain;
  sd_chain_struct *space_chain = current_space;

  for (label_chain = label_symbols_rootp;
       label_chain;
       label_chain = label_chain->lss_next)
    if (space_chain == label_chain->lss_space && label_chain->lss_label)
      return label_chain;

  return NULL;
}

/* Defines a label for the current space.  If one is already defined,
   this function will replace it with the new label.  */

void
pa_define_label (symbol)
     symbolS *symbol;
{
  label_symbol_struct *label_chain = pa_get_label ();
  sd_chain_struct *space_chain = current_space;

  if (label_chain)
    label_chain->lss_label = symbol;
  else
    {
      /* Create a new label entry and add it to the head of the chain.  */
      label_chain
	= (label_symbol_struct *) xmalloc (sizeof (label_symbol_struct));
      label_chain->lss_label = symbol;
      label_chain->lss_space = space_chain;
      label_chain->lss_next = NULL;

      if (label_symbols_rootp)
	label_chain->lss_next = label_symbols_rootp;

      label_symbols_rootp = label_chain;
    }
}

/* Removes a label definition for the current space.
   If there is no label_symbol_struct entry, then no action is taken.  */

static void
pa_undefine_label ()
{
  label_symbol_struct *label_chain;
  label_symbol_struct *prev_label_chain = NULL;
  sd_chain_struct *space_chain = current_space;

  for (label_chain = label_symbols_rootp;
       label_chain;
       label_chain = label_chain->lss_next)
    {
      if (space_chain == label_chain->lss_space && label_chain->lss_label)
	{
	  /* Remove the label from the chain and free its memory.  */
	  if (prev_label_chain)
	    prev_label_chain->lss_next = label_chain->lss_next;
	  else
	    label_symbols_rootp = label_chain->lss_next;

	  free (label_chain);
	  break;
	}
      prev_label_chain = label_chain;
    }
}


/* An HPPA-specific version of fix_new.  This is required because the HPPA
   code needs to keep track of some extra stuff.  Each call to fix_new_hppa
   results in the creation of an instance of an hppa_fix_struct.  An
   hppa_fix_struct stores the extra information along with a pointer to the
   original fixS.  This is attached to the original fixup via the
   tc_fix_data field.  */

static void
fix_new_hppa (frag, where, size, add_symbol, offset, exp, pcrel,
	      r_type, r_field, r_format, arg_reloc, unwind_bits)
     fragS *frag;
     int where;
     int size;
     symbolS *add_symbol;
     long offset;
     expressionS *exp;
     int pcrel;
     bfd_reloc_code_real_type r_type;
     enum hppa_reloc_field_selector_type r_field;
     int r_format;
     long arg_reloc;
     int* unwind_bits;
{
  fixS *new_fix;

  struct hppa_fix_struct *hppa_fix = (struct hppa_fix_struct *)
  obstack_alloc (&notes, sizeof (struct hppa_fix_struct));

  if (exp != NULL)
    new_fix = fix_new_exp (frag, where, size, exp, pcrel, r_type);
  else
    new_fix = fix_new (frag, where, size, add_symbol, offset, pcrel, r_type);
  new_fix->tc_fix_data = (void *) hppa_fix;
  hppa_fix->fx_r_type = r_type;
  hppa_fix->fx_r_field = r_field;
  hppa_fix->fx_r_format = r_format;
  hppa_fix->fx_arg_reloc = arg_reloc;
  hppa_fix->segment = now_seg;
#ifdef OBJ_SOM
  if (r_type == R_ENTRY || r_type == R_EXIT)
    new_fix->fx_offset = *unwind_bits;
#endif

  /* foo-$global$ is used to access non-automatic storage.  $global$
     is really just a marker and has served its purpose, so eliminate
     it now so as not to confuse write.c.  */
  if (new_fix->fx_subsy
      && !strcmp (S_GET_NAME (new_fix->fx_subsy), "$global$"))
    new_fix->fx_subsy = NULL;
}

/* Parse a .byte, .word, .long expression for the HPPA.  Called by
   cons via the TC_PARSE_CONS_EXPRESSION macro.  */

void
parse_cons_expression_hppa (exp)
     expressionS *exp;
{
  hppa_field_selector = pa_chk_field_selector (&input_line_pointer);
  expression (exp);
}

/* This fix_new is called by cons via TC_CONS_FIX_NEW.
   hppa_field_selector is set by the parse_cons_expression_hppa.  */

void
cons_fix_new_hppa (frag, where, size, exp)
     fragS *frag;
     int where;
     int size;
     expressionS *exp;
{
  unsigned int rel_type;

  /* Get a base relocation type.  */
  if (is_DP_relative (*exp))
    rel_type = R_HPPA_GOTOFF;
  else if (is_complex (*exp))
    rel_type = R_HPPA_COMPLEX;
  else
    rel_type = R_HPPA;

  if (hppa_field_selector != e_psel && hppa_field_selector != e_fsel)
    as_warn ("Invalid field selector.  Assuming F%%.");

  fix_new_hppa (frag, where, size,
		(symbolS *) NULL, (offsetT) 0, exp, 0, rel_type,
		hppa_field_selector, 32, 0, NULL);

  /* Reset field selector to its default state.  */
  hppa_field_selector = 0;
}

/* This function is called once, at assembler startup time.  It should
   set up all the tables, etc. that the MD part of the assembler will need.  */

void
md_begin ()
{
  const char *retval = NULL;
  int lose = 0;
  unsigned int i = 0;

  last_call_info = NULL;
  call_info_root = NULL;

  /* Set the default machine type.  */
  if (!bfd_set_arch_mach (stdoutput, bfd_arch_hppa, 10))
    as_warn ("could not set architecture and machine");

  /* Folding of text and data segments fails miserably on the PA.
     Warn user and disable "-R" option.  */
  if (flag_readonly_data_in_text)
    {
      as_warn ("-R option not supported on this target.");
      flag_readonly_data_in_text = 0;
    }

  pa_spaces_begin ();

  op_hash = hash_new ();

  while (i < NUMOPCODES)
    {
      const char *name = pa_opcodes[i].name;
      retval = hash_insert (op_hash, name, (struct pa_opcode *) &pa_opcodes[i]);
      if (retval != NULL && *retval != '\0')
	{
	  as_fatal ("Internal error: can't hash `%s': %s\n", name, retval);
	  lose = 1;
	}
      do
	{
	  if ((pa_opcodes[i].match & pa_opcodes[i].mask)
	      != pa_opcodes[i].match)
	    {
	      fprintf (stderr, "internal error: losing opcode: `%s' \"%s\"\n",
		       pa_opcodes[i].name, pa_opcodes[i].args);
	      lose = 1;
	    }
	  ++i;
	}
      while (i < NUMOPCODES && !strcmp (pa_opcodes[i].name, name));
    }

  if (lose)
    as_fatal ("Broken assembler.  No assembly attempted.");

  /* SOM will change text_section.  To make sure we never put
     anything into the old one switch to the new one now.  */
  subseg_set (text_section, 0);

  dummy_symbol = symbol_find_or_make ("L$dummy");
  S_SET_SEGMENT (dummy_symbol, text_section);
}

/* Assemble a single instruction storing it into a frag.  */
void
md_assemble (str)
     char *str;
{
  char *to;

  /* The had better be something to assemble.  */
  assert (str);

  /* If we are within a procedure definition, make sure we've
     defined a label for the procedure; handle case where the
     label was defined after the .PROC directive.

     Note there's not need to diddle with the segment or fragment
     for the label symbol in this case.  We have already switched
     into the new $CODE$ subspace at this point.  */
  if (within_procedure && last_call_info->start_symbol == NULL)
    {
      label_symbol_struct *label_symbol = pa_get_label ();

      if (label_symbol)
	{
	  if (label_symbol->lss_label)
	    {
	      last_call_info->start_symbol = label_symbol->lss_label;
	      label_symbol->lss_label->bsym->flags |= BSF_FUNCTION;
#ifdef OBJ_SOM
	      /* Also handle allocation of a fixup to hold the unwind
		 information when the label appears after the proc/procend.  */
	      if (within_entry_exit)
		{
		  char *where = frag_more (0);

		  fix_new_hppa (frag_now, where - frag_now->fr_literal, 0,
				NULL, (offsetT) 0, NULL,
				0, R_HPPA_ENTRY, e_fsel, 0, 0,
				(int *)&last_call_info->ci_unwind.descriptor);
		}
#endif
	    }
	  else
	    as_bad ("Missing function name for .PROC (corrupted label chain)");
	}
      else
	as_bad ("Missing function name for .PROC");
    }

  /* Assemble the instruction.  Results are saved into "the_insn".  */
  pa_ip (str);

  /* Get somewhere to put the assembled instrution.  */
  to = frag_more (4);

  /* Output the opcode. */
  md_number_to_chars (to, the_insn.opcode, 4);

  /* If necessary output more stuff.  */
  if (the_insn.reloc != R_HPPA_NONE)
    fix_new_hppa (frag_now, (to - frag_now->fr_literal), 4, NULL,
		  (offsetT) 0, &the_insn.exp, the_insn.pcrel,
		  the_insn.reloc, the_insn.field_selector,
		  the_insn.format, the_insn.arg_reloc, NULL);
}

/* Do the real work for assembling a single instruction.  Store results
   into the global "the_insn" variable.  */

static void
pa_ip (str)
     char *str;
{
  char *error_message = "";
  char *s, c, *argstart, *name, *save_s;
  const char *args;
  int match = FALSE;
  int comma = 0;
  int cmpltr, nullif, flag, cond, num;
  unsigned long opcode;
  struct pa_opcode *insn;

  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  /* Skip to something interesting.  */
  for (s = str; isupper (*s) || islower (*s) || (*s >= '0' && *s <= '3'); ++s)
    ;

  switch (*s)
    {

    case '\0':
      break;

    case ',':
      comma = 1;

      /*FALLTHROUGH */

    case ' ':
      *s++ = '\0';
      break;

    default:
      as_fatal ("Unknown opcode: `%s'", str);
    }

  save_s = str;

  /* Convert everything into lower case.  */
  while (*save_s)
    {
      if (isupper (*save_s))
	*save_s = tolower (*save_s);
      save_s++;
    }

  /* Look up the opcode in the has table.  */
  if ((insn = (struct pa_opcode *) hash_find (op_hash, str)) == NULL)
    {
      as_bad ("Unknown opcode: `%s'", str);
      return;
    }

  if (comma)
    {
      *--s = ',';
    }

  /* Mark the location where arguments for the instruction start, then
     start processing them.  */
  argstart = s;
  for (;;)
    {
      /* Do some initialization.  */
      opcode = insn->match;
      bzero (&the_insn, sizeof (the_insn));

      the_insn.reloc = R_HPPA_NONE;

      /* If this instruction is specific to a particular architecture,
	 then set a new architecture.  */
      if (bfd_get_mach (stdoutput) < insn->arch)
	{
	  if (!bfd_set_arch_mach (stdoutput, bfd_arch_hppa, insn->arch))
	    as_warn ("could not update architecture and machine");
	}

      /* Build the opcode, checking as we go to make
         sure that the operands match.  */
      for (args = insn->args;; ++args)
	{
	  switch (*args)
	    {

	    /* End of arguments.  */
	    case '\0':
	      if (*s == '\0')
		match = TRUE;
	      break;

	    case '+':
	      if (*s == '+')
		{
		  ++s;
		  continue;
		}
	      if (*s == '-')
		continue;
	      break;

	    /* These must match exactly.  */
	    case '(':
	    case ')':
	    case ',':
	    case ' ':
	      if (*s++ == *args)
		continue;
	      break;

	    /* Handle a 5 bit register or control register field at 10.  */
	    case 'b':
	    case '^':
	      num = pa_parse_number (&s, 0);
	      CHECK_FIELD (num, 31, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 21);

	    /* Handle a 5 bit register field at 15.  */
	    case 'x':
	      num = pa_parse_number (&s, 0);
	      CHECK_FIELD (num, 31, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 16);

	    /* Handle a 5 bit register field at 31.  */
	    case 'y':
	    case 't':
	      num = pa_parse_number (&s, 0);
	      CHECK_FIELD (num, 31, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 0);

	    /* Handle a 5 bit field length at 31.  */
	    case 'T':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 32, 1, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, 32 - num, 0);

	    /* Handle a 5 bit immediate at 15.  */
	    case '5':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 15, -16, 0);
	      low_sign_unext (num, 5, &num);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 16);

	    /* Handle a 5 bit immediate at 31.  */
	    case 'V':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 15, -16, 0)
		low_sign_unext (num, 5, &num);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 0);

	    /* Handle an unsigned 5 bit immediate at 31.  */
	    case 'r':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 31, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 0);

	    /* Handle an unsigned 5 bit immediate at 15.  */
	    case 'R':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 31, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 16);

	    /* Handle a 2 bit space identifier at 17.  */
	    case 's':
	      num = pa_parse_number (&s, 0);
	      CHECK_FIELD (num, 3, 0, 1);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 14);

	    /* Handle a 3 bit space identifier at 18.  */
	    case 'S':
	      num = pa_parse_number (&s, 0);
	      CHECK_FIELD (num, 7, 0, 1);
	      dis_assemble_3 (num, &num);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 13);

	    /* Handle a completer for an indexing load or store.  */
	    case 'c':
	      {
		int uu = 0;
		int m = 0;
		int i = 0;
		while (*s == ',' && i < 2)
		  {
		    s++;
		    if (strncasecmp (s, "sm", 2) == 0)
		      {
			uu = 1;
			m = 1;
			s++;
			i++;
		      }
		    else if (strncasecmp (s, "m", 1) == 0)
		      m = 1;
		    else if (strncasecmp (s, "s", 1) == 0)
		      uu = 1;
		    else
		      as_bad ("Invalid Indexed Load Completer.");
		    s++;
		    i++;
		  }
		if (i > 2)
		  as_bad ("Invalid Indexed Load Completer Syntax.");
		opcode |= m << 5;
		INSERT_FIELD_AND_CONTINUE (opcode, uu, 13);
	      }

	    /* Handle a short load/store completer.  */
	    case 'C':
	      {
		int a = 0;
		int m = 0;
		if (*s == ',')
		  {
		    s++;
		    if (strncasecmp (s, "ma", 2) == 0)
		      {
			a = 0;
			m = 1;
		      }
		    else if (strncasecmp (s, "mb", 2) == 0)
		      {
			a = 1;
			m = 1;
		      }
		    else
		      as_bad ("Invalid Short Load/Store Completer.");
		    s += 2;
		  }
		opcode |= m << 5;
		INSERT_FIELD_AND_CONTINUE (opcode, a, 13);
	      }

	    /* Handle a stbys completer.  */
	    case 'Y':
	      {
		int a = 0;
		int m = 0;
		int i = 0;
		while (*s == ',' && i < 2)
		  {
		    s++;
		    if (strncasecmp (s, "m", 1) == 0)
		      m = 1;
		    else if (strncasecmp (s, "b", 1) == 0)
		      a = 0;
		    else if (strncasecmp (s, "e", 1) == 0)
		      a = 1;
		    else
		      as_bad ("Invalid Store Bytes Short Completer");
		    s++;
		    i++;
		  }
		if (i > 2)
		  as_bad ("Invalid Store Bytes Short Completer");
		opcode |= m << 5;
		INSERT_FIELD_AND_CONTINUE (opcode, a, 13);
	      }

	    /* Handle a non-negated compare/stubtract condition.  */
	    case '<':
	      cmpltr = pa_parse_nonneg_cmpsub_cmpltr (&s, 1);
	      if (cmpltr < 0)
		{
		  as_bad ("Invalid Compare/Subtract Condition: %c", *s);
		  cmpltr = 0;
		}
	      INSERT_FIELD_AND_CONTINUE (opcode, cmpltr, 13);

	    /* Handle a negated or non-negated compare/subtract condition.  */
	    case '?':
	      save_s = s;
	      cmpltr = pa_parse_nonneg_cmpsub_cmpltr (&s, 1);
	      if (cmpltr < 0)
		{
		  s = save_s;
		  cmpltr = pa_parse_neg_cmpsub_cmpltr (&s, 1);
		  if (cmpltr < 0)
		    {
		      as_bad ("Invalid Compare/Subtract Condition.");
		      cmpltr = 0;
		    }
		  else
		    {
		      /* Negated condition requires an opcode change.  */
		      opcode |= 1 << 27;
		    }
		}
	      INSERT_FIELD_AND_CONTINUE (opcode, cmpltr, 13);

	    /* Handle non-negated add condition.  */
	    case '!':
	      cmpltr = pa_parse_nonneg_add_cmpltr (&s, 1);
	      if (cmpltr < 0)
		{
		  as_bad ("Invalid Compare/Subtract Condition: %c", *s);
		  cmpltr = 0;
		}
	      INSERT_FIELD_AND_CONTINUE (opcode, cmpltr, 13);

	    /* Handle a negated or non-negated add condition.  */
	    case '@':
	      save_s = s;
	      cmpltr = pa_parse_nonneg_add_cmpltr (&s, 1);
	      if (cmpltr < 0)
		{
		  s = save_s;
		  cmpltr = pa_parse_neg_add_cmpltr (&s, 1);
		  if (cmpltr < 0)
		    {
		      as_bad ("Invalid Compare/Subtract Condition");
		      cmpltr = 0;
		    }
		  else
		    {
		      /* Negated condition requires an opcode change.  */
		      opcode |= 1 << 27;
		    }
		}
	      INSERT_FIELD_AND_CONTINUE (opcode, cmpltr, 13);

	    /* Handle a compare/subtract condition.  */
	    case 'a':
	      cmpltr = 0;
	      flag = 0;
	      save_s = s;
	      if (*s == ',')
		{
		  cmpltr = pa_parse_nonneg_cmpsub_cmpltr (&s, 0);
		  if (cmpltr < 0)
		    {
		      flag = 1;
		      s = save_s;
		      cmpltr = pa_parse_neg_cmpsub_cmpltr (&s, 0);
		      if (cmpltr < 0)
			{
			  as_bad ("Invalid Compare/Subtract Condition");
			}
		    }
		}
	      opcode |= cmpltr << 13;
	      INSERT_FIELD_AND_CONTINUE (opcode, flag, 12);

	    /* Handle a non-negated add condition.  */
	    case 'd':
	      cmpltr = 0;
	      nullif = 0;
	      flag = 0;
	      if (*s == ',')
		{
		  s++;
		  name = s;
		  while (*s != ',' && *s != ' ' && *s != '\t')
		    s += 1;
		  c = *s;
		  *s = 0x00;
		  if (strcmp (name, "=") == 0)
		    cmpltr = 1;
		  else if (strcmp (name, "<") == 0)
		    cmpltr = 2;
		  else if (strcmp (name, "<=") == 0)
		    cmpltr = 3;
		  else if (strcasecmp (name, "nuv") == 0)
		    cmpltr = 4;
		  else if (strcasecmp (name, "znv") == 0)
		    cmpltr = 5;
		  else if (strcasecmp (name, "sv") == 0)
		    cmpltr = 6;
		  else if (strcasecmp (name, "od") == 0)
		    cmpltr = 7;
		  else if (strcasecmp (name, "n") == 0)
		    nullif = 1;
		  else if (strcasecmp (name, "tr") == 0)
		    {
		      cmpltr = 0;
		      flag = 1;
		    }
		  else if (strcmp (name, "<>") == 0)
		    {
		      cmpltr = 1;
		      flag = 1;
		    }
		  else if (strcmp (name, ">=") == 0)
		    {
		      cmpltr = 2;
		      flag = 1;
		    }
		  else if (strcmp (name, ">") == 0)
		    {
		      cmpltr = 3;
		      flag = 1;
		    }
		  else if (strcasecmp (name, "uv") == 0)
		    {
		      cmpltr = 4;
		      flag = 1;
		    }
		  else if (strcasecmp (name, "vnz") == 0)
		    {
		      cmpltr = 5;
		      flag = 1;
		    }
		  else if (strcasecmp (name, "nsv") == 0)
		    {
		      cmpltr = 6;
		      flag = 1;
		    }
		  else if (strcasecmp (name, "ev") == 0)
		    {
		      cmpltr = 7;
		      flag = 1;
		    }
		  else
		    as_bad ("Invalid Add Condition: %s", name);
		  *s = c;
		}
	      nullif = pa_parse_nullif (&s);
	      opcode |= nullif << 1;
	      opcode |= cmpltr << 13;
	      INSERT_FIELD_AND_CONTINUE (opcode, flag, 12);

	    /* HANDLE a logical instruction condition.  */
	    case '&':
	      cmpltr = 0;
	      flag = 0;
	      if (*s == ',')
		{
		  s++;
		  name = s;
		  while (*s != ',' && *s != ' ' && *s != '\t')
		    s += 1;
		  c = *s;
		  *s = 0x00;
		  if (strcmp (name, "=") == 0)
		    cmpltr = 1;
		  else if (strcmp (name, "<") == 0)
		    cmpltr = 2;
		  else if (strcmp (name, "<=") == 0)
		    cmpltr = 3;
		  else if (strcasecmp (name, "od") == 0)
		    cmpltr = 7;
		  else if (strcasecmp (name, "tr") == 0)
		    {
		      cmpltr = 0;
		      flag = 1;
		    }
		  else if (strcmp (name, "<>") == 0)
		    {
		      cmpltr = 1;
		      flag = 1;
		    }
		  else if (strcmp (name, ">=") == 0)
		    {
		      cmpltr = 2;
		      flag = 1;
		    }
		  else if (strcmp (name, ">") == 0)
		    {
		      cmpltr = 3;
		      flag = 1;
		    }
		  else if (strcasecmp (name, "ev") == 0)
		    {
		      cmpltr = 7;
		      flag = 1;
		    }
		  else
		    as_bad ("Invalid Logical Instruction Condition.");
		  *s = c;
		}
	      opcode |= cmpltr << 13;
	      INSERT_FIELD_AND_CONTINUE (opcode, flag, 12);

	    /* Handle a unit instruction condition.  */
	    case 'U':
	      cmpltr = 0;
	      flag = 0;
	      if (*s == ',')
		{
		  s++;
		  if (strncasecmp (s, "sbz", 3) == 0)
		    {
		      cmpltr = 2;
		      s += 3;
		    }
		  else if (strncasecmp (s, "shz", 3) == 0)
		    {
		      cmpltr = 3;
		      s += 3;
		    }
		  else if (strncasecmp (s, "sdc", 3) == 0)
		    {
		      cmpltr = 4;
		      s += 3;
		    }
		  else if (strncasecmp (s, "sbc", 3) == 0)
		    {
		      cmpltr = 6;
		      s += 3;
		    }
		  else if (strncasecmp (s, "shc", 3) == 0)
		    {
		      cmpltr = 7;
		      s += 3;
		    }
		  else if (strncasecmp (s, "tr", 2) == 0)
		    {
		      cmpltr = 0;
		      flag = 1;
		      s += 2;
		    }
		  else if (strncasecmp (s, "nbz", 3) == 0)
		    {
		      cmpltr = 2;
		      flag = 1;
		      s += 3;
		    }
		  else if (strncasecmp (s, "nhz", 3) == 0)
		    {
		      cmpltr = 3;
		      flag = 1;
		      s += 3;
		    }
		  else if (strncasecmp (s, "ndc", 3) == 0)
		    {
		      cmpltr = 4;
		      flag = 1;
		      s += 3;
		    }
		  else if (strncasecmp (s, "nbc", 3) == 0)
		    {
		      cmpltr = 6;
		      flag = 1;
		      s += 3;
		    }
		  else if (strncasecmp (s, "nhc", 3) == 0)
		    {
		      cmpltr = 7;
		      flag = 1;
		      s += 3;
		    }
		  else
		    as_bad ("Invalid Logical Instruction Condition.");
		}
	      opcode |= cmpltr << 13;
	      INSERT_FIELD_AND_CONTINUE (opcode, flag, 12);

	    /* Handle a shift/extract/deposit condition.  */
	    case '|':
	    case '>':
	      cmpltr = 0;
	      if (*s == ',')
		{
		  save_s = s++;
		  name = s;
		  while (*s != ',' && *s != ' ' && *s != '\t')
		    s += 1;
		  c = *s;
		  *s = 0x00;
		  if (strcmp (name, "=") == 0)
		    cmpltr = 1;
		  else if (strcmp (name, "<") == 0)
		    cmpltr = 2;
		  else if (strcasecmp (name, "od") == 0)
		    cmpltr = 3;
		  else if (strcasecmp (name, "tr") == 0)
		    cmpltr = 4;
		  else if (strcmp (name, "<>") == 0)
		    cmpltr = 5;
		  else if (strcmp (name, ">=") == 0)
		    cmpltr = 6;
		  else if (strcasecmp (name, "ev") == 0)
		    cmpltr = 7;
		  /* Handle movb,n.  Put things back the way they were.
		     This includes moving s back to where it started.  */
		  else if (strcasecmp (name, "n") == 0 && *args == '|')
		    {
		      *s = c;
		      s = save_s;
		      continue;
		    }
		  else
		    as_bad ("Invalid Shift/Extract/Deposit Condition.");
		  *s = c;
		}
	      INSERT_FIELD_AND_CONTINUE (opcode, cmpltr, 13);

	    /* Handle bvb and bb conditions.  */
	    case '~':
	      cmpltr = 0;
	      if (*s == ',')
		{
		  s++;
		  if (strncmp (s, "<", 1) == 0)
		    {
		      cmpltr = 2;
		      s++;
		    }
		  else if (strncmp (s, ">=", 2) == 0)
		    {
		      cmpltr = 6;
		      s += 2;
		    }
		  else
		    as_bad ("Invalid Bit Branch Condition: %c", *s);
		}
	      INSERT_FIELD_AND_CONTINUE (opcode, cmpltr, 13);

	    /* Handle a system control completer.  */
	    case 'Z':
	      if (*s == ',' && (*(s + 1) == 'm' || *(s + 1) == 'M'))
		{
		  flag = 1;
		  s += 2;
		}
	      else
		flag = 0;

	      INSERT_FIELD_AND_CONTINUE (opcode, flag, 5);

	    /* Handle a nullification completer for branch instructions.  */
	    case 'n':
	      nullif = pa_parse_nullif (&s);
	      INSERT_FIELD_AND_CONTINUE (opcode, nullif, 1);

	    /* Handle a nullification completer for copr and spop insns.  */
	    case 'N':
	      nullif = pa_parse_nullif (&s);
	      INSERT_FIELD_AND_CONTINUE (opcode, nullif, 5);

	    /* Handle a 11 bit immediate at 31.  */
	    case 'i':
	      the_insn.field_selector = pa_chk_field_selector (&s);
	      get_expression (s);
	      s = expr_end;
	      if (the_insn.exp.X_op == O_constant)
		{
		  num = evaluate_absolute (&the_insn);
		  CHECK_FIELD (num, 1023, -1024, 0);
		  low_sign_unext (num, 11, &num);
		  INSERT_FIELD_AND_CONTINUE (opcode, num, 0);
		}
	      else
		{
		  if (is_DP_relative (the_insn.exp))
		    the_insn.reloc = R_HPPA_GOTOFF;
		  else if (is_PC_relative (the_insn.exp))
		    the_insn.reloc = R_HPPA_PCREL_CALL;
		  else
		    the_insn.reloc = R_HPPA;
		  the_insn.format = 11;
		  continue;
		}

	    /* Handle a 14 bit immediate at 31.  */
	    case 'j':
	      the_insn.field_selector = pa_chk_field_selector (&s);
	      get_expression (s);
	      s = expr_end;
	      if (the_insn.exp.X_op == O_constant)
		{
		  num = evaluate_absolute (&the_insn);
		  CHECK_FIELD (num, 8191, -8192, 0);
		  low_sign_unext (num, 14, &num);
		  INSERT_FIELD_AND_CONTINUE (opcode, num, 0);
		}
	      else
		{
		  if (is_DP_relative (the_insn.exp))
		    the_insn.reloc = R_HPPA_GOTOFF;
		  else if (is_PC_relative (the_insn.exp))
		    the_insn.reloc = R_HPPA_PCREL_CALL;
		  else
		    the_insn.reloc = R_HPPA;
		  the_insn.format = 14;
		  continue;
		}

	    /* Handle a 21 bit immediate at 31.  */
	    case 'k':
	      the_insn.field_selector = pa_chk_field_selector (&s);
	      get_expression (s);
	      s = expr_end;
	      if (the_insn.exp.X_op == O_constant)
		{
		  num = evaluate_absolute (&the_insn);
		  CHECK_FIELD (num >> 11, 1048575, -1048576, 0);
		  dis_assemble_21 (num, &num);
		  INSERT_FIELD_AND_CONTINUE (opcode, num, 0);
		}
	      else
		{
		  if (is_DP_relative (the_insn.exp))
		    the_insn.reloc = R_HPPA_GOTOFF;
		  else if (is_PC_relative (the_insn.exp))
		    the_insn.reloc = R_HPPA_PCREL_CALL;
		  else
		    the_insn.reloc = R_HPPA;
		  the_insn.format = 21;
		  continue;
		}

	    /* Handle a 12 bit branch displacement.  */
	    case 'w':
	      the_insn.field_selector = pa_chk_field_selector (&s);
	      get_expression (s);
	      s = expr_end;
	      the_insn.pcrel = 1;
	      if (!strcmp (S_GET_NAME (the_insn.exp.X_add_symbol), "L$0\001"))
		{
		  unsigned int w1, w, result;

		  num = evaluate_absolute (&the_insn);
		  if (num % 4)
		    {
		      as_bad ("Branch to unaligned address");
		      break;
		    }
		  CHECK_FIELD (num, 8199, -8184, 0);
		  sign_unext ((num - 8) >> 2, 12, &result);
		  dis_assemble_12 (result, &w1, &w);
		  INSERT_FIELD_AND_CONTINUE (opcode, ((w1 << 2) | w), 0);
		}
	      else
		{
		  the_insn.reloc = R_HPPA_PCREL_CALL;
		  the_insn.format = 12;
		  the_insn.arg_reloc = last_call_desc.arg_reloc;
		  bzero (&last_call_desc, sizeof (struct call_desc));
		  s = expr_end;
		  continue;
		}

	    /* Handle a 17 bit branch displacement.  */
	    case 'W':
	      the_insn.field_selector = pa_chk_field_selector (&s);
	      get_expression (s);
	      s = expr_end;
	      the_insn.pcrel = 1;
	      if (!the_insn.exp.X_add_symbol
		  || !strcmp (S_GET_NAME (the_insn.exp.X_add_symbol),
			      "L$0\001"))
		{
		  unsigned int w2, w1, w, result;

		  num = evaluate_absolute (&the_insn);
		  if (num % 4)
		    {
		      as_bad ("Branch to unaligned address");
		      break;
		    }
		  CHECK_FIELD (num, 262143, -262144, 0);

		  if (the_insn.exp.X_add_symbol)
		    num -= 8;

		  sign_unext (num >> 2, 17, &result);
		  dis_assemble_17 (result, &w1, &w2, &w);
		  INSERT_FIELD_AND_CONTINUE (opcode,
					   ((w2 << 2) | (w1 << 16) | w), 0);
		}
	      else
		{
		  the_insn.reloc = R_HPPA_PCREL_CALL;
		  the_insn.format = 17;
		  the_insn.arg_reloc = last_call_desc.arg_reloc;
		  bzero (&last_call_desc, sizeof (struct call_desc));
		  continue;
		}

	    /* Handle an absolute 17 bit branch target.  */
	    case 'z':
	      the_insn.field_selector = pa_chk_field_selector (&s);
	      get_expression (s);
	      s = expr_end;
	      the_insn.pcrel = 0;
	      if (!the_insn.exp.X_add_symbol
		  || !strcmp (S_GET_NAME (the_insn.exp.X_add_symbol),
			      "L$0\001"))
		{
		  unsigned int w2, w1, w, result;

		  num = evaluate_absolute (&the_insn);
		  if (num % 4)
		    {
		      as_bad ("Branch to unaligned address");
		      break;
		    }
		  CHECK_FIELD (num, 262143, -262144, 0);

		  if (the_insn.exp.X_add_symbol)
		    num -= 8;

		  sign_unext (num >> 2, 17, &result);
		  dis_assemble_17 (result, &w1, &w2, &w);
		  INSERT_FIELD_AND_CONTINUE (opcode,
					   ((w2 << 2) | (w1 << 16) | w), 0);
		}
	      else
		{
		  the_insn.reloc = R_HPPA_ABS_CALL;
		  the_insn.format = 17;
		  the_insn.arg_reloc = last_call_desc.arg_reloc;
		  bzero (&last_call_desc, sizeof (struct call_desc));
		  continue;
		}

	    /* Handle a 5 bit shift count at 26.  */
	    case 'p':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 31, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, 31 - num, 5);

	    /* Handle a 5 bit bit position at 26.  */
	    case 'P':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 31, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 5);

	    /* Handle a 5 bit immediate at 10.  */
	    case 'Q':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 31, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 21);

	    /* Handle a 13 bit immediate at 18.  */
	    case 'A':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 8191, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 13);

	    /* Handle a 26 bit immediate at 31.  */
	    case 'D':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 671108864, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 0);

	    /* Handle a 3 bit SFU identifier at 25.  */
	    case 'f':
	      if (*s++ != ',')
		as_bad ("Invalid SFU identifier");
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 7, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 6);

	    /* Handle a 20 bit SOP field for spop0.  */
	    case 'O':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 1048575, 0, 0);
	      num = (num & 0x1f) | ((num & 0x000fffe0) << 6);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 0);

	    /* Handle a 15bit SOP field for spop1.  */
	    case 'o':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 32767, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 11);

	    /* Handle a 10bit SOP field for spop3.  */
	    case '0':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 1023, 0, 0);
	      num = (num & 0x1f) | ((num & 0x000003e0) << 6);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 0);

	    /* Handle a 15 bit SOP field for spop2.  */
	    case '1':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 32767, 0, 0);
	      num = (num & 0x1f) | ((num & 0x00007fe0) << 6);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 0);

	    /* Handle a 3-bit co-processor ID field.  */
	    case 'u':
	      if (*s++ != ',')
		as_bad ("Invalid COPR identifier");
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 7, 0, 0);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 6);

	    /* Handle a 22bit SOP field for copr.  */
	    case '2':
	      num = pa_get_absolute_expression (&the_insn, &s);
	      s = expr_end;
	      CHECK_FIELD (num, 4194303, 0, 0);
	      num = (num & 0x1f) | ((num & 0x003fffe0) << 4);
	      INSERT_FIELD_AND_CONTINUE (opcode, num, 0);

	    /* Handle a source FP operand format completer.  */
	    case 'F':
	      flag = pa_parse_fp_format (&s);
	      the_insn.fpof1 = flag;
	      INSERT_FIELD_AND_CONTINUE (opcode, flag, 11);

	    /* Handle a destination FP operand format completer.  */
	    case 'G':
	      /* pa_parse_format needs the ',' prefix.  */
	      s--;
	      flag = pa_parse_fp_format (&s);
	      the_insn.fpof2 = flag;
	      INSERT_FIELD_AND_CONTINUE (opcode, flag, 13);

	    /* Handle FP compare conditions.  */
	    case 'M':
	      cond = pa_parse_fp_cmp_cond (&s);
	      INSERT_FIELD_AND_CONTINUE (opcode, cond, 0);

	    /* Handle L/R register halves like 't'.  */
	    case 'v':
	      {
		struct pa_11_fp_reg_struct result;

		pa_parse_number (&s, &result);
		CHECK_FIELD (result.number_part, 31, 0, 0);
		opcode |= result.number_part;

		/* 0x30 opcodes are FP arithmetic operation opcodes
		   and need to be turned into 0x38 opcodes.  This
		   is not necessary for loads/stores.  */
		if (need_pa11_opcode (&the_insn, &result)
		    && ((opcode & 0xfc000000) == 0x30000000))
		  opcode |= 1 << 27;

		INSERT_FIELD_AND_CONTINUE (opcode, result.l_r_select & 1, 6);
	      }

	    /* Handle L/R register halves like 'b'.  */
	    case 'E':
	      {
		struct pa_11_fp_reg_struct result;

		pa_parse_number (&s, &result);
		CHECK_FIELD (result.number_part, 31, 0, 0);
		opcode |= result.number_part << 21;
		if (need_pa11_opcode (&the_insn, &result))
		  {
		    opcode |= (result.l_r_select & 1) << 7;
		    opcode |= 1 << 27;
		  }
		continue;
	      }

	    /* Handle L/R register halves like 'x'.  */
	    case 'X':
	      {
		struct pa_11_fp_reg_struct result;

		pa_parse_number (&s, &result);
		CHECK_FIELD (result.number_part, 31, 0, 0);
		opcode |= (result.number_part & 0x1f) << 16;
		if (need_pa11_opcode (&the_insn, &result))
		  {
		    opcode |= (result.l_r_select & 1) << 12;
		    opcode |= 1 << 27;
		  }
		continue;
	      }

	    /* Handle a 5 bit register field at 10.  */
	    case '4':
	      {
		struct pa_11_fp_reg_struct result;

		pa_parse_number (&s, &result);
		CHECK_FIELD (result.number_part, 31, 0, 0);
		if (the_insn.fpof1 == SGL)
		  {
		    if (result.number_part < 16)
		      {
			as_bad  ("Invalid register for single precision fmpyadd or fmpysub");
			break;
		      }

		    result.number_part &= 0xF;
		    result.number_part |= (result.l_r_select & 1) << 4;
		  }
		INSERT_FIELD_AND_CONTINUE (opcode, result.number_part, 21);
	      }

	    /* Handle a 5 bit register field at 15.  */
	    case '6':
	      {
		struct pa_11_fp_reg_struct result;

		pa_parse_number (&s, &result);
		CHECK_FIELD (result.number_part, 31, 0, 0);
		if (the_insn.fpof1 == SGL)
		  {
		    if (result.number_part < 16)
		      {
			as_bad  ("Invalid register for single precision fmpyadd or fmpysub");
			break;
		      }
		    result.number_part &= 0xF;
		    result.number_part |= (result.l_r_select & 1) << 4;
		  }
		INSERT_FIELD_AND_CONTINUE (opcode, result.number_part, 16);
	      }

	    /* Handle a 5 bit register field at 31.  */
	    case '7':
	      {
		struct pa_11_fp_reg_struct result;

		pa_parse_number (&s, &result);
		CHECK_FIELD (result.number_part, 31, 0, 0);
		if (the_insn.fpof1 == SGL)
		  {
		    if (result.number_part < 16)
		      {
			as_bad  ("Invalid register for single precision fmpyadd or fmpysub");
			break;
		      }
		    result.number_part &= 0xF;
		    result.number_part |= (result.l_r_select & 1) << 4;
		  }
		INSERT_FIELD_AND_CONTINUE (opcode, result.number_part, 0);
	      }

	    /* Handle a 5 bit register field at 20.  */
	    case '8':
	      {
		struct pa_11_fp_reg_struct result;

		pa_parse_number (&s, &result);
		CHECK_FIELD (result.number_part, 31, 0, 0);
		if (the_insn.fpof1 == SGL)
		  {
		    if (result.number_part < 16)
		      {
			as_bad  ("Invalid register for single precision fmpyadd or fmpysub");
			break;
		      }
		    result.number_part &= 0xF;
		    result.number_part |= (result.l_r_select & 1) << 4;
		  }
		INSERT_FIELD_AND_CONTINUE (opcode, result.number_part, 11);
	      }

	    /* Handle a 5 bit register field at 25.  */
	    case '9':
	      {
		struct pa_11_fp_reg_struct result;

		pa_parse_number (&s, &result);
		CHECK_FIELD (result.number_part, 31, 0, 0);
		if (the_insn.fpof1 == SGL)
		  {
		    if (result.number_part < 16)
		      {
			as_bad  ("Invalid register for single precision fmpyadd or fmpysub");
			break;
		      }
		    result.number_part &= 0xF;
		    result.number_part |= (result.l_r_select & 1) << 4;
		  }
		INSERT_FIELD_AND_CONTINUE (opcode, result.number_part, 6);
	      }

	    /* Handle a floating point operand format at 26.
	       Only allows single and double precision.  */
	    case 'H':
	      flag = pa_parse_fp_format (&s);
	      switch (flag)
		{
		case SGL:
		  opcode |= 0x20;
		case DBL:
		  the_insn.fpof1 = flag;
		  continue;

		case QUAD:
		case ILLEGAL_FMT:
		default:
		  as_bad ("Invalid Floating Point Operand Format.");
		}
	      break;

	    default:
	      abort ();
	    }
	  break;
	}

      /* Check if the args matched.  */
      if (match == FALSE)
	{
	  if (&insn[1] - pa_opcodes < NUMOPCODES
	      && !strcmp (insn->name, insn[1].name))
	    {
	      ++insn;
	      s = argstart;
	      continue;
	    }
	  else
	    {
	      as_bad ("Invalid operands %s", error_message);
	      return;
	    }
	}
      break;
    }

  the_insn.opcode = opcode;
}

/* Turn a string in input_line_pointer into a floating point constant of type
   type, and store the appropriate bytes in *litP.  The number of LITTLENUMS
   emitted is stored in *sizeP .  An error message or NULL is returned.  */

#define MAX_LITTLENUMS 6

char *
md_atof (type, litP, sizeP)
     char type;
     char *litP;
     int *sizeP;
{
  int prec;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  LITTLENUM_TYPE *wordP;
  char *t;

  switch (type)
    {

    case 'f':
    case 'F':
    case 's':
    case 'S':
      prec = 2;
      break;

    case 'd':
    case 'D':
    case 'r':
    case 'R':
      prec = 4;
      break;

    case 'x':
    case 'X':
      prec = 6;
      break;

    case 'p':
    case 'P':
      prec = 6;
      break;

    default:
      *sizeP = 0;
      return "Bad call to MD_ATOF()";
    }
  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;
  *sizeP = prec * sizeof (LITTLENUM_TYPE);
  for (wordP = words; prec--;)
    {
      md_number_to_chars (litP, (valueT) (*wordP++), sizeof (LITTLENUM_TYPE));
      litP += sizeof (LITTLENUM_TYPE);
    }
  return NULL;
}

/* Write out big-endian.  */

void
md_number_to_chars (buf, val, n)
     char *buf;
     valueT val;
     int n;
{
  number_to_chars_bigendian (buf, val, n);
}

/* Translate internal representation of relocation info to BFD target
   format.  */

arelent **
tc_gen_reloc (section, fixp)
     asection *section;
     fixS *fixp;
{
  arelent *reloc;
  struct hppa_fix_struct *hppa_fixp;
  bfd_reloc_code_real_type code;
  static arelent *no_relocs = NULL;
  arelent **relocs;
  bfd_reloc_code_real_type **codes;
  int n_relocs;
  int i;

  hppa_fixp = (struct hppa_fix_struct *) fixp->tc_fix_data;
  if (fixp->fx_addsy == 0)
    return &no_relocs;
  assert (hppa_fixp != 0);
  assert (section != 0);

  reloc = (arelent *) bfd_alloc_by_size_t (stdoutput, sizeof (arelent));
  assert (reloc != 0);

  reloc->sym_ptr_ptr = &fixp->fx_addsy->bsym;
  codes = (bfd_reloc_code_real_type **) hppa_gen_reloc_type (stdoutput,
			       fixp->fx_r_type,
			       hppa_fixp->fx_r_format,
			       hppa_fixp->fx_r_field,
			       fixp->fx_subsy != NULL);

  for (n_relocs = 0; codes[n_relocs]; n_relocs++)
    ;

  relocs = (arelent **)
    bfd_alloc_by_size_t (stdoutput, sizeof (arelent *) * n_relocs + 1);
  assert (relocs != 0);

  reloc = (arelent *) bfd_alloc_by_size_t (stdoutput,
					   sizeof (arelent) * n_relocs);
  if (n_relocs > 0)
    assert (reloc != 0);

  for (i = 0; i < n_relocs; i++)
    relocs[i] = &reloc[i];

  relocs[n_relocs] = NULL;

#ifdef OBJ_ELF
  switch (fixp->fx_r_type)
    {
    default:
      assert (n_relocs == 1);

      code = *codes[0];

      reloc->sym_ptr_ptr = &fixp->fx_addsy->bsym;
      reloc->howto = bfd_reloc_type_lookup (stdoutput, code);
      reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
      reloc->addend = 0;	/* default */

      assert (reloc->howto && code == reloc->howto->type);

      /* Now, do any processing that is dependent on the relocation type.  */
      switch (code)
	{
	case R_PARISC_DLTREL21L:
	case R_PARISC_DLTREL14R:
	case R_PARISC_DLTREL14F:
	case R_PARISC_PLABEL32:
	case R_PARISC_PLABEL21L:
	case R_PARISC_PLABEL14R:
	  /* For plabel relocations, the addend of the
	     relocation should be either 0 (no static link) or 2
	     (static link required).

	     FIXME: We always assume no static link!

	     We also slam a zero addend into the DLT relative relocs;
	     it doesn't make a lot of sense to use any addend since
	     it gets you a different (eg unknown) DLT entry.  */
	  reloc->addend = 0;
	  break;

	case R_PARISC_PCREL21L:
	case R_PARISC_PCREL17R:
	case R_PARISC_PCREL17F:
	case R_PARISC_PCREL17C:
	case R_PARISC_PCREL14R:
	case R_PARISC_PCREL14F:
	  /* The constant is stored in the instruction.  */
	  reloc->addend = HPPA_R_ADDEND (hppa_fixp->fx_arg_reloc, 0);
	  break;
	default:
	  reloc->addend = fixp->fx_offset;
	  break;
	}
      break;
    }
#else /* OBJ_SOM */

  /* Walk over reach relocation returned by the BFD backend.  */
  for (i = 0; i < n_relocs; i++)
    {
      code = *codes[i];

      relocs[i]->sym_ptr_ptr = &fixp->fx_addsy->bsym;
      relocs[i]->howto = bfd_reloc_type_lookup (stdoutput, code);
      relocs[i]->address = fixp->fx_frag->fr_address + fixp->fx_where;

      switch (code)
	{
	case R_COMP2:
	  /* The only time we ever use a R_COMP2 fixup is for the difference
	     of two symbols.  With that in mind we fill in all four
	     relocs now and break out of the loop.  */
	  assert (i == 1);
	  relocs[0]->sym_ptr_ptr = &bfd_abs_symbol;
	  relocs[0]->howto = bfd_reloc_type_lookup (stdoutput, *codes[0]);
	  relocs[0]->address = fixp->fx_frag->fr_address + fixp->fx_where;
	  relocs[0]->addend = 0;
	  relocs[1]->sym_ptr_ptr = &fixp->fx_addsy->bsym;
	  relocs[1]->howto = bfd_reloc_type_lookup (stdoutput, *codes[1]);
	  relocs[1]->address = fixp->fx_frag->fr_address + fixp->fx_where;
	  relocs[1]->addend = 0;
	  relocs[2]->sym_ptr_ptr = &fixp->fx_subsy->bsym;
	  relocs[2]->howto = bfd_reloc_type_lookup (stdoutput, *codes[2]);
	  relocs[2]->address = fixp->fx_frag->fr_address + fixp->fx_where;
	  relocs[2]->addend = 0;
	  relocs[3]->sym_ptr_ptr = &bfd_abs_symbol;
	  relocs[3]->howto = bfd_reloc_type_lookup (stdoutput, *codes[3]);
	  relocs[3]->address = fixp->fx_frag->fr_address + fixp->fx_where;
	  relocs[3]->addend = 0;
	  relocs[4]->sym_ptr_ptr = &bfd_abs_symbol;
	  relocs[4]->howto = bfd_reloc_type_lookup (stdoutput, *codes[4]);
	  relocs[4]->address = fixp->fx_frag->fr_address + fixp->fx_where;
	  relocs[4]->addend = 0;
	  goto done;
	case R_PCREL_CALL:
	case R_ABS_CALL:
	  relocs[i]->addend = HPPA_R_ADDEND (hppa_fixp->fx_arg_reloc, 0);
	  break;

	case R_DLT_REL:
	case R_DATA_PLABEL:
	case R_CODE_PLABEL:
	  /* For plabel relocations, the addend of the
	     relocation should be either 0 (no static link) or 2
	     (static link required).

	     FIXME: We always assume no static link!

	     We also slam a zero addend into the DLT relative relocs;
	     it doesn't make a lot of sense to use any addend since
	     it gets you a different (eg unknown) DLT entry.  */
	  relocs[i]->addend = 0;
	  break;

	case R_N_MODE:
	case R_S_MODE:
	case R_D_MODE:
	case R_R_MODE:
	case R_FSEL:
	case R_LSEL:
	case R_RSEL:
	case R_BEGIN_BRTAB:
	case R_END_BRTAB:
	case R_BEGIN_TRY:
	case R_N0SEL:
	case R_N1SEL:
	  /* There is no symbol or addend associated with these fixups.  */
	  relocs[i]->sym_ptr_ptr = &dummy_symbol->bsym;
	  relocs[i]->addend = 0;
	  break;

	case R_END_TRY:
	case R_ENTRY:
	case R_EXIT:
	  /* There is no symbol associated with these fixups.  */
	  relocs[i]->sym_ptr_ptr = &dummy_symbol->bsym;
	  relocs[i]->addend = fixp->fx_offset;
	  break;

	default:
	  relocs[i]->addend = fixp->fx_offset;
	}
    }

 done:
#endif

  return relocs;
}

/* Process any machine dependent frag types.  */

void
md_convert_frag (abfd, sec, fragP)
     register bfd *abfd;
     register asection *sec;
     register fragS *fragP;
{
  unsigned int address;

  if (fragP->fr_type == rs_machine_dependent)
    {
      switch ((int) fragP->fr_subtype)
	{
	case 0:
	  fragP->fr_type = rs_fill;
	  know (fragP->fr_var == 1);
	  know (fragP->fr_next);
	  address = fragP->fr_address + fragP->fr_fix;
	  if (address % fragP->fr_offset)
	    {
	      fragP->fr_offset =
		fragP->fr_next->fr_address
		- fragP->fr_address
		- fragP->fr_fix;
	    }
	  else
	    fragP->fr_offset = 0;
	  break;
	}
    }
}

/* Round up a section size to the appropriate boundary. */

valueT
md_section_align (segment, size)
     asection *segment;
     valueT size;
{
  int align = bfd_get_section_alignment (stdoutput, segment);
  int align2 = (1 << align) - 1;

  return (size + align2) & ~align2;
}

/* Create a short jump from FROM_ADDR to TO_ADDR.  Not used on the PA.  */
void
md_create_short_jump (ptr, from_addr, to_addr, frag, to_symbol)
     char *ptr;
     addressT from_addr, to_addr;
     fragS *frag;
     symbolS *to_symbol;
{
  fprintf (stderr, "pa_create_short_jmp\n");
  abort ();
}

/* Create a long jump from FROM_ADDR to TO_ADDR.  Not used on the PA.  */
void
md_create_long_jump (ptr, from_addr, to_addr, frag, to_symbol)
     char *ptr;
     addressT from_addr, to_addr;
     fragS *frag;
     symbolS *to_symbol;
{
  fprintf (stderr, "pa_create_long_jump\n");
  abort ();
}

/* Return the approximate size of a frag before relaxation has occurred.  */
int
md_estimate_size_before_relax (fragP, segment)
     register fragS *fragP;
     asection *segment;
{
  int size;

  size = 0;

  while ((fragP->fr_fix + size) % fragP->fr_offset)
    size++;

  return size;
}

CONST char *md_shortopts = "";
struct option md_longopts[] = {
  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof(md_longopts);

int
md_parse_option (c, arg)
     int c;
     char *arg;
{
  return 0;
}

void
md_show_usage (stream)
     FILE *stream;
{
}

/* We have no need to default values of symbols.  */

symbolS *
md_undefined_symbol (name)
     char *name;
{
  return 0;
}

/* Apply a fixup to an instruction.  */

int
md_apply_fix (fixP, valp)
     fixS *fixP;
     valueT *valp;
{
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  struct hppa_fix_struct *hppa_fixP;
  long new_val, result;
  unsigned int w1, w2, w, resulti;

  hppa_fixP = (struct hppa_fix_struct *) fixP->tc_fix_data;
  /* SOM uses R_HPPA_ENTRY and R_HPPA_EXIT relocations which can
     never be "applied" (they are just markers).  Likewise for
     R_HPPA_BEGIN_BRTAB and R_HPPA_END_BRTAB.  */
#ifdef OBJ_SOM
  if (fixP->fx_r_type == R_HPPA_ENTRY
      || fixP->fx_r_type == R_HPPA_EXIT
      || fixP->fx_r_type == R_HPPA_BEGIN_BRTAB
      || fixP->fx_r_type == R_HPPA_END_BRTAB
      || fixP->fx_r_type == R_HPPA_BEGIN_TRY)
    return 1;

  /* Disgusting.  We must set fx_offset ourselves -- R_HPPA_END_TRY
     fixups are considered not adjustable, which in turn causes
     adjust_reloc_syms to not set fx_offset.  Ugh.  */
  if (fixP->fx_r_type == R_HPPA_END_TRY)
    {
      fixP->fx_offset = *valp;
      return 1;
    }
#endif

  /* There should have been an HPPA specific fixup associated
     with the GAS fixup.  */
  if (hppa_fixP)
    {
      unsigned long buf_wd = bfd_get_32 (stdoutput, buf);
      unsigned char fmt = bfd_hppa_insn2fmt (buf_wd);

      /* If there is a symbol associated with this fixup, then it's something
	 which will need a SOM relocation (except for some PC-relative relocs).
	 In such cases we should treat the "val" or "addend" as zero since it
	 will be added in as needed from fx_offset in tc_gen_reloc.  */
      if ((fixP->fx_addsy != NULL
	   || fixP->fx_r_type == R_HPPA_NONE)
#ifdef OBJ_SOM
	  && fmt != 32
	  || hppa_fixP->fx_r_field == e_psel
	  || hppa_fixP->fx_r_field == e_rpsel
	  || hppa_fixP->fx_r_field == e_lpsel
	  || hppa_fixP->fx_r_field == e_tsel
	  || hppa_fixP->fx_r_field == e_rtsel
	  || hppa_fixP->fx_r_field == e_ltsel
#endif
	  )
	new_val = ((fmt == 12 || fmt == 17) ? 8 : 0);
#ifdef OBJ_SOM
      /* This is truely disgusting.  The machine independent code blindly
	 adds in the value of the symbol being relocated against.  Damn!  */
      else if (fmt == 32
	       && fixP->fx_addsy != NULL
	       && S_GET_SEGMENT (fixP->fx_addsy) != bfd_com_section_ptr)
	new_val = hppa_field_adjust (*valp - S_GET_VALUE (fixP->fx_addsy),
				     0, hppa_fixP->fx_r_field);
#endif
      else
	new_val = hppa_field_adjust (*valp, 0, hppa_fixP->fx_r_field);

      /* Handle pc-relative exceptions from above.  */
#define arg_reloc_stub_needed(CALLER, CALLEE) \
  ((CALLEE) && (CALLER) && ((CALLEE) != (CALLER)))
      if ((fmt == 12 || fmt == 17)
	  && fixP->fx_addsy
	  && fixP->fx_pcrel
	  && !arg_reloc_stub_needed (((obj_symbol_type *)
				fixP->fx_addsy->bsym)->tc_data.hppa_arg_reloc,
				    hppa_fixP->fx_arg_reloc)
	  && ((int)(*valp) > -262144 && (int)(*valp) < 262143)
	  && S_GET_SEGMENT (fixP->fx_addsy) == hppa_fixP->segment
	  && !(fixP->fx_subsy
	       && S_GET_SEGMENT (fixP->fx_subsy) != hppa_fixP->segment))
	      
	new_val = hppa_field_adjust (*valp, 0, hppa_fixP->fx_r_field);
#undef arg_reloc_stub_needed
	
      switch (fmt)
	{
	/* Handle all opcodes with the 'j' operand type.  */
	case 14:
	  CHECK_FIELD (new_val, 8191, -8192, 0);

	  /* Mask off 14 bits to be changed.  */
	  bfd_put_32 (stdoutput,
		      bfd_get_32 (stdoutput, buf) & 0xffffc000,
		      buf);
	  low_sign_unext (new_val, 14, &resulti);
	  result = resulti;
	  break;

	/* Handle all opcodes with the 'k' operand type.  */
	case 21:
	  CHECK_FIELD (new_val, 2097152, 0, 0);

	  /* Mask off 21 bits to be changed.  */
	  bfd_put_32 (stdoutput,
		      bfd_get_32 (stdoutput, buf) & 0xffe00000,
		      buf);
	  dis_assemble_21 (new_val, &resulti);
	  result = resulti;
	  break;

	/* Handle all the opcodes with the 'i' operand type.  */
	case 11:
	  CHECK_FIELD (new_val, 1023, -1023, 0);

	  /* Mask off 11 bits to be changed.  */
	  bfd_put_32 (stdoutput,
		      bfd_get_32 (stdoutput, buf) & 0xffff800,
		      buf);
	  low_sign_unext (new_val, 11, &resulti);
	  result = resulti;
	  break;

	/* Handle all the opcodes with the 'w' operand type.  */
	case 12:
	  CHECK_FIELD (new_val, 8199, -8184, 0);

	  /* Mask off 11 bits to be changed.  */
	  sign_unext ((new_val - 8) >> 2, 12, &resulti);
	  bfd_put_32 (stdoutput,
		      bfd_get_32 (stdoutput, buf) & 0xffffe002,
		      buf);

	  dis_assemble_12 (resulti, &w1, &w);
	  result = ((w1 << 2) | w);
	  break;

	/* Handle some of the opcodes with the 'W' operand type.  */
	case 17:
	  CHECK_FIELD (new_val, 262143, -262144, 0);

	  /* Mask off 17 bits to be changed.  */
	  bfd_put_32 (stdoutput,
		      bfd_get_32 (stdoutput, buf) & 0xffe0e002,
		      buf);
	  sign_unext ((new_val - 8) >> 2, 17, &resulti);
	  dis_assemble_17 (resulti, &w1, &w2, &w);
	  result = ((w2 << 2) | (w1 << 16) | w);
	  break;

	case 32:
	  result = 0;
	  bfd_put_32 (stdoutput, new_val, buf);
	  break;

	default:
	  as_bad ("Unknown relocation encountered in md_apply_fix.");
	  return 0;
	}

      /* Insert the relocation.  */
      bfd_put_32 (stdoutput, bfd_get_32 (stdoutput, buf) | result, buf);
      return 1;
    }
  else
    {
      printf ("no hppa_fixup entry for this fixup (fixP = 0x%x, type = 0x%x)\n",
	      (unsigned int) fixP, fixP->fx_r_type);
      return 0;
    }
}

/* Exactly what point is a PC-relative offset relative TO?
   On the PA, they're relative to the address of the offset.  */

long
md_pcrel_from (fixP)
     fixS *fixP;
{
  return fixP->fx_where + fixP->fx_frag->fr_address;
}

/* Return nonzero if the input line pointer is at the end of
   a statement.  */

static int
is_end_of_statement ()
{
  return ((*input_line_pointer == '\n')
	  || (*input_line_pointer == ';')
	  || (*input_line_pointer == '!'));
}

/* Read a number from S.  The number might come in one of many forms,
   the most common will be a hex or decimal constant, but it could be
   a pre-defined register (Yuk!), or an absolute symbol.

   Return a number or -1 for failure.

   When parsing PA-89 FP register numbers RESULT will be
   the address of a structure to return information about
   L/R half of FP registers, store results there as appropriate.

   pa_parse_number can not handle negative constants and will fail
   horribly if it is passed such a constant.  */

static int
pa_parse_number (s, result)
     char **s;
     struct pa_11_fp_reg_struct *result;
{
  int num;
  char *name;
  char c;
  symbolS *sym;
  int status;
  char *p = *s;

  /* Skip whitespace before the number.  */
  while (*p == ' ' || *p == '\t')
    p = p + 1;

  /* Store info in RESULT if requested by caller.  */
  if (result)
    {
      result->number_part = -1;
      result->l_r_select = -1;
    }
  num = -1;

  if (isdigit (*p))
    {
      /* Looks like a number.  */
      num = 0;

      if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X'))
	{
	  /* The number is specified in hex.  */
	  p += 2;
	  while (isdigit (*p) || ((*p >= 'a') && (*p <= 'f'))
		 || ((*p >= 'A') && (*p <= 'F')))
	    {
	      if (isdigit (*p))
		num = num * 16 + *p - '0';
	      else if (*p >= 'a' && *p <= 'f')
		num = num * 16 + *p - 'a' + 10;
	      else
		num = num * 16 + *p - 'A' + 10;
	      ++p;
	    }
	}
      else
	{
	  /* The number is specified in decimal.  */
	  while (isdigit (*p))
	    {
	      num = num * 10 + *p - '0';
	      ++p;
	    }
	}

      /* Store info in RESULT if requested by the caller.  */
      if (result)
	{
	  result->number_part = num;

	  if (IS_R_SELECT (p))
	    {
	      result->l_r_select = 1;
	      ++p;
	    }
	  else if (IS_L_SELECT (p))
	    {
	      result->l_r_select = 0;
	      ++p;
	    }
	  else
	    result->l_r_select = 0;
	}
    }
  else if (*p == '%')
    {
      /* The number might be a predefined register.  */
      num = 0;
      name = p;
      p++;
      c = *p;
      /* Tege hack: Special case for general registers as the general
         code makes a binary search with case translation, and is VERY
         slow. */
      if (c == 'r')
	{
	  p++;
	  if (*p == 'e' && *(p + 1) == 't'
	      && (*(p + 2) == '0' || *(p + 2) == '1'))
	    {
	      p += 2;
	      num = *p - '0' + 28;
	      p++;
	    }
	  else if (*p == 'p')
	    {
	      num = 2;
	      p++;
	    }
	  else if (!isdigit (*p))
	    {
	      if (print_errors)
		as_bad ("Undefined register: '%s'.", name);
	      num = -1;
	    }
	  else
	    {
	      do
		num = num * 10 + *p++ - '0';
	      while (isdigit (*p));
	    }
	}
      else
	{
	  /* Do a normal register search.  */
	  while (is_part_of_name (c))
	    {
	      p = p + 1;
	      c = *p;
	    }
	  *p = 0;
	  status = reg_name_search (name);
	  if (status >= 0)
	    num = status;
	  else
	    {
	      if (print_errors)
		as_bad ("Undefined register: '%s'.", name);
	      num = -1;
	    }
	  *p = c;
	}

      /* Store info in RESULT if requested by caller.  */
      if (result)
	{
	  result->number_part = num;
	  if (IS_R_SELECT (p - 1))
	    result->l_r_select = 1;
	  else if (IS_L_SELECT (p - 1))
	    result->l_r_select = 0;
	  else
	    result->l_r_select = 0;
	}
    }
  else
    {
      /* And finally, it could be a symbol in the absolute section which
         is effectively a constant.  */
      num = 0;
      name = p;
      c = *p;
      while (is_part_of_name (c))
	{
	  p = p + 1;
	  c = *p;
	}
      *p = 0;
      if ((sym = symbol_find (name)) != NULL)
	{
	  if (S_GET_SEGMENT (sym) == &bfd_abs_section)
	    num = S_GET_VALUE (sym);
	  else
	    {
	      if (print_errors)
		as_bad ("Non-absolute symbol: '%s'.", name);
	      num = -1;
	    }
	}
      else
	{
	  /* There is where we'd come for an undefined symbol
	     or for an empty string.  For an empty string we
	     will return zero.  That's a concession made for
	     compatability with the braindamaged HP assemblers.  */
	  if (*name == 0)
	    num = 0;
	  else
	    {
	      if (print_errors)
		as_bad ("Undefined absolute constant: '%s'.", name);
	      num = -1;
	    }
	}
      *p = c;

      /* Store info in RESULT if requested by caller.  */
      if (result)
	{
	  result->number_part = num;
	  if (IS_R_SELECT (p - 1))
	    result->l_r_select = 1;
	  else if (IS_L_SELECT (p - 1))
	    result->l_r_select = 0;
	  else
	    result->l_r_select = 0;
	}
    }

  *s = p;
  return num;
}

#define REG_NAME_CNT	(sizeof(pre_defined_registers) / sizeof(struct pd_reg))

/* Given NAME, find the register number associated with that name, return
   the integer value associated with the given name or -1 on failure.  */

static int
reg_name_search (name)
     char *name;
{
  int middle, low, high;
  int cmp;

  low = 0;
  high = REG_NAME_CNT - 1;

  do
    {
      middle = (low + high) / 2;
      cmp = strcasecmp (name, pre_defined_registers[middle].name);
      if (cmp < 0)
	high = middle - 1;
      else if (cmp > 0)
	low = middle + 1;
      else
	return pre_defined_registers[middle].value;
    }
  while (low <= high);

  return -1;
}


/* Return nonzero if the given INSN and L/R information will require
   a new PA-1.1 opcode.  */

static int
need_pa11_opcode (insn, result)
     struct pa_it *insn;
     struct pa_11_fp_reg_struct *result;
{
  if (result->l_r_select == 1 && !(insn->fpof1 == DBL && insn->fpof2 == DBL))
    {
      /* If this instruction is specific to a particular architecture,
	 then set a new architecture.  */
      if (bfd_get_mach (stdoutput) < pa11)
	{
	  if (!bfd_set_arch_mach (stdoutput, bfd_arch_hppa, pa11))
	    as_warn ("could not update architecture and machine");
	}
      return TRUE;
    }
  else
    return FALSE;
}

/* Parse a condition for a fcmp instruction.  Return the numerical
   code associated with the condition.  */

static int
pa_parse_fp_cmp_cond (s)
     char **s;
{
  int cond, i;

  cond = 0;

  for (i = 0; i < 32; i++)
    {
      if (strncasecmp (*s, fp_cond_map[i].string,
		       strlen (fp_cond_map[i].string)) == 0)
	{
	  cond = fp_cond_map[i].cond;
	  *s += strlen (fp_cond_map[i].string);
	  /* If not a complete match, back up the input string and
	     report an error.  */
	  if (**s != ' ' && **s != '\t')
	    {
	      *s -= strlen (fp_cond_map[i].string);
	      break;
	    }
	  while (**s == ' ' || **s == '\t')
	    *s = *s + 1;
	  return cond;
	}
    }

  as_bad ("Invalid FP Compare Condition: %s", *s);

  /* Advance over the bogus completer.  */
  while (**s != ',' && **s != ' ' && **s != '\t')
    *s += 1;

  return 0;
}

/* Parse an FP operand format completer returning the completer
   type.  */

static fp_operand_format
pa_parse_fp_format (s)
     char **s;
{
  int format;

  format = SGL;
  if (**s == ',')
    {
      *s += 1;
      if (strncasecmp (*s, "sgl", 3) == 0)
	{
	  format = SGL;
	  *s += 4;
	}
      else if (strncasecmp (*s, "dbl", 3) == 0)
	{
	  format = DBL;
	  *s += 4;
	}
      else if (strncasecmp (*s, "quad", 4) == 0)
	{
	  format = QUAD;
	  *s += 5;
	}
      else
	{
	  format = ILLEGAL_FMT;
	  as_bad ("Invalid FP Operand Format: %3s", *s);
	}
    }

  return format;
}

/* Convert from a selector string into a selector type.  */

static int
pa_chk_field_selector (str)
     char **str;
{
  int middle, low, high;
  int cmp;
  char name[4];

  /* Read past any whitespace.  */
  /* FIXME: should we read past newlines and formfeeds??? */
  while (**str == ' ' || **str == '\t' || **str == '\n' || **str == '\f')
    *str = *str + 1;

  if ((*str)[1] == '\'' || (*str)[1] == '%')
    name[0] = tolower ((*str)[0]),
    name[1] = 0;
  else if ((*str)[2] == '\'' || (*str)[2] == '%')
    name[0] = tolower ((*str)[0]),
    name[1] = tolower ((*str)[1]),
    name[2] = 0;
#ifdef OBJ_SOM
  else if ((*str)[3] == '\'' || (*str)[3] == '%')
    name[0] = tolower ((*str)[0]),
    name[1] = tolower ((*str)[1]),
    name[2] = tolower ((*str)[2]),
    name[3] = 0;
#endif
  else
    return e_fsel;

  low = 0;
  high = sizeof (selector_table) / sizeof (struct selector_entry) - 1;

  do
    {
      middle = (low + high) / 2;
      cmp = strcmp (name, selector_table[middle].prefix);
      if (cmp < 0)
	high = middle - 1;
      else if (cmp > 0)
	low = middle + 1;
      else
	{
	  *str += strlen (name) + 1;
#ifndef OBJ_SOM
	  if (selector_table[middle].field_selector == e_nsel)
	    return e_fsel;
#endif
	  return selector_table[middle].field_selector;
	}
    }
  while (low <= high);

  return e_fsel;
}

/* Mark (via expr_end) the end of an expression (I think).  FIXME.  */

static int
get_expression (str)
     char *str;
{
  char *save_in;
  asection *seg;

  save_in = input_line_pointer;
  input_line_pointer = str;
  seg = expression (&the_insn.exp);
  if (!(seg == absolute_section
	|| seg == undefined_section
	|| SEG_NORMAL (seg)))
    {
      as_warn ("Bad segment in expression.");
      expr_end = input_line_pointer;
      input_line_pointer = save_in;
      return 1;
    }
  expr_end = input_line_pointer;
  input_line_pointer = save_in;
  return 0;
}

/* Mark (via expr_end) the end of an absolute expression.  FIXME. */
static int
pa_get_absolute_expression (insn, strp)
     struct pa_it *insn;
     char **strp;
{
  char *save_in;

  insn->field_selector = pa_chk_field_selector (strp);
  save_in = input_line_pointer;
  input_line_pointer = *strp;
  expression (&insn->exp);
  if (insn->exp.X_op != O_constant)
    {
      as_bad ("Bad segment (should be absolute).");
      expr_end = input_line_pointer;
      input_line_pointer = save_in;
      return 0;
    }
  expr_end = input_line_pointer;
  input_line_pointer = save_in;
  return evaluate_absolute (insn);
}

/* Evaluate an absolute expression EXP which may be modified by
   the selector FIELD_SELECTOR.  Return the value of the expression.  */
static int
evaluate_absolute (insn)
     struct pa_it *insn;
{
  int value;
  expressionS exp;
  int field_selector = insn->field_selector;

  exp = insn->exp;
  value = exp.X_add_number;

  switch (field_selector)
    {
    /* No change.  */
    case e_fsel:
      break;

    /* If bit 21 is on then add 0x800 and arithmetic shift right 11 bits.  */
    case e_lssel:
      if (value & 0x00000400)
	value += 0x800;
      value = (value & 0xfffff800) >> 11;
      break;

    /* Sign extend from bit 21.  */
    case e_rssel:
      if (value & 0x00000400)
	value |= 0xfffff800;
      else
	value &= 0x7ff;
      break;

    /* Arithmetic shift right 11 bits.  */
    case e_lsel:
      value = (value & 0xfffff800) >> 11;
      break;

    /* Set bits 0-20 to zero.  */
    case e_rsel:
      value = value & 0x7ff;
      break;

    /* Add 0x800 and arithmetic shift right 11 bits.  */
    case e_ldsel:
      value += 0x800;
      value = (value & 0xfffff800) >> 11;
      break;

    /* Set bitgs 0-21 to one.  */
    case e_rdsel:
      value |= 0xfffff800;
      break;

#define RSEL_ROUND(c)  (((c) + 0x1000) & ~0x1fff)
    case e_rrsel:
      value = (RSEL_ROUND (value) & 0x7ff) + (value - RSEL_ROUND (value));
      break;

    case e_lrsel:
      value = (RSEL_ROUND (value) >> 11) & 0x1fffff;
      break;
#undef RSEL_ROUND

    default:
      BAD_CASE (field_selector);
      break;
    }
  return value;
}

/* Given an argument location specification return the associated
   argument location number.  */

static unsigned int
pa_build_arg_reloc (type_name)
     char *type_name;
{

  if (strncasecmp (type_name, "no", 2) == 0)
    return 0;
  if (strncasecmp (type_name, "gr", 2) == 0)
    return 1;
  else if (strncasecmp (type_name, "fr", 2) == 0)
    return 2;
  else if (strncasecmp (type_name, "fu", 2) == 0)
    return 3;
  else
    as_bad ("Invalid argument location: %s\n", type_name);

  return 0;
}

/* Encode and return an argument relocation specification for
   the given register in the location specified by arg_reloc.  */

static unsigned int
pa_align_arg_reloc (reg, arg_reloc)
     unsigned int reg;
     unsigned int arg_reloc;
{
  unsigned int new_reloc;

  new_reloc = arg_reloc;
  switch (reg)
    {
    case 0:
      new_reloc <<= 8;
      break;
    case 1:
      new_reloc <<= 6;
      break;
    case 2:
      new_reloc <<= 4;
      break;
    case 3:
      new_reloc <<= 2;
      break;
    default:
      as_bad ("Invalid argument description: %d", reg);
    }

  return new_reloc;
}

/* Parse a PA nullification completer (,n).  Return nonzero if the
   completer was found; return zero if no completer was found.  */

static int
pa_parse_nullif (s)
     char **s;
{
  int nullif;

  nullif = 0;
  if (**s == ',')
    {
      *s = *s + 1;
      if (strncasecmp (*s, "n", 1) == 0)
	nullif = 1;
      else
	{
	  as_bad ("Invalid Nullification: (%c)", **s);
	  nullif = 0;
	}
      *s = *s + 1;
    }

  return nullif;
}

/* Parse a non-negated compare/subtract completer returning the
   number (for encoding in instrutions) of the given completer.

   ISBRANCH specifies whether or not this is parsing a condition
   completer for a branch (vs a nullification completer for a
   computational instruction.  */

static int
pa_parse_nonneg_cmpsub_cmpltr (s, isbranch)
     char **s;
     int isbranch;
{
  int cmpltr;
  char *name = *s + 1;
  char c;
  char *save_s = *s;

  cmpltr = 0;
  if (**s == ',')
    {
      *s += 1;
      while (**s != ',' && **s != ' ' && **s != '\t')
	*s += 1;
      c = **s;
      **s = 0x00;
      if (strcmp (name, "=") == 0)
	{
	  cmpltr = 1;
	}
      else if (strcmp (name, "<") == 0)
	{
	  cmpltr = 2;
	}
      else if (strcmp (name, "<=") == 0)
	{
	  cmpltr = 3;
	}
      else if (strcmp (name, "<<") == 0)
	{
	  cmpltr = 4;
	}
      else if (strcmp (name, "<<=") == 0)
	{
	  cmpltr = 5;
	}
      else if (strcasecmp (name, "sv") == 0)
	{
	  cmpltr = 6;
	}
      else if (strcasecmp (name, "od") == 0)
	{
	  cmpltr = 7;
	}
      /* If we have something like addb,n then there is no condition
         completer.  */
      else if (strcasecmp (name, "n") == 0 && isbranch)
	{
	  cmpltr = 0;
	}
      else
	{
	  cmpltr = -1;
	}
      **s = c;
    }

  /* Reset pointers if this was really a ,n for a branch instruction.  */
  if (cmpltr == 0 && *name == 'n' && isbranch)
    *s = save_s;

  return cmpltr;
}

/* Parse a negated compare/subtract completer returning the
   number (for encoding in instrutions) of the given completer.

   ISBRANCH specifies whether or not this is parsing a condition
   completer for a branch (vs a nullification completer for a
   computational instruction.  */

static int
pa_parse_neg_cmpsub_cmpltr (s, isbranch)
     char **s;
     int isbranch;
{
  int cmpltr;
  char *name = *s + 1;
  char c;
  char *save_s = *s;

  cmpltr = 0;
  if (**s == ',')
    {
      *s += 1;
      while (**s != ',' && **s != ' ' && **s != '\t')
	*s += 1;
      c = **s;
      **s = 0x00;
      if (strcasecmp (name, "tr") == 0)
	{
	  cmpltr = 0;
	}
      else if (strcmp (name, "<>") == 0)
	{
	  cmpltr = 1;
	}
      else if (strcmp (name, ">=") == 0)
	{
	  cmpltr = 2;
	}
      else if (strcmp (name, ">") == 0)
	{
	  cmpltr = 3;
	}
      else if (strcmp (name, ">>=") == 0)
	{
	  cmpltr = 4;
	}
      else if (strcmp (name, ">>") == 0)
	{
	  cmpltr = 5;
	}
      else if (strcasecmp (name, "nsv") == 0)
	{
	  cmpltr = 6;
	}
      else if (strcasecmp (name, "ev") == 0)
	{
	  cmpltr = 7;
	}
      /* If we have something like addb,n then there is no condition
         completer.  */
      else if (strcasecmp (name, "n") == 0 && isbranch)
	{
	  cmpltr = 0;
	}
      else
	{
	  cmpltr = -1;
	}
      **s = c;
    }

  /* Reset pointers if this was really a ,n for a branch instruction.  */
  if (cmpltr == 0 && *name == 'n' && isbranch)
    *s = save_s;

  return cmpltr;
}

/* Parse a non-negated addition completer returning the number
   (for encoding in instrutions) of the given completer.

   ISBRANCH specifies whether or not this is parsing a condition
   completer for a branch (vs a nullification completer for a
   computational instruction.  */

static int
pa_parse_nonneg_add_cmpltr (s, isbranch)
     char **s;
     int isbranch;
{
  int cmpltr;
  char *name = *s + 1;
  char c;
  char *save_s = *s;

  cmpltr = 0;
  if (**s == ',')
    {
      *s += 1;
      while (**s != ',' && **s != ' ' && **s != '\t')
	*s += 1;
      c = **s;
      **s = 0x00;
      if (strcmp (name, "=") == 0)
	{
	  cmpltr = 1;
	}
      else if (strcmp (name, "<") == 0)
	{
	  cmpltr = 2;
	}
      else if (strcmp (name, "<=") == 0)
	{
	  cmpltr = 3;
	}
      else if (strcasecmp (name, "nuv") == 0)
	{
	  cmpltr = 4;
	}
      else if (strcasecmp (name, "znv") == 0)
	{
	  cmpltr = 5;
	}
      else if (strcasecmp (name, "sv") == 0)
	{
	  cmpltr = 6;
	}
      else if (strcasecmp (name, "od") == 0)
	{
	  cmpltr = 7;
	}
      /* If we have something like addb,n then there is no condition
         completer.  */
      else if (strcasecmp (name, "n") == 0 && isbranch)
	{
	  cmpltr = 0;
	}
      else
	{
	  cmpltr = -1;
	}
      **s = c;
    }

  /* Reset pointers if this was really a ,n for a branch instruction.  */
  if (cmpltr == 0 && *name == 'n' && isbranch)
    *s = save_s;

  return cmpltr;
}

/* Parse a negated addition completer returning the number
   (for encoding in instrutions) of the given completer.

   ISBRANCH specifies whether or not this is parsing a condition
   completer for a branch (vs a nullification completer for a
   computational instruction).  */

static int
pa_parse_neg_add_cmpltr (s, isbranch)
     char **s;
     int isbranch;
{
  int cmpltr;
  char *name = *s + 1;
  char c;
  char *save_s = *s;

  cmpltr = 0;
  if (**s == ',')
    {
      *s += 1;
      while (**s != ',' && **s != ' ' && **s != '\t')
	*s += 1;
      c = **s;
      **s = 0x00;
      if (strcasecmp (name, "tr") == 0)
	{
	  cmpltr = 0;
	}
      else if (strcmp (name, "<>") == 0)
	{
	  cmpltr = 1;
	}
      else if (strcmp (name, ">=") == 0)
	{
	  cmpltr = 2;
	}
      else if (strcmp (name, ">") == 0)
	{
	  cmpltr = 3;
	}
      else if (strcasecmp (name, "uv") == 0)
	{
	  cmpltr = 4;
	}
      else if (strcasecmp (name, "vnz") == 0)
	{
	  cmpltr = 5;
	}
      else if (strcasecmp (name, "nsv") == 0)
	{
	  cmpltr = 6;
	}
      else if (strcasecmp (name, "ev") == 0)
	{
	  cmpltr = 7;
	}
      /* If we have something like addb,n then there is no condition
         completer.  */
      else if (strcasecmp (name, "n") == 0 && isbranch)
	{
	  cmpltr = 0;
	}
      else
	{
	  cmpltr = -1;
	}
      **s = c;
    }

  /* Reset pointers if this was really a ,n for a branch instruction.  */
  if (cmpltr == 0 && *name == 'n' && isbranch)
    *s = save_s;

  return cmpltr;
}

/* Handle an alignment directive.  Special so that we can update the
   alignment of the subspace if necessary.  */
static void
pa_align (bytes)
{
  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  /* Let the generic gas code do most of the work.  */
  s_align_bytes (bytes);

  /* If bytes is a power of 2, then update the current subspace's
     alignment if necessary.  */
  if (log2 (bytes) != -1)
    record_alignment (current_subspace->ssd_seg, log2 (bytes));
}

/* Handle a .BLOCK type pseudo-op.  */

static void
pa_block (z)
     int z;
{
  char *p;
  long int temp_fill;
  unsigned int temp_size;
  int i;

  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  temp_size = get_absolute_expression ();

  /* Always fill with zeros, that's what the HP assembler does.  */
  temp_fill = 0;

  p = frag_var (rs_fill, (int) temp_size, (int) temp_size,
		(relax_substateT) 0, (symbolS *) 0, 1, NULL);
  bzero (p, temp_size);

  /* Convert 2 bytes at a time.  */

  for (i = 0; i < temp_size; i += 2)
    {
      md_number_to_chars (p + i,
			  (valueT) temp_fill,
			  (int) ((temp_size - i) > 2 ? 2 : (temp_size - i)));
    }

  pa_undefine_label ();
  demand_empty_rest_of_line ();
}

/* Handle a .begin_brtab and .end_brtab pseudo-op.  */

static void
pa_brtab (begin)
     int begin;
{

#ifdef OBJ_SOM
  /* The BRTAB relocations are only availble in SOM (to denote
     the beginning and end of branch tables).  */
  char *where = frag_more (0);

  fix_new_hppa (frag_now, where - frag_now->fr_literal, 0,
		NULL, (offsetT) 0, NULL,
		0, begin ? R_HPPA_BEGIN_BRTAB : R_HPPA_END_BRTAB,
		e_fsel, 0, 0, NULL);
#endif

  demand_empty_rest_of_line ();
}

/* Handle a .begin_try and .end_try pseudo-op.  */

static void
pa_try (begin)
     int begin;
{
#ifdef OBJ_SOM
  expressionS exp;
  char *where = frag_more (0);

  if (! begin)
    expression (&exp);

  /* The TRY relocations are only availble in SOM (to denote
     the beginning and end of exception handling regions).  */

  fix_new_hppa (frag_now, where - frag_now->fr_literal, 0,
		NULL, (offsetT) 0, begin ? NULL : &exp,
		0, begin ? R_HPPA_BEGIN_TRY : R_HPPA_END_TRY,
		e_fsel, 0, 0, NULL);
#endif

  demand_empty_rest_of_line ();
}

/* Handle a .CALL pseudo-op.  This involves storing away information
   about where arguments are to be found so the linker can detect
   (and correct) argument location mismatches between caller and callee.  */

static void
pa_call (unused)
     int unused;
{
  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  pa_call_args (&last_call_desc);
  demand_empty_rest_of_line ();
}

/* Do the dirty work of building a call descriptor which describes
   where the caller placed arguments to a function call.  */

static void
pa_call_args (call_desc)
     struct call_desc *call_desc;
{
  char *name, c, *p;
  unsigned int temp, arg_reloc;

  while (!is_end_of_statement ())
    {
      name = input_line_pointer;
      c = get_symbol_end ();
      /* Process a source argument.  */
      if ((strncasecmp (name, "argw", 4) == 0))
	{
	  temp = atoi (name + 4);
	  p = input_line_pointer;
	  *p = c;
	  input_line_pointer++;
	  name = input_line_pointer;
	  c = get_symbol_end ();
	  arg_reloc = pa_build_arg_reloc (name);
	  call_desc->arg_reloc |= pa_align_arg_reloc (temp, arg_reloc);
	}
      /* Process a return value.  */
      else if ((strncasecmp (name, "rtnval", 6) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	  input_line_pointer++;
	  name = input_line_pointer;
	  c = get_symbol_end ();
	  arg_reloc = pa_build_arg_reloc (name);
	  call_desc->arg_reloc |= (arg_reloc & 0x3);
	}
      else
	{
	  as_bad ("Invalid .CALL argument: %s", name);
	}
      p = input_line_pointer;
      *p = c;
      if (!is_end_of_statement ())
	input_line_pointer++;
    }
}

/* Return TRUE if FRAG1 and FRAG2 are the same.  */

static int
is_same_frag (frag1, frag2)
     fragS *frag1;
     fragS *frag2;
{

  if (frag1 == NULL)
    return (FALSE);
  else if (frag2 == NULL)
    return (FALSE);
  else if (frag1 == frag2)
    return (TRUE);
  else if (frag2->fr_type == rs_fill && frag2->fr_fix == 0)
    return (is_same_frag (frag1, frag2->fr_next));
  else
    return (FALSE);
}

#ifdef OBJ_ELF
/* Build an entry in the UNWIND subspace from the given function
   attributes in CALL_INFO.  This is not needed for SOM as using
   R_ENTRY and R_EXIT relocations allow the linker to handle building
   of the unwind spaces.  */

static void
pa_build_unwind_subspace (call_info)
     struct call_info *call_info;
{
  char *unwind;
  asection *seg, *save_seg;
  subsegT subseg, save_subseg;
  int i;
  char c, *p;

  /* Get into the right seg/subseg.  This may involve creating
     the seg the first time through.  Make sure to have the
     old seg/subseg so that we can reset things when we are done.  */
  subseg = SUBSEG_UNWIND;
  seg = bfd_get_section_by_name (stdoutput, UNWIND_SECTION_NAME);
  if (seg == ASEC_NULL)
    {
      seg = bfd_make_section_old_way (stdoutput, UNWIND_SECTION_NAME);
      bfd_set_section_flags (stdoutput, seg,
			     SEC_READONLY | SEC_HAS_CONTENTS
			     | SEC_LOAD | SEC_RELOC);
    }

  save_seg = now_seg;
  save_subseg = now_subseg;
  subseg_set (seg, subseg);


  /* Get some space to hold relocation information for the unwind
     descriptor.  */
  p = frag_more (4);
  md_number_to_chars (p, 0, 4);

  /* Relocation info. for start offset of the function.  */
  fix_new_hppa (frag_now, p - frag_now->fr_literal, 4,
		call_info->start_symbol, (offsetT) 0,
		(expressionS *) NULL, 0, R_PARISC_DIR32, e_fsel, 32, 0, NULL);

  p = frag_more (4);
  md_number_to_chars (p, 0, 4);

  /* Relocation info. for end offset of the function.

     Because we allow reductions of 32bit relocations for ELF, this will be
     reduced to section_sym + offset which avoids putting the temporary
     symbol into the symbol table.  It (should) end up giving the same
     value as call_info->start_symbol + function size once the linker is
     finished with its work.  */

  fix_new_hppa (frag_now, p - frag_now->fr_literal, 4,
		call_info->end_symbol, (offsetT) 0,
		(expressionS *) NULL, 0, R_PARISC_DIR32, e_fsel, 32, 0, NULL);

  /* Dump it. */
  unwind = (char *) &call_info->ci_unwind;
  for (i = 8; i < sizeof (struct unwind_table); i++)
    {
      c = *(unwind + i);
      {
	FRAG_APPEND_1_CHAR (c);
      }
    }

  /* Return back to the original segment/subsegment.  */
  subseg_set (save_seg, save_subseg);
}
#endif

/* Process a .CALLINFO pseudo-op.  This information is used later
   to build unwind descriptors and maybe one day to support
   .ENTER and .LEAVE.  */

static void
pa_callinfo (unused)
     int unused;
{
  char *name, c, *p;
  int temp;

  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  /* .CALLINFO must appear within a procedure definition.  */
  if (!within_procedure)
    as_bad (".callinfo is not within a procedure definition");

  /* Mark the fact that we found the .CALLINFO for the
     current procedure.  */
  callinfo_found = TRUE;

  /* Iterate over the .CALLINFO arguments.  */
  while (!is_end_of_statement ())
    {
      name = input_line_pointer;
      c = get_symbol_end ();
      /* Frame size specification.  */
      if ((strncasecmp (name, "frame", 5) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	  input_line_pointer++;
	  temp = get_absolute_expression ();
	  if ((temp & 0x3) != 0)
	    {
	      as_bad ("FRAME parameter must be a multiple of 8: %d\n", temp);
	      temp = 0;
	    }

	  /* callinfo is in bytes and unwind_desc is in 8 byte units.  */
	  last_call_info->ci_unwind.descriptor.frame_size = temp / 8;

	}
      /* Entry register (GR, GR and SR) specifications.  */
      else if ((strncasecmp (name, "entry_gr", 8) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	  input_line_pointer++;
	  temp = get_absolute_expression ();
	  /* The HP assembler accepts 19 as the high bound for ENTRY_GR
	     even though %r19 is caller saved.  I think this is a bug in
	     the HP assembler, and we are not going to emulate it.  */
	  if (temp < 3 || temp > 18)
	    as_bad ("Value for ENTRY_GR must be in the range 3..18\n");
	  last_call_info->ci_unwind.descriptor.entry_gr = temp - 2;
	}
      else if ((strncasecmp (name, "entry_fr", 8) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	  input_line_pointer++;
	  temp = get_absolute_expression ();
	  /* Similarly the HP assembler takes 31 as the high bound even
	     though %fr21 is the last callee saved floating point register.  */
	  if (temp < 12 || temp > 21)
	    as_bad ("Value for ENTRY_FR must be in the range 12..21\n");
	  last_call_info->ci_unwind.descriptor.entry_fr = temp - 11;
	}
      else if ((strncasecmp (name, "entry_sr", 8) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	  input_line_pointer++;
	  temp = get_absolute_expression ();
	  if (temp != 3)
	    as_bad ("Value for ENTRY_SR must be 3\n");
	}
      /* Note whether or not this function performs any calls.  */
      else if ((strncasecmp (name, "calls", 5) == 0) ||
	       (strncasecmp (name, "caller", 6) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	}
      else if ((strncasecmp (name, "no_calls", 8) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	}
      /* Should RP be saved into the stack.  */
      else if ((strncasecmp (name, "save_rp", 7) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	  last_call_info->ci_unwind.descriptor.save_rp = 1;
	}
      /* Likewise for SP.  */
      else if ((strncasecmp (name, "save_sp", 7) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	  last_call_info->ci_unwind.descriptor.save_sp = 1;
	}
      /* Is this an unwindable procedure.  If so mark it so
         in the unwind descriptor.  */
      else if ((strncasecmp (name, "no_unwind", 9) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	  last_call_info->ci_unwind.descriptor.cannot_unwind = 1;
	}
      /* Is this an interrupt routine.  If so mark it in the
         unwind descriptor.  */
      else if ((strncasecmp (name, "hpux_int", 7) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	  last_call_info->ci_unwind.descriptor.hpux_interrupt_marker = 1;
	}
      /* Is this a millicode routine.  "millicode" isn't in my
	 assembler manual, but my copy is old.  The HP assembler
	 accepts it, and there's a place in the unwind descriptor
	 to drop the information, so we'll accept it too.  */
      else if ((strncasecmp (name, "millicode", 9) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	  last_call_info->ci_unwind.descriptor.millicode = 1;
	}
      else
	{
	  as_bad ("Invalid .CALLINFO argument: %s", name);
	  *input_line_pointer = c;
	}
      if (!is_end_of_statement ())
	input_line_pointer++;
    }

  demand_empty_rest_of_line ();
}

/* Switch into the code subspace.  */

static void
pa_code (unused)
     int unused;
{
  current_space = is_defined_space ("$TEXT$");
  current_subspace
    = pa_subsegment_to_subspace (current_space->sd_seg, 0);
  s_text (0);
  pa_undefine_label ();
}

/* This is different than the standard GAS s_comm(). On HP9000/800 machines,
   the .comm pseudo-op has the following symtax:

   <label> .comm <length>

   where <label> is optional and is a symbol whose address will be the start of
   a block of memory <length> bytes long. <length> must be an absolute
   expression.  <length> bytes will be allocated in the current space
   and subspace.

   Also note the label may not even be on the same line as the .comm.

   This difference in syntax means the colon function will be called
   on the symbol before we arrive in pa_comm.  colon will set a number
   of attributes of the symbol that need to be fixed here.  In particular
   the value, section pointer, fragment pointer, flags, etc.  What
   a pain.

   This also makes error detection all but impossible.  */

static void
pa_comm (unused)
     int unused;
{
  unsigned int size;
  symbolS *symbol;
  label_symbol_struct *label_symbol = pa_get_label ();

  if (label_symbol)
    symbol = label_symbol->lss_label;
  else
    symbol = NULL;

  SKIP_WHITESPACE ();
  size = get_absolute_expression ();

  if (symbol)
    {
      S_SET_VALUE (symbol, size);
      S_SET_SEGMENT (symbol, bfd_und_section_ptr);
      S_SET_EXTERNAL (symbol);

      /* colon() has already set the frag to the current location in the
         current subspace; we need to reset the fragment to the zero address
         fragment.  We also need to reset the segment pointer.  */
      symbol->sy_frag = &zero_address_frag;
    }
  demand_empty_rest_of_line ();
}

/* Process a .END pseudo-op.  */

static void
pa_end (unused)
     int unused;
{
  demand_empty_rest_of_line ();
}

/* Process a .ENTER pseudo-op.  This is not supported.  */
static void
pa_enter (unused)
     int unused;
{
  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  abort ();
}

/* Process a .ENTRY pseudo-op.  .ENTRY marks the beginning of the
   procesure.  */
static void
pa_entry (unused)
     int unused;
{
  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  if (!within_procedure)
    as_bad ("Misplaced .entry. Ignored.");
  else
    {
      if (!callinfo_found)
	as_bad ("Missing .callinfo.");
    }
  demand_empty_rest_of_line ();
  within_entry_exit = TRUE;

#ifdef OBJ_SOM
  /* SOM defers building of unwind descriptors until the link phase.
     The assembler is responsible for creating an R_ENTRY relocation
     to mark the beginning of a region and hold the unwind bits, and
     for creating an R_EXIT relocation to mark the end of the region.

     FIXME.  ELF should be using the same conventions!  The problem
     is an unwind requires too much relocation space.  Hmmm.  Maybe
     if we split the unwind bits up between the relocations which
     denote the entry and exit points.  */
  if (last_call_info->start_symbol != NULL)
    {
      char *where = frag_more (0);

      fix_new_hppa (frag_now, where - frag_now->fr_literal, 0,
		    NULL, (offsetT) 0, NULL,
		    0, R_HPPA_ENTRY, e_fsel, 0, 0,
		    (int *) &last_call_info->ci_unwind.descriptor);
    }
#endif
}

/* Handle a .EQU pseudo-op.  */

static void
pa_equ (reg)
     int reg;
{
  label_symbol_struct *label_symbol = pa_get_label ();
  symbolS *symbol;

  if (label_symbol)
    {
      symbol = label_symbol->lss_label;
      if (reg)
	S_SET_VALUE (symbol, pa_parse_number (&input_line_pointer, 0));
      else
	S_SET_VALUE (symbol, (unsigned int) get_absolute_expression ());
      S_SET_SEGMENT (symbol, bfd_abs_section_ptr);
    }
  else
    {
      if (reg)
	as_bad (".REG must use a label");
      else
	as_bad (".EQU must use a label");
    }

  pa_undefine_label ();
  demand_empty_rest_of_line ();
}

/* Helper function.  Does processing for the end of a function.  This
   usually involves creating some relocations or building special
   symbols to mark the end of the function.  */

static void
process_exit ()
{
  char *where;

  where = frag_more (0);

#ifdef OBJ_ELF
  /* Mark the end of the function, stuff away the location of the frag
     for the end of the function, and finally call pa_build_unwind_subspace
     to add an entry in the unwind table.  */
  hppa_elf_mark_end_of_function ();
  pa_build_unwind_subspace (last_call_info);
#else
  /* SOM defers building of unwind descriptors until the link phase.
     The assembler is responsible for creating an R_ENTRY relocation
     to mark the beginning of a region and hold the unwind bits, and
     for creating an R_EXIT relocation to mark the end of the region.

     FIXME.  ELF should be using the same conventions!  The problem
     is an unwind requires too much relocation space.  Hmmm.  Maybe
     if we split the unwind bits up between the relocations which
     denote the entry and exit points.  */
  fix_new_hppa (frag_now, where - frag_now->fr_literal, 0,
		NULL, (offsetT) 0,
		NULL, 0, R_HPPA_EXIT, e_fsel, 0, 0,
		(int *) &last_call_info->ci_unwind.descriptor + 1);
#endif
}

/* Process a .EXIT pseudo-op.  */

static void
pa_exit (unused)
     int unused;
{
  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  if (!within_procedure)
    as_bad (".EXIT must appear within a procedure");
  else
    {
      if (!callinfo_found)
	as_bad ("Missing .callinfo");
      else
	{
	  if (!within_entry_exit)
	    as_bad ("No .ENTRY for this .EXIT");
	  else
	    {
	      within_entry_exit = FALSE;
	      process_exit ();
	    }
	}
    }
  demand_empty_rest_of_line ();
}

/* Process a .EXPORT directive.  This makes functions external
   and provides information such as argument relocation entries
   to callers.  */

static void
pa_export (unused)
     int unused;
{
  char *name, c, *p;
  symbolS *symbol;

  name = input_line_pointer;
  c = get_symbol_end ();
  /* Make sure the given symbol exists.  */
  if ((symbol = symbol_find_or_make (name)) == NULL)
    {
      as_bad ("Cannot define export symbol: %s\n", name);
      p = input_line_pointer;
      *p = c;
      input_line_pointer++;
    }
  else
    {
      /* OK.  Set the external bits and process argument relocations.  */
      S_SET_EXTERNAL (symbol);
      p = input_line_pointer;
      *p = c;
      if (!is_end_of_statement ())
	{
	  input_line_pointer++;
	  pa_type_args (symbol, 1);
	}
    }

  demand_empty_rest_of_line ();
}

/* Helper function to process arguments to a .EXPORT pseudo-op.  */

static void
pa_type_args (symbolP, is_export)
     symbolS *symbolP;
     int is_export;
{
  char *name, c, *p;
  unsigned int temp, arg_reloc;
  pa_symbol_type type = SYMBOL_TYPE_UNKNOWN;
  obj_symbol_type *symbol = (obj_symbol_type *) symbolP->bsym;

  if (strncasecmp (input_line_pointer, "absolute", 8) == 0)

    {
      input_line_pointer += 8;
      symbolP->bsym->flags &= ~BSF_FUNCTION;
      S_SET_SEGMENT (symbolP, bfd_abs_section_ptr);
      type = SYMBOL_TYPE_ABSOLUTE;
    }
  else if (strncasecmp (input_line_pointer, "code", 4) == 0)
    {
      input_line_pointer += 4;
      /* IMPORTing/EXPORTing CODE types for functions is meaningless for SOM,
         instead one should be IMPORTing/EXPORTing ENTRY types.

         Complain if one tries to EXPORT a CODE type since that's never
         done.  Both GCC and HP C still try to IMPORT CODE types, so
         silently fix them to be ENTRY types.  */
      if (symbolP->bsym->flags & BSF_FUNCTION)
	{
	  if (is_export)
	    as_tsktsk ("Using ENTRY rather than CODE in export directive for %s", symbolP->bsym->name);

	  symbolP->bsym->flags |= BSF_FUNCTION;
	  type = SYMBOL_TYPE_ENTRY;
	}
      else
	{
	  symbolP->bsym->flags &= ~BSF_FUNCTION;
	  type = SYMBOL_TYPE_CODE;
	}
    }
  else if (strncasecmp (input_line_pointer, "data", 4) == 0)
    {
      input_line_pointer += 4;
      symbolP->bsym->flags &= ~BSF_FUNCTION;
      type = SYMBOL_TYPE_DATA;
    }
  else if ((strncasecmp (input_line_pointer, "entry", 5) == 0))
    {
      input_line_pointer += 5;
      symbolP->bsym->flags |= BSF_FUNCTION;
      type = SYMBOL_TYPE_ENTRY;
    }
  else if (strncasecmp (input_line_pointer, "millicode", 9) == 0)
    {
      input_line_pointer += 9;
      symbolP->bsym->flags |= BSF_FUNCTION;
      type = SYMBOL_TYPE_MILLICODE;
    }
  else if (strncasecmp (input_line_pointer, "plabel", 6) == 0)
    {
      input_line_pointer += 6;
      symbolP->bsym->flags &= ~BSF_FUNCTION;
      type = SYMBOL_TYPE_PLABEL;
    }
  else if (strncasecmp (input_line_pointer, "pri_prog", 8) == 0)
    {
      input_line_pointer += 8;
      symbolP->bsym->flags |= BSF_FUNCTION;
      type = SYMBOL_TYPE_PRI_PROG;
    }
  else if (strncasecmp (input_line_pointer, "sec_prog", 8) == 0)
    {
      input_line_pointer += 8;
      symbolP->bsym->flags |= BSF_FUNCTION;
      type = SYMBOL_TYPE_SEC_PROG;
    }

  /* SOM requires much more information about symbol types
     than BFD understands.  This is how we get this information
     to the SOM BFD backend.  */
#ifdef obj_set_symbol_type
  obj_set_symbol_type (symbolP->bsym, (int) type);
#endif

  /* Now that the type of the exported symbol has been handled,
     handle any argument relocation information.  */
  while (!is_end_of_statement ())
    {
      if (*input_line_pointer == ',')
	input_line_pointer++;
      name = input_line_pointer;
      c = get_symbol_end ();
      /* Argument sources.  */
      if ((strncasecmp (name, "argw", 4) == 0))
	{
	  p = input_line_pointer;
	  *p = c;
	  input_line_pointer++;
	  temp = atoi (name + 4);
	  name = input_line_pointer;
	  c = get_symbol_end ();
	  arg_reloc = pa_align_arg_reloc (temp, pa_build_arg_reloc (name));
	  symbol->tc_data.hppa_arg_reloc |= arg_reloc;
	  *input_line_pointer = c;
	}
      /* The return value.  */
      else if ((strncasecmp (name, "rtnval", 6)) == 0)
	{
	  p = input_line_pointer;
	  *p = c;
	  input_line_pointer++;
	  name = input_line_pointer;
	  c = get_symbol_end ();
	  arg_reloc = pa_build_arg_reloc (name);
	  symbol->tc_data.hppa_arg_reloc |= arg_reloc;
	  *input_line_pointer = c;
	}
      /* Privelege level.  */
      else if ((strncasecmp (name, "priv_lev", 8)) == 0)
	{
	  p = input_line_pointer;
	  *p = c;
	  input_line_pointer++;
	  temp = atoi (input_line_pointer);
	  c = get_symbol_end ();
	  *input_line_pointer = c;
	}
      else
	{
	  as_bad ("Undefined .EXPORT/.IMPORT argument (ignored): %s", name);
	  p = input_line_pointer;
	  *p = c;
	}
      if (!is_end_of_statement ())
	input_line_pointer++;
    }
}

/* Handle an .IMPORT pseudo-op.  Any symbol referenced in a given
   assembly file must either be defined in the assembly file, or
   explicitly IMPORTED from another.  */

static void
pa_import (unused)
     int unused;
{
  char *name, c, *p;
  symbolS *symbol;

  name = input_line_pointer;
  c = get_symbol_end ();

  symbol = symbol_find (name);
  /* Ugh.  We might be importing a symbol defined earlier in the file,
     in which case all the code below will really screw things up
     (set the wrong segment, symbol flags & type, etc).  */
  if (symbol == NULL || !S_IS_DEFINED (symbol))
    {
      symbol = symbol_find_or_make (name);
      p = input_line_pointer;
      *p = c;

      if (!is_end_of_statement ())
	{
	  input_line_pointer++;
	  pa_type_args (symbol, 0);
	}
      else
	{
	  /* Sigh.  To be compatable with the HP assembler and to help
	     poorly written assembly code, we assign a type based on
	     the the current segment.  Note only BSF_FUNCTION really
	     matters, we do not need to set the full SYMBOL_TYPE_* info.  */
	  if (now_seg == text_section)
	    symbol->bsym->flags |= BSF_FUNCTION;

	  /* If the section is undefined, then the symbol is undefined
	     Since this is an import, leave the section undefined.  */
	  S_SET_SEGMENT (symbol, bfd_und_section_ptr);
	}
    }
  else
    {
      /* The symbol was already defined.  Just eat everything up to
	 the end of the current statement.  */
      while (!is_end_of_statement ())
	input_line_pointer++;
    }

  demand_empty_rest_of_line ();
}

/* Handle a .LABEL pseudo-op.  */

static void
pa_label (unused)
     int unused;
{
  char *name, c, *p;

  name = input_line_pointer;
  c = get_symbol_end ();

  if (strlen (name) > 0)
    {
      colon (name);
      p = input_line_pointer;
      *p = c;
    }
  else
    {
      as_warn ("Missing label name on .LABEL");
    }

  if (!is_end_of_statement ())
    {
      as_warn ("extra .LABEL arguments ignored.");
      ignore_rest_of_line ();
    }
  demand_empty_rest_of_line ();
}

/* Handle a .LEAVE pseudo-op.  This is not supported yet.  */

static void
pa_leave (unused)
     int unused;
{
  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  abort ();
}

/* Handle a .LEVEL pseudo-op.  */

static void
pa_level (unused)
     int unused;
{
  char *level;

  level = input_line_pointer;
  if (strncmp (level, "1.0", 3) == 0)
    {
      input_line_pointer += 3;
      if (!bfd_set_arch_mach (stdoutput, bfd_arch_hppa, 10))
	as_warn ("could not set architecture and machine");
    }
  else if (strncmp (level, "1.1", 3) == 0)
    {
      input_line_pointer += 3;
      if (!bfd_set_arch_mach (stdoutput, bfd_arch_hppa, 11))
	as_warn ("could not set architecture and machine");
    }
  else
    {
      as_bad ("Unrecognized .LEVEL argument\n");
      ignore_rest_of_line ();
    }
  demand_empty_rest_of_line ();
}

/* Handle a .ORIGIN pseudo-op.  */

static void
pa_origin (unused)
     int unused;
{
  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  s_org (0);
  pa_undefine_label ();
}

/* Handle a .PARAM pseudo-op.  This is much like a .EXPORT, except it
   is for static functions.  FIXME.  Should share more code with .EXPORT.  */

static void
pa_param (unused)
     int unused;
{
  char *name, c, *p;
  symbolS *symbol;

  name = input_line_pointer;
  c = get_symbol_end ();

  if ((symbol = symbol_find_or_make (name)) == NULL)
    {
      as_bad ("Cannot define static symbol: %s\n", name);
      p = input_line_pointer;
      *p = c;
      input_line_pointer++;
    }
  else
    {
      S_CLEAR_EXTERNAL (symbol);
      p = input_line_pointer;
      *p = c;
      if (!is_end_of_statement ())
	{
	  input_line_pointer++;
	  pa_type_args (symbol, 0);
	}
    }

  demand_empty_rest_of_line ();
}

/* Handle a .PROC pseudo-op.  It is used to mark the beginning
   of a procedure from a syntatical point of view.  */

static void
pa_proc (unused)
     int unused;
{
  struct call_info *call_info;

  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  if (within_procedure)
    as_fatal ("Nested procedures");

  /* Reset global variables for new procedure.  */
  callinfo_found = FALSE;
  within_procedure = TRUE;

  /* Create another call_info structure.  */
  call_info = (struct call_info *) xmalloc (sizeof (struct call_info));

  if (!call_info)
    as_fatal ("Cannot allocate unwind descriptor\n");

  bzero (call_info, sizeof (struct call_info));

  call_info->ci_next = NULL;

  if (call_info_root == NULL)
    {
      call_info_root = call_info;
      last_call_info = call_info;
    }
  else
    {
      last_call_info->ci_next = call_info;
      last_call_info = call_info;
    }

  /* set up defaults on call_info structure */

  call_info->ci_unwind.descriptor.cannot_unwind = 0;
  call_info->ci_unwind.descriptor.region_desc = 1;
  call_info->ci_unwind.descriptor.hpux_interrupt_marker = 0;

  /* If we got a .PROC pseudo-op, we know that the function is defined
     locally.  Make sure it gets into the symbol table.  */
  {
    label_symbol_struct *label_symbol = pa_get_label ();

    if (label_symbol)
      {
	if (label_symbol->lss_label)
	  {
	    last_call_info->start_symbol = label_symbol->lss_label;
	    label_symbol->lss_label->bsym->flags |= BSF_FUNCTION;
	  }
	else
	  as_bad ("Missing function name for .PROC (corrupted label chain)");
      }
    else
      last_call_info->start_symbol = NULL;
  }

  demand_empty_rest_of_line ();
}

/* Process the syntatical end of a procedure.  Make sure all the
   appropriate pseudo-ops were found within the procedure.  */

static void
pa_procend (unused)
     int unused;
{

  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  /* If we are within a procedure definition, make sure we've
     defined a label for the procedure; handle case where the
     label was defined after the .PROC directive.

     Note there's not need to diddle with the segment or fragment
     for the label symbol in this case.  We have already switched
     into the new $CODE$ subspace at this point.  */
  if (within_procedure && last_call_info->start_symbol == NULL)
    {
      label_symbol_struct *label_symbol = pa_get_label ();

      if (label_symbol)
	{
	  if (label_symbol->lss_label)
	    {
	      last_call_info->start_symbol = label_symbol->lss_label;
	      label_symbol->lss_label->bsym->flags |= BSF_FUNCTION;
#ifdef OBJ_SOM
	      /* Also handle allocation of a fixup to hold the unwind
		 information when the label appears after the proc/procend.  */
	      if (within_entry_exit)
		{
		  char *where = frag_more (0);

		  fix_new_hppa (frag_now, where - frag_now->fr_literal, 0,
				NULL, (offsetT) 0, NULL,
				0, R_HPPA_ENTRY, e_fsel, 0, 0,
				(int *) &last_call_info->ci_unwind.descriptor);
		}
#endif
	    }
	  else
	    as_bad ("Missing function name for .PROC (corrupted label chain)");
	}
      else
	as_bad ("Missing function name for .PROC");
    }

  if (!within_procedure)
    as_bad ("misplaced .procend");

  if (!callinfo_found)
    as_bad ("Missing .callinfo for this procedure");

  if (within_entry_exit)
    as_bad ("Missing .EXIT for a .ENTRY");

#ifdef OBJ_ELF
  /* ELF needs to mark the end of each function so that it can compute
     the size of the function (apparently its needed in the symbol table).  */
  hppa_elf_mark_end_of_function ();
#endif

  within_procedure = FALSE;
  demand_empty_rest_of_line ();
  pa_undefine_label ();
}

/* Parse the parameters to a .SPACE directive; if CREATE_FLAG is nonzero,
   then create a new space entry to hold the information specified
   by the parameters to the .SPACE directive.  */

static sd_chain_struct *
pa_parse_space_stmt (space_name, create_flag)
     char *space_name;
     int create_flag;
{
  char *name, *ptemp, c;
  char loadable, defined, private, sort;
  int spnum, temp;
  asection *seg = NULL;
  sd_chain_struct *space;

  /* load default values */
  spnum = 0;
  sort = 0;
  loadable = TRUE;
  defined = TRUE;
  private = FALSE;
  if (strcmp (space_name, "$TEXT$") == 0)
    {
      seg = pa_def_spaces[0].segment;
      defined = pa_def_spaces[0].defined;
      private = pa_def_spaces[0].private;
      sort = pa_def_spaces[0].sort;
      spnum = pa_def_spaces[0].spnum;
    }
  else if (strcmp (space_name, "$PRIVATE$") == 0)
    {
      seg = pa_def_spaces[1].segment;
      defined = pa_def_spaces[1].defined;
      private = pa_def_spaces[1].private;
      sort = pa_def_spaces[1].sort;
      spnum = pa_def_spaces[1].spnum;
    }

  if (!is_end_of_statement ())
    {
      print_errors = FALSE;
      ptemp = input_line_pointer + 1;
      /* First see if the space was specified as a number rather than
         as a name.  According to the PA assembly manual the rest of
         the line should be ignored.  */
      temp = pa_parse_number (&ptemp, 0);
      if (temp >= 0)
	{
	  spnum = temp;
	  input_line_pointer = ptemp;
	}
      else
	{
	  while (!is_end_of_statement ())
	    {
	      input_line_pointer++;
	      name = input_line_pointer;
	      c = get_symbol_end ();
	      if ((strncasecmp (name, "spnum", 5) == 0))
		{
		  *input_line_pointer = c;
		  input_line_pointer++;
		  spnum = get_absolute_expression ();
		}
	      else if ((strncasecmp (name, "sort", 4) == 0))
		{
		  *input_line_pointer = c;
		  input_line_pointer++;
		  sort = get_absolute_expression ();
		}
	      else if ((strncasecmp (name, "unloadable", 10) == 0))
		{
		  *input_line_pointer = c;
		  loadable = FALSE;
		}
	      else if ((strncasecmp (name, "notdefined", 10) == 0))
		{
		  *input_line_pointer = c;
		  defined = FALSE;
		}
	      else if ((strncasecmp (name, "private", 7) == 0))
		{
		  *input_line_pointer = c;
		  private = TRUE;
		}
	      else
		{
		  as_bad ("Invalid .SPACE argument");
		  *input_line_pointer = c;
		  if (!is_end_of_statement ())
		    input_line_pointer++;
		}
	    }
	}
      print_errors = TRUE;
    }

  if (create_flag && seg == NULL)
    seg = subseg_new (space_name, 0);

  /* If create_flag is nonzero, then create the new space with
     the attributes computed above.  Else set the values in
     an already existing space -- this can only happen for
     the first occurence of a built-in space.  */
  if (create_flag)
    space = create_new_space (space_name, spnum, loadable, defined,
			      private, sort, seg, 1);
  else
    {
      space = is_defined_space (space_name);
      SPACE_SPNUM (space) = spnum;
      SPACE_DEFINED (space) = defined & 1;
      SPACE_USER_DEFINED (space) = 1;
    }

#ifdef obj_set_section_attributes
  obj_set_section_attributes (seg, defined, private, sort, spnum);
#endif

  return space;
}

/* Handle a .SPACE pseudo-op; this switches the current space to the
   given space, creating the new space if necessary.  */

static void
pa_space (unused)
     int unused;
{
  char *name, c, *space_name, *save_s;
  int temp;
  sd_chain_struct *sd_chain;

  if (within_procedure)
    {
      as_bad ("Can\'t change spaces within a procedure definition. Ignored");
      ignore_rest_of_line ();
    }
  else
    {
      /* Check for some of the predefined spaces.   FIXME: most of the code
         below is repeated several times, can we extract the common parts
         and place them into a subroutine or something similar?  */
      /* FIXME Is this (and the next IF stmt) really right?
	 What if INPUT_LINE_POINTER points to "$TEXT$FOO"?  */
      if (strncmp (input_line_pointer, "$TEXT$", 6) == 0)
	{
	  input_line_pointer += 6;
	  sd_chain = is_defined_space ("$TEXT$");
	  if (sd_chain == NULL)
	    sd_chain = pa_parse_space_stmt ("$TEXT$", 1);
	  else if (SPACE_USER_DEFINED (sd_chain) == 0)
	    sd_chain = pa_parse_space_stmt ("$TEXT$", 0);

	  current_space = sd_chain;
	  subseg_set (text_section, sd_chain->sd_last_subseg);
	  current_subspace
	    = pa_subsegment_to_subspace (text_section,
					 sd_chain->sd_last_subseg);
	  demand_empty_rest_of_line ();
	  return;
	}
      if (strncmp (input_line_pointer, "$PRIVATE$", 9) == 0)
	{
	  input_line_pointer += 9;
	  sd_chain = is_defined_space ("$PRIVATE$");
	  if (sd_chain == NULL)
	    sd_chain = pa_parse_space_stmt ("$PRIVATE$", 1);
	  else if (SPACE_USER_DEFINED (sd_chain) == 0)
	    sd_chain = pa_parse_space_stmt ("$PRIVATE$", 0);

	  current_space = sd_chain;
	  subseg_set (data_section, sd_chain->sd_last_subseg);
	  current_subspace
	    = pa_subsegment_to_subspace (data_section,
					 sd_chain->sd_last_subseg);
	  demand_empty_rest_of_line ();
	  return;
	}
      if (!strncasecmp (input_line_pointer,
			GDB_DEBUG_SPACE_NAME,
			strlen (GDB_DEBUG_SPACE_NAME)))
	{
	  input_line_pointer += strlen (GDB_DEBUG_SPACE_NAME);
	  sd_chain = is_defined_space (GDB_DEBUG_SPACE_NAME);
	  if (sd_chain == NULL)
	    sd_chain = pa_parse_space_stmt (GDB_DEBUG_SPACE_NAME, 1);
	  else if (SPACE_USER_DEFINED (sd_chain) == 0)
	    sd_chain = pa_parse_space_stmt (GDB_DEBUG_SPACE_NAME, 0);

	  current_space = sd_chain;

	  {
	    asection *gdb_section
	    = bfd_make_section_old_way (stdoutput, GDB_DEBUG_SPACE_NAME);

	    subseg_set (gdb_section, sd_chain->sd_last_subseg);
	    current_subspace
	      = pa_subsegment_to_subspace (gdb_section,
					   sd_chain->sd_last_subseg);
	  }
	  demand_empty_rest_of_line ();
	  return;
	}

      /* It could be a space specified by number.  */
      print_errors = 0;
      save_s = input_line_pointer;
      if ((temp = pa_parse_number (&input_line_pointer, 0)) >= 0)
	{
	  if ((sd_chain = pa_find_space_by_number (temp)))
	    {
	      current_space = sd_chain;

	      subseg_set (sd_chain->sd_seg, sd_chain->sd_last_subseg);
	      current_subspace
		= pa_subsegment_to_subspace (sd_chain->sd_seg,
					     sd_chain->sd_last_subseg);
	      demand_empty_rest_of_line ();
	      return;
	    }
	}

      /* Not a number, attempt to create a new space.  */
      print_errors = 1;
      input_line_pointer = save_s;
      name = input_line_pointer;
      c = get_symbol_end ();
      space_name = xmalloc (strlen (name) + 1);
      strcpy (space_name, name);
      *input_line_pointer = c;

      sd_chain = pa_parse_space_stmt (space_name, 1);
      current_space = sd_chain;

      subseg_set (sd_chain->sd_seg, sd_chain->sd_last_subseg);
      current_subspace = pa_subsegment_to_subspace (sd_chain->sd_seg,
						  sd_chain->sd_last_subseg);
      demand_empty_rest_of_line ();
    }
}

/* Switch to a new space.  (I think).  FIXME.  */

static void
pa_spnum (unused)
     int unused;
{
  char *name;
  char c;
  char *p;
  sd_chain_struct *space;

  name = input_line_pointer;
  c = get_symbol_end ();
  space = is_defined_space (name);
  if (space)
    {
      p = frag_more (4);
      md_number_to_chars (p, SPACE_SPNUM (space), 4);
    }
  else
    as_warn ("Undefined space: '%s' Assuming space number = 0.", name);

  *input_line_pointer = c;
  demand_empty_rest_of_line ();
}

/* If VALUE is an exact power of two between zero and 2^31, then
   return log2 (VALUE).  Else return -1.  */

static int
log2 (value)
     int value;
{
  int shift = 0;

  while ((1 << shift) != value && shift < 32)
    shift++;

  if (shift >= 32)
    return -1;
  else
    return shift;
}

/* Handle a .SUBSPACE pseudo-op; this switches the current subspace to the
   given subspace, creating the new subspace if necessary.

   FIXME.  Should mirror pa_space more closely, in particular how
   they're broken up into subroutines.  */

static void
pa_subspace (create_new)
     int create_new;
{
  char *name, *ss_name, *alias, c;
  char loadable, code_only, common, dup_common, zero, sort;
  int i, access, space_index, alignment, quadrant, applicable, flags;
  sd_chain_struct *space;
  ssd_chain_struct *ssd;
  asection *section;

  if (current_space == NULL)
    as_fatal ("Must be in a space before changing or declaring subspaces.\n");

  if (within_procedure)
    {
      as_bad ("Can\'t change subspaces within a procedure definition. Ignored");
      ignore_rest_of_line ();
    }
  else
    {
      name = input_line_pointer;
      c = get_symbol_end ();
      ss_name = xmalloc (strlen (name) + 1);
      strcpy (ss_name, name);
      *input_line_pointer = c;

      /* Load default values.  */
      sort = 0;
      access = 0x7f;
      loadable = 1;
      common = 0;
      dup_common = 0;
      code_only = 0;
      zero = 0;
      space_index = ~0;
      alignment = 1;
      quadrant = 0;
      alias = NULL;

      space = current_space;
      if (create_new)
	ssd = NULL;
      else
	ssd = is_defined_subspace (ss_name);
      /* Allow user to override the builtin attributes of subspaces.  But
         only allow the attributes to be changed once!  */
      if (ssd && SUBSPACE_DEFINED (ssd))
	{
	  subseg_set (ssd->ssd_seg, ssd->ssd_subseg);
	  current_subspace = ssd;
	  if (!is_end_of_statement ())
	    as_warn ("Parameters of an existing subspace can\'t be modified");
	  demand_empty_rest_of_line ();
	  return;
	}
      else
	{
	  /* A new subspace.  Load default values if it matches one of
	     the builtin subspaces.  */
	  i = 0;
	  while (pa_def_subspaces[i].name)
	    {
	      if (strcasecmp (pa_def_subspaces[i].name, ss_name) == 0)
		{
		  loadable = pa_def_subspaces[i].loadable;
		  common = pa_def_subspaces[i].common;
		  dup_common = pa_def_subspaces[i].dup_common;
		  code_only = pa_def_subspaces[i].code_only;
		  zero = pa_def_subspaces[i].zero;
		  space_index = pa_def_subspaces[i].space_index;
		  alignment = pa_def_subspaces[i].alignment;
		  quadrant = pa_def_subspaces[i].quadrant;
		  access = pa_def_subspaces[i].access;
		  sort = pa_def_subspaces[i].sort;
		  if (USE_ALIASES && pa_def_subspaces[i].alias)
		    alias = pa_def_subspaces[i].alias;
		  break;
		}
	      i++;
	    }
	}

      /* We should be working with a new subspace now.  Fill in
         any information as specified by the user.  */
      if (!is_end_of_statement ())
	{
	  input_line_pointer++;
	  while (!is_end_of_statement ())
	    {
	      name = input_line_pointer;
	      c = get_symbol_end ();
	      if ((strncasecmp (name, "quad", 4) == 0))
		{
		  *input_line_pointer = c;
		  input_line_pointer++;
		  quadrant = get_absolute_expression ();
		}
	      else if ((strncasecmp (name, "align", 5) == 0))
		{
		  *input_line_pointer = c;
		  input_line_pointer++;
		  alignment = get_absolute_expression ();
		  if (log2 (alignment) == -1)
		    {
		      as_bad ("Alignment must be a power of 2");
		      alignment = 1;
		    }
		}
	      else if ((strncasecmp (name, "access", 6) == 0))
		{
		  *input_line_pointer = c;
		  input_line_pointer++;
		  access = get_absolute_expression ();
		}
	      else if ((strncasecmp (name, "sort", 4) == 0))
		{
		  *input_line_pointer = c;
		  input_line_pointer++;
		  sort = get_absolute_expression ();
		}
	      else if ((strncasecmp (name, "code_only", 9) == 0))
		{
		  *input_line_pointer = c;
		  code_only = 1;
		}
	      else if ((strncasecmp (name, "unloadable", 10) == 0))
		{
		  *input_line_pointer = c;
		  loadable = 0;
		}
	      else if ((strncasecmp (name, "common", 6) == 0))
		{
		  *input_line_pointer = c;
		  common = 1;
		}
	      else if ((strncasecmp (name, "dup_comm", 8) == 0))
		{
		  *input_line_pointer = c;
		  dup_common = 1;
		}
	      else if ((strncasecmp (name, "zero", 4) == 0))
		{
		  *input_line_pointer = c;
		  zero = 1;
		}
	      else if ((strncasecmp (name, "first", 5) == 0))
		as_bad ("FIRST not supported as a .SUBSPACE argument");
	      else
		as_bad ("Invalid .SUBSPACE argument");
	      if (!is_end_of_statement ())
		input_line_pointer++;
	    }
	}

      /* Compute a reasonable set of BFD flags based on the information
         in the .subspace directive.  */
      applicable = bfd_applicable_section_flags (stdoutput);
      flags = 0;
      if (loadable)
	flags |= (SEC_ALLOC | SEC_LOAD);
      if (code_only)
	flags |= SEC_CODE;
      if (common || dup_common)
	flags |= SEC_IS_COMMON;

      flags |= SEC_RELOC | SEC_HAS_CONTENTS;

      /* This is a zero-filled subspace (eg BSS).  */
      if (zero)
	flags &= ~(SEC_LOAD | SEC_HAS_CONTENTS);

      applicable &= flags;

      /* If this is an existing subspace, then we want to use the
         segment already associated with the subspace.

         FIXME NOW!  ELF BFD doesn't appear to be ready to deal with
         lots of sections.  It might be a problem in the PA ELF
         code, I do not know yet.  For now avoid creating anything
         but the "standard" sections for ELF.  */
      if (create_new)
	section = subseg_force_new (ss_name, 0);
      else if (ssd)
	section = ssd->ssd_seg;
      else if (alias)
	section = subseg_new (alias, 0);
      else if (!alias && USE_ALIASES)
	{
	  as_warn ("Ignoring subspace decl due to ELF BFD bugs.");
	  demand_empty_rest_of_line ();
	  return;
	}
      else
	section = subseg_new (ss_name, 0);

      if (zero)
	seg_info (section)->bss = 1;

      /* Now set the flags.  */
      bfd_set_section_flags (stdoutput, section, applicable);

      /* Record any alignment request for this section.  */
      record_alignment (section, log2 (alignment));

      /* Set the starting offset for this section.  */
      bfd_set_section_vma (stdoutput, section,
			   pa_subspace_start (space, quadrant));

      /* Now that all the flags are set, update an existing subspace,
         or create a new one.  */
      if (ssd)

	current_subspace = update_subspace (space, ss_name, loadable,
					    code_only, common, dup_common,
					    sort, zero, access, space_index,
					    alignment, quadrant,
					    section);
      else
	current_subspace = create_new_subspace (space, ss_name, loadable,
						code_only, common,
						dup_common, zero, sort,
						access, space_index,
					      alignment, quadrant, section);

      demand_empty_rest_of_line ();
      current_subspace->ssd_seg = section;
      subseg_set (current_subspace->ssd_seg, current_subspace->ssd_subseg);
    }
  SUBSPACE_DEFINED (current_subspace) = 1;
}


/* Create default space and subspace dictionaries.  */

static void
pa_spaces_begin ()
{
  int i;

  space_dict_root = NULL;
  space_dict_last = NULL;

  i = 0;
  while (pa_def_spaces[i].name)
    {
      char *name;

      /* Pick the right name to use for the new section.  */
      if (pa_def_spaces[i].alias && USE_ALIASES)
	name = pa_def_spaces[i].alias;
      else
	name = pa_def_spaces[i].name;

      pa_def_spaces[i].segment = subseg_new (name, 0);
      create_new_space (pa_def_spaces[i].name, pa_def_spaces[i].spnum,
			pa_def_spaces[i].loadable, pa_def_spaces[i].defined,
			pa_def_spaces[i].private, pa_def_spaces[i].sort,
			pa_def_spaces[i].segment, 0);
      i++;
    }

  i = 0;
  while (pa_def_subspaces[i].name)
    {
      char *name;
      int applicable, subsegment;
      asection *segment = NULL;
      sd_chain_struct *space;

      /* Pick the right name for the new section and pick the right
         subsegment number.  */
      if (pa_def_subspaces[i].alias && USE_ALIASES)
	{
	  name = pa_def_subspaces[i].alias;
	  subsegment = pa_def_subspaces[i].subsegment;
	}
      else
	{
	  name = pa_def_subspaces[i].name;
	  subsegment = 0;
	}

      /* Create the new section.  */
      segment = subseg_new (name, subsegment);


      /* For SOM we want to replace the standard .text, .data, and .bss
         sections with our own.   We also want to set BFD flags for
	 all the built-in subspaces.  */
      if (!strcmp (pa_def_subspaces[i].name, "$CODE$") && !USE_ALIASES)
	{
	  text_section = segment;
	  applicable = bfd_applicable_section_flags (stdoutput);
	  bfd_set_section_flags (stdoutput, segment,
				 applicable & (SEC_ALLOC | SEC_LOAD
					       | SEC_RELOC | SEC_CODE
					       | SEC_READONLY
					       | SEC_HAS_CONTENTS));
	}
      else if (!strcmp (pa_def_subspaces[i].name, "$DATA$") && !USE_ALIASES)
	{
	  data_section = segment;
	  applicable = bfd_applicable_section_flags (stdoutput);
	  bfd_set_section_flags (stdoutput, segment,
				 applicable & (SEC_ALLOC | SEC_LOAD
					       | SEC_RELOC
					       | SEC_HAS_CONTENTS));


	}
      else if (!strcmp (pa_def_subspaces[i].name, "$BSS$") && !USE_ALIASES)
	{
	  bss_section = segment;
	  applicable = bfd_applicable_section_flags (stdoutput);
	  bfd_set_section_flags (stdoutput, segment,
				 applicable & SEC_ALLOC);
	}
      else if (!strcmp (pa_def_subspaces[i].name, "$LIT$") && !USE_ALIASES)
	{
	  applicable = bfd_applicable_section_flags (stdoutput);
	  bfd_set_section_flags (stdoutput, segment,
				 applicable & (SEC_ALLOC | SEC_LOAD
					       | SEC_RELOC
					       | SEC_READONLY
					       | SEC_HAS_CONTENTS));
	}
      else if (!strcmp (pa_def_subspaces[i].name, "$MILLICODE$")
	       && !USE_ALIASES)
	{
	  applicable = bfd_applicable_section_flags (stdoutput);
	  bfd_set_section_flags (stdoutput, segment,
				 applicable & (SEC_ALLOC | SEC_LOAD
					       | SEC_RELOC
					       | SEC_READONLY
					       | SEC_HAS_CONTENTS));
	}
      else if (!strcmp (pa_def_subspaces[i].name, "$UNWIND$") && !USE_ALIASES)
	{
	  applicable = bfd_applicable_section_flags (stdoutput);
	  bfd_set_section_flags (stdoutput, segment,
				 applicable & (SEC_ALLOC | SEC_LOAD
					       | SEC_RELOC
					       | SEC_READONLY
					       | SEC_HAS_CONTENTS));
	}

      /* Find the space associated with this subspace.  */
      space = pa_segment_to_space (pa_def_spaces[pa_def_subspaces[i].
						 def_space_index].segment);
      if (space == NULL)
	{
	  as_fatal ("Internal error: Unable to find containing space for %s.",
		    pa_def_subspaces[i].name);
	}

      create_new_subspace (space, name,
			   pa_def_subspaces[i].loadable,
			   pa_def_subspaces[i].code_only,
			   pa_def_subspaces[i].common,
			   pa_def_subspaces[i].dup_common,
			   pa_def_subspaces[i].zero,
			   pa_def_subspaces[i].sort,
			   pa_def_subspaces[i].access,
			   pa_def_subspaces[i].space_index,
			   pa_def_subspaces[i].alignment,
			   pa_def_subspaces[i].quadrant,
			   segment);
      i++;
    }
}



/* Create a new space NAME, with the appropriate flags as defined
   by the given parameters.  */

static sd_chain_struct *
create_new_space (name, spnum, loadable, defined, private,
		  sort, seg, user_defined)
     char *name;
     int spnum;
     int loadable;
     int defined;
     int private;
     int sort;
     asection *seg;
     int user_defined;
{
  sd_chain_struct *chain_entry;

  chain_entry = (sd_chain_struct *) xmalloc (sizeof (sd_chain_struct));
  if (!chain_entry)
    as_fatal ("Out of memory: could not allocate new space chain entry: %s\n",
	      name);

  SPACE_NAME (chain_entry) = (char *) xmalloc (strlen (name) + 1);
  strcpy (SPACE_NAME (chain_entry), name);
  SPACE_DEFINED (chain_entry) = defined;
  SPACE_USER_DEFINED (chain_entry) = user_defined;
  SPACE_SPNUM (chain_entry) = spnum;

  chain_entry->sd_seg = seg;
  chain_entry->sd_last_subseg = -1;
  chain_entry->sd_subspaces = NULL;
  chain_entry->sd_next = NULL;

  /* Find spot for the new space based on its sort key.  */
  if (!space_dict_last)
    space_dict_last = chain_entry;

  if (space_dict_root == NULL)
    space_dict_root = chain_entry;
  else
    {
      sd_chain_struct *chain_pointer;
      sd_chain_struct *prev_chain_pointer;

      chain_pointer = space_dict_root;
      prev_chain_pointer = NULL;

      while (chain_pointer)
	{
	  prev_chain_pointer = chain_pointer;
	  chain_pointer = chain_pointer->sd_next;
	}

      /* At this point we've found the correct place to add the new
         entry.  So add it and update the linked lists as appropriate.  */
      if (prev_chain_pointer)
	{
	  chain_entry->sd_next = chain_pointer;
	  prev_chain_pointer->sd_next = chain_entry;
	}
      else
	{
	  space_dict_root = chain_entry;
	  chain_entry->sd_next = chain_pointer;
	}

      if (chain_entry->sd_next == NULL)
	space_dict_last = chain_entry;
    }

  /* This is here to catch predefined spaces which do not get
     modified by the user's input.  Another call is found at
     the bottom of pa_parse_space_stmt to handle cases where
     the user modifies a predefined space.  */
#ifdef obj_set_section_attributes
  obj_set_section_attributes (seg, defined, private, sort, spnum);
#endif

  return chain_entry;
}

/* Create a new subspace NAME, with the appropriate flags as defined
   by the given parameters.

   Add the new subspace to the subspace dictionary chain in numerical
   order as defined by the SORT entries.  */

static ssd_chain_struct *
create_new_subspace (space, name, loadable, code_only, common,
		     dup_common, is_zero, sort, access, space_index,
		     alignment, quadrant, seg)
     sd_chain_struct *space;
     char *name;
     int loadable, code_only, common, dup_common, is_zero;
     int sort;
     int access;
     int space_index;
     int alignment;
     int quadrant;
     asection *seg;
{
  ssd_chain_struct *chain_entry;

  chain_entry = (ssd_chain_struct *) xmalloc (sizeof (ssd_chain_struct));
  if (!chain_entry)
    as_fatal ("Out of memory: could not allocate new subspace chain entry: %s\n", name);

  SUBSPACE_NAME (chain_entry) = (char *) xmalloc (strlen (name) + 1);
  strcpy (SUBSPACE_NAME (chain_entry), name);

  /* Initialize subspace_defined.  When we hit a .subspace directive
     we'll set it to 1 which "locks-in" the subspace attributes.  */
  SUBSPACE_DEFINED (chain_entry) = 0;

  chain_entry->ssd_subseg = USE_ALIASES ? pa_next_subseg (space) : 0;
  chain_entry->ssd_seg = seg;
  chain_entry->ssd_next = NULL;

  /* Find spot for the new subspace based on its sort key.  */
  if (space->sd_subspaces == NULL)
    space->sd_subspaces = chain_entry;
  else
    {
      ssd_chain_struct *chain_pointer;
      ssd_chain_struct *prev_chain_pointer;

      chain_pointer = space->sd_subspaces;
      prev_chain_pointer = NULL;

      while (chain_pointer)
	{
	  prev_chain_pointer = chain_pointer;
	  chain_pointer = chain_pointer->ssd_next;
	}

      /* Now we have somewhere to put the new entry.  Insert it and update
         the links.  */
      if (prev_chain_pointer)
	{
	  chain_entry->ssd_next = chain_pointer;
	  prev_chain_pointer->ssd_next = chain_entry;
	}
      else
	{
	  space->sd_subspaces = chain_entry;
	  chain_entry->ssd_next = chain_pointer;
	}
    }

#ifdef obj_set_subsection_attributes
  obj_set_subsection_attributes (seg, space->sd_seg, access,
				 sort, quadrant);
#endif

  return chain_entry;
}

/* Update the information for the given subspace based upon the
   various arguments.   Return the modified subspace chain entry.  */

static ssd_chain_struct *
update_subspace (space, name, loadable, code_only, common, dup_common, sort,
		 zero, access, space_index, alignment, quadrant, section)
     sd_chain_struct *space;
     char *name;
     int loadable;
     int code_only;
     int common;
     int dup_common;
     int zero;
     int sort;
     int access;
     int space_index;
     int alignment;
     int quadrant;
     asection *section;
{
  ssd_chain_struct *chain_entry;

  chain_entry = is_defined_subspace (name);

#ifdef obj_set_subsection_attributes
  obj_set_subsection_attributes (section, space->sd_seg, access,
				 sort, quadrant);
#endif

  return chain_entry;
}

/* Return the space chain entry for the space with the name NAME or
   NULL if no such space exists.  */

static sd_chain_struct *
is_defined_space (name)
     char *name;
{
  sd_chain_struct *chain_pointer;

  for (chain_pointer = space_dict_root;
       chain_pointer;
       chain_pointer = chain_pointer->sd_next)
    {
      if (strcmp (SPACE_NAME (chain_pointer), name) == 0)
	return chain_pointer;
    }

  /* No mapping from segment to space was found.  Return NULL.  */
  return NULL;
}

/* Find and return the space associated with the given seg.  If no mapping
   from the given seg to a space is found, then return NULL.

   Unlike subspaces, the number of spaces is not expected to grow much,
   so a linear exhaustive search is OK here.  */

static sd_chain_struct *
pa_segment_to_space (seg)
     asection *seg;
{
  sd_chain_struct *space_chain;

  /* Walk through each space looking for the correct mapping.  */
  for (space_chain = space_dict_root;
       space_chain;
       space_chain = space_chain->sd_next)
    {
      if (space_chain->sd_seg == seg)
	return space_chain;
    }

  /* Mapping was not found.  Return NULL.  */
  return NULL;
}

/* Return the space chain entry for the subspace with the name NAME or
   NULL if no such subspace exists.

   Uses a linear search through all the spaces and subspaces, this may
   not be appropriate if we ever being placing each function in its
   own subspace.  */

static ssd_chain_struct *
is_defined_subspace (name)
     char *name;
{
  sd_chain_struct *space_chain;
  ssd_chain_struct *subspace_chain;

  /* Walk through each space.  */
  for (space_chain = space_dict_root;
       space_chain;
       space_chain = space_chain->sd_next)
    {
      /* Walk through each subspace looking for a name which matches.  */
      for (subspace_chain = space_chain->sd_subspaces;
	   subspace_chain;
	   subspace_chain = subspace_chain->ssd_next)
	if (strcmp (SUBSPACE_NAME (subspace_chain), name) == 0)
	  return subspace_chain;
    }

  /* Subspace wasn't found.  Return NULL.  */
  return NULL;
}

/* Find and return the subspace associated with the given seg.  If no
   mapping from the given seg to a subspace is found, then return NULL.

   If we ever put each procedure/function within its own subspace
   (to make life easier on the compiler and linker), then this will have
   to become more efficient.  */

static ssd_chain_struct *
pa_subsegment_to_subspace (seg, subseg)
     asection *seg;
     subsegT subseg;
{
  sd_chain_struct *space_chain;
  ssd_chain_struct *subspace_chain;

  /* Walk through each space.  */
  for (space_chain = space_dict_root;
       space_chain;
       space_chain = space_chain->sd_next)
    {
      if (space_chain->sd_seg == seg)
	{
	  /* Walk through each subspace within each space looking for
	     the correct mapping.  */
	  for (subspace_chain = space_chain->sd_subspaces;
	       subspace_chain;
	       subspace_chain = subspace_chain->ssd_next)
	    if (subspace_chain->ssd_subseg == (int) subseg)
	      return subspace_chain;
	}
    }

  /* No mapping from subsegment to subspace found.  Return NULL.  */
  return NULL;
}

/* Given a number, try and find a space with the name number.

   Return a pointer to a space dictionary chain entry for the space
   that was found or NULL on failure.  */

static sd_chain_struct *
pa_find_space_by_number (number)
     int number;
{
  sd_chain_struct *space_chain;

  for (space_chain = space_dict_root;
       space_chain;
       space_chain = space_chain->sd_next)
    {
      if (SPACE_SPNUM (space_chain) == number)
	return space_chain;
    }

  /* No appropriate space found.  Return NULL.  */
  return NULL;
}

/* Return the starting address for the given subspace.  If the starting
   address is unknown then return zero.  */

static unsigned int
pa_subspace_start (space, quadrant)
     sd_chain_struct *space;
     int quadrant;
{
  /* FIXME.  Assumes everyone puts read/write data at 0x4000000, this
     is not correct for the PA OSF1 port.  */
  if ((strcmp (SPACE_NAME (space), "$PRIVATE$") == 0) && quadrant == 1)
    return 0x40000000;
  else if (space->sd_seg == data_section && quadrant == 1)
    return 0x40000000;
  else
    return 0;
}

/* FIXME.  Needs documentation.  */
static int
pa_next_subseg (space)
     sd_chain_struct *space;
{

  space->sd_last_subseg++;
  return space->sd_last_subseg;
}

/* Helper function for pa_stringer.  Used to find the end of
   a string.  */

static unsigned int
pa_stringer_aux (s)
     char *s;
{
  unsigned int c = *s & CHAR_MASK;

  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  switch (c)
    {
    case '\"':
      c = NOT_A_CHAR;
      break;
    default:
      break;
    }
  return c;
}

/* Handle a .STRING type pseudo-op.  */

static void
pa_stringer (append_zero)
     int append_zero;
{
  char *s, num_buf[4];
  unsigned int c;
  int i;

  /* Preprocess the string to handle PA-specific escape sequences.
     For example, \xDD where DD is a hexidecimal number should be
     changed to \OOO where OOO is an octal number.  */

  /* Skip the opening quote.  */
  s = input_line_pointer + 1;

  while (is_a_char (c = pa_stringer_aux (s++)))
    {
      if (c == '\\')
	{
	  c = *s;
	  switch (c)
	    {
	      /* Handle \x<num>.  */
	    case 'x':
	      {
		unsigned int number;
		int num_digit;
		char dg;
		char *s_start = s;

		/* Get pas the 'x'.  */
		s++;
		for (num_digit = 0, number = 0, dg = *s;
		     num_digit < 2
		     && (isdigit (dg) || (dg >= 'a' && dg <= 'f')
			 || (dg >= 'A' && dg <= 'F'));
		     num_digit++)
		  {
		    if (isdigit (dg))
		      number = number * 16 + dg - '0';
		    else if (dg >= 'a' && dg <= 'f')
		      number = number * 16 + dg - 'a' + 10;
		    else
		      number = number * 16 + dg - 'A' + 10;

		    s++;
		    dg = *s;
		  }
		if (num_digit > 0)
		  {
		    switch (num_digit)
		      {
		      case 1:
			sprintf (num_buf, "%02o", number);
			break;
		      case 2:
			sprintf (num_buf, "%03o", number);
			break;
		      }
		    for (i = 0; i <= num_digit; i++)
		      s_start[i] = num_buf[i];
		  }
		break;
	      }
	    /* This might be a "\"", skip over the escaped char.  */
	    default:
	      s++;
	      break;
	    }
	}
    }
  stringer (append_zero);
  pa_undefine_label ();
}

/* Handle a .VERSION pseudo-op.  */

static void
pa_version (unused)
     int unused;
{
  obj_version (0);
  pa_undefine_label ();
}

/* Handle a .COPYRIGHT pseudo-op.  */

static void
pa_copyright (unused)
     int unused;
{
  obj_copyright (0);
  pa_undefine_label ();
}

/* Just like a normal cons, but when finished we have to undefine
   the latest space label.  */

static void
pa_cons (nbytes)
     int nbytes;
{
  cons (nbytes);
  pa_undefine_label ();
}

/* Switch to the data space.  As usual delete our label.  */

static void
pa_data (unused)
     int unused;
{
  current_space = is_defined_space ("$PRIVATE$");
  current_subspace
    = pa_subsegment_to_subspace (current_space->sd_seg, 0);
  s_data (0);
  pa_undefine_label ();
}

/* Like float_cons, but we need to undefine our label.  */

static void
pa_float_cons (float_type)
     int float_type;
{
  float_cons (float_type);
  pa_undefine_label ();
}

/* Like s_fill, but delete our label when finished.  */

static void
pa_fill (unused)
     int unused;
{
  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  s_fill (0);
  pa_undefine_label ();
}

/* Like lcomm, but delete our label when finished.  */

static void
pa_lcomm (needs_align)
     int needs_align;
{
  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  s_lcomm (needs_align);
  pa_undefine_label ();
}

/* Like lsym, but delete our label when finished.  */

static void
pa_lsym (unused)
     int unused;
{
  /* We must have a valid space and subspace.  */
  pa_check_current_space_and_subspace ();

  s_lsym (0);
  pa_undefine_label ();
}

/* Switch to the text space.  Like s_text, but delete our
   label when finished.  */
static void
pa_text (unused)
     int unused;
{
  current_space = is_defined_space ("$TEXT$");
  current_subspace
    = pa_subsegment_to_subspace (current_space->sd_seg, 0);

  s_text (0);
  pa_undefine_label ();
}

/* On the PA relocations which involve function symbols must not be
   adjusted.  This so that the linker can know when/how to create argument
   relocation stubs for indirect calls and calls to static functions.

   "T" field selectors create DLT relative fixups for accessing
   globals and statics in PIC code; each DLT relative fixup creates
   an entry in the DLT table.  The entries contain the address of
   the final target (eg accessing "foo" would create a DLT entry
   with the address of "foo").

   Unfortunately, the HP linker doesn't take into account any addend
   when generating the DLT; so accessing $LIT$+8 puts the address of
   $LIT$ into the DLT rather than the address of $LIT$+8.

   The end result is we can't perform relocation symbol reductions for
   any fixup which creates entries in the DLT (eg they use "T" field
   selectors).

   Reject reductions involving symbols with external scope; such
   reductions make life a living hell for object file editors. 

   FIXME.  Also reject R_HPPA relocations which are 32bits wide in
   the code space.  The SOM BFD backend doesn't know how to pull the
   right bits out of an instruction.  */

int
hppa_fix_adjustable (fixp)
     fixS *fixp;
{
  struct hppa_fix_struct *hppa_fix;

  hppa_fix = (struct hppa_fix_struct *) fixp->tc_fix_data;

#ifdef OBJ_SOM
  /* Reject reductions of symbols in 32bit relocs.  */
  if (fixp->fx_r_type == R_HPPA && hppa_fix->fx_r_format == 32)
    return 0;

  /* Reject reductions of symbols in sym1-sym2 expressions when
     the fixup will occur in a CODE subspace. 

     XXX FIXME: Long term we probably want to reject all of these;
     for example reducing in the debug section would lose if we ever
     supported using the optimizing hp linker.  */
  if (fixp->fx_addsy
      && fixp->fx_subsy
      && (hppa_fix->segment->flags & SEC_CODE))
    {
      /* Apparently sy_used_in_reloc never gets set for sub symbols.  */
      fixp->fx_subsy->sy_used_in_reloc = 1;
      return 0;
    }

  /* We can't adjust any relocs that use LR% and RR% field selectors.
     That confuses the HP linker.  */
  if (hppa_fix->fx_r_field == e_lrsel
      || hppa_fix->fx_r_field == e_rrsel
      || hppa_fix->fx_r_field == e_nlrsel)
    return 0;
#endif

  /* Reject reductions of symbols in DLT relative relocs,
     relocations with plabels.  */
  if (hppa_fix->fx_r_field == e_tsel
      || hppa_fix->fx_r_field == e_ltsel
      || hppa_fix->fx_r_field == e_rtsel
      || hppa_fix->fx_r_field == e_psel
      || hppa_fix->fx_r_field == e_rpsel
      || hppa_fix->fx_r_field == e_lpsel)
    return 0;

  if (fixp->fx_addsy && fixp->fx_addsy->bsym->flags & BSF_GLOBAL)
    return 0;

  /* Reject reductions of function symbols.  */
  if (fixp->fx_addsy == 0
      || (fixp->fx_addsy->bsym->flags & BSF_FUNCTION) == 0)
    return 1;

  return 0;
}

/* Return nonzero if the fixup in FIXP will require a relocation,
   even it if appears that the fixup could be completely handled
   within GAS.  */

int
hppa_force_relocation (fixp)
     fixS *fixp;
{
  struct hppa_fix_struct *hppa_fixp;
  int distance;

  hppa_fixp = (struct hppa_fix_struct *) fixp->tc_fix_data;
#ifdef OBJ_SOM
  if (fixp->fx_r_type == R_HPPA_ENTRY || fixp->fx_r_type == R_HPPA_EXIT
      || fixp->fx_r_type == R_HPPA_BEGIN_BRTAB
      || fixp->fx_r_type == R_HPPA_END_BRTAB
      || fixp->fx_r_type == R_HPPA_BEGIN_TRY
      || fixp->fx_r_type == R_HPPA_END_TRY
      || (fixp->fx_addsy != NULL && fixp->fx_subsy != NULL
	  && (hppa_fixp->segment->flags & SEC_CODE) != 0))
    return 1;
#endif

#define arg_reloc_stub_needed(CALLER, CALLEE) \
  ((CALLEE) && (CALLER) && ((CALLEE) != (CALLER)))

  /* It is necessary to force PC-relative calls/jumps to have a relocation
     entry if they're going to need either a argument relocation or long
     call stub.  FIXME.  Can't we need the same for absolute calls?  */
  if (fixp->fx_pcrel && fixp->fx_addsy
      && (arg_reloc_stub_needed (((obj_symbol_type *)
				  fixp->fx_addsy->bsym)->tc_data.hppa_arg_reloc,

				 hppa_fixp->fx_arg_reloc)))
    return 1;
  distance = (fixp->fx_offset + S_GET_VALUE (fixp->fx_addsy)
	      - md_pcrel_from (fixp));
  /* Now check and see if we're going to need a long-branch stub.  */
  if (fixp->fx_r_type == R_HPPA_PCREL_CALL
      && (distance > 262143 || distance < -262144))
    return 1;

#undef arg_reloc_stub_needed

  /* No need (yet) to force another relocations to be emitted.  */
  return 0;
}

/* Now for some ELF specific code.  FIXME.  */
#ifdef OBJ_ELF
/* Mark the end of a function so that it's possible to compute
   the size of the function in hppa_elf_final_processing.  */

static void
hppa_elf_mark_end_of_function ()
{
  /* ELF does not have EXIT relocations.  All we do is create a
     temporary symbol marking the end of the function.  */
  char *name = (char *)
    xmalloc (strlen ("L$\001end_") +
	     strlen (S_GET_NAME (last_call_info->start_symbol)) + 1);

  if (name)
    {
      symbolS *symbolP;

      strcpy (name, "L$\001end_");
      strcat (name, S_GET_NAME (last_call_info->start_symbol));

      /* If we have a .exit followed by a .procend, then the
	 symbol will have already been defined.  */
      symbolP = symbol_find (name);
      if (symbolP)
	{
	  /* The symbol has already been defined!  This can
	     happen if we have a .exit followed by a .procend.

	     This is *not* an error.  All we want to do is free
	     the memory we just allocated for the name and continue.  */
	  xfree (name);
	}
      else
	{
	  /* symbol value should be the offset of the
	     last instruction of the function */
	  symbolP = symbol_new (name, now_seg, (valueT) (frag_now_fix () - 4),
				frag_now);

	  assert (symbolP);
	  symbolP->bsym->flags = BSF_LOCAL;
	  symbol_table_insert (symbolP);
	}

      if (symbolP)
	last_call_info->end_symbol = symbolP;
      else
	as_bad ("Symbol '%s' could not be created.", name);

    }
  else
    as_bad ("No memory for symbol name.");

}

/* For ELF, this function serves one purpose:  to setup the st_size
   field of STT_FUNC symbols.  To do this, we need to scan the
   call_info structure list, determining st_size in by taking the
   difference in the address of the beginning/end marker symbols.  */

void
elf_hppa_final_processing ()
{
  struct call_info *call_info_pointer;

  for (call_info_pointer = call_info_root;
       call_info_pointer;
       call_info_pointer = call_info_pointer->ci_next)
    {
      elf_symbol_type *esym
      = (elf_symbol_type *) call_info_pointer->start_symbol->bsym;
      esym->internal_elf_sym.st_size =
	S_GET_VALUE (call_info_pointer->end_symbol)
	- S_GET_VALUE (call_info_pointer->start_symbol) + 4;
    }
}
#endif
