/*!
file: dbitvec.cpp
date: mar 3, 2010
author: fadi 

brief: drives and tests dbitvec
  */

#include <dbitvec.h>
#include <iostream>
#include <vector>

using namespace std;

typedef vector< dbitvec> vecbitvec;

int
main() {
    try{
        dbitvec x(12);

        cout  << "expect 0: " << x.getBit(1) << endl;
        x.setBit(1);
        cout  << "expect 1: " << x.getBit(1) << endl;
        cout  << "expect 1: " << x[1] << endl;

        x.setBit(1,false);
        cout  << "expect 0: " << x[1] << endl;

        cout  << "expect 0: " << x.getBit(2) << endl;
        cout  << "expect 0: " << x[2] << endl;
        x[3] = true;
        cout  << "expect 1: " << x.getBit(3) << endl;
        cout  << "expect 1: " << x[3] << endl;

        cout  << x.getBit(15) << endl;
    } catch (dbitvec::exception_em e) {
        cout << "met exception " << e << endl;
    }

    vecbitvec xx;
    for (int i = 0; i < 10; i++) {
        // note that push_back will create a copy of the passed
        // object, so if we allocate in the constructor
        // we will be allocating twice
        xx.push_back( dbitvec() );
        xx[i].resize(15);
    }

    xx[8][12] = true;
    xx[1][1] = false;
    cout << "expect 1 0 0: " << xx[8][12] << " " <<  xx[1][1] << " " << xx[2][2] << endl;

    xx[8][12] = false;
    cout << "expect 0 0 0: " << xx[8][12] << " " <<  xx[1][1] << " " << xx[2][2] << endl;
    return 0;
}
