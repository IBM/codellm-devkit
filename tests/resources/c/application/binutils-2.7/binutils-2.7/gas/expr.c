/* expr.c -operands, expressions-
   Copyright (C) 1987, 90, 91, 92, 93, 94, 95, 1996
   Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

/*
 * This is really a branch office of as-read.c. I split it out to clearly
 * distinguish the world of expressions from the world of statements.
 * (It also gives smaller files to re-compile.)
 * Here, "operand"s are of expressions, not instructions.
 */

#include <ctype.h>
#include <string.h>

#include "as.h"
#include "libiberty.h"
#include "obstack.h"

static void floating_constant PARAMS ((expressionS * expressionP));
static void integer_constant PARAMS ((int radix, expressionS * expressionP));
static void mri_char_constant PARAMS ((expressionS *));
static void current_location PARAMS ((expressionS *));
static void clean_up_expression PARAMS ((expressionS * expressionP));

extern const char EXP_CHARS[], FLT_CHARS[];

/* We keep a mapping of expression symbols to file positions, so that
   we can provide better error messages.  */

struct expr_symbol_line
{
  struct expr_symbol_line *next;
  symbolS *sym;
  char *file;
  unsigned int line;
};

static struct expr_symbol_line *expr_symbol_lines;

/* Build a dummy symbol to hold a complex expression.  This is how we
   build expressions up out of other expressions.  The symbol is put
   into the fake section expr_section.  */

symbolS *
make_expr_symbol (expressionP)
     expressionS *expressionP;
{
  const char *fake;
  symbolS *symbolP;
  struct expr_symbol_line *n;

  if (expressionP->X_op == O_symbol
      && expressionP->X_add_number == 0)
    return expressionP->X_add_symbol;

  /* FIXME: This should be something which decode_local_label_name
     will handle.  */
  fake = FAKE_LABEL_NAME;

  /* Putting constant symbols in absolute_section rather than
     expr_section is convenient for the old a.out code, for which
     S_GET_SEGMENT does not always retrieve the value put in by
     S_SET_SEGMENT.  */
  symbolP = symbol_create (fake,
			   (expressionP->X_op == O_constant
			    ? absolute_section
			    : expr_section),
			   0, &zero_address_frag);
  symbolP->sy_value = *expressionP;

  if (expressionP->X_op == O_constant)
    resolve_symbol_value (symbolP);

  n = (struct expr_symbol_line *) xmalloc (sizeof *n);
  n->sym = symbolP;
  as_where (&n->file, &n->line);
  n->next = expr_symbol_lines;
  expr_symbol_lines = n;

  return symbolP;
}

/* Return the file and line number for an expr symbol.  Return
   non-zero if something was found, 0 if no information is known for
   the symbol.  */

int
expr_symbol_where (sym, pfile, pline)
     symbolS *sym;
     char **pfile;
     unsigned int *pline;
{
  register struct expr_symbol_line *l;

  for (l = expr_symbol_lines; l != NULL; l = l->next)
    {
      if (l->sym == sym)
	{
	  *pfile = l->file;
	  *pline = l->line;
	  return 1;
	}
    }

  return 0;
}

/*
 * Build any floating-point literal here.
 * Also build any bignum literal here.
 */

/* Seems atof_machine can backscan through generic_bignum and hit whatever
   happens to be loaded before it in memory.  And its way too complicated
   for me to fix right.  Thus a hack.  JF:  Just make generic_bignum bigger,
   and never write into the early words, thus they'll always be zero.
   I hate Dean's floating-point code.  Bleh.  */
LITTLENUM_TYPE generic_bignum[SIZE_OF_LARGE_NUMBER + 6];
FLONUM_TYPE generic_floating_point_number =
{
  &generic_bignum[6],		/* low (JF: Was 0) */
  &generic_bignum[SIZE_OF_LARGE_NUMBER + 6 - 1], /* high JF: (added +6) */
  0,				/* leader */
  0,				/* exponent */
  0				/* sign */
};
/* If nonzero, we've been asked to assemble nan, +inf or -inf */
int generic_floating_point_magic;

static void
floating_constant (expressionP)
     expressionS *expressionP;
{
  /* input_line_pointer->*/
  /* floating-point constant. */
  int error_code;

  error_code = atof_generic (&input_line_pointer, ".", EXP_CHARS,
			     &generic_floating_point_number);

  if (error_code)
    {
      if (error_code == ERROR_EXPONENT_OVERFLOW)
	{
	  as_bad ("bad floating-point constant: exponent overflow, probably assembling junk");
	}
      else
	{
	  as_bad ("bad floating-point constant: unknown error code=%d.", error_code);
	}
    }
  expressionP->X_op = O_big;
  /* input_line_pointer->just after constant, */
  /* which may point to whitespace. */
  expressionP->X_add_number = -1;
}

static void
integer_constant (radix, expressionP)
     int radix;
     expressionS *expressionP;
{
  char *start;		/* start of number. */
  char *suffix = NULL;
  char c;
  valueT number;	/* offset or (absolute) value */
  short int digit;	/* value of next digit in current radix */
  short int maxdig = 0;/* highest permitted digit value. */
  int too_many_digits = 0;	/* if we see >= this number of */
  char *name;		/* points to name of symbol */
  symbolS *symbolP;	/* points to symbol */

  int small;			/* true if fits in 32 bits. */

  /* May be bignum, or may fit in 32 bits. */
  /* Most numbers fit into 32 bits, and we want this case to be fast.
     so we pretend it will fit into 32 bits.  If, after making up a 32
     bit number, we realise that we have scanned more digits than
     comfortably fit into 32 bits, we re-scan the digits coding them
     into a bignum.  For decimal and octal numbers we are
     conservative: Some numbers may be assumed bignums when in fact
     they do fit into 32 bits.  Numbers of any radix can have excess
     leading zeros: We strive to recognise this and cast them back
     into 32 bits.  We must check that the bignum really is more than
     32 bits, and change it back to a 32-bit number if it fits.  The
     number we are looking for is expected to be positive, but if it
     fits into 32 bits as an unsigned number, we let it be a 32-bit
     number.  The cavalier approach is for speed in ordinary cases. */
  /* This has been extended for 64 bits.  We blindly assume that if
     you're compiling in 64-bit mode, the target is a 64-bit machine.
     This should be cleaned up.  */

#ifdef BFD64
#define valuesize 64
#else /* includes non-bfd case, mostly */
#define valuesize 32
#endif

  if (flag_m68k_mri && radix == 0)
    {
      int flt = 0;

      /* In MRI mode, the number may have a suffix indicating the
         radix.  For that matter, it might actually be a floating
         point constant.  */
      for (suffix = input_line_pointer; isalnum (*suffix); suffix++)
	{
	  if (*suffix == 'e' || *suffix == 'E')
	    flt = 1;
	}

      if (suffix == input_line_pointer)
	{
	  radix = 10;
	  suffix = NULL;
	}
      else
	{
	  c = *--suffix;
	  if (islower (c))
	    c = toupper (c);
	  if (c == 'B')
	    radix = 2;
	  else if (c == 'D')
	    radix = 10;
	  else if (c == 'O' || c == 'Q')
	    radix = 8;
	  else if (c == 'H')
	    radix = 16;
	  else if (suffix[1] == '.' || c == 'E' || flt)
	    {
	      floating_constant (expressionP);
	      return;
	    }
	  else
	    {
	      radix = 10;
	      suffix = NULL;
	    }
	}
    }

  switch (radix)
    {
    case 2:
      maxdig = 2;
      too_many_digits = valuesize + 1;
      break;
    case 8:
      maxdig = radix = 8;
      too_many_digits = (valuesize + 2) / 3 + 1;
      break;
    case 16:
      maxdig = radix = 16;
      too_many_digits = (valuesize + 3) / 4 + 1;
      break;
    case 10:
      maxdig = radix = 10;
      too_many_digits = (valuesize + 12) / 4; /* very rough */
    }
#undef valuesize
  start = input_line_pointer;
  c = *input_line_pointer++;
  for (number = 0;
       (digit = hex_value (c)) < maxdig;
       c = *input_line_pointer++)
    {
      number = number * radix + digit;
    }
  /* c contains character after number. */
  /* input_line_pointer->char after c. */
  small = (input_line_pointer - start - 1) < too_many_digits;
  if (!small)
    {
      /*
       * we saw a lot of digits. manufacture a bignum the hard way.
       */
      LITTLENUM_TYPE *leader;	/*->high order littlenum of the bignum. */
      LITTLENUM_TYPE *pointer;	/*->littlenum we are frobbing now. */
      long carry;

      leader = generic_bignum;
      generic_bignum[0] = 0;
      generic_bignum[1] = 0;
      input_line_pointer = start;	/*->1st digit. */
      c = *input_line_pointer++;
      for (;
	   (carry = hex_value (c)) < maxdig;
	   c = *input_line_pointer++)
	{
	  for (pointer = generic_bignum;
	       pointer <= leader;
	       pointer++)
	    {
	      long work;

	      work = carry + radix * *pointer;
	      *pointer = work & LITTLENUM_MASK;
	      carry = work >> LITTLENUM_NUMBER_OF_BITS;
	    }
	  if (carry)
	    {
	      if (leader < generic_bignum + SIZE_OF_LARGE_NUMBER - 1)
		{
		  /* room to grow a longer bignum. */
		  *++leader = carry;
		}
	    }
	}
      /* again, c is char after number, */
      /* input_line_pointer->after c. */
      know (LITTLENUM_NUMBER_OF_BITS == 16);
      if (leader < generic_bignum + 2)
	{
	  /* will fit into 32 bits. */
	  number =
	    ((generic_bignum[1] & LITTLENUM_MASK) << LITTLENUM_NUMBER_OF_BITS)
	    | (generic_bignum[0] & LITTLENUM_MASK);
	  small = 1;
	}
      else
	{
	  number = leader - generic_bignum + 1;	/* number of littlenums in the bignum. */
	}
    }

  if (flag_m68k_mri && suffix != NULL && input_line_pointer - 1 == suffix)
    c = *input_line_pointer++;

  if (small)
    {
      /*
       * here with number, in correct radix. c is the next char.
       * note that unlike un*x, we allow "011f" "0x9f" to
       * both mean the same as the (conventional) "9f". this is simply easier
       * than checking for strict canonical form. syntax sux!
       */

      if (LOCAL_LABELS_FB && c == 'b')
	{
	  /*
	   * backward ref to local label.
	   * because it is backward, expect it to be defined.
	   */
	  /* Construct a local label.  */
	  name = fb_label_name ((int) number, 0);

	  /* seen before, or symbol is defined: ok */
	  symbolP = symbol_find (name);
	  if ((symbolP != NULL) && (S_IS_DEFINED (symbolP)))
	    {
	      /* local labels are never absolute. don't waste time
		 checking absoluteness. */
	      know (SEG_NORMAL (S_GET_SEGMENT (symbolP)));

	      expressionP->X_op = O_symbol;
	      expressionP->X_add_symbol = symbolP;
	    }
	  else
	    {
	      /* either not seen or not defined. */
	      /* @@ Should print out the original string instead of
		 the parsed number.  */
	      as_bad ("backw. ref to unknown label \"%d:\", 0 assumed.",
		      (int) number);
	      expressionP->X_op = O_constant;
	    }

	  expressionP->X_add_number = 0;
	}			/* case 'b' */
      else if (LOCAL_LABELS_FB && c == 'f')
	{
	  /*
	   * forward reference. expect symbol to be undefined or
	   * unknown. undefined: seen it before. unknown: never seen
	   * it before.
	   * construct a local label name, then an undefined symbol.
	   * don't create a xseg frag for it: caller may do that.
	   * just return it as never seen before.
	   */
	  name = fb_label_name ((int) number, 1);
	  symbolP = symbol_find_or_make (name);
	  /* we have no need to check symbol properties. */
#ifndef many_segments
	  /* since "know" puts its arg into a "string", we
	     can't have newlines in the argument.  */
	  know (S_GET_SEGMENT (symbolP) == undefined_section || S_GET_SEGMENT (symbolP) == text_section || S_GET_SEGMENT (symbolP) == data_section);
#endif
	  expressionP->X_op = O_symbol;
	  expressionP->X_add_symbol = symbolP;
	  expressionP->X_add_number = 0;
	}			/* case 'f' */
      else if (LOCAL_LABELS_DOLLAR && c == '$')
	{
	  /* If the dollar label is *currently* defined, then this is just
	     another reference to it.  If it is not *currently* defined,
	     then this is a fresh instantiation of that number, so create
	     it.  */

	  if (dollar_label_defined ((long) number))
	    {
	      name = dollar_label_name ((long) number, 0);
	      symbolP = symbol_find (name);
	      know (symbolP != NULL);
	    }
	  else
	    {
	      name = dollar_label_name ((long) number, 1);
	      symbolP = symbol_find_or_make (name);
	    }

	  expressionP->X_op = O_symbol;
	  expressionP->X_add_symbol = symbolP;
	  expressionP->X_add_number = 0;
	}			/* case '$' */
      else
	{
	  expressionP->X_op = O_constant;
#ifdef TARGET_WORD_SIZE
	  /* Sign extend NUMBER.  */
	  number |= (-(number >> (TARGET_WORD_SIZE - 1))) << (TARGET_WORD_SIZE - 1);
#endif
	  expressionP->X_add_number = number;
	  input_line_pointer--;	/* restore following character. */
	}			/* really just a number */
    }
  else
    {
      /* not a small number */
      expressionP->X_op = O_big;
      expressionP->X_add_number = number;	/* number of littlenums */
      input_line_pointer--;	/*->char following number. */
    }
}

/* Parse an MRI multi character constant.  */

static void
mri_char_constant (expressionP)
     expressionS *expressionP;
{
  int i;

  if (*input_line_pointer == '\''
      && input_line_pointer[1] != '\'')
    {
      expressionP->X_op = O_constant;
      expressionP->X_add_number = 0;
      return;
    }

  /* In order to get the correct byte ordering, we must build the
     number in reverse.  */
  for (i = SIZE_OF_LARGE_NUMBER - 1; i >= 0; i--)
    {
      int j;

      generic_bignum[i] = 0;
      for (j = 0; j < CHARS_PER_LITTLENUM; j++)
	{
	  if (*input_line_pointer == '\'')
	    {
	      if (input_line_pointer[1] != '\'')
		break;
	      ++input_line_pointer;
	    }
	  generic_bignum[i] <<= 8;
	  generic_bignum[i] += *input_line_pointer;
	  ++input_line_pointer;
	}

      if (i < SIZE_OF_LARGE_NUMBER - 1)
	{
	  /* If there is more than one littlenum, left justify the
             last one to make it match the earlier ones.  If there is
             only one, we can just use the value directly.  */
	  for (; j < CHARS_PER_LITTLENUM; j++)
	    generic_bignum[i] <<= 8;
	}

      if (*input_line_pointer == '\''
	  && input_line_pointer[1] != '\'')
	break;
    }

  if (i < 0)
    {
      as_bad ("Character constant too large");
      i = 0;
    }

  if (i > 0)
    {
      int c;
      int j;

      c = SIZE_OF_LARGE_NUMBER - i;
      for (j = 0; j < c; j++)
	generic_bignum[j] = generic_bignum[i + j];
      i = c;
    }

  know (LITTLENUM_NUMBER_OF_BITS == 16);
  if (i > 2)
    {
      expressionP->X_op = O_big;
      expressionP->X_add_number = i;
    }
  else
    {
      expressionP->X_op = O_constant;
      if (i < 2)
	expressionP->X_add_number = generic_bignum[0] & LITTLENUM_MASK;
      else
	expressionP->X_add_number =
	  (((generic_bignum[1] & LITTLENUM_MASK)
	    << LITTLENUM_NUMBER_OF_BITS)
	   | (generic_bignum[0] & LITTLENUM_MASK));
    }

  /* Skip the final closing quote.  */
  ++input_line_pointer;
}

/* Return an expression representing the current location.  This
   handles the magic symbol `.'.  */

static void
current_location (expressionp)
     expressionS *expressionp;
{
  if (now_seg == absolute_section)
    {
      expressionp->X_op = O_constant;
      expressionp->X_add_number = abs_section_offset;
    }
  else
    {
      symbolS *symbolp;

      symbolp = symbol_new (FAKE_LABEL_NAME, now_seg,
			    (valueT) frag_now_fix (),
			    frag_now);
      expressionp->X_op = O_symbol;
      expressionp->X_add_symbol = symbolp;
      expressionp->X_add_number = 0;
    }
}

/*
 * Summary of operand().
 *
 * in:	Input_line_pointer points to 1st char of operand, which may
 *	be a space.
 *
 * out:	A expressionS.
 *	The operand may have been empty: in this case X_op == O_absent.
 *	Input_line_pointer->(next non-blank) char after operand.
 */

static segT
operand (expressionP)
     expressionS *expressionP;
{
  char c;
  symbolS *symbolP;	/* points to symbol */
  char *name;		/* points to name of symbol */
  segT segment;

  /* All integers are regarded as unsigned unless they are negated.
     This is because the only thing which cares whether a number is
     unsigned is the code in emit_expr which extends constants into
     bignums.  It should only sign extend negative numbers, so that
     something like ``.quad 0x80000000'' is not sign extended even
     though it appears negative if valueT is 32 bits.  */
  expressionP->X_unsigned = 1;

  /* digits, assume it is a bignum. */

  SKIP_WHITESPACE ();		/* leading whitespace is part of operand. */
  c = *input_line_pointer++;	/* input_line_pointer->past char in c. */

  switch (c)
    {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      input_line_pointer--;

      integer_constant (flag_m68k_mri ? 0 : 10, expressionP);
      break;

    case '0':
      /* non-decimal radix */

      if (flag_m68k_mri)
	{
	  char *s;

	  /* Check for a hex constant.  */
	  for (s = input_line_pointer; hex_p (*s); s++)
	    ;
	  if (*s == 'h' || *s == 'H')
	    {
	      --input_line_pointer;
	      integer_constant (0, expressionP);
	      break;
	    }
	}

      c = *input_line_pointer;
      switch (c)
	{
	case 'o':
	case 'O':
	case 'q':
	case 'Q':
	case '8':
	case '9':
	  if (flag_m68k_mri)
	    {
	      integer_constant (0, expressionP);
	      break;
	    }
	  /* Fall through.  */
	default:
	default_case:
	  if (c && strchr (FLT_CHARS, c))
	    {
	      input_line_pointer++;
	      floating_constant (expressionP);
	      expressionP->X_add_number = -(isupper (c) ? tolower (c) : c);
	    }
	  else
	    {
	      /* The string was only zero */
	      expressionP->X_op = O_constant;
	      expressionP->X_add_number = 0;
	    }

	  break;

	case 'x':
	case 'X':
	  if (flag_m68k_mri)
	    goto default_case;
	  input_line_pointer++;
	  integer_constant (16, expressionP);
	  break;

	case 'b':
	  if (LOCAL_LABELS_FB && ! flag_m68k_mri)
	    {
	      switch (input_line_pointer[1])
		{
		case '+':
		case '-':
		  /* If unambiguously a difference expression, treat
		     it as one by indicating a label; otherwise, it's
		     always a binary number.  */
		  {
		    char *cp = input_line_pointer + 1;
		    while (strchr ("0123456789", *++cp))
		      ;
		    if (*cp == 'b' || *cp == 'f')
		      goto is_0b_label;
		  }
		  goto is_0b_binary;
		case '0':    case '1':
		  /* Some of our code elsewhere does permit digits
		     greater than the expected base; for consistency,
		     do the same here.  */
		case '2':    case '3':    case '4':    case '5':
		case '6':    case '7':    case '8':    case '9':
		  goto is_0b_binary;
		case 0:
		  goto is_0b_label;
		default:
		  goto is_0b_label;
		}
	    is_0b_label:
	      input_line_pointer--;
	      integer_constant (10, expressionP);
	      break;
	    is_0b_binary:
	      ;
	    }
	case 'B':
	  input_line_pointer++;
	  if (flag_m68k_mri)
	    goto default_case;
	  integer_constant (2, expressionP);
	  break;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	  integer_constant (flag_m68k_mri ? 0 : 8, expressionP);
	  break;

	case 'f':
	  if (LOCAL_LABELS_FB)
	    {
	      /* If it says "0f" and it could possibly be a floating point
		 number, make it one.  Otherwise, make it a local label,
		 and try to deal with parsing the rest later.  */
	      if (!input_line_pointer[1]
		  || (is_end_of_line[0xff & input_line_pointer[1]]))
		goto is_0f_label;
	      {
		char *cp = input_line_pointer + 1;
		int r = atof_generic (&cp, ".", EXP_CHARS,
				      &generic_floating_point_number);
		switch (r)
		  {
		  case 0:
		  case ERROR_EXPONENT_OVERFLOW:
		    if (*cp == 'f' || *cp == 'b')
		      /* looks like a difference expression */
		      goto is_0f_label;
		    else
		      goto is_0f_float;
		  default:
		    as_fatal ("expr.c(operand): bad atof_generic return val %d",
			      r);
		  }
	      }

	      /* Okay, now we've sorted it out.  We resume at one of these
		 two labels, depending on what we've decided we're probably
		 looking at.  */
	    is_0f_label:
	      input_line_pointer--;
	      integer_constant (10, expressionP);
	      break;

	    is_0f_float:
	      /* fall through */
	      ;
	    }

	case 'd':
	case 'D':
	  if (flag_m68k_mri)
	    {
	      integer_constant (0, expressionP);
	      break;
	    }
	  /* Fall through.  */
	case 'F':
	case 'r':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
	  input_line_pointer++;
	  floating_constant (expressionP);
	  expressionP->X_add_number = -(isupper (c) ? tolower (c) : c);
	  break;

	case '$':
	  if (LOCAL_LABELS_DOLLAR)
	    {
	      integer_constant (10, expressionP);
	      break;
	    }
	  else
	    goto default_case;
	}

      break;

    case '(':
    case '[':
      /* didn't begin with digit & not a name */
      segment = expression (expressionP);
      /* Expression() will pass trailing whitespace */
      if ((c == '(' && *input_line_pointer++ != ')')
	  || (c == '[' && *input_line_pointer++ != ']'))
	{
	  as_bad ("Missing ')' assumed");
	  input_line_pointer--;
	}
      SKIP_WHITESPACE ();
      /* here with input_line_pointer->char after "(...)" */
      return segment;

    case 'E':
      if (! flag_m68k_mri || *input_line_pointer != '\'')
	goto de_fault;
      as_bad ("EBCDIC constants are not supported");
      /* Fall through.  */
    case 'A':
      if (! flag_m68k_mri || *input_line_pointer != '\'')
	goto de_fault;
      ++input_line_pointer;
      /* Fall through.  */
    case '\'':
      if (! flag_m68k_mri)
	{
	  /* Warning: to conform to other people's assemblers NO
	     ESCAPEMENT is permitted for a single quote. The next
	     character, parity errors and all, is taken as the value
	     of the operand. VERY KINKY.  */
	  expressionP->X_op = O_constant;
	  expressionP->X_add_number = *input_line_pointer++;
	  break;
	}

      mri_char_constant (expressionP);
      break;

    case '+':
      (void) operand (expressionP);
      break;

    case '"':
      /* Double quote is the bitwise not operator in MRI mode.  */
      if (! flag_m68k_mri)
	goto de_fault;
      /* Fall through.  */
    case '!':
    case '~':
    case '-':
      {
	operand (expressionP);
	if (expressionP->X_op == O_constant)
	  {
	    /* input_line_pointer -> char after operand */
	    if (c == '-')
	      {
		expressionP->X_add_number = - expressionP->X_add_number;
		/* Notice: '-' may overflow: no warning is given. This is
		   compatible with other people's assemblers. Sigh.  */
		expressionP->X_unsigned = 0;
	      }
	    else if (c == '~' || c == '"')
	      expressionP->X_add_number = ~ expressionP->X_add_number;
	    else
	      expressionP->X_add_number = ! expressionP->X_add_number;
	  }
	else if (expressionP->X_op != O_illegal
		 && expressionP->X_op != O_absent)
	  {
	    expressionP->X_add_symbol = make_expr_symbol (expressionP);
	    if (c == '-')
	      expressionP->X_op = O_uminus;
	    else if (c == '~' || c == '"')
	      expressionP->X_op = O_bit_not;
	    else
	      expressionP->X_op = O_logical_not;
	    expressionP->X_add_number = 0;
	  }
	else
	  as_warn ("Unary operator %c ignored because bad operand follows",
		   c);
      }
      break;

    case '$':
      /* $ is the program counter when in MRI mode, or when DOLLAR_DOT
         is defined.  */
#ifndef DOLLAR_DOT
      if (! flag_m68k_mri)
	goto de_fault;
#endif
      if (flag_m68k_mri && hex_p (*input_line_pointer))
	{
	  /* In MRI mode, $ is also used as the prefix for a
             hexadecimal constant.  */
	  integer_constant (16, expressionP);
	  break;
	}

      if (is_part_of_name (*input_line_pointer))
	goto isname;

      current_location (expressionP);
      break;

    case '.':
      if (!is_part_of_name (*input_line_pointer))
	{
	  current_location (expressionP);
	  break;
	}
      else if ((strncasecmp (input_line_pointer, "startof.", 8) == 0
		&& ! is_part_of_name (input_line_pointer[8]))
	       || (strncasecmp (input_line_pointer, "sizeof.", 7) == 0
		   && ! is_part_of_name (input_line_pointer[7])))
	{
	  int start;

	  start = (input_line_pointer[1] == 't'
		   || input_line_pointer[1] == 'T');
	  input_line_pointer += start ? 8 : 7;
	  SKIP_WHITESPACE ();
	  if (*input_line_pointer != '(')
	    as_bad ("syntax error in .startof. or .sizeof.");
	  else
	    {
	      char *buf;

	      ++input_line_pointer;
	      SKIP_WHITESPACE ();
	      name = input_line_pointer;
	      c = get_symbol_end ();

	      buf = (char *) xmalloc (strlen (name) + 10);
	      if (start)
		sprintf (buf, ".startof.%s", name);
	      else
		sprintf (buf, ".sizeof.%s", name);
	      symbolP = symbol_make (buf);
	      free (buf);

	      expressionP->X_op = O_symbol;
	      expressionP->X_add_symbol = symbolP;
	      expressionP->X_add_number = 0;

	      *input_line_pointer = c;
	      SKIP_WHITESPACE ();
	      if (*input_line_pointer != ')')
		as_bad ("syntax error in .startof. or .sizeof.");
	      else
		++input_line_pointer;
	    }
	  break;
	}
      else
	{
	  goto isname;
	}
    case ',':
    case '\n':
    case '\0':
    eol:
      /* can't imagine any other kind of operand */
      expressionP->X_op = O_absent;
      input_line_pointer--;
      break;

    case '%':
      if (! flag_m68k_mri)
	goto de_fault;
      integer_constant (2, expressionP);
      break;

    case '@':
      if (! flag_m68k_mri)
	goto de_fault;
      integer_constant (8, expressionP);
      break;

    case ':':
      if (! flag_m68k_mri)
	goto de_fault;

      /* In MRI mode, this is a floating point constant represented
         using hexadecimal digits.  */

      ++input_line_pointer;
      integer_constant (16, expressionP);
      break;

    case '*':
      if (! flag_m68k_mri || is_part_of_name (*input_line_pointer))
	goto de_fault;

      current_location (expressionP);
      break;

    default:
    de_fault:
      if (is_end_of_line[(unsigned char) c])
	goto eol;
      if (is_name_beginner (c))	/* here if did not begin with a digit */
	{
	  /*
	   * Identifier begins here.
	   * This is kludged for speed, so code is repeated.
	   */
	isname:
	  name = --input_line_pointer;
	  c = get_symbol_end ();

#ifdef TC_I960
	  /* The MRI i960 assembler permits
	         lda sizeof code,g13
	     */
	  if (flag_mri
	      && (strcasecmp (name, "sizeof") == 0
		  || strcasecmp (name, "startof") == 0))
	    {
	      int start;
	      char *buf;

	      start = (name[1] == 't'
		       || name[1] == 'T');

	      *input_line_pointer = c;
	      SKIP_WHITESPACE ();

	      name = input_line_pointer;
	      c = get_symbol_end ();

	      buf = (char *) xmalloc (strlen (name) + 10);
	      if (start)
		sprintf (buf, ".startof.%s", name);
	      else
		sprintf (buf, ".sizeof.%s", name);
	      symbolP = symbol_make (buf);
	      free (buf);

	      expressionP->X_op = O_symbol;
	      expressionP->X_add_symbol = symbolP;
	      expressionP->X_add_number = 0;

	      *input_line_pointer = c;
	      SKIP_WHITESPACE ();

	      break;
	    }	      
#endif

	  symbolP = symbol_find_or_make (name);

	  /* If we have an absolute symbol or a reg, then we know its
	     value now.  */
	  segment = S_GET_SEGMENT (symbolP);
	  if (segment == absolute_section)
	    {
	      expressionP->X_op = O_constant;
	      expressionP->X_add_number = S_GET_VALUE (symbolP);
	    }
	  else if (segment == reg_section)
	    {
	      expressionP->X_op = O_register;
	      expressionP->X_add_number = S_GET_VALUE (symbolP);
	    }
	  else
	    {
	      expressionP->X_op = O_symbol;
	      expressionP->X_add_symbol = symbolP;
	      expressionP->X_add_number = 0;
	    }
	  *input_line_pointer = c;
	}
      else
	{
	  /* Let the target try to parse it.  Success is indicated by changing
	     the X_op field to something other than O_absent and pointing
	     input_line_pointer passed the expression.  If it can't parse the
	     expression, X_op and input_line_pointer should be unchanged.  */
	  expressionP->X_op = O_absent;
	  --input_line_pointer;
	  md_operand (expressionP);
	  if (expressionP->X_op == O_absent)
	    {
	      ++input_line_pointer;
	      as_bad ("Bad expression");
	      expressionP->X_op = O_constant;
	      expressionP->X_add_number = 0;
	    }
	}
      break;
    }

  /*
   * It is more 'efficient' to clean up the expressionS when they are created.
   * Doing it here saves lines of code.
   */
  clean_up_expression (expressionP);
  SKIP_WHITESPACE ();		/*->1st char after operand. */
  know (*input_line_pointer != ' ');

  /* The PA port needs this information.  */
  if (expressionP->X_add_symbol)
    expressionP->X_add_symbol->sy_used = 1;

  switch (expressionP->X_op)
    {
    default:
      return absolute_section;
    case O_symbol:
      return S_GET_SEGMENT (expressionP->X_add_symbol);
    case O_register:
      return reg_section;
    }
}				/* operand() */

/* Internal. Simplify a struct expression for use by expr() */

/*
 * In:	address of a expressionS.
 *	The X_op field of the expressionS may only take certain values.
 *	Elsewise we waste time special-case testing. Sigh. Ditto SEG_ABSENT.
 * Out:	expressionS may have been modified:
 *	'foo-foo' symbol references cancelled to 0,
 *		which changes X_op from O_subtract to O_constant.
 *	Unused fields zeroed to help expr().
 */

static void
clean_up_expression (expressionP)
     expressionS *expressionP;
{
  switch (expressionP->X_op)
    {
    case O_illegal:
    case O_absent:
      expressionP->X_add_number = 0;
      /* Fall through.  */
    case O_big:
    case O_constant:
    case O_register:
      expressionP->X_add_symbol = NULL;
      /* Fall through.  */
    case O_symbol:
    case O_uminus:
    case O_bit_not:
      expressionP->X_op_symbol = NULL;
      break;
    case O_subtract:
      if (expressionP->X_op_symbol == expressionP->X_add_symbol
	  || ((expressionP->X_op_symbol->sy_frag
	       == expressionP->X_add_symbol->sy_frag)
	      && SEG_NORMAL (S_GET_SEGMENT (expressionP->X_add_symbol))
	      && (S_GET_VALUE (expressionP->X_op_symbol)
		  == S_GET_VALUE (expressionP->X_add_symbol))))
	{
	  addressT diff = (S_GET_VALUE (expressionP->X_add_symbol)
			   - S_GET_VALUE (expressionP->X_op_symbol));

	  expressionP->X_op = O_constant;
	  expressionP->X_add_symbol = NULL;
	  expressionP->X_op_symbol = NULL;
	  expressionP->X_add_number += diff;
	}
      break;
    default:
      break;
    }
}

/* Expression parser. */

/*
 * We allow an empty expression, and just assume (absolute,0) silently.
 * Unary operators and parenthetical expressions are treated as operands.
 * As usual, Q==quantity==operand, O==operator, X==expression mnemonics.
 *
 * We used to do a aho/ullman shift-reduce parser, but the logic got so
 * warped that I flushed it and wrote a recursive-descent parser instead.
 * Now things are stable, would anybody like to write a fast parser?
 * Most expressions are either register (which does not even reach here)
 * or 1 symbol. Then "symbol+constant" and "symbol-symbol" are common.
 * So I guess it doesn't really matter how inefficient more complex expressions
 * are parsed.
 *
 * After expr(RANK,resultP) input_line_pointer->operator of rank <= RANK.
 * Also, we have consumed any leading or trailing spaces (operand does that)
 * and done all intervening operators.
 *
 * This returns the segment of the result, which will be
 * absolute_section or the segment of a symbol.
 */

#undef __
#define __ O_illegal

static operatorT op_encoding[256] =
{				/* maps ASCII->operators */

  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,

  __, O_bit_or_not, __, __, __, O_modulus, O_bit_and, __,
  __, __, O_multiply, O_add, __, O_subtract, __, O_divide,
  __, __, __, __, __, __, __, __,
  __, __, __, __, O_lt, __, O_gt, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, O_bit_exclusive_or, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, O_bit_inclusive_or, __, __, __,

  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __
};


/*
 *	Rank	Examples
 *	0	operand, (expression)
 *	1	||
 *	2	&&
 *	3	= <> < <= >= >
 *	4	+ -
 *	5	used for * / % in MRI mode
 *	6	& ^ ! |
 *	7	* / % << >>
 *	8	unary - unary ~
 */
static operator_rankT op_rank[] =
{
  0,	/* O_illegal */
  0,	/* O_absent */
  0,	/* O_constant */
  0,	/* O_symbol */
  0,	/* O_symbol_rva */
  0,	/* O_register */
  0,	/* O_bit */
  8,	/* O_uminus */
  8,	/* O_bit_not */
  8,	/* O_logical_not */
  7,	/* O_multiply */
  7,	/* O_divide */
  7,	/* O_modulus */
  7,	/* O_left_shift */
  7,	/* O_right_shift */
  6,	/* O_bit_inclusive_or */
  6,	/* O_bit_or_not */
  6,	/* O_bit_exclusive_or */
  6,	/* O_bit_and */
  4,	/* O_add */
  4,	/* O_subtract */
  3,	/* O_eq */
  3,	/* O_ne */
  3,	/* O_lt */
  3,	/* O_le */
  3,	/* O_ge */
  3,	/* O_gt */
  2,	/* O_logical_and */
  1	/* O_logical_or */
};

/* Initialize the expression parser.  */

void
expr_begin ()
{
  /* In MRI mode for the m68k, multiplication and division have lower
     precedence than the bit wise operators.  */
  if (flag_m68k_mri)
    {
      op_rank[O_multiply] = 5;
      op_rank[O_divide] = 5;
      op_rank[O_modulus] = 5;
      op_encoding['"'] = O_bit_not;
    }

  /* Verify that X_op field is wide enough.  */
  {
    expressionS e;
    e.X_op = O_max;
    assert (e.X_op == O_max);
  }
}

/* Return the encoding for the operator at INPUT_LINE_POINTER.
   Advance INPUT_LINE_POINTER to the last character in the operator
   (i.e., don't change it for a single character operator).  */

static inline operatorT
operator ()
{
  int c;
  operatorT ret;

  c = *input_line_pointer;

  switch (c)
    {
    default:
      return op_encoding[c];

    case '<':
      switch (input_line_pointer[1])
	{
	default:
	  return op_encoding[c];
	case '<':
	  ret = O_left_shift;
	  break;
	case '>':
	  ret = O_ne;
	  break;
	case '=':
	  ret = O_le;
	  break;
	}
      ++input_line_pointer;
      return ret;

    case '>':
      switch (input_line_pointer[1])
	{
	default:
	  return op_encoding[c];
	case '>':
	  ret = O_right_shift;
	  break;
	case '=':
	  ret = O_ge;
	  break;
	}
      ++input_line_pointer;
      return ret;

    case '!':
      /* We accept !! as equivalent to ^ for MRI compatibility.  */
      if (input_line_pointer[1] != '!')
	{
	  if (flag_m68k_mri)
	    return O_bit_inclusive_or;
	  return op_encoding[c];
	}
      ++input_line_pointer;
      return O_bit_exclusive_or;

    case '|':
      if (input_line_pointer[1] != '|')
	return op_encoding[c];

      ++input_line_pointer;
      return O_logical_or;

    case '&':
      if (input_line_pointer[1] != '&')
	return op_encoding[c];

      ++input_line_pointer;
      return O_logical_and;
    }

  /*NOTREACHED*/
}	

/* Parse an expression.  */

segT
expr (rank, resultP)
     operator_rankT rank;	/* Larger # is higher rank. */
     expressionS *resultP;	/* Deliver result here. */
{
  segT retval;
  expressionS right;
  operatorT op_left;
  operatorT op_right;

  know (rank >= 0);

  retval = operand (resultP);

  know (*input_line_pointer != ' ');	/* Operand() gobbles spaces. */

  op_left = operator ();
  while (op_left != O_illegal && op_rank[(int) op_left] > rank)
    {
      segT rightseg;

      input_line_pointer++;	/*->after 1st character of operator. */

      rightseg = expr (op_rank[(int) op_left], &right);
      if (right.X_op == O_absent)
	{
	  as_warn ("missing operand; zero assumed");
	  right.X_op = O_constant;
	  right.X_add_number = 0;
	  right.X_add_symbol = NULL;
	  right.X_op_symbol = NULL;
	}

      know (*input_line_pointer != ' ');

      if (retval == undefined_section)
	{
	  if (SEG_NORMAL (rightseg))
	    retval = rightseg;
	}
      else if (! SEG_NORMAL (retval))
	retval = rightseg;
      else if (SEG_NORMAL (rightseg)
	       && retval != rightseg
#ifdef DIFF_EXPR_OK
	       && op_left != O_subtract
#endif
	       )
	as_bad ("operation combines symbols in different segments");

      op_right = operator ();

      know (op_right == O_illegal || op_rank[(int) op_right] <= op_rank[(int) op_left]);
      know ((int) op_left >= (int) O_multiply
	    && (int) op_left <= (int) O_logical_or);

      /* input_line_pointer->after right-hand quantity. */
      /* left-hand quantity in resultP */
      /* right-hand quantity in right. */
      /* operator in op_left. */

      if (resultP->X_op == O_big)
	{
	  as_warn ("left operand is a %s; integer 0 assumed",
		   resultP->X_add_number > 0 ? "bignum" : "float");
	  resultP->X_op = O_constant;
	  resultP->X_add_number = 0;
	  resultP->X_add_symbol = NULL;
	  resultP->X_op_symbol = NULL;
	}
      if (right.X_op == O_big)
	{
	  as_warn ("right operand is a %s; integer 0 assumed",
		   right.X_add_number > 0 ? "bignum" : "float");
	  right.X_op = O_constant;
	  right.X_add_number = 0;
	  right.X_add_symbol = NULL;
	  right.X_op_symbol = NULL;
	}

      /* Optimize common cases.  */
      if (op_left == O_add && right.X_op == O_constant)
	{
	  /* X + constant.  */
	  resultP->X_add_number += right.X_add_number;
	}
      /* This case comes up in PIC code.  */
      else if (op_left == O_subtract
	       && right.X_op == O_symbol
	       && resultP->X_op == O_symbol
	       && (right.X_add_symbol->sy_frag
		   == resultP->X_add_symbol->sy_frag)
	       && SEG_NORMAL (S_GET_SEGMENT (right.X_add_symbol)))

	{
	  resultP->X_add_number += right.X_add_number;
	  resultP->X_add_number += (S_GET_VALUE (resultP->X_add_symbol)
				    - S_GET_VALUE (right.X_add_symbol));
	  resultP->X_op = O_constant;
	  resultP->X_add_symbol = 0;
	}
      else if (op_left == O_subtract && right.X_op == O_constant)
	{
	  /* X - constant.  */
	  resultP->X_add_number -= right.X_add_number;
	}
      else if (op_left == O_add && resultP->X_op == O_constant)
	{
	  /* Constant + X.  */
	  resultP->X_op = right.X_op;
	  resultP->X_add_symbol = right.X_add_symbol;
	  resultP->X_op_symbol = right.X_op_symbol;
	  resultP->X_add_number += right.X_add_number;
	  retval = rightseg;
	}
      else if (resultP->X_op == O_constant && right.X_op == O_constant)
	{
	  /* Constant OP constant.  */
	  offsetT v = right.X_add_number;
	  if (v == 0 && (op_left == O_divide || op_left == O_modulus))
	    {
	      as_warn ("division by zero");
	      v = 1;
	    }
	  switch (op_left)
	    {
	    default:			abort ();
	    case O_multiply:		resultP->X_add_number *= v; break;
	    case O_divide:		resultP->X_add_number /= v; break;
	    case O_modulus:		resultP->X_add_number %= v; break;
	    case O_left_shift:		resultP->X_add_number <<= v; break;
	    case O_right_shift:		resultP->X_add_number >>= v; break;
	    case O_bit_inclusive_or:	resultP->X_add_number |= v; break;
	    case O_bit_or_not:		resultP->X_add_number |= ~v; break;
	    case O_bit_exclusive_or:	resultP->X_add_number ^= v; break;
	    case O_bit_and:		resultP->X_add_number &= v; break;
	    case O_add:			resultP->X_add_number += v; break;
	    case O_subtract:		resultP->X_add_number -= v; break;
	    case O_eq:
	      resultP->X_add_number =
		resultP->X_add_number == v ? ~ (offsetT) 0 : 0;
	      break;
	    case O_ne:
	      resultP->X_add_number =
		resultP->X_add_number != v ? ~ (offsetT) 0 : 0;
	      break;
	    case O_lt:
	      resultP->X_add_number =
		resultP->X_add_number <  v ? ~ (offsetT) 0 : 0;
	      break;
	    case O_le:
	      resultP->X_add_number =
		resultP->X_add_number <= v ? ~ (offsetT) 0 : 0;
	      break;
	    case O_ge:
	      resultP->X_add_number =
		resultP->X_add_number >= v ? ~ (offsetT) 0 : 0;
	      break;
	    case O_gt:
	      resultP->X_add_number =
		resultP->X_add_number >  v ? ~ (offsetT) 0 : 0;
	      break;
	    case O_logical_and:
	      resultP->X_add_number = resultP->X_add_number && v;
	      break;
	    case O_logical_or:
	      resultP->X_add_number = resultP->X_add_number || v;
	      break;
	    }
	}
      else if (resultP->X_op == O_symbol
	       && right.X_op == O_symbol
	       && (op_left == O_add
		   || op_left == O_subtract
		   || (resultP->X_add_number == 0
		       && right.X_add_number == 0)))
	{
	  /* Symbol OP symbol.  */
	  resultP->X_op = op_left;
	  resultP->X_op_symbol = right.X_add_symbol;
	  if (op_left == O_add)
	    resultP->X_add_number += right.X_add_number;
	  else if (op_left == O_subtract)
	    resultP->X_add_number -= right.X_add_number;
	}
      else
	{
	  /* The general case.  */
	  resultP->X_add_symbol = make_expr_symbol (resultP);
	  resultP->X_op_symbol = make_expr_symbol (&right);
	  resultP->X_op = op_left;
	  resultP->X_add_number = 0;
	  resultP->X_unsigned = 1;
	}

      op_left = op_right;
    }				/* While next operator is >= this rank. */

  /* The PA port needs this information.  */
  if (resultP->X_add_symbol)
    resultP->X_add_symbol->sy_used = 1;

  return resultP->X_op == O_constant ? absolute_section : retval;
}

/*
 *			get_symbol_end()
 *
 * This lives here because it belongs equally in expr.c & read.c.
 * Expr.c is just a branch office read.c anyway, and putting it
 * here lessens the crowd at read.c.
 *
 * Assume input_line_pointer is at start of symbol name.
 * Advance input_line_pointer past symbol name.
 * Turn that character into a '\0', returning its former value.
 * This allows a string compare (RMS wants symbol names to be strings)
 * of the symbol name.
 * There will always be a char following symbol name, because all good
 * lines end in end-of-line.
 */
char
get_symbol_end ()
{
  char c;

  /* We accept \001 in a name in case this is being called with a
     constructed string.  */
  while (is_part_of_name (c = *input_line_pointer++)
	 || c == '\001')
    ;
  *--input_line_pointer = 0;
  return (c);
}


unsigned int
get_single_number ()
{
  expressionS exp;
  operand (&exp);
  return exp.X_add_number;

}

/* end of expr.c */
