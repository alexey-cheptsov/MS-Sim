/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G1_2_COMMANDS_H
#define G1_2_COMMANDS_H

namespace G1_2 {
    
    class Commands_qmt {
    public:

	enum Command_codes {
            set_q		= 0,
            set_qm		= 1,
            set_qs		= 2,
	    set_qms		= 3,
	    set_r_reg		= 4,            
            
            get_q	        = 5,
            get_qm	        = 6,
            get_qs	        = 7,
            get_qms	        = 8,
            get_r_reg		= 9,
            
            simulation          = 10,
            save                = 11,
            stop                = 12,
            id                  = 13,
            init_time		= 14
        };
    };

}; // namespace G1_2

#endif // G1_2_COMMANDS_H