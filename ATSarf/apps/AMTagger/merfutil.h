#ifndef MERFUTIL_H
#define MERFUTIL_H

typedef enum {
    NONE,
    OR,
    AND,
    UPTO,
    STAR,
    PLUS,
    SEQUENCE,
    KUESTION, // Can't use the Q due to Qt platform
    KEY
} Operation;
#endif // MERFUTIL_H
