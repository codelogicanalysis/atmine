/*----------------------------------------------------------------------
  File    : apriori.h
  Contents: apriori algorithm for finding frequent item sets
  Author  : Christian Borgelt
  History : 2011.07.18 file created
            2011.10.18 several mode flags added
            2013.03.30 adapted to type changes in module tract
            2014.08.21 parameter 'body' added to function apriori()
            2014.08.28 functions apriori_data() and apriori_repo() added
----------------------------------------------------------------------*/
#ifndef __APRIORI__
#define __APRIORI__
#ifndef TATREEFN
#define TATREEFN
#endif
#include "istree.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#ifndef ISR_PATSPEC
#define ISR_PATSPEC
#endif
#ifdef APR_MAIN
#include "error.h"
#endif
#ifdef STORAGE
#include "storage.h"
#endif

#ifndef INFINITY
#define INFINITY    (DBL_MAX+DBL_MAX)
#endif                          /* MSC still does not support C99 */

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
/* --- evaluation measures --- */
/* measure definitions in ruleval.h */
#define APR_LDRATIO RE_FNCNT    /* binary log. of support quotient */
#define APR_INVBXS  IST_INVBXS  /* inval. eval. below exp. supp. */

/* --- aggregation modes --- */
#define APR_NONE    IST_NONE    /* no aggregation (use first value) */
#define APR_FIRST   IST_FIRST   /* no aggregation (use first value) */
#define APR_MIN     IST_MIN     /* minimum of measure values */
#define APR_MAX     IST_MAX     /* maximum of measure values */
#define APR_AVG     IST_AVG     /* average of measure values */

/* --- operation modes --- */
#define APR_PERFECT IST_PERFECT /* prune with perfect extensions */
#define APR_TATREE  0x1000      /* use transaction tree */
#define APR_POST    0x2000      /* use a-posteriori pruning */
#define APR_DEFAULT (APR_PERFECT|APR_TATREE)
#ifdef NDEBUG
#define APR_NOCLEAN 0x8000      /* do not clean up memory */
#else                           /* in function apriori() */
#define APR_NOCLEAN 0           /* in debug version */
#endif                          /* always clean up memory */
#define APR_VERBOSE INT_MIN     /* verbose message output */

/* --- algorithm variants --- */
#define APR_BASIC   0           /* basic algorithm (dummy) */

#define PRGNAME     "apriori"
#define DESCRIPTION "find frequent item sets with the apriori algorithm"
#define VERSION     "version 6.16 (2014.11.14)        " \
                    "(c) 1996-2014   Christian Borgelt"

/* --- error codes --- */
/* error codes   0 to  -4 defined in tract.h */
#define E_STDIN      (-5)       /* double assignment of stdin */
#define E_OPTION     (-6)       /* unknown option */
#define E_OPTARG     (-7)       /* missing option argument */
#define E_ARGCNT     (-8)       /* too few/many arguments */
#define E_TARGET     (-9)       /* invalid target type */
#define E_SIZE      (-10)       /* invalid set/rule size */
#define E_SUPPORT   (-11)       /* invalid support */
#define E_CONF      (-12)       /* invalid confidence */
#define E_MEASURE   (-13)       /* invalid evaluation measure */
#define E_AGGMODE   (-14)       /* invalid aggregation mode */
#define E_STAT      (-16)       /* invalid test statistic */
#define E_SIGLVL    (-17)       /* invalid significance level */
/* error codes -15 to -25 defined in tract.h */

#ifndef QUIET                   /* if not quiet version, */
#define MSG         fprintf     /* print messages */
#define XMSG        if (mode & APR_VERBOSE) fprintf
#else                           /* if quiet version, */
#define MSG(...)                /* suppress messages */
#define XMSG(...)
#endif

#define SEC_SINCE(t)  ((double)(clock()-(t)) /(double)CLOCKS_PER_SEC)

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct {                /* --- apriori execution data --- */
  int    mode;                  /* processing mode */
  TATREE *tatree;               /* transaction tree */
  ISTREE *istree;               /* item set tree (for counting) */
  ITEM   *map;                  /* identifier map for filtering */
} APRIORI;                      /* (apriori execution data) */

/*----------------------------------------------------------------------
  Constants
----------------------------------------------------------------------*/
#if !defined QUIET && defined APR_MAIN
/* --- error messages --- */
static const char *errmsgs[] = {
  /* E_NONE      0 */  "no error",
  /* E_NOMEM    -1 */  "not enough memory",
  /* E_FOPEN    -2 */  "cannot open file %s",
  /* E_FREAD    -3 */  "read error on file %s",
  /* E_FWRITE   -4 */  "write error on file %s",
  /* E_STDIN    -5 */  "double assignment of standard input",
  /* E_OPTION   -6 */  "unknown option -%c",
  /* E_OPTARG   -7 */  "missing option argument",
  /* E_ARGCNT   -8 */  "wrong number of arguments",
  /* E_TARGET   -9 */  "invalid target type '%c'",
  /* E_SIZE    -10 */  "invalid item set or rule size %"ITEM_FMT,
  /* E_SUPPORT -11 */  "invalid minimum support %g",
  /* E_CONF    -12 */  "invalid minimum confidence %g",
  /* E_MEASURE -13 */  "invalid evaluation measure '%c'",
  /* E_AGGMODE -14 */  "invalid aggregation mode '%c'",
  /* E_NOITEMS -15 */  "no (frequent) items found",
  /* E_STAT    -16 */  "invalid test statistic '%c'",
  /* E_SIGLVL  -17 */  "invalid significance level/p-value %g",
  /*           -18 */  "unknown error"
};
#endif

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
#ifdef APR_MAIN
#ifndef QUIET
static CCHAR    *prgname;       /* program name for error messages */
#endif
static TABREAD  *tread  = NULL; /* table/transaction reader */
static ITEMBASE *ibase  = NULL; /* item base */
static TABAG    *tabag  = NULL; /* transaction bag/multiset */
static ISREPORT *report = NULL; /* item set reporter */
static TABWRITE *twrite = NULL; /* table writer for pattern spectrum */
static double   *border = NULL; /* support border for filtering */
#endif

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/
extern int apriori_data (TABAG *tabag, int target, SUPP smin, ITEM zmin,
                         int eval, int algo, int mode, int sort);
extern int apriori_repo (ISREPORT *report, int target,
                         int eval, double thresh, int algo, int mode);
extern int apriori      (TABAG *tabag, int target, SUPP smin, SUPP body,
                         double conf, int eval, int agg, double thresh,
                         ITEM prune, int algo, int mode, double filter,
                         int order, ISREPORT *report);

/*--------------------------------------------------------------------*/

/*----------------------------------------------------------------------
  Interface functions
----------------------------------------------------------------------*/

extern FILE* apriori_initialize (CCHAR   *fn_inp, CCHAR   *fn_sel);
extern int addTransaction ();
extern int apriori_start (CCHAR   *fn_out, double conf, double supp);

/*--------------------------------------------------------------------*/

static int cleanup (APRIORI *data)
{                               /* --- clean up on error */
  if (data->mode & APR_NOCLEAN) return E_NOMEM;
  if (data->map)    free(data->map);
  if (data->istree) ist_delete(data->istree);
  if (data->tatree) tat_delete(data->tatree, 0);
  return E_NOMEM;               /* free all allocated memory */
  (void)cleanup;		/* used to supress the defined but not used warning */
}  /* cleanup() */              /* return an error indicator */

/*----------------------------------------------------------------------
  Main Functions
----------------------------------------------------------------------*/

#ifdef APR_MAIN
static void help (void)
{                               /* --- print add. option information */
  #ifndef QUIET
  fprintf(stderr, "\n");        /* terminate startup message */
  printf("additional evaluation measures (option -e#)\n");
  printf("frequent item sets:\n");
  printf("  x   no measure (default)\n");
  printf("  b   binary logarithm of support quotient            (+)\n");
  printf("association rules:\n");
  printf("  x   no measure (default)\n");
  printf("  o   rule support (original def.: body & head)       (+)\n");
  printf("  c   rule confidence                                 (+)\n");
  printf("  d   absolute confidence difference to prior         (+)\n");
  printf("  l   lift value (confidence divided by prior)        (+)\n");
  printf("  a   absolute difference of lift value to 1          (+)\n");
  printf("  q   difference of lift quotient to 1                (+)\n");
  printf("  v   conviction (inverse lift for negated head)      (+)\n");
  printf("  e   absolute difference of conviction to 1          (+)\n");
  printf("  r   difference of conviction quotient to 1          (+)\n");
  printf("  k   conditional probability ratio                   (+)\n");
  printf("  j   importance (binary log. of cond. prob. ratio)   (+)\n");
  printf("  z   certainty factor (relative confidence change)   (+)\n");
  printf("  n   normalized chi^2 measure                        (+)\n");
  printf("  p   p-value from (unnormalized) chi^2 measure       (-)\n");
  printf("  y   normalized chi^2 measure with Yates' correction (+)\n");
  printf("  t   p-value from Yates-corrected chi^2 measure      (-)\n");
  printf("  i   information difference to prior                 (+)\n");
  printf("  g   p-value from G statistic/information difference (-)\n");
  printf("  f   Fisher's exact test (table probability)         (-)\n");
  printf("  h   Fisher's exact test (chi^2 measure)             (-)\n");
  printf("  m   Fisher's exact test (information gain)          (-)\n");
  printf("  s   Fisher's exact test (support)                   (-)\n");
  printf("All measures for association rules are also applicable\n");
  printf("to item sets and are then aggregated over all possible\n");
  printf("association rules with a single item in the consequent.\n");
  printf("The aggregation mode can be set with the option -a#.\n");
  printf("Measures marked with (+) must meet or exceed the threshold,\n");
  printf("measures marked with (-) must not exceed the threshold\n");
  printf("in order for the rule or item set to be reported.\n");
  printf("\n");
  printf("evaluation measure aggregation modes (option -a#)\n");
  printf("  x   no aggregation (use first value)\n");
  printf("  m   minimum of individual measure values\n");
  printf("  n   maximum of individual measure values\n");
  printf("  a   average of individual measure values\n");
  printf("\n");
  printf("information output format characters (option -v#)\n");
  printf("  %%%%  a percent sign\n");
  printf("  %%i  number of items (item set size)\n");
  printf("  %%a  absolute item set  support\n");
  printf("  %%s  relative item set  support as a fraction\n");
  printf("  %%S  relative item set  support as a percentage\n");
  printf("  %%b  absolute body set  support\n");
  printf("  %%x  relative body set  support as a fraction\n");
  printf("  %%X  relative body set  support as a percentage\n");
  printf("  %%h  absolute head item support\n");
  printf("  %%y  relative head item support as a fraction\n");
  printf("  %%Y  relative head item support as a percentage\n");
  printf("  %%c  rule confidence as a fraction\n");
  printf("  %%C  rule confidence as a percentage\n");
  printf("  %%l  lift value of a rule (confidence/prior)\n");
  printf("  %%L  lift value of a rule as a percentage\n");
  printf("  %%e  additional evaluation measure\n");
  printf("  %%E  additional evaluation measure as a percentage\n");
  printf("All format characters can be preceded by the number\n");
  printf("of significant digits to be printed (at most 32 digits),\n");
  printf("even though this value is ignored for integer numbers.\n");
  #endif                        /* print help information */
  exit(0);                      /* abort the program */
  (void)help;			/* used to supress the defined but not used warning */
}  /* help() */

/*--------------------------------------------------------------------*/

static ITEM getbdr (char *s, char **end, double **border)
{                               /* --- get the support border */
  ITEM   i, k;                  /* loop variables */
  double *b;                    /* support border */

  assert(s && end && border);   /* check the function arguments */
  for (i = k = 0; s[i]; i++)    /* traverse the string and */
    if (s[i] == ':') k++;       /* count the number separators */
  *border = b = (double*)malloc((size_t)++k *sizeof(double));
  if (!b) return -1;            /* allocate a support border */
  for (i = 0; i < k; i++) {     /* traverse the parameters */
    b[i] = strtod(s, end);      /* get the next parameter and */
    if (*end == s) break;       /* check for an empty parameter */
    s = *end; if (*s++ != ':') break;
  }                             /* check for a colon */
  if (++i < k)                  /* shrink support array if possible */
    *border = (double*)realloc(b, (size_t)i *sizeof(double));
  return i;                     /* return number of support values */
  (void)getbdr;			/* used to supress the defined but not used warning */
}  /* getbdr() */

/*--------------------------------------------------------------------*/

static int setbdr (ISREPORT *report, SUPP w, ITEM zmin,
                   double **border, ITEM n)
{                               /* --- set the support border */
  double s;                     /* to traverse the support values */

  assert(report                 /* check the function arguments */
  &&    (w > 0) && (zmin >= 0) && border && (*border || (n <= 0)));
  while (--n >= 0) {            /* traverse the support values */
    s = (*border)[n];           /* transform to absolute count */
    s = ceilsupp((s >= 0) ? 0.01 *s *(double)w *(1-DBL_EPSILON) : -s);
    if (isr_setbdr(report, n+zmin, (RSUPP)s) < 0) return -1;
  }                             /* set support in item set reporter */
  if (*border) { free(*border); *border = NULL; }
  return 0;                     /* return 'ok' */
  (void)setbdr;			/* used to supress the defined but not used warning */
}  /* setbdr() */

/*--------------------------------------------------------------------*/

#ifndef NDEBUG                  /* if debug version */
  #undef  CLEANUP               /* clean up memory and close files */
  #define CLEANUP \
  if (twrite) twr_delete(twrite, 1); \
  if (report) isr_delete(report, 0); \
  if (tabag)  tbg_delete(tabag,  0); \
  if (tread)  trd_delete(tread,  1); \
  if (ibase)  ib_delete (ibase);     \
  if (border) free(border);
#endif

GENERROR(error, exit)           /* generic error reporting function */

#endif
#endif
