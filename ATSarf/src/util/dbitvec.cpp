/*!
file: dbitvec.h
date: mar 3, 2010
author: fadi

brief: provides a bitvector with [] access and dynamic size

  */
#include "dbitvec.h"
#include <string.h> // for memset


void dbitvec::resize(unsigned int length)  {
    unsigned int newbytes = (length + 7) >> 3;
    unsigned char *newdata =  new unsigned char [newbytes];

    if (newdata == NULL) {
        throw MEM_EXCPT;
    }

    if (size == 0) {
        size = length;
        bytes = newbytes;
        data = newdata;
        reset();
        return;
    }

    memset(newdata, 0, newbytes);
    memcpy(newdata, data, (bytes < newbytes) ? bytes : newbytes);
    size = length;
    bytes = newbytes;
    delete [] data;
    data = newdata;
}

bool dbitvec::NothingSet() {
    dbitvec temp(length());
    return *this == temp;
}

void dbitvec::show() {}

