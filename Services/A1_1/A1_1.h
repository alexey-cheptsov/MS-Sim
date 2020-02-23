/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef A1_1_H
#define A1_1_H

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

#include "../../buffer.h"
#include "../../microservice.h"
#include "../../Numerics/Euler.h"
#include "../../Numerics/RK4.h"

#include "../../Monitoring/monitoring.h"

using namespace std;
using namespace A1_1;

namespace A1_1 {

    /*
     * Microservice for q - approximation unit
     */
    class q: public Microservice {
    public:
    
	// Description of the element's environment
	string name="";
	string element="";
	string section ="";
	string network="";
	
	// Properties
	float flow;         // air flow
	float flow_sensor;  // air sensor data
	
	// Settings
	float S;   	// cut square
	float r;   	// (specific) aerodynamic resistance = R/L
	float l;   	// (specific) length of the approx. element = L/N
	
	float r_reg;  	// (specific) aerodynamic resistance of regulator

	float p_start; 	// start and end P values, needed during the simulation
	float p_end;
    
	Solver* solver;
	
	float alfa;
    	float beta;
	
	// Monitoring properties
	Time_MS time_ms; // time stamp for real-time option
	float time_ms_relative;     // time stamp for relative time option
	Monitoring* monitoring = nullptr;
	Monitoring_opts* mon_opts = nullptr;
	fstream* output = nullptr; 	// output file for q
	
	// Constants
	const float ro = 1.25;

	// Numerics - params to be used by children-models
	float flow_prev_step; // value of flow in the previous step of time integration
	float k1_q, k2_q, k3_q, k4_q;

	
	q(int id_, string id_str_, Communicator* communicator_, float S_, float r_, float l_, Solver_Params& solv_params_) 
		: Microservice(id_, id_str_, communicator_) 
        {
    	    S = S_;
	    r = r_;
	    l = l_;
	
	    flow = 0;
    	    flow_sensor = 0;
    	    r_reg = 0;
    	    
    	    alfa = (S)   / (ro * l);
    	    beta = (S*r) / (ro);
        
    	    init_solver(solv_params_);
	};

	q(int id_, string id_str_, Communicator* communicator_, 
	  string name_, string element_, string section_, string network_,
	  float S_, float r_, float l_, Solver_Params& solv_params_)
		: q(id_, id_str_, communicator_, S_, r_, l_, solv_params_)
        {
    	    name = name_;
            element = element_;
            section = section_;
            network = network_;
	};

	
	q(int id_, string id_str_, Communicator* communicator_, 
	  string name_, string element_, string section_, string network_,
	  Monitoring_opts* mon_opts_,
	  float S_, float r_, float l_, Solver_Params& solv_params_)
		: q(id_, id_str_, communicator_, 
		name_, element_, section_, network_,
		S_, r_, l_, solv_params_)
        {
            mon_opts = mon_opts_;
	    monitoring = new Monitoring(mon_opts_);
	};
	
	void init_solver(Solver_Params params) {
	    RK4* rk4 = new RK4(params);
            solver = rk4;
	};
	
	void init_monitoring() {
	    if (monitoring != nullptr) {
		if (mon_opts->flag_output_file) {
        	    output = new fstream();
        	    monitoring->fout_1 = output;
        	
            	    output->open("output/" + id_str + ".csv", ios::out);
	    	    *output << "ExperimentID;Network;Section;Element;@timestamp;" + name << endl;
		}
    	    }
	}
    
    
	float calc_dq(float p_start, float p_end, float flow) {
	    return (p_start-p_end)*alfa - flow*fabs(flow)*beta;
	};
	
	virtual void simulation_q() {
    	    float k1_p_start, k1_p_end, k2_p_start, k2_p_end, k3_p_start, k3_p_end;
    	    float h = solver->h;
	    
	    send_q();
	    recv_p();
	    
	    // k1
	    k1_q = calc_dq(p_start, p_end, flow);
	    send_k(k1_q);
	    
	    // k2
	    recv_k(&k1_p_start, &k1_p_end);
	    k2_q = calc_dq(p_start + h*k1_p_start/2, p_end + h*k1_p_end/2, flow + h*k1_q/2);
	    send_k(k2_q);
	    
	    // k3
	    recv_k(&k2_p_start, &k2_p_end);
	    k3_q = calc_dq(p_start + h*k2_p_start/2, p_end + h*k2_p_end/2, flow + h*k2_q/2);
	    send_k(k3_q);
	    
	    // k4
	    recv_k(&k3_p_start, &k3_p_end);
	    k4_q = calc_dq(p_start + h*k3_p_start, p_end + h*k3_p_end, flow + h*k3_q);
	
	    flow_prev_step = flow;
	    solver->solve(&flow, k1_q, k2_q, k3_q, k4_q);
	    
	    //if (id_str == "Q_OS_q0") {
	    //    stringstream out;
	    //    
	    //    out << id_str << ": q_old =" << flow_prev_step << ", q_new =" << flow << endl
            //            << "p_start = " << p_start << ", p_end = " << p_end << " --> k1 = " << k1_q << endl
            //            << "k1_p_start = " << k1_p_start << ", k1_p_end =" << k1_p_end << " --> k2 = " << k2_q << endl
            //            << "k2_p_start = " << k2_p_start << ", k2_p_end =" << k2_p_end << " --> k3 = " << k3_q << endl
            //            << "k3_p_start = " << k3_p_start << ", k3_p_end =" << k3_p_end << " --> k4 = " << k4_q << endl << endl;
    	    //    cout << out.str();
    	    //}
        };
        
	void recv_p() {
	    buffer_sync(Ports_q::num_set_pstart);
	    buffer_sync(Ports_q::num_set_pend);
	    
	    p_start = get_buffer_value_stack<float>(Ports_q::num_set_pstart);
	    p_end   = get_buffer_value_stack<float>(Ports_q::num_set_pend);
	    
	    buffer_clear(Ports_q::num_set_pstart);
	    buffer_clear(Ports_q::num_set_pend);
	};    
	
	void recv_k(float* k_p_start, float* k_p_end) {
	    buffer_sync(Ports_q::num_set_pstart);
	    buffer_sync(Ports_q::num_set_pend);
	
	    *k_p_start = get_buffer_value_stack<float>(Ports_q::num_set_pstart);
	    *k_p_end   = get_buffer_value_stack<float>(Ports_q::num_set_pend);
	    
	    buffer_clear(Ports_q::num_set_pstart);
	    buffer_clear(Ports_q::num_set_pend);
	};    
	
	void send_q() {
    	    add_buffer_value<float>(Ports_q::num_get_q, flow/*value*/);
    	    buffer_flush_collective_replicate(Ports_q::num_get_q);
    	    buffer_clear(Ports_q::num_get_q);
	};
	
	void send_k(float k) {
    	    add_buffer_value<float>(Ports_q::num_get_q, k/*value*/);
    	    buffer_flush_collective_replicate(Ports_q::num_get_q);
    	    buffer_clear(Ports_q::num_get_q);
	};
	

	/*
         * Implementation of the communication protocol
         */
         
         virtual void command__set_q() {
    	    buffer_sync(Ports_q::set_q);
    	    flow = get_buffer_value<float>(Ports_q::set_q, 0);
    	    buffer_clear(Ports_q::set_q);
    	    
    	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Initialized q=" << flow << endl;
    	    cout << out.str();
	}
         
        virtual void command__set_qs() {
    	    buffer_sync(Ports_q::set_qs);
    	    flow_sensor = get_buffer_value<float>(Ports_q::set_qs, 0);
    	    buffer_clear(Ports_q::set_qs);
    	    
    	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Initialized qs=" << flow_sensor << endl;
    	    cout << out.str();
	}
          
        virtual void command__set_r_reg() {
    	    buffer_sync(Ports_q::set_r_reg);
    	    r_reg = get_buffer_value<float>(Ports_q::set_r_reg, 0);
    	    buffer_clear(Ports_q::set_r_reg);
    	    
    	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Initialized r_reg=" << r_reg << endl;
    	    cout << out.str();
	}
        
	virtual void command__get_q() {
	    buffer_clear(Ports_q::get_q);
    	    add_buffer_value<float>(Ports_q::get_q, flow /*value*/);
    	    buffer_flush_collective_gather(Ports_q::get_q);
    	    buffer_clear(Ports_q::get_q);
	}


	virtual void command__get_qs() {
	    buffer_clear(Ports_q::get_qs);
    	    add_buffer_value<float>(Ports_q::get_qs, flow_sensor /*value*/);
    	    buffer_flush_collective_gather(Ports_q::get_qs);
    	    buffer_clear(Ports_q::get_qs);
	}
	
	virtual void command__get_r_reg() {
	    buffer_clear(Ports_q::get_r_reg);
    	    add_buffer_value<float>(Ports_q::get_r_reg, r_reg /*value*/);
    	    buffer_flush_collective_gather(Ports_q::get_r_reg);
    	    buffer_clear(Ports_q::get_r_reg);
	}
    
	virtual void command__simulation() {
	    int counter = 0;
            int nr_steps = solver->nr_num_steps;
            
            while (counter<nr_steps) {
                simulation_q();
            
                counter++;
                
                // storing q
                if (monitoring != nullptr) {
            	    if (mon_opts->flag_is_realtime) {
            		time_ms.increment_time_ms(solver->h); // incrementing time counter
			monitoring->add_entry(network, section, element, name, 
				       	  time_ms.time_stamp(), flow);
		    } else {
			time_ms_relative += solver->h;
			monitoring->add_entry(network, section, element, name, 
				       	  time_ms_relative, flow);
		    }
		}
            }
	}
    
	virtual void command__save() {
	    // output to screen
	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): q=" << flow << endl;
    	    cout << out.str();
    	    
    	    // output to data layer
            if (monitoring != nullptr) {
		if (mon_opts->flag_is_realtime) {
            	    time_ms.init_time();
	    	    monitoring->add_entry(network, section, element, name,
                                      time_ms.time_stamp(), flow);
                } else {
            	    monitoring->add_entry(network, section, element, name,
                                          time_ms_relative, flow);
                }
            }

	}
	
	virtual void command__stop() {
	    if (monitoring != nullptr) {
		if (mon_opts->flag_output_file)
		    output->close();	
		    
		monitoring->data_flush(id_str);
	    }
	
	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Received stop. Terminating" << endl;
    	    cout << out.str();
	}

	virtual void command__id() {
	    // output to display
	    stringstream out;
    	    out << "Hello from: ms_" << id << "(" << id_str << "): q=" << flow << endl;
    	    cout << out.str();
	}
    
	virtual void command__init_time() {
	    if (monitoring != nullptr)
		if (mon_opts->flag_is_realtime)
		    time_ms.init_time();
		else
		    time_ms_relative = 0;
	}
	
	void run() {
	    init_monitoring();
	
	    bool finish = false;
	    
	    while (!finish) {
	
		// receiving command from the Master
		buffer_sync(Ports_q::command_flow);
		int command = get_buffer_value_stack<int>(Ports_q::command_flow);
	    
		switch (command) {
		
		    // 0
		    case Commands_q::set_q: {
			command__set_q();
                	break;    
            	    }
            	    
		    // 1
            	    case Commands_q::set_qs: {
			command__set_qs();
                	break;    
            	    }
            	    
            	    // 2
            	    case Commands_q::set_r_reg: {
			command__set_r_reg();
                	break;    
            	    }

		    // 3            	    
            	    case Commands_q::get_q: {
			command__get_q();
			break;
            	    }
                
		    // 4            	    
            	    case Commands_q::get_qs: {
			command__get_qs();
			break;
            	    }
            	    
            	    // 5            	    
            	    case Commands_q::get_r_reg: {
			command__get_r_reg();
			break;
            	    }

		    // 6
		    case Commands_q::simulation: {
			command__simulation();
			break;
		    }

		    // 7		
		    case Commands_q::save: {
                	command__save();
                	break;
            	    }

		    // 8
		    case Commands_q::stop: {
    			command__stop();
    			finish = true;		
			break;
		    }

		    // 9 
		    case Commands_q::id: {
    			command__id();
    			break;
		    }    


		    // 10	    
		    case Commands_q::init_time: {
    			command__init_time();
    			break;
		    }    
		}
	    }
	}
		
    }; // class q


}; // namespace A1_1


#endif // A1_1_H