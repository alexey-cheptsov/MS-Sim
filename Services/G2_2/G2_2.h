/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G2_2_H
#define G2_2_H

#include "commands.h"
#include "ports.h"
#include "proxies.h"

#include "../G1_2/G1_2.h"
#include "../A1_2/A1_2.h"

#include "../../mpi_deployment.h"
#include "../../Monitoring/monitoring.h"

using namespace std;
using namespace G1_2;
using namespace A1_2;

namespace G2_2 {

    /*
     * Microservice for Qmt - element
     */
    class Qmt: public Microservice {
    public:
    
	int N;		// nr. of approx_elements
	
	qmt** qq;	// underlying MS
	p**   pp;
	
	DeploymentPool* deployment_pool;
	
	Qmt(int id_, string id_str_, MpiCommunicator* communicator_, 
	  string element_, string section_, string network_,
	  Monitoring_opts* mon_opts_,
	  float S_, float R_, float L_, float dX_, 
	  Solver_Params& solv_params_) 
		: Microservice(id_, id_str_, communicator_) 
        {
	    N = std::round(L_/dX_);
    	    float dX = L_/N;   // correction of deltaX value
    	    
    	    // Setup of communication map
    	    set_communications();
	    //communicator->print_comm_links();

    	    // Initialization of underlying microservices
    	    qq   = new qmt*[N];
    	    pp   = new p*  [N-1];
    	    
    	    // Initialization of underlying microservices    
    	    for (int i=0; i<N; i++)
    		if (i==0) {
    		    qq[i] = new qmt(i+1/*id*/, id_str + "_qmt" + to_string(i), id_str + "_q" + to_string(i), communicator_,
    			        "qmt" + to_string(i), "q" + to_string(i), element_, section_, network_,
    			        mon_opts_,
    			        S_ /*S*/, R_/L_ /*r*/, L_/N /*l*/, solv_params_);
    		} else {
    		    qq[i] = new qmt(i+1/*id*/, id_str + "_qmt" + to_string(i), id_str + "_q" + to_string(i), communicator_,
    			        "qmt" + to_string(i), "q" + to_string(i), element_, section_, network_,
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
	    add_proxy(new LocalIntBuffer  (id, id_str,  Proxies_qmt::command_flow,  communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qmt::set_q,         communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qmt::get_q,         communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qmt::set_qm,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qmt::get_qm,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qmt::set_qs,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qmt::get_qs,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qmt::set_qms,       communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qmt::get_qms,       communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qmt::set_r_reg,     communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_qmt::get_r_reg,     communicator));

	    add_proxy(new LocalIntBuffer  (id, id_str,  Proxies_p::command_flow,    communicator));
            add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_p::set_p,           communicator));
            add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_p::get_p,           communicator));
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
    	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 10 /*port*/,qq[i]->communicator));
    	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 11 /*port*/,qq[i]->communicator));
	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 12 /*port*/,qq[i]->communicator));
    	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 13 /*port*/,qq[i]->communicator));
    	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 14 /*port*/,qq[i]->communicator));
    	        qq[i]->add_buffer(new LocalFloatBuffer(qq[i]->id /*ms_id*/, qq[i]->id_str, 15 /*port*/,qq[i]->communicator));
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
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_qmt::command_flow  + proxy_disp /*port*/,
                                                     id_str + "_qmt" + to_string(i) /*rcv_id*/, Ports_qmt::command_flow));
        
            // Command Flow Master->P
            for (int i=0; i<N-1; i++)
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_p::command_flow  + proxy_disp /*port*/,
                                                     id_str + "_p" + to_string(i) /*rcv_id*/, Ports_p::command_flow));
        
            // Data Flow Master->Q
            for (int i=0; i<N; i++){
        	communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_qmt::set_q  + proxy_disp /*port*/,
                                                     id_str + "_qmt" + to_string(i) /*rcv_id*/, Ports_qmt::set_q));
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_qmt::set_qm  + proxy_disp /*port*/,
                                                     id_str + "_qmt" + to_string(i) /*rcv_id*/, Ports_qmt::set_qm));
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_qmt::set_qs  + proxy_disp /*port*/,
                                                     id_str + "_qmt" + to_string(i) /*rcv_id*/, Ports_qmt::set_qs));
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_qmt::set_qms  + proxy_disp /*port*/,
                                                     id_str + "_qmt" + to_string(i) /*rcv_id*/, Ports_qmt::set_qms));
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_qmt::set_r_reg  + proxy_disp /*port*/,
                                                     id_str + "_qmt" + to_string(i) /*rcv_id*/, Ports_qmt::set_r_reg));
    	    }
            // Data Flow Master->P
            for (int i=0; i<N-1; i++)
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_p::set_p + proxy_disp  /*port*/,
	                                             id_str + "_p" + to_string(i) /*rcv_id*/, Ports_p::set_p));
        
            // Data Flow Q->Master
            for (int i=0; i<N; i++) {
                communicator->add_comm_link(new CommLink(id_str + "_qmt" + to_string(i) /*snd_id*/, Ports_qmt::get_q,
                                                     master /*rcv_id*/, Proxies_qmt::get_q + proxy_disp /*port*/));
                communicator->add_comm_link(new CommLink(id_str + "_qmt" + to_string(i) /*snd_id*/, Ports_qmt::get_qm,
                                                     master /*rcv_id*/, Proxies_qmt::get_qm + proxy_disp /*port*/));
                communicator->add_comm_link(new CommLink(id_str + "_qmt" + to_string(i) /*snd_id*/, Ports_qmt::get_qs,
                                                     master /*rcv_id*/, Proxies_qmt::get_qs + proxy_disp /*port*/));
		communicator->add_comm_link(new CommLink(id_str + "_qmt" + to_string(i) /*snd_id*/, Ports_qmt::get_qms,
                                                     master /*rcv_id*/, Proxies_qmt::get_qms + proxy_disp /*port*/));
                communicator->add_comm_link(new CommLink(id_str + "_qmt" + to_string(i) /*snd_id*/, Ports_qmt::get_r_reg,
                                                     master /*rcv_id*/, Proxies_qmt::get_r_reg + proxy_disp /*port*/));                                         
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
            
            // Data Flow qmt->qmt (transport)
            for (int i=0; i<N-1; i++) {
                communicator->add_comm_link(new CommLink(id_str + "_qmt"+to_string(i)/*snd_id*/, Ports_qmt::gas_get_qm,
            						id_str + "_qmt"+to_string(i+1)/*snd_id*/, Ports_qmt::gas_set_qm));
            }                                   
    	};
	
	
	/*
         * Implementation of the communication protocol
         */
	virtual void command__set_Q() {
    	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized qq = {";

    	    // Receive value from Master
    	    buffer_sync(Ports_Qmt::set_Q);
            float value[N];
            for (int i=0; i<N; i++)
    		value[i] = get_buffer_value<float>(Ports_Qmt::set_Q, i/*index*/); // q[0..N]
            buffer_clear(Ports_Qmt::set_Q);

            //Propagation to underlying q's
            for (int i=0; i<N; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_qmt::set_q, value[i] /*value*/);
            }
            proxy_flush_collective_spread(Proxies_qmt::set_q);
            proxy_clear(Proxies_qmt::set_q);

            add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::set_q /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);

            out << "}" << endl;
            cout << out.str();
        }
        
        virtual void command__set_Qm() {
	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized qm = {";

    	    // Receive value from Master
    	    buffer_sync(Ports_Qmt::set_Qm);
    	    float value[N];
            for (int i=0; i<N; i++)
    		value[i] = get_buffer_value<float>(Ports_Qmt::set_Qm, i/*index*/); // q[0..N]
            buffer_clear(Ports_Qmt::set_Qm);

            //Propagation to underlying q's
            for (int i=0; i<N; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_qmt::set_qm, value[i] /*value*/);
            }
            proxy_flush_collective_spread(Proxies_qmt::set_qm);
            proxy_clear(Proxies_qmt::set_qm);

            add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::set_qm /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);

            out << "}" << endl;
            cout << out.str();
        }
        
        virtual void command__set_Qs() {
	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized qs = {";

    	    // Receive value from Master
    	    buffer_sync(Ports_Qmt::set_Qs);
    	    float value[N];
            for (int i=0; i<N; i++)
    		value[i] = get_buffer_value<float>(Ports_Qmt::set_Qs, i/*index*/); // q[0..N]
            buffer_clear(Ports_Qmt::set_Qs);

            //Propagation to underlying q's
            for (int i=0; i<N; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_qmt::set_qs, value[i] /*value*/);
            }
            proxy_flush_collective_spread(Proxies_qmt::set_qs);
            proxy_clear(Proxies_qmt::set_qs);

            add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::set_qs /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);

            out << "}" << endl;
            cout << out.str();
        }

	virtual void command__set_Qms() {
    	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized qms = {";

            // Receive value from Master
	    buffer_sync(Ports_Qmt::set_Qms);
    	    float value[N];
            for (int i=0; i<N; i++)
    		value[i] = get_buffer_value<float>(Ports_Qmt::set_Qms, i/*index*/); // q[0..N]
            buffer_clear(Ports_Qmt::set_Qms);

            //Propagation to underlying q's
            for (int i=0; i<N; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_qmt::set_qms, value[i] /*value*/);
            }
            proxy_flush_collective_spread(Proxies_qmt::set_qms);
            proxy_clear(Proxies_qmt::set_qms);

            add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::set_qms /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);

            out << "}" << endl;
            cout << out.str();
        }
    
	virtual void command__set_R_reg() {
    	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized r_reg = {";

            // Receive value from Master
	    buffer_sync(Ports_Qmt::set_R_reg);
    	    float value[N];
            for (int i=0; i<N; i++)
    	    value[i] = get_buffer_value<float>(Ports_Qmt::set_R_reg, i/*index*/); // q[0..N]
            buffer_clear(Ports_Qmt::set_R_reg);

            //Propagation to underlying q's
            for (int i=0; i<N; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_qmt::set_r_reg, value[i] /*value*/);
            }
            proxy_flush_collective_spread(Proxies_qmt::set_r_reg);
            proxy_clear(Proxies_qmt::set_r_reg);

            add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::set_r_reg /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);

            out << "}" << endl;
            cout << out.str();
        }
        
        virtual void command__set_P() {
    	    stringstream out;
            out << "ms_" << id << "(" << id_str << "): Initialized pp = {";

    	    // Receive value from Master
    	    buffer_sync(Ports_Qmt::set_P);
    	    float value[N];
            for (int i=0; i<N; i++)
    		value[i] = get_buffer_value<float>(Ports_Qmt::set_P, i/*index*/); // qs[0..N]
            buffer_clear(Ports_Qmt::set_P);

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
	    add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::get_q /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);
	    
	    proxy_sync(Proxies_qmt::get_q);
	    for (int i=0; i<N; i++)
                q[i] = get_proxy_value<float>(Proxies_qmt::get_q, i /*index*/);
            proxy_clear(Proxies_qmt::get_q);
            
            // Sending to Master
            for (int i=0; i<N; i++)
                add_buffer_value<float>(Ports_Qmt::get_Q, q[i] /*value*/);
            buffer_flush_collective_gather(Ports_Qmt::get_Q);
            buffer_clear(Ports_Qmt::get_Q);
        }
        
        virtual void command__get_Qm() {
            float q[N];

	    // Receive value from underlying q's
	    add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::get_qm /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);
	    
	    proxy_sync(Proxies_qmt::get_qm);
	    for (int i=0; i<N; i++)
                q[i] = get_proxy_value<float>(Proxies_qmt::get_qm, i /*index*/);
            proxy_clear(Proxies_qmt::get_qm);
            
            // Sending to Master
            for (int i=0; i<N; i++)
                add_buffer_value<float>(Ports_Qmt::get_Qm, q[i] /*value*/);
            buffer_flush_collective_gather(Ports_Qmt::get_Qm);
            buffer_clear(Ports_Qmt::get_Qm);
        }
        
        virtual void command__get_Qs() {
            float q[N];

	    // Receive value from underlying q's
	    add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::get_qs /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);
	    
	    proxy_sync(Proxies_qmt::get_qs);
	    for (int i=0; i<N; i++)
                q[i] = get_proxy_value<float>(Proxies_qmt::get_qs, i /*index*/);
            proxy_clear(Proxies_qmt::get_qs);
            
            // Sending to Master
            for (int i=0; i<N; i++)
                add_buffer_value<float>(Ports_Qmt::get_Qs, q[i] /*value*/);
            buffer_flush_collective_gather(Ports_Qmt::get_Qs);
            buffer_clear(Ports_Qmt::get_Qs);
        }
        
        virtual void command__get_Qms() {
            float q[N];

	    // Receive value from underlying q's
	    add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::get_qms /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);
	    
	    proxy_sync(Proxies_qmt::get_qms);
	    for (int i=0; i<N; i++)
                q[i] = get_proxy_value<float>(Proxies_qmt::get_qms, i /*index*/);
            proxy_clear(Proxies_qmt::get_qms);
            
            // Sending to Master
            for (int i=0; i<N; i++)
                add_buffer_value<float>(Ports_Qmt::get_Qms, q[i] /*value*/);
            buffer_flush_collective_gather(Ports_Qmt::get_Qms);
            buffer_clear(Ports_Qmt::get_Qms);
        }
        
        virtual void command__get_R_reg() {
            float value[N];

	    // Receive value from underlying q's
	    add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::get_r_reg /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);
	    
	    proxy_sync(Proxies_qmt::get_r_reg);
	    for (int i=0; i<N; i++)
                value[i] = get_proxy_value<float>(Proxies_qmt::get_r_reg, i /*index*/);
            proxy_clear(Proxies_qmt::get_r_reg);
            
            // Sending to Master
            for (int i=0; i<N; i++)
                add_buffer_value<float>(Ports_Qmt::get_R_reg, value[i] /*value*/);
            buffer_flush_collective_gather(Ports_Qmt::get_R_reg);
            buffer_clear(Ports_Qmt::get_R_reg);
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
                add_buffer_value<float>(Ports_Qmt::get_P, p[i] /*value*/);
            buffer_flush_collective_gather(Ports_Qmt::get_P);
            buffer_clear(Ports_Qmt::get_P);
        }
        
        virtual void command__simulation() {
	    add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::simulation /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::simulation /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

        virtual void command__save() {
	    add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::save /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::save /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

        virtual void command__stop() {
	    add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::stop /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::stop /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

        virtual void command__id() {
            add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::id /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::id /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

	virtual void command__init_time() {
	    add_proxy_value<int>(Proxies_qmt::command_flow, Commands_qmt::init_time /*value*/);
            proxy_flush_collective_replicate(Proxies_qmt::command_flow);
            proxy_clear(Proxies_qmt::command_flow);
            
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::init_time /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

	
	void run() {
            bool finish = false;

            while (!finish) {
                // receiving command from the Master
                buffer_sync(Ports_Qmt::command_flow);
                int command = get_buffer_value_stack<int>(Ports_Qmt::command_flow);

                switch (command) {
                    // 0
                    case Commands_Qmt::set_Q: {
                        command__set_Q();
                        break;
                    }
                    
                    // 1
                    case Commands_Qmt::set_Qm: {
                        command__set_Qm();
                        break;
                    }

                    // 2
                    case Commands_Qmt::set_Qs: {
                        command__set_Qs();
                        break;
                    }
                    
                    // 3
                    case Commands_Qmt::set_Qms: {
                        command__set_Qms();
                        break;
                    }
                    
                    // 4
                    case Commands_Qmt::set_R_reg: {
                        command__set_R_reg();
                        break;
                    }
                    
                    // 5
                    case Commands_Qmt::set_P: {
                        command__set_P();
                        break;
                    }

                    // 6                    
                    case Commands_Qmt::get_Q: {
                        command__get_Q();
                        break;
                    }

		    // 7                 
                    case Commands_Qmt::get_Qm: {
                        command__get_Qm();
                        break;
                    }

                    // 8                  
                    case Commands_Qmt::get_Qs: {
                        command__get_Qs();
                        break;
                    }
                    
                    // 9                    
                    case Commands_Qmt::get_Qms: {
                        command__get_Qms();
                        break;
                    }
                    
                    // 10                    
                    case Commands_Qmt::get_R_reg: {
                        command__get_R_reg();
                        break;
                    }

                    // 11                   
                    case Commands_Qmt::get_P: {
                        command__get_P();
                        break;
                    }

                    // 12
                    case Commands_Qmt::simulation: {
                        command__simulation();
                        break;
                    }

                    // 13             
                    case Commands_Qmt::save: {
                        command__save();
                        break;
                    }

                    // 14
                    case Commands_Qmt::stop: {
                        command__stop();
                        finish = true;
                        break;
                    }

                    // 15
                    case Commands_Qmt::id: {
                        command__id();
                        break;
                    }

                    // 16           
                    case Commands_Qmt::init_time: {
                        command__init_time();
                        break;
                    }
                }
            }
        }

    }; // class Qmt


}; // namespace G2_2


#endif // G2_2_H