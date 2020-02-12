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

#include "../../Services/A1_1/A1_1.h"
#include "../../Services/A1_2/A1_2.h"
#include "../../mpi_communicator.h"
#include "../../mpi_deployment.h"


using namespace std;
using namespace A1_1;
using namespace A1_2;

class Element_model: public Microservice {
public:

    int	  n;	// nr. of q- approx. elements
    float S;	// model params
    float R;
    float L;
    float dX;
        
    float H_start; // pressures
    float H_end;
        
    Solver_Params solv_params; // numeric params
    
    int proxy_disp_p = 10; // proxy displacement of p- nodes
                           // (contiguous numbering of proxy-ports)

    q** Q; // underlying services
    p** P;
    
    DeploymentPool* deployment_pool;
                        
    Element_model(int id_, string id_str_, 
	int n_, float S_, float R_, float L_, float dX_,
	float H_start_, float H_end_, 
	Solver_Params& solv_params_,
	MpiCommunicator* communicator_,
	Monitoring_opts* mon_opts_) 
	    : Microservice(id_, id_str_, communicator_)
    {
	n=n_;
	S=S_;
	R=R_;
	L=L_;
	dX=dX_;
        H_start = H_start_;
        H_end = H_end_;
        solv_params = solv_params_;
        
        // ???
        //communicator = communicator_;
        
        // Setup of communication map
        set_communications();
        //communicator->print_comm_links();
            
        // Initialization of underlying microservices
	Q = new q*[n];
        P = new p*[n+1];
        
	// Initialization of microservices
        for (int i=0; i<n; i++) {
	    Q[i] = new q(i+1/*id*/, "q" + to_string(i), communicator_, 
			 "q" + to_string(i) /*name*/, "OS" /*element*/, "Section1" /*section*/, "Network1" /*network*/,
			 mon_opts_,
			 S /*S*/, R/L /*r*/, L/n /*l*/, solv_params);
	}

        for (int i=0; i<n+1; i++){
    	    bool is_bound = false;
    	    if ((i==0) || (i==n)) 
    		is_bound = true;
    		
	    P[i] = new p(n+(i+1)/*id*/, "p" + to_string(i), communicator_, 
			 "p" + to_string(i) /*name*/, "OS" /*element*/, "Section1" /*section*/, "Network1" /*network*/,
			 mon_opts_,
			 S /*S*/, L/n /*dX*/, is_bound, solv_params);
	}

	// Init underlying ms
        init_proxies();
        init_buffers();

        // Deployment pool initialization
        deployment_pool = new MpiDeploymentPool(communicator_->mpi_map);

        // Spawning worker threads for underlying ms
        for (int i=0; i<n; i++) // Q
            deployment_pool->add_ms(Q[i]);
        for (int i=0; i<n+1; i++) // P
            deployment_pool->add_ms(P[i]);

        deployment_pool->deploy_all();
        deployment_pool->join_all();
    };
    
    void init_proxies() {
	// 
        // Initialization of communication proxies
	//
        add_proxy(new LocalIntBuffer  (id, id_str,  Ports_q::command_flow,   communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_q::set_q,          communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_q::get_q,          communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_q::set_qs,         communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_q::get_qs,         communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_q::set_r_reg,      communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_q::get_r_reg,      communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_q::num_set_pstart, communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_q::num_set_pend,   communicator));
        add_proxy(new LocalFloatBuffer(id, id_str,  Ports_q::num_get_q,      communicator));

    
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
	for (int i=0; i<n; i++) {
    	    Q[i]->add_buffer(new LocalIntBuffer  (Q[i]->id /*ms_id*/, Q[i]->id_str, 0 /*port*/, Q[i]->communicator));
            Q[i]->add_buffer(new LocalFloatBuffer(Q[i]->id /*ms_id*/, Q[i]->id_str, 1 /*port*/, Q[i]->communicator));
	    Q[i]->add_buffer(new LocalFloatBuffer(Q[i]->id /*ms_id*/, Q[i]->id_str, 2 /*port*/, Q[i]->communicator));
    	    Q[i]->add_buffer(new LocalFloatBuffer(Q[i]->id /*ms_id*/, Q[i]->id_str, 3 /*port*/, Q[i]->communicator));
            Q[i]->add_buffer(new LocalFloatBuffer(Q[i]->id /*ms_id*/, Q[i]->id_str, 4 /*port*/, Q[i]->communicator));
	    Q[i]->add_buffer(new LocalFloatBuffer(Q[i]->id /*ms_id*/, Q[i]->id_str, 5 /*port*/, Q[i]->communicator));
	    Q[i]->add_buffer(new LocalFloatBuffer(Q[i]->id /*ms_id*/, Q[i]->id_str, 6 /*port*/, Q[i]->communicator));
	    Q[i]->add_buffer(new LocalFloatBuffer(Q[i]->id /*ms_id*/, Q[i]->id_str, 7 /*port*/, Q[i]->communicator));
	    Q[i]->add_buffer(new LocalFloatBuffer(Q[i]->id /*ms_id*/, Q[i]->id_str, 8 /*port*/, Q[i]->communicator));
	    Q[i]->add_buffer(new LocalFloatBuffer(Q[i]->id /*ms_id*/, Q[i]->id_str, 9 /*port*/, Q[i]->communicator));
        }

        for (int i=0; i<n+1; i++) {
	    P[i]->add_buffer(new LocalIntBuffer  (P[i]->id /*ms_id*/, P[i]->id_str, 0 /*port*/, P[i]->communicator));
    	    P[i]->add_buffer(new LocalFloatBuffer(P[i]->id /*ms_id*/, P[i]->id_str, 1 /*port*/, P[i]->communicator));
            P[i]->add_buffer(new LocalFloatBuffer(P[i]->id /*ms_id*/, P[i]->id_str, 2 /*port*/, P[i]->communicator));
	    P[i]->add_buffer(new LocalFloatBuffer(P[i]->id /*ms_id*/, P[i]->id_str, 3 /*port*/, P[i]->communicator));
    	    P[i]->add_buffer(new LocalFloatBuffer(P[i]->id /*ms_id*/, P[i]->id_str, 4 /*port*/, P[i]->communicator));
    	    P[i]->add_buffer(new LocalFloatBuffer(P[i]->id /*ms_id*/, P[i]->id_str, 5 /*port*/, P[i]->communicator));
        }
    }
    
    void set_communications() {
	string master = id_str;

        // Command Flow Master->Q
	for (int i=0; i<n; i++) 
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_q::command_flow  + proxy_disp /*port*/, 
						 "q" + to_string(i) /*rcv_id*/, Ports_q::command_flow));
    
        // Command Flow Master->P
	for (int i=0; i<n+1; i++) 
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, proxy_disp_p + Ports_p::command_flow  + proxy_disp /*port*/, 
						 "p" + to_string(i) /*rcv_id*/, Ports_p::command_flow));

        // Data Flow Master->Q
	for (int i=0; i<n; i++) 
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, Ports_q::set_q  + proxy_disp /*port*/, 
						 "q" + to_string(i) /*rcv_id*/, Ports_q::set_q));

        // Data Flow Master->P
	for (int i=0; i<n+1; i++)
	    communicator->add_comm_link(new CommLink(master /*snd_id*/, proxy_disp_p + Ports_p::set_p + proxy_disp  /*port*/, 
						 "p" + to_string(i) /*rcv_id*/, Ports_p::set_p));

	// Data Flow Q->Master
        for (int i=0; i<n; i++) 
	    communicator->add_comm_link(new CommLink("q" + to_string(i) /*snd_id*/, Ports_q::get_q,
						 master /*rcv_id*/, Ports_q::get_q + proxy_disp  /*port*/));
						 
	// Data Flow P->Master
        for (int i=0; i<n+1; i++) 
	    communicator->add_comm_link(new CommLink("p" + to_string(i) /*rcv_id*/, Ports_p::get_p, 
						 master /*snd_id*/, proxy_disp_p + Ports_p::get_p + proxy_disp  /*port*/));
    

        // Data Flow Q->P
        for (int i=0; i<n; i++) { 
	    communicator->add_comm_link(new CommLink("q"+to_string(i)/*snd_id*/, 	Ports_q::num_get_q, 
						 "p"+to_string(i) /*rcv_id*/,	Ports_p::num_set_qout));
						 
	    communicator->add_comm_link(new CommLink("q"+to_string(i)/*snd_id*/, 	Ports_q::num_get_q, 
						 "p"+to_string(i+1) /*rcv_id*/, Ports_p::num_set_qin));
        }
    
	// Data Flow P->Q
	for (int i=1; i<n; i++) { 
	    communicator->add_comm_link(new CommLink("p"+to_string(i)/*snd_id*/, 	Ports_p::num_get_p, 
						 "q"+to_string(i) /*rcv_id*/, 	Ports_q::num_set_pstart));
						 
	    communicator->add_comm_link(new CommLink("p"+to_string(i)/*snd_id*/, 	Ports_p::num_get_p, 
						 "q"+to_string(i-1)/*rcv_id*/, 	Ports_q::num_set_pend));
        }
        
	communicator->add_comm_link(new CommLink("p0"/*snd_id*/, 		Ports_p::num_get_p, 
					     "q0"/*rcv_id*/, 			Ports_q::num_set_pstart));
	communicator->add_comm_link(new CommLink("p"+to_string(n)/*snd_id*/, 	Ports_p::num_get_p, 
					     "q"+to_string(n-1)/*rcv_id*/, 	Ports_q::num_set_pend));
    }                
                
    // sets value of all q-approx_elements
    void set_Q(float* values) {
	// preparation of buffer
	proxy_clear(Ports_q::set_q);
	
	for (int i=0; i<n; i++) {
	    add_proxy_value<float>(Ports_q::set_q, values[i]);
	}
	proxy_flush_collective_spread(Ports_q::set_q);
	proxy_clear(Ports_q::set_q);
	
	add_proxy_value<int>(Ports_q::command_flow, Commands_q::set_q);
	proxy_flush_collective_replicate(Ports_q::command_flow);
	proxy_clear(Ports_q::command_flow);
    }
    
    // sets p of all p-approx_element
    void set_P(float* values) {
	proxy_clear(Ports_p::set_p);

	for (int i=0; i<n+1; i++)
	    add_proxy_value<float>(proxy_disp_p + Ports_p::set_p, values[i]);
	proxy_flush_collective_spread(proxy_disp_p + Ports_p::set_p);
	proxy_clear(proxy_disp_p + Ports_p::set_p);
	
	add_proxy_value<int>(proxy_disp_p + Ports_p::command_flow, Commands_p::set_p);
	proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);
	proxy_clear(proxy_disp_p + Ports_p::command_flow);
    }

    // gets p of a p-approx_element
    void get_P(float* values) {
	proxy_clear(proxy_disp_p + Ports_p::get_p);
    
	add_proxy_value<int>(proxy_disp_p + Ports_p::command_flow, Commands_p::get_p);
	proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);
	proxy_clear(proxy_disp_p + Ports_p::command_flow);	
	
	proxy_sync(proxy_disp_p + Ports_p::get_p);
	for (int i=0; i<n+1; i++)
	    values[i] = get_proxy_value<float>(proxy_disp_p + Ports_p::get_p, i);    
    }

    // propagates single q value to all q-approx_elements
    void get_Q(float* values) {
	proxy_clear(Ports_q::get_q);
    
	add_proxy_value<int>(Ports_q::command_flow, Commands_q::get_q);
	proxy_flush_collective_replicate(Ports_q::command_flow);
	proxy_clear(Ports_q::command_flow);	
	
	proxy_sync(Ports_q::get_q);
	for (int i=0; i<n; i++)
	    values[i] = get_proxy_value<float>(Ports_q::get_q, i);
    }

    // sets output files
    void setup_fout(fstream& foutH, fstream& foutQ, fstream& foutP, int N, float H_start) {
        foutH.open("outpu/H.csv", ios::out);
        foutH << "t;H" << endl << "0;" << H_start << endl;
     

        foutQ.open("output/Q.csv", ios::out);
        foutQ << "t";
        for (int i=0; i<N; i++)
            foutQ << ";" << "q" << i;
        foutQ << endl;

        foutP.open("output/P.csv", ios::out);
        foutP << "t";
        for (int i=0; i<N+1; i++)
	    foutP << ";" << "P" << i;
	foutP << endl;
    }

    // output of Q and P into files
    void fout(fstream& foutH, fstream& foutQ, fstream& foutP, int N,
    	float H, float* Q, float* P,
	float time) 
    {
        foutQ << time;
        for (int i=0; i<N; i++)
	    foutQ << ";" << Q[i];
	foutQ << endl;
    
	foutP << time;
	for (int i=0; i<N+1; i++)
	    foutP << ";" << P[i];
	foutP << endl;

	foutH << time << ";" << H << endl;
    }
    

    // inits time stamp for all ms
    void init_time() {
        add_proxy_value(Ports_q::command_flow, Commands_q::init_time/*value*/);
        proxy_flush_collective_replicate(Ports_q::command_flow);
        proxy_clear(Ports_q::command_flow);

        add_proxy_value(proxy_disp_p + Ports_p::command_flow, Commands_p::init_time /*value*/);
        proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);
        proxy_clear(proxy_disp_p + Ports_p::command_flow);
    }
    
    // performs 1 simulation step
    void simulation_step() {
	add_proxy_value<int>(Ports_q::command_flow, Commands_q::simulation);
	add_proxy_value<int>(proxy_disp_p + Ports_p::command_flow, Commands_p::simulation);

        proxy_flush_collective_replicate(Ports_q::command_flow);
        proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);

    	proxy_clear(Ports_q::command_flow);
    	proxy_clear(proxy_disp_p + Ports_p::command_flow);    	    
    }

    // requests the underlying ms to store data
    void ms_output() {
	add_proxy_value<int>(Ports_q::command_flow, Commands_q::save);
	add_proxy_value<int>(proxy_disp_p + Ports_p::command_flow, Commands_p::save);
	
        proxy_flush_collective_replicate(Ports_q::command_flow);
        proxy_flush_collective_replicate(proxy_disp_p + Ports_p::command_flow);
        
    	proxy_clear(Ports_q::command_flow);
    	proxy_clear(proxy_disp_p + Ports_p::command_flow);
    }

        
    void run() {

	float q[n];
	float p[n+1];

	int is_boundary[n+1];
	        
        // Setting initial values of P and Q
        p[0] = H_end;
        for (int i=1; i<n+1; i++) {
	    p[i] = 0;
	}
	
	for (int i=0; i<n; i++) {
	    q[i] = 0;
	}
	
	// Setting initial values	
        set_P(p);
        set_Q(q);        
        
        // Preparation for results output
        fstream foutH;
        fstream foutQ;
        fstream foutP;
	setup_fout(foutH, foutQ, foutP, n, H_start);

	bool is_converged = false;
	int num_step = 0;

	float q_old[n];
        for (int i=0; i<n; i++)
	    q_old[i]=0;

	init_time();

	// Simulation
//	for (int i=0; i<2; i++) {
	while ( !is_converged ) {
    	    cout << "============== Iteration " << num_step << "==============" << endl;
    	    
    	    simulation_step();
    	    num_step++;
	    
    	    get_Q(q);

            float store_interval = 0.5;
	    float cur_mod_time = solv_params.nr_num_steps * num_step * solv_params.time_step;

	    //if (cur_mod_time/store_interval - round(cur_mod_time/store_interval)==0)
    	    fout(foutH, foutQ, foutP, n,
                H_end, q, p,
                cur_mod_time);

    	    is_converged = true;
   	    for (int i=0; i<n; i++) {
        	if (fabs(q[i]-q_old[i]) > solv_params.precision)
            	    is_converged = false;
        	q_old[i] = q[i];
    	    }
	}


	// Stopping the worker-ms
        add_proxy_value<int>(Ports_q::command_flow, Commands_q::stop);
        proxy_flush_collective_replicate(Ports_q::command_flow);
        proxy_clear(Ports_q::command_flow);

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
    
    mpi_map.add({"A1", 0});
    mpi_map.add({"q0", 1});
    mpi_map.add({"q1", 2});
    mpi_map.add({"q2", 3});
    mpi_map.add({"q3", 4});
    mpi_map.add({"q4", 5});
    mpi_map.add({"q5", 6});
    mpi_map.add({"q6", 7});
    mpi_map.add({"q7", 8});
    mpi_map.add({"q8", 9});
    mpi_map.add({"q9", 10});
    mpi_map.add({"p0", 11});
    mpi_map.add({"p1", 12});
    mpi_map.add({"p2", 13});
    mpi_map.add({"p3", 14});
    mpi_map.add({"p4", 15});
    mpi_map.add({"p5", 16});
    mpi_map.add({"p6", 17});
    mpi_map.add({"p7", 18});
    mpi_map.add({"p8", 19});
    mpi_map.add({"p9", 20});
    mpi_map.add({"p10",21});  
    
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

    Element_model* model_A1 = new Element_model(
	    0, "A1",
            n, S, R, L, dX,
            H_start, H_end,
            solv_params,
            communicator,
            mon_opts);
        
    MpiDeploymentPool deployment_pool(mpi_map);
    deployment_pool.add_ms(model_A1);
    deployment_pool.deploy_all();
    deployment_pool.join_all();
    
    MPI_Finalize();
}