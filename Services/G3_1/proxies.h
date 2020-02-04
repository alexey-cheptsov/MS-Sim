/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G3_1_PROXIES_H
#define G3_1_PROXIES_H

#include "../A2_1/ports.h" // Q
#include "../G2_1/ports.h" // Qm
#include "../G2_2/ports.h" // Qmt
#include "../A1_2/ports.h" // P

namespace G3_1 {

    class Proxies_OS {
    public:

	enum Port_codes {
            command_flow        = A2_1::Ports_Q::command_flow,
	    
	    set_Q               = A2_1::Ports_Q::set_Q,
            get_Q               = A2_1::Ports_Q::get_Q,
	    set_Qs              = A2_1::Ports_Q::set_Qs,
            get_Qs              = A2_1::Ports_Q::get_Qs,
            set_P	        = A2_1::Ports_Q::set_P,
            get_P	        = A2_1::Ports_Q::get_P,
            set_R_reg	        = A2_1::Ports_Q::set_R_reg,
            get_R_reg	        = A2_1::Ports_Q::get_R_reg            
	};
    };

    class Proxies_Streb {
    public:

	enum Port_codes {
            command_flow        = A2_1::Ports_Q::command_flow	+ Proxies_OS::get_R_reg+1,
	    
	    set_Q               = A2_1::Ports_Q::set_Q 		+ Proxies_OS::get_R_reg+1,
            get_Q               = A2_1::Ports_Q::get_Q 		+ Proxies_OS::get_R_reg+1,
	    set_Qs              = A2_1::Ports_Q::set_Qs 	+ Proxies_OS::get_R_reg+1,
            get_Qs              = A2_1::Ports_Q::get_Qs 	+ Proxies_OS::get_R_reg+1, 
            set_P	        = A2_1::Ports_Q::set_P 		+ Proxies_OS::get_R_reg+1,
            get_P	        = A2_1::Ports_Q::get_P 		+ Proxies_OS::get_R_reg+1,
            set_R_reg	        = A2_1::Ports_Q::set_R_reg 	+ Proxies_OS::get_R_reg+1,
            get_R_reg	        = A2_1::Ports_Q::get_R_reg 	+ Proxies_OS::get_R_reg+1,           
	};
    };

    class Proxies_AM {
    public:

	enum Port_codes {
            command_flow        = G2_1::Ports_Qm::command_flow 	 + Proxies_Streb::get_R_reg+1,
	    
	    set_Q               = G2_1::Ports_Qm::set_Q		 + Proxies_Streb::get_R_reg+1,
            get_Q               = G2_1::Ports_Qm::get_Q	 	 + Proxies_Streb::get_R_reg+1,
	    set_Qm              = G2_1::Ports_Qm::set_Qm 	 + Proxies_Streb::get_R_reg+1,
            get_Qm              = G2_1::Ports_Qm::get_Qm  	 + Proxies_Streb::get_R_reg+1,
            set_Qm0             = G2_1::Ports_Qm::set_Qm0  	 + Proxies_Streb::get_R_reg+1,
            get_Qm0             = G2_1::Ports_Qm::get_Qm0  	 + Proxies_Streb::get_R_reg+1,
            set_P	        = G2_1::Ports_Qm::set_P  	 + Proxies_Streb::get_R_reg+1,
            get_P	        = G2_1::Ports_Qm::get_P  	 + Proxies_Streb::get_R_reg+1,
	};
    };

    class Proxies_VS {
    public:

	enum Port_codes {
            command_flow        = G2_2::Ports_Qmt::command_flow 	+ Proxies_AM::get_P+1,
	    
	    set_Q               = G2_2::Ports_Qmt::set_Q		+ Proxies_AM::get_P+1,
            get_Q               = G2_2::Ports_Qmt::get_Q	 	+ Proxies_AM::get_P+1,
	    set_Qm              = G2_2::Ports_Qmt::set_Qm 	 	+ Proxies_AM::get_P+1,
            get_Qm              = G2_2::Ports_Qmt::get_Qm  	 	+ Proxies_AM::get_P+1,
            set_Qs              = G2_2::Ports_Qmt::set_Qs 	 	+ Proxies_AM::get_P+1,
            get_Qs              = G2_2::Ports_Qmt::get_Qs  		+ Proxies_AM::get_P+1,
            set_Qms             = G2_2::Ports_Qmt::set_Qms  	 	+ Proxies_AM::get_P+1,
            get_Qms             = G2_2::Ports_Qmt::get_Qms  	 	+ Proxies_AM::get_P+1,
            set_R_reg           = G2_2::Ports_Qmt::set_R_reg  	 	+ Proxies_AM::get_P+1,
            get_R_reg           = G2_2::Ports_Qmt::get_R_reg  	 	+ Proxies_AM::get_P+1,
            set_P	        = G2_2::Ports_Qmt::set_P  	 	+ Proxies_AM::get_P+1,
            get_P	        = G2_2::Ports_Qmt::get_P  	 	+ Proxies_AM::get_P+1,
	};
    };

    class Proxies_p {
    public:

	enum Port_codes {
	    command_flow        = A1_2::Ports_p::command_flow 		+ Proxies_VS::get_P+1 /*displ*/,

            set_p               = A1_2::Ports_p::set_p        		+ Proxies_VS::get_P+1 /*displ*/,
            get_p               = A1_2::Ports_p::get_p        		+ Proxies_VS::get_P+1 /*displ*/
	};
    };
}; // namespace G3_1

#endif // G3_1_PROXIES