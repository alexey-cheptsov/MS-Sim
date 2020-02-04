/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G2_2_PORTS_H
#define G2_2_PORTS_H

namespace G2_2 {

    class Ports_Qmt {
    public:

	enum Port_codes {
    	    // with Master
            command_flow        = 0, // [i:1] - receives commands

            set_Q               = 1, // [i:N] - receives new state of "q's" and initializes it            
            get_Q               = 2, // [o:N] - sends current state of "q's"
            set_Qm              = 3, // [i:N] - receives new state of "q_sensor's" and initializes it            
            get_Qm              = 4, // [o:N] - sends current state of "q_sensor's"
            set_Qs              = 5, // [i:N] - receives new state of "q_sensor's" and initializes it            
            get_Qs              = 6, // [o:N] - sends current state of "q_sensor's"
            set_Qms             = 7, // [i:N] - receives new state of "qm_sensor's" and initializes it            
            get_Qms             = 8, // [o:N] - sends current state of "qm_sensor's"
            set_R_reg           = 9, // [i:N] - receives new state of "r_reg's" and initializes it            
            get_R_reg           = 10,// [o:N] - receives new state of "r_reg's" and initializes it            
            set_P               = 11,// [i:N-1] - receives new state of "p's" and initializes it            
            get_P               = 12 // [o:N-1] - sends current state of "p's"
	};
    };

}; // namespace G2_2

#endif // G2_2_PORTS_H