#include "ibex.h"
#include "boxManager.h"
#include "ibex_P_StructNSLyap.h"
#include "utilities.h"
#include "debug.h"
#include <iostream>
#include <typeinfo>
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace ibex;

int main(int argc, char** argv) {
	// setup
    /* command line arguments:
        -r=[NUMBER}: length of the initial box' side.
        -t=[NUMBER}: tolerance.
        -v=[NUMBER]: verbose level [0-3].
    */
    double rad = 4;
    double tol = 0.05;
    debug d1(true), d2(false), d3(false);
    bool invariant = false, light_save = false, box_rejection = true;

    if(argc < 2 || strcmp(argv[1],"-help") == 0){
        cerr << "usage:\n\t./bnb_solver <system_name> [-r=(side length)] [-t=(tolerance)] [-v=(0-3, verboseness)] [-l (saves to text file instead of .cov)] [-c (disables box rejection)]";
		cerr << endl << endl;
		cerr << "example:\n\t./bnb_solver Ball2D_1 -r=6 -t=0.05 -v=1 -l -c";
		cerr << endl;
		exit(0);
    }

    if(argc > 2){
        for(int i = 2; i < argc; i++){
            string sarg = string(argv[i]);
            if(sarg == "-c"){
                box_rejection = false;
            }
            else if(sarg == "-l"){
                light_save = true;
            }
            else if(sarg.substr(0,3) == "-v="){
                int verbose = stoi(sarg.substr(3));
                switch(verbose){
                    case 0:
                        d1.setState(false);
                        break;
                    case 3:
                        d3.setState(true);
                    case 2:
                        d2.setState(true);
                }
            }
            else if(sarg.substr(0,3) == "-r="){
                rad = stod(sarg.substr(3));
            }
            else if(sarg.substr(0,3) == "-t="){
                tol = stod(sarg.substr(3));
            }
        }
    }

    string prefix = "../systems/";

    stringstream s;
	s << prefix << argv[1] << ".txt";

    Function f, V;

	try {
		P_StructNSLyap struc(s.str().c_str());
        parser::init_function_by_copy(f,struc.f);
        parser::init_function_by_copy(V,struc.V);
    }
    catch(UnknownFileException& e) {
		ibex_error("file not found");
	} catch(SyntaxError& e) {
		cout << e << endl;
	}

    d2 << "Vector field: " << f << endl;
    d2 << "Lyapunov function: " << V << endl;

    int n = V.args()[0].dim.vec_size();
    Variable z(n, "z");
    Function dV(V, Function::DIFF);

    const ExprNode* e_hV = &(dV[0](z)*f[0](z));

    for(int i = 1; i < n; i++){
        e_hV = &(*e_hV + dV[i](z)*f[i](z));
    }

    Function hV(z, *e_hV, "dotV");

    d2 << "Lie derivative: " << hV << endl;

    double _xinit[n][2];

    for(int i = 0; i < n; i++){
        _xinit[i][0] = -rad;
        _xinit[i][1] = rad;
    }

    IntervalVector xinit(n, _xinit);

    // evaluate minimum over the initial box boundary
    SystemFactory opt_fac;
    opt_fac.add_var(V.args()[0]);
    // minimize V(x)
    opt_fac.add_goal(V);
    System opt_sys(opt_fac);
    // build the default optimizer
    DefaultOptimizer optimizer(opt_sys,1e-05);

    // run it on each border of the initial box, keep the minimum
    vector<double> mins;
    for(int i = 0; i < n; i++){
        optimizer.optimize(squish(_xinit, i, n));
        mins.push_back(optimizer.get_loup());
    }

    double alpha = *min_element(mins.begin(), mins.end());

    d2 << "Minimimum over the boundary: " << alpha << endl;

    CovIBUList xcov(n), Vcov(2); //final result
    multiset<boxManager> result;

    IntervalVector image(2);

    image[0] = V.eval(xinit)
                & ( V.eval( IntervalVector(xinit.mid()) )
                + V.gradient(xinit) * ( xinit - xinit.mid() ) );
    image[1] = hV.eval(xinit)
                & ( hV.eval( IntervalVector(xinit.mid()) )
                + hV.gradient(xinit) * ( xinit - xinit.mid() ) );

    boxManager dcinit(xinit,image);

    multiset<boxManager> boxes; // boxes are ordered by lower bound of [V(z)]
    multiset<boxObserver> obs; // boxes are ordered by max_diam of the domain; only pointers are kept

    obs.insert(boxObserver(boxes.insert(dcinit)));

    int discarted = 0;
    int total_boxes = 0;
    Interval c_m(0,0);
    bool init_cm = true;

    d1 << "Starting evalution in " << (light_save ? "light save" : "COV save") << " mode." << endl;
    d1 << "Box rejection " << (box_rejection ? "enabled." : "disabled.") << endl;
    d1 << "Evaluation started.." << endl;

    auto start = high_resolution_clock::now();

    if(box_rejection){
        while(!boxes.empty()) {
            multiset<boxObserver>::iterator obsIter = --obs.end();
            boxObserver localobs = *(obsIter);
            boxManager localdc = *(localobs.getIterator());

            obs.erase(obsIter);
            boxes.erase(localobs.getIterator());

            if(localdc.max_diam() > tol){
                pair<boxManager,boxManager> p;

                int rej = localdc.bisect(V, hV, p);

                bool f = (rej % 2 == 0) ? reduce(p.first, boxes, alpha) : false;
                bool s = (rej < 2) ? reduce(p.second, boxes, alpha) : false;

                if(f){
                    obs.insert(boxObserver(boxes.insert(p.first)));
                }
                else{
                    discarted++;
                }
                if(s){
                    obs.insert(boxObserver(boxes.insert(p.second)));
                }
                else{
                    discarted++;
                }
            }
            else {
                IntervalVector localimage = localdc.getImage();
                total_boxes++;

                if(localimage[0].contains(alpha)){
                    if(!light_save){
                        xcov.add_boundary(localdc.getDomain());
                        Vcov.add_boundary(localimage);
                    }
                    else{
                        result.insert(localdc);
                    }
                    if (localimage[1].ub() < 0){ // could be moved down?
                        invariant = true;
                    }
                }
                else if(localimage[0].ub() < alpha){
                    if(!light_save){
                        xcov.add_inner(localdc.getDomain());
                        Vcov.add_inner(localimage);
                    }
                    else{
                        result.insert(localdc);
                    }
                    if(localimage[1].contains(0)){
                        if(init_cm){
                            c_m = localimage[0];
                            init_cm = false;
                        }
                        else{
                            c_m |= localimage[0];
                        }
                    }
                }
                else{
                    xcov.add(localdc.getDomain());
                    Vcov.add(localimage);
                }
            }
        }
    }
    else {
        while(!boxes.empty()) {
            multiset<boxObserver>::iterator obsIter = --obs.end();
            boxObserver localobs = *(obsIter);
            boxManager localdc = *(localobs.getIterator());

            obs.erase(obsIter);
            boxes.erase(localobs.getIterator());

            if(localdc.max_diam() > tol){
                pair<boxManager,boxManager> p;

                localdc.no_checks_bisect(V, hV, p);

                obs.insert(boxObserver(boxes.insert(p.first)));
                obs.insert(boxObserver(boxes.insert(p.second)));
            }
            else {
                total_boxes++;

                if(!light_save){
                    xcov.add_inner(localdc.getDomain());
                    Vcov.add_inner(localdc.getImage());
                }
                else{
                    result.insert(localdc);
                }
            }
        }
    }


    auto stop = high_resolution_clock::now();

    // duration can be nano, micro, milli or seconds
    auto duration = duration_cast<milliseconds>(stop - start);

    d1 << "Ended. Time: " << duration.count() << " milliseconds." << endl;

    if(!light_save){
        string save_prefix = "../res/";

        stringstream sx, sv;
    	sx << save_prefix << "xcov." << argv[1] << ".cov";
    	sv << save_prefix << "Vcov." << argv[1] << ".cov";

        xcov.save(sx.str().c_str());
        Vcov.save(sv.str().c_str());
    }
    else{
        stringstream sb;
    	sb << "../res/" << argv[1] << ".math";
        save_data(sb.str().c_str(), result);
    }

    if(box_rejection){
        d2 << "Discarted boxes: " << discarted << endl;
    }
    d1 << "Total boxes: " << total_boxes << endl;

    string inv_message = invariant ? "Possible invariant set found." : "No invariant sets found.";
    if(box_rejection){
        d1 << inv_message << endl;
        d2 << "Zero crossing in " << c_m << endl;
    }

	return 0;
}
