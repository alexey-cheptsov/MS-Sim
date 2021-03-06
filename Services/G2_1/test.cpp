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

#include "G2_1.h"
#include "../../mpi_communicator.h"
#include "../../mpi_deployment.h"


using namespace std;
using namespace G2_1;


int main (int argc, char* argv[]) {
    int mpi_rank, mpi_size, provided;
    
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    // parameters of Q-element
    float S = 7.0;   // square cut
    float R = 4.68;  // resistance
    float L = 1130;   // length
    float dX = 113;
    
    // gas parameters of qm-elements
    float A = 1460.0;
    float BRf = 1.71;
    float Qm0 = 0.0175;
            
    // parameters of P-elements
    float H_start = 0;
    float H_end = 242.6;
            
    // numeric parameters
    Solver_Params solv_params = { 0.45        /*time step in s.*/,
                                  0.0001      /*precision*/,
                                  30          /*nr. of numeric steps in sim block*/  };
    
    // Deployment options
    MpiProcessMap mpi_map;
    
    mpi_map.add({"Qm0",    0});
    mpi_map.add({"Qm0_qm0", 1});
    mpi_map.add({"Qm0_q0",  1});
    mpi_map.add({"Qm0_qm1", 2});
    mpi_map.add({"Qm0_q1",  2});
    mpi_map.add({"Qm0_qm2", 3});
    mpi_map.add({"Qm0_q2",  3});
    mpi_map.add({"Qm0_qm3", 4});
    mpi_map.add({"Qm0_q3",  4});
    mpi_map.add({"Qm0_qm4", 5});
    mpi_map.add({"Qm0_q4",  5});
    mpi_map.add({"Qm0_qm5", 6});
    mpi_map.add({"Qm0_q5",  6});
    mpi_map.add({"Qm0_qm6", 7});
    mpi_map.add({"Qm0_q6",  7});
    mpi_map.add({"Qm0_qm7", 8});
    mpi_map.add({"Qm0_q7",  8});
    mpi_map.add({"Qm0_qm8", 9});
    mpi_map.add({"Qm0_q8",  9});
    mpi_map.add({"Qm0_qm9",10});
    mpi_map.add({"Qm0_q9", 10});
    mpi_map.add({"Qm0_p0", 11});
    mpi_map.add({"Qm0_p1", 12});
    mpi_map.add({"Qm0_p2", 13});
    mpi_map.add({"Qm0_p3", 14});
    mpi_map.add({"Qm0_p4", 15});
    mpi_map.add({"Qm0_p5", 16});
    mpi_map.add({"Qm0_p6", 17});
    mpi_map.add({"Qm0_p7", 18});
    mpi_map.add({"Qm0_p8", 19});
    
    MpiCommunicator* communicator = new MpiCommunicator(mpi_map);
    
    char experiment_id_char[23];
    if (mpi_rank == 0) {
        Time_MS now;
        now.init_time();
        string experiment_id = now.time_stamp();
        cout << "Experiment_id: " << experiment_id << endl;
        strcpy(experiment_id_char, experiment_id.c_str());
    }
    MPI_Bcast(experiment_id_char, 23, MPI_CHAR, 0, MPI_COMM_WORLD);
            
    string experiment_id(experiment_id_char);

    
    Monitoring_opts* mon_opts = new Monitoring_opts();
    mon_opts->experiment_id     = experiment_id;
    mon_opts->flag_output_file  = 1;
    mon_opts->buf_size          = 10;


    Qm* Q0 = new Qm(0, "Qm0", communicator,
	"OS" /*element*/, "Section1" /*section*/, "Network1" /*network*/,
	 mon_opts,
	 S /*S*/, R /*R*/, L /*L*/, dX, 
	 A, BRf, solv_params);
		 
    Q0->add_buffer(new LocalIntBuffer(Q0->id /*ms_id*/, Q0->id_str, 0 /*port*/, Q0->communicator));
    Q0->add_buffer_value(0, Commands_Qm::id);
    Q0->add_buffer_value(0, Commands_Qm::stop);
    
    MpiDeploymentPool deployment_pool(mpi_map);
    deployment_pool.add_ms(Q0);
    deployment_pool.deploy_all();
    deployment_pool.join_all();
    
    MPI_Finalize();
}