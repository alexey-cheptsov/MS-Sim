/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G1_1_H
#define G1_1_H

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
using namespace G1_1;
using namespace A1_1;

namespace G1_1 {

    /*
     * Microservice for q - approximation unit
     */
    class qm: public Microservice {
    public:
    
        // Description of the element's environment
        string name="";
        string element="";
        string section ="";
        string network="";
    
	// Funtional dependencies
	q* air;
    
	// Properties
	float flow_gas;     	// methan generation from dynamic sources
	float qm0;
	
	// Settings
	float A=0;   // parameters of the filtration area
        float BRf=0; //
        float V = 0;
        
        // Monitoring properties
        Monitoring* monitoring = nullptr;
        Monitoring_opts* mon_opts;
        fstream* output_air = nullptr;      // output file for q
        fstream* output_gas = nullptr;      // output file for qm
        
        // Generic constructor
        qm(int id_, string id_str_, string air_id_str_, Communicator* communicator_,
            string name_, string element_, string section_, string network_,
            Monitoring_opts* mon_opts_,
            float S_, float r_, float l_, Solver_Params& solv_params_)
            : Microservice(id_, id_str_, communicator_)
        {
    	    name = name_;
            element = element_;
            section = section_;
            network = network_;

    	    flow_gas = 0;
            qm0 = 0;

            air = new q(1000 + id_, air_id_str_, communicator_,  name_, element_, section_, network_, 
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
        }
        
        // Constructor for model with A, BRf parameters
        qm(int id_, string id_str_, string air_id_str_, Communicator* communicator_,
            string name_, string element_, string section_, string network_,
            Monitoring_opts* mon_opts_,
            float S_, float r_, float l_, float A_, float BRf_, Solver_Params& solv_params_)
            : qm(id_, id_str_, air_id_str_, communicator_,
        	name_, element_, section_, network_,
        	mon_opts_,
        	S_, r_, l_, solv_params_)
        {
            A = A_;
            BRf = BRf_;
        };
        
        // Constructor for model with V parameter
        qm(int id_, string id_str_, string air_id_str_, Communicator* communicator_,
            string name_, string element_, string section_, string network_,
            Monitoring_opts* mon_opts_,
            float S_, float r_, float l_, float V_, Solver_Params& solv_params_)
            : qm(id_, id_str_, air_id_str_, communicator_,
        	name_, element_, section_, network_,
        	mon_opts_,
        	S_, r_, l_, solv_params_)
        {
            V = V_;
        };
        
        void init_monitoring() {
    	    if (monitoring != nullptr) {
        	if (mon_opts->flag_output_file) {
            	    output_air = new fstream();
            	    output_gas = new fstream();
            	    monitoring->fout_1 = output_air;
            	    monitoring->fout_2 = output_gas;
                
            	    output_air->open("output/" + id_str + "_air.csv", ios::out);
            	    output_gas->open("output/" + id_str + "_gas.csv", ios::out);

            	    *output_air << "ExperimentID;Network;Section;Element;@timestamp;" + name + "_air" << endl;
            	    *output_gas << "ExperimentID;Network;Section;Element;@timestamp;" + name + "_gas" << endl;
            	}
            }   
        }


	// Sending qm to the attached qmt-transport element
	void send_qm() {
            add_buffer_value<float>(Ports_qm::gas_get_qm, flow_gas/*value*/);
            buffer_flush_collective_gather(Ports_qm::gas_get_qm);
            buffer_clear(Ports_qm::gas_get_qm);
        };
        
        // in case of V
        float calc_dqm_V(float flow_gas, float dq, float q) {
            if (q != 0)
                return flow_gas*dq/q + (qm0-flow_gas)*q/V;
            else
                return 0;
        };

	// in case of A and BRf
	float calc_dqm_ABRf(float flow_gas, float dq, float q) {
            return (qm0-flow_gas)/A + dq*q*2*BRf/A;
        };

	virtual void simulation_qm() {
	    float k1_qm, k2_qm, k3_qm, k4_qm;
	    float h = air->solver->h;
        
    	    if (V==0) {
        	k1_qm = calc_dqm_ABRf(flow_gas,             air->k1_q, air->flow_prev_step);
        	k2_qm = calc_dqm_ABRf(flow_gas + h*k1_qm/2, air->k2_q, air->flow_prev_step);
        	k3_qm = calc_dqm_ABRf(flow_gas + h*k2_qm/2, air->k3_q, air->flow_prev_step);
        	k4_qm = calc_dqm_ABRf(flow_gas + h*k3_qm,   air->k4_q, air->flow_prev_step);
    	    } else {
    		k1_qm = calc_dqm_V(flow_gas,             air->k1_q, air->flow_prev_step);
        	k2_qm = calc_dqm_V(flow_gas + h*k1_qm/2, air->k2_q, air->flow_prev_step);
        	k3_qm = calc_dqm_V(flow_gas + h*k2_qm/2, air->k3_q, air->flow_prev_step);
        	k4_qm = calc_dqm_V(flow_gas + h*k3_qm,   air->k4_q, air->flow_prev_step);
    	    }

	    float qm_old = flow_gas;
            air->solver->solve(&flow_gas, k1_qm, k2_qm, k3_qm, k4_qm);
            
            /*
             * Work-around to limit disturbance by too quickly growing airflows
             */
            if (fabs(qm_old-flow_gas) > 0.1)
        	flow_gas = qm_old + 0.001;

    	    //if (id_str == "Q_AM_qm0") {
            //    stringstream out;
            //    cout << "qm_old = " << qm_old << " --> qm = " << flow_gas << endl
            //	     << "k1_q = " << air->k1_q << " --> k1_qm = " << k1_qm << endl
            //         << "k2_q = " << air->k2_q << " --> k2_qm = " << k2_qm << endl
            //         << "k3_q = " << air->k3_q << " --> k3_qm = " << k3_qm << endl
            //         << "k4_q = " << air->k4_q << " --> k4_qm = " << k4_qm << endl
            //         << endl;
            //  cout << out.str();
            //}

        };
        
	/*
         * Implementation of the communication protocol
         */
         
         virtual void command__set_q() {
    	    buffer_sync(Ports_qm::set_q);
    	    air->flow = get_buffer_value<float>(Ports_qm::set_q, 0);
    	    buffer_clear(Ports_qm::set_q);
    	    
    	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Initialized q=" << air->flow << endl;
    	    cout << out.str();
	}
         
	virtual void command__set_qm() {
    	    buffer_sync(Ports_qm::set_qm);
    	    flow_gas = get_buffer_value<float>(Ports_qm::set_qm, 0);
    	    buffer_clear(Ports_qm::set_qm);
    	    
    	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Initialized qm=" << flow_gas << endl;
    	    cout << out.str();
	}

	virtual void command__set_qm0() {
    	    buffer_sync(Ports_qm::set_qm0);
    	    qm0 = get_buffer_value<float>(Ports_qm::set_qm0, 0);
    	    buffer_clear(Ports_qm::set_qm);
    	    
    	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Initialized qm0=" << flow_gas << endl;
    	    cout << out.str();
	}
	
	virtual void command__get_q() {
	    //stringstream out;
    	    //out << "ms_" << id << "(" << id_str << "): reporting q = " << air->flow << endl;
    	    //cout << out.str();
		    
	    buffer_clear(Ports_qm::get_q);
    	    add_buffer_value<float>(Ports_qm::get_q, air->flow /*value*/);
    	    buffer_flush_collective_gather(Ports_qm::get_q);
    	    buffer_clear(Ports_qm::get_q);
	}

	virtual void command__get_qm() {
	    buffer_clear(Ports_qm::get_qm);
    	    add_buffer_value<float>(Ports_qm::get_qm, flow_gas /*value*/);
    	    buffer_flush_collective_gather(Ports_qm::get_qm);
    	    buffer_clear(Ports_qm::get_qm);
	}
	
    
	virtual void command__simulation() {
	    int counter = 0;
            int nr_steps = air->solver->nr_num_steps;
            
            while (counter<nr_steps) {
        	send_qm();
        	
                air->simulation_q();
                simulation_qm();
            
                counter++;
                
                // storing q and qm
                if (monitoring != nullptr) {
                    air->time_ms.increment_time_ms(air->solver->h); // incrementing time counter
		    
		    monitoring->add_entry(air->network, air->section, air->element, air->name, 
				      air->time_ms.time_stamp(), 
				      air->flow, flow_gas);
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
		air->time_ms.init_time();
            
        	monitoring->add_entry(air->network, air->section, air->element, air->name,
                                      air->time_ms.time_stamp(), 
                                      air->flow, flow_gas);
            }
	}
	
	virtual void command__stop() {
	    if (monitoring != nullptr) {
		monitoring->data_flush(id_str);
		
		if (mon_opts->flag_output_file) {
		    output_air->close();
		    output_gas->close();
		}
	    }
	    
	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Received stop. Terminating" << endl;
    	    cout << out.str();
	}

	virtual void command__id() {
	    // output to display
	    stringstream out;
    	    out << "Hello from: ms_" << id << "(" << id_str << "): q=" << air->flow << ", qm=" << flow_gas << ", qm0=" << qm0 << endl;
    	    cout << out.str();
	}
    
	virtual void command__init_time() {
	    air->time_ms.init_time();
	}
	
	void run() {
	    init_monitoring();
	
	    bool finish = false;
	    
	    while (!finish) {
	
		// receiving command from the Master
		buffer_sync(Ports_qm::command_flow);
		int command = get_buffer_value_stack<int>(Ports_qm::command_flow);
	    
		switch (command) {
		
		    // 0
		    case Commands_qm::set_q: {
			command__set_q();
                	break;    
            	    }
            	    
		    // 1
            	    case Commands_qm::set_qm: {
			command__set_qm();
                	break;    
            	    }
            	    
            	    // 2
            	    case Commands_qm::set_qm0: {
			command__set_qm0();
                	break;    
            	    }

		    // 3            	    
            	    case Commands_qm::get_q: {
			command__get_q();
			break;
            	    }
                
            	    // 4
            	    case Commands_qm::get_qm: {
			command__get_qm();
			break;
            	    }

		    // 5                
		    case Commands_qm::simulation: {
			command__simulation();
			break;
		    }

		    // 6		
		    case Commands_qm::save: {
                	command__save();
                	break;
            	    }

		    // 7
		    case Commands_qm::stop: {
    			command__stop();
    			finish = true;		
			break;
		    }

		    // 8 
		    case Commands_qm::id: {
    			command__id();
    			break;
		    }    

		    // 9
		    case Commands_qm::init_time: {
    			command__init_time();
    			break;
		    }    
		}
	    }
	}
		
    }; // class qm
    
}; // namespace G1_1


#endif // G1_1_H