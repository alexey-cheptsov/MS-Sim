/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/*
 * Euler.h
 *
 *  Created on: Aug 8, 2017
 *      Author: hpcochep
 */

#ifndef SRC_NUMERICS_EULER_H_
#define SRC_NUMERICS_EULER_H_

#include "Solver.h"
#include <cmath> // fabs


class Euler: public Solver {
public:

	Euler(Solver_Params& params_) :
			Solver(params_) {
	}

	bool solve(float* lvalue, float const rvalue) {

		float is_converged = false;
		float lvalue_next = (*lvalue) + (h) * (rvalue);
		
		if (fabs(lvalue_next - *lvalue) < p )
		    is_converged = true;
		    
		*lvalue = lvalue_next;    
		
		return is_converged;

	}

};

#endif /* SRC_NUMERICS_EULER_H_ */
