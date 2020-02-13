/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef G3_1_H
#define G3_1_H

#include "commands.h"
#include "ports.h"
#include "proxies.h"

#include "../A2_1/A2_1.h"
#include "../G2_1/G2_1.h"
#include "../G2_2/G2_2.h"
#include "../A1_2/A1_2.h"

#include "../../mpi_deployment.h"
#include "../../Monitoring/monitoring.h"

using namespace std;
using namespace A2_1;
using namespace G2_1;
using namespace G2_2;
using namespace A1_2;

namespace G3_1 {

    /*
     * Microservice for Q+Qm+Qmt - gas section
     */
    class QQmt: public Microservice {
    public:
    
	Q*   Q_OS;	// underlying MS
	Q*   Q_Streb;
	Qm*  Q_AM;	
	Qmt* Q_VS;	
	p**  Pqmt;
	
	int N_OS;
	int N_Streb;
	int N_AM;
	int N_VS;
	
	DeploymentPool* deployment_pool;
	
	QQmt(int id_, string id_str_, MpiCommunicator* communicator_, 
		string section_, string network_,
		Monitoring_opts* mon_opts_,
    /*OS*/	float OS_S_,    float OS_R_,    float OS_L_,
    /*Streb*/	float Streb_S_, float Streb_R_, float Streb_L_,
    /*AM*/	float AM_S_,    float AM_R_,    float AM_L_,    float AM_V_,
    /*VS*/	float VS_S_,    float VS_R_,    float VS_L_,
		float dX_,
		Solver_Params& solv_params_) 
		    : Microservice(id_, id_str_, communicator_) 
        {
    	    N_OS    = round(OS_L_   /dX_);
    	    N_Streb = round(Streb_L_/dX_);
    	    N_AM    = round(AM_L_   /dX_);
    	    N_VS    = round(VS_L_   /dX_);
    	    
    	    // Setup of communication map
    	    set_communications();
	    //communicator->print_comm_links();

    	    // Initialization of underlying microservices
            Q_OS = new Q(2/*id*/, "Q_OS", communicator_,
                    "OS" /*element*/, section_, network_,
                    mon_opts_,
                    OS_S_ /*S*/, OS_R_ /*R*/, OS_L_ /*L*/, dX_, solv_params_);

	    Q_Streb = new Q(3/*id*/, "Q_Streb", communicator_,
                    "Streb" /*element*/, section_, network_,
                    mon_opts_,
                    Streb_S_ /*S*/, Streb_R_ /*R*/, Streb_L_ /*L*/, dX_, solv_params_);

	    Q_AM = new Qm(4/*id*/, "Q_AM", communicator_,
                    "AM" /*element*/, section_, network_,
                    mon_opts_,
                    AM_S_ /*S*/, AM_R_ /*R*/, AM_L_ /*L*/, dX_,
                    AM_V_, solv_params_);
		
	    Q_VS = new Qmt(5/*id*/, "Q_VS", communicator_,
                    "VS" /*element*/, section_, network_,
                    mon_opts_,
                    VS_S_ /*S*/, VS_R_ /*R*/, VS_L_ /*L*/, dX_,
                    solv_params_);

    	    Pqmt   = new p*[2];
    	    Pqmt[0] = new p(6/*id*/, "Pqmt0", communicator_,
    		      "Pqmt0", section_, network_,
    		      mon_opts_,
    		      OS_S_ /*S*/, dX_ /*dX*/, false /*is_boundary*/, solv_params_);
    	    Pqmt[1] = new p(7/*id*/, "Pqmt1", communicator_,
    		      "Pqmt1", section_, network_,
    		      mon_opts_,
    		      VS_S_ /*S*/, dX_ /*dX*/, false /*is_boundary*/, solv_params_);
	    	    
	    // Init underlying ms
	    init_proxies();
	    init_buffers();
	    
	    // Deployment pool initialization
    	    deployment_pool = new MpiDeploymentPool(communicator_->mpi_map);
    	    
    	    // Spawning worker threads for underlying ms
    	    deployment_pool->add_ms(Q_OS);
    	    deployment_pool->add_ms(Q_Streb);    	    
    	    deployment_pool->add_ms(Q_AM);
    	    deployment_pool->add_ms(Q_VS);
    	    deployment_pool->add_ms(Pqmt[0]);
    	    deployment_pool->add_ms(Pqmt[1]);    	    

	    deployment_pool->deploy_all();
	}
	
	void init_proxies() {
    	    // 
	    // Initialization of communication proxies
	    //
	    add_proxy(new LocalIntBuffer  (id, id_str,  Proxies_OS::command_flow, communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_OS::set_Q,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_OS::get_Q,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_OS::set_Qs,       communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_OS::get_Qs,       communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_OS::set_P,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_OS::get_P,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_OS::set_R_reg,    communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_OS::get_R_reg,    communicator));

	    add_proxy(new LocalIntBuffer  (id, id_str,  Proxies_Streb::command_flow,  communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_Streb::set_Q,         communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_Streb::get_Q,         communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_Streb::set_Qs,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_Streb::get_Qs,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_Streb::set_P,         communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_Streb::get_P,         communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_Streb::set_R_reg,     communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_Streb::get_R_reg,     communicator));

	    add_proxy(new LocalIntBuffer  (id, id_str,  Proxies_AM::command_flow, communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_AM::set_Q,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_AM::get_Q,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_AM::set_Qm,       communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_AM::get_Qm,       communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_AM::set_Qm0,      communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_AM::get_Qm0,      communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_AM::set_P,    	  communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_AM::get_P,    	  communicator));

	    add_proxy(new LocalIntBuffer  (id, id_str,  Proxies_VS::command_flow, communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_VS::set_Q,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_VS::get_Q,        communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_VS::set_Qm,       communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_VS::get_Qm,       communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_VS::set_Qs,       communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_VS::get_Qs,       communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_VS::set_Qms,      communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_VS::get_Qms,      communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_VS::set_R_reg,    communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_VS::get_R_reg,    communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_VS::set_P,    	  communicator));
    	    add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_VS::get_P,    	  communicator));

	    add_proxy(new LocalIntBuffer  (id, id_str,  Proxies_p::command_flow,    communicator));
            add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_p::set_p,           communicator));
            add_proxy(new LocalFloatBuffer(id, id_str,  Proxies_p::get_p,           communicator));
    	}
    	
    	void init_buffers() {
    	    // 
	    // Initialization of underlying MS' communication buffers
	    //
    	    Q_OS->add_buffer(new LocalIntBuffer  (Q_OS->id /*ms_id*/, Q_OS->id_str, 0 /*port*/, Q_OS->communicator));
            Q_OS->add_buffer(new LocalFloatBuffer(Q_OS->id /*ms_id*/, Q_OS->id_str, 1 /*port*/, Q_OS->communicator));
	    Q_OS->add_buffer(new LocalFloatBuffer(Q_OS->id /*ms_id*/, Q_OS->id_str, 2 /*port*/, Q_OS->communicator));
    	    Q_OS->add_buffer(new LocalFloatBuffer(Q_OS->id /*ms_id*/, Q_OS->id_str, 3 /*port*/, Q_OS->communicator));
            Q_OS->add_buffer(new LocalFloatBuffer(Q_OS->id /*ms_id*/, Q_OS->id_str, 4 /*port*/, Q_OS->communicator));
	    Q_OS->add_buffer(new LocalFloatBuffer(Q_OS->id /*ms_id*/, Q_OS->id_str, 5 /*port*/, Q_OS->communicator));
	    Q_OS->add_buffer(new LocalFloatBuffer(Q_OS->id /*ms_id*/, Q_OS->id_str, 6 /*port*/, Q_OS->communicator));
	    Q_OS->add_buffer(new LocalFloatBuffer(Q_OS->id /*ms_id*/, Q_OS->id_str, 7 /*port*/, Q_OS->communicator));
    	    Q_OS->add_buffer(new LocalFloatBuffer(Q_OS->id /*ms_id*/, Q_OS->id_str, 8 /*port*/, Q_OS->communicator));
    	    
    	    Q_Streb->add_buffer(new LocalIntBuffer  (Q_Streb->id /*ms_id*/, Q_Streb->id_str, 0 /*port*/, Q_Streb->communicator));
            Q_Streb->add_buffer(new LocalFloatBuffer(Q_Streb->id /*ms_id*/, Q_Streb->id_str, 1 /*port*/, Q_Streb->communicator));
            Q_Streb->add_buffer(new LocalFloatBuffer(Q_Streb->id /*ms_id*/, Q_Streb->id_str, 2 /*port*/, Q_Streb->communicator));
            Q_Streb->add_buffer(new LocalFloatBuffer(Q_Streb->id /*ms_id*/, Q_Streb->id_str, 3 /*port*/, Q_Streb->communicator));
            Q_Streb->add_buffer(new LocalFloatBuffer(Q_Streb->id /*ms_id*/, Q_Streb->id_str, 4 /*port*/, Q_Streb->communicator));
            Q_Streb->add_buffer(new LocalFloatBuffer(Q_Streb->id /*ms_id*/, Q_Streb->id_str, 5 /*port*/, Q_Streb->communicator));
            Q_Streb->add_buffer(new LocalFloatBuffer(Q_Streb->id /*ms_id*/, Q_Streb->id_str, 6 /*port*/, Q_Streb->communicator));
            Q_Streb->add_buffer(new LocalFloatBuffer(Q_Streb->id /*ms_id*/, Q_Streb->id_str, 7 /*port*/, Q_Streb->communicator));
            Q_Streb->add_buffer(new LocalFloatBuffer(Q_Streb->id /*ms_id*/, Q_Streb->id_str, 8 /*port*/, Q_Streb->communicator));
            
    	    Q_AM->add_buffer(new LocalIntBuffer  (Q_AM->id /*ms_id*/, Q_AM->id_str, 0 /*port*/, Q_AM->communicator));
            Q_AM->add_buffer(new LocalFloatBuffer(Q_AM->id /*ms_id*/, Q_AM->id_str, 1 /*port*/, Q_AM->communicator));
            Q_AM->add_buffer(new LocalFloatBuffer(Q_AM->id /*ms_id*/, Q_AM->id_str, 2 /*port*/, Q_AM->communicator));
            Q_AM->add_buffer(new LocalFloatBuffer(Q_AM->id /*ms_id*/, Q_AM->id_str, 3 /*port*/, Q_AM->communicator));
            Q_AM->add_buffer(new LocalFloatBuffer(Q_AM->id /*ms_id*/, Q_AM->id_str, 4 /*port*/, Q_AM->communicator));
            Q_AM->add_buffer(new LocalFloatBuffer(Q_AM->id /*ms_id*/, Q_AM->id_str, 5 /*port*/, Q_AM->communicator));
            Q_AM->add_buffer(new LocalFloatBuffer(Q_AM->id /*ms_id*/, Q_AM->id_str, 6 /*port*/, Q_AM->communicator));
            Q_AM->add_buffer(new LocalFloatBuffer(Q_AM->id /*ms_id*/, Q_AM->id_str, 7 /*port*/, Q_AM->communicator));
            Q_AM->add_buffer(new LocalFloatBuffer(Q_AM->id /*ms_id*/, Q_AM->id_str, 8 /*port*/, Q_AM->communicator));
            
            Q_VS->add_buffer(new LocalIntBuffer  (Q_VS->id /*ms_id*/, Q_VS->id_str, 0 /*port*/, Q_VS->communicator));
            Q_VS->add_buffer(new LocalFloatBuffer(Q_VS->id /*ms_id*/, Q_VS->id_str, 1 /*port*/, Q_VS->communicator));
            Q_VS->add_buffer(new LocalFloatBuffer(Q_VS->id /*ms_id*/, Q_VS->id_str, 2 /*port*/, Q_VS->communicator));
            Q_VS->add_buffer(new LocalFloatBuffer(Q_VS->id /*ms_id*/, Q_VS->id_str, 3 /*port*/, Q_VS->communicator));
            Q_VS->add_buffer(new LocalFloatBuffer(Q_VS->id /*ms_id*/, Q_VS->id_str, 4 /*port*/, Q_VS->communicator));
            Q_VS->add_buffer(new LocalFloatBuffer(Q_VS->id /*ms_id*/, Q_VS->id_str, 5 /*port*/, Q_VS->communicator));
            Q_VS->add_buffer(new LocalFloatBuffer(Q_VS->id /*ms_id*/, Q_VS->id_str, 6 /*port*/, Q_VS->communicator));
            Q_VS->add_buffer(new LocalFloatBuffer(Q_VS->id /*ms_id*/, Q_VS->id_str, 7 /*port*/, Q_VS->communicator));
            Q_VS->add_buffer(new LocalFloatBuffer(Q_VS->id /*ms_id*/, Q_VS->id_str, 8 /*port*/, Q_VS->communicator));
            Q_VS->add_buffer(new LocalFloatBuffer(Q_VS->id /*ms_id*/, Q_VS->id_str, 9 /*port*/, Q_VS->communicator));
            Q_VS->add_buffer(new LocalFloatBuffer(Q_VS->id /*ms_id*/, Q_VS->id_str, 10/*port*/, Q_VS->communicator));
            Q_VS->add_buffer(new LocalFloatBuffer(Q_VS->id /*ms_id*/, Q_VS->id_str, 11/*port*/, Q_VS->communicator));
            Q_VS->add_buffer(new LocalFloatBuffer(Q_VS->id /*ms_id*/, Q_VS->id_str, 12/*port*/, Q_VS->communicator));

	    for (int i=0; i<2; i++) {
                Pqmt[i]->add_buffer(new LocalIntBuffer  (Pqmt[i]->id /*ms_id*/, Pqmt[i]->id_str, 0 /*port*/, Pqmt[i]->communicator));
	        Pqmt[i]->add_buffer(new LocalFloatBuffer(Pqmt[i]->id /*ms_id*/, Pqmt[i]->id_str, 1 /*port*/, Pqmt[i]->communicator));
                Pqmt[i]->add_buffer(new LocalFloatBuffer(Pqmt[i]->id /*ms_id*/, Pqmt[i]->id_str, 2 /*port*/, Pqmt[i]->communicator));
                Pqmt[i]->add_buffer(new LocalFloatBuffer(Pqmt[i]->id /*ms_id*/, Pqmt[i]->id_str, 3 /*port*/, Pqmt[i]->communicator));
                Pqmt[i]->add_buffer(new LocalFloatBuffer(Pqmt[i]->id /*ms_id*/, Pqmt[i]->id_str, 4 /*port*/, Pqmt[i]->communicator));
                Pqmt[i]->add_buffer(new LocalFloatBuffer(Pqmt[i]->id /*ms_id*/, Pqmt[i]->id_str, 5 /*port*/, Pqmt[i]->communicator));
	    }    
	};
	
	void set_communications() {
	    string master = id_str;
        
    	    //
            // Command Flow Master->Q
            //
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_OS::command_flow  + proxy_disp /*port*/,
        	                                     "Q_OS" /*rcv_id*/, Ports_Q::command_flow));
    	    communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_Streb::command_flow  + proxy_disp /*port*/,
        	                                     "Q_Streb" /*rcv_id*/, Ports_Q::command_flow));
    	    communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_AM::command_flow  + proxy_disp /*port*/,
        	                                     "Q_AM" /*rcv_id*/, Ports_Qm::command_flow));
    	    communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_VS::command_flow  + proxy_disp /*port*/,
        	                                     "Q_VS" /*rcv_id*/, Ports_Qmt::command_flow));        
        
    	    //	                                     
            // Command Flow Master->P
            //
            for (int i=0; i<2; i++)
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_p::command_flow  + proxy_disp /*port*/,
                                                     "Pqmt" + to_string(i) /*rcv_id*/, Ports_p::command_flow));
        
    	    //
            // Data Flow Master->Q
            //
        	// OS
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_OS::set_Q  + proxy_disp /*port*/,
                                                 "Q_OS" /*rcv_id*/, Ports_Q::set_Q,
                                                 N_OS /*size*/));
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_OS::set_P  + proxy_disp /*port*/,
                                                 "Q_OS" /*rcv_id*/, Ports_Q::set_P,
                                                 N_OS-1 /*size*/));
        	// Streb
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_Streb::set_Q  + proxy_disp /*port*/,
                                                 "Q_Streb" /*rcv_id*/, Ports_Q::set_Q,
                                                 N_Streb /*size*/));
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_Streb::set_P  + proxy_disp /*port*/,
                                                 "Q_Streb" /*rcv_id*/, Ports_Q::set_P,
                                                 N_Streb-1 /*size*/));
		// AM
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_AM::set_Q  + proxy_disp /*port*/,
                                                 "Q_AM" /*rcv_id*/, Ports_Qm::set_Q,
                                                 N_AM /*size*/));
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_AM::set_Qm  + proxy_disp /*port*/,
                                                 "Q_AM" /*rcv_id*/, Ports_Qm::set_Qm,
                                                 N_AM /*size*/));
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_AM::set_Qm0  + proxy_disp /*port*/,
                                                 "Q_AM" /*rcv_id*/, Ports_Qm::set_Qm0));
                                                 /*1*/
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_AM::set_P  + proxy_disp /*port*/,
                                                 "Q_AM" /*rcv_id*/, Ports_Qm::set_P,
                                                 N_AM-1 /*size*/));
		// VS
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_VS::set_Q  + proxy_disp /*port*/,
                                                 "Q_VS" /*rcv_id*/, Ports_Qmt::set_Q,
                                                 N_VS /*size*/));
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_VS::set_Qm  + proxy_disp /*port*/,
                                                 "Q_VS" /*rcv_id*/, Ports_Qmt::set_Qm,
                                                 N_VS /*size*/));
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_VS::set_Qs  + proxy_disp /*port*/,
                                                 "Q_VS" /*rcv_id*/, Ports_Qmt::set_Qs,
                                                 N_VS /*size*/));
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_VS::set_Qms  + proxy_disp /*port*/,
                                                 "Q_VS" /*rcv_id*/, Ports_Qmt::set_Qms,
                                                 N_VS /*size*/));
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_VS::set_R_reg  + proxy_disp /*port*/,
                                                 "Q_VS" /*rcv_id*/, Ports_Qmt::set_R_reg,
                                                 N_VS /*size*/));
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_VS::set_P  + proxy_disp /*port*/,
                                                 "Q_VS" /*rcv_id*/, Ports_Qmt::set_P,
                                                 N_VS-1 /*size*/));

	    //
            // Data Flow Master->P
            //
            for (int i=0; i<2; i++)
                communicator->add_comm_link(new CommLink(master /*snd_id*/, Proxies_p::set_p + proxy_disp  /*port*/,
	                                             "Pqmt" + to_string(i) /*rcv_id*/, Ports_p::set_p));
        
    	    //
            // Data Flow Q->Master
            //
        	// OS
	    communicator->add_comm_link(new CommLink("Q_OS" /*snd_id*/, Ports_Q::get_Q,
                                                 master /*rcv_id*/, Proxies_OS::get_Q + proxy_disp  /*port*/,
                                                 N_OS /*size*/));
            communicator->add_comm_link(new CommLink("Q_OS" /*snd_id*/, Ports_Q::get_P,
                                                 master /*rcv_id*/, Proxies_OS::get_P + proxy_disp  /*port*/,
                                                 N_OS-1 /*size*/));
        	// Streb
	    communicator->add_comm_link(new CommLink("Q_Streb" /*snd_id*/, Ports_Q::get_Q,
                                                 master /*rcv_id*/, Proxies_Streb::get_Q + proxy_disp  /*port*/,
                                                 N_Streb /*size*/));
            communicator->add_comm_link(new CommLink("Q_Streb" /*snd_id*/, Ports_Q::get_P,
                                                 master /*rcv_id*/, Proxies_Streb::get_P + proxy_disp  /*port*/,
                                                 N_Streb-1 /*size*/));
		// AM
	    communicator->add_comm_link(new CommLink("Q_AM" /*snd_id*/, Ports_Qm::get_Q,
                                                 master /*rcv_id*/, Proxies_AM::get_Q + proxy_disp  /*port*/,
                                                 N_AM /*size*/));
            communicator->add_comm_link(new CommLink("Q_AM" /*snd_id*/, Ports_Qm::get_Qm,
                                                 master /*rcv_id*/, Proxies_AM::get_Qm + proxy_disp  /*port*/,
                                                 N_AM /*size*/));
            communicator->add_comm_link(new CommLink("Q_AM" /*snd_id*/, Ports_Qm::get_Qm0,
                                                 master /*rcv_id*/, Proxies_AM::get_Qm0 + proxy_disp  /*port*/));
                                                 /*1*/
            communicator->add_comm_link(new CommLink("Q_AM" /*snd_id*/, Ports_Qm::get_P,
                                                 master /*rcv_id*/, Proxies_AM::get_P + proxy_disp  /*port*/,
                                                 N_AM-1 /*size*/));
		// VS
	    communicator->add_comm_link(new CommLink("Q_VS" /*snd_id*/, Ports_Qmt::get_Q,
                                                 master /*rcv_id*/, Proxies_VS::get_Q + proxy_disp  /*port*/,
                                                 N_VS /*size*/));
            communicator->add_comm_link(new CommLink("Q_VS" /*snd_id*/, Ports_Qmt::get_Qm,
                                                 master /*rcv_id*/, Proxies_VS::get_Qm + proxy_disp  /*port*/,
                                                 N_VS /*size*/));
            communicator->add_comm_link(new CommLink("Q_VS" /*snd_id*/, Ports_Qmt::get_Qs,
                                                 master /*rcv_id*/, Proxies_VS::get_Qs + proxy_disp  /*port*/,
                                                 N_VS /*size*/));
            communicator->add_comm_link(new CommLink("Q_VS" /*snd_id*/, Ports_Qmt::get_Qms,
                                                 master /*rcv_id*/, Proxies_VS::get_Qms + proxy_disp  /*port*/,
                                                 N_VS /*size*/));
            communicator->add_comm_link(new CommLink("Q_VS" /*snd_id*/, Ports_Qmt::get_R_reg,
                                                 master /*rcv_id*/, Proxies_VS::get_R_reg + proxy_disp  /*port*/,
                                                 N_VS /*size*/));
            communicator->add_comm_link(new CommLink("Q_VS" /*snd_id*/, Ports_Qmt::get_P,
                                                 master /*rcv_id*/, Proxies_VS::get_P + proxy_disp  /*port*/,
                                                 N_VS-1 /*size*/));

	    //
            // Data Flow P->Master
            //
            for (int i=0; i<2; i++)
                communicator->add_comm_link(new CommLink("Pqmt" + to_string(i) /*rcv_id*/, Ports_p::get_p,
                                                     master /*snd_id*/, Proxies_p::get_p + proxy_disp  /*port*/));
        
    	    //
            // Data Flow Q->P
            //
        	// Pqmt0
    	    communicator->add_comm_link(new CommLink("Q_OS_q"+to_string(N_OS-1)/*snd_id*/, Ports_q::num_get_q,
                                                 "Pqmt0" /*rcv_id*/, Ports_p::num_set_qin));
	    communicator->add_comm_link(new CommLink("Q_Streb_q0"/*snd_id*/, Ports_q::num_get_q,
                                                 "Pqmt0" /*rcv_id*/,   Ports_p::num_set_qout));
            communicator->add_comm_link(new CommLink("Q_AM_q0"/*snd_id*/, Ports_q::num_get_q,
                                                 "Pqmt0" /*rcv_id*/,   Ports_p::num_set_qout));
        	// Pqmt1
    	    communicator->add_comm_link(new CommLink("Q_Streb_q"+to_string(N_Streb-1)/*snd_id*/, Ports_q::num_get_q,
                                                 "Pqmt1" /*rcv_id*/, Ports_p::num_set_qin));
    	    communicator->add_comm_link(new CommLink("Q_AM_q"+to_string(N_AM-1)/*snd_id*/, Ports_q::num_get_q,
                                                 "Pqmt1" /*rcv_id*/, Ports_p::num_set_qin));                                                 
            communicator->add_comm_link(new CommLink("Q_VS_q0"/*snd_id*/, Ports_q::num_get_q,
                                                 "Pqmt1" /*rcv_id*/, Ports_p::num_set_qout));
	    //
            // Data Flow P->Q
            //
        	// Pqmt0
            communicator->add_comm_link(new CommLink("Pqmt0"/*snd_id*/, Ports_p::num_get_p,
                                                 "Q_Streb_q0" /*rcv_id*/, Ports_q::num_set_pstart));
            communicator->add_comm_link(new CommLink("Pqmt0"/*snd_id*/, Ports_p::num_get_p,
                                                 "Q_AM_q0" /*rcv_id*/, Ports_q::num_set_pstart));                                 
            communicator->add_comm_link(new CommLink("Pqmt0"/*snd_id*/, Ports_p::num_get_p,
                                                 "Q_OS_q"+to_string(N_OS-1)/*rcv_id*/, Ports_q::num_set_pend));
        	// Pqmt1
            communicator->add_comm_link(new CommLink("Pqmt1"/*snd_id*/, Ports_p::num_get_p,
                                                 "Q_VS_q0" /*rcv_id*/, Ports_q::num_set_pstart));
            communicator->add_comm_link(new CommLink("Pqmt1"/*snd_id*/, Ports_p::num_get_p,
                                                 "Q_Streb_q"+to_string(N_Streb-1)/*rcv_id*/, Ports_q::num_set_pend));
            communicator->add_comm_link(new CommLink("Pqmt1"/*snd_id*/, Ports_p::num_get_p,
                                                 "Q_AM_q"+to_string(N_AM-1)/*rcv_id*/, Ports_q::num_set_pend));

            // Data Flow Qm->Qmt
            communicator->add_comm_link(new CommLink("Q_AM_qmt"+to_string(N_AM-1)/*snd_id*/, Ports_qmt::gas_get_qm,
            					     "Q_VS_qmt0" /*snd_id*/, Ports_qmt::gas_set_qm));
    	};
	
	
	/*
         * Implementation of the communication protocol
         */
	virtual void command__set_Q_OS() {
    	    stringstream out;
    	    
    	    float value[N_OS];

    	    // Receive value from Master
    	    buffer_sync(Ports_QQmt::set_Q_OS);
    	    
            for (int i=0; i<N_OS; i++)
    		value[i] = get_buffer_value<float>(Ports_QQmt::set_Q_OS, i/*index*/); // q[0..N]
            buffer_clear(Ports_QQmt::set_Q_OS);

	    //
            //Propagation to underlying Q's
            //
    	    out << "ms_" << id << "(" << id_str << "): Initialized qq_OS = {";
            for (int i=0; i<N_OS; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_OS::set_Q, value[i] /*value*/);
            }
            proxy_flush_collective_replicate(Proxies_OS::set_Q);
            proxy_clear(Proxies_OS::set_Q);

            add_proxy_value<int>(Proxies_OS::command_flow, Commands_Q::set_Q /*value*/);
            proxy_flush_collective_replicate(Proxies_OS::command_flow);
            proxy_clear(Proxies_OS::command_flow);

            out << "}" << endl;
            cout << out.str();
        }

	virtual void command__set_Q_Streb() {
    	    stringstream out;
    	    
    	    float value[N_Streb];

    	    // Receive value from Master
    	    buffer_sync(Ports_QQmt::set_Q_Streb);

            for (int i=0; i<N_Streb; i++)
    		value[i] = get_buffer_value<float>(Ports_QQmt::set_Q_Streb, i/*index*/); // q[0..N]
            buffer_clear(Ports_QQmt::set_Q_Streb);

	    //
            //Propagation to underlying Q's
            //
    	    out << "ms_" << id << "(" << id_str << "): Initialized qq_Streb = {";
            for (int i=0; i<N_Streb; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_Streb::set_Q, value[i] /*value*/);
            }
            proxy_flush_collective_replicate(Proxies_Streb::set_Q);
            proxy_clear(Proxies_Streb::set_Q);

            add_proxy_value<int>(Proxies_Streb::command_flow, Commands_Q::set_Q /*value*/);
            proxy_flush_collective_replicate(Proxies_Streb::command_flow);
            proxy_clear(Proxies_Streb::command_flow);

            out << "}" << endl;
            cout << out.str();
        }

	virtual void command__set_Q_AM() {
    	    stringstream out;
    	    
    	    float value[N_AM];

    	    // Receive value from Master
    	    buffer_sync(Ports_QQmt::set_Q_AM);

            for (int i=0; i<N_AM; i++)
    		value[i] = get_buffer_value<float>(Ports_QQmt::set_Q_AM, i/*index*/); // q[0..N]
            buffer_clear(Ports_QQmt::set_Q_AM);

	    //
            //Propagation to underlying Q's
            //
    	    out << "ms_" << id << "(" << id_str << "): Initialized qq_AM = {";
            for (int i=0; i<N_AM; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_AM::set_Q, value[i] /*value*/);
            }
            proxy_flush_collective_replicate(Proxies_AM::set_Q);
            proxy_clear(Proxies_AM::set_Q);

            add_proxy_value<int>(Proxies_AM::command_flow, Commands_Q::set_Q /*value*/);
            proxy_flush_collective_replicate(Proxies_AM::command_flow);
            proxy_clear(Proxies_AM::command_flow);

            out << "}" << endl;
            cout << out.str();
        }

	virtual void command__set_Q_VS() {
    	    stringstream out;
    	    
    	    float value[N_VS];

    	    // Receive value from Master
    	    buffer_sync(Ports_QQmt::set_Q_VS);

            for (int i=0; i<N_VS; i++)
    		value[i] = get_buffer_value<float>(Ports_QQmt::set_Q_VS, i/*index*/); // q[0..N]
            buffer_clear(Ports_QQmt::set_Q_VS);

	    //
            //Propagation to underlying Q's
            //
    	    out << "ms_" << id << "(" << id_str << "): Initialized qq_VS = {";
            for (int i=0; i<N_VS; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_VS::set_Q, value[i] /*value*/);
            }
            proxy_flush_collective_replicate(Proxies_VS::set_Q);
            proxy_clear(Proxies_VS::set_Q);

            add_proxy_value<int>(Proxies_VS::command_flow, Commands_Qmt::set_Q /*value*/);
            proxy_flush_collective_replicate(Proxies_VS::command_flow);
            proxy_clear(Proxies_VS::command_flow);

            out << "}" << endl;
            cout << out.str();
        }

	virtual void command__set_Qm_AM() {
    	    stringstream out;
    	    
    	    float value[N_AM];

    	    // Receive value from Master
    	    buffer_sync(Ports_QQmt::set_Qm_AM);
    	    
            for (int i=0; i<N_AM; i++)
    		value[i] = get_buffer_value<float>(Ports_QQmt::set_Qm_AM, i/*index*/); // q[0..N]
            buffer_clear(Ports_QQmt::set_Qm_AM);

	    //
            //Propagation to underlying Q's
            //
    	    out << "ms_" << id << "(" << id_str << "): Initialized qm_AM = {";
            for (int i=0; i<N_AM; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_AM::set_Qm, value[i] /*value*/);
            }
            proxy_flush_collective_replicate(Proxies_AM::set_Qm);
            proxy_clear(Proxies_AM::set_Qm);

            add_proxy_value<int>(Proxies_AM::command_flow, Commands_Qm::set_Qm /*value*/);
            proxy_flush_collective_replicate(Proxies_AM::command_flow);
            proxy_clear(Proxies_AM::command_flow);

            out << "}" << endl;
            cout << out.str();
        }


	virtual void command__set_Qm0_AM() {
    	    stringstream out;
    	    
    	    float value[1];

    	    // Receive value from Master
    	    buffer_sync(Ports_QQmt::set_Qm0_AM);

            for (int i=0; i<1; i++)
    		value[i] = get_buffer_value<float>(Ports_QQmt::set_Qm0_AM, i/*index*/); // qm[0]
            buffer_clear(Ports_QQmt::set_Qm0_AM);
            
	    //
            //Propagation to underlying Q's
            //
    	    out << "ms_" << id << "(" << id_str << "): Initialized qm0_AM = {";
            for (int i=0; i<1; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_AM::set_Qm0, value[i] /*value*/);
            }
            proxy_flush_collective_replicate(Proxies_AM::set_Qm0);
            proxy_clear(Proxies_AM::set_Qm0);

            add_proxy_value<int>(Proxies_AM::command_flow, Commands_Qm::set_Qm0 /*value*/);
            proxy_flush_collective_replicate(Proxies_AM::command_flow);
            proxy_clear(Proxies_AM::command_flow);

            out << "}" << endl;
            cout << out.str();
        }

	virtual void command__set_Qms_VS() {
    	    stringstream out;
    	    
    	    float value[N_VS];

    	    // Receive value from Master
    	    buffer_sync(Ports_QQmt::set_Qms_VS);

            for (int i=0; i<N_VS; i++)
    		value[i] = get_buffer_value<float>(Ports_QQmt::set_Qms_VS, i/*index*/); // q[0..N]
            buffer_clear(Ports_QQmt::set_Qms_VS);

	    //
            //Propagation to underlying Q's
            //
    	    out << "ms_" << id << "(" << id_str << "): Initialized qms_VS = {";
            for (int i=0; i<N_VS; i++) {
                out << value[i] << ", ";
                add_proxy_value<float>(Proxies_VS::set_Qms, value[i] /*value*/);
            }
            proxy_flush_collective_replicate(Proxies_VS::set_Qms);
            proxy_clear(Proxies_VS::set_Qms);

            add_proxy_value<int>(Proxies_VS::command_flow, Commands_Qmt::set_Qms /*value*/);
            proxy_flush_collective_replicate(Proxies_VS::command_flow);
            proxy_clear(Proxies_VS::command_flow);

            out << "}" << endl;
            cout << out.str();
        }

        
        virtual void command__get_Q_OS() {
    	    stringstream out;
    	    
    	    float value[N_OS];
    	    
    	    // Receive value from underlying q's
            add_proxy_value<int>(Proxies_OS::command_flow, Commands_Q::get_Q /*value*/);
            proxy_flush_collective_replicate(Proxies_OS::command_flow);
            proxy_clear(Proxies_OS::command_flow);

            proxy_sync(Proxies_OS::get_Q);
            for (int i=0; i<N_OS; i++)
                value[i] = get_proxy_value<float>(Proxies_OS::get_Q, i /*index*/);
            proxy_clear(Proxies_OS::get_Q);

    	    // Sending to Master
            for (int i=0; i<N_OS; i++)
                add_buffer_value<float>(Ports_QQmt::get_Q_OS, value[i] /*value*/);
            buffer_flush_collective_gather(Ports_QQmt::get_Q_OS);
            buffer_clear(Ports_QQmt::get_Q_OS);
        }
        
        virtual void command__get_Q_Streb() {
    	    stringstream out;
    	    
    	    float value[N_Streb];
    	    
    	    // Receive value from underlying q's
            add_proxy_value<int>(Proxies_Streb::command_flow, Commands_Q::get_Q /*value*/);
            proxy_flush_collective_replicate(Proxies_Streb::command_flow);
            proxy_clear(Proxies_Streb::command_flow);

            proxy_sync(Proxies_Streb::get_Q);
            for (int i=0; i<N_Streb; i++)
                value[i] = get_proxy_value<float>(Proxies_Streb::get_Q, i /*index*/);
            proxy_clear(Proxies_Streb::get_Q);

    	    // Sending to Master
            for (int i=0; i<N_Streb; i++)
                add_buffer_value<float>(Ports_QQmt::get_Q_Streb, value[i] /*value*/);
            buffer_flush_collective_gather(Ports_QQmt::get_Q_Streb);
            buffer_clear(Ports_QQmt::get_Q_Streb);
        }
        
        virtual void command__get_Q_AM() {
    	    stringstream out;
    	    
    	    float value[N_AM];
    	    
    	    // Receive value from underlying q's
            add_proxy_value<int>(Proxies_AM::command_flow, Commands_Qm::get_Q /*value*/);
            proxy_flush_collective_replicate(Proxies_AM::command_flow);
            proxy_clear(Proxies_AM::command_flow);

            proxy_sync(Proxies_AM::get_Q);
            for (int i=0; i<N_AM; i++)
                value[i] = get_proxy_value<float>(Proxies_AM::get_Q, i /*index*/);
            proxy_clear(Proxies_AM::get_Q);

    	    // Sending to Master
            for (int i=0; i<N_AM; i++)
                add_buffer_value<float>(Ports_QQmt::get_Q_AM, value[i] /*value*/);
            buffer_flush_collective_gather(Ports_QQmt::get_Q_AM);
            buffer_clear(Ports_QQmt::get_Q_AM);
        }

        virtual void command__get_Q_VS() {
    	    stringstream out;
    	    
    	    float value[N_VS];
    	    
    	    // Receive value from underlying q's
            add_proxy_value<int>(Proxies_VS::command_flow, Commands_Qmt::get_Q /*value*/);
            proxy_flush_collective_replicate(Proxies_VS::command_flow);
            proxy_clear(Proxies_VS::command_flow);

            proxy_sync(Proxies_VS::get_Q);
            for (int i=0; i<N_VS; i++)
                value[i] = get_proxy_value<float>(Proxies_VS::get_Q, i /*index*/);
            proxy_clear(Proxies_VS::get_Q);

    	    // Sending to Master
            for (int i=0; i<N_VS; i++)
                add_buffer_value<float>(Ports_QQmt::get_Q_VS, value[i] /*value*/);
            buffer_flush_collective_gather(Ports_QQmt::get_Q_VS);
            buffer_clear(Ports_QQmt::get_Q_VS);
        }

        virtual void command__get_Qm_AM() {
    	    stringstream out;
    	    
    	    float value[N_AM];
    	    
    	    // Receive value from underlying q's
            add_proxy_value<int>(Proxies_AM::command_flow, Commands_Qm::get_Qm /*value*/);
            proxy_flush_collective_replicate(Proxies_AM::command_flow);
            proxy_clear(Proxies_AM::command_flow);

            proxy_sync(Proxies_AM::get_Qm);
            for (int i=0; i<N_AM; i++)
                value[i] = get_proxy_value<float>(Proxies_AM::get_Qm, i /*index*/);
            proxy_clear(Proxies_AM::get_Qm);

    	    // Sending to Master
            for (int i=0; i<N_AM; i++)
                add_buffer_value<float>(Ports_QQmt::get_Qm_AM, value[i] /*value*/);
            buffer_flush_collective_gather(Ports_QQmt::get_Qm_AM);
            buffer_clear(Ports_QQmt::get_Qm_AM);
        }



        virtual void command__simulation() {
    		// OS
	    add_proxy_value<int>(Proxies_OS::command_flow, Commands_Q::simulation /*value*/);
            proxy_flush_collective_replicate(Proxies_OS::command_flow);
            proxy_clear(Proxies_OS::command_flow);
        	// Streb
            add_proxy_value<int>(Proxies_Streb::command_flow, Commands_Q::simulation /*value*/);
            proxy_flush_collective_replicate(Proxies_Streb::command_flow);
            proxy_clear(Proxies_Streb::command_flow);
        	// AM
            add_proxy_value<int>(Proxies_AM::command_flow, Commands_Qm::simulation /*value*/);
            proxy_flush_collective_replicate(Proxies_AM::command_flow);
            proxy_clear(Proxies_AM::command_flow);
        	// VS
            add_proxy_value<int>(Proxies_VS::command_flow, Commands_Qmt::simulation /*value*/);
            proxy_flush_collective_replicate(Proxies_VS::command_flow);
            proxy_clear(Proxies_VS::command_flow);
        	// P
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::simulation /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);
        }

        virtual void command__save() {
    		// OS
	    add_proxy_value<int>(Proxies_OS::command_flow, Commands_Q::save /*value*/);
	    proxy_flush_collective_replicate(Proxies_OS::command_flow);
            proxy_clear(Proxies_OS::command_flow);
        	// Streb
            add_proxy_value<int>(Proxies_Streb::command_flow, Commands_Q::save /*value*/);
            proxy_flush_collective_replicate(Proxies_Streb::command_flow);
            proxy_clear(Proxies_Streb::command_flow);
        	// AM
            add_proxy_value<int>(Proxies_AM::command_flow, Commands_Qm::save /*value*/);
            proxy_flush_collective_replicate(Proxies_AM::command_flow);
            proxy_clear(Proxies_AM::command_flow);
        	// VS
            add_proxy_value<int>(Proxies_VS::command_flow, Commands_Qmt::save /*value*/);
            proxy_flush_collective_replicate(Proxies_VS::command_flow);
            proxy_clear(Proxies_VS::command_flow);
        	// P
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::save /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);

        }

        virtual void command__stop() {
    		// OS
	    add_proxy_value<int>(Proxies_OS::command_flow, Commands_Q::stop /*value*/);
            proxy_flush_collective_replicate(Proxies_OS::command_flow);
	    proxy_clear(Proxies_OS::command_flow);
        	// Streb
            add_proxy_value<int>(Proxies_Streb::command_flow, Commands_Q::stop /*value*/);
            proxy_flush_collective_replicate(Proxies_Streb::command_flow);
            proxy_clear(Proxies_Streb::command_flow);
        	// AM
            add_proxy_value<int>(Proxies_AM::command_flow, Commands_Qm::stop /*value*/);
            proxy_flush_collective_replicate(Proxies_AM::command_flow);
            proxy_clear(Proxies_AM::command_flow);
        	// VS
            add_proxy_value<int>(Proxies_VS::command_flow, Commands_Qmt::stop /*value*/);
            proxy_flush_collective_replicate(Proxies_VS::command_flow);
            proxy_clear(Proxies_VS::command_flow);
        	// P
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::stop /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);        
        }

        virtual void command__id() {
    		// OS
	    add_proxy_value<int>(Proxies_OS::command_flow, Commands_Q::id /*value*/);
            proxy_flush_collective_replicate(Proxies_OS::command_flow);
            proxy_clear(Proxies_OS::command_flow);
        	// Streb
            add_proxy_value<int>(Proxies_Streb::command_flow, Commands_Q::id /*value*/);
            proxy_flush_collective_replicate(Proxies_Streb::command_flow);
            proxy_clear(Proxies_Streb::command_flow);
        	// AM
            add_proxy_value<int>(Proxies_AM::command_flow, Commands_Qm::id /*value*/);
            proxy_flush_collective_replicate(Proxies_AM::command_flow);
            proxy_clear(Proxies_AM::command_flow);
        	// VS
            add_proxy_value<int>(Proxies_VS::command_flow, Commands_Qmt::id /*value*/);
            proxy_flush_collective_replicate(Proxies_VS::command_flow);
            proxy_clear(Proxies_VS::command_flow);
        	// P
            add_proxy_value<int>(Proxies_p::command_flow, Commands_p::id /*value*/);
            proxy_flush_collective_replicate(Proxies_p::command_flow);
            proxy_clear(Proxies_p::command_flow);        
        }

	virtual void command__init_time() {
    		// OS
	    add_proxy_value<int>(Proxies_OS::command_flow, Commands_Q::init_time /*value*/);
            proxy_flush_collective_replicate(Proxies_OS::command_flow);
            proxy_clear(Proxies_OS::command_flow);
        	// Streb
            add_proxy_value<int>(Proxies_Streb::command_flow, Commands_Q::init_time /*value*/);
            proxy_flush_collective_replicate(Proxies_Streb::command_flow);
            proxy_clear(Proxies_Streb::command_flow);
        	// AM
            add_proxy_value<int>(Proxies_AM::command_flow, Commands_Qm::init_time /*value*/);
            proxy_flush_collective_replicate(Proxies_AM::command_flow);
            proxy_clear(Proxies_AM::command_flow);
        	// VS
            add_proxy_value<int>(Proxies_VS::command_flow, Commands_Qmt::init_time /*value*/);
            proxy_flush_collective_replicate(Proxies_VS::command_flow);
            proxy_clear(Proxies_VS::command_flow);
        	// P
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
                    case Commands_QQmt::set_Q_OS: {
                        command__set_Q_OS();
                        break;
                    }
                    
                    // 1
                    case Commands_QQmt::set_Q_Streb: {
                        command__set_Q_Streb();
                        break;
                    }
                    
                    // 2
                    case Commands_QQmt::set_Q_AM: {
                        command__set_Q_AM();
                        break;
                    }
                    
                    // 3
                    case Commands_QQmt::set_Q_VS: {
                        command__set_Q_VS();
                        break;
                    }
                    
                    // 4
                    case Commands_QQmt::set_Qm_AM: {
                        command__set_Qm_AM();
                        break;
                    }
                    
                    
                    // ...
                    
                    // 7
                    case Commands_QQmt::set_Qms_VS: {
                        command__set_Qms_VS();
                        break;
                    }
                    
                    // 8
                    case Commands_QQmt::set_Qm0_AM: {
                        command__set_Qm0_AM();
                        break;
                    }
                    
                    // ...
        
                    
                    // 10
                    case Commands_QQmt::get_Q_OS: {
                        command__get_Q_OS();
                        break;
                    }
                    
                    // 11
                    case Commands_QQmt::get_Q_Streb: {
                        command__get_Q_Streb();
                        break;
                    }
                    
                    // 12
                    case Commands_QQmt::get_Q_AM: {
                        command__get_Q_AM();
                        break;
                    }
                    
                    // 13
                    case Commands_QQmt::get_Q_VS: {
                        command__get_Q_VS();
                        break;
                    }
                    
                    // 14
                    case Commands_QQmt::get_Qm_AM: {
                        command__get_Qm_AM();
                        break;
                    }
                    
                    
                    // ...
                    
                    // 20
                    case Commands_QQmt::simulation: {
                        command__simulation();
                        break;
                    }

                    // 21  
                    case Commands_QQmt::save: {
                        command__save();
                        break;
                    }

                    // 22
                    case Commands_QQmt::stop: {
                        command__stop();
                        finish = true;
                        break;
                    }

                    // 23
                    case Commands_QQmt::id: {
                        command__id();
                        break;
                    }

                    // 24           
                    case Commands_QQmt::init_time: {
                        command__init_time();
                        break;
                    }
                }
            }
        }

    }; // class QQmt


}; // namespace G3_1


#endif // G3_1_H