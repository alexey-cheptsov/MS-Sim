/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef A2_1_PROXIES_H
#define A2_1_PROXIES_H

#include "../A1_1/ports.h"
#include "../A1_2/ports.h"

namespace A2_1 {

    class Proxies_q {
    public:

	enum Port_codes {
            command_flow        = A1_1::Ports_q::command_flow,
	    
	    set_q               = A1_1::Ports_q::set_q,
            get_q               = A1_1::Ports_q::get_q,
	    set_qs              = A1_1::Ports_q::set_qs,
            get_qs              = A1_1::Ports_q::get_qs,
            set_r_reg           = A1_1::Ports_q::set_r_reg,
            get_r_reg           = A1_1::Ports_q::get_r_reg
	};
    };

    class Proxies_p {
    public:

	enum Port_codes {
	    command_flow        = A1_2::Ports_p::command_flow + Proxies_q::get_r_reg+1 /*displ*/,

            set_p               = A1_2::Ports_p::set_p        + Proxies_q::get_r_reg+1 /*displ*/,
            get_p               = A1_2::Ports_p::get_p        + Proxies_q::get_r_reg+1 /*displ*/,
	};
    };
}; // namespace A2_1

#endif // A2_1_PROXIES