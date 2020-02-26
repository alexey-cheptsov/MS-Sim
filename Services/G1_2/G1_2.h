/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G1_2_H
#define G1_2_H

#include <iostream>
#include <fstream> // fout
#include <sstream> // stringstream
#include <vector>
#include <thread> 
#include <chrono> // chrono::milliseconds(x)
#include <cmath>  // fabs
#include <iomanip> // std::setprecision

#include "commands.h"
#include "ports.h"

#include "../A1_1/A1_1.h"

#include "../../buffer.h"
#include "../../microservice.h"
#include "../../Numerics/Euler.h"
#include "../../Numerics/RK4.h"

#include "../../Monitoring/monitoring.h"

using namespace std;
using namespace G1_2;
using namespace A1_1;

namespace G1_2 {

    /*
     * Microservice for q - approximation unit
     */
    class qmt: public Microservice {
    public:
	// Description of the element's environment
        string name="";
        string air_name="";
        string element="";
        string section ="";
        string network="";
    
	// Funtional dependencies
	q* air;
    
	// Properties
	float flow_gas;     	// methan generation from dynamic sources
	float flow_gas_sensor;  // qm_streb
	
	// Monitoring properties
        Monitoring* monitoring = nullptr;
        Monitoring_opts* mon_opts;
        fstream* output_gas = nullptr;      // output file for qmt
	
        qmt(int id_, string id_str_, string air_id_str_, Communicator* communicator_,
            string name_, string air_name_, string element_, string section_, string network_,
            Monitoring_opts* mon_opts_,
            float S_, float r_, float l_, Solver_Params& solv_params_)
            : Microservice(id_, id_str_, communicator_)
        {
    	    name = name_;
    	    air_name = air_name_;
            element = element_;
            section = section_;
            network = network_;

            flow_gas = 0;
            flow_gas_sensor = 0;
            
            air = new q(1000 + id_, air_id_str_, communicator_,  air_name_, element_, section_, network_,
        		mon_opts_,
        		S_, r_, l_, solv_params_);
        
    	    air->add_buffer(new LocalIntBuffer  (air->id /*ms_id*/, air->id_str, 0 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 1 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 2 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 3 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 4 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 5 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 6 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 7 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 8 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 9 /*port*/, communicator));

	    mon_opts = mon_opts_;
	    monitoring = new Monitoring(mon_opts_);
        };
        
        qmt(int id_, string id_str_, string air_id_str_, Communicator* communicator_,
            string name_, string air_name_, string element_, string section_, string network_,
            float S_, float r_, float l_, Solver_Params& solv_params_)
            : Microservice(id_, id_str_, communicator_)
        {
    	    name = name_;
    	    air_name = air_name_;
            element = element_;
            section = section_;
            network = network_;

            flow_gas = 0;
            flow_gas_sensor = 0;
            
            air = new q(1000 + id_, air_id_str_, communicator_,  air_name_, element_, section_, network_,
        		S_, r_, l_, solv_params_);
        
    	    air->add_buffer(new LocalIntBuffer  (air->id /*ms_id*/, air->id_str, 0 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 1 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 2 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 3 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 4 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 5 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 6 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 7 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 8 /*port*/, communicator));
            air->add_buffer(new LocalFloatBuffer(air->id /*ms_id*/, air->id_str, 9 /*port*/, communicator));
        };
        
        void init_monitoring() {
    	    if (monitoring != nullptr) {
        	if (mon_opts->flag_output_file) {
            	    output_gas = new fstream();
            	    monitoring->fout_1 = output_gas;

            	    output_gas->open("output/" + id_str + ".csv", ios::out);

            	    *output_gas << "ExperimentID;Network;Section;Element;@timestamp;" + name << endl;
            	}
            }
        }


	// Sending qm to the next qmt transport element
	void send_qm() {
            add_buffer_value<float>(Ports_qmt::gas_get_qm, flow_gas + flow_gas_sensor /*value*/);
            buffer_flush_collective_gather(Ports_qmt::gas_get_qm);
            buffer_clear(Ports_qmt::gas_get_qm);
        };

	// Receiving qm from previous qm or qmt element(s)
	void recv_qm() {
	    buffer_sync(Ports_qmt::gas_set_qm);
            
            flow_gas = 0;
            for (int i=0; i<buffer_get_size(Ports_qmt::gas_set_qm); i++)
                flow_gas += get_buffer_value<float>(Ports_qmt::gas_set_qm, i /*index*/);

            buffer_clear(Ports_qmt::gas_set_qm);
        };

	/*
         * Implementation of the communication protocol
         */
         
         virtual void command__set_q() {
    	    buffer_sync(Ports_qmt::set_q);
    	    air->flow = get_buffer_value<float>(Ports_qmt::set_q, 0);
    	    buffer_clear(Ports_qmt::set_q);
    	    
    	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Initialized q=" << air->flow << endl;
    	    cout << out.str();
	}
	
	virtual void command__set_qm() {
    	    buffer_sync(Ports_qmt::set_qm);
    	    flow_gas = get_buffer_value<float>(Ports_qmt::set_qm, 0);
    	    buffer_clear(Ports_qmt::set_qm);
    	    
    	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Initialized qm=" << flow_gas << endl;
    	    cout << out.str();
	}
         
        virtual void command__set_qs() {
    	    buffer_sync(Ports_qmt::set_qs);
    	    air->flow_sensor = get_buffer_value<float>(Ports_qmt::set_qs, 0);
    	    buffer_clear(Ports_qmt::set_qs);
    	    
    	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Initialized qs=" << air->flow_sensor << endl;
    	    cout << out.str();
	}
	
	virtual void command__set_qms() {
    	    buffer_sync(Ports_qmt::set_qms);
    	    flow_gas_sensor = get_buffer_value<float>(Ports_qmt::set_qms, 0);
    	    buffer_clear(Ports_qmt::set_qms);
    	    
    	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Initialized qms=" << flow_gas_sensor << endl;
    	    cout << out.str();
	}
	
	virtual void command__set_r_reg() {
    	    buffer_sync(Ports_qmt::set_r_reg);
    	    air->r_reg = get_buffer_value<float>(Ports_qmt::set_r_reg, 0);
    	    buffer_clear(Ports_qmt::set_r_reg);
    	    
    	    air->alfa = (air->S)   / (air->ro * air->l);
            air->beta = (air->S*(air->r+air->r_reg)) / (air->ro);

    	    
    	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Initialized r_reg=" << air->r_reg << endl;
    	    cout << out.str();
	}
	
	virtual void command__get_q() {
	    //stringstream out;
    	    //out << "ms_" << id << "(" << id_str << "): reporting q = " << flow << endl;
    	    //cout << out.str();
		    
	    buffer_clear(Ports_qmt::get_q);
    	    add_buffer_value<float>(Ports_qmt::get_q, air->flow /*value*/);
    	    buffer_flush_collective_gather(Ports_qmt::get_q);
    	    buffer_clear(Ports_qmt::get_q);
	}
	
	virtual void command__get_qm() {
	    buffer_clear(Ports_qmt::get_qm);
    	    add_buffer_value<float>(Ports_qmt::get_qm, flow_gas /*value*/);
    	    buffer_flush_collective_gather(Ports_qmt::get_qm);
    	    buffer_clear(Ports_qmt::get_qm);
	}

	virtual void command__get_qs() {
	    buffer_clear(Ports_qmt::get_qs);
    	    add_buffer_value<float>(Ports_qmt::get_qs, air->flow_sensor /*value*/);
    	    buffer_flush_collective_gather(Ports_qmt::get_qs);
    	    buffer_clear(Ports_qmt::get_qs);
	}
	
	virtual void command__get_qms() {
	    buffer_clear(Ports_qmt::get_qms);
    	    add_buffer_value<float>(Ports_qmt::get_qms, flow_gas_sensor /*value*/);
    	    buffer_flush_collective_gather(Ports_qmt::get_qms);
    	    buffer_clear(Ports_qmt::get_qms);
	}
	
	virtual void command__get_r_reg() {
	    buffer_clear(Ports_qmt::get_r_reg);
    	    add_buffer_value<float>(Ports_qmt::get_r_reg, air->r_reg /*value*/);
    	    buffer_flush_collective_gather(Ports_qmt::get_r_reg);
    	    buffer_clear(Ports_qmt::get_r_reg);
	}
    
	virtual void command__simulation() {
	    int counter = 0;
            int nr_steps = air->solver->nr_num_steps;
            
            while (counter<nr_steps) {
                air->simulation_q();
		send_qm();
		recv_qm();
		            
                counter++;

                // storing q and qm
                if (monitoring != nullptr) {
            	    if (mon_opts->flag_is_realtime) {
            		air->time_ms.increment_time_ms(air->solver->h); // incrementing time counter
            	    
			air->monitoring->add_entry(air->network, air->section, air->element, air->name, 
					      air->time_ms.time_stamp(), 
					      air->flow);
			monitoring->add_entry(network, section, element, name, 
					      air->time_ms.time_stamp(), 
					      flow_gas);
		    } else {
			air->time_ms_relative += air->solver->h; // incrementing time counter
			
			air->monitoring->add_entry(air->network, air->section, air->element, air->name,
                                                   air->time_ms_relative,
                                                   air->flow);
                        monitoring->add_entry(network, section, element, name,
                                              air->time_ms_relative,
                                              flow_gas);
		    }
		}
            }
	}
    
	virtual void command__save() {
	    // output to screen
	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): q=" << air->flow << ", qm=" << flow_gas << endl;
    	    cout << out.str();

	    // output to data layer
	    if (monitoring != nullptr) {
                if (mon_opts->flag_is_realtime) {
                    air->time_ms.init_time();
            
                    air->monitoring->add_entry(air->network, air->section, air->element, air->name,
                                          air->time_ms.time_stamp(),
                                          air->flow);
                    monitoring->add_entry(network, section, element, name,
                                          air->time_ms.time_stamp(),
                                          flow_gas);
                } else {
                    air->monitoring->add_entry(air->network, air->section, air->element, air->name,
                                          air->time_ms_relative,
                                          air->flow);
                    monitoring->add_entry(network, section, element, name,
                                          air->time_ms_relative,
                                          flow_gas);
                }
            }



	}
	
	virtual void command__stop() {
	    if (monitoring != nullptr) {
		if (mon_opts->flag_output_file) {
		    air->output->close();
		    output_gas->close();
		}
	    
		air->monitoring->data_flush(air->id_str); 
		monitoring->data_flush(id_str);
	    }
	
	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Received stop. Terminating" << endl;
    	    cout << out.str();
	}

	virtual void command__id() {
	    // output to display
	    stringstream out;
    	    out << "Hello from: ms_" << id << "(" << id_str << "): q=" << air->flow << ", qm=" << flow_gas << endl;
    	    cout << out.str();
	}
    
	virtual void command__init_time() {
	    air->time_ms.init_time();
	}
	
	void run() {
	    air->init_monitoring();
	    init_monitoring();
	
	    bool finish = false;
	    
	    while (!finish) {
	
		// receiving command from the Master
		buffer_sync(Ports_qmt::command_flow);
		int command = get_buffer_value_stack<int>(Ports_qmt::command_flow);
	    
		switch (command) {
		
		    // 0
		    case Commands_qmt::set_q: {
			command__set_q();
                	break;    
            	    }
            	    
            	    // 1
		    case Commands_qmt::set_qm: {
			command__set_qm();
                	break;    
            	    }
            	    
		    // 2
            	    case Commands_qmt::set_qs: {
			command__set_qs();
                	break;    
            	    }
            	    
            	    // 3
            	    case Commands_qmt::set_qms: {
			command__set_qms();
                	break;    
            	    }
            	    
            	    // 4
            	    case Commands_qmt::set_r_reg: {
			command__set_r_reg();
                	break;    
            	    }

		    // 5            	    
            	    case Commands_qmt::get_q: {
			command__get_q();
			break;
            	    }
            	    
            	    // 6            	    
            	    case Commands_qmt::get_qm: {
			command__get_qm();
			break;
            	    }
                
		    // 7
            	    case Commands_qmt::get_qs: {
			command__get_qs();
			break;
            	    }
            	    
            	    // 8
            	    case Commands_qmt::get_qms: {
			command__get_qms();
			break;
            	    }
            	    
            	    // 9
            	    case Commands_qmt::get_r_reg: {
			command__get_r_reg();
			break;
            	    }

		    // 10                
		    case Commands_qmt::simulation: {
			command__simulation();
			break;
		    }

		    // 11		
		    case Commands_qmt::save: {
                	command__save();
                	break;
            	    }

		    // 12
		    case Commands_qmt::stop: {
    			command__stop();
    			finish = true;		
			break;
		    }

		    // 13
		    case Commands_qmt::id: {
    			command__id();
    			break;
		    }    

		    // 14
		    case Commands_qmt::init_time: {
    			command__init_time();
    			break;
		    }    
		}
	    }
	}
    }; // class qmt


}; // namespace G1_2


#endif // G1_2_H