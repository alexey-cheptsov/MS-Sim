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

#include "../../Services/A3_1/A3_1.h"
#include "../../Services/A1_2/A1_2.h"
#include "../../mpi_communicator.h"
#include "../../mpi_deployment.h"


using namespace std;
using namespace A3_1;
using namespace A1_2;

enum Elements : int {
    q340 = 0,
    q486 = 1,
    q487 = 2,
    q541 = 3,
    q582 = 4,
    q380 = 5,
    
    qtotal = 6
};

enum Nodes : int {
    p541 		= 0,
    p486 		= 0,
    p486_487 		= 1,
    p487_541_582 	= 2,
    p582_340 		= 3,
    p340_380 		= 4,
    p380 		= 5,
    
    ptotal = 6
};

class Section_model: public Microservice {
public:

    float* H; // [Nodes::ptotal];
    float q_init; // initializatin value of all elements' q's
    
    Solver_Params solv_params;
    
    int N[Elements::qtotal]; // nr. of approx. units in every element
    
    int proxy_disp_p = 9; // proxy displacement of p- nodes
                           // (contiguous numbering of proxy-ports)

    Q* QQ[Elements::qtotal];  // underlying MS
    p* PP[Nodes::ptotal];
    
    DeploymentPool* deployment_pool;
                        
    Section_model(int id_, string id_str_, MpiCommunicator* communicator_,
                string section_, string network_,
                Monitoring_opts* mon_opts_,
        /*Q*/   float* S_, float* R_, float* L_, float q_init_,
        /*P*/	float* SP_, bool* is_bound_, float* H_,
    		float dX_,
                Solver_Params& solv_params_)
            	    : Microservice(id_, id_str_, communicator_)
    {
	H = H_;
	q_init = q_init_;
	
	for (int i=0; i<Elements::qtotal; i++)
	    N[i] = round(L_[i]/dX_);

	solv_params = solv_params_;
	
        // Setup of communication map
        set_communications();
        //communicator->print_comm_links();
        
        for (int i=0; i<Elements::qtotal; i++)
    	    QQ[i] = new Q(0/*id*/, "Q"+to_string(i), communicator_,
                    "Q"+to_string(i) /*element*/, "Section1" /*section*/, "Network1" /*network*/,
                    mon_opts_,
                    S_[i] /*S*/, R_[i] /*R*/, L_[i] /*L*/, dX_, solv_params);

	for (int i=0; i<Nodes::ptotal; i++)
            PP[i] = new p(0/*id*/, "P"+to_string(i), communicator_, 
			 "P"+to_string(i) /*name*/, "Section1" /*section*/, "Network1" /*network*/,
			 mon_opts_,
			 SP_[i] /*S*/, dX_ /*dX*/, is_bound_[i] /*is_bound*/, solv_params_);
	
	// Init underlying ms
        init_proxies();
        init_buffers();

        // Deployment pool initialization
        deployment_pool = new MpiDeploymentPool(communicator_->mpi_map);

        // Spawning worker threads for underlying ms
        for (int i=0; i<Elements::qtotal; i++)
    	    deployment_pool->add_ms(QQ[i]);
    	for (int i=0; i<Nodes::ptotal; i++)
    	    deployment_pool->add_ms(PP[i]);
    	    
        deployment_pool->deploy_all();
        deployment_pool->join_all();
    };
    
    void init_proxies() {
	// 
        // Initialization of communication proxies
	//

	add_proxy(new LocalIntBuffer  (id, id_str,  Ports_Q::command_flow,   communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_Q::set_Q,          communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_Q::get_Q,          communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_Q::set_Qs,         communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_Q::get_Qs,         communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_Q::set_P,          communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_Q::get_P,          communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_Q::set_R_reg,      communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_Q::get_R_reg,      communicator));

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
	for (int i=0; i<Elements::qtotal; i++) {
    	    QQ[i]->add_buffer(new LocalIntBuffer  (QQ[i]->id /*ms_id*/, QQ[i]->id_str, 0 /*port*/, QQ[i]->communicator));
            QQ[i]->add_buffer(new LocalFloatBuffer(QQ[i]->id /*ms_id*/, QQ[i]->id_str, 1 /*port*/, QQ[i]->communicator));
	    QQ[i]->add_buffer(new LocalFloatBuffer(QQ[i]->id /*ms_id*/, QQ[i]->id_str, 2 /*port*/, QQ[i]->communicator));
    	    QQ[i]->add_buffer(new LocalFloatBuffer(QQ[i]->id /*ms_id*/, QQ[i]->id_str, 3 /*port*/, QQ[i]->communicator));
            QQ[i]->add_buffer(new LocalFloatBuffer(QQ[i]->id /*ms_id*/, QQ[i]->id_str, 4 /*port*/, QQ[i]->communicator));
	    QQ[i]->add_buffer(new LocalFloatBuffer(QQ[i]->id /*ms_id*/, QQ[i]->id_str, 5 /*port*/, QQ[i]->communicator));
	    QQ[i]->add_buffer(new LocalFloatBuffer(QQ[i]->id /*ms_id*/, QQ[i]->id_str, 6 /*port*/, QQ[i]->communicator));
	    QQ[i]->add_buffer(new LocalFloatBuffer(QQ[i]->id /*ms_id*/, QQ[i]->id_str, 7 /*port*/, QQ[i]->communicator));
	    QQ[i]->add_buffer(new LocalFloatBuffer(QQ[i]->id /*ms_id*/, QQ[i]->id_str, 8 /*port*/, QQ[i]->communicator));
        }

        for (int i=0; i<Nodes::ptotal; i++) {
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

	// Command Flow Master->Q
        for (int i=0; i<Elements::qtotal; i++)
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_Q::command_flow  + proxy_disp /*port*/,
                                                 "Q" + to_string(i) /*rcv_id*/, Ports_Q::command_flow));

        // Command Flow Master->P
        for (int i=0; i<Nodes::ptotal; i++)
            communicator->add_comm_link(new CommLink(master /*snd_id*/, proxy_disp_p + Ports_p::command_flow  + proxy_disp /*port*/,
                                                 "P" + to_string(i) /*rcv_id*/, Ports_p::command_flow));

        // Data Flow Master->Q
        for (int i=0; i<Elements::qtotal; i++) {
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_Q::set_Q  + proxy_disp /*port*/,
                                                 "Q" + to_string(i) /*rcv_id*/, Ports_Q::set_Q,
                                                 N[i] /*size*/));
            communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_Q::set_P  + proxy_disp /*port*/,
                                                 "Q" + to_string(i) /*rcv_id*/, Ports_Q::set_P,
                                                 N[i]-1 /*size*/));
        }

        // Data Flow Master->P
        for (int i=0; i<Nodes::ptotal; i++)
            communicator->add_comm_link(new CommLink(master /*snd_id*/, proxy_disp_p + Ports_p::set_p + proxy_disp  /*port*/,
                                                 "P" + to_string(i) /*rcv_id*/, Ports_p::set_p));

        // Data Flow Q->Master
        for (int i=0; i<Elements::qtotal; i++) {
            communicator->add_comm_link(new CommLink("Q" + to_string(i) /*snd_id*/, Ports_Q::get_Q,
                                                 master /*rcv_id*/, Ports_Q::get_Q + proxy_disp  /*port*/,
                                                 N[i] /*size*/));
            communicator->add_comm_link(new CommLink("Q" + to_string(i) /*snd_id*/, Ports_Q::get_P,
                                                 master /*rcv_id*/, Ports_Q::get_P + proxy_disp  /*port*/,
                                                 N[i]-1 /*size*/));
        }

        // Data Flow P->Master
        for (int i=0; i<Nodes::ptotal; i++)
            communicator->add_comm_link(new CommLink("P" + to_string(i) /*rcv_id*/, Ports_p::get_p,
                                                 master /*snd_id*/, proxy_disp_p + Ports_p::get_p + proxy_disp  /*port*/));


        // Data Flow Q->P : TOPOLOGY
            // p541
            // p486
    	    // p380
    	    // p486_487
	communicator->add_comm_link(new CommLink("Q"+to_string(Elements::q486)+"_q"+to_string(N[Elements::q486]-1)/*snd_id*/, Ports_q::num_get_q,
                                                 "P"+to_string(Nodes::p486_487) /*rcv_id*/,   Ports_p::num_set_qin));
        communicator->add_comm_link(new CommLink("Q"+to_string(Elements::q487)+"_q0"/*snd_id*/, Ports_q::num_get_q,
                                                 "P"+to_string(Nodes::p486_487) /*rcv_id*/,   Ports_p::num_set_qout));
            // p487_541_582
	communicator->add_comm_link(new CommLink("Q"+to_string(Elements::q487)+"_q"+to_string(N[Elements::q487]-1)/*snd_id*/, Ports_q::num_get_q,
                                                 "P"+to_string(Nodes::p487_541_582) /*rcv_id*/,   Ports_p::num_set_qin));
	communicator->add_comm_link(new CommLink("Q"+to_string(Elements::q541)+"_q"+to_string(N[Elements::q541]-1)/*snd_id*/, Ports_q::num_get_q,
                                                 "P"+to_string(Nodes::p487_541_582) /*rcv_id*/,   Ports_p::num_set_qin));
        communicator->add_comm_link(new CommLink("Q"+to_string(Elements::q582)+"_q0"/*snd_id*/, Ports_q::num_get_q,
                                                 "P"+to_string(Nodes::p487_541_582) /*rcv_id*/,   Ports_p::num_set_qout));
    	    // p582_340
	communicator->add_comm_link(new CommLink("Q"+to_string(Elements::q582)+"_q"+to_string(N[Elements::q582]-1)/*snd_id*/, Ports_q::num_get_q,
                                                 "P"+to_string(Nodes::p582_340) /*rcv_id*/,   Ports_p::num_set_qin));
        communicator->add_comm_link(new CommLink("Q"+to_string(Elements::q340)+"_q0"/*snd_id*/, Ports_q::num_get_q,
                                                 "P"+to_string(Nodes::p582_340) /*rcv_id*/,   Ports_p::num_set_qout));
    	    // p340_380
	communicator->add_comm_link(new CommLink("Q"+to_string(Elements::q340)+"_q"+to_string(N[Elements::q340]-1)/*snd_id*/, Ports_q::num_get_q,
                                                 "P"+to_string(Nodes::p340_380) /*rcv_id*/,   Ports_p::num_set_qin));
        communicator->add_comm_link(new CommLink("Q"+to_string(Elements::q380)+"_q0"/*snd_id*/, Ports_q::num_get_q,
                                                 "P"+to_string(Nodes::p340_380) /*rcv_id*/,   Ports_p::num_set_qout));

            
            

        // Data Flow P->Q : TOPOLOGY
    	    // q340
    	communicator->add_comm_link(new CommLink("P"+to_string(Nodes::p582_340)/*snd_id*/, Ports_p::num_get_p,
                                                 "Q"+to_string(Elements::q340)+"_q0" /*rcv_id*/, Ports_q::num_set_pstart));
        communicator->add_comm_link(new CommLink("P"+to_string(Nodes::p340_380)/*snd_id*/, Ports_p::num_get_p,
                                                 "Q"+to_string(Elements::q340)+"_q"+to_string(N[Elements::q340]-1)/*rcv_id*/, Ports_q::num_set_pend));
    	    // q486
    	communicator->add_comm_link(new CommLink("P"+to_string(Nodes::p486)/*snd_id*/, Ports_p::num_get_p,
                                                 "Q"+to_string(Elements::q486)+"_q0" /*rcv_id*/, Ports_q::num_set_pstart));
        communicator->add_comm_link(new CommLink("P"+to_string(Nodes::p486_487)/*snd_id*/, Ports_p::num_get_p,
                                                 "Q"+to_string(Elements::q486)+"_q"+to_string(N[Elements::q486]-1)/*rcv_id*/, Ports_q::num_set_pend));
    	    // q487
    	communicator->add_comm_link(new CommLink("P"+to_string(Nodes::p486_487)/*snd_id*/, Ports_p::num_get_p,
                                                 "Q"+to_string(Elements::q487)+"_q0" /*rcv_id*/, Ports_q::num_set_pstart));
        communicator->add_comm_link(new CommLink("P"+to_string(Nodes::p487_541_582)/*snd_id*/, Ports_p::num_get_p,
                                                 "Q"+to_string(Elements::q487)+"_q"+to_string(N[Elements::q487]-1)/*rcv_id*/, Ports_q::num_set_pend));
    	    // q541
    	communicator->add_comm_link(new CommLink("P"+to_string(Nodes::p541)/*snd_id*/, Ports_p::num_get_p,
                                                 "Q"+to_string(Elements::q541)+"_q0" /*rcv_id*/, Ports_q::num_set_pstart));
        communicator->add_comm_link(new CommLink("P"+to_string(Nodes::p487_541_582)/*snd_id*/, Ports_p::num_get_p,
                                                 "Q"+to_string(Elements::q541)+"_q"+to_string(N[Elements::q541]-1)/*rcv_id*/, Ports_q::num_set_pend));
    	    // q582
    	communicator->add_comm_link(new CommLink("P"+to_string(Nodes::p487_541_582)/*snd_id*/, Ports_p::num_get_p,
                                                 "Q"+to_string(Elements::q582)+"_q0" /*rcv_id*/, Ports_q::num_set_pstart));
        communicator->add_comm_link(new CommLink("P"+to_string(Nodes::p582_340)/*snd_id*/, Ports_p::num_get_p,
                                                 "Q"+to_string(Elements::q582)+"_q"+to_string(N[Elements::q582]-1)/*rcv_id*/, Ports_q::num_set_pend));
    	    // q380
    	communicator->add_comm_link(new CommLink("P"+to_string(Nodes::p340_380)/*snd_id*/, Ports_p::num_get_p,
                                                 "Q"+to_string(Elements::q380)+"_q0" /*rcv_id*/, Ports_q::num_set_pstart));
        communicator->add_comm_link(new CommLink("P"+to_string(Nodes::p380)/*snd_id*/, Ports_p::num_get_p,
                                                 "Q"+to_string(Elements::q380)+"_q"+to_string(N[Elements::q380]-1)/*rcv_id*/, Ports_q::num_set_pend));
						 
    }                
                
    // sets value of all q-approx_elements
    void set_Q(float** values /*[Elements::qtotal][Elements::N[i]]*/) {
	// preparation of buffer
	proxy_clear(Ports_Q::set_Q);
	
	for (int i=0; i<Elements::qtotal; i++)
	    for (int j=0; j<N[i]; j++)
		add_proxy_value<float>(Ports_Q::set_Q, values[i][j]);

	proxy_flush_collective_spread(Ports_Q::set_Q);
	proxy_clear(Ports_Q::set_Q);
	
	add_proxy_value<int>(Ports_Q::command_flow, Commands_Q::set_Q);
	proxy_flush_collective_replicate(Ports_Q::command_flow);
	proxy_clear(Ports_Q::command_flow);
    }

    // sets value of all q-approx_elements
    void set_R_reg(float** values /*[Elements::qtotal][Elements::N[i]]*/) {
	// preparation of buffer
	proxy_clear(Ports_Q::set_R_reg);
	
	for (int i=0; i<Elements::qtotal; i++)
	    for (int j=0; j<N[i]; j++)
		add_proxy_value<float>(Ports_Q::set_R_reg, values[i][j]);

	proxy_flush_collective_replicate(Ports_Q::set_R_reg);
	proxy_clear(Ports_Q::set_R_reg);
	
	add_proxy_value<int>(Ports_Q::command_flow, Commands_Q::set_R_reg);
	proxy_flush_collective_replicate(Ports_Q::command_flow);
	proxy_clear(Ports_Q::command_flow);
    }


    // sets value of all p-approx_elements
    void set_P(float** values /*[Elements::qtotal][Elements::N[i]]*/) {
	// preparation of buffer
	proxy_clear(Ports_Q::set_P);
	
	for (int i=0; i<Elements::qtotal; i++)
	    for (int j=0; j<N[i]; j++)
		add_proxy_value<float>(Ports_Q::set_P, values[i][j]);

	proxy_flush_collective_replicate(Ports_Q::set_P);
	proxy_clear(Ports_Q::set_P);
	
	add_proxy_value<int>(Ports_Q::command_flow, Commands_Q::set_P);
	proxy_flush_collective_replicate(Ports_Q::command_flow);
	proxy_clear(Ports_Q::command_flow);
    }
    
    
    void set_p(float* values  /*[Nodes::ptotal]*/) {
	proxy_clear(proxy_disp_p + Ports_p::set_p);

	for (int i=0; i<Nodes::ptotal; i++)
	    add_proxy_value<float>(proxy_disp_p + Ports_p::set_p, values[i]);
	proxy_flush_collective_spread(proxy_disp_p + Ports_p::set_p);
	proxy_clear(proxy_disp_p + Ports_p::set_p);
	
	add_proxy_value<int>(proxy_disp_p + Ports_p::command_flow, Commands_p::set_p);
	proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);
	proxy_clear(proxy_disp_p + Ports_p::command_flow);
    }

    // Gets value of all q-approx_elements
    void get_Q(float** values /*[Elements::qtotal][Elements::N[i]]*/) {
        proxy_clear(Ports_Q::get_Q);

        add_proxy_value<int>(Ports_Q::command_flow, Commands_Q::get_Q);
        proxy_flush_collective_replicate(Ports_Q::command_flow);
        proxy_clear(Ports_Q::command_flow);

        proxy_sync(Ports_Q::get_Q);
        int counter = 0;
        for (int i=0; i<Elements::qtotal; i++)
	    for (int j=0; j<N[i]; j++)
                values[i][j] = get_proxy_value<float>(Ports_Q::get_Q, counter++);
        proxy_clear(Ports_Q::get_Q);
    }
    
    // inits time stamp for all ms
    void init_time() {
        add_proxy_value(Ports_Q::command_flow, Commands_Q::init_time/*value*/);
        proxy_flush_collective_replicate(Ports_Q::command_flow);
        proxy_clear(Ports_Q::command_flow);
    
        add_proxy_value(proxy_disp_p + Ports_p::command_flow, Commands_p::init_time /*value*/);
        proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);
        proxy_clear(proxy_disp_p + Ports_p::command_flow);
    }

    // performs 1 simulation step
    void simulation_step() {
        add_proxy_value<int>(Ports_Q::command_flow, Commands_Q::simulation/*value*/);
        proxy_flush_collective_replicate(Ports_Q::command_flow);
        proxy_clear(Ports_Q::command_flow);

        add_proxy_value<int>(proxy_disp_p + Ports_p::command_flow, Commands_p::simulation /*value*/);
        proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);
        proxy_clear(proxy_disp_p + Ports_p::command_flow);
    }
    
    // sets pressure of node P5
    void set_P5(float P) {
	proxy_clear(proxy_disp_p + Ports_p::set_p);
	add_proxy_value<float>(proxy_disp_p + Ports_p::set_p, H[Nodes::p380]);
	proxy_flush_p2p_replicate(proxy_disp_p + Ports_p::set_p, "P"+to_string(Nodes::p380));
	proxy_clear(proxy_disp_p + Ports_p::set_p);
	
	add_proxy_value<int>(proxy_disp_p + Ports_p::command_flow, Commands_p::set_p);
	proxy_flush_p2p_replicate(proxy_disp_p + Ports_p::command_flow, "P"+to_string(Nodes::p380));
	proxy_clear(proxy_disp_p + Ports_p::command_flow);    
    }

    void run() {
	float* q[Elements::qtotal];
	float* q_old[Elements::qtotal];
	float P[Nodes::ptotal];
	
	for (int i=0; i<Elements::qtotal; i++) {
	    q[i] = new float[N[i]];
	    q_old[i] = new float[N[i]];
	}
	
	//    
	// Setting initial values 
	//
	    // q
	for (int i=0; i<Elements::qtotal; i++)
	    for (int j=0; j<N[i]; j++) {
		q[i][j] = q_init;
		q_old[i][j] = q_init;
	}
	set_Q(q);
	
	    // P
	set_p(H);

        
	int num_step = 0;
	init_time();

	//
	// Simulation part 1 - raise of P
	//
	float H_new = -36.7;
	
	bool is_converged = false;
	while ( !is_converged ) {
	    // P
	    if ( H[Nodes::p380] > H_new ) {
		H[Nodes::p380] = max (H_new, H[Nodes::p380] + (-1)*(solv_params.nr_num_steps*solv_params.time_step));
		set_P5(H[Nodes::p380]);
	    }
	
    	    cout << "============== Iteration " << num_step << "==============" << endl;
    	    
    	    simulation_step();
    	    num_step++;
	    
    	    get_Q(q);

    	    is_converged = true;
    	    
    	    if (fabs(q[0][0]-q_old[0][0]) > solv_params.precision)
                is_converged = false;
            	
    	    for (int i=0; i<Elements::qtotal; i++)
		for (int j=0; j<N[Elements::qtotal]; j++)
		    q_old[i][j] = q[i][j];
	}
	
	//
	// Simulation part 2 - drop of P
	//
	H_new = -18.35;
	
	is_converged = false;
	while ( !is_converged ) {
	    // P
            if ( H[Nodes::p380] < H_new ) {
                H[Nodes::p380] = min (H_new, H[Nodes::p380] + (1)*(solv_params.nr_num_steps*solv_params.time_step));
                set_P5(H[Nodes::p380]);
            }

    	    cout << "============== Iteration " << num_step << "==============" << endl;
    	    
    	    simulation_step();
    	    num_step++;
	    
    	    get_Q(q);

    	    is_converged = true;
    	    
    	    if (fabs(q[0][0]-q_old[0][0]) > solv_params.precision)
                is_converged = false;
            	
    	    for (int i=0; i<Elements::qtotal; i++)
		for (int j=0; j<N[Elements::qtotal]; j++)
		    q_old[i][j] = q[i][j];
	}


	//
	// Simulation part 3 - raise of P
	//
	H_new = -36.7;
	
	is_converged = false;
	while ( !is_converged ) {
	    // P
            if ( H[Nodes::p380] > H_new ) {
                H[Nodes::p380] = max (H_new, H[Nodes::p380] + (-1)*(solv_params.nr_num_steps*solv_params.time_step));
                set_P5(H[Nodes::p380]);
            }

	
    	    cout << "============== Iteration " << num_step << "==============" << endl;
    	    
    	    simulation_step();
    	    num_step++;
	    
    	    get_Q(q);

    	    is_converged = true;
    	    
    	    if (fabs(q[0][0]-q_old[0][0]) > solv_params.precision)
                is_converged = false;
            	
    	    for (int i=0; i<Elements::qtotal; i++)
		for (int j=0; j<N[Elements::qtotal]; j++)
		    q_old[i][j] = q[i][j];
	}



	// Stopping the worker-ms
	add_proxy_value<int>(Ports_Q::command_flow, Commands_Q::stop);
        proxy_flush_collective_replicate(Ports_Q::command_flow);
        proxy_clear(Ports_Q::command_flow);

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

    //
    // parameters of Q-elements
    //
    
    float q_init = 0;
    float dX = 50;
    
    float S[Elements::qtotal], R[Elements::qtotal], L[Elements::qtotal];
    
	// s340
    L[Elements::q340] = 250;
    S[Elements::q340] = 4.5;
    R[Elements::q340] = 0.03;
	// s582
    L[Elements::q582] = 450;
    S[Elements::q582] = 8.5;
    R[Elements::q582] = 0.054;
	// s541
    L[Elements::q541] = 220;
    S[Elements::q541] = 9.4;
    R[Elements::q541] = 0.00885;
	// s487
    L[Elements::q487] = 100;
    S[Elements::q487] = 11.0;
    R[Elements::q487] = 0.00195;
	// s486
    L[Elements::q486] = 390;
    S[Elements::q486] = 16.0;
    R[Elements::q486] = 0.00291;
	// s380
    L[Elements::q380] = 750;
    S[Elements::q380] = 10.0;
    R[Elements::q380] = 0.054;

    
    //
    // parameters of P-elements
    //
    
    float SP[Nodes::ptotal], H[Nodes::ptotal];
    bool is_bound[Nodes::ptotal];
    
    	// s380
    H[Nodes::p380] = 0;//-36.7;
    is_bound[Nodes::p380] = 1;
	// s541 & s486
    H[Nodes::p541] = 0;
    is_bound[Nodes::p541] = 1;
    	// s486_487
    H[Nodes::p486_487] = 0;
    is_bound[Nodes::p486_487] = 0;
    SP[Nodes::p486_487] = max({S[Elements::q486],S[Elements::q487]});
    	// s487_541_582
    H[Nodes::p487_541_582] = 0;
    is_bound[Nodes::p487_541_582] = 0;
    SP[Nodes::p487_541_582] = max({S[Elements::q487],S[Elements::q541],S[Elements::q582]});
    	// s582_340
    H[Nodes::p582_340] = 0;
    is_bound[Nodes::p582_340] = 0;
    SP[Nodes::p582_340] = max({S[Elements::q582],S[Elements::q340]});
    	// s340_380
    H[Nodes::p340_380] = 0;
    is_bound[Nodes::p340_380] = 0;
    SP[Nodes::p340_380] = max({S[Elements::q340],S[Elements::q380]});
    
    //            
    // numeric parameters
    //
    Solver_Params solv_params = { 0.15        /*time step in s.*/,
                                  0.0001      /*precision*/,
                                  30          /*nr. of numeric steps in sim block*/  };
    //
    // Deployment options
    //
    MpiProcessMap mpi_map;
    
    mpi_map.add({"pls11",               	  	 0});
    
    mpi_map.add({"Q"+to_string(Elements::q340),    	 1});
    
    mpi_map.add({"Q"+to_string(Elements::q340)+"_q0",    2});
    mpi_map.add({"Q"+to_string(Elements::q340)+"_q1",    3});
    mpi_map.add({"Q"+to_string(Elements::q340)+"_q2",    4});
    mpi_map.add({"Q"+to_string(Elements::q340)+"_q3",    5});
    mpi_map.add({"Q"+to_string(Elements::q340)+"_q4",    6});
    mpi_map.add({"Q"+to_string(Elements::q340)+"_p0",    7});
    mpi_map.add({"Q"+to_string(Elements::q340)+"_p1",    8});
    mpi_map.add({"Q"+to_string(Elements::q340)+"_p2",    9});
    mpi_map.add({"Q"+to_string(Elements::q340)+"_p3",    10});
    
    mpi_map.add({"Q"+to_string(Elements::q582),    	 11});
    
    mpi_map.add({"Q"+to_string(Elements::q582)+"_q0",    12});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_q1",    13});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_q2",    14});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_q3",    15});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_q4",    16});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_q5",    17});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_q6",    18});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_q7",    19});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_q8",    20});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_p0",    21});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_p1",    22});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_p2",    23});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_p3",    24});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_p4",    25});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_p5",    26});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_p6",    27});
    mpi_map.add({"Q"+to_string(Elements::q582)+"_p7",    28});
    
    mpi_map.add({"Q"+to_string(Elements::q541),    	 29});
    
    mpi_map.add({"Q"+to_string(Elements::q541)+"_q0",    30});
    mpi_map.add({"Q"+to_string(Elements::q541)+"_q1",    31});
    mpi_map.add({"Q"+to_string(Elements::q541)+"_q2",    32});
    mpi_map.add({"Q"+to_string(Elements::q541)+"_q3",    33});
    mpi_map.add({"Q"+to_string(Elements::q541)+"_p0",    34});
    mpi_map.add({"Q"+to_string(Elements::q541)+"_p1",    35});
    mpi_map.add({"Q"+to_string(Elements::q541)+"_p2",    36});
    
    mpi_map.add({"Q"+to_string(Elements::q487),    	 37});
    
    mpi_map.add({"Q"+to_string(Elements::q487)+"_q0",    38});
    mpi_map.add({"Q"+to_string(Elements::q487)+"_q1",    39});
    mpi_map.add({"Q"+to_string(Elements::q487)+"_p0",    40});
    
    mpi_map.add({"Q"+to_string(Elements::q486),    	 41});
    
    mpi_map.add({"Q"+to_string(Elements::q486)+"_q0",    42});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_q1",    43});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_q2",    44});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_q3",    45});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_q4",    46});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_q5",    47});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_q6",    48});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_q7",    49});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_p0",    50});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_p1",    51});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_p2",    52});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_p3",    53});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_p4",    54});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_p5",    55});
    mpi_map.add({"Q"+to_string(Elements::q486)+"_p6",    56});

    
    mpi_map.add({"Q"+to_string(Elements::q380),    	 57});
    
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q0",    58});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q1",    59});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q2",    60});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q3",    61});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q4",    62});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q5",    63});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q6",    64});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q7",    65});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q8",    66});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q9",    67});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q10",   68});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q11",   69});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q12",   70});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q13",   71});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_q14",   72});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p0",    73});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p1",    74});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p2",    75});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p3",    75});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p4",    76});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p5",    77});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p6",    78});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p7",    79});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p8",    80});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p9",    81});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p10",   82});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p11",   83});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p12",   84});
    mpi_map.add({"Q"+to_string(Elements::q380)+"_p13",   85});
    
    mpi_map.add({"P0",					 86});
    mpi_map.add({"P1",					 87});
    mpi_map.add({"P2",					 88});
    mpi_map.add({"P3",					 89});
    mpi_map.add({"P4",					 90});
    mpi_map.add({"P5",					 91});
    
    
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
    mon_opts->experiment_id             = experiment_id;
    mon_opts->flag_is_realtime          = 0;
    mon_opts->flag_output_csv           = 1;
    mon_opts->flag_output_es            = 0;
    mon_opts->flag_output_es_via_files  = 0;
    mon_opts->buf_size                  = 100;

    if ((mpi_rank == 0)&&((mon_opts->flag_output_es)||(mon_opts->flag_output_es_via_files))) {
        Monitoring mon(mon_opts);
        mon.mapping();
    }

    Section_model* model = new Section_model(
	    0, "pls11", communicator,
	    "Section1" /*section*/, "Network1" /*network*/,
            mon_opts,
	    S, R, L, q_init,
	    SP, is_bound, H,
            dX,
            solv_params);

    MpiDeploymentPool deployment_pool(mpi_map);
    deployment_pool.add_ms(model);
    deployment_pool.deploy_all();
    deployment_pool.join_all();
    
    MPI_Finalize();
}