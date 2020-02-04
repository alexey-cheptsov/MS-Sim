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

#ifndef SRC_NUMERICS_RK4_H_
#define SRC_NUMERICS_RK4_H_

#include "Solver.h"

class RK4: public Solver {
public:

	RK4(Solver_Params& params_) :
                        Solver(params_) {
        }


	bool solve(float* lvalue, float const k1, float const k2, float const k3, float const k4) {

		float is_converged = false;
		
		float lvalue_next = (*lvalue) + (h/6) * (k1 + 2*k2 + 2*k3 + k4);
		
		if (fabs(lvalue_next - *lvalue) < p )
		    is_converged = true;
		    
		*lvalue = lvalue_next;    
		
		return is_converged;
	}
	
	bool solve(float* lvalue, float rvalue) {

                float is_converged = false;
                
                float lvalue_next = (*lvalue) + h * rvalue;

                if (fabs(lvalue_next - *lvalue) < p )
                    is_converged = true;

                *lvalue = lvalue_next;

                return is_converged;
        }


};

#endif /* SRC_NUMERICS_EULER_H_ */
