#ifndef PRE_DIRECTIVES_H
#define PRE_DIRECTIVES_H

#define IDMAPFN
#ifndef TATREEFN
    #define TATREEFN
#endif
#define APR_MAIN
#ifdef  APR_MAIN
    #ifndef PSP_REPORT
        #define PSP_REPORT
    #endif
    #ifndef TA_READ
        #define TA_READ
    #endif
#endif
#ifndef ISR_PATSPEC
    #define ISR_PATSPEC
#endif

#endif // PRE_DIRECTIVES_H
