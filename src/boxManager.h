#ifndef BOXMANAGER_H
#define BOXMANAGER_H
#include <iostream>
#include "ibex.h"

using namespace std;
using namespace ibex;

class boxManager{
    public:
        boxManager(IntervalVector dom, IntervalVector im);
        boxManager();
        double max_diam() const;
        double im_diam() const;
        IntervalVector getDomain() const;
        IntervalVector getImage() const;
        double getImageUB() const;
        void setImage(IntervalVector image);
        bool operator<(const boxManager dc) const;
        int bisect(Function &V, Function &hV, pair<boxManager,boxManager> &p);
        void no_checks_bisect(Function &V, Function &hV, pair<boxManager,boxManager> &p);
        boxManager dc_ub();
        friend ostream &operator<<(ostream &os, boxManager const &m);

    protected:
        static bool cluster_box(IntervalVector &Vg, IntervalVector &hVg);

        IntervalVector domain;
        IntervalVector image;
};

#endif //BOXMANAGER_H
