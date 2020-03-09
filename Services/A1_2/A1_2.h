/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef A1_2_H
#define A1_2_H

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
using namespace A1_2;

namespace A1_2 {

    /*
     * Microservice for p - approximation unit
     */
 
    class p: public Microservice {
    public:
    
	const float ro = 1.25;
    	const float a = 330; // m/s
        
	// Description of the element's environment
        string name="";
        string element="";
        string section ="";
        string network="";

	// Properties
	bool is_boundary; 	// indicates that the pressure value is fixed by a boundary condition
	float pressure; 	// Pressure
	float S; 		// Cut square
    
	float dx;   		// mean length of the connected q-approximation_elements
	
	float q_in[10]; 	// all incoming q-flows
	float q_out[10]; 	// all outcoming q-flows
	int nr_q_in, nr_q_out;
	
	Solver* solver;
	
	// Monitoring properties
        Time_MS time_ms;
        float time_ms_relative;     // time stamp for relative time option
        Monitoring* monitoring = nullptr;
        Monitoring_opts* mon_opts = nullptr;
        
        vector<fstream*> output;
        vector<Entry_to_save<float>> entries_to_save;

	p(int id_, string id_str_, float S_, float dx_, bool is_boundary_, Solver_Params solv_params_) 
	    : Microservice(id_, id_str_) 
	{
	    S = S_;
	    dx = dx_;
	
	    pressure = 0;
	    is_boundary = is_boundary_;
	
	    init_solver(solv_params_);
            
            nr_q_in = 1;
            nr_q_out = 1;
    	};
	
	p(int id_, string id_str_, Communicator* communicator_, float S_, float dx_, bool is_boundary_, Solver_Params solv_params_) 
	    : Microservice(id_, id_str_, communicator_) 
	{
	    S = S_;
	    dx = dx_;
	
	    pressure = 0;
	    is_boundary = is_boundary_;
	
	    init_solver(solv_params_);
            
            nr_q_in = communicator->get_nr_comm_links_by_receiver(id, id_str, Ports_p::num_set_qin);
            nr_q_out = communicator->get_nr_comm_links_by_receiver(id, id_str, Ports_p::num_set_qout);
	};
	
	p(int id_, string id_str_, Communicator* communicator_, 
	  string name_, string element_, string section_, string network_,
	  float S_, float dx_, bool is_boundary_, Solver_Params solv_params_) 
	    : p(id_, id_str_, communicator_, S_, dx_, is_boundary_, solv_params_)

	{
	    name = name_;
            element = element_;
            section = section_;
            network = network_;
	};
	
	p(int id_, string id_str_, Communicator* communicator_, 
	  string name_, string element_, string section_, string network_,
	  Monitoring_opts* mon_opts_,
	  float S_, float dx_, bool is_boundary_, Solver_Params solv_params_) 
	    : p(id_, id_str_, communicator_, S_, dx_, is_boundary_, solv_params_)

	{
	    name = name_;
            element = element_;
            section = section_;
            network = network_;
            
            mon_opts = mon_opts_;
            monitoring = new Monitoring(mon_opts_);
	};
    

	p(int id_, string id_str_, Communicator* communicator_, 
	  string name_, string section_, string network_,
	  Monitoring_opts* mon_opts_,
	  float S_, float dx_, bool is_boundary_, Solver_Params solv_params_) 
	    : p(id_, id_str_, communicator_, S_, dx_, is_boundary_, solv_params_)

	{
	    name = name_;
            section = section_;
            network = network_;
	    
	    mon_opts = mon_opts_;
	    monitoring = new Monitoring(mon_opts_);
	};
	
	p(int id_, string id_str_, Communicator* communicator_, 
	  string name_, string network_,
	  Monitoring_opts* mon_opts_,
	  float S_, float dx_, bool is_boundary_, Solver_Params solv_params_) 
	    : p(id_, id_str_, communicator_, S_, dx_, is_boundary_, solv_params_)

	{
	    name = name_;
            network = network_;
            
            mon_opts = mon_opts_;
            monitoring = new Monitoring(mon_opts_);
	};
    
    	void init_solver(Solver_Params params) {
            RK4* rk4 = new RK4(params);
            solver = rk4;
        };
        
        void init_monitoring() {
    	    if (monitoring != nullptr) {
    		entries_to_save.push_back(Entry_to_save<float>()); // entry for "p"
    		monitoring->ss.push_back(new stringstream());

        	if (mon_opts->flag_output_file) {
        	    output.push_back(new fstream());            // file for "p"
            	    monitoring->fout = output;
                
            	    output[0]->open("output/" + id_str + ".csv", ios::in|ios::out);
                    *output[0] << "ExperimentID;Network;Section;Element;@timestamp;" + name << endl;
                }
            }   
        }            
        
        void recv_q() {
    	    buffer_sync(Ports_p::num_set_qin);
	    buffer_sync(Ports_p::num_set_qout);
	    
    	    for (int i=0; i<nr_q_in; i++)
    	        q_in[i] = get_buffer_value_stack<float>(Ports_p::num_set_qin);
    		    
    	    for (int i=0; i<nr_q_out; i++)
    	        q_out[i] = get_buffer_value_stack<float>(Ports_p::num_set_qout);
    	        
    	    buffer_clear(Ports_p::num_set_qin);
	    buffer_clear(Ports_p::num_set_qout);
	};    
	
	void recv_k(float* k_q_in, float* k_q_out) {
	    buffer_sync(Ports_p::num_set_qin);
	    buffer_sync(Ports_p::num_set_qout);
	    
    	    for (int i=0; i<nr_q_in; i++)
    	        k_q_in[i] = get_buffer_value_stack<float>(Ports_p::num_set_qin);
    		    
    	    for (int i=0; i<nr_q_out; i++)
    	        k_q_out[i] = get_buffer_value_stack<float>(Ports_p::num_set_qout);
    	        
	    buffer_clear(Ports_p::num_set_qin);
	    buffer_clear(Ports_p::num_set_qout);
	};    
	
	void send_p() {
    	    add_buffer_value<float>(Ports_p::num_get_p, pressure /*value*/);
	    buffer_flush_collective_replicate(Ports_p::num_get_p);
	    buffer_clear(Ports_p::num_get_p);
	};
	
	void send_k(float k) {
    	    add_buffer_value<float>(Ports_p::num_get_p, k /*value*/);
	    buffer_flush_collective_replicate(Ports_p::num_get_p);
	    buffer_clear(Ports_p::num_get_p);
	};
	
	float calc_dp(float* q_in, float* q_out) {
	    float q_sum = 0;
	    	    
	    for (int i=0; i<nr_q_in; i++)
		q_sum += q_in[i];
	
	    for (int i=0; i<nr_q_out; i++)
		q_sum -= q_out[i];	
	
	    return q_sum*ro*a*a / (S*dx);    
	};
	
	float calc_dp(float* q_in, float* k_q_in, float* q_out, float* k_q_out, float h, float div_factor) {
	    float q_sum = 0;
	    	    
	    for (int i=0; i<nr_q_in; i++)
		q_sum += (q_in[i] + h*k_q_in[i]/div_factor);
	
	    for (int i=0; i<nr_q_out; i++)
		q_sum -= (q_out[i] + h*k_q_out[i]/div_factor);
	
	    return q_sum*ro*a*a / (S*dx);    
	};

	void simulation_p() {
	    float k1_q_in[10], k1_q_out[10], k2_q_in[10], k2_q_out[10], k3_q_in[10], k3_q_out[10];
	    float k1_p, k2_p, k3_p, k4_p;
	    
	    float h = solver->h;

	    send_p(); // sending p value to the neighboring q-elements
	
	    if (!is_boundary) {
	        recv_q(); // receiving q values from the neighboring q-elements
	        // k1
		k1_p = calc_dp(q_in, q_out);
		send_k(k1_p);
	    
		// k2
		recv_k(k1_q_in, k1_q_out);
		k2_p = calc_dp(q_in, k1_q_in, q_out, k1_q_out, h, 2);
		send_k(k2_p);
		
		// k3
		recv_k(k2_q_in, k2_q_out);
	        k3_p = calc_dp(q_in, k2_q_in, q_out, k2_q_out, h, 2);
    		send_k(k3_p);
	    
		// k4
		recv_k(k3_q_in, k3_q_out);
		k4_p = calc_dp(q_in, k3_q_in, q_out, k3_q_out, h, 1);
	    
		float pressure_old = pressure;
		solver->solve(&pressure, k1_p, k2_p, k3_p, k4_p);
		
		//if (id_str == "p1") {
	    	//    stringstream out;
	    	//    
	    	//    out << id_str << ": p_old =" << pressure_old << ", p_new =" << pressure << endl
	    	//        << "q_in = " << q_in[0] << ", q_out = " << q_out[0] << " --> k1 = " << k1_p << endl
	    	//        << "k1_q_in = " << k1_q_in[0] << ", k1_q_out =" << k1_q_out[0] << " --> k2 = " << k2_p << endl
	    	//        << "k2_q_in = " << k2_q_in[0] << ", k2_q_out =" << k2_q_out[0] << " --> k3 = " << k3_p << endl
	    	//        << "k3_q_in = " << k3_q_in[0] << ", k3_q_out =" << k3_q_out[0] << " --> k4 = " << k4_p << endl << endl;	    	    
        	//    cout << out.str();
    		//}
	    } else {
		send_k(0); // k1
		send_k(0); // k2
		send_k(0); // k3
	    }
	};
	
	/*
         * Implementation of the communication protocol
         */

	virtual void command__stop() {
	    if (monitoring != nullptr) {
		monitoring->data_flush();
		
		if (mon_opts->flag_output_file)
		    for (int i=0; i<output.size(); i++)
                        output[i]->close();
	    }
	
	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): Received stop. Terminating" << endl;
    	    cout << out.str();
	};
	
	virtual void command__set_p() {
	    buffer_sync(Ports_p::set_p);
            pressure = get_buffer_value<float>(Ports_p::set_p, 0);
            buffer_clear(Ports_p::set_p);

	    stringstream out;
            out << "ms" << id << "(" << id_str << "): Initialized p=" << pressure << ", boundary=" << is_boundary << endl;
            cout << out.str();    
	};
	
	virtual void command__get_p() {
	    //stringstream out;
            //out << "ms_" << id << "(" << id_str << "): reporting P = " << P << endl;
            //cout << out.str();
		    
	    buffer_clear(Ports_p::get_p);
            add_buffer_value<float>(Ports_p::get_p, pressure /*value*/);
            buffer_flush_collective_gather(Ports_p::get_p);
            buffer_clear(Ports_p::get_p);
	};

	
	
	virtual void command__simulation() {
    	    int counter = 0;
            int nr_steps = solver->nr_num_steps;

            while (counter<nr_steps) {
                simulation_p();

                counter++;

		// storing results
                if (monitoring != nullptr) {
            	    entries_to_save[0].id = name;
                    entries_to_save[0].value = pressure;
                
            	    if (mon_opts->flag_is_realtime) {
            		time_ms.increment_time_ms(solver->h);
                        monitoring->add_entry<float>(network, section, element,
                                          time_ms.time_stamp(), entries_to_save);
                    } else {
                	time_ms_relative += solver->h;
                        monitoring->add_entry(network, section, element,
                                          time_ms_relative, entries_to_save);
                    }
                }
            }
	};
	
	virtual void command__save() {
	    // output to screen
	    stringstream out;
    	    out << "ms_" << id << "(" << id_str << "): p=" << pressure << endl;
    	    cout << out.str();
    	    
    	    // output to data layer
            if (monitoring != nullptr) {
                entries_to_save[0].id = name;
                entries_to_save[0].value = pressure;
                
                if (mon_opts->flag_is_realtime) {
                    monitoring->add_entry<float>(network, section, element,
                                      time_ms.time_stamp(), entries_to_save);
                } else {
            	    time_ms_relative += solver->h;
                    monitoring->add_entry(network, section, element,
                                      time_ms_relative, entries_to_save);
                }
            }
	};
	
	virtual void command__id() {
	    stringstream out;
            out << "Hello from: ms" << id << "(" << id_str << ")" << endl;
            cout << out.str();    
	};
	
	
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
		buffer_sync(Ports_p::command_flow);
		int command = get_buffer_value_stack<int>(Ports_p::command_flow);

		switch (command) {
		
		    // 0
		    case Commands_p::set_p: {
			command__set_p();
			break;
		    }
		    
		    // 1
		    case Commands_p::get_p: {
			command__get_p();
			break;
            	    }
            	    
            	    // 2
		    case Commands_p::simulation: {
			command__simulation();		
			break;
		    }
		    
		    // 3
		    case Commands_p::save: {
			command__save();
			break;
		    }
		    
		    // 4
		    case Commands_p::stop: {
			finish = true;
			command__stop();
			break;
		    }

		    // 5
		    case Commands_p::id: {
			command__id();
			break;    
		    }
		    
		    // 6
		    case Commands_p::init_time: {
                        command__init_time();
                        break;
                    }
		}
	    }
	}
    }; // class p

}; // namespace A1_2




#endif // A1_2_H