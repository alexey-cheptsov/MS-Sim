/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

#include "buffer.h"
#include "mpi_communicator.h"

using namespace std;

void MpiCommunicator::get_lb_info() {
    stringstream out;
    out << "Nr. of lb's in MPI_Communicator: " << all_lb.size()  << endl;
    out << "Nr. of comm links in MPI_Communicator: " << comm_links.size()  << endl;
    cout << out.str();
}

LocalBuffer* MpiCommunicator::get_lb_byid(int ms_id, int port) {
        for (int i=0; i<all_lb.size(); i++)
            if ((all_lb[i]->ms_id==ms_id)&&(all_lb[i]->port==port))
                return all_lb[i];
    
        return nullptr;
}


LocalBuffer* MpiCommunicator::get_lb_byid(string ms_id_str, int port) {
        for (int i=0; i<all_lb.size(); i++)
            if ((all_lb[i]->ms_id_str==ms_id_str)&&(all_lb[i]->port==port))
                return all_lb[i];

	int mpi_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	
	cout << "Rank " << mpi_rank << ": MpiCommunicator: Cannot find buffer for " << ms_id_str << ":" << port << endl;
	exit(1);
	
        return nullptr;
}

LocalBuffer* MpiCommunicator::get_dest_buffer(CommLink* destination) {
        if (!destination->type) // int_id
            return get_lb_byid(destination->rcv_id, destination->rcv_port);
        else            // string_id
            return get_lb_byid(destination->rcv_id_str, destination->rcv_port);
}
            
LocalBuffer* MpiCommunicator::get_src_buffer(CommLink* destination) {
        if (!destination->type) // int_id
            return get_lb_byid(destination->snd_id, destination->snd_port);
        else            // string_id
            return get_lb_byid(destination->snd_id_str, destination->snd_port);
}



void MpiCommunicator::flush_p2p_replicate(int ms_id, string ms_id_str, int port, string dst) {
					
    vector<CommLink*> destinations = get_comm_links_by_sender(ms_id, ms_id_str, port);
    // TO-DO: Error handling when no destinations available (wrong comm map)
    LocalBuffer* snd_buf = get_src_buffer(destinations[0]);
    
    //int mpi_rank;
    //MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    
    for (int i=0; i<destinations.size(); i++) {
        if (destinations[i]->rcv_id_str == dst) {
        
    	    int dest_port = destinations[i]->rcv_port;
    	    string dest_ms_id_str = destinations[i]->rcv_id_str;
    	    int dest_mpi_rank = mpi_map.get_mpi_rank(dest_ms_id_str);
        
    	    LocalIntBuffer*     int_snd_buf = dynamic_cast<LocalIntBuffer*>(snd_buf);
	    LocalFloatBuffer* float_snd_buf = dynamic_cast<LocalFloatBuffer*>(snd_buf);
	    
	    for (int j=0; j<snd_buf->get_size(); j++) {
		if (int_snd_buf) {
        	    int value = int_snd_buf->get_int_value(j /*index*/);
        	    int tag = mpi_map.get_tag_by_receiver(dest_ms_id_str, dest_port);

		    send_int_value(value, dest_mpi_rank, tag);
    	    	    //cout << "Rank " << mpi_rank << ": MPI_Send: " << ms_id_str << ":" << port << " --{" << value << "}--> " << dest_ms_id_str << ":" << dest_port << " (mpi_rank = " << dest_mpi_rank << ", tag = " << tag << ")"<< endl;
    		} else if (float_snd_buf) {
	    	    float value = float_snd_buf->get_float_value(j /*index*/);
	    	    int tag = mpi_map.get_tag_by_receiver(dest_ms_id_str, dest_port);
	        
	    	    send_float_value(value, dest_mpi_rank, tag);
            	    //cout << ms_id_str << ":" << port << " --{" << value << "}--> " << dest_ms_id_str << ":" << dest_port << " (mpi_rank = " << dest_mpi_rank << ", tag = " << tag << ")"<< endl;
		}
	    }
	}
    }
}

void MpiCommunicator::flush_collective_replicate(int ms_id, string ms_id_str, int port) {
    vector<CommLink*> destinations = get_comm_links_by_sender(ms_id, ms_id_str, port);
    // TO-DO: Error handling when no destinations available (wrong comm map)
    LocalBuffer* snd_buf = get_src_buffer(destinations[0]);

    //int mpi_rank;
    //MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    
    for (int i=0; i<destinations.size(); i++) {
	int dest_port = destinations[i]->rcv_port;
	string dest_ms_id_str = destinations[i]->rcv_id_str;
        int dest_mpi_rank = mpi_map.get_mpi_rank(dest_ms_id_str);

        LocalIntBuffer*     int_snd_buf = dynamic_cast<LocalIntBuffer*>(snd_buf);
	LocalFloatBuffer* float_snd_buf = dynamic_cast<LocalFloatBuffer*>(snd_buf);
	
	for (int j=0; j<snd_buf->get_size(); j++) {
    	    if (int_snd_buf) {
    	    	int value = int_snd_buf->get_int_value(j /*index*/);
    	    	int tag = mpi_map.get_tag_by_receiver(dest_ms_id_str, dest_port);

		//if (ms_id_str == "Qm0")
		//cout << "Rank " << mpi_rank << ": Attempting MPI_Send: " << ms_id_str << ":" << port << " --{" << value << "}--> " << dest_ms_id_str << ":" << dest_port << " (mpi_rank = " << dest_mpi_rank << ", tag = " << tag << ")"<< endl;

    	    	send_int_value(value, dest_mpi_rank, tag);
    	    	
	    	//if (ms_id_str == "Qmt0")
		//cout << "Rank " << mpi_rank << ": MPI_Send [" << j <<"/" << snd_buf->get_size() << "]: " << ms_id_str << ":" << port << " --{" << value << "}--> " << dest_ms_id_str << ":" << dest_port << " (mpi_rank = " << dest_mpi_rank << ", tag = " << tag << ")"<< endl;

            } else if (float_snd_buf) {
	        float value = float_snd_buf->get_float_value(j /*index*/);
	        int tag = mpi_map.get_tag_by_receiver(dest_ms_id_str, dest_port);
	        
	        send_float_value(value, dest_mpi_rank, tag);
	        
	        //if (ms_id_str == "q0")
                //cout << ms_id_str << " (mpi rank = " << mpi_rank << ") :" << port << " --{" << value << "}--> " << dest_ms_id_str << ":" << dest_port << " (mpi_rank = " << dest_mpi_rank << ", tag = " << tag << ")"<< endl;
	    }
	}
    }
};

void MpiCommunicator::flush_collective_spread(int ms_id, string ms_id_str, int port) {
    //int mpi_rank;
    //MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    vector<CommLink*> destinations = get_comm_links_by_sender(ms_id, ms_id_str, port);
    // TO-DO: Error handling when no destinations available (wrong comm map)
    LocalBuffer* snd_buf = get_src_buffer(destinations[0]);
    int nr_entries_per_dest = snd_buf->get_size() / destinations.size();

    LocalIntBuffer*     int_snd_buf = dynamic_cast<LocalIntBuffer*>(snd_buf);
    LocalFloatBuffer* float_snd_buf = dynamic_cast<LocalFloatBuffer*>(snd_buf);
    
    for (int i=0; i<destinations.size(); i++) {
    
	string dest_ms_id_str = destinations[i]->rcv_id_str;
	int dest_mpi_rank = mpi_map.get_mpi_rank(dest_ms_id_str);
	int dest_port = destinations[i]->rcv_port;
	
	for (int j=0; j<nr_entries_per_dest; j++) {
	    int value_index_in_snd_buf = i*nr_entries_per_dest + j;
	
	    if (int_snd_buf) {
    		int value = int_snd_buf->get_int_value(value_index_in_snd_buf /*index*/);
		int tag = mpi_map.get_tag_by_receiver(dest_ms_id_str, dest_port);
    		
    		send_int_value(value, dest_mpi_rank, tag);
    		//cout << "Rank " << mpi_rank << ": MPI_Send: " << ms_id_str << ":" << port << " --{" << value << "}--> " << dest_ms_id_str << ":" << dest_port << " (mpi_rank = " << dest_mpi_rank << ", tag = " << tag << ")"<< endl;
    	    }
	    else if (float_snd_buf) {
    		float value = float_snd_buf->get_float_value(value_index_in_snd_buf /*index*/);
		int tag = mpi_map.get_tag_by_receiver(dest_ms_id_str, dest_port);

    		send_float_value(value, dest_mpi_rank, tag);
    		//cout << "Rank " << mpi_rank << ": MPI_Send: " << ms_id_str << ":" << port << " --{" << value << "}--> " << dest_ms_id_str << ":" << dest_port << " (mpi_rank = " << dest_mpi_rank << ", tag = " << tag << ")"<< endl;
	    }
	}
    }
};


void MpiCommunicator::flush_collective_gather(int ms_id, string ms_id_str, int port) {

    //int mpi_rank;
    //MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    vector<CommLink*> destinations = get_comm_links_by_sender(ms_id, ms_id_str, port);
    if (destinations.size() > 1) {
	cout << ms_id_str << ":" << port << " - Wrong use of flush_collective_gather communicator. Destinations nr. must be 1 or 0 (here - )" 
	    << destinations.size() << endl;
	exit(1);
    }
    
    if (destinations.size() == 1) {

        CommLink* destination = destinations[0];
        LocalBuffer* snd_buf = get_src_buffer(destination);    
    
	LocalIntBuffer*     int_snd_buf = dynamic_cast<LocalIntBuffer*>(snd_buf);
	LocalFloatBuffer* float_snd_buf = dynamic_cast<LocalFloatBuffer*>(snd_buf);
	
	string dest_ms_id_str = destination->rcv_id_str;
        int dest_mpi_rank = mpi_map.get_mpi_rank(dest_ms_id_str);
        int dest_port = destination->rcv_port;
	
	for (int i=0; i<snd_buf->get_size(); i++) {
	    if (int_snd_buf) {
    		int value = int_snd_buf->get_int_value(i /*index*/);
    		int tag = mpi_map.get_tag_by_receiver(dest_ms_id_str, dest_port);
    		
    		send_int_value(value, dest_mpi_rank, tag);
    		//cout << "Rank " << mpi_rank << ": MPI_Send: " << ms_id_str << ":" << port << " --{" << value << "}--> " << dest_ms_id_str << ":" << dest_port << " (mpi_rank = " << dest_mpi_rank << ", tag = " << tag << ")"<< endl;
    	    } else if (float_snd_buf) {
		float value = float_snd_buf->get_float_value(i /*index*/);
		int tag = mpi_map.get_tag_by_receiver(dest_ms_id_str, dest_port);

    		send_float_value(value, dest_mpi_rank, tag);
    		//cout << "Rank " << mpi_rank << ": MPI_Send: " << ms_id_str << ":" << port << " --{" << value << "}--> " << dest_ms_id_str << ":" << dest_port << " (mpi_rank = " << dest_mpi_rank << ", tag = " << tag << ")"<< endl;
	    }
	}
    }
};

void MpiCommunicator::sync(int rcv_id, string rcv_id_str, int port) {
    LocalBuffer* rcv_buf;
    vector<CommLink*> sources = get_comm_links_by_receiver(rcv_id, rcv_id_str, port);
    
    //int mpi_rank;
    //MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    
    if (sources.size() > 0) {
	rcv_buf = get_lb_byid(rcv_id_str, port);
    
	// check whether the buffer is int or float
	LocalIntBuffer*     int_rcv_buf = dynamic_cast<LocalIntBuffer*>(rcv_buf);
	LocalFloatBuffer* float_rcv_buf = dynamic_cast<LocalFloatBuffer*>(rcv_buf);
	
	for (int i=0; i<sources.size(); i++) {
	    int src_port = sources[i]->snd_port;
    	    string src_ms_id_str = sources[i]->snd_id_str;
    	    int src_mpi_rank = mpi_map.get_mpi_rank(src_ms_id_str);

	    for (int j=0; j<sources[i]->size; j++)
		if (int_rcv_buf) {
		    int tag = mpi_map.get_tag_by_receiver(rcv_id_str, port);
		    //if (rcv_id_str == "Q0_p0")
		    //cout << "Rank " << mpi_rank << ": MPI_Recv: " << rcv_id_str << ":" << port << " Awaiting message from rank " << src_mpi_rank << " with tag " <<  tag << endl;
		    
		    int value = recv_int_value(src_mpi_rank, tag);
		    rcv_buf->add_value(value);
		    
		    //if (rcv_id_str == "QQmt0")
    		    //cout << "Rank " << mpi_rank << ": MPI_Recv: " << rcv_id_str << ":" << port << " <--{" << value << "}--  from rank " << src_mpi_rank << " with tag " <<  tag << endl;
		} else if (float_rcv_buf) {
		    int tag = mpi_map.get_tag_by_receiver(rcv_id_str, port);
		    
		    //if (rcv_id_str == "QPQ0_Q0")
		    //cout << "MPI_Recv: " << rcv_id_str << ":" << port << " Awaiting message with tag " <<  tag << endl;
		    
		    float value = recv_float_value(src_mpi_rank, tag);
		    rcv_buf->add_value(value);
		    
		    //if (rcv_id_str == "QQmt0")
		    //cout << "MPI_Recv: " << rcv_id_str << ":" << port << " <--{" << value << "}--  from rank " << src_mpi_rank << " with tag " <<  tag << endl;
		}
	}
    } else {
	// nothing to do - no incoming connections on this port
    }
    
};