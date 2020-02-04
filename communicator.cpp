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

using namespace std;

void NumaCommunicator::get_lb_info() {

    for (int i=0; i< all_lb.size(); i++) {
    
	LocalBuffer* base = all_lb[i];
	LocalIntBuffer* intbuf = dynamic_cast<LocalIntBuffer*>(base);
	LocalFloatBuffer* floatbuf = dynamic_cast<LocalFloatBuffer*>(base);
	
	if (intbuf) {
	    stringstream out;
    	    out << "   - ms[" << all_lb[i]->ms_id << "], port[" << all_lb[i]->port << "]:" << all_lb[i]->get_int_value(0)  << endl;
    	    cout << out.str();
    	}
    
	if (floatbuf) {
	    stringstream out;
    	    out << "   - ms[" << all_lb[i]->ms_id << "], port[" << all_lb[i]->port << "]:" << all_lb[i]->get_float_value(0)  << endl;
    	    cout << out.str();
    	}
    
    }
    
}

LocalBuffer* NumaCommunicator::get_lb_byid(int ms_id, int port) {
        for (int i=0; i<all_lb.size(); i++)
            if ((all_lb[i]->ms_id==ms_id)&&(all_lb[i]->port==port))
                return all_lb[i];
    
        return nullptr;
}


LocalBuffer* NumaCommunicator::get_lb_byid(string ms_id, int port) {
        for (int i=0; i<all_lb.size(); i++)
            if ((all_lb[i]->ms_id_str==ms_id)&&(all_lb[i]->port==port))
                return all_lb[i];
    
	cout << "NumaCommunicator: Cannot find buffer with id " << ms_id << ", port " << port << endl;
	exit(1);
	
        return nullptr;
}

LocalBuffer* NumaCommunicator::get_dest_buffer(CommLink* destination) {
        if (!destination->type) // int_id
            return get_lb_byid(destination->rcv_id, destination->rcv_port);
        else            // string_id
            return get_lb_byid(destination->rcv_id_str, destination->rcv_port);
}
            
LocalBuffer* NumaCommunicator::get_src_buffer(CommLink* destination) {
        if (!destination->type) // int_id
            return get_lb_byid(destination->snd_id, destination->snd_port);
        else            // string_id
            return get_lb_byid(destination->snd_id_str, destination->snd_port);
}



void NumaCommunicator::flush_p2p_replicate(int ms_id, string ms_id_str, int port, string dst) {
					
    vector<CommLink*> destinations = get_comm_links_by_sender(ms_id, ms_id_str, port);
    // TO-DO: Error handling when no destinations available (wrong comm map)
    LocalBuffer* snd_buf = get_src_buffer(destinations[0]);
    
    for (int i=0; i<destinations.size(); i++) {
        if (destinations[i]->rcv_id_str == dst) {
        
	    LocalBuffer* rcv_buf = get_dest_buffer(destinations[i]);
    	    LocalIntBuffer*     int_snd_buf = dynamic_cast<LocalIntBuffer*>(snd_buf);
	    LocalFloatBuffer* float_snd_buf = dynamic_cast<LocalFloatBuffer*>(snd_buf);
	    
	    for (int j=0; j<snd_buf->get_size(); j++) {
		if (int_snd_buf) {
        	    int value = int_snd_buf->get_int_value(j /*index*/);
	    	    rcv_buf->add_value(value);
    		} else if (float_snd_buf) {
	    	    float value = float_snd_buf->get_float_value(j /*index*/);
		    rcv_buf->add_value(value);
		}
	    }
	}
    }
}

void NumaCommunicator::flush_collective_replicate(int ms_id, string ms_id_str, int port) {
    vector<CommLink*> destinations = get_comm_links_by_sender(ms_id, ms_id_str, port);
    // TO-DO: Error handling when no destinations available (wrong comm map)
    LocalBuffer* snd_buf = get_src_buffer(destinations[0]);
    
    for (int i=0; i<destinations.size(); i++) {
	//int dst_ms_id = destinations[i]->rcv_id;
	//string dst_ms_id_str = destinations[i]->rcv_id_str;	
        //int dst_port = destinations[i]->rcv_port;
        //cout << "Processing destination " << i << ": " << dst_ms_id_str << ":" << dst_port << endl;        

	LocalBuffer* rcv_buf = get_dest_buffer(destinations[i]);
        LocalIntBuffer*     int_snd_buf = dynamic_cast<LocalIntBuffer*>(snd_buf);
	LocalFloatBuffer* float_snd_buf = dynamic_cast<LocalFloatBuffer*>(snd_buf);
    
	for (int j=0; j<snd_buf->get_size(); j++) {
    	    if (int_snd_buf) {
    	    	int value = int_snd_buf->get_int_value(j /*index*/);
    		rcv_buf->add_value(value);
    		
    		//stringstream out;
		//out << "NumaIntCommunicator: ms_" << snd_buf->ms_id << "(" << snd_buf->ms_id_str << "):"
		//    << snd_buf->port << " --> {" << value << "} --> ms_" << rcv_buf->ms_id << "(" << rcv_buf->ms_id_str << "):"<< rcv_buf->port << endl;
		//cout << out.str();	
	
            } else if (float_snd_buf) {
	        float value = float_snd_buf->get_float_value(j /*index*/);
	        rcv_buf->add_value(value);

		//if (ms_id_str == "Q0") {
		//    stringstream out;
		//    out << "NumaFloatCommunicator: ms_" << snd_buf->ms_id << "(" << snd_buf->ms_id_str << "):"
		//	<< snd_buf->port << " --> {" << value << "} --> ms_" << rcv_buf->ms_id << "(" << rcv_buf->ms_id_str << "):"<< rcv_buf->port << endl;
		//    out << "Buffer: " << rcv_buf << endl;
		//    out << "New buf_size: " << rcv_buf->get_size() << endl;
		//    cout << out.str();
		//}
	    }
	}
    }
};

void NumaCommunicator::flush_collective_spread(int ms_id, string ms_id_str, int port) {

    vector<CommLink*> destinations = get_comm_links_by_sender(ms_id, ms_id_str, port);
    // TO-DO: Error handling when no destinations available (wrong comm map)
    LocalBuffer* snd_buf = get_src_buffer(destinations[0]);

//    cout << "!!!!!!!!!!!!!!!Flushing_spread for ms " << ms_id_str << ", port " << port << ", buf_size = " << snd_buf->get_size() << ", dests = " << destinations.size() << endl;

    for (int i=0; i<destinations.size(); i++) {
    
	int nr_entries_per_dest = snd_buf->get_size() / destinations.size();
	
	LocalBuffer* rcv_buf = get_dest_buffer(destinations[i]);
    	LocalIntBuffer*     int_snd_buf = dynamic_cast<LocalIntBuffer*>(snd_buf);
	LocalFloatBuffer* float_snd_buf = dynamic_cast<LocalFloatBuffer*>(snd_buf);
	
	for (int j=0; j<nr_entries_per_dest; j++) {
	    int value_index_in_snd_buf = i*nr_entries_per_dest + j;
	
	    if (int_snd_buf) {
    		int value = int_snd_buf->get_int_value(value_index_in_snd_buf /*index*/);
		rcv_buf->add_value(value);
    	    }
	    else if (float_snd_buf) {
		float value = float_snd_buf->get_float_value(value_index_in_snd_buf /*index*/);
		rcv_buf->add_value(value);
		
		//stringstream out;
		//out << "NumaFloatCommunicator: ms_" << snd_buf->ms_id << "(" << snd_buf->ms_id_str << "):"
		//    << snd_buf->port << " --> {" << value << "} --> ms_" << rcv_buf->ms_id << "(" << rcv_buf->ms_id_str << "):"<< rcv_buf->port << endl;
		//cout << out.str();
	    }
	}
    }
};


void NumaCommunicator::flush_collective_gather(int ms_id, string ms_id_str, int port) {

    vector<CommLink*> destinations = get_comm_links_by_sender(ms_id, ms_id_str, port);
    if (destinations.size() > 1) {
	cout << ms_id_str << ":" << port << " - Wrong use of flush_collective_gather communicator. Destinations nr. must be 1 or 0 (here - )" 
	    << destinations.size() << endl;
	exit(1);
    }
    
    if (destinations.size() == 1) {

        CommLink* destination = destinations[0];
        LocalBuffer* snd_buf = get_src_buffer(destination);    
    
        //
        // Calculate the amount of entries to be sent by ms-predecessors
        // in the collective communication
        //
        vector<CommLink*> all_sources_of_my_destination 
    	    = get_comm_links_by_receiver(destination->rcv_id, destination->rcv_id_str, destination->rcv_port);
    	
        int displ=0; // my displacement in the common receiver buffer
    
	for (int i=0; i<all_sources_of_my_destination.size(); i++) {
	    if (all_sources_of_my_destination[i]->snd_id_str == ms_id_str)
		break;
	
	    displ += all_sources_of_my_destination[i]->size;
	}
    
        LocalBuffer* rcv_buf = get_dest_buffer(destination);
	LocalIntBuffer*     int_snd_buf = dynamic_cast<LocalIntBuffer*>(snd_buf);
	LocalFloatBuffer* float_snd_buf = dynamic_cast<LocalFloatBuffer*>(snd_buf);
	
        // waiting until the ms-predecessors are done with flush
	bool buf_ready = false;
	while (!buf_ready) { 
	    int size = rcv_buf->get_size();
	    if (displ==size)
		buf_ready = true;
//	    cout << "Stucking in ms " << ms_id_str << ":" << port << ": displ = " << displ << ", size = " << size << endl;
	}

//    stringstream out;
//    out << "ms_" << ms_id << "(" << ms_id_str << "): displ = " << displ<< ", size = " << rcv_buf->get_size() <<" Flushing  {";

	for (int i=0; i<snd_buf->get_size(); i++) {
	    if (int_snd_buf) {
    		int value = int_snd_buf->get_int_value(i /*index*/);
		rcv_buf->add_value(value);
    	    } else if (float_snd_buf) {
		float value = float_snd_buf->get_float_value(i /*index*/);
	    
//  out << "[" << rcv_buf->get_size() << "]" << value << ", ";
	    
		rcv_buf->add_value(value);
	    }
	}
    
//    out << "}" << endl;
//    cout << out.str();
    }
};

void NumaCommunicator::sync(int rcv_id, string rcv_id_str, int port) {

    LocalBuffer* rcv_buf;
    vector<CommLink*> sources = get_comm_links_by_receiver(rcv_id, rcv_id_str, port);
    
    if (sources.size() > 0) {
    
	// check whether to use id or id_str communicator
	if (!sources[0]->type) 
	    // int_id
	    rcv_buf = get_lb_byid(rcv_id, port);
	else
	    // string_id
	    rcv_buf = get_lb_byid(rcv_id_str, port);
    
	// calculation of the number of incoming messages
	int weight = 0;
	for (int i=0; i<sources.size(); i++)
	    weight += sources[i]->size;
    
	while (rcv_buf->get_size() < weight) { // waiting until the buffer is filled
	    this_thread::sleep_for(chrono::milliseconds(1));
	    //cout << "Stucking in while in NumaCommunicator::sync\n";
	}
    } else {
	// nothing to do - no incoming connections on this port
    }
    
};