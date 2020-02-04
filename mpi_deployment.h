/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef MPI_DEPLOYMENT_H
#define MPI_DEPLOYMENT_H

#include <sstream>
#include <iostream>
#include <vector>
#include <memory> // shared_ptr
#include <mutex>  // std::mutex

#include "microservice.h"
#include "communicator.h"
#include "deployment.h"
#include "mpi_communicator.h"


using namespace std;

class MpiDeploymentPool : public DeploymentPool {
public:

    MpiProcessMap mpi_process_map;
    vector<thread> ms_threads;
    int mpi_rank;

    MpiDeploymentPool(MpiProcessMap& mpi_process_map_) : DeploymentPool() {
	mpi_process_map = mpi_process_map_;
        MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    }


    void deploy_all() {
	lock.lock();
	
	for (int i=0; i<ms_pool.size(); i++) {
	    Microservice* ms = ms_pool[i];
	    
	    int target_rank = mpi_process_map.get_mpi_rank(ms->id_str);
	    if (mpi_rank == target_rank)
	        ms_threads.push_back(thread(&Microservice::run, ms));
	}
	
	lock.unlock();
	
	join_all();
    }
    
    void join_all() {
	lock.lock();
	for (thread &t: ms_threads)
    	    if (t.joinable())
    		t.join();
    	lock.unlock();
    
    }
};

#endif // MPI_DEPLOYMENT_H