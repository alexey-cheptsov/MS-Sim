/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G2_2_PROXIES_H
#define G2_2_PROXIES_H

#include "../G1_2/ports.h"
#include "../A1_2/ports.h"

namespace G2_2 {

    class Proxies_qmt {
    public:

	enum Port_codes {
            command_flow        = G1_2::Ports_qmt::command_flow,
	    
	    set_q               = G1_2::Ports_qmt::set_q,
            get_q               = G1_2::Ports_qmt::get_q,
            set_qm              = G1_2::Ports_qmt::set_qm,
            get_qm              = G1_2::Ports_qmt::get_qm,
	    set_qs              = G1_2::Ports_qmt::set_qs,
            get_qs              = G1_2::Ports_qmt::get_qs,
            set_qms	        = G1_2::Ports_qmt::set_qms,
            get_qms	        = G1_2::Ports_qmt::get_qms,
            set_r_reg	        = G1_2::Ports_qmt::set_r_reg,
            get_r_reg	        = G1_2::Ports_qmt::get_r_reg            
	};
    };

    class Proxies_p {
    public:

	enum Port_codes {
	    command_flow        = A1_2::Ports_p::command_flow + Proxies_qmt::get_r_reg+1 /*displ*/,

            set_p               = A1_2::Ports_p::set_p        + Proxies_qmt::get_r_reg+1 /*displ*/,
            get_p               = A1_2::Ports_p::get_p        + Proxies_qmt::get_r_reg+1 /*displ*/
	};
    };
}; // namespace G2_2

#endif // G2_2_PROXIES