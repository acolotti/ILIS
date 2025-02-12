//============================================================================
//                                  I B E X
// File        : ibex_P_StructBubibex.h
// Authors     : Gilles Chabert, Alessandro Colotti
// Copyright   : IMT Atlantique
// License     : See the LICENSE file
// Created     : Nov 05, 2019
// Modified		 : Oct 02, 2020
//============================================================================

#include "ibex_P_StructNSLyap.h"
#include "ibex_P_SysGenerator.h"
#include "ibex_SyntaxError.h"
#include "ibex_UnknownFileException.h"

#include <string.h>

#ifndef _WIN32 // MinGW does not support mutex
#include <mutex>
namespace {
std::mutex mtx;
}
#define LOCK mtx.lock()
#define UNLOCK mtx.unlock()
#else
#define LOCK
#define UNLOCK
#endif

extern int ibexparse();
extern void ibexrestart(FILE *);
extern FILE* ibexin;

using namespace std;

namespace ibex {

P_StructNSLyap::P_StructNSLyap(const char* filename){
	FILE* fd = fopen(filename, "r");

	if (fd == NULL) throw UnknownFileException(filename);

	LOCK;

	ibexin = fd;

	try {
		parser::pstruct = this;
		ibexparse();
		parser::pstruct = NULL;
	}

	catch(SyntaxError& e) {
		parser::pstruct = NULL;
		fclose(fd);
		ibexrestart(ibexin);
		UNLOCK;
		throw e;
	}

	fclose(fd);

	UNLOCK;
}

void P_StructNSLyap::begin() {
	P_Struct::begin();
}

void P_StructNSLyap::end() {

	for (vector<Function*>::iterator it = source.func.begin(); it!=source.func.end(); ++it) {
		if (strcmp((*it)->name,"V")==0)
			parser::init_function_by_copy(V,**it);
		else if (strcmp((*it)->name,"f")==0)
			parser::init_function_by_copy(f,**it);
	}

	if (V.name==NULL)
		throw SyntaxError("input file must contain a function named \"V\"");
	if (f.name==NULL)
		throw SyntaxError("input file must contain a function named \"f\"");

	/* if x was declared as a constant... */
	//	bool x_cst_found=false;
	//	for (vector<const char*>::const_iterator it = scopes.top().cst.begin(); it !=scopes.top().cst.end(); ++it) {
	//
	//		if (strcmp(*it,"x")) {
	//			Domain* d= new Domain(scopes.top().get_cst(*it).get());
	//			switch (d->dim.type()) {
	//			case Dim::SCALAR :
	//				xhat.resize(1);
	//				xhat[0]=d->i().lb();
	//				break;
	//			case Dim::ROW_VECTOR:
	//			case Dim::COL_VECTOR:
	//				xhat.resize(d->dim.vec_size());
	//				xhat = d->v().lb();
	//				break;
	//			case Dim::MATRIX:
	//				throw SyntaxError("unexpected matrix constant \"x\"");
	//			}
	//			x_cst_found = true;
	//		}
	//	}
	//	if (!x_cst_found)
	//		throw SyntaxError("input file must contain a constant named \"x\"");

	P_Struct::end();
}

} // end namespace ibex
