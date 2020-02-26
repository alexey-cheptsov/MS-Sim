/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef A3_1_PROXIES_H
#define A3_1_PROXIES_H

#include "../A2_1/ports.h" // Q
#include "../A1_2/ports.h" // P

namespace A3_1 {

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
            command_flow        = A2_1::Ports_Q::command_flow	+ Proxies_Streb::get_R_reg+1,
	    
	    set_Q               = A2_1::Ports_Q::set_Q 		+ Proxies_Streb::get_R_reg+1,
            get_Q               = A2_1::Ports_Q::get_Q 		+ Proxies_Streb::get_R_reg+1,
	    set_Qs              = A2_1::Ports_Q::set_Qs 	+ Proxies_Streb::get_R_reg+1,
            get_Qs              = A2_1::Ports_Q::get_Qs 	+ Proxies_Streb::get_R_reg+1, 
            set_P	        = A2_1::Ports_Q::set_P 		+ Proxies_Streb::get_R_reg+1,
            get_P	        = A2_1::Ports_Q::get_P 		+ Proxies_Streb::get_R_reg+1,
            set_R_reg	        = A2_1::Ports_Q::set_R_reg 	+ Proxies_Streb::get_R_reg+1,
            get_R_reg	        = A2_1::Ports_Q::get_R_reg 	+ Proxies_Streb::get_R_reg+1,           
	};
    };

    class Proxies_VS {
    public:

	enum Port_codes {
            command_flow        = A2_1::Ports_Q::command_flow	+ Proxies_AM::get_R_reg+1,
	    
	    set_Q               = A2_1::Ports_Q::set_Q 		+ Proxies_AM::get_R_reg+1,
            get_Q               = A2_1::Ports_Q::get_Q 		+ Proxies_AM::get_R_reg+1,
	    set_Qs              = A2_1::Ports_Q::set_Qs 	+ Proxies_AM::get_R_reg+1,
            get_Qs              = A2_1::Ports_Q::get_Qs 	+ Proxies_AM::get_R_reg+1, 
            set_P	        = A2_1::Ports_Q::set_P 		+ Proxies_AM::get_R_reg+1,
            get_P	        = A2_1::Ports_Q::get_P 		+ Proxies_AM::get_R_reg+1,
            set_R_reg	        = A2_1::Ports_Q::set_R_reg 	+ Proxies_AM::get_R_reg+1,
            get_R_reg	        = A2_1::Ports_Q::get_R_reg 	+ Proxies_AM::get_R_reg+1,           
	};
    };

    class Proxies_p {
    public:

	enum Port_codes {
	    command_flow        = A1_2::Ports_p::command_flow 		+ Proxies_VS::get_R_reg+1 /*displ*/,

            set_p               = A1_2::Ports_p::set_p        		+ Proxies_VS::get_R_reg+1 /*displ*/,
            get_p               = A1_2::Ports_p::get_p        		+ Proxies_VS::get_R_reg+1 /*displ*/
	};
    };
}; // namespace A3_1

#endif // A3_1_PROXIES