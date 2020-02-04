/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/*
 * euler.h
 *
 *  Created on: Jul 27, 2017
 *      Author: hpcochep
 */

#ifndef SRC_NUMERICS_SOLVER_H_
#define SRC_NUMERICS_SOLVER_H_

struct Solver_Params {
    float time_step;
    float precision;
    float nr_num_steps;
};

class Solver {
public:

	float h; // numeric integration step in secs.
	float p; // precison
	float nr_num_steps;

	Solver(Solver_Params& params_) : h(params_.time_step),p(params_.precision),nr_num_steps(params_.nr_num_steps) {
	};

	virtual bool solve(float* lvalue, float const rvalue) { // returns true if method converged
	};
	
	virtual bool solve(float* lvalue, float const k1, float const k2, float const k3, float const k4)  {
	};

};

#endif /* SRC_NUMERICS_SOLVER_H_ */
