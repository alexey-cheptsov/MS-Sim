/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef A1_2_PORTS_H
#define A1_2_PORTS_H


namespace A1_2 {

    class Ports_p {
    public:

	enum Port_codes {
	// with Master
            command_flow        = 0, // [i:1] - receives commands
            
	    set_p               = 1, // [i:1] - receives new state of "q" and initializes it            
            get_p               = 2, // [o:1] - sends current state of "q"
        
        // with q-services
            num_set_qin		= 3, // [i:1] - receives q_in
            num_set_qout	= 4, // [i:1] - receives q_out
            num_get_p		= 5  // [o:1] - sends p

	};
    };

}; // namespace A1_2

#endif // A1_2_PORTS_H