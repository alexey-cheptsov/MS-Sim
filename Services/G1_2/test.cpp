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

#include "G1_2.h"
#include "../../mpi_communicator.h"
#include "../../mpi_deployment.h"


using namespace std;
using namespace G1_2;


int main (int argc, char* argv[]) {
    int mpi_rank, mpi_size, provided;
    
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    // parameters of Q-element
    float S = 7.0;   // square cut
    float R = 4.68;  // resistance
    float L = 113;   // length
    
    // parameters of P-elements
    float H_start = 0;
    float H_end = 242.6;
            
    // numeric parameters
    Solver_Params solv_params = { 0.45        /*time step in s.*/,
                                  0.0001      /*precision*/,
                                  30          /*nr. of numeric steps in sim block*/  };
    
    // Deployment options
    MpiProcessMap mpi_map;
    
    mpi_map.add({"qmt0", 0});
    mpi_map.add({"q0", 0});
    
    MpiCommunicator* communicator = new MpiCommunicator(mpi_map);
    
    Monitoring_opts* mon_opts = new Monitoring_opts();
    mon_opts->experiment_id     = "00000000000";
    mon_opts->flag_output_file  = 1;

    qmt q0(0, "qmt0", "q0", communicator,
		"qmt0" /*name*/, "q0" /*air_name*/, "OS" /*element*/, "Section1" /*section*/, "Network1" /*network*/,
		 mon_opts,
		 S /*S*/, R/L /*r*/, L /*l*/, solv_params);
		 
    q0.add_buffer(new LocalIntBuffer  (q0.id /*ms_id*/, q0.id_str, 0 /*port*/, q0.communicator));
    
    q0.add_buffer_value(0, Commands_qmt::id);
    q0.add_buffer_value(0, Commands_qmt::stop);
    
    MpiDeploymentPool deployment_pool(mpi_map);
    deployment_pool.add_ms(&q0);
    deployment_pool.deploy_all();
    
        
    deployment_pool.join_all();
    
    MPI_Finalize();
}