/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef MICROSERVICE_H
#define MICROSERVICE_H

#include <iostream>
#include <vector>
#include <sstream>
#include <chrono>
#include <thread>

#include "buffer.h"

using namespace std;

class Microservice {
public:
    Microservice(int id_, string id_str_) : id(id_), id_str(id_str_) {
	communicator = new NumaCommunicator();
    };
    Microservice(int id_, string id_str_, Communicator* communicator_) : 
	id(id_), id_str(id_str_), communicator(communicator_) {};
    
    
    // displaicement of proxy ports
    const int proxy_disp = 1000; 

    int id;
    string id_str;
    virtual void run() {};
    
    template<class T>
    void add_buffer(T* lb) {
	if (buffers.size() != lb->port) {
	    stringstream out;
	    out << id_str << ": Adding new buffer failed. Wrong port number (" << lb->port <<") . Execution stopped" << endl;
	    cout << out.str();
	    exit(1);
	}
	buffers.push_back(lb);
    };
        
    template<typename T>
    void add_buffer_value(int port, T value) {
	if (port < buffers.size())
            buffers[port]->add_value(value);
        else {
            stringstream out;
            out << id_str << ": Adding new element to buffer " << port << " failed. Port not available. Execution stopped" << endl;
            cout << out.str();
            exit(1);
        }
    };
    
    template<typename T>
    T get_buffer_value (int port, int index) {
	return 0;
    };  
    
    template<typename T>
    T get_buffer_value_stack (int port) {
	return 0;
    };  

    int buffer_get_size(int port) {
	return buffers[port]->get_size();
    }
        
    // i:[1..n] -> j:[1..n]
    void buffer_flush_p2p_replicate(int port, string dest) {
	buffers[port]->flush_p2p_replicate(dest);
    }
    
    // i:[1..n] -> j1..jm[1..n]
    void buffer_flush_collective_replicate(int port) {
	buffers[port]->flush_collective_replicate();
    }
    
    // i:[1..n] -> j1..jm[1..n/m]
    void buffer_flush_collective_spread(int port) {
	buffers[port]->flush_collective_spread();
    }
    
    // i1..im:[1..n] -> j[1..n*m]
    void buffer_flush_collective_gather(int port) {
	buffers[port]->flush_collective_gather();
    }
    
    void buffer_sync(int port) {
	buffers[port]->sync();
    }
    
    void buffer_clear(int port) {
	buffers[port]->clear();
    }
    
    template<class T>
    void add_proxy(T* lb) {
	if (proxies.size() != lb->port) {
	    stringstream out;
	    out << "ms_" << id_str << ": Adding new proxy failed. Wrong port number (" << lb->port <<") . Execution stopped" << endl;
	    cout << out.str();
	    exit(1);
	}
	
	lb->port += 1000;
	proxies.push_back(lb);
    };
    
    template<typename T>
    void add_proxy_value(int port, T value) {
	proxies[port]->add_value(value);
    };
    
    template<typename T>
    T get_proxy_value (int port, int index) {
	return 0;
    };  
    
    template<typename T>
    T get_proxy_value_stack (int port) {
	return 0;
    };  

    int proxy_get_size(int port) {
	return proxies[port]->get_size();
    }
        
    // i:[1..n] -> j:[1..n]
    void proxy_flush_p2p_replicate(int port, string dest) {
	proxies[port]->flush_p2p_replicate(dest);
    }
    
    // i:[1..n] -> j1..jm[1..n]
    void proxy_flush_collective_replicate(int port) {
	proxies[port]->flush_collective_replicate();
    }
    
    // i:[1..n] -> j1..jm[1..n/m]
    void proxy_flush_collective_spread(int port) {
	proxies[port]->flush_collective_spread();
    }
    
    // i1..im:[1..n] -> j[1..n*m]
    void proxy_flush_collective_gather(int port) {
	proxies[port]->flush_collective_gather();
    }
    
    void proxy_sync(int port) {
	proxies[port]->sync();
    }
    
    void proxy_clear(int port) {
	proxies[port]->clear();
    }
    

    
    vector<LocalBuffer*> buffers;
    vector<LocalBuffer*> proxies;
    
    Communicator* communicator;
    
    LocalBuffer* get_lb_byid(int ms_id, int port) {
        for (int i=0; i<buffers.size(); i++)
            if ((buffers[i]->ms_id==ms_id)&&(buffers[i]->port==port))
                return buffers[i];

        return nullptr;
    }
    
    LocalBuffer* get_lb_byid(string ms_id, int port) {
        for (int i=0; i<buffers.size(); i++)
            if ((buffers[i]->ms_id_str==ms_id)&&(buffers[i]->port==port))
                return buffers[i];

        return nullptr;
    }
    
    LocalBuffer* get_proxy_byid(int ms_id, int port) {
        for (int i=0; i<proxies.size(); i++)
            if ((proxies[i]->ms_id==ms_id)&&(proxies[i]->port==port))
                return proxies[i];

        return nullptr;
    }
    
    LocalBuffer* get_proxy_byid(string ms_id, int port) {
        for (int i=0; i<proxies.size(); i++)
            if ((proxies[i]->ms_id_str==ms_id)&&(proxies[i]->port==port))
                return proxies[i];

        return nullptr;
    }

};

template<> int Microservice::get_buffer_value<int>(int port, int index) {
    return buffers[port]->get_int_value(index);
};

template<> float Microservice::get_buffer_value<float>(int port, int index) {
    return buffers[port]->get_float_value(index);
};

template<> int Microservice::get_buffer_value_stack<int>(int port) {

    while (buffers[port]->get_size()==0) {
	this_thread::sleep_for(chrono::microseconds(1));
	//cout << "Stucking in while in MS::get_buffer_value_stack_int\n";	
    }
    
    int value = buffers[port]->get_int_value_and_shift(); // waiting until element appears
    return value;
};
    
template<> float Microservice::get_buffer_value_stack<float>(int port) {

    while (buffers[port]->get_size()==0) {
	this_thread::sleep_for(chrono::microseconds(1));
	//cout << "Stucking in while in MS::get_buffer_value_stack_float\n";
    } // waiting until element appears

    float value = buffers[port]->get_float_value_and_shift();
    return value;
};


template<> int Microservice::get_proxy_value<int>(int port, int index) {
    return proxies[port]->get_int_value(index);
};

template<> float Microservice::get_proxy_value<float>(int port, int index) {
    return proxies[port]->get_float_value(index);
} ;

template<> int Microservice::get_proxy_value_stack<int>(int port) {

    while (proxies[port]->get_size()==0) {
	this_thread::sleep_for(chrono::microseconds(1));
	//cout << "Stucking in while in MS::get_buffer_value_stack_int\n";	
    }
    
    int value = proxies[port]->get_int_value_and_shift(); // waiting until element appears
    return value;
};
    
template<> float Microservice::get_proxy_value_stack<float>(int port) {

    while (proxies[port]->get_size()==0) {
	this_thread::sleep_for(chrono::microseconds(1));
	//cout << "Stucking in while in MS::get_buffer_value_stack_float\n";
    } // waiting until element appears

    float value = proxies[port]->get_float_value_and_shift();
    return value;
}

#endif // MICROSERVICE_H