/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef MPI_COMMUNICATOR_H
#define MPI_COMMUNICATOR_H

#include "communicator.h"
#include "mpi.h"

using namespace std;

const int nr_ports = 50; // maxim nr. of ports per MPI process

struct MpiProcessMapEntry {
    string id_str;
    int mpi_rank;
};

class MpiProcessMap {
public:
    vector<MpiProcessMapEntry> mpi_map;

    void add(MpiProcessMapEntry new_map) {
	mpi_map.push_back(new_map);
    }

    int get_mpi_rank(string ms_id_str) {
	for (int i=0; i<mpi_map.size(); i++)
	    if (mpi_map[i].id_str == ms_id_str)
		return mpi_map[i].mpi_rank;
	return -1;		
    }
    
    int get_tag_by_receiver(string ms_id_str, int port) {
	int tag = 0;
	
	for (int i=0; i<mpi_map.size(); i++) 
	    if (mpi_map[i].id_str == ms_id_str)
		break;
	    else
		tag += nr_ports;
	
	tag += port;
	return tag;
    }
    
    int get_size() {
	return mpi_map.size();
    }
    
    void print() {
	for (int i=0; i<mpi_map.size(); i++)
	    cout << mpi_map[i].id_str << "\t" <<mpi_map[i].mpi_rank << "\t" << nr_ports << endl;
    }
};


class MpiCommunicator: public Communicator {
public:
    MpiCommunicator(MpiProcessMap mpi_map_) 
	: Communicator() 
    {
	mpi_map = mpi_map_;
    };
    
    vector<LocalBuffer*> all_lb;
    MpiProcessMap mpi_map;
    
    void flush_p2p_replicate(int ms_id, string ms_id_str, int port, string dst);
    void flush_collective_replicate(int ms_id, string ms_id_str, int port);
    void flush_collective_spread(int ms_id, string ms_id_str, int port);
    void flush_collective_gather(int ms_id, string ms_id_str, int port);
    
    void sync(int rcv_id, string rcv_id_str, int port);

    void add_lb(LocalBuffer* lb) {
        all_lb.push_back(lb);
        //cout << "MpiCommunicator: New LB added" << endl;
    };
    
    void send_int_value(int value, int dest_mpi_rank, int dest_port) {
	MPI_Send(
	    &value,
	    1,
	    MPI_INT,
	    dest_mpi_rank,
	    dest_port,
	    MPI_COMM_WORLD
	);
    }
    
    void send_float_value(float value, int dest_mpi_rank, int dest_port) {
	MPI_Send(
	    &value,
	    1,
	    MPI_FLOAT,
	    dest_mpi_rank,
	    dest_port,
	    MPI_COMM_WORLD
	);
    }
    
    int recv_int_value(int src_mpi_rank, int src_port) {
	int value;
	MPI_Status status;
    
	MPI_Recv(
	    &value,
	    1,
	    MPI_INT,
	    src_mpi_rank,
	    src_port,
	    MPI_COMM_WORLD,
	    &status
	);
	
	return value;
    }

    float recv_float_value(int src_mpi_rank, int src_port) {
	float value;
	MPI_Status status;

	MPI_Recv(
	    &value,
	    1,
	    MPI_FLOAT,
	    src_mpi_rank,
	    src_port,
	    MPI_COMM_WORLD,
	    &status
	);
	
	return value;
    }
    
    LocalBuffer* get_dest_buffer(CommLink* destination);
    LocalBuffer* get_src_buffer(CommLink* destination);
    
    LocalBuffer* get_lb_byid(int ms_id, int port);
    LocalBuffer* get_lb_byid(string ms_id, int port);
    
    void get_lb_info();
    
};

#endif // MPI_COMMUNICATOR_H