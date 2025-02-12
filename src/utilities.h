//============================================================================
//                                  I B E X
// File        : ibex_P_StructBubibex.h
// Authors     : Gilles Chabert
// Copyright   : IMT Atlantique
// License     : See the LICENSE file
// Created     : Nov 05, 2019
//============================================================================

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <iostream>
#include <fstream>
#include "ibex.h"
#include "boxManager.h"

using namespace std;
using namespace ibex;

class boxObserver{
    public:
        boxObserver(multiset<boxManager>::iterator it);
        multiset<boxManager>::iterator getIterator() const;
        bool operator<(const boxObserver dco) const;

    protected:
        double diam;
        multiset<boxManager>::iterator it;
};

bool reduce(boxManager &dc, const multiset<boxManager> &boxes, double alpha);
IntervalVector squish(double bounds[][2], int i, int n);
void save_data(const char *fname, const multiset<boxManager> &result);

#endif
