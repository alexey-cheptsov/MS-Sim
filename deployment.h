/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef DEPLOYMENT_H
#define DEPLOYMENT_H

#include <sstream>
#include <iostream>
#include <vector>
#include <memory> // shared_ptr
#include <mutex>  // std::mutex

#include "microservice.h"
#include "communicator.h"

using namespace std;

class DeploymentEntry {
public:
    DeploymentEntry() {};
    
    string ms_id;
    string host;
};

class DeploymentTable {
public:
    DeploymentTable() {};
    
    vector<DeploymentEntry> entries;    
};

class DeploymentPool {
public:
    
    DeploymentPool() {}; 
    
    mutex lock;
    vector<Microservice*> ms_pool;
    
    int get_size() {return ms_pool.size();};
    
    void add_ms(Microservice* ms_to_add) {
	lock.lock();
	ms_pool.push_back(ms_to_add);
	lock.unlock();
    }
    
    virtual void deploy_all() {};
    virtual void join_all() {};
};

class ThreadDeploymentPool : public DeploymentPool {
public:

    vector<thread> ms_threads;

    void deploy_all() {
	lock.lock();
	
	for (int i=0; i<ms_pool.size(); i++) {
	    Microservice* ms = ms_pool[i];
	    ms_threads.push_back(thread(&Microservice::run, ms));
	}
	
	lock.unlock();
    }
    
    void join_all() {
	lock.lock();
	for (thread &t: ms_threads)
    	    if (t.joinable())
    		t.join();
    	lock.unlock();
    
    }
};

#endif // DEPLOYMENT_H