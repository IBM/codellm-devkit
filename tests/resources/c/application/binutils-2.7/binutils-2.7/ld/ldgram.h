typedef union {
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
} YYSTYPE;
#define	INT	258
#define	NAME	259
#define	LNAME	260
#define	PLUSEQ	261
#define	MINUSEQ	262
#define	MULTEQ	263
#define	DIVEQ	264
#define	LSHIFTEQ	265
#define	RSHIFTEQ	266
#define	ANDEQ	267
#define	OREQ	268
#define	OROR	269
#define	ANDAND	270
#define	EQ	271
#define	NE	272
#define	LE	273
#define	GE	274
#define	LSHIFT	275
#define	RSHIFT	276
#define	UNARY	277
#define	END	278
#define	ALIGN_K	279
#define	BLOCK	280
#define	BIND	281
#define	QUAD	282
#define	LONG	283
#define	SHORT	284
#define	BYTE	285
#define	SECTIONS	286
#define	PHDRS	287
#define	SIZEOF_HEADERS	288
#define	OUTPUT_FORMAT	289
#define	FORCE_COMMON_ALLOCATION	290
#define	OUTPUT_ARCH	291
#define	INCLUDE	292
#define	MEMORY	293
#define	DEFSYMEND	294
#define	NOLOAD	295
#define	DSECT	296
#define	COPY	297
#define	INFO	298
#define	OVERLAY	299
#define	DEFINED	300
#define	TARGET_K	301
#define	SEARCH_DIR	302
#define	MAP	303
#define	ENTRY	304
#define	SIZEOF	305
#define	NEXT	306
#define	ADDR	307
#define	STARTUP	308
#define	HLL	309
#define	SYSLIB	310
#define	FLOAT	311
#define	NOFLOAT	312
#define	ORIGIN	313
#define	FILL	314
#define	LENGTH	315
#define	CREATE_OBJECT_SYMBOLS	316
#define	INPUT	317
#define	GROUP	318
#define	OUTPUT	319
#define	CONSTRUCTORS	320
#define	ALIGNMOD	321
#define	AT	322
#define	PROVIDE	323
#define	CHIP	324
#define	LIST	325
#define	SECT	326
#define	ABSOLUTE	327
#define	LOAD	328
#define	NEWLINE	329
#define	ENDWORD	330
#define	ORDER	331
#define	NAMEWORD	332
#define	FORMAT	333
#define	PUBLIC	334
#define	BASE	335
#define	ALIAS	336
#define	TRUNCATE	337
#define	REL	338
#define	INPUT_SCRIPT	339
#define	INPUT_MRI_SCRIPT	340
#define	INPUT_DEFSYM	341
#define	CASE	342
#define	EXTERN	343
#define	START	344


extern YYSTYPE yylval;
