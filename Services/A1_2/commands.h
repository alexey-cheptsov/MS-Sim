/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef A1_2_COMMANDS_H
#define A1_2_COMMANDS_H

namespace A1_2 {
    
    class Commands_p {
    public:

	enum Command_codes {
            set_p		= 0,
            get_p		= 1,
            
            simulation          = 2,
            save                = 3,
            stop                = 4,
            id			= 5,
            init_time		= 6,
            flush_data		= 7
            
        };
    };

}; // namespace A1_2

#endif // A1_2_COMMANDS_H