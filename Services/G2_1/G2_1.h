/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G2_1_H
#define G2_1_H

#include "commands.h"
#include "ports.h"
#include "proxies.h"

#include "../G1_1/G1_1.h"
#include "../A1_2/A1_2.h"

#include "../../mpi_deployment.h"
#include "../../Monitoring/monitoring.h"

using namespace std;
using namespace G1_1;
using namespace A1_2;

namespace G2_1 {

    /*
     * Microservice for Qm - element
     */
    class Qm: public Microservice {
    public:
    
	int N;		// nr. of approx_elements
	
	qm** qq;	// underlying MS
	p** pp;
	
	DeploymentPool* deployment_pool;
	
	Qm(int id_, string id_str_, MpiCommunicator* communicator_, 
	  string element_, string section_, string network_,
	  Monitoring_opts* mon_opts_,
	  float S_, float R_, float L_, float dX_, 
	  float A_, float BRf_, Solver_Params& solv_params_) 
		: Microservice(id_, id_str_, communicator_) 
        {
	    N = std::round(L_/dX_);
    	    float dX = L_/N;   // correction of deltaX value
    	    
    	    // Setup of communication map
    	    set_communications();
	    //communicator->print_comm_links();

    	    // Initialization of underlying microservices
    	    qq   = new qm*[N];
    	    pp   = new p* [N-1];
    	    
    	    // Initialization of underlying microservices    
    	    for (int i=0; i<N; i++)
    		if (i==0) {
    		    qq[i] = new qm(i+1/*id*/, id_str + "_qm" + to_string(i), id_str + "_q" + to_string(i), communicator_,
    			       "qm" + to_string(i), "q" + to_string(i), element_, section_, network_,
    			       mon_opts_,
    			       S_ /*S*/, R_/L_ /*r*/, L_/N /*l*/, A_, BRf_, solv_params_);
    		} else {
    		    qq[i] = new qm(i+1/*id*/, id_str + "_qm" + to_string(i), id_str + "_q" + to_string(i), communicator_,
    			       "qm" + to_string(i), "q" + to_string(i), element_, section_, network_,
    			       mon_opts_,
    			       S_ /*S*/, R_/L_ /*r*/, L_/N /*l*/, A_, BRf_, solv_params_);
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
	
	Qm(int id_, string id_str_, MpiCommunicator* communicator_, 
	  string element_, string section_, string network_,
	  Monitoring_opts* mon_opts_,
	  float S_, float R_, float L_, float dX_, 
	  float V_, Solver_Params& solv_params_) 
		: Microservice(id_, id_str_, communicator_) 
        {
	    N = std::round(L_/dX_);
    	    float dX = L_/N;   // correction of deltaX value
    	    
    	    // Setup of communication map
    	    set_communications();
	    //communicator->print_comm_links();

    	    // Initialization of underlying microservices
    	    qq   = new qm*[N];
    	    pp   = new p* [N-1];
    	    
    	    // Initialization of underlying microservices    
    	    for (int i=0; i<N; i++)
    		qq[i] = new qm(i+1/*id*/, id_str + "_qm" + to_string(i), id_str + "_q" + to_string(i), communicator_,
    			       "qm" + to_string(i), "q" + to_string(i), element_, section_, network_,
    			       mon_opts_,
    			       S_ /*S*/, R_/L_ /*r*/, L_/N /*l*/, V_, solv_params_);
    
	    for (int i=0; i<N-1; i++)
    		pp[i] = new p(N+(i+1)/*id*/, id_str + "_p" + to_string(i), communicator_,
    			      "p" + to_string(i), element_, section_, network_,
    			      mon_opts_,
    			      S_ /*S*/, L_/N /*dX*/, false /*is_boundary*/, solv_params_);
	    	    
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
	    add_proxy(new LocalIntBuffer  (id, id_str,  Proxies_qm::command_flow,  communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qm::set_q,         communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qm::get_q,         communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qm::set_qm,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qm::get_qm,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qm::set_qm0,       communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qm::get_qm0,       communicator));

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
	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 10/*port*/, qq[i]->communicator));
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
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_qm::command_flow  + proxy_disp /*port*/,
                                                     id_str + "_qm" + to_string(i) /*rcv_id*/, Ports_qm::command_flow));
        
            // Command Flow Master->P
            for (int i=0; i<N-1; i++)
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_p::command_flow  + proxy_disp /*port*/,
                                                     id_str + "_p" + to_string(i) /*rcv_id*/, Ports_p::command_flow));
        
            // Data Flow Master->Q
            for (int i=0; i<N; i++){
        	communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_qm::set_q  + proxy_disp /*port*/,
                                                     id_str + "_qm" + to_string(i) /*rcv_id*/, Ports_qm::set_q));
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_qm::set_qm  + proxy_disp /*port*/,
                                                     id_str + "_qm" + to_string(i) /*rcv_id*/, Ports_qm::set_qm));
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_qm::set_qm0  + proxy_disp /*port*/,
                                                     id_str + "_qm" + to_string(i) /*rcv_id*/, Ports_qm::set_qm0));
    	    }
            // Data Flow Master->P
            for (int i=0; i<N-1; i++)
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_p::set_p + proxy_disp  /*port*/,
	                                             id_str + "_p" + to_string(i) /*rcv_id*/, Ports_p::set_p));
        
            // Data Flow Q->Master
            for (int i=0; i<N; i++) {
                communicator->add_comm_link(new CommLink(id_str + "_qm" + to_string(i) /*snd_id*/, Ports_qm::get_q,
                                                     master /*rcv_id*/, Proxies_qm::get_q + proxy_disp /*port*/));
                communicator->add_comm_link(new CommLink(id_str + "_qm" + to_string(i) /*snd_id*/, Ports_qm::get_qm,
                                                     master /*rcv_id*/, Proxies_qm::get_qm + proxy_disp /*port*/));
		communicator->add_comm_link(new CommLink(id_str + "_qm" + to_string(i) /*snd_id*/, Ports_qm::get_qm0,
                                                     master /*rcv_id*/, Proxies_qm::get_qm0 + proxy_disp /*port*/));
    	    }
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
	    buffer_sync(Ports_Qm::set_Q);
            float value[N];
            for (int i=0; i<N; i++)
        	value[i] = get_buffer_value<float>(Ports_Qm::set_Q, i/*index*/); // q[0..N]
            buffer_clear(Ports_Qm::set_Q);

            //Propagation to underlying q's
            for (int i=0; i<N; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_qm::set_q, value[i] /*value*/);
            }
            proxy_flush_collective_spread(Proxies_qm::set_q);
            proxy_clear(Proxies_qm::set_q);

            add_proxy_value<int>(Proxies_qm::command_flow, Commands_qm::set_q /*value*/);
            proxy_flush_collective_replicate(Proxies_qm::command_flow);
            proxy_clear(Proxies_qm::command_flow);

            out << "}" << endl;
            cout << out.str();
        }
        
	virtual void command__set_Qm() {
	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized qm = {";

	    // Receive value from Master
	    buffer_sync(Ports_Qm::set_Qm);
	    float value[N];
            for (int i=0; i<N; i++)
        	value[i] = get_buffer_value<float>(Ports_Qm::set_Qm, i/*index*/); // q[0..N]
            buffer_clear(Ports_Qm::set_Qm);

            //Propagation to underlying q's
            for (int i=0; i<N; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_qm::set_qm, value[i] /*value*/);
            }
            proxy_flush_collective_spread(Proxies_qm::set_qm);
            proxy_clear(Proxies_qm::set_qm);

            add_proxy_value<int>(Proxies_qm::command_flow, Commands_qm::set_qm /*value*/);
            proxy_flush_collective_replicate(Proxies_qm::command_flow);
            proxy_clear(Proxies_qm::command_flow);

            out << "}" << endl;
            cout << out.str();
        }

	virtual void command__set_Qm0() {
	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized qm0 = {";

	    // Receive value from Master
	    buffer_sync(Ports_Qm::set_Qm0);
	    float value[N];
            for (int i=0; i<N; i++)
        	value[i] = get_buffer_value<float>(Ports_Qm::set_Qm0, i/*index*/); // q[0..N]
            buffer_clear(Ports_Qm::set_Qm0);

            //Propagation to underlying q's
            for (int i=0; i<N; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_qm::set_qm0, value[i] /*value*/);
            }
            proxy_flush_collective_spread(Proxies_qm::set_qm0);
            proxy_clear(Proxies_qm::set_qm0);

            add_proxy_value<int>(Proxies_qm::command_flow, Commands_qm::set_qm0 /*value*/);
            proxy_flush_collective_replicate(Proxies_qm::command_flow);
            proxy_clear(Proxies_qm::command_flow);

            out << "}" << endl;
            cout << out.str();
        }
        
        virtual void command__set_P() {
	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized pp = {";

	    // Receive value from Master
	    buffer_sync(Ports_Qm::set_P);
	    float value[N];
            for (int i=0; i<N; i++)
        	value[i] = get_buffer_value<float>(Ports_Qm::set_P, i/*index*/); // qs[0..N]
            buffer_clear(Ports_Qm::set_P);

            //Propagation to underlying qs's
            for (int i=0; i<N-1; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_p::set_p, value[i] /*value*/);
            }
            proxy_flush_collective_spread(Proxies_p::set_p);
            proxy_clear(Proxies_p::set_p);

            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::set_p /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);

            out << "}" << endl;
            cout << out.str();
        }
        
        virtual void command__get_Q() {
            float q[N];

	    // Receive value from underlying q's
	    add_proxy_value<int>(Proxies_qm::command_flow, Commands_qm::get_q /*value*/);
            proxy_flush_collective_replicate(Proxies_qm::command_flow);
            proxy_clear(Proxies_qm::command_flow);
	    
	    proxy_sync(Proxies_qm::get_q);
	    for (int i=0; i<N; i++)
                q[i] = get_proxy_value<float>(Proxies_qm::get_q, i /*index*/);
            proxy_clear(Proxies_qm::get_q);
            
            // Sending to Master
            for (int i=0; i<N; i++)
                add_buffer_value<float>(Ports_Qm::get_Q, q[i] /*value*/);
            buffer_flush_collective_gather(Ports_Qm::get_Q);
            buffer_clear(Ports_Qm::get_Q);
        }
        
        virtual void command__get_Qm() {
            float q[N];

	    // Receive value from underlying q's
	    add_proxy_value<int>(Proxies_qm::command_flow, Commands_qm::get_qm /*value*/);
            proxy_flush_collective_replicate(Proxies_qm::command_flow);
            proxy_clear(Proxies_qm::command_flow);
	    
	    proxy_sync(Proxies_qm::get_qm);
	    for (int i=0; i<N; i++)
                q[i] = get_proxy_value<float>(Proxies_qm::get_qm, i /*index*/);
            proxy_clear(Proxies_qm::get_qm);
            
            // Sending to Master
            for (int i=0; i<N; i++)
                add_buffer_value<float>(Ports_Qm::get_Qm, q[i] /*value*/);
            buffer_flush_collective_gather(Ports_Qm::get_Qm);
            buffer_clear(Ports_Qm::get_Qm);
        }
        
        virtual void command__get_Qm0() {
            float q[N];

	    // Receive value from underlying q's
	    add_proxy_value<int>(Proxies_qm::command_flow, Commands_qm::get_qm0 /*value*/);
            proxy_flush_collective_replicate(Proxies_qm::command_flow);
            proxy_clear(Proxies_qm::command_flow);
	    
	    proxy_sync(Proxies_qm::get_qm0);
	    for (int i=0; i<N; i++)
                q[i] = get_proxy_value<float>(Proxies_qm::get_qm0, i /*index*/);
            proxy_clear(Proxies_qm::get_qm0);
            
            // Sending to Master
            for (int i=0; i<N; i++)
                add_buffer_value<float>(Ports_Qm::get_Qm0, q[i] /*value*/);
            buffer_flush_collective_gather(Ports_Qm::get_Qm0);
            buffer_clear(Ports_Qm::get_Qm0);
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
                add_buffer_value<float>(Ports_Qm::get_P, p[i] /*value*/);
            buffer_flush_collective_gather(Ports_Qm::get_P);
            buffer_clear(Ports_Qm::get_P);
        }
        
        virtual void command__simulation() {
	    add_proxy_value<int>(Proxies_qm::command_flow, Commands_qm::simulation /*value*/);
            proxy_flush_collective_replicate(Proxies_qm::command_flow);
            proxy_clear(Proxies_qm::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::simulation /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

        virtual void command__save() {
	    add_proxy_value<int>(Proxies_qm::command_flow, Commands_qm::save /*value*/);
            proxy_flush_collective_replicate(Proxies_qm::command_flow);
            proxy_clear(Proxies_qm::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::save /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

        virtual void command__stop() {
	    add_proxy_value<int>(Proxies_qm::command_flow, Commands_qm::stop /*value*/);
            proxy_flush_collective_replicate(Proxies_qm::command_flow);
            proxy_clear(Proxies_qm::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::stop /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

        virtual void command__id() {
            add_proxy_value<int>(Proxies_qm::command_flow, Commands_qm::id /*value*/);
            proxy_flush_collective_replicate(Proxies_qm::command_flow);
            proxy_clear(Proxies_qm::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::id /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

	virtual void command__init_time() {
	    add_proxy_value<int>(Proxies_qm::command_flow, Commands_q::init_time /*value*/);
            proxy_flush_collective_replicate(Proxies_qm::command_flow);
            proxy_clear(Proxies_qm::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::init_time /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

	virtual void command__flush_data() {
	    add_proxy_value<int>(Proxies_qm::command_flow, Commands_q::flush_data /*value*/);
            proxy_flush_collective_replicate(Proxies_qm::command_flow);
            proxy_clear(Proxies_qm::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::flush_data /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

	
	void run() {
            bool finish = false;

            while (!finish) {
                // receiving command from the Master
                buffer_sync(Ports_Qm::command_flow);
                int command = get_buffer_value_stack<int>(Ports_Qm::command_flow);

                switch (command) {
                    // 0
                    case Commands_Qm::set_Q: {
                        command__set_Q();
                        break;
                    }

                    // 1
                    case Commands_Qm::set_Qm: {
                        command__set_Qm();
                        break;
                    }
                    
                    // 2
                    case Commands_Qm::set_Qm0: {
                        command__set_Qm0();
                        break;
                    }
                    
                    // 3
                    case Commands_Qm::set_P: {
                        command__set_P();
                        break;
                    }

                    // 4                    
                    case Commands_Qm::get_Q: {
                        command__get_Q();
                        break;
                    }

                    // 5                    
                    case Commands_Qm::get_Qm: {
                        command__get_Qm();
                        break;
                    }
                    
                    // 6                    
                    case Commands_Qm::get_Qm0: {
                        command__get_Qm0();
                        break;
                    }

                    // 7                    
                    case Commands_Qm::get_P: {
                        command__get_P();
                        break;
                    }

                    // 8
                    case Commands_Qm::simulation: {
                        command__simulation();
                        break;
                    }

                    // 9                
                    case Commands_Qm::save: {
                        command__save();
                        break;
                    }

                    // 10
                    case Commands_Qm::stop: {
                        command__stop();
                        finish = true;
                        break;
                    }

                    // 11
                    case Commands_Qm::id: {
                        command__id();
                        break;
                    }

                    // 12           
                    case Commands_Qm::init_time: {
                        command__init_time();
                        break;
                    }

                    // 13           
                    case Commands_Qm::flush_data: {
                        command__flush_data();
                        break;
                    }
                }
            }
        }

    }; // class Qm


}; // namespace G2_1


#endif // G2_1_H