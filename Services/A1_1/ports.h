/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef A1_1_PORTS_H
#define A1_1_PORTS_H


namespace A1_1 {

    class Ports_q {
    public:

	enum Port_codes {
	// with Master
            command_flow        = 0, // [i:1] - receives commands
            
	    set_q               = 1, // [i:1] - receives new state of "q" and initializes it            
            get_q               = 2, // [o:1] - sends current state of "q"
            set_qs              = 3, // [i:1] - receives new state of "q_sensor" and initializes it            
            get_qs              = 4, // [o:1] - sends current state of "q_sensor"
            set_r_reg           = 5, // [i:1] - receives new state of "r_reg" and initializes it            
            get_r_reg           = 6, // [o:1] - sends current state of "r_reg"
            
        // with p-services
            num_set_pstart	= 7, // [i:1] - receives pstart
            num_set_pend	= 8, // [i:1] - receives pend
            num_get_q		= 9  // [o:1] - sends q
	};
    };

}; // namespace A1_1

#endif // A1_1_PORTS_H