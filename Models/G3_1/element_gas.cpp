/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/*
 * This is a model of a simple ventilation element with unsteady
 * air distribution with neither leaks nor methan transport
 */

#include "mpi.h"

#include "../../Services/G3_1/G3_1.h"
#include "../../Services/A1_2/A1_2.h"
#include "../../mpi_communicator.h"
#include "../../mpi_deployment.h"


using namespace std;
using namespace G3_1;
using namespace A1_2;

class Element_model: public Microservice {
public:

    float H_start; // pressures
    float H_end;
    
    float Qm0;
    Solver_Params solv_params;
    
    int N_OS;
    int N_Streb;
    int N_AM;
    int N_VS;
        
    int proxy_disp_p = 21; // proxy displacement of p- nodes
                           // (contiguous numbering of proxy-ports)

    QQmt* QQmt0;      // underlying MS
    p**   PP;
    
    DeploymentPool* deployment_pool;
                        
    Element_model(int id_, string id_str_, MpiCommunicator* communicator_,
                string section_, string network_,
                float H_start_, float H_end_, float Qm0_,
                Monitoring_opts* mon_opts_,
    /*OS*/      float OS_S_,    float OS_R_,    float OS_L_,
    /*Streb*/   float Streb_S_, float Streb_R_, float Streb_L_,
    /*AM*/      float AM_S_,    float AM_R_,    float AM_L_,    float AM_V_,
    /*VS*/      float VS_S_,    float VS_R_,    float VS_L_,
                float dX_,
                Solver_Params& solv_params_)
            	    : Microservice(id_, id_str_, communicator_)
    {
    	N_OS = round(OS_L_/dX_);
	N_Streb = round(Streb_L_/dX_);
	N_AM = round(AM_L_/dX_);
	N_VS = round(VS_L_/dX_);
	
	H_start = H_start_;
	H_end = H_end_;
	
	Qm0 = Qm0_;
	solv_params = solv_params_;
	
        // Setup of communication map
        set_communications();
        //communicator->print_comm_links();
            

	QQmt0 = new QQmt(1, "QQmt0", communicator_,
            "Section1" /*section*/, "Network1" /*network*/,
            mon_opts_,
/*OS*/      OS_S_, OS_R_, OS_L_,
/*Streb*/   Streb_S_, Streb_R_, Streb_L_,
/*AM*/      AM_S_, AM_R_, AM_L_, AM_V_,
/*VS*/      VS_S_, VS_R_, VS_L_,
            dX_,
            solv_params_);

	PP = new p*[2];

        PP[0] = new p(2/*id*/, "P0", communicator_, 
			 "P0" /*name*/, "Section1" /*section*/, "Network1" /*network*/,
			 mon_opts_,
			 OS_S_ /*S*/, dX_ /*dX*/, true /*is_bound*/, solv_params_);
	PP[1] = new p(3/*id*/, "P1", communicator, 
			 "P1" /*name*/, "Section1" /*section*/, "Network1" /*network*/,
			 mon_opts_,
			 VS_S_ /*S*/, dX_ /*dX*/, true /*is_bound*/, solv_params_);
	
	// Init underlying ms
        init_proxies();
        init_buffers();

        // Deployment pool initialization
        deployment_pool = new MpiDeploymentPool(communicator_->mpi_map);

        // Spawning worker threads for underlying ms
        deployment_pool->add_ms(QQmt0);
        deployment_pool->add_ms(PP[0]);
        deployment_pool->add_ms(PP[1]);

        deployment_pool->deploy_all();
        deployment_pool->join_all();
    };
    
    void init_proxies() {
	// 
        // Initialization of communication proxies
	//
        add_proxy(new LocalIntBuffer  (id, id_str,  Ports_QQmt::command_flow,   communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::set_Q_OS,       communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::set_Q_Streb,    communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::set_Q_AM,       communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::set_Q_VS,       communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::get_Q_OS,       communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::get_Q_Streb,    communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::get_Q_AM,       communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::get_Q_VS,       communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::set_Qm_AM,      communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::set_Qm_VS,      communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::get_Qm_AM,      communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::get_Qm_VS,      communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::set_Qs_VS,      communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::get_Qs_VS,      communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::set_Qms_VS,     communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::get_Qms_VS,     communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::set_Qm0_AM,     communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::get_Qm0_AM,     communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::set_R_reg_VS,   communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_QQmt::get_R_reg_VS,   communicator));
        
        add_proxy(new LocalIntBuffer  (id, id_str,  proxy_disp_p + Ports_p::command_flow, communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  proxy_disp_p + Ports_p::set_p,        communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  proxy_disp_p + Ports_p::get_p,        communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  proxy_disp_p + Ports_p::num_set_qin,  communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  proxy_disp_p + Ports_p::num_set_qout, communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  proxy_disp_p + Ports_p::num_get_p,    communicator));
    }
    
    void init_buffers() {
	// 
	// Initialization of communication buffers
        //
	for (int i=0; i<1; i++) {
    	    QQmt0->add_buffer(new LocalIntBuffer  (QQmt0->id /*ms_id*/, QQmt0->id_str, 0 /*port*/, QQmt0->communicator));
            QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 1 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 2 /*port*/, QQmt0->communicator));
    	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 3 /*port*/, QQmt0->communicator));
            QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 4 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 5 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 6 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 7 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 8 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 9 /*port*/, QQmt0->communicator));
    	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 10 /*port*/, QQmt0->communicator));
            QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 11 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 12 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 13 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 14 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 15 /*port*/, QQmt0->communicator));
    	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 16 /*port*/, QQmt0->communicator));
            QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 17 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 18 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 19 /*port*/, QQmt0->communicator));
	    QQmt0->add_buffer(new LocalFloatBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 20 /*port*/, QQmt0->communicator));
        }

        for (int i=0; i<2; i++) {
	    PP[i]->add_buffer(new LocalIntBuffer  (PP[i]->id /*ms_id*/, PP[i]->id_str, 0 /*port*/, PP[i]->communicator));
    	    PP[i]->add_buffer(new LocalFloatBuffer(PP[i]->id /*ms_id*/, PP[i]->id_str, 1 /*port*/, PP[i]->communicator));
            PP[i]->add_buffer(new LocalFloatBuffer(PP[i]->id /*ms_id*/, PP[i]->id_str, 2 /*port*/, PP[i]->communicator));
	    PP[i]->add_buffer(new LocalFloatBuffer(PP[i]->id /*ms_id*/, PP[i]->id_str, 3 /*port*/, PP[i]->communicator));
    	    PP[i]->add_buffer(new LocalFloatBuffer(PP[i]->id /*ms_id*/, PP[i]->id_str, 4 /*port*/, PP[i]->communicator));
    	    PP[i]->add_buffer(new LocalFloatBuffer(PP[i]->id /*ms_id*/, PP[i]->id_str, 5 /*port*/, PP[i]->communicator));
        }
    }
    
    void set_communications() {
	string master = id_str;

	//
        // Command Flow 
        //
    	    // Master->QQmt
	communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_QQmt::command_flow  + proxy_disp /*port*/, 
						 "QQmt0" /*rcv_id*/, Ports_QQmt::command_flow));
    	    // Master->P
	for (int i=0; i<2; i++) 
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, proxy_disp_p + Ports_p::command_flow  + proxy_disp /*port*/,
						 "P" + to_string(i) /*rcv_id*/, Ports_p::command_flow));

        // Data Flow Master->QQmt
	communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_QQmt::set_Q_OS  + proxy_disp /*port*/, 
					 "QQmt0" /*rcv_id*/, Ports_QQmt::set_Q_OS,
					 N_OS /*size*/));
	communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_QQmt::set_Q_Streb  + proxy_disp /*port*/, 
					 "QQmt0" /*rcv_id*/, Ports_QQmt::set_Q_Streb,
					 N_Streb /*size*/));
	communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_QQmt::set_Q_AM  + proxy_disp /*port*/, 
					 "QQmt0" /*rcv_id*/, Ports_QQmt::set_Q_AM,
					 N_AM /*size*/));
	communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_QQmt::set_Q_VS  + proxy_disp /*port*/, 
					 "QQmt0" /*rcv_id*/, Ports_QQmt::set_Q_VS,
					 N_VS /*size*/));
	communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_QQmt::set_Qm_AM  + proxy_disp /*port*/, 
					 "QQmt0" /*rcv_id*/, Ports_QQmt::set_Qm_AM,
					 N_AM /*size*/));
	communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_QQmt::set_Qm_VS  + proxy_disp /*port*/, 
					 "QQmt0" /*rcv_id*/, Ports_QQmt::set_Qm_VS,
					 N_VS /*size*/));
	communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_QQmt::set_Qs_VS  + proxy_disp /*port*/, 
					 "QQmt0" /*rcv_id*/, Ports_QQmt::set_Qs_VS,
					 N_VS /*size*/));
	communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_QQmt::set_Qms_VS  + proxy_disp /*port*/, 
					 "QQmt0" /*rcv_id*/, Ports_QQmt::set_Qms_VS,
					 N_VS /*size*/));
	communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_QQmt::set_Qm0_AM  + proxy_disp /*port*/, 
					 "QQmt0" /*rcv_id*/, Ports_QQmt::set_Qm0_AM));
					 /*1*/
	communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_QQmt::set_R_reg_VS  + proxy_disp /*port*/, 
					 "QQmt0" /*rcv_id*/, Ports_QQmt::set_R_reg_VS,
					 N_VS /*size*/));
        // Data Flow Master->P
	for (int i=0; i<2; i++)
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, proxy_disp_p + Ports_p::set_p + proxy_disp  /*port*/, 
						 "P" + to_string(i) /*rcv_id*/, Ports_p::set_p));

	// Data Flow Q->Master
	communicator->add_comm_link(new CommLink("QQmt0" /*snd_id*/, Ports_QQmt::get_Q_OS,
					 master /*rcv_id*/, Ports_QQmt::get_Q_OS + proxy_disp  /*port*/,
					 N_OS /*size*/));
	communicator->add_comm_link(new CommLink("QQmt0" /*snd_id*/, Ports_QQmt::get_Q_Streb,
					 master /*rcv_id*/, Ports_QQmt::get_Q_Streb + proxy_disp  /*port*/,
					 N_Streb /*size*/));
	communicator->add_comm_link(new CommLink("QQmt0" /*snd_id*/, Ports_QQmt::get_Q_AM,
					 master /*rcv_id*/, Ports_QQmt::get_Q_AM + proxy_disp  /*port*/,
					 N_AM /*size*/));
	communicator->add_comm_link(new CommLink("QQmt0" /*snd_id*/, Ports_QQmt::get_Q_VS,
					 master /*rcv_id*/, Ports_QQmt::get_Q_VS + proxy_disp  /*port*/,
					 N_VS /*size*/));
	communicator->add_comm_link(new CommLink("QQmt0" /*snd_id*/, Ports_QQmt::get_Qm_AM,
					 master /*rcv_id*/, Ports_QQmt::get_Qm_AM + proxy_disp  /*port*/,
					 N_AM /*size*/));
	communicator->add_comm_link(new CommLink("QQmt0" /*snd_id*/, Ports_QQmt::get_Qm_VS,
					 master /*rcv_id*/, Ports_QQmt::get_Qm_VS + proxy_disp  /*port*/,
					 N_VS /*size*/));
	communicator->add_comm_link(new CommLink("QQmt0" /*snd_id*/, Ports_QQmt::get_Qs_VS,
					 master /*rcv_id*/, Ports_QQmt::get_Qs_VS + proxy_disp  /*port*/,
					 N_VS /*size*/));
	communicator->add_comm_link(new CommLink("QQmt0" /*snd_id*/, Ports_QQmt::get_Qms_VS,
					 master /*rcv_id*/, Ports_QQmt::get_Qms_VS + proxy_disp  /*port*/,
					 N_VS /*size*/));
	communicator->add_comm_link(new CommLink("QQmt0" /*snd_id*/, Ports_QQmt::get_Qm0_AM,
					 master /*rcv_id*/, Ports_QQmt::get_Qm0_AM + proxy_disp  /*port*/,
					 N_AM /*size*/));
	communicator->add_comm_link(new CommLink("QQmt0" /*snd_id*/, Ports_QQmt::get_R_reg_VS,
					 master /*rcv_id*/, Ports_QQmt::get_R_reg_VS + proxy_disp  /*port*/,
					 N_VS /*size*/));
	// Data Flow P->Master
        for (int i=0; i<2; i++) 
	    communicator->add_comm_link(new CommLink("P" + to_string(i) /*rcv_id*/, Ports_p::get_p, 
						 master /*snd_id*/, proxy_disp_p + Ports_p::get_p + proxy_disp  /*port*/));
    
	// Data Flow P->Q
        communicator->add_comm_link(new CommLink("P0"/*snd_id*/, Ports_p::num_get_p, 
						 "Q_OS_q0" /*rcv_id*/, Ports_q::num_set_pstart));
						 
        communicator->add_comm_link(new CommLink("P1"/*snd_id*/, Ports_p::num_get_p, 
						 "Q_VS_q"+to_string(N_VS-1)/*rcv_id*/, Ports_q::num_set_pend));
						 
    }                
                
    // sets value of all q-approx_elements
    void set_Q_OS(float* values /*[n]*/) {
	// preparation of buffer
	proxy_clear(Ports_QQmt::set_Q_OS);
	
	for (int i=0; i<N_OS; i++) {
	    add_proxy_value<float>(Ports_QQmt::set_Q_OS, values[i]);
	}
	proxy_flush_collective_replicate(Ports_QQmt::set_Q_OS);
	proxy_clear(Ports_QQmt::set_Q_OS);
	
	add_proxy_value<int>(Ports_QQmt::command_flow, Commands_QQmt::set_Q_OS);
	proxy_flush_collective_replicate(Ports_QQmt::command_flow);
	proxy_clear(Ports_QQmt::command_flow);
    }

    // sets value of all q-approx_elements
    void set_Q_Streb(float* values /*[n]*/) {
	// preparation of buffer
	proxy_clear(Ports_QQmt::set_Q_Streb);
	
	for (int i=0; i<N_Streb; i++) {
	    add_proxy_value<float>(Ports_QQmt::set_Q_Streb, values[i]);
	}
	proxy_flush_collective_replicate(Ports_QQmt::set_Q_Streb);
	proxy_clear(Ports_QQmt::set_Q_Streb);
	
	add_proxy_value<int>(Ports_QQmt::command_flow, Commands_QQmt::set_Q_Streb);
	proxy_flush_collective_replicate(Ports_QQmt::command_flow);
	proxy_clear(Ports_QQmt::command_flow);
    }
    
    // sets value of all q-approx_elements
    void set_Q_AM(float* values /*[n]*/) {
	// preparation of buffer
	proxy_clear(Ports_QQmt::set_Q_AM);
	
	for (int i=0; i<N_AM; i++) {
	    add_proxy_value<float>(Ports_QQmt::set_Q_AM, values[i]);
	}
	proxy_flush_collective_spread(Ports_QQmt::set_Q_AM);
	proxy_clear(Ports_QQmt::set_Q_AM);
	
	add_proxy_value<int>(Ports_QQmt::command_flow, Commands_QQmt::set_Q_AM);
	proxy_flush_collective_replicate(Ports_QQmt::command_flow);
	proxy_clear(Ports_QQmt::command_flow);
    }
    
    // sets value of all q-approx_elements
    void set_Q_VS(float* values /*[n]*/) {
	// preparation of buffer
	proxy_clear(Ports_QQmt::set_Q_VS);
	
	for (int i=0; i<N_VS; i++) {
	    add_proxy_value<float>(Ports_QQmt::set_Q_VS, values[i]);
	}
	proxy_flush_collective_spread(Ports_QQmt::set_Q_VS);
	proxy_clear(Ports_QQmt::set_Q_VS);
	
	add_proxy_value<int>(Ports_QQmt::command_flow, Commands_QQmt::set_Q_VS);
	proxy_flush_collective_replicate(Ports_QQmt::command_flow);
	proxy_clear(Ports_QQmt::command_flow);
    }

    // sets value of all q-approx_elements
    void set_Qm_AM(float* values /*[n]*/) {
	// preparation of buffer
	proxy_clear(Ports_QQmt::set_Qm_AM);
	
	for (int i=0; i<N_AM; i++) {
	    add_proxy_value<float>(Ports_QQmt::set_Qm_AM, values[i]);
	}
	proxy_flush_collective_spread(Ports_QQmt::set_Qm_AM);
	proxy_clear(Ports_QQmt::set_Qm_AM);
	
	add_proxy_value<int>(Ports_QQmt::command_flow, Commands_QQmt::set_Qm_AM);
	proxy_flush_collective_replicate(Ports_QQmt::command_flow);
	proxy_clear(Ports_QQmt::command_flow);
    }
    
    // sets value of all q-approx_elements
    void set_Qms_VS(float* values /*[n]*/) {
	// preparation of buffer
	proxy_clear(Ports_QQmt::set_Qms_VS);
	
	for (int i=0; i<N_VS; i++) {
	    add_proxy_value<float>(Ports_QQmt::set_Qms_VS, values[i]);
	}
	proxy_flush_collective_spread(Ports_QQmt::set_Qms_VS);
	proxy_clear(Ports_QQmt::set_Qms_VS);
	
	add_proxy_value<int>(Ports_QQmt::command_flow, Commands_QQmt::set_Qms_VS);
	proxy_flush_collective_replicate(Ports_QQmt::command_flow);
	proxy_clear(Ports_QQmt::command_flow);
    }

    // sets value of all q-approx_elements
    void set_Qm0_AM(float* values /*[1]*/) {
	// preparation of buffer
	proxy_clear(Ports_QQmt::set_Qm0_AM);
	
	for (int i=0; i<1; i++) {
	    add_proxy_value<float>(Ports_QQmt::set_Qm0_AM, values[i]);
	}
	proxy_flush_collective_spread(Ports_QQmt::set_Qm0_AM);
	proxy_clear(Ports_QQmt::set_Qm0_AM);
	
	add_proxy_value<int>(Ports_QQmt::command_flow, Commands_QQmt::set_Qm0_AM);
	proxy_flush_collective_replicate(Ports_QQmt::command_flow);
	proxy_clear(Ports_QQmt::command_flow);
    }
    
    // sets p of all P_element
    void set_P(float* values  /*[2]*/) {
	proxy_clear(proxy_disp_p + Ports_p::set_p);

	for (int i=0; i<2; i++)
	    add_proxy_value<float>(proxy_disp_p + Ports_p::set_p, values[i]);
	proxy_flush_collective_spread(proxy_disp_p + Ports_p::set_p);
	proxy_clear(proxy_disp_p + Ports_p::set_p);
	
	add_proxy_value<int>(proxy_disp_p + Ports_p::command_flow, Commands_p::set_p);
	proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);
	proxy_clear(proxy_disp_p + Ports_p::command_flow);
    }

    // gets p of all P-elements
    void get_p(float* values  /*[2]*/) {
	proxy_clear(proxy_disp_p + Ports_p::get_p);
    
	add_proxy_value<int>(proxy_disp_p + Ports_p::command_flow, Commands_p::get_p);
	proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);
	proxy_clear(proxy_disp_p + Ports_p::command_flow);	
	
	proxy_sync(proxy_disp_p + Ports_p::get_p);
	for (int i=0; i<2; i++)
	    values[i] = get_proxy_value<float>(proxy_disp_p + Ports_p::get_p, i);    
    }
    
    // gets value of all q-approx_elements
    void get_Q_OS(float* values /*[n]*/) {
        proxy_clear(Ports_QQmt::get_Q_OS);

        add_proxy_value<int>(Ports_QQmt::command_flow, Commands_QQmt::get_Q_OS);
        proxy_flush_collective_replicate(Ports_QQmt::command_flow);
        proxy_clear(Ports_QQmt::command_flow);
    
        proxy_sync(Ports_QQmt::get_Q_OS);
        for (int i=0; i<N_OS; i++)
            values[i] = get_proxy_value<float>(Ports_QQmt::get_Q_OS, i);
        proxy_clear(Ports_QQmt::get_Q_OS);
    }


    // gets value of all q-approx_elements
    void get_Q_VS(float* values /*[n]*/) {
        proxy_clear(Ports_QQmt::get_Q_VS);

        add_proxy_value<int>(Ports_QQmt::command_flow, Commands_QQmt::get_Q_VS);
        proxy_flush_collective_replicate(Ports_QQmt::command_flow);
        proxy_clear(Ports_QQmt::command_flow);
    
        proxy_sync(Ports_QQmt::get_Q_VS);
        for (int i=0; i<N_VS; i++)
            values[i] = get_proxy_value<float>(Ports_QQmt::get_Q_VS, i);
        proxy_clear(Ports_QQmt::get_Q_VS);
    }

    // gets value of all q-approx_elements
    void get_Qm_AM(float* values /*[n]*/) {
        proxy_clear(Ports_QQmt::get_Qm_AM);

        add_proxy_value<int>(Ports_QQmt::command_flow, Commands_QQmt::get_Qm_AM);
        proxy_flush_collective_replicate(Ports_QQmt::command_flow);
        proxy_clear(Ports_QQmt::command_flow);
    
        proxy_sync(Ports_QQmt::get_Qm_AM);
        for (int i=0; i<N_AM; i++)
            values[i] = get_proxy_value<float>(Ports_QQmt::get_Qm_AM, i);
        proxy_clear(Ports_QQmt::get_Qm_AM);
    }

    // inits time stamp for all ms
    void init_time() {
	add_proxy_value(Ports_QQmt::command_flow, Commands_QQmt::init_time/*value*/);
        proxy_flush_collective_replicate(Ports_QQmt::command_flow);
        proxy_clear(Ports_QQmt::command_flow);

        add_proxy_value(proxy_disp_p + Ports_p::command_flow, Commands_p::init_time /*value*/);
        proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);
        proxy_clear(proxy_disp_p + Ports_p::command_flow);
    }
    
    // performs 1 simulation step
    void simulation_step() {
	add_proxy_value<int>(Ports_QQmt::command_flow, Commands_QQmt::simulation/*value*/);
        proxy_flush_collective_replicate(Ports_QQmt::command_flow);
        proxy_clear(Ports_QQmt::command_flow);

        add_proxy_value<int>(proxy_disp_p + Ports_p::command_flow, Commands_p::simulation /*value*/);
        proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);
        proxy_clear(proxy_disp_p + Ports_p::command_flow);
    }

        
    void run() {

	float q_OS[N_OS];
	float q_Streb[N_Streb];
	float q_AM[N_AM];
	float q_VS[N_VS];
	float qm_AM[N_AM];
	float qm_VS[N_VS];
	float qms_VS[N_VS];
	
	float q_OS_old[N_OS];
	float q_Streb_old[N_Streb];
	float q_AM_old[N_AM];
	float q_VS_old[N_VS];
	float qm_AM_old[N_AM];
	float qm_VS_old[N_VS];
	
	float qm0_AM[1];


	float P[2];

        // Setting initial values of P and Q
        P[0] = H_end;
        P[1] = 0;
        set_P(P);
        
	for (int i=0; i<N_OS; i++) {
	    q_OS[i] = 7.2;
	    q_OS_old[i] = 7.2;
	}
	set_Q_OS(q_OS);
	
	for (int i=0; i<N_Streb; i++) {
	    q_Streb[i] = 5.6;
	    q_Streb_old[i] = 5.6;
	}
	set_Q_Streb(q_Streb);
		
	for (int i=0; i<N_AM; i++) {
	    q_AM[i] = 1.6;
	    qm_AM[i] = Qm0;
	    
	    q_AM_old[i] = 1.6;
	    qm_AM_old[i] = Qm0;
	}
	set_Q_AM(q_AM);
	set_Qm_AM(qm_AM);
	
	for (int i=0; i<1; i++) {
	    qm0_AM[i] = Qm0;
	}
	set_Qm0_AM(qm0_AM);
	
	for (int i=0; i<N_VS; i++) {
	    q_VS[i] = 7.2;
	    qm_VS[i] = 7.2;
	    qms_VS[i] = Qm0;
	    
	    q_VS_old[i] = 7.2;
	    qm_VS_old[i] = Qm0;
	}
	qms_VS[0] = Qm0;
	set_Q_VS(q_VS);
	set_Qms_VS(qms_VS);
	

	bool is_converged = false;
	int num_step = 0;

	init_time();

	// Simulation
//	for (int i=0; i<1; i++) {
	while ( !is_converged ) {
    	    cout << "============== Iteration " << num_step << "==============" << endl;
    	    
    	    simulation_step();
    	    num_step++;
	    
    	    get_Q_VS(q_VS);
    	    get_Qm_AM(qm_AM);

    	    is_converged = true;
    	    
    	    if (fabs(q_VS[0]-q_VS_old[0]) > solv_params.precision)
                is_converged = false;
            else if (fabs(qm_AM[0]-qm_AM_old[0]) > solv_params.precision)
                is_converged = false;
            	
    	    q_VS_old[0] = q_VS[0];
    	    qm_AM_old[0] = qm_AM[0];
	}

	// Stopping the worker-ms
        add_proxy_value<int>(Ports_QQmt::command_flow, Commands_QQmt::stop);
        proxy_flush_collective_replicate(Ports_QQmt::command_flow);
        proxy_clear(Ports_QQmt::command_flow);

        add_proxy_value<int>(proxy_disp_p + Ports_p::command_flow, Commands_p::stop);
        proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);
        proxy_clear(proxy_disp_p + Ports_p::command_flow);
    }
}; // class Master


int main (int argc, char* argv[]) {
    int mpi_rank, mpi_size;
    int provided;
    
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    int   n = 10;         // nr. of q- approx.elements
        
    //
    // parameters of Q-elements
    //
        // OS
    float OS_S = 7.0;   // square cut
    float OS_R = 0.24;  // resistance
    float OS_L = 500;   // length
        // Streb
    float Streb_S = 2.3;   // square cut
    float Streb_R = 6.61;  // resistance
    float Streb_L = 130;   // length
        // AM
    float AM_S = 2.3;   // square cut
    float AM_R = 81  ;  // resistance
    float AM_L = 130;   // length
    float AM_V = 2800.0;
    float AM_Qm0 = 0.0175;
        // VS
    float VS_S = 5.5;   // square cut
    float VS_R = 0.44;  // resistance
    float VS_L = 500;   // length

    float dX = 50;


    // parameters of P-elements
    float H_start = 0;
    float H_end = 242.6;
            
    // numeric parameters
    Solver_Params solv_params = { 0.15        /*time step in s.*/,
                                  0.000001    /*precision*/,
                                  30          /*nr. of numeric steps in sim block*/  };
    
    // Deployment options
    MpiProcessMap mpi_map;
    
    mpi_map.add({"QQmt0",               0});

        // OS
    mpi_map.add({"Q_OS",                1});
    mpi_map.add({"Q_OS_q0",             2});
    mpi_map.add({"Q_OS_q1",             3});
    mpi_map.add({"Q_OS_q2",             4});
    mpi_map.add({"Q_OS_q3",             5});
    mpi_map.add({"Q_OS_q4",             6});
    mpi_map.add({"Q_OS_q5",             7});
    mpi_map.add({"Q_OS_q6",             8});
    mpi_map.add({"Q_OS_q7",             9});
    mpi_map.add({"Q_OS_q8",             10});
    mpi_map.add({"Q_OS_q9",             11});
    mpi_map.add({"Q_OS_p0",             12});
    mpi_map.add({"Q_OS_p1",             13});
    mpi_map.add({"Q_OS_p2",             14});
    mpi_map.add({"Q_OS_p3",             15});
    mpi_map.add({"Q_OS_p4",             16});
    mpi_map.add({"Q_OS_p5",             17});
    mpi_map.add({"Q_OS_p6",             18});
    mpi_map.add({"Q_OS_p7",             19});
    mpi_map.add({"Q_OS_p8",             20});

        // Streb
    mpi_map.add({"Q_Streb",             21});
    mpi_map.add({"Q_Streb_q0",          22});
    mpi_map.add({"Q_Streb_q1",          23});
    mpi_map.add({"Q_Streb_q2",          24});
    mpi_map.add({"Q_Streb_p0",          25});
    mpi_map.add({"Q_Streb_p1",          26});

        // AM
    mpi_map.add({"Q_AM",                27});
    mpi_map.add({"Q_AM_qm0",            28});
    mpi_map.add({"Q_AM_q0",             28});
    mpi_map.add({"Q_AM_qmt1",            29});
    mpi_map.add({"Q_AM_q1",             29});
    mpi_map.add({"Q_AM_qmt2",            30});
    mpi_map.add({"Q_AM_q2",             30});
    mpi_map.add({"Q_AM_p0",             31});
    mpi_map.add({"Q_AM_p1",             32});

        // VS
    mpi_map.add({"Q_VS",                33});
    mpi_map.add({"Q_VS_qmt0",           34});
    mpi_map.add({"Q_VS_q0",             34});
    mpi_map.add({"Q_VS_qmt1",           35});
    mpi_map.add({"Q_VS_q1",             35});
    mpi_map.add({"Q_VS_qmt2",           36});
    mpi_map.add({"Q_VS_q2",             36});
    mpi_map.add({"Q_VS_qmt3",           37});
    mpi_map.add({"Q_VS_q3",             37});
    mpi_map.add({"Q_VS_qmt4",           38});
    mpi_map.add({"Q_VS_q4",             38});
    mpi_map.add({"Q_VS_qmt5",           39});
    mpi_map.add({"Q_VS_q5",             39});
    mpi_map.add({"Q_VS_qmt6",           40});
    mpi_map.add({"Q_VS_q6",             40});
    mpi_map.add({"Q_VS_qmt7",           41});
    mpi_map.add({"Q_VS_q7",             41});
    mpi_map.add({"Q_VS_qmt8",           42});
    mpi_map.add({"Q_VS_q8",             42});
    mpi_map.add({"Q_VS_qmt9",           43});
    mpi_map.add({"Q_VS_q9",             43});
    mpi_map.add({"Q_VS_p0",             44});
    mpi_map.add({"Q_VS_p1",             45});
    mpi_map.add({"Q_VS_p2",             46});
    mpi_map.add({"Q_VS_p3",             47});
    mpi_map.add({"Q_VS_p4",             48});
    mpi_map.add({"Q_VS_p5",             49});
    mpi_map.add({"Q_VS_p6",             50});
    mpi_map.add({"Q_VS_p7",             51});
    mpi_map.add({"Q_VS_p8",             52});

    mpi_map.add({"Pqmt0",               53});
    mpi_map.add({"Pqmt1",               54});


    mpi_map.add({"G3_1",                      55});
    mpi_map.add({"P0",                        56});
    mpi_map.add({"P1",                        57});


    MpiCommunicator* communicator = new MpiCommunicator(mpi_map);

    char experiment_id_char[23+1];
    if (mpi_rank == 0) {
	Time_MS now;
        now.init_time();
	string experiment_id = now.time_stamp();
	cout << "Experiment_id: " << experiment_id << endl;
	strcpy(experiment_id_char, experiment_id.c_str());
    }
    MPI_Bcast(experiment_id_char, 24, MPI_CHAR, 0, MPI_COMM_WORLD);

    string experiment_id(experiment_id_char);
    
    Monitoring_opts* mon_opts = new Monitoring_opts();
    mon_opts->experiment_id     = experiment_id;
    mon_opts->flag_output_file  = 1;
    mon_opts->buf_size 		= 1;

    Element_model* model_G3_1 = new Element_model(
	    0, "G3_1", communicator,
	    "Section1" /*section*/, "Network1" /*network*/,
	    H_start, H_end, AM_Qm0,
            mon_opts,
/*OS*/      OS_S, OS_R, OS_L,
/*Streb*/   Streb_S, Streb_R, Streb_L,
/*AM*/      AM_S, AM_R, AM_L, AM_V,
/*VS*/      VS_S, VS_R, VS_L,
            dX,
            solv_params);

    MpiDeploymentPool deployment_pool(mpi_map);
    deployment_pool.add_ms(model_G3_1);
    deployment_pool.deploy_all();
    deployment_pool.join_all();
    
    MPI_Finalize();
}