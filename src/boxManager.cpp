#include "boxManager.h"

using namespace std;
using namespace ibex;

boxManager::boxManager(IntervalVector dom, IntervalVector im){
    domain = dom;
    image = im;
}

boxManager::boxManager(){
    domain = IntervalVector(1);
    image = IntervalVector(2);
}

double boxManager::max_diam() const{
    return domain.max_diam();
}

double boxManager::im_diam() const{
    return image.max_diam();
}

IntervalVector boxManager::getDomain() const{
    return domain;
}

IntervalVector boxManager::getImage() const{
    return image;
}

double boxManager::getImageUB() const{
    return image[0].ub();
}

void boxManager::setImage(IntervalVector image){
    this->image = image;
}

bool boxManager::operator<(const boxManager dc) const{
    return this->image[0].lb() < dc.image[0].lb();
}

int boxManager::bisect(Function &V, Function &hV, pair<boxManager,boxManager> &p){
    pair<IntervalVector,IntervalVector> bd =
        this->domain.bisect(domain.extr_diam_index(false));

    IntervalVector im1(2), im2(2);

    int rej = 0;

    // first boxManager
    IntervalVector Vg = V.gradient(bd.first);
    IntervalVector hVg = hV.gradient(bd.first);

    if(cluster_box(Vg,hVg)){
        rej++;
    }
    else{
        im1[0] = V.eval(bd.first)
                    & ( V.eval( IntervalVector(bd.first.mid()) )
                    + Vg * ( bd.first - bd.first.mid() ) )
                    & this->image[0];
        im1[1] = hV.eval(bd.first)
                    & ( hV.eval( IntervalVector(bd.first.mid()) )
                    + hVg * ( bd.first - bd.first.mid() ) );
    }

    // second boxManager
    Vg = V.gradient(bd.second);
    hVg = hV.gradient(bd.second);

    if(cluster_box(Vg,hVg)){
        rej = rej + 2;
    }
    else{
        im2[0] = V.eval(bd.second)
                    & ( V.eval( IntervalVector(bd.second.mid()) )
                    + Vg * ( bd.second - bd.second.mid() ) )
                    & this->image[0];
        im2[1] = hV.eval(bd.second)
                    & ( hV.eval( IntervalVector(bd.second.mid()) )
                    + hVg * ( bd.second - bd.second.mid() ) );
    }

    boxManager dc1(bd.first, im1), dc2(bd.second, im2);

    p = pair<boxManager,boxManager> (dc1,dc2);

    return rej;
}

void boxManager::no_checks_bisect(Function &V, Function &hV, pair<boxManager,boxManager> &p){
    pair<IntervalVector,IntervalVector> bd =
        this->domain.bisect(domain.extr_diam_index(false));

    IntervalVector im1(2), im2(2);

    // first boxManager
    IntervalVector Vg = V.gradient(bd.first);
    IntervalVector hVg = hV.gradient(bd.first);


    im1[0] = V.eval(bd.first)
                & ( V.eval( IntervalVector(bd.first.mid()) )
                + Vg * ( bd.first - bd.first.mid() ) );
    im1[1] = hV.eval(bd.first)
                & ( hV.eval( IntervalVector(bd.first.mid()) )
                + hVg * ( bd.first - bd.first.mid() ) );

    // second boxManager
    Vg = V.gradient(bd.second);
    hVg = hV.gradient(bd.second);

    im2[0] = V.eval(bd.second)
                & ( V.eval( IntervalVector(bd.second.mid()) )
                + Vg * ( bd.second - bd.second.mid() ) );
    im2[1] = hV.eval(bd.second)
                & ( hV.eval( IntervalVector(bd.second.mid()) )
                + hVg * ( bd.second - bd.second.mid() ) );

    boxManager dc1(bd.first, im1), dc2(bd.second, im2);

    p = pair<boxManager,boxManager> (dc1,dc2);
}

boxManager boxManager::dc_ub(){
    IntervalVector tim = image;
    tim[0] = Interval(image[0].ub());

    return boxManager(domain, tim);
}

ostream& operator<<(ostream &os, boxManager const &m) {
    return os << m.domain << " : " << m.image;
}

bool boxManager::cluster_box(IntervalVector &Vg, IntervalVector &hVg){
    int n = Vg.size(), r = n, i;
    bool f;

    for(i = 0; i < n - 1; i++){
        if(!Vg[i].contains(0) || !hVg[0].contains(0)){
            r = i;
            f = !Vg[i].contains(0);
            break;
        }
    }

    for(i = r + 1; i < n; i++){
        Interval G = (f ? hVg[i] - Vg[i]*hVg[r]/Vg[r] : Vg[i] - hVg[i]*Vg[r]/hVg[r]);
        if(!G.contains(0)){
            return true;
        }
    }

    return false;
}
