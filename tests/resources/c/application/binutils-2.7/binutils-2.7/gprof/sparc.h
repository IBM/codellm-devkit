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
 *
 *      @(#)tahoe.h     1.4 (Berkeley) 6/1/90
 */

    /*
     *        opcode of the `callf' instruction
     */
#define	CALL	(0xc0000000)
    /*
     *        offset (in bytes) of the code from the entry address of a routine.
     *  (see asgnsamples for use and explanation.)
     */
#define OFFSET_TO_CODE	0
#define	UNITS_TO_CODE	(OFFSET_TO_CODE / sizeof(UNIT))
