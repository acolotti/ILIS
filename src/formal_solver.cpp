#include "ibex.h"
#include "ibex_P_StructNSLyap.h"
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
        -r=[NUMBER}: define radius of initial box
        -t=[NUMBER}: define tolerance of algorithm
        -v=[NUMBER]: define verbose level
    */
    double rad = 10;
    debug d1(true), d2(false), d3(false);
    bool invariant = false, light_save = false;

    if(argc < 2 || strcmp(argv[1],"-help") == 0){
        cerr << "usage:\n\t./formal_solve <system_name> [-v=(0-3, verboseness)] [-l (for light save)]";
		cerr << endl << endl;
		cerr << "example:\n\t./formal_solve Ball2D_1 -v=1 -l";
		cerr << endl;
		exit(0);
    }

    if(argc > 2){
        for(int i = 2; i < argc; i++){
            string sarg = string(argv[i]);
            if(sarg == "-l"){
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

    double _xinit[n+2][2];

    // system's state box
    for(int i = 0; i < n; i++){
        _xinit[i][0] = -rad;
        _xinit[i][1] = rad;
    }
    // lambda box
    _xinit[n][0] = -rad;
    _xinit[n][1] = rad;
    // radius box
    _xinit[n+1][0] = 0;
    _xinit[n+1][1] = 2000;

    IntervalVector xinit(n+2, _xinit);

    Variable l("l"), r("r");

    // solve system over the initial box boundary
    SystemFactory solve_fac;
    solve_fac.add_var(z);
    solve_fac.add_var(l);
    solve_fac.add_var(r);

    // create system to solve
    Function dhV(hV, Function::DIFF);

    d2 << "Gradient of V(x): " << dV << endl;
    d2 << "Gradient of hV(x): " << dhV << endl;

    solve_fac.add_ctr_eq(dV(z) + l*dhV(z));
    solve_fac.add_ctr_eq(V(z) - r);
    solve_fac.add_ctr_eq(hV(z));


    System solve_sys(solve_fac);
    // build the default optimizer
    DefaultSolver solver(solve_sys,1e-07);

    auto start = high_resolution_clock::now();

    solver.solve(xinit); // Run the solver

    auto stop = high_resolution_clock::now();

    // milliseconds can be nano, micro, milli or seconds
    auto duration = duration_cast<milliseconds>(stop - start);

    d1 << "Ended. Time: " << duration.count() << " milliseconds." << endl;

    /* Display the solutions. */
	d1 << "Solutions: " << solver.get_data() << endl;

    /*
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
    }*/

	return 0;
}
