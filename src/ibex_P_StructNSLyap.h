//============================================================================
//                                  I B E X
// File        : ibex_P_StructBubibex.h
// Authors     : Gilles Chabert
// Copyright   : IMT Atlantique
// License     : See the LICENSE file
// Created     : Nov 05, 2019
//============================================================================

#ifndef _IBEX_PARSER_STRUCT_NSLYAP_H_
#define _IBEX_PARSER_STRUCT_NSLYAP_H_

#include "ibex_P_Struct.h"
#include "ibex_System.h"

namespace ibex {

/**
 * \ingroup parser
 *
 * \brief Lyapunov parsing
 */
class P_StructNSLyap : public parser::P_Struct {
public:

	/**
	 * Load the structure for the Lyapunov plugin
	 *
	 */
	P_StructNSLyap(const char* filename);

	/**
	 * Initialize parsing of a package.
	 */
	virtual void begin();

	/**
	 * End parsing of a package.
	 */
	virtual void end();

	/**
	 * Function "f" of the vector field to be built.
	 */
	//Function f;

	/**
	 * Lyapunov function "v"
	 */
	Function V;

	/**
	 * Vector field "f"
	 */
	Function f;
};

} // end namespace ibex

#endif
