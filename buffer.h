/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef BUFFER_H
#define BUFFER_H

#include <sstream>
#include <iostream>
#include <vector>
#include <memory> // shared_ptr
#include <mutex>  // std::mutex

#include "communicator.h"

using namespace std;

class LocalBuffer {
public:
    
    //virtual ~LocalBuffer() { }; // a destructor is used to make the class polymorphic 
				  // (having at least one virtual method)

    LocalBuffer(int ms_id_, string ms_id_str_, int port_, Communicator* communicator_) : 
	ms_id(ms_id_), ms_id_str(ms_id_str_), port(port_), communicator(communicator_) {};
				  
    int port;
    int ms_id;
    string ms_id_str;
    
    mutex lock;
    
    Communicator* communicator;
    
    virtual int get_size() {};
    
    virtual void add_value(int value) {}; // {cout << "LocalBuffer: int value added" << endl;};
    
    virtual void add_value(float value) {}; // {cout << "LocalBuffer: float value added" << endl;};
    
    virtual int get_int_value(int index) {}; // { cout << "LocalBuffer: int value retrieved" << endl; return 0; };
    
    virtual int get_int_value_and_shift() {}; // { cout << "LocalBuffer: shifted int value retrieved" << endl; return 0; };
    
    virtual float get_float_value(int index) {}; // { cout << "LocalBuffer: float value retrieved" << endl; return 0; };
    
    virtual float get_float_value_and_shift() {}; // { cout << "LocalBuffer: shifted int value retrieved" << endl; return 0; };
    
    void flush_collective_replicate() {
	communicator->flush_collective_replicate(ms_id, ms_id_str, port /*sender*/);	
    };	

    void flush_collective_spread() {
	communicator->flush_collective_spread(ms_id, ms_id_str, port /*sender*/);
    };	
    
    void flush_collective_gather() {
	communicator->flush_collective_gather(ms_id, ms_id_str, port /*sender*/);
    };	
    
    // flush for a specific destination (as string)
    void flush_p2p_replicate(string dest) {
	communicator->flush_p2p_replicate(ms_id, ms_id_str, port /*sender*/,
						dest /*receiver*/);	
    };	
    
    void sync() {
	//stringstream out;
	//out << "syncing LocalBuffer: ms_" << ms_id << "(" << ms_id_str << "):" << port << ", size=" << size << endl;	
	//cout << out.str();
	
	//communicator->get_lb_info();
	communicator->sync(ms_id, ms_id_str, port /*sender*/);
    };
    
    void set_lock() {
	
	lock.lock();
	
    	//stringstream out;
    	//out << "Set-Lock: " << ms_id_str << endl;
    	//cout << out.str();
    	
    };
    
    void reset_lock() {
	
	lock.unlock();
		
	//stringstream out;
    	//out << "Reset-Lock: " << ms_id_str << endl;
    	//cout << out.str();
    }


    virtual void clear() {};    
    
};

class LocalIntBuffer: public LocalBuffer {
public:

    LocalIntBuffer(int ms_id_, string ms_id_str_, int port_, Communicator* communicator_) : 
	    LocalBuffer(ms_id_, ms_id_str_, port_, communicator_) {

	communicator->add_lb(this);
    };

    
    vector<int> lb;
    
    int get_size() {
	int size;
	
	set_lock();
	size = lb.size();
	reset_lock();
        
        return size;
    };
    
    void add_value(int value) {
    
	set_lock();
    
	lb.push_back(value);
    	
    	//stringstream out;
    	//out << "ms[" << ms_id << "]:" << port << " : LocalIntBuffer: Value added: " << value << endl;
    	//cout << out.str();
    	
    	reset_lock();
    };
    
    int get_int_value(int index) {
    
	set_lock();
    
	if (lb.size() > index) {
	    //cout << "LocalIntBuffer: int value retrieved: " << lb[index] << endl;
	    reset_lock();
	    return lb[index];
	} else
	{
	    cout << "LocalIntBuffer: error 44444 while retrieving int value for index " << index << endl;
	    reset_lock();
	    return 44444;
	}
	    
	    
        // deleting the retrieved element from the buffer
        //this->buffer[port].erase(this->buffer.begin() + 0/*index*/);
    };
    
    int get_int_value_and_shift() {
    
	set_lock();
    
	if (lb.size() > 0) {
	    int value = lb[0];
	    
	    // deleting the retrieved element from the buffer
    	    lb.erase(lb.begin() + 0/*index*/);
    	    
    	    reset_lock();
	    return value;
	    
	} else {
	    reset_lock();    
	    return 4444;
	}
	    
        // deleting the retrieved element from the buffer
        //this->buffer[port].erase(this->buffer.begin() + 0/*index*/);
    };
    
    void clear() {
    
	set_lock();
    
	lb.clear();
	
	//stringstream out;
	//out << "Cleared LocalIntBuffer: ms[" << ms_id << "], port[" << port << "]" << endl;
	//cout << out.str();
	
	reset_lock();
    }

};

class LocalFloatBuffer: public LocalBuffer {
public:

    LocalFloatBuffer(int ms_id_, string ms_id_str_, int port_, Communicator* communicator_) : 
	    LocalBuffer(ms_id_, ms_id_str_, port_, communicator_) {
	
	communicator->add_lb(this);
    };

    vector<float> lb;
    
    int get_size() {
	
	set_lock();
	int size = lb.size();
	reset_lock();
	
	return size;
    };
    
    void add_value(float value) {
    
	set_lock();
    
	lb.push_back(value);
	
	//stringstream out;
	//out << "Size_old = " << size_tmp << ", new = " << size;
    	//out << " ms" << ms_id << "(" << ms_id_str << "):" << port << " :LocalFloatBuffer: Value added: " << value 
    	//    << ", Size=" << size << "(" << lb.size() << ")" << endl;
    	//cout << out.str();	
    	
    	reset_lock();
    };
    
    float get_float_value(int index) {
    
	set_lock();
    
	if (lb.size() > index) {
	    //cout << "LocalFloatBuffer: float value retrieved: " << lb[index] << endl;
	    reset_lock();
	    return lb[index];
	} else {
	    reset_lock();
	    return 444;
	}
	    
	
        // deleting the retrieved element from the buffer
        // this->buffer[port].erase(this->buffer.begin() + 0/*index*/);
    };
    
    float get_float_value_and_shift() {
    
	set_lock();
	
	if (lb.size() > 0) {
	    float value = lb[0];
	    
	    // deleting the retrieved element from the buffer
    	    lb.erase(lb.begin() + 0/*index*/);
    	    
    	    //stringstream out;
	    //out << "Decremeneted Buffer: ms_" << ms_id << "(" << ms_id_str << ", port[" << port << "] : New size = " << size << endl;
	    //cout << out.str();
    	    
    	    reset_lock();
	    return value;
	    
	} else {
	    reset_lock();
	    return 44;
	}
	    
        // deleting the retrieved element from the buffer
        //this->buffer[port].erase(this->buffer.begin() + 0/*index*/);
    };

    
    void clear() {
    
	set_lock();
	
	lb.clear();
	
	//stringstream out;
	//out << "Cleared LocalFloatBuffer: ms_" << ms_id << "(" << ms_id_str << ", port[" << port << "] : New size = " << size << endl;
	//cout << out.str();
	
	reset_lock();
    }
};

#endif // BUFFER_H