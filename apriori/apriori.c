/*----------------------------------------------------------------------
  File    : apriori.c
  Contents: apriori algorithm for finding association rules
  Author  : Christian Borgelt
  History : 1996.02.14 file created
            1996.07.26 output precision reduced
            1996.11.22 options -b, -f, and -r added
            1996.11.24 option -e added (add. evaluation measures)
            1997.08.18 normalized chi^2 measure and option -m added
            1997.10.13 quiet version (no output to stdout or stderr)
            1998.01.27 adapted to changed ist_create() function
            1998.08.08 optional input file (item appearances) added
            1998.09.07 hyperedge mode added (option -h)
            1998.12.08 output of absolute support added (option -a)
            1998.12.09 conversion of names to a scanable form added
            1999.02.09 input from stdin, output to stdout added
            1999.08.09 bug in check of support parameter fixed (<= 0)
            1999.11.05 rule evaluation measure IST_LIFTDIFF added
            1999.11.08 output of add. rule eval. measure value added
            2000.03.16 optional use of original rule support definition
            2001.04.01 option -h replaced by option -t (target type)
            2001.05.26 extended support output added
            2001.08.15 module scan used for item name formatting
            2001.11.18 item and transaction functions made a module
            2001.11.19 options -C, -l changed, option -y removed
            2001.12.28 adapted to module tract, some improvements
            2002.01.11 evaluation measures codes changed to letters
            2002.02.10 option -q extended by a direction parameter
            2002.02.11 memory usage minimization option added
            2002.06.09 arbitrary supp./conf. formats made possible
            2003.01.09 option -k added (user-defined item separator)
            2003.01.14 check for empty transaction set added
            2003.07.17 item filtering w.r.t. usage added (option -u)
            2003.07.17 sorting w.r.t. transaction size sum added
            2003.07.18 maximal item set filter added
            2003.08.11 closed  item set filter added
            2003.08.15 item filtering for transaction tree added
            2003.08.16 parameter for transaction filtering added
            2003.08.18 dynamic filtering decision based on times added
            2004.03.25 option -S added (maximal support of a set/rule)
            2004.05.09 additional selection measures for sets added
            2004.10.28 two unnecessary assignments removed
            2004.11.23 absolute/relative support output changed
            2005.01.25 bug in output of absolute/relative support fixed
            2005.01.31 another bug in this output fixed
            2005.06.20 use of flag for "no item sorting" corrected
            2007.02.13 adapted to modified module tabread
            2008.08.12 option -l removed (do not load transactions)
            2008.08.14 adapted to extension of function tbg_filter()
            2008.08.18 more flexible output format control (numbers)
            2008.09.01 option -I added (implication sign for rules)
            2008.09.07 optional a-posteriori pruning added
            2008.09.09 more flexible information output control
            2008.09.10 item set extraction and evaluation redesigned
            2008.10.30 adapted to changes in item set reporting
            2008.11.13 adapted to changes in transaction management
            2008.11.19 adapted to modified transaction tree interface
            2008.12.06 perfect extension pruning added (optional)
            2009.05.28 adapted to modified function tbg_filter()
            2010.03.02 bug concerning size-sorted output fixed
            2010.03.04 several additional evaluation measures added
            2010.06.18 filtering for increase of evaluation added (-i)
            2010.07.14 output file made optional (for benchmarking)
            2010.08.22 adapted to modified modules tract and tabread
            2010.08.30 Fisher's exact test added as evaluation measure
            2010.10.15 adapted to modified interface of module report
            2010.10.22 chi^2 measure with Yates correction added
            2010.11.24 adapted to modified error reporting (tract)
            2010.12.11 adapted to a generic error reporting function
            2011.03.20 optional integer transaction weights added
            2011.07.08 adapted to modified function tbg_recode()
            2011.07.21 bug in function apriori fixed (closed/maximal)
            2011.07.25 threshold inverted for measures yielding p-values
            2011.08.04 weak forward pruning with evaluation added
            2011.08.16 new target type 'generators' added (option -tg)
            2011.08.28 output of item set counters per size added
            2011.10.18 all processing shifted to apriori function
            2012.01.23 rounding the mininum support for mining fixed
            2012.04.30 bug in apriori() fixed ("rule(s)" output)
            2012.06.13 bug in apriori() fixed (APR_INVBXS in eval)
            2012.11.29 bug in apriori() fixed (perf. ext. and ISR_RULE)
            2013.03.30 adapted to type changes in module tract
            2013.06.03 real-valued support made possible (SUPP==double)
            2013.10.18 optional pattern spectrum collection added
            2013.11.12 item selection file changed to option -R#
            2014.05.12 option -F# added (support border for filtering)
            2014.05.15 evaluation measures "cprob" and "import" added
            2014.05.31 bug with number of items in selection file fixed
            2014.07.03 missing terminal output of item set sizes added
            2014.08.18 preprocessing of transactions moved to apriori()
            2014.08.21 adapted to modified item set reporter interface
            2014.08.28 functions apriori_data() and apriori_repo() added
            2014.10.24 changed from LGPL license to MIT license
------------------------------------------------------------------------
  Reference for the Apriori algorithm:
    R. Agrawal and R. Srikant.
    Fast Algorithms for Mining Association Rules.
    Proc. 20th Int. Conf. on Very Large Databases
    (VLDB 1994, Santiago de Chile), 487-499.
    Morgan Kaufmann, San Mateo, CA, USA 1994
----------------------------------------------------------------------*/
#include "apriori.h"

/*----------------------------------------------------------------------
  Apriori Algorithm (with plain transactions)
----------------------------------------------------------------------*/

int apriori_data (TABAG *tabag, int target, SUPP smin, ITEM zmin,
                  int eval, int algo, int mode, int sort)
{                               /* --- prepare data for Apriori */
  ITEM    m;                    /* number of items */
  TID     n;                    /* number of transactions */
  SUPP    w;                    /* total transaction weight, buffer */
  clock_t t;                    /* timer for measurements */

  assert(tabag);                /* check the function arguments */

  /* --- sort and recode items --- */
  t = clock();                  /* start timer, print log message */
  XMSG(stderr, "filtering, sorting and recoding items ... ");
  m = tbg_recode(tabag, smin, -1, -1, sort);
  if (m <  0) return E_NOMEM;   /* recode items and transactions */
  if (m <= 0) return E_NOITEMS; /* and check the number of items */
  XMSG(stderr, "[%"ITEM_FMT" item(s)]", m);
  XMSG(stderr, " done [%.2fs].\n", SEC_SINCE(t));

  /* --- sort and reduce transactions --- */
  t = clock();                  /* start timer, print log message */
  XMSG(stderr, "sorting and reducing transactions ... ");
  eval &= ~APR_INVBXS;          /* get evaluation without flags */
  if (!(target & ISR_RULES)     /* filter transactions if possible */
  &&  ((eval <= RE_NONE) || (eval >= RE_FNCNT)))
    tbg_filter(tabag, zmin, NULL, 0);
  tbg_itsort(tabag, +1, 0);     /* sort items in transactions and */
  tbg_sort  (tabag, +1, 0);     /* sort the trans. lexicographically */
  n = tbg_reduce(tabag, 0);     /* reduce transactions to unique ones */
  w = tbg_wgt(tabag);           /* get the new transaction weight */
  XMSG(stderr, "[%"TID_FMT, n); /* print number of transactions */
  if (w != (SUPP)n) { XMSG(stderr, "/%"SUPP_FMT, w); }
  XMSG(stderr, " transaction(s)] done [%.2fs].\n", SEC_SINCE(t));
  return 0;                     /* return 'ok' */
}  /* apr_data() */

/*--------------------------------------------------------------------*/

int apriori_repo (ISREPORT *report, int target,
                  int eval, double thresh, int algo, int mode)
{                               /* --- prepare reporter for Apriori */
  assert(report);               /* check the function arguments */
  if (eval == APR_LDRATIO)      /* set additional evaluation measure */
    isr_seteval(report, isr_logrto, NULL, +1, thresh);
  return (isr_settarg(report, target, ISR_NOFILTER, -1)) ? E_NOMEM : 0;
}  /* apriori_repo() */

int apriori (TABAG *tabag, int target, SUPP smin, SUPP body,
             double conf, int eval, int agg, double thresh,
             ITEM prune, int algo, int mode, double filter,
             int order, ISREPORT *report)
{                               /* --- apriori algorithm */
  ITEM    m, i, k;              /* number of items, loop variables */
  int     e;                    /* evaluation without flags */
  ITEM    size, zmin, zmax;     /* number of items in set/rule */
  clock_t t, tt, tc, x;         /* timers for measurements */
  APRIORI a = { 0, NULL, NULL, NULL };  /* apriori execution data */

  assert(tabag && report);      /* check the function arguments */
  a.mode = mode;                /* note the processing mode */
  e = eval & ~APR_INVBXS;       /* get evaluation without flags */
  if (e <= RE_NONE)             /* if there is no evaluation, */
    prune = ITEM_MIN;           /* do not prune with evaluation */

  /* --- create transaction tree --- */
  tt = 0;                       /* init. the tree construction time */
  if (mode & APR_TATREE) {      /* if to use a transaction tree */
    t = clock();                /* start the timer for construction */
    XMSG(stderr, "building transaction tree ... ");
    a.tatree = tat_create(tabag);  /* create a transaction tree */
    if (!a.tatree) return E_NOMEM; /* as a compressed representation */
    XMSG(stderr, "[%"SIZE_FMT" node(s)]", tat_size(a.tatree));
    XMSG(stderr, " done [%.2fs].\n", SEC_SINCE(t));
    tt = clock() -t;            /* note the time for the construction */
  }                             /* of the transaction tree */

  /* --- create item set tree --- */
  if ((target & (ISR_CLOSED|ISR_MAXIMAL|ISR_RULES))
  ||  ((e > RE_NONE) && (e < RE_FNCNT)) || order)
    mode &= ~IST_PERFECT;       /* remove perfect extension pruning */
  t = clock(); tc = 0;          /* start the timer for the search */
  a.istree = ist_create(tbg_base(tabag), mode, smin, body, conf);
  if (!a.istree) return cleanup(&a);
  zmin = isr_zmin(report);      /* create an item set tree */
  zmax = isr_zmax(report);      /* and configure it */
  if ((target & (ISR_CLOSED|ISR_MAXIMAL))
  && !(target & ISR_RULES) && (zmax < ITEM_MAX))
    zmax += 1;                  /* adapt the maximum size */
  if (zmax > (m = tbg_max(tabag))) zmax = m;
  ist_setsize(a.istree, zmin, zmax);
  if (e == APR_LDRATIO)         /* set add. evaluation measure */
       isr_seteval(report, isr_logrto, NULL, +1, thresh);
  else ist_seteval(a.istree, eval, agg, thresh, prune);

  /* --- check item subsets --- */
  XMSG(stderr, "checking subsets of size 1");
  m = tbg_itemcnt(tabag);       /* get the number of items */
  a.map = (ITEM*)malloc((size_t)m *sizeof(ITEM));
  if (!a.map) return cleanup(&a);
  for (i = m; 1; ) {            /* traverse the item set sizes */
    size = ist_height(a.istree);/* get the current item set size */
    if (size >= zmax) break;    /* abort if maximal size is reached */
    if ((filter != 0)           /* if to filter w.r.t. item usage */
    && ((i = ist_check(a.istree, (int*)a.map)) <= size))
      break;                    /* check which items are still used */
    if (mode & APR_POST)        /* if a-posteriori pruning requested, */
      ist_prune(a.istree);      /* prune infrequent item sets */
    k = ist_addlvl(a.istree);   /* add a level to the item set tree */
    if (k < 0) return cleanup(&a);
    if (k > 0) break;           /* if no level was added, abort */
    if (((filter < 0)           /* if to filter w.r.t. item usage and */
    &&   ((double)i < -filter *(double)m))    /* enough items removed */
    ||  ((filter > 0) && (i < m)/* or counting time is long enough */
    && ((double)i*(double)tt < (double)filter*(double)m*(double)tc))) {
      m = i;                    /* note the new number of items */
      x = clock();              /* start the timer for filtering */
      if (a.tatree) {           /* if a transaction tree was created */
        if (tat_filter(a.tatree, size+1, (int*)a.map, 0) != 0)
          return cleanup(&a); } /* filter the transaction tree */
      else {                    /* if there is only a transaction bag */
        tbg_filter(tabag, size+1, (int*)a.map, 0);
        tbg_sort  (tabag, 0,0); /* remove unnecessary items and */
        tbg_reduce(tabag, 0);   /* transactions and reduce */
      }                         /* transactions to unique ones */
      tt = clock() -x;          /* note the filter/rebuild time */
    }
    size += 1;                  /* increment the item set size */
    XMSG(stderr, " %"ITEM_FMT, size);          /* and print it */
    x = clock();                /* start the timer for counting */
    if (a.tatree) ist_countx(a.istree, a.tatree);
    else          ist_countb(a.istree, tabag);
    ist_commit(a.istree);       /* count the transaction tree/bag */
    tc = clock() -x;            /* compute the new counting time */
  }
  free(a.map); a.map = NULL;    /* delete filter map and trans. tree */
  if (!(mode & APR_NOCLEAN) && a.tatree) {
    tat_delete(a.tatree, 0); a.tatree = NULL; }
  XMSG(stderr, " done [%.2fs].\n", SEC_SINCE(t));

  /* --- filter found item sets --- */
  if ((prune >  ITEM_MIN)       /* if to filter with evaluation */
  &&  (prune <= 0)) {           /* (backward and weak forward) */
    t = clock();                /* start the timer for filtering */
    XMSG(stderr, "filtering with evaluation ... ");
    ist_filter(a.istree,prune); /* mark non-qualifying item sets */
    XMSG(stderr, "done [%.2fs].\n", SEC_SINCE(t));
  }                             /* filter with evaluation */
  if (target & (ISR_CLOSED|ISR_MAXIMAL|ISR_GENERAS)) {
    t = clock();                /* start the timer for filtering */
    XMSG(stderr, "filtering for %s item sets ... ",
         (target & ISR_GENERAS) ? "generator" :
         (target & ISR_MAXIMAL) ? "maximal" : "closed");
    ist_clomax(a.istree, target | ((prune > ITEM_MIN) ? IST_SAFE : 0));
    XMSG(stderr, "done [%.2fs].\n", SEC_SINCE(t));
  }                             /* filter closed/maximal/generators */

  /* --- report item sets/association rules --- */
  t = clock();                  /* start the output timer */
  XMSG(stderr, "writing %s ... ", isr_name(report));
  ist_init(a.istree, order);    /* initialize the extraction */
  if (ist_report(a.istree, report, target) < 0)
    return cleanup(&a);         /* report item sets/rules */
  XMSG(stderr, "[%"SIZE_FMT" %s(s)]", isr_repcnt(report),
               (target == ISR_RULES) ? "rule" : "set");
  XMSG(stderr, " done [%.2fs].\n", SEC_SINCE(t));
  #ifdef BENCH                  /* if benchmark version, */
  ist_stats(a.istree);          /* show the search statistics */
  #endif                        /* (especially memory usage) */
  cleanup(&a);                  /* clean up the allocated memory */
  return 0;                     /* return 'ok' */
}  /* apriori() */


/*---------------------------------------------------------------------
  Interface functions
----------------------------------------------------------------------*/

FILE* apriori_initialize(CCHAR   *fn_inp, CCHAR   *fn_sel) {
    /* --- read item selection/appearance indicators --- */
    ibase = ib_create(0, 0);      /* create an item base */
    if (!ibase) error(E_NOMEM);   /* to manage the items */
    tread = trd_create();         /* create a transaction reader */
    if (!tread) error(E_NOMEM);   /* and configure the characters */
    trd_allchs(tread, NULL, NULL, NULL, "", NULL);
    if (fn_sel) {                 /* if an item selection is given */
        clock_t t = clock();      /* start timer, open input file */
        if (trd_open(tread, NULL, fn_sel) != 0) {
            error(E_FOPEN, trd_name(tread));
            return NULL;
        }
        MSG(stderr, "reading %s ... ", trd_name(tread));
        ITEM m = ib_readapp(ibase,tread);        /* read the item appearances */
        if (m < 0) {
            error((int)-m, ib_errmsg(ibase, NULL, 0));
            return NULL;
        }
        trd_close(tread);           /* close the input file */
        MSG(stderr, "[%"ITEM_FMT" item(s)]", ib_cnt(ibase));
        MSG(stderr, " done [%.2fs].\n", SEC_SINCE(t));
    }                             /* print a log message */

    /* --- read transaction database --- */
    tabag = tbg_create(ibase);    /* create a transaction bag to store the transactions */
    if (!tabag) {
        error(E_NOMEM);
        return NULL;
    }
    FILE* fp = fopen(fn_inp, "w+");
    if (!fp) {
        return NULL;
    }
    tread->name = fn_inp;
    tread->file  = fp;
    tread->delim = tread->last = TRD_EOF;
    tread->next  = tread->end  = tread->buf;
    tread->rec   = 1;               /* current record is the first */
    tread->pos   = 0;               /* position is before first field */
    tread->field[tread->len = 0] = 0; /* current field is empty */
    MSG(stderr, "reading %s ... ", trd_name(tread));
    return fp;
}

int addTransaction() {
    int k = tbg_read(tabag, tread, 0);
    if (k < 0) {
        error(-k, tbg_errmsg(tabag, NULL, 0));
        return -1;
    }
    return 0;
}

#if 0
int addTransaction(CCHAR   *fn_inp) {
    if (trd_open(tread, NULL, fn_inp) != 0) {
        error(E_FOPEN, trd_name(tread));
        return -1;
    }
    int k = tbg_read(tabag, tread, 0);
    if (k < 0) {
        error(-k, tbg_errmsg(tabag, NULL, 0));
        return -1;
    }
    return 0;
}
#endif

int apriori_start(CCHAR   *fn_out, double conf, double supp) {
    CCHAR   *info = (supp < 0) ? " (%b, %C)" : " (%X, %C)";
    trd_delete(tread, 1);         /* read the transaction database, */
    tread = NULL;                 /* then delete the table reader */
    ITEM m = ib_cnt(ibase);            /* get the number of items, */
    TID n = tbg_cnt(tabag);           /* the number of transactions, */
    SUPP w = tbg_wgt(tabag);           /* the total transaction weight */
    MSG(stderr, "[%"ITEM_FMT" item(s), %"TID_FMT, m, n);
    if (w != (SUPP)n) {
        MSG(stderr, "/%"SUPP_FMT, w);
    }
    //MSG(stderr, " transaction(s)] done [%.2fs].", SEC_SINCE(t));
    if ((m <= 0) || (n <= 0)) {   /* check for at least one item */
        error(E_NOITEMS);         /* and at least one transaction */
        return -1;
    }
    MSG(stderr, "\n");            /* terminate the log message */
    conf *= 0.01;                 /* scale the minimum confidence */
    supp = (supp >= 0) ? 0.01 *supp *(double)w *(1-DBL_EPSILON) : -supp;
    double smax = (double)w *(1+DBL_EPSILON);
    int target = ISR_RULES;
    int orig = 0;
    SUPP body = (SUPP)ceilsupp(supp);  /* compute absolute support values */
    SUPP smin = (SUPP)ceilsupp((!(target & ISR_RULES) || orig)
         ? supp : ceilsupp(supp) *conf *(1-DBL_EPSILON));

    /* --- find frequent item sets/association rules --- */
    int mode = APR_DEFAULT;
    mode |= APR_VERBOSE|APR_NOCLEAN;
    int k = apriori_data(tabag, target, smin, 1, RE_NONE, mode, APR_BASIC, 2);
    if (k) error(k);              /* prepare data for Apriori */
    report = isr_create(ibase);   /* create an item set reporter */
    if (!report) error(E_NOMEM);  /* and configure it */
    isr_setsize(report,        1, (ITEM)7);
    isr_setsupp(report, (RSUPP)smin, (RSUPP)floorsupp(smax));
    if (setbdr(report, w, 1, &border, 0) != 0)
      error(E_NOMEM);             /* set limits and support border */
    if (isr_setfmt(report, 0, "", " ", " <- ", info) != 0)
      error(E_NOMEM);             /* set the output format strings */
    k = isr_open(report, NULL, fn_out);
    if (k) error(k, isr_name(report)); /* open item set output file */
    if ((apriori_repo(report, target, RE_NONE, 0.1, APR_BASIC, mode) < 0)
    ||  (isr_setup(report) < 0))  /* prepare reporter for Apriori */
      error(E_NOMEM);             /* and set up the item set reporter */
    k = apriori(tabag, target, smin, body, conf, RE_NONE, APR_NONE, 0.1,
                ITEM_MIN, APR_BASIC, mode, 0.01, 0, report);
    if (k) error(k);              /* find frequent item sets */
    if (isr_close(report) != 0)   /* close item set output file */
      error(E_FWRITE, isr_name(report));

    /* --- clean up --- */
    CLEANUP;                      /* clean up memory and close files */
    SHOWMEM;                      /* show (final) memory usage */
    return 0;
}

/*--------------------------------------------------------------------*/

#ifdef APR_MAIN

/*--------------------------------------------------------------------*/
#if 0
int main (int argc, char *argv[])
{                               /* --- main function */
  int     i, k = 0;             /* loop variables, counters */
  char    *s;                   /* to traverse the options */
  CCHAR   **optarg = NULL;      /* option argument */
  CCHAR   *fn_inp  = NULL;      /* name of the input  file */
  CCHAR   *fn_out  = NULL;      /* name of the output file */
  CCHAR   *fn_sel  = NULL;      /* name of item selection file */
  CCHAR   *fn_psp  = NULL;      /* name of pattern spectrum file */
  CCHAR   *recseps = NULL;      /* record  separators */
  CCHAR   *fldseps = NULL;      /* field   separators */
  CCHAR   *blanks  = NULL;      /* blank   characters */
  CCHAR   *comment = NULL;      /* comment characters */
  CCHAR   *hdr     = "";        /* record header  for output */
  CCHAR   *sep     = " ";       /* item separator for output */
  CCHAR   *imp     = " <- ";    /* implication sign for ass. rules */
  CCHAR   *dflt    = " (%S)";   /* default format for check */
  CCHAR   *info    = dflt;      /* format for information output */
  int     target   = 's';       /* target type (e.g. closed/maximal) */
  double  supp     = 10;        /* minimum support of an item set */
  SUPP    smin     = 1;         /* minimum support of an item set */
  double  smax     = 100;       /* maximum support of an item set */
  SUPP    body     = 10;        /* minimum support of a rule body */
  int     orig     = 0;         /* flag for rule support definition */
  double  conf     = 80;        /* minimum confidence (in percent) */
  ITEM    zmin     = 1;         /* minimum rule/item set size */
  ITEM    zmax     = ITEM_MAX;  /* maximum rule/item set size */
  int     eval     = 'x';       /* additional evaluation measure */
  int     eflgs    = 0;         /* evaluation measure flags */
  int     agg      = 'x';       /* aggregation mode for eval. measure */
  double  thresh   = 10;        /* threshold for evaluation measure */
  ITEM    prune    = ITEM_MIN;  /* (min. size for) evaluation pruning */
  int     sort     = 2;         /* flag for item sorting and recoding */
  int     algo     = APR_BASIC; /* variant of apriori algorithm */
  int     mode     = APR_DEFAULT;  /* search mode (e.g. pruning) */
  double  filter   = 0.01;      /* item usage filtering parameter */
  int     order    = 0;         /* size order item set/rule output */
  int     mtar     = 0;         /* mode for transaction reading */
  int     scan     = 0;         /* flag for scanable item output */
  int     bdrcnt   = 0;         /* number of support values in border */
  int     stats    = 0;         /* flag for item set statistics */
  PATSPEC *psp;                 /* collected pattern spectrum */
  ITEM    m;                    /* number of items */
  TID     n;                    /* number of transactions */
  SUPP    w;                    /* total transaction weight */
  clock_t t;                    /* timer for measurements */

  #ifndef QUIET                 /* if not quiet version */
  prgname = argv[0];            /* get program name for error msgs. */

  /* --- print usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no arguments are given */
    printf("usage: %s [options] infile [outfile]\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-t#      target type                              "
                    "(default: %c)\n", target);
    printf("         (s: frequent, c: closed, m: maximal item sets,\n");
    printf("          g: generators, r: association rules)\n");
    printf("-m#      minimum number of items per set/rule     "
                    "(default: %"ITEM_FMT")\n", zmin);
    printf("-n#      maximum number of items per set/rule     "
                    "(default: no limit)\n");
    printf("-s#      minimum support of an item set/rule      "
                    "(default: %g%%)\n", supp);
    printf("-S#      maximum support of an item set/rule      "
                    "(default: %g%%)\n", smax);
    printf("         (positive: percentage, "
                     "negative: absolute number)\n");
    printf("-o       use original rule support definition     "
                    "(body & head)\n");
    printf("-c#      minimum confidence of an assoc. rule     "
                    "(default: %g%%)\n", conf);
    printf("-e#      additional evaluation measure            "
                    "(default: none)\n");
    printf("-a#      aggregation mode for evaluation measure  "
                    "(default: none)\n");
    printf("-d#      threshold for add. evaluation measure    "
                    "(default: %g%%)\n", thresh);
    printf("-z       invalidate eval. below expected support  "
                    "(default: evaluate all)\n");
    printf("-p#      (min. size for) pruning with evaluation  "
                    "(default: no pruning)\n");
    printf("         (< 0: weak forward, > 0 strong forward, "
                     "= 0: backward pruning)\n");
    printf("-q#      sort items w.r.t. their frequency        "
                    "(default: %d)\n", sort);
    printf("         (1: ascending, -1: descending, 0: do not sort,\n"
           "          2: ascending, -2: descending w.r.t. "
                    "transaction size sum)\n");
    printf("-u#      filter unused items from transactions    "
                    "(default: %g)\n", filter);
    printf("         (0: do not filter items w.r.t. usage in sets,\n"
           "         <0: fraction of removed items for filtering,\n"
           "         >0: take execution times ratio into account)\n");
    printf("-x       do not prune with perfect extensions     "
                    "(default: prune)\n");
    printf("-y       a-posteriori pruning of infrequent item sets\n");
    printf("-T       do not organize transactions as a prefix tree\n");
    printf("-F#:#..  support border for filtering item sets   "
                    "(default: none)\n");
    printf("         (list of minimum support values, "
                    "one per item set size,\n");
    printf("         starting at the minimum size, "
                    "as given with option -m#)\n");
    printf("-R#      read item selection/appearance indicators\n");
    printf("-P#      write a pattern spectrum to a file\n");
    printf("-Z       print item set statistics "
                    "(number of item sets per size)\n");
    printf("-g       write item names in scanable form "
                    "(quote certain characters)\n");
    printf("-h#      record header  for output                "
                    "(default: \"%s\")\n", hdr);
    printf("-k#      item separator for output                "
                    "(default: \"%s\")\n", sep);
    printf("-I#      implication sign for association rules   "
                    "(default: \"%s\")\n", imp);
    printf("-v#      output format for set/rule information   "
                    "(default: \"%s\")\n", info);
    printf("-j#      sort item sets in output by their size   "
                    "(default: no sorting)\n");
    printf("         (< 0: descending, > 0: ascending order)\n");
    printf("-w       integer transaction weight in last field "
                    "(default: only items)\n");
    printf("-r#      record/transaction separators            "
                    "(default: \"\\n\")\n");
    printf("-f#      field /item        separators            "
                    "(default: \" \\t,\")\n");
    printf("-b#      blank   characters                       "
                    "(default: \" \\t\\r\")\n");
    printf("-C#      comment characters                       "
                    "(default: \"#\")\n");
    printf("-!       print additional option information\n");
    printf("infile   file to read transactions from           "
                    "[required]\n");
    printf("outfile  file to write item sets/assoc. rules to  "
                    "[optional]\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */
  #endif  /* #ifndef QUIET */
  /* free option characters: il [A-Z]\[CFIPSTZ] */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse the arguments */
    s = argv[i];                /* get an option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (*s) {              /* traverse the options */
        switch (*s++) {         /* evaluate the options */
          case '!': help();                          break;
          case 't': target = (*s) ? *s++ : 's';      break;
          case 'm': zmin   = (ITEM)strtol(s, &s, 0); break;
          case 'n': zmax   = (ITEM)strtol(s, &s, 0); break;
          case 's': supp   =       strtod(s, &s);    break;
          case 'S': smax   =       strtod(s, &s);    break;
          case 'o': orig   = 1;                      break;
          case 'c': conf   =       strtod(s, &s);    break;
          case 'e': eval   = (*s) ? *s++ : 0;        break;
          case 'a': agg    = (*s) ? *s++ : 0;        break;
          case 'd': thresh =       strtod(s, &s);    break;
          case 'z': eflgs |= APR_INVBXS;             break;
          case 'p': prune  = (ITEM)strtol(s, &s, 0); break;
          case 'q': sort   = (int) strtol(s, &s, 0); break;
          case 'u': filter =       strtod(s, &s);    break;
          case 'x': mode  &= ~APR_PERFECT;           break;
          case 'y': mode  |=  APR_POST;              break;
          case 'T': mode  &= ~APR_TATREE;            break;
          case 'F': bdrcnt = getbdr(s, &s, &border); break;
          case 'R': optarg = &fn_sel;                break;
          case 'P': optarg = &fn_psp;                break;
          case 'Z': stats  = 1;                      break;
          case 'g': scan   = 1;                      break;
          case 'h': optarg = &hdr;                   break;
          case 'k': optarg = &sep;                   break;
          case 'I': optarg = &imp;                   break;
          case 'v': optarg = &info;                  break;
          case 'j': order  = (int) strtol(s, &s, 0); break;
          case 'w': mtar  |= TA_WEIGHT;              break;
          case 'r': optarg = &recseps;               break;
          case 'f': optarg = &fldseps;               break;
          case 'b': optarg = &blanks;                break;
          case 'C': optarg = &comment;               break;
          default : error(E_OPTION, *--s);           break;
        }                       /* set the option variables */
        if (optarg && *s) { *optarg = s; optarg = NULL; break; }
      } }                       /* get an option argument */
    else {                      /* -- if argument is no option */
      switch (k++) {            /* evaluate non-options */
        case  0: fn_inp = s;      break;
        case  1: fn_out = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg)       error(E_OPTARG);     /* check option arguments */
  if (k      < 1)   error(E_ARGCNT);     /* and number of arguments */
  if (zmin   < 0)   error(E_SIZE, zmin); /* check the size limits */
  if (zmax   < 0)   error(E_SIZE, zmax); /* and the minimum support */
  if (supp   > 100) error(E_SUPPORT, supp);
  if (bdrcnt < 0)   error(E_NOMEM);
  if ((conf  < 0) || (conf > 100))
    error(E_CONF, conf);        /* check the minimum confidence */
  if ((!fn_inp || !*fn_inp) && (fn_sel && !*fn_sel))
    error(E_STDIN);             /* stdin must not be used twice */
  switch (target) {             /* check and translate target type */
    case 's': target = ISR_ALL;              break;
    case 'c': target = ISR_CLOSED;           break;
    case 'm': target = ISR_MAXIMAL;          break;
    case 'g': target = ISR_GENERAS;          break;
    case 'r': target = ISR_RULES;            break;
    default : error(E_TARGET, (char)target); break;
  }                             /* (get target type code) */
  switch (eval) {               /* check and translate measure */
    case 'x': eval = RE_NONE;                break;
    case 'o': eval = RE_SUPP;                break;
    case 'c': eval = RE_CONF;                break;
    case 'd': eval = RE_CONFDIFF;            break;
    case 'l': eval = RE_LIFT;                break;
    case 'a': eval = RE_LIFTDIFF;            break;
    case 'q': eval = RE_LIFTQUOT;            break;
    case 'v': eval = RE_CVCT;                break;
    case 'e': eval = RE_CVCTDIFF;            break;
    case 'r': eval = RE_CVCTQUOT;            break;
    case 'k': eval = RE_CPROB;               break;
    case 'j': eval = RE_IMPORT;              break;
    case 'z': eval = RE_CERT;                break;
    case 'n': eval = RE_CHI2;                break;
    case 'p': eval = RE_CHI2PVAL;            break;
    case 'y': eval = RE_YATES;               break;
    case 't': eval = RE_YATESPVAL;           break;
    case 'i': eval = RE_INFO;                break;
    case 'g': eval = RE_INFOPVAL;            break;
    case 'f': eval = RE_FETPROB;             break;
    case 'h': eval = RE_FETCHI2;             break;
    case 'm': eval = RE_FETINFO;             break;
    case 's': eval = RE_FETSUPP;             break;
    case 'b': eval = APR_LDRATIO;            break;
    default : error(E_MEASURE, (char)eval);  break;
  }  /* free: u w */            /* (get evaluation measure code) */
  eval |= eflgs;                /* add evaluation measure flags */
  switch (agg) {                /* check and translate agg. mode */
    case 'x': agg  = APR_NONE;               break;
    case 'm': agg  = APR_MIN;                break;
    case 'n': agg  = APR_MAX;                break;
    case 'a': agg  = APR_AVG;                break;
    default : error(E_AGGMODE, (char)agg);   break;
  }                             /* (get aggregation mode code) */
  if ((filter <= -1) || (filter >= 1))
    filter = 0;                 /* check and adapt the filter option */
  if (target & ISR_RULES) fn_psp = NULL;
  else conf = 100;              /* no pattern spectrum for rules */
  if (info == dflt) {           /* if default info. format is used, */
    if (target != ISR_RULES)    /* set default according to target */
         info = (supp < 0) ? " (%a)"     : " (%S)";
    else info = (supp < 0) ? " (%b, %C)" : " (%X, %C)";
  }                             /* select absolute/relative support */
  thresh *= 0.01;               /* scale the evaluation threshold */
  MSG(stderr, "\n");            /* terminate the startup message */

  /* --- read item selection/appearance indicators --- */
  ibase = ib_create(0, 0);      /* create an item base */
  if (!ibase) error(E_NOMEM);   /* to manage the items */
  tread = trd_create();         /* create a transaction reader */
  if (!tread) error(E_NOMEM);   /* and configure the characters */
  trd_allchs(tread, recseps, fldseps, blanks, "", comment);
  if (fn_sel) {                 /* if an item selection is given */
    t = clock();                /* start timer, open input file */
    if (trd_open(tread, NULL, fn_sel) != 0)
      error(E_FOPEN, trd_name(tread));
    MSG(stderr, "reading %s ... ", trd_name(tread));
    m = (target == ISR_RULES)   /* depending on the target type */
      ? ib_readapp(ibase,tread) /* read the item appearances */
      : ib_readsel(ibase,tread);/* or a simple item selection */
    if (m < 0) error((int)-m, ib_errmsg(ibase, NULL, 0));
    trd_close(tread);           /* close the input file */
    MSG(stderr, "[%"ITEM_FMT" item(s)]", ib_cnt(ibase));
    MSG(stderr, " done [%.2fs].\n", SEC_SINCE(t));
  }                             /* print a log message */

  /* --- read transaction database --- */
  tabag = tbg_create(ibase);    /* create a transaction bag */
  if (!tabag) error(E_NOMEM);   /* to store the transactions */
  t = clock();                  /* start timer, open input file */
  if (trd_open(tread, NULL, fn_inp) != 0)
    error(E_FOPEN, trd_name(tread));
  MSG(stderr, "reading %s ... ", trd_name(tread));
  k = tbg_read(tabag, tread, mtar);
  if (k < 0) error(-k, tbg_errmsg(tabag, NULL, 0));
  trd_delete(tread, 1);         /* read the transaction database, */
  tread = NULL;                 /* then delete the table reader */
  m = ib_cnt(ibase);            /* get the number of items, */
  n = tbg_cnt(tabag);           /* the number of transactions, */
  w = tbg_wgt(tabag);           /* the total transaction weight */
  MSG(stderr, "[%"ITEM_FMT" item(s), %"TID_FMT, m, n);
  if (w != (SUPP)n) { MSG(stderr, "/%"SUPP_FMT, w); }
  MSG(stderr, " transaction(s)] done [%.2fs].", SEC_SINCE(t));
  if ((m <= 0) || (n <= 0))     /* check for at least one item */
    error(E_NOITEMS);           /* and at least one transaction */
  MSG(stderr, "\n");            /* terminate the log message */
  conf *= 0.01;                 /* scale the minimum confidence */
  supp = (supp >= 0) ? 0.01 *supp *(double)w *(1-DBL_EPSILON) : -supp;
  smax = (smax >= 0) ? 0.01 *smax *(double)w *(1+DBL_EPSILON) : -smax;
  body = (SUPP)ceilsupp(supp);  /* compute absolute support values */
  smin = (SUPP)ceilsupp((!(target & ISR_RULES) || orig)
       ? supp : ceilsupp(supp) *conf *(1-DBL_EPSILON));

  /* --- find frequent item sets/association rules --- */
  mode |= APR_VERBOSE|APR_NOCLEAN;
  k = apriori_data(tabag, target, smin, zmin, eval, mode, algo, sort);
  if (k) error(k);              /* prepare data for Apriori */
  report = isr_create(ibase);   /* create an item set reporter */
  if (!report) error(E_NOMEM);  /* and configure it */
  isr_setsize(report,        zmin, zmax);
  isr_setsupp(report, (RSUPP)smin, (RSUPP)floorsupp(smax));
  if (setbdr(report, w, zmin, &border, bdrcnt) != 0)
    error(E_NOMEM);             /* set limits and support border */
  if (fn_psp && (isr_addpsp(report, NULL) < 0))
    error(E_NOMEM);             /* set a pattern spectrum if req. */
  if (isr_setfmt(report, scan, hdr, sep, imp, info) != 0)
    error(E_NOMEM);             /* set the output format strings */
  k = isr_open(report, NULL, fn_out);
  if (k) error(k, isr_name(report)); /* open item set output file */
  if ((apriori_repo(report, target, eval, thresh, algo, mode) < 0)
  ||  (isr_setup(report) < 0))  /* prepare reporter for Apriori */
    error(E_NOMEM);             /* and set up the item set reporter */
  k = apriori(tabag, target, smin, body, conf, eval, agg, thresh,
              prune, algo, mode, filter, order, report);
  if (k) error(k);              /* find frequent item sets */
  if (stats)                    /* print item set statistics */
    isr_prstats(report, stdout, 0);
  if (isr_close(report) != 0)   /* close item set output file */
    error(E_FWRITE, isr_name(report));

  /* --- write pattern spectrum --- */
  if (fn_psp) {                 /* if to write a pattern spectrum */
    t = clock();                /* start timer, create table write */
    psp    = isr_getpsp(report);/* get the pattern spectrum */
    twrite = twr_create();      /* create a table writer and */
    if (!twrite) error(E_NOMEM);/* open the output file */
    if (twr_open(twrite, NULL, fn_psp) != 0)
      error(E_FOPEN,  twr_name(twrite));
    MSG(stderr, "writing %s ... ", twr_name(twrite));
    if (psp_report(psp, twrite, 1.0) != 0)
      error(E_FWRITE, twr_name(twrite));
    twr_delete(twrite, 1);      /* write the pattern spectrum */
    twrite = NULL;              /* and delete the table writer */
    MSG(stderr, "[%"SIZE_FMT" signature(s)]", psp_sigcnt(psp));
    MSG(stderr, " done [%.2fs].\n", SEC_SINCE(t));
  }                             /* write a log message */

  /* --- clean up --- */
  CLEANUP;                      /* clean up memory and close files */
  SHOWMEM;                      /* show (final) memory usage */
  return 0;                     /* return 'ok' */
}  /* main() */
//#else
int main (int argc, char *argv[])
{                               /* --- main function */
    int k = apriori_initialize(NULL);
    if(k < 0) {
        return 0;
    }
    addTransaction("transaction1.tab");
    addTransaction("transaction2.tab");
    addTransaction("transaction3.tab");
    addTransaction("transaction4.tab");
    addTransaction("transaction5.tab");
    addTransaction("transaction6.tab");
    addTransaction("transaction7.tab");
    addTransaction("transaction8.tab");
    addTransaction("transaction9.tab");
    addTransaction("transaction10.tab");
    apriori_start("test1.rules",80,20);
    return 0;
}
#endif
#endif  /* #ifdef APR_MAIN ... */
