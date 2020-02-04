/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef A2_1_PORTS_H
#define A2_1_PORTS_H

namespace A2_1 {

    class Ports_Q {
    public:

	enum Port_codes {
    	    // with Master
            command_flow        = 0, // [i:1] - receives commands

            set_Q               = 1, // [i:N] - receives new state of "q's" and initializes it            
            get_Q               = 2, // [o:N] - sends current state of "q's"
            set_Qs              = 3, // [i:N] - receives new state of "q_sensor's" and initializes it            
            get_Qs              = 4, // [o:N] - sends current state of "q_sensor's"
            set_P               = 5, // [i:N-1] - receives new state of "p's" and initializes it            
            get_P               = 6, // [o:N-1] - sends current state of "p's"
            set_R_reg           = 7, // [i:N] - receives new state of "r_reg's" and initializes it            
            get_R_reg           = 8, // [o:N] - sends current state of "r_reg's"
	};
    };

}; // namespace A2_1

#endif // A2_1_PORTS_H