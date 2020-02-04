/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G3_1_PORTS_H
#define G3_1_PORTS_H

namespace G3_1 {

    class Ports_QQmt {
    public:

	enum Port_codes {
    	    // with Master
            command_flow           = 0, // [i:1] - receives commands

            set_Q_OS               = 1, // [i:N] - receives new state of "q's" and initializes it            
            set_Q_Streb            = 2, // [i:N] - receives new state of "q's" and initializes it            
            set_Q_AM               = 3, // [i:N] - receives new state of "q's" and initializes it            
            set_Q_VS               = 4, // [i:N] - receives new state of "q's" and initializes it            
            
            get_Q_OS               = 5, // [o:N] - sends current state of "q's"
            get_Q_Streb            = 6, // [o:N] - sends current state of "q's"
            get_Q_AM               = 7, // [o:N] - sends current state of "q's"
            get_Q_VS               = 8, // [o:N] - sends current state of "q's"
            
            set_Qm_AM              = 9, // [i:N] - receives new state of "q_sensor's" and initializes it            
            set_Qm_VS              = 10, // [i:N] - receives new state of "q_sensor's" and initializes it            
            get_Qm_AM              = 11, // [o:N] - sends current state of "q_sensor's"
            get_Qm_VS              = 12, // [o:N] - sends current state of "q_sensor's"
            
            set_Qs_VS              = 13, // [i:N] - receives new state of "q_sensor's" and initializes it            
            get_Qs_VS              = 14, // [o:N] - sends current state of "q_sensor's"
            
            set_Qms_VS             = 15, // [i:N] - receives new state of "qm_sensor's" and initializes it            
            get_Qms_VS             = 16, // [o:N] - sends current state of "qm_sensor's"
            
            set_Qm0_AM             = 17, // [i:N] - receives new state of "qm_sensor's" and initializes it            
            get_Qm0_AM             = 18, // [o:N] - sends current state of "qm_sensor's"
            
            set_R_reg_VS           = 19, // [i:N] - receives new state of "r_reg's" and initializes it            
            get_R_reg_VS           = 20  // [o:N] - receives new state of "r_reg's" and initializes it            
	};
    };

}; // namespace G3_1

#endif // G3_1_PORTS_H