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

#include "../../Services/A2_1/A2_1.h"
#include "../../Services/A1_2/A1_2.h"
#include "../../mpi_communicator.h"
#include "../../mpi_deployment.h"


using namespace std;
using namespace A2_1;
using namespace A1_2;

class Element_model: public Microservice {
public:

    float S;	// model params
    float R;
    float L;
    float dX;
    int n;
        
    float H_start; // pressures
    float H_end;
        
    Solver_Params solv_params; // numeric params
    
    int proxy_disp_p = 9; // proxy displacement of p- nodes
                          // (contiguous numbering of proxy-ports)

    Q** QQ; // underlying services
    p** PP;
    
    DeploymentPool* deployment_pool;
                        
    Element_model(int id_, string id_str_, 
	float S_, float R_, float L_, float dX_,
	float H_start_, float H_end_, 
	Solver_Params& solv_params_,
	MpiCommunicator* communicator_,
	Monitoring_opts* mon_opts_) 
	    : Microservice(id_, id_str_, communicator_)
    {
	S=S_;
	R=R_;
	L=L_;
	dX=dX_;
        H_start = H_start_;
        H_end = H_end_;
        solv_params = solv_params_;
        
        n = L_/dX_;
        
        // Setup of communication map
        set_communications();
        //communicator->print_comm_links();
            
        // Initialization of underlying microservices
	QQ = new Q*[1];
        PP = new p*[2];
        
	// Initialization of microservices
	QQ[0] = new Q(1/*id*/, "Q0", communicator_,
		    "OS" /*element*/, "Section1" /*section*/, "Network1" /*network*/,
		    mon_opts_,
		    S /*S*/, R /*R*/, L /*L*/, dX, solv_params);
		
        PP[0] = new p(2/*id*/, "P0", communicator, 
			 "P0" /*name*/, "Section1" /*section*/, "Network1" /*network*/,
			 mon_opts_,
			 S /*S*/, dX /*dX*/, true /*is_bound*/, solv_params);
	PP[1] = new p(3/*id*/, "P1", communicator, 
			 "P1" /*name*/, "Section1" /*section*/, "Network1" /*network*/,
			 mon_opts_,
			 S /*S*/, dX /*dX*/, true /*is_bound*/, solv_params);
	
	// Init underlying ms
        init_proxies();
        init_buffers();

        // Deployment pool initialization
        deployment_pool = new MpiDeploymentPool(communicator_->mpi_map);

        // Spawning worker threads for underlying ms
        deployment_pool->add_ms(QQ[0]);
        deployment_pool->add_ms(PP[0]);
        deployment_pool->add_ms(PP[1]);

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
	for (int i=0; i<1; i++) {
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

        // Command Flow Master->Q
	for (int i=0; i<1; i++) 
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_Q::command_flow  + proxy_disp /*port*/, 
						 "Q" + to_string(i) /*rcv_id*/, Ports_Q::command_flow));
    
        // Command Flow Master->P
	for (int i=0; i<2; i++) 
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, proxy_disp_p + Ports_p::command_flow  + proxy_disp /*port*/,
						 "P" + to_string(i) /*rcv_id*/, Ports_p::command_flow));

        // Data Flow Master->Q
	for (int i=0; i<1; i++) {
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_Q::set_Q  + proxy_disp /*port*/, 
						 "Q" + to_string(i) /*rcv_id*/, Ports_Q::set_Q,
						 n /*size*/));
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_Q::set_P  + proxy_disp /*port*/, 
						 "Q" + to_string(i) /*rcv_id*/, Ports_Q::set_P,
						 n-1 /*size*/));
	}

        // Data Flow Master->P
	for (int i=0; i<2; i++)
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, proxy_disp_p + Ports_p::set_p + proxy_disp  /*port*/, 
						 "P" + to_string(i) /*rcv_id*/, Ports_p::set_p));

	// Data Flow Q->Master
        for (int i=0; i<1; i++) {
	    communicator->add_comm_link(new CommLink("Q" + to_string(i) /*snd_id*/, Ports_Q::get_Q,
						 master /*rcv_id*/, Ports_Q::get_Q + proxy_disp  /*port*/,
						 n /*size*/));
	    communicator->add_comm_link(new CommLink("Q" + to_string(i) /*snd_id*/, Ports_Q::get_P,
						 master /*rcv_id*/, Ports_Q::get_P + proxy_disp  /*port*/,
						 n-1 /*size*/));
	}
						 
	// Data Flow P->Master
        for (int i=0; i<2; i++) 
	    communicator->add_comm_link(new CommLink("P" + to_string(i) /*rcv_id*/, Ports_p::get_p, 
						 master /*snd_id*/, proxy_disp_p + Ports_p::get_p + proxy_disp  /*port*/));
    

        // Data Flow Q->P
        for (int i=0; i<1; i++) { 
	    communicator->add_comm_link(new CommLink("Q"+to_string(i)+"_q0"/*snd_id*/, Ports_q::num_get_q, 
						 "P"+to_string(i) /*rcv_id*/,	Ports_p::num_set_qout));
						 
	    communicator->add_comm_link(new CommLink("Q"+to_string(i)+"_q"+to_string(n-1)/*snd_id*/, Ports_q::num_get_q, 
						 "P"+to_string(i+1) /*rcv_id*/, Ports_p::num_set_qin));
        }
    
	// Data Flow P->Q
	for (int i=0; i<1; i++) { 
	    communicator->add_comm_link(new CommLink("P"+to_string(i)/*snd_id*/, Ports_p::num_get_p, 
						 "Q"+to_string(i)+"_q0" /*rcv_id*/, Ports_q::num_set_pstart));
						 
	    communicator->add_comm_link(new CommLink("P"+to_string(i+1)/*snd_id*/, Ports_p::num_get_p, 
						 "Q"+to_string(i)+"_q"+to_string(n-1)/*rcv_id*/, Ports_q::num_set_pend));
        }
        
    }                
                
    // sets value of all q-approx_elements
    void set_Q(float* values /*[n]*/) {
	// preparation of buffer
	proxy_clear(Ports_Q::set_Q);
	
	for (int i=0; i<n; i++) {
	    add_proxy_value<float>(Ports_Q::set_Q, values[i]);
	}
	proxy_flush_collective_spread(Ports_Q::set_Q);
	proxy_clear(Ports_Q::set_Q);
	
	add_proxy_value<int>(Ports_Q::command_flow, Commands_Q::set_Q);
	proxy_flush_collective_replicate(Ports_Q::command_flow);
	proxy_clear(Ports_Q::command_flow);
    }
    
    // sets p of all p-approx_element
    void set_P(float* values  /*[n-1]*/) {
	proxy_clear(Ports_Q::set_P);

	for (int i=0; i<n-1; i++)
	    add_proxy_value<float>(Ports_Q::set_P, values[i]);
	proxy_flush_collective_spread(Ports_Q::set_P);
	proxy_clear(Ports_Q::set_P);
	
	add_proxy_value<int>(Ports_Q::command_flow, Commands_Q::set_P);
	proxy_flush_collective_replicate(Ports_Q::command_flow);
	proxy_clear(Ports_Q::command_flow);
    }

    // sets p of all P_element
    void set_p(float* values  /*[2]*/) {
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

    // Gets q of q-approx_elements
    void get_Q(float* values  /*[n]*/) {
	proxy_clear(Ports_Q::get_Q);
    
	add_proxy_value<int>(Ports_Q::command_flow, Commands_Q::get_Q);
	proxy_flush_collective_replicate(Ports_Q::command_flow);
	proxy_clear(Ports_Q::command_flow);	
	
	proxy_sync(Ports_Q::get_Q);
	for (int i=0; i<n; i++)
	    values[i] = get_proxy_value<float>(Ports_Q::get_Q, i);
	proxy_clear(Ports_Q::get_Q);
    }

    // Gets p of q-approx_elements
    void get_P(float* values  /*[n-1]*/) {
	proxy_clear(Ports_Q::get_P);
    
	add_proxy_value<int>(Ports_Q::command_flow, Commands_Q::get_P);
	proxy_flush_collective_replicate(Ports_Q::command_flow);
	proxy_clear(Ports_Q::command_flow);	
	
	proxy_sync(Ports_Q::get_P);
	for (int i=0; i<n-1; i++)
	    values[i] = get_proxy_value<float>(Ports_Q::get_P, i);
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

        
    void run() {

	float q[n];
	float p[n+1];
	float P[2];

        // Setting initial values of P and Q
        P[0] = H_end;
        for (int i=1; i<2; i++)
	    P[i] = 0;
	
	for (int i=0; i<n; i++)
	    q[i] = 0;
	    
	for (int i=0; i<n-1; i++)
	    p[i] = 0;

	// Setting initial values	
        set_P(p);
        set_Q(q);        
        set_p(P);
        
	bool is_converged = false;
	int num_step = 0;

	float q_old[n];
        for (int i=0; i<n; i++)
	    q_old[i]=0;

	init_time();

	// Simulation
//	for (int i=0; i<1; i++) {
	while ( !is_converged ) {
    	    cout << "============== Iteration " << num_step << "==============" << endl;
    	    
    	    simulation_step();
    	    num_step++;
	    
    	    get_Q(q);

	    float cur_mod_time = solv_params.nr_num_steps * num_step * solv_params.time_step;

    	    is_converged = true;
   	    for (int i=0; i<n; i++) {
        	if (fabs(q[i]-q_old[i]) > solv_params.precision)
            	    is_converged = false;
            	
        	q_old[i] = q[i];
    	    }
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

    int   n = 10;         // nr. of q- approx.elements
        
    // parameters of Q-elements
    float S = 7.0;   // square cut
    float R = 4.68;  // resistance
    float L = 1130;  // length
    
    float dX = 113;
            
    // parameters of P-elements
    float H_start = 0;
    float H_end = 242.6;
            
    // numeric parameters
    Solver_Params solv_params = { 0.45        /*time step in s.*/,
                                  0.0001      /*precision*/,
                                  30          /*nr. of numeric steps in sim block*/  };
    
    // Deployment options
    MpiProcessMap mpi_map;
    
    mpi_map.add({"A2_1",  0});
    mpi_map.add({"Q0",    1});
    mpi_map.add({"P0",    2});
    mpi_map.add({"P1",    3});    
    mpi_map.add({"Q0_q0", 4});
    mpi_map.add({"Q0_q1", 5});
    mpi_map.add({"Q0_q2", 6});
    mpi_map.add({"Q0_q3", 7});
    mpi_map.add({"Q0_q4", 8});
    mpi_map.add({"Q0_q5", 9});
    mpi_map.add({"Q0_q6", 10});
    mpi_map.add({"Q0_q7", 11});
    mpi_map.add({"Q0_q8", 12});
    mpi_map.add({"Q0_q9", 13});
    mpi_map.add({"Q0_p0", 14});
    mpi_map.add({"Q0_p1", 15});
    mpi_map.add({"Q0_p2", 16});
    mpi_map.add({"Q0_p3", 17});
    mpi_map.add({"Q0_p4", 18});
    mpi_map.add({"Q0_p5", 19});
    mpi_map.add({"Q0_p6", 20});
    mpi_map.add({"Q0_p7", 21});
    mpi_map.add({"Q0_p8", 22});
    
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
    mon_opts->buf_size 		= 10;

    Element_model* model_A2_1 = new Element_model(
	    0, "A2_1",
            S, R, L, dX,
            H_start, H_end,
            solv_params,
            communicator,
            mon_opts);

    MpiDeploymentPool deployment_pool(mpi_map);
    deployment_pool.add_ms(model_A2_1);
    deployment_pool.deploy_all();
    deployment_pool.join_all();
    
    MPI_Finalize();
}