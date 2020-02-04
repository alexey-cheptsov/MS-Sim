/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G1_2_PORTS_H
#define G1_2_PORTS_H


namespace G1_2 {

    class Ports_qmt {
    public:

	enum Port_codes {
	// with Master
            command_flow        = 0, // [i:1] - receives commands
            
	    set_q               = 1, // [i:1] - receives new state of "q" and initializes it            
            get_q               = 2, // [o:1] - sends current state of "q"
            set_qm              = 3, // [i:1] - receives new state of "q_m" and initializes it            
            get_qm              = 4, // [o:1] - sends current state of "q_m"
            set_qs              = 5, // [i:1] - receives new state of "q_sensor" and initializes it            
            get_qs              = 6, // [o:1] - sends current state of "q_sensor"
            set_qms             = 7, // [i:1] - receives new state of "q_ms" and initializes it            
            get_qms             = 8, // [o:1] - sends current state of "q_ms"
            set_r_reg           = 9, // [i:1] - receives new state of "r_reg" and initializes it            
            get_r_reg           = 10, // [o:1] - sends current state of "r_reg"
            
        // with p-services
            num_set_pstart	= 11, // [i:1] - receives pstart
            num_set_pend	= 12,// [i:1] - receives pend
            num_get_q		= 13,// [o:1] - sends q, ki
            
        // with qm-services
    	    gas_set_qm		= 14, // [i:1] - receives qm from previous qm/qmt-elements
    	    gas_get_qm		= 15  // [o:1] - sends qm to further qm-elements
	};
    };

}; // namespace G1_2

#endif // G1_2_PORTS_H