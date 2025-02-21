/* Should gas use high-level BFD interfaces?  */
#undef BFD_ASSEMBLER

/* Some assert/preprocessor combinations are incapable of handling
   certain kinds of constructs in the argument of assert.  For example,
   quoted strings (if requoting isn't done right) or newlines.  */
#undef BROKEN_ASSERT

/* If we aren't doing cross-assembling, some operations can be optimized,
   since byte orders and value sizes don't need to be adjusted.  */
#undef CROSS_COMPILE

/* Some gas code wants to know these parameters.  */
#undef TARGET_ALIAS
#undef TARGET_CPU
#undef TARGET_CANONICAL
#undef TARGET_OS
#undef TARGET_VENDOR

/* Sometimes the system header files don't declare malloc and realloc.  */
#undef NEED_DECLARATION_MALLOC

/* Sometimes the system header files don't declare free.  */
#undef NEED_DECLARATION_FREE

/* Sometimes errno.h doesn't declare errno itself.  */
#undef NEED_DECLARATION_ERRNO

#undef MANY_SEGMENTS

/* Needed only for sparc configuration.  */
#undef SPARC_V9
#undef SPARC_ARCH64

/* Needed only for some configurations that can produce multiple output
   formats.  */
#undef DEFAULT_EMULATION
#undef EMULATIONS
#undef USE_EMULATIONS
#undef OBJ_MAYBE_AOUT
#undef OBJ_MAYBE_BOUT
#undef OBJ_MAYBE_COFF
#undef OBJ_MAYBE_ECOFF
#undef OBJ_MAYBE_ELF
#undef OBJ_MAYBE_GENERIC
#undef OBJ_MAYBE_HP300
#undef OBJ_MAYBE_IEEE
#undef OBJ_MAYBE_SOM
#undef OBJ_MAYBE_VMS

/* Used for some of the COFF configurations, when the COFF code needs
   to select something based on the CPU type before it knows it... */
#undef I386COFF
#undef M68KCOFF
#undef M88KCOFF
