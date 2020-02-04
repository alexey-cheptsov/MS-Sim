/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G1_1_PORTS_H
#define G1_1_PORTS_H


namespace G1_1 {

    class Ports_qm {
    public:

	enum Port_codes {
	// with Master
            command_flow        = 0, // [i:1] - receives commands
            
	    set_q               = 1, // [i:1] - receives new state of "q" and initializes it            
            get_q               = 2, // [o:1] - sends current state of "q"
            set_qm              = 3, // [i:1] - receives new state of "q_m" and initializes it            
            get_qm              = 4, // [o:1] - sends current state of "q_m"
            set_qm0             = 5, // [i:1] - receives new state of "q_m0" and initializes it            
            get_qm0             = 6, // [i:1] - sends new state of "q_m0" and initializes it            
            
        // with p-services
            num_set_pstart	= 7, // [i:1] - receives pstart
            num_set_pend	= 8, // [i:1] - receives pend
            num_get_q		= 9, // [o:1] - sends q, ki
            
        // with qmt-services
    	    gas_get_qm		= 10  // [o:1] - sends qm to gmt-elements
	};
    };

}; // namespace G1_1

#endif // G1_1_PORTS_H