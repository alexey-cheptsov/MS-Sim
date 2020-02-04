/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G1_1_COMMANDS_H
#define G1_1_COMMANDS_H

namespace G1_1 {
    
    class Commands_qm {
    public:

	enum Command_codes {
            set_q		= 0,
            set_qm		= 1,
            set_qm0		= 2,
            get_q	        = 3,
            get_qm	        = 4,
            get_qm0	        = 5,
            
            simulation          = 6,
            save                = 7,
            stop                = 8,
            id                  = 9,
            init_time		= 10
        };
    };

}; // namespace G1_1

#endif // G1_1_COMMANDS_H