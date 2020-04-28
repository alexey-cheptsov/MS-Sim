/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef A2_1_H
#define A2_1_H

#include "commands.h"
#include "ports.h"
#include "proxies.h"

#include "../A1_1/A1_1.h"
#include "../A1_2/A1_2.h"

#include "../../mpi_deployment.h"
#include "../../Monitoring/monitoring.h"

using namespace std;
using namespace A1_1;
using namespace A2_1;

namespace A2_1 {

    /*
     * Microservice for Q - element
     */
    class Q: public Microservice {
    public:
    
	int N;		// nr. of approx_elements
	float L;	// length
	
	q** qq;		// underlying MS
	p** pp;
	
	DeploymentPool* deployment_pool;
	
	Q(int id_, string id_str_, MpiCommunicator* communicator_, 
	  string element_, string section_, string network_,
	  Monitoring_opts* mon_opts_,
	  float S_, float R_, float L_, float dX_, Solver_Params& solv_params_) 
		: Microservice(id_, id_str_, communicator_) 
        {
	    N = std::round(L_/dX_);
	    L = L_;
    	    float dX = L_/N;   // correction of deltaX value
    	    
    	    // Setup of communication map
    	    set_communications();
	    //communicator->print_comm_links();

    	    // Initialization of underlying microservices
    	    qq   = new q*[N];
    	    pp   = new p*[N-1];
    	    
    	    // Initialization of underlying microservices    
    	    for (int i=0; i<N; i++)
    		if (i==0) {
    		    qq[i] = new q(i+1/*id*/, id_str + "_q" + to_string(i), communicator_,
    			      "q" + to_string(i), element_, section_, network_,
    			      mon_opts_,
    			      S_ /*S*/, R_/L_ /*r*/, L_/N /*l*/, solv_params_);
    		} else {
    		    qq[i] = new q(i+1/*id*/, id_str + "_q" + to_string(i), communicator_,
    			      "q" + to_string(i), element_, section_, network_,
    			      mon_opts_,
    			      S_ /*S*/, R_/L_ /*r*/, L_/N /*l*/, solv_params_);
    		}
    
	    for (int i=0; i<N-1; i++)
		if (i==0) {
    		    pp[i] = new p(N+(i+1)/*id*/, id_str + "_p" + to_string(i), communicator_,
    			      "p" + to_string(i), element_, section_, network_,
    			      mon_opts_,
    			      S_ /*S*/, L_/N /*dX*/, false /*is_boundary*/, solv_params_);
    		} else {
    		    pp[i] = new p(N+(i+1)/*id*/, id_str + "_p" + to_string(i), communicator_,
    			      "p" + to_string(i), element_, section_, network_,
    			      mon_opts_,
    			      S_ /*S*/, L_/N /*dX*/, false /*is_boundary*/, solv_params_);
		}
	    	    
	    // Init underlying ms
	    init_proxies();
	    init_buffers();
	    
	    // Deployment pool initialization
    	    deployment_pool = new MpiDeploymentPool(communicator_->mpi_map);
    	    
    	    // Spawning worker threads for underlying ms
	    for (int i=0; i<N; i++) // Q
    		deployment_pool->add_ms(qq[i]);
	    for (int i=0; i<N-1; i++) // P
    		deployment_pool->add_ms(pp[i]);

	    deployment_pool->deploy_all();
	}
	
	void init_proxies() {
    	    // 
	    // Initialization of communication proxies
	    //
	    add_proxy(new LocalIntBuffer  (id, id_str,  Proxies_q::command_flow,   communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_q::set_q,          communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_q::get_q,          communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_q::set_qs,         communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_q::get_qs,         communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_q::set_r_reg,      communicator));
	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_q::get_r_reg,      communicator));

	    add_proxy(new LocalIntBuffer  (id, id_str,  Proxies_p::command_flow,   communicator));
            add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_p::set_p,          communicator));
            add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_p::get_p,          communicator));
    	}
    	
    	void init_buffers() {
    	    // 
	    // Initialization of underlying MS' communication buffers
	    //
	    for (int i=0; i<N; i++) {
        	qq[i]->add_buffer(new LocalIntBuffer  (qq[i]->id /*ms_id*/, qq[i]->id_str, 0 /*port*/, qq[i]->communicator));
                qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 1 /*port*/, qq[i]->communicator));
	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 2 /*port*/, qq[i]->communicator));
    	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 3 /*port*/, qq[i]->communicator));
                qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 4 /*port*/, qq[i]->communicator));
	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 5 /*port*/, qq[i]->communicator));
	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 6 /*port*/, qq[i]->communicator));
	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 7 /*port*/, qq[i]->communicator));
    	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 8 /*port*/, qq[i]->communicator));
	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 9 /*port*/, qq[i]->communicator));
            }

	    for (int i=0; i<N-1; i++) {
                pp[i]->add_buffer(new LocalIntBuffer  (pp[i]->id /*ms_id*/, pp[i]->id_str, 0 /*port*/, pp[i]->communicator));
	        pp[i]->add_buffer(new LocalFloatBuffer(pp[i]->id /*ms_id*/, pp[i]->id_str, 1 /*port*/, pp[i]->communicator));
                pp[i]->add_buffer(new LocalFloatBuffer(pp[i]->id /*ms_id*/, pp[i]->id_str, 2 /*port*/, pp[i]->communicator));
                pp[i]->add_buffer(new LocalFloatBuffer(pp[i]->id /*ms_id*/, pp[i]->id_str, 3 /*port*/, pp[i]->communicator));
                pp[i]->add_buffer(new LocalFloatBuffer(pp[i]->id /*ms_id*/, pp[i]->id_str, 4 /*port*/, pp[i]->communicator));
                pp[i]->add_buffer(new LocalFloatBuffer(pp[i]->id /*ms_id*/, pp[i]->id_str, 5 /*port*/, pp[i]->communicator));
	    }    
	};
	
	void set_communications() {
	    string master = id_str;
        
            // Command Flow Master->Q
            for (int i=0; i<N; i++)
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_q::command_flow  + proxy_disp /*port*/,
                                                     id_str + "_q" + to_string(i) /*rcv_id*/, Ports_q::command_flow));
        
            // Command Flow Master->P
            for (int i=0; i<N-1; i++)
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_p::command_flow  + proxy_disp /*port*/,
                                                     id_str + "_p" + to_string(i) /*rcv_id*/, Ports_p::command_flow));
        
            // Data Flow Master->Q
            for (int i=0; i<N; i++) {
        	communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_q::set_q  + proxy_disp /*port*/,
                                                     id_str + "_q" + to_string(i) /*rcv_id*/, Ports_q::set_q));
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_q::set_r_reg  + proxy_disp /*port*/,
                                                     id_str + "_q" + to_string(i) /*rcv_id*/, Ports_q::set_r_reg));
    	    }
        
            // Data Flow Master->P
            for (int i=0; i<N-1; i++)
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_p::set_p + proxy_disp  /*port*/,
	                                             id_str + "_p" + to_string(i) /*rcv_id*/, Ports_p::set_p));
        
            // Data Flow Q->Master
            for (int i=0; i<N; i++)
                communicator->add_comm_link(new CommLink(id_str + "_q" + to_string(i) /*snd_id*/, Ports_q::get_q,
                                                     master /*rcv_id*/, Proxies_q::get_q + proxy_disp /*port*/));
        
            // Data Flow P->Master
            for (int i=0; i<N-1; i++)
                communicator->add_comm_link(new CommLink(id_str + "_p" + to_string(i) /*rcv_id*/, Ports_p::get_p,
                                                     master /*snd_id*/, Proxies_p::get_p + proxy_disp  /*port*/));
        
        
            // Data Flow Q->P
            for (int i=0; i<N-1; i++) {
                communicator->add_comm_link(new CommLink(id_str + "_q"+to_string(i)/*snd_id*/, Ports_q::num_get_q,
                                                     id_str + "_p"+to_string(i) /*rcv_id*/, Ports_p::num_set_qin));
    
                communicator->add_comm_link(new CommLink(id_str + "_q"+to_string(i+1)/*snd_id*/, Ports_q::num_get_q,
                                                     id_str + "_p"+to_string(i) /*rcv_id*/, Ports_p::num_set_qout));
            }
        
            // Data Flow P->Q
            for (int i=0; i<N-1; i++) {
                communicator->add_comm_link(new CommLink(id_str + "_p"+to_string(i)/*snd_id*/, Ports_p::num_get_p,
                                                     id_str + "_q"+to_string(i+1) /*rcv_id*/,   Ports_q::num_set_pstart));
    
                communicator->add_comm_link(new CommLink(id_str + "_p"+to_string(i)/*snd_id*/, Ports_p::num_get_p,
	                                             id_str + "_q"+to_string(i)/*rcv_id*/,  Ports_q::num_set_pend));
            }                                     
    	};
	
	
	/*
         * Implementation of the communication protocol
         */
	virtual void command__set_Q() {
	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized qq = {";

	    // Receive value from Master
	    buffer_sync(Ports_Q::set_Q);
	    float value[N];
	    for (int i=0; i<N; i++)
		value[i] = get_buffer_value<float>(Ports_Q::set_Q, i/*index*/); // q[0..N]
            buffer_clear(Ports_Q::set_Q);

            //Propagation to underlying q's

            for (int i=0; i<N; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_q::set_q, value[i] /*value*/);
            }
            proxy_flush_collective_spread(Proxies_q::set_q);
            proxy_clear(Proxies_q::set_q);

            add_proxy_value<int>(Proxies_q::command_flow, Commands_q::set_q /*value*/);
            proxy_flush_collective_replicate(Proxies_q::command_flow);
            proxy_clear(Proxies_q::command_flow);

            out << "}" << endl;
            cout << out.str();
        }
        
        virtual void command__set_Qs() {
	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized qqs = {";

	    // Receive value from Master
	    buffer_sync(Ports_Q::set_Qs);
	    float value[N];
	    for (int i=0; i<N; i++)
		value[i] = get_buffer_value<float>(Ports_Q::set_Qs, i/*index*/); // qs[0..N]
            buffer_clear(Ports_Q::set_Qs);

            //Propagation to underlying qs's

            for (int i=0; i<N; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_q::set_qs, value[i] /*value*/);
            }
            proxy_flush_collective_spread(Proxies_q::set_qs);
            proxy_clear(Proxies_q::set_qs);

            add_proxy_value<int>(Proxies_q::command_flow, Commands_q::set_qs /*value*/);
            proxy_flush_collective_replicate(Proxies_q::command_flow);
            proxy_clear(Proxies_q::command_flow);

            out << "}" << endl;
            cout << out.str();
        }
        
        virtual void command__set_P() {
	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized pp = {";

	    // Receive value from Master
	    buffer_sync(Ports_Q::set_P);
            float P_set = get_buffer_value<float>(Ports_Q::set_P, 0/*index*/); // qs[0..N]
            buffer_clear(Ports_Q::set_P);

            //Propagation to underlying qs's

            for (int i=0; i<N-1; i++) {
                out << P_set << ", ";
                add_proxy_value<float>(Proxies_p::set_p, P_set /*value*/);
            }
            proxy_flush_collective_spread(Proxies_p::set_p);
            proxy_clear(Proxies_p::set_p);

            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::set_p /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);

            out << "}" << endl;
            cout << out.str();
        }
        
        virtual void command__set_R_reg() {
	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized r_reg's = {";

	    // Receive value from Master
	    buffer_sync(Ports_Q::set_R_reg);
	    float value[N];
	    for (int i=0; i<N; i++) {
		value[i] = get_buffer_value<float>(Ports_Q::set_R_reg, i/*index*/); // r_reg[0..N]
		value[i] = value[i]/L; // obtaining specific value of r_reg
	    }
            buffer_clear(Ports_Q::set_R_reg);

            //Propagation to underlying q's

            for (int i=0; i<N; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_q::set_r_reg, value[i] /*value*/);
            }
            proxy_flush_collective_spread(Proxies_q::set_r_reg);
            proxy_clear(Proxies_q::set_r_reg);

            add_proxy_value<int>(Proxies_q::command_flow, Commands_q::set_r_reg /*value*/);
            proxy_flush_collective_replicate(Proxies_q::command_flow);
            proxy_clear(Proxies_q::command_flow);

            out << "}" << endl;
            cout << out.str();
        }

        
        virtual void command__get_Q() {
            float q[N];

	    // Receive value from underlying q's
	    add_proxy_value<int>(Proxies_q::command_flow, Commands_q::get_q /*value*/);
            proxy_flush_collective_replicate(Proxies_q::command_flow);
            proxy_clear(Proxies_q::command_flow);
	    
	    proxy_sync(Proxies_q::get_q);
	    for (int i=0; i<N; i++)
                q[i] = get_proxy_value<float>(Proxies_q::get_q, i /*index*/);
            proxy_clear(Proxies_q::get_q);
            
            // Sending to Master
            for (int i=0; i<N; i++)
                add_buffer_value<float>(Ports_Q::get_Q, q[i] /*value*/);
            buffer_flush_collective_gather(Ports_Q::get_Q);
            buffer_clear(Ports_Q::get_Q);
        }
        
        virtual void command__get_Qs() {
            float qs[N];

	    // Receive value from underlying q's
	    add_proxy_value<int>(Proxies_q::command_flow, Commands_q::get_qs /*value*/);
            proxy_flush_collective_replicate(Proxies_q::command_flow);
            proxy_clear(Proxies_q::command_flow);
	    
	    proxy_sync(Proxies_q::get_qs);
	    for (int i=0; i<N; i++)
                qs[i] = get_proxy_value<float>(Proxies_q::get_qs, i /*index*/);
            proxy_clear(Proxies_q::get_qs);

            // Sending to Master
            for (int i=0; i<N; i++)
                add_buffer_value<float>(Ports_Q::get_Qs, qs[i] /*value*/);
            buffer_flush_collective_gather(Ports_Q::get_Qs);
            buffer_clear(Ports_Q::get_Qs);
        }
        
        virtual void command__get_P() {
            float p[N-1];

	    // Receive value from underlying pp
	    add_proxy_value<int>(Proxies_p::command_flow, Commands_p::get_p /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
	    
	    proxy_sync(Proxies_p::get_p);
	    for (int i=0; i<N; i++)
                p[i] = get_proxy_value<float>(Proxies_p::get_p, i /*index*/);
            proxy_clear(Proxies_p::get_p);

            // Sending to Master
            for (int i=0; i<N; i++)
                add_buffer_value<float>(Ports_Q::get_P, p[i] /*value*/);
            buffer_flush_collective_gather(Ports_Q::get_P);
            buffer_clear(Ports_Q::get_P);
        }
        
        virtual void command__get_R_reg() {
            float r_reg[N];

	    // Receive value from underlying q's
	    add_proxy_value<int>(Proxies_q::command_flow, Commands_q::get_r_reg /*value*/);
            proxy_flush_collective_replicate(Proxies_q::command_flow);
            proxy_clear(Proxies_q::command_flow);
	    
	    proxy_sync(Proxies_q::get_r_reg);
	    for (int i=0; i<N; i++)
                r_reg[i] = get_proxy_value<float>(Proxies_q::get_r_reg, i /*index*/);
            proxy_clear(Proxies_q::get_r_reg);

            // Sending to Master
            for (int i=0; i<N; i++)
                add_buffer_value<float>(Ports_Q::get_R_reg, r_reg[i] /*value*/);
            buffer_flush_collective_gather(Ports_Q::get_R_reg);
            buffer_clear(Ports_Q::get_R_reg);
        }
        
        virtual void command__simulation() {
	    add_proxy_value<int>(Proxies_q::command_flow, Commands_q::simulation /*value*/);
            proxy_flush_collective_replicate(Proxies_q::command_flow);
            proxy_clear(Proxies_q::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::simulation /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

        virtual void command__save() {
	    add_proxy_value<int>(Proxies_q::command_flow, Commands_q::save /*value*/);
            proxy_flush_collective_replicate(Proxies_q::command_flow);
            proxy_clear(Proxies_q::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::save /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

        virtual void command__stop() {
	    add_proxy_value<int>(Proxies_q::command_flow, Commands_q::stop /*value*/);
            proxy_flush_collective_replicate(Proxies_q::command_flow);
            proxy_clear(Proxies_q::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::stop /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

        virtual void command__id() {
            add_proxy_value<int>(Proxies_q::command_flow, Commands_q::id /*value*/);
            proxy_flush_collective_replicate(Proxies_q::command_flow);
            proxy_clear(Proxies_q::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::id /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

	virtual void command__init_time() {
	    add_proxy_value<int>(Proxies_q::command_flow, Commands_q::init_time /*value*/);
            proxy_flush_collective_replicate(Proxies_q::command_flow);
            proxy_clear(Proxies_q::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::init_time /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }
        
        virtual void command__flush_data() {
	    add_proxy_value<int>(Proxies_q::command_flow, Commands_q::flush_data /*value*/);
            proxy_flush_collective_replicate(Proxies_q::command_flow);
            proxy_clear(Proxies_q::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::flush_data /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }
        
	
	void run() {
            bool finish = false;

            while (!finish) {
                // receiving command from the Master
                buffer_sync(Ports_Q::command_flow);
                int command = get_buffer_value_stack<int>(Ports_Q::command_flow);

                switch (command) {
                    // 0
                    case Commands_Q::set_Q: {
                        command__set_Q();
                        break;
                    }

                    // 1
                    case Commands_Q::set_Qs: {
                        command__set_Qs();
                        break;
                    }
                    
                    // 2
                    case Commands_Q::set_P: {
                        command__set_P();
                        break;
                    }

                    // 3
                    case Commands_Q::set_R_reg: {
                        command__set_R_reg();
                        break;
                    }

                    // 4                    
                    case Commands_Q::get_Q: {
                        command__get_Q();
                        break;
                    }

                    // 5                    
                    case Commands_Q::get_Qs: {
                        command__get_Qs();
                        break;
                    }
                    
                    // 6                    
                    case Commands_Q::get_P: {
                        command__get_P();
                        break;
                    }

                    // 7                    
                    case Commands_Q::get_R_reg: {
                        command__get_R_reg();
                        break;
                    }

                    // 8
                    case Commands_Q::simulation: {
                        command__simulation();
                        break;
                    }

                    // 9                
                    case Commands_Q::save: {
                        command__save();
                        break;
                    }

                    // 10
                    case Commands_Q::stop: {
                        command__stop();
                        finish = true;
                        break;
                    }

                    // 11
                    case Commands_Q::id: {
                        command__id();
                        break;
                    }

                    // 12           
                    case Commands_Q::init_time: {
                        command__init_time();
                        break;
                    }
                    
                    // 13
                    case Commands_Q::flush_data: {
                        command__flush_data();
                        break;
                    }
                }
            }
        }

    }; // class Q


}; // namespace A2_1


#endif // A2_1_H