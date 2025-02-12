#include "utilities.h"

using namespace std;
using namespace ibex;

boxObserver::boxObserver(multiset<boxManager>::iterator it){
    this->diam = it->max_diam();
    this->it = it;
}

multiset<boxManager>::iterator boxObserver::getIterator() const{
    return it;
}

bool boxObserver::operator<(const boxObserver dco) const{
    return this->diam < dco.diam;
}

bool reduce(boxManager &dc, const multiset<boxManager> &boxes, double alpha){
    IntervalVector dc_im = dc.getImage();
    Interval dc_V = dc_im[0];

    if(dc_V.is_empty() || dc_V.lb() > alpha){
        return false;
    }

    multiset<boxManager>::const_iterator iter = boxes.upper_bound(dc.dc_ub());
    multiset<boxManager>::const_iterator beg = boxes.begin();

    if(iter == beg){
        return true;
    }

    while(iter-- != beg && iter->getImageUB() > dc_V.lb()){
        IntervalVector tmp_im = iter->getImage();

        if(dc_im[1].ub() < tmp_im[1].lb()
            && dc_V.overlaps(tmp_im[0])
            && !tmp_im[0].is_strict_interior_subset(dc_V)){

            Interval inter = dc_V & tmp_im[0];
            if(inter != dc_V){
                dc_V = (inter.lb() == dc_V.lb()) ?
                        Interval(inter.ub(), dc_V.ub()) : Interval(dc_V.lb(), inter.lb());
            }
            else{
                return false;
            }
        }
        if(dc_V.lb() > alpha){
            return false;
        }
    }

    dc_im[0] = dc_V;
    dc.setImage(dc_im);

    return true;
}

IntervalVector squish(double bounds[][2], int i, int n){
    double tmp_st = bounds[i][0];
    bounds[i][0] = bounds[i][1];
    IntervalVector squished(n, bounds);
    bounds[i][0] = tmp_st;
    return squished;
}

void save_data(const char *fname, const multiset<boxManager> &result){
    // initialize text
    stringstream sx, sv;
    sx << "px = {";
    sv << "pv = {";

    multiset<boxManager>::const_iterator it = result.begin();

    Vector mx = it->getDomain().mid(), mv = it->getImage().mid();
    int n = mx.size();
    sx << "{";
    for(int i = 0; i < n-1; i++){
        sx << mx[i] << ",";
    }
    sx << mx[n-1] << "}";
    sv << "{" << mv[0] << "," << mv[1] << "}";
    it++;

    for(it; it != result.end(); it++){
        Vector mx = it->getDomain().mid(), mv = it->getImage().mid();
        sx << ",{";
        for(int i = 0; i < mx.size()-1; i++){
            sx << mx[i] << ",";
        }
        sx << mx[n-1] << "}";
        sv << ",{" << mv[0] << "," << mv[1] << "}";
    }

    sx << "};";
    sv << "};";

    ofstream myfile;
    myfile.open (fname);
    myfile << sx.str() << endl << sv.str();
    myfile.close();
}
