/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef A1_1_COMMANDS_H
#define A1_1_COMMANDS_H

namespace A1_1 {
    
    class Commands_q {
    public:

	enum Command_codes {
            set_q		= 0,
            set_qs		= 1,
            set_r_reg		= 2,
            
            get_q	        = 3,
            get_qs	        = 4,
            get_r_reg		= 5,
            
            simulation          = 6,
            save                = 7,
            stop                = 8,
            id                  = 9,
            init_time		= 10
        };
    };

}; // namespace A1_1

#endif // A1_1_COMMANDS_H