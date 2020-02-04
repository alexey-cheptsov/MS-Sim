/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G2_1_PORTS_H
#define G2_1_PORTS_H

namespace G2_1 {

    class Ports_Qm {
    public:

	enum Port_codes {
    	    // with Master
            command_flow        = 0, // [i:1] - receives commands

            set_Q               = 1, // [i:N] - receives new state of "q's" and initializes it            
            get_Q               = 2, // [o:N] - sends current state of "q's"
            set_Qm              = 3, // [i:N] - receives new state of "q_m's" and initializes it            
            get_Qm              = 4, // [o:N] - sends current state of "q_m's"
            set_Qm0             = 5, // [i:N] - receives new state of "qm0's" and initializes it            
            get_Qm0             = 6, // [o:N] - sends current state of "qm0's"
            set_P               = 7, // [i:N-1] - receives new state of "p's" and initializes it            
            get_P               = 8, // [o:N-1] - sends current state of "p's"
	};
    };

}; // namespace G2_1

#endif // G2_1_PORTS_H