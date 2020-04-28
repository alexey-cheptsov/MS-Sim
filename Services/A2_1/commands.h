/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef A2_1_COMMANDS_H
#define A2_1_COMMANDS_H

namespace A2_1 {
    
    class Commands_Q {
    public:

	enum Command_codes {
	    set_Q               = 0,
            set_Qs              = 1,
            set_P		= 2,
            set_R_reg		= 3,
            get_Q               = 4,
            get_Qs              = 5,
            get_P		= 6,
            get_R_reg		= 7,

            simulation          = 8,
            save                = 9,
            stop                = 10,
            id                  = 11,
            init_time           = 12,
            flush_data		= 13
        };
    };
}; // namespace A2_1

#endif // A2_1_COMMANDS_H