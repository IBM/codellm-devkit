/* "Normal" configuration for alloca.  */

#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not __GNUC__ */
#ifdef sparc
#include <alloca.h>
#ifdef __STDC__
extern void *__builtin_alloca();
#else
extern char *__builtin_alloca();  /* Stupid include file doesn't declare it */
#endif
#else
#ifdef __STDC__
PTR alloca (size_t);
#else
PTR alloca ();			/* must agree with functions.def */
#endif
#endif /* sparc */
#endif /* not __GNUC__ */
