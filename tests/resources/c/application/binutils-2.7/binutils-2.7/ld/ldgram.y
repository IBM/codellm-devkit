/* A YACC grammer to parse a superset of the AT&T linker scripting languaue.
   Copyright (C) 1991, 92, 93, 94, 95, 1996 Free Software Foundation, Inc.
   Written by Steve Chamberlain of Cygnus Support (steve@cygnus.com).

This file is part of GNU ld.

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

%{
/*

 */

#define DONTDECLARE_MALLOC

#include "bfd.h"
#include "sysdep.h"
#include "bfdlink.h"
#include "ld.h"    
#include "ldexp.h"
#include "ldver.h"
#include "ldlang.h"
#include "ldemul.h"
#include "ldfile.h"
#include "ldmisc.h"
#include "ldmain.h"
#include "mri.h"
#include "ldlex.h"

#ifndef YYDEBUG
#define YYDEBUG 1
#endif

static enum section_type sectype;

lang_memory_region_type *region;


char *current_file;
boolean ldgram_want_filename = true;
boolean had_script = false;
boolean force_make_executable = false;

boolean ldgram_in_script = false;
boolean ldgram_had_equals = false;


#define ERROR_NAME_MAX 20
static char *error_names[ERROR_NAME_MAX];
static int error_index;
#define PUSH_ERROR(x) if (error_index < ERROR_NAME_MAX) error_names[error_index] = x; error_index++;
#define POP_ERROR()   error_index--;
%}
%union {
  bfd_vma integer;
  char *name;
  int token;
  union etree_union *etree;
  struct phdr_info
    {
      boolean filehdr;
      boolean phdrs;
      union etree_union *at;
      union etree_union *flags;
    } phdr;
}

%type <etree> exp opt_exp_with_type mustbe_exp opt_at phdr_type phdr_val
%type <integer> fill_opt
%type <name> memspec_opt casesymlist
%token <integer> INT  
%token <name> NAME LNAME
%type  <integer> length
%type <phdr> phdr_qualifiers

%right <token> PLUSEQ MINUSEQ MULTEQ DIVEQ  '=' LSHIFTEQ RSHIFTEQ   ANDEQ OREQ 
%right <token> '?' ':'
%left <token> OROR
%left <token>  ANDAND
%left <token> '|'
%left <token>  '^'
%left  <token> '&'
%left <token>  EQ NE
%left  <token> '<' '>' LE GE
%left  <token> LSHIFT RSHIFT

%left  <token> '+' '-'
%left  <token> '*' '/' '%'

%right UNARY
%token END 
%left <token> '('
%token <token> ALIGN_K BLOCK BIND QUAD LONG SHORT BYTE
%token SECTIONS PHDRS
%token '{' '}'
%token SIZEOF_HEADERS OUTPUT_FORMAT FORCE_COMMON_ALLOCATION OUTPUT_ARCH
%token SIZEOF_HEADERS
%token INCLUDE
%token MEMORY DEFSYMEND
%token NOLOAD DSECT COPY INFO OVERLAY
%token NAME LNAME DEFINED TARGET_K SEARCH_DIR MAP ENTRY
%token <integer> SIZEOF NEXT ADDR
%token STARTUP HLL SYSLIB FLOAT NOFLOAT
%token ORIGIN FILL
%token LENGTH CREATE_OBJECT_SYMBOLS INPUT GROUP OUTPUT CONSTRUCTORS
%token ALIGNMOD AT PROVIDE
%type <token> assign_op atype
%type <name>  filename
%token CHIP LIST SECT ABSOLUTE  LOAD NEWLINE ENDWORD ORDER NAMEWORD
%token FORMAT PUBLIC DEFSYMEND BASE ALIAS TRUNCATE REL
%token INPUT_SCRIPT INPUT_MRI_SCRIPT INPUT_DEFSYM CASE EXTERN START

%%

file:	
		INPUT_SCRIPT script_file
	|	INPUT_MRI_SCRIPT mri_script_file
	|	INPUT_DEFSYM defsym_expr
	;


filename:  NAME;


defsym_expr:
		{ ldlex_defsym(); }
		NAME '=' exp
		{
		  ldlex_popstate();
		  lang_add_assignment(exp_assop($3,$2,$4));
		}

/* SYNTAX WITHIN AN MRI SCRIPT FILE */  
mri_script_file:
		{
		  ldlex_mri_script ();
		  PUSH_ERROR ("MRI style script");
		}
	     mri_script_lines
		{
		  ldlex_popstate ();
		  mri_draw_tree ();
		  POP_ERROR ();
		}
	;

mri_script_lines:
		mri_script_lines mri_script_command NEWLINE
          |
	;

mri_script_command:
		CHIP  exp 
	|	CHIP  exp ',' exp
	|	NAME 	{
			einfo("%P%F: unrecognised keyword in MRI style script '%s'\n",$1);
			}
	|	LIST  	{
			config.map_filename = "-";
			}
        |       ORDER ordernamelist
	|       ENDWORD 
        |       PUBLIC NAME '=' exp
 			{ mri_public($2, $4); }
        |       PUBLIC NAME ',' exp
 			{ mri_public($2, $4); }
        |       PUBLIC NAME  exp 
 			{ mri_public($2, $3); }
	| 	FORMAT NAME
			{ mri_format($2); }
	|	SECT NAME ',' exp
			{ mri_output_section($2, $4);}
	|	SECT NAME  exp
			{ mri_output_section($2, $3);}
	|	SECT NAME '=' exp
			{ mri_output_section($2, $4);}
	|	ALIGN_K NAME '=' exp
			{ mri_align($2,$4); }
	|	ALIGN_K NAME ',' exp
			{ mri_align($2,$4); }
	|	ALIGNMOD NAME '=' exp
			{ mri_alignmod($2,$4); }
	|	ALIGNMOD NAME ',' exp
			{ mri_alignmod($2,$4); }
	|	ABSOLUTE mri_abs_name_list
	|	LOAD	 mri_load_name_list
	|       NAMEWORD NAME 
			{ mri_name($2); }   
	|	ALIAS NAME ',' NAME
			{ mri_alias($2,$4,0);}
	|	ALIAS NAME ',' INT
			{ mri_alias($2,0,(int) $4);}
	|	BASE     exp
			{ mri_base($2); }
        |       TRUNCATE INT
		{  mri_truncate((unsigned int) $2); }
	|	CASE casesymlist
	|	EXTERN extern_name_list
	|	INCLUDE filename
		{ ldfile_open_command_file ($2); } mri_script_lines END
	|	START NAME
		{ lang_add_entry ($2, false); }
        |
	;

ordernamelist:
	      ordernamelist ',' NAME         { mri_order($3); }
	|     ordernamelist  NAME         { mri_order($2); }
      	|
	;

mri_load_name_list:
		NAME
			{ mri_load($1); }
	|	mri_load_name_list ',' NAME { mri_load($3); }
	;

mri_abs_name_list:
 		NAME
 			{ mri_only_load($1); }
	|	mri_abs_name_list ','  NAME
 			{ mri_only_load($3); }
	;

casesymlist:
	  /* empty */ { $$ = NULL; }
	| NAME
	| casesymlist ',' NAME
	;

extern_name_list:
	  NAME
			{ ldlang_add_undef ($1); }
	| extern_name_list ',' NAME
			{ ldlang_add_undef ($3); }
	;

script_file:
	{
	 ldlex_both();
	}
       ifile_list
	{
	ldlex_popstate();
	}
        ;


ifile_list:
       ifile_list ifile_p1
        |
	;



ifile_p1:
		memory
	|	sections
	|	phdrs
	|	startup
	|	high_level_library
	|	low_level_library
	|	floating_point_support
	|	statement_anywhere
        |	 ';'
	|	TARGET_K '(' NAME ')'
		{ lang_add_target($3); }
	|	SEARCH_DIR '(' filename ')'
		{ ldfile_add_library_path ($3, false); }
	|	OUTPUT '(' filename ')'
		{ lang_add_output($3, 1); }
        |	OUTPUT_FORMAT '(' NAME ')'
		  { lang_add_output_format ($3, (char *) NULL,
					    (char *) NULL, 1); }
	|	OUTPUT_FORMAT '(' NAME ',' NAME ',' NAME ')'
		  { lang_add_output_format ($3, $5, $7, 1); }
        |	OUTPUT_ARCH '(' NAME ')'
		  { ldfile_set_output_arch($3); }
	|	FORCE_COMMON_ALLOCATION
		{ command_line.force_common_definition = true ; }
	|	INPUT '(' input_list ')'
	|	GROUP
		  { lang_enter_group (); }
		    '(' input_list ')'
		  { lang_leave_group (); }
     	|	MAP '(' filename ')'
		{ lang_add_map($3); }
	|	INCLUDE filename 
		{ ldfile_open_command_file($2); } ifile_list END
	;

input_list:
		NAME
		{ lang_add_input_file($1,lang_input_file_is_search_file_enum,
				 (char *)NULL); }
	|	input_list ',' NAME
		{ lang_add_input_file($3,lang_input_file_is_search_file_enum,
				 (char *)NULL); }
	|	input_list NAME
		{ lang_add_input_file($2,lang_input_file_is_search_file_enum,
				 (char *)NULL); }
	|	LNAME
		{ lang_add_input_file($1,lang_input_file_is_l_enum,
				 (char *)NULL); }
	|	input_list ',' LNAME
		{ lang_add_input_file($3,lang_input_file_is_l_enum,
				 (char *)NULL); }
	|	input_list LNAME
		{ lang_add_input_file($2,lang_input_file_is_l_enum,
				 (char *)NULL); }
	;

sections:
		SECTIONS '{' sec_or_group_p1 '}'
	;

sec_or_group_p1:
		sec_or_group_p1 section
	|	sec_or_group_p1 statement_anywhere
	|
	;

statement_anywhere:
		ENTRY '(' NAME ')'
		{ lang_add_entry ($3, false); }
	|	assignment end
	;

file_NAME_list:
		NAME
			{ lang_add_wild($1, current_file); }
	|	file_NAME_list opt_comma NAME
			{ lang_add_wild($3, current_file); }
	;

input_section_spec:
		NAME
		{
		lang_add_wild((char *)NULL, $1);
		}
        |	'['
			{
			current_file = (char *)NULL;
			}
			file_NAME_list
		']'
	|	NAME
			{
			current_file =$1;
			}
		'(' file_NAME_list ')'
	|	'*'
			{
			current_file = (char *)NULL;
			}
		'(' file_NAME_list ')'
	;

statement:
	  	assignment end
	|	CREATE_OBJECT_SYMBOLS
		{
 		lang_add_attribute(lang_object_symbols_statement_enum); 
	      	}
        |	';'
        |	CONSTRUCTORS
		{
 		
		  lang_add_attribute(lang_constructors_statement_enum); 
		}
	| input_section_spec
        | length '(' exp ')'
        	        {
			lang_add_data((int) $1,$3);
			}
  
	| FILL '(' exp ')'
			{
			  lang_add_fill
			    (exp_get_value_int($3,
					       0,
					       "fill value",
					       lang_first_phase_enum));
			}
	;

statement_list:
		statement_list statement
  	|  	statement
	;
  
statement_list_opt:
		/* empty */
	|	statement_list
	;

length:
		QUAD
			{ $$ = $1; }
	|	LONG
			{ $$ = $1; }
	| 	SHORT
			{ $$ = $1; }
	|	BYTE
			{ $$ = $1; }
	;

fill_opt:
          '=' mustbe_exp
		{
		  $$ =	 exp_get_value_int($2,
					   0,
					   "fill value",
					   lang_first_phase_enum);
		}
	| 	{ $$ = 0; }
	;

		

assign_op:
		PLUSEQ
			{ $$ = '+'; }
	|	MINUSEQ
			{ $$ = '-'; }
	| 	MULTEQ
			{ $$ = '*'; }
	| 	DIVEQ
			{ $$ = '/'; }
	| 	LSHIFTEQ
			{ $$ = LSHIFT; }
	| 	RSHIFTEQ
			{ $$ = RSHIFT; }
	| 	ANDEQ
			{ $$ = '&'; }
	| 	OREQ
			{ $$ = '|'; }

	;

end:	';' | ','
	;


assignment:
		NAME '=' mustbe_exp
		{
		  lang_add_assignment (exp_assop ($2, $1, $3));
		}
	|	NAME assign_op mustbe_exp
		{
		  lang_add_assignment (exp_assop ('=', $1,
						  exp_binop ($2,
							     exp_nameop (NAME,
									 $1),
							     $3)));
		}
	|	PROVIDE '(' NAME '=' mustbe_exp ')'
		{
		  lang_add_assignment (exp_provide ($3, $5));
		}
	;


opt_comma:
		','	|	;


memory:
		MEMORY '{' memory_spec memory_spec_list '}'
	;

memory_spec_list:
		memory_spec_list memory_spec
	|	memory_spec_list ',' memory_spec
	|
	;


memory_spec: 		NAME
			{ region = lang_memory_region_lookup($1); }
		attributes_opt ':'
		origin_spec opt_comma length_spec

	; origin_spec:
	ORIGIN '=' mustbe_exp
		{ region->current =
		 region->origin =
		 exp_get_vma($3, 0L,"origin", lang_first_phase_enum);
}
	; length_spec:
             LENGTH '=' mustbe_exp
               { region->length = exp_get_vma($3,
					       ~((bfd_vma)0),
					       "length",
					       lang_first_phase_enum);
		}
	

attributes_opt:
		  '(' NAME ')'
			{
			lang_set_flags(&region->flags, $2);
			}
	|
  
	;

startup:
	STARTUP '(' filename ')'
		{ lang_startup($3); }
	;

high_level_library:
		HLL '(' high_level_library_NAME_list ')'
	|	HLL '(' ')'
			{ ldemul_hll((char *)NULL); }
	;

high_level_library_NAME_list:
		high_level_library_NAME_list opt_comma filename
			{ ldemul_hll($3); }
	|	filename
			{ ldemul_hll($1); }

	;

low_level_library:
	SYSLIB '(' low_level_library_NAME_list ')'
	; low_level_library_NAME_list:
		low_level_library_NAME_list opt_comma filename
			{ ldemul_syslib($3); }
	|
	;

floating_point_support:
		FLOAT
			{ lang_float(true); }
	|	NOFLOAT
			{ lang_float(false); }
	;
		

mustbe_exp:		 { ldlex_expression(); }
		exp
			 { ldlex_popstate(); $$=$2;}
	;

exp	:
		'-' exp %prec UNARY
			{ $$ = exp_unop('-', $2); }
	|	'(' exp ')'
			{ $$ = $2; }
	|	NEXT '(' exp ')' %prec UNARY
			{ $$ = exp_unop((int) $1,$3); }
	|	'!' exp %prec UNARY
			{ $$ = exp_unop('!', $2); }
	|	'+' exp %prec UNARY
			{ $$ = $2; }
	|	'~' exp %prec UNARY
			{ $$ = exp_unop('~', $2);}

	|	exp '*' exp
			{ $$ = exp_binop('*', $1, $3); }
	|	exp '/' exp
			{ $$ = exp_binop('/', $1, $3); }
	|	exp '%' exp
			{ $$ = exp_binop('%', $1, $3); }
	|	exp '+' exp
			{ $$ = exp_binop('+', $1, $3); }
	|	exp '-' exp
			{ $$ = exp_binop('-' , $1, $3); }
	|	exp LSHIFT exp
			{ $$ = exp_binop(LSHIFT , $1, $3); }
	|	exp RSHIFT exp
			{ $$ = exp_binop(RSHIFT , $1, $3); }
	|	exp EQ exp
			{ $$ = exp_binop(EQ , $1, $3); }
	|	exp NE exp
			{ $$ = exp_binop(NE , $1, $3); }
	|	exp LE exp
			{ $$ = exp_binop(LE , $1, $3); }
  	|	exp GE exp
			{ $$ = exp_binop(GE , $1, $3); }
	|	exp '<' exp
			{ $$ = exp_binop('<' , $1, $3); }
	|	exp '>' exp
			{ $$ = exp_binop('>' , $1, $3); }
	|	exp '&' exp
			{ $$ = exp_binop('&' , $1, $3); }
	|	exp '^' exp
			{ $$ = exp_binop('^' , $1, $3); }
	|	exp '|' exp
			{ $$ = exp_binop('|' , $1, $3); }
	|	exp '?' exp ':' exp
			{ $$ = exp_trinop('?' , $1, $3, $5); }
	|	exp ANDAND exp
			{ $$ = exp_binop(ANDAND , $1, $3); }
	|	exp OROR exp
			{ $$ = exp_binop(OROR , $1, $3); }
	|	DEFINED '(' NAME ')'
			{ $$ = exp_nameop(DEFINED, $3); }
	|	INT
			{ $$ = exp_intop($1); }
        |	SIZEOF_HEADERS
			{ $$ = exp_nameop(SIZEOF_HEADERS,0); }

	|	SIZEOF '(' NAME ')'
			{ $$ = exp_nameop(SIZEOF,$3); }
	|	ADDR '(' NAME ')'
			{ $$ = exp_nameop(ADDR,$3); }
	|	ABSOLUTE '(' exp ')'
			{ $$ = exp_unop(ABSOLUTE, $3); }
	|	ALIGN_K '(' exp ')'
			{ $$ = exp_unop(ALIGN_K,$3); }
	|	BLOCK '(' exp ')'
			{ $$ = exp_unop(ALIGN_K,$3); }
	|	NAME
			{ $$ = exp_nameop(NAME,$1); }
	;


opt_at:
		AT '(' exp ')' { $$ = $3; }
	|	{ $$ = 0; }
	;

section:	NAME 		{ ldlex_expression(); }
		opt_exp_with_type 
		opt_at   	{ ldlex_popstate(); }
		'{'
			{
			  lang_enter_output_section_statement($1, $3,
							      sectype,
							      0, 0, 0, $4);
			}
		statement_list_opt 	
 		'}' {ldlex_expression();} memspec_opt phdr_opt fill_opt
		{
		  ldlex_popstate();
		  lang_leave_output_section_statement($13, $11);
		}
		opt_comma
	|	/* The GROUP case is just enough to support the gcc
		   svr3.ifile script.  It is not intended to be full
		   support.  I'm not even sure what GROUP is supposed
		   to mean.  */
		GROUP { ldlex_expression (); }
		opt_exp_with_type
		{
		  ldlex_popstate ();
		  lang_add_assignment (exp_assop ('=', ".", $3));
		}
		'{' sec_or_group_p1 '}'
	;

type:
	   NOLOAD  { sectype = noload_section; }
	|  DSECT   { sectype = dsect_section; }
	|  COPY    { sectype = copy_section; }
	|  INFO    { sectype = info_section; }
	|  OVERLAY { sectype = overlay_section; }
	;

atype:
	 	'(' type ')'
  	| 	/* EMPTY */ { sectype = normal_section; }
	;

opt_exp_with_type:
		exp atype ':'		{ $$ = $1; }
	|	atype ':'		{ $$ = (etree_type *)NULL;  }
	|	/* The BIND cases are to support the gcc svr3.ifile
		   script.  They aren't intended to implement full
		   support for the BIND keyword.  I'm not even sure
		   what BIND is supposed to mean.  */
		BIND '(' exp ')' atype ':' { $$ = $3; }
	|	BIND '(' exp ')' BLOCK '(' exp ')' atype ':'
		{ $$ = $3; }
	;

memspec_opt:
		'>' NAME
		{ $$ = $2; }
	|	{ $$ = "*default*"; }
	;

phdr_opt:
		/* empty */
	|	phdr_opt ':' NAME
		{
		  lang_section_in_phdr ($3);
		}
	;

phdrs:
		PHDRS '{' phdr_list '}'
	;

phdr_list:
		/* empty */
	|	phdr_list phdr
	;

phdr:
		NAME { ldlex_expression (); }
		  phdr_type phdr_qualifiers { ldlex_popstate (); }
		  ';'
		{
		  lang_new_phdr ($1, $3, $4.filehdr, $4.phdrs, $4.at,
				 $4.flags);
		}
	;

phdr_type:
		exp
		{
		  $$ = $1;

		  if ($1->type.node_class == etree_name
		      && $1->type.node_code == NAME)
		    {
		      const char *s;
		      unsigned int i;
		      static const char * const phdr_types[] =
			{
			  "PT_NULL", "PT_LOAD", "PT_DYNAMIC",
			  "PT_INTERP", "PT_NOTE", "PT_SHLIB",
			  "PT_PHDR"
			};

		      s = $1->name.name;
		      for (i = 0;
			   i < sizeof phdr_types / sizeof phdr_types[0];
			   i++)
			if (strcmp (s, phdr_types[i]) == 0)
			  {
			    $$ = exp_intop (i);
			    break;
			  }
		    }
		}
	;

phdr_qualifiers:
		/* empty */
		{
		  memset (&$$, 0, sizeof (struct phdr_info));
		}
	|	NAME phdr_val phdr_qualifiers
		{
		  $$ = $3;
		  if (strcmp ($1, "FILEHDR") == 0 && $2 == NULL)
		    $$.filehdr = true;
		  else if (strcmp ($1, "PHDRS") == 0 && $2 == NULL)
		    $$.phdrs = true;
		  else if (strcmp ($1, "FLAGS") == 0 && $2 != NULL)
		    $$.flags = $2;
		  else
		    einfo ("%X%P:%S: PHDRS syntax error at `%s'\n", $1);
		}
	|	AT '(' exp ')' phdr_qualifiers
		{
		  $$ = $5;
		  $$.at = $3;
		}
	;

phdr_val:
		/* empty */
		{
		  $$ = NULL;
		}
	| '(' exp ')'
		{
		  $$ = $2;
		}
	;

%%
void
yyerror(arg) 
     const char *arg;
{ 
  if (ldfile_assumed_script)
    einfo ("%P:%s: file format not recognized; treating as linker script\n",
	   ldfile_input_filename);
  if (error_index > 0 && error_index < ERROR_NAME_MAX)
     einfo ("%P%F:%S: %s in %s\n", arg, error_names[error_index-1]);
  else
     einfo ("%P%F:%S: %s\n", arg);
}
