/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G2_1_PROXIES_H
#define G2_1_PROXIES_H

#include "../G1_1/ports.h"
#include "../G1_2/ports.h"
#include "../A1_2/ports.h"

namespace G2_1 {

    class Proxies_qm {
    public:

	enum Port_codes {
            command_flow        = G1_1::Ports_qm::command_flow,
	    
	    set_q               = G1_1::Ports_qm::set_q,
            get_q               = G1_1::Ports_qm::get_q,
	    set_qm              = G1_1::Ports_qm::set_qm,
            get_qm              = G1_1::Ports_qm::get_qm,
            set_qm0	        = G1_1::Ports_qm::set_qm0,
            get_qm0	        = G1_1::Ports_qm::get_qm0
	};
    };

    class Proxies_qmt {
    public:

	enum Port_codes {
            command_flow        = G1_2::Ports_qmt::command_flow  + Proxies_qm::get_qm0+1 /*displ*/,
	    
	    set_q               = G1_2::Ports_qmt::set_q         + Proxies_qm::get_qm0+1 /*displ*/,
            get_q               = G1_2::Ports_qmt::get_q         + Proxies_qm::get_qm0+1 /*displ*/,
	    set_qm              = G1_2::Ports_qmt::set_qm        + Proxies_qm::get_qm0+1 /*displ*/,
            get_qm              = G1_2::Ports_qmt::get_qm        + Proxies_qm::get_qm0+1 /*displ*/,
	};
    };

    class Proxies_p {
    public:

	enum Port_codes {
	    command_flow        = A1_2::Ports_p::command_flow + Proxies_qmt::get_qm+1 /*displ*/,

            set_p               = A1_2::Ports_p::set_p        + Proxies_qmt::get_qm+1 /*displ*/,
            get_p               = A1_2::Ports_p::get_p        + Proxies_qmt::get_qm+1 /*displ*/
	};
    };
}; // namespace G2_1

#endif // G2_1_PROXIES