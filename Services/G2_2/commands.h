/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G2_2_COMMANDS_H
#define G2_2_COMMANDS_H

namespace G2_2 {
    
    class Commands_Qmt {
    public:

	enum Command_codes {
	    set_Q               = 0,
	    set_Qm              = 1,
            set_Qs              = 2,
            set_Qms             = 3,
            set_R_reg           = 4,
            set_P		= 5,
            
            get_Q               = 6,
            get_Qm              = 7,
            get_Qs              = 8,
            get_Qms             = 9,
            get_R_reg           = 10,
            get_P		= 11,

            simulation          = 12,
            save                = 13,
            stop                = 14,
            id                  = 15,
            init_time           = 16,
            flush_data          = 17
        };
    };
}; // namespace G2_2

#endif // G2_2_COMMANDS_H