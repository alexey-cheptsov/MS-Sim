/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H


#include <iostream>
#include <vector>
#include <memory> // shared_ptr

#include "buffer.h"

using namespace std;

class LocalBuffer;

class CommLink {
public:
    CommLink(int snd_id_, int snd_port_, int rcv_id_, int rcv_port_) : 
	snd_id(snd_id_), snd_port(snd_port_), rcv_id(rcv_id_), rcv_port(rcv_port_) { type=0; size = 1; };

    CommLink(string snd_id_, int snd_port_, string rcv_id_, int rcv_port_) : 
	snd_id_str(snd_id_), snd_port(snd_port_), rcv_id_str(rcv_id_), rcv_port(rcv_port_) { type=1; size = 1; };
    
    CommLink(string snd_id_, int snd_port_, string rcv_id_, int rcv_port_, int size_) : 
	snd_id_str(snd_id_), snd_port(snd_port_), rcv_id_str(rcv_id_), rcv_port(rcv_port_), size(size_) { type=1; };
    
    
    int snd_id;
    string snd_id_str;
    int snd_port;
    
    int rcv_id;
    string rcv_id_str;
    int rcv_port;
    
    int size;
    bool type; // 0 - int, 1 - string
};


class Communicator {
public:
    Communicator() {};
	    
    int total_ports_nr;
    vector<CommLink*> comm_links;
    
    void add_comm_link(CommLink* link) {
	comm_links.push_back(link);
    }
    
    void print_comm_links() {
	for (int i=0; i<comm_links.size(); i++)
	    if (!comm_links[i]->type) // int
		cout << "[" << i << "]: " << comm_links[i]->snd_id << ":" << comm_links[i]->snd_port 
		     << " --> " << comm_links[i]->rcv_id << ":" << comm_links[i]->rcv_port 
		     << " , <" << comm_links[i]->size << ">" << endl;
	    else // string
	        cout << "[" << i << "]: " << comm_links[i]->snd_id_str << ":" << comm_links[i]->snd_port 
	    	     << " --> " << comm_links[i]->rcv_id_str << ":" << comm_links[i]->rcv_port 
	    	     << " , <" << comm_links[i]->size << ">" << endl;
    }
    
    vector<CommLink*> get_comm_links_by_sender(int snd_id, string snd_id_str, int snd_port) {
    	vector<CommLink*> related_links;
    
	for (int i=0; i<comm_links.size(); i++)
	    if (!comm_links[i]->type) { // int_id
		if ((comm_links[i]->snd_id==snd_id)&&(comm_links[i]->snd_port==snd_port))
		    related_links.push_back(comm_links[i]);

	    } else // string_id
		if ((comm_links[i]->snd_id_str==snd_id_str)&&(comm_links[i]->snd_port==snd_port))
		    related_links.push_back(comm_links[i]);
	
	return related_links;
    }
    
    vector<CommLink*> get_comm_links_by_receiver(int rcv_id, string rcv_id_str, int rcv_port) {
    	vector<CommLink*> related_links;    
    
	for (int i=0; i<comm_links.size(); i++)
	    if (!comm_links[i]->type) { // int_id
		if ((comm_links[i]->rcv_id==rcv_id)&&(comm_links[i]->rcv_port==rcv_port))
		    related_links.push_back(comm_links[i]);
	    } else // string_id
	    if ((comm_links[i]->rcv_id_str==rcv_id_str)&&(comm_links[i]->rcv_port==rcv_port))
		    related_links.push_back(comm_links[i]);
	
	return related_links;
    }
    
    int get_nr_comm_links_by_receiver(int rcv_id, string rcv_id_str, int rcv_port) {
    	int nr_incoming_links = 0;
    
	for (int i=0; i<comm_links.size(); i++)
	    if (!comm_links[i]->type) { // int_id
		if ((comm_links[i]->rcv_id==rcv_id)&&(comm_links[i]->rcv_port==rcv_port))
		    nr_incoming_links++;
	    } else // string_id
	    if ((comm_links[i]->rcv_id_str==rcv_id_str)&&(comm_links[i]->rcv_port==rcv_port))
		    nr_incoming_links++;
	
	return nr_incoming_links;
    }
    
    void append_communicator(Communicator* appender_communicator) {
	for (int i=0; i<appender_communicator->comm_links.size(); i++)
                add_comm_link(new CommLink(
            		appender_communicator->comm_links[i]->snd_id_str,
            		appender_communicator->comm_links[i]->snd_port,
            		appender_communicator->comm_links[i]->rcv_id_str,
            		appender_communicator->comm_links[i]->rcv_port));
    }
    
    
    virtual LocalBuffer* get_dest_buffer(CommLink* destination) {};
    virtual LocalBuffer* get_src_buffer(CommLink* destination) {};
    
    
    virtual void flush_p2p_replicate(int ms_id, string ms_id_str, int port, string dst) {};    
    virtual void flush_collective_replicate(int ms_id, string ms_id_str, int port) {};
    virtual void flush_collective_spread(int ms_id, string ms_id_str, int port) {};    
    virtual void flush_collective_gather(int ms_id, string ms_id_str, int port) {};    

    
    virtual void sync(int rcv_id, string rcv_id_str, int port) {};
    
    virtual void add_lb(LocalBuffer* lb) {};
    
    virtual void clear() {};
    
    virtual void get_lb_info() {};
};


class NumaCommunicator: public Communicator {
public:
    NumaCommunicator() : Communicator() {};
	    
    vector<LocalBuffer*> all_lb;
    
    void flush_p2p_replicate(int ms_id, string ms_id_str, int port, string dst);
    void flush_collective_replicate(int ms_id, string ms_id_str, int port);
    void flush_collective_spread(int ms_id, string ms_id_str, int port);
    void flush_collective_gather(int ms_id, string ms_id_str, int port);
    
    
    void sync(int rcv_id, string rcv_id_str, int port);
    
    void clear() {cout << "NumaCommunicator: clear" << endl;};

    void add_lb(LocalBuffer* lb) {
	all_lb.push_back(lb);
	//cout << "NumaCommunicator: New LB added" << endl;
    };
    
    LocalBuffer* get_dest_buffer(CommLink* destination);
    LocalBuffer* get_src_buffer(CommLink* destination);
    
    LocalBuffer* get_lb_byid(int ms_id, int port);
    LocalBuffer* get_lb_byid(string ms_id, int port);
    
    void get_lb_info();
    
};

#endif // COMMUNICATOR_H