/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G2_1_COMMANDS_H
#define G2_1_COMMANDS_H

namespace G2_1 {
    
    class Commands_Qm {
    public:

	enum Command_codes {
	    set_Q               = 0,
            set_Qm              = 1,
            set_Qm0             = 2,
            set_P		= 3,
            get_Q               = 4,
            get_Qm              = 5,
            get_Qm0             = 6,
            get_P		= 7,

            simulation          = 8,
            save                = 9,
            stop                = 10,
            id                  = 11,
            init_time           = 12
        };
    };
}; // namespace G2_1

#endif // G2_1_COMMANDS_H