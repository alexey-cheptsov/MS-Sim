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

#include "A1_2.h"
#include "../../mpi_communicator.h"
#include "../../mpi_deployment.h"


using namespace std;
using namespace A1_2;


int main (int argc, char* argv[]) {
    int mpi_rank, mpi_size, provided;
    
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    // parameters of Q-elements
    float S = 7.0;   // square cut
    float L = 113;   // length
            
    // numeric parameters
    Solver_Params solv_params = { 0.45        /*time step in s.*/,
                                  0.0001      /*precision*/,
                                  30          /*nr. of numeric steps in sim block*/  };
    
    // Deployment options
    MpiProcessMap mpi_map;
    
    mpi_map.add({"p0", 0});
    
    MpiCommunicator* communicator = new MpiCommunicator(mpi_map);
    
    fstream* output = new fstream();
    output->open("output/p0.csv", ios::out);

    Monitoring_opts* mon_opts = new Monitoring_opts();
    mon_opts->experiment_id     = "00000000000";
    mon_opts->flag_output_file  = 1;


    p p0(0, "p0", communicator,
		"p0" /*name*/, "OS" /*element*/, "Section1" /*section*/, "Network1" /*network*/,
		 mon_opts,
		 S /*S*/, L /*dx*/, false /*is_boundary*/, solv_params);
		 
    p0.add_buffer(new LocalIntBuffer  (p0.id /*ms_id*/, p0.id_str, 0 /*port*/, p0.communicator));
    
    p0.add_buffer_value(0, Commands_p::id);
    p0.add_buffer_value(0, Commands_p::stop);
    
    MpiDeploymentPool deployment_pool(mpi_map);
    deployment_pool.add_ms(&p0);
    deployment_pool.deploy_all();
    
    
    deployment_pool.join_all();
    
    MPI_Finalize();
}