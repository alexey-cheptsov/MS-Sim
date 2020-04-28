/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G3_1_COMMANDS_H
#define G3_1_COMMANDS_H

namespace G3_1 {
    
    class Commands_QQmt {
    public:

	enum Command_codes {
	// Set
	    set_Q_OS            = 0,
	    set_Q_Streb         = 1,
	    set_Q_AM            = 2,
	    set_Q_VS            = 3,
	    
	    set_Qm_AM           = 4,
	    set_Qm_VS           = 5,
	    
            set_Qs_VS           = 6,
            set_Qms_VS          = 7,
            
            set_Qm0_AM          = 8,
        
            set_R_reg_VS        = 9,
            
        // Get
            get_Q_OS            = 10,
	    get_Q_Streb         = 11,
	    get_Q_AM            = 12,
	    get_Q_VS            = 13,
	    
	    get_Qm_AM           = 14,
	    get_Qm_VS           = 15,
	    
            get_Qs_VS           = 16,
            get_Qms_VS          = 17,
        
            get_Qm0_AM          = 18,
            
            get_R_reg_VS        = 19,
        
        // Commands
            simulation          = 20,
            save                = 21,
            stop                = 22,
            id                  = 23,
            init_time           = 24,
            flush_data          = 25

        };
    };
}; // namespace G3_1

#endif // G3_1_COMMANDS_H