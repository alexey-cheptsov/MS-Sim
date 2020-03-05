/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef MONITORING_H
#define MONITORING_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include <chrono> // chrono::milliseconds(x)

#include "dirent.h"
#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h> 
#include <malloc.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <curses.h>

using namespace std;

class Monitoring_opts {
public:
    string experiment_id 	= "2019-01-01T00:00:00.000";
    
    bool flag_is_realtime	= 0;
    bool flag_output_file 	= 0;
    bool flag_output_display	= 0;
    bool flag_output_uri 	= 0;
    
    int buf_size		= 0;
    
    char* uri			= "http://localhost:9200/ms/";    
};

template <typename T> 
struct Entry_to_save {
    string id;
    T value;
};

class Monitoring {
public:
    typedef long unsigned int size_t;

    #define SUCCESS 0
    #define FAILED 1

    #define headercode_char_size 11
    /*******************************************************************************
    * Variables Declarations
    ******************************************************************************/
    
    Monitoring_opts* mon_opts = nullptr;
    
    struct curl_slist *headers = NULL;

    struct url_data {
	    size_t size;
	    char* headercode;
	    char* data;
    };

    string experiment_id;

    char* uri = nullptr;

    vector<string> fnames;	
    vector<fstream*> fout;

    char bulk[512];
    char post[512];
 

    vector<stringstream*> ss;

    int counter = 0;
  

    Monitoring(Monitoring_opts* mon_opts_) {
	mon_opts = mon_opts_;
    
	// FOR EXAMPLE: if ((mon_opts_->flag_output_file) && (mon_opts_->flag_output_file)) 
	experiment_id = mon_opts_->experiment_id;
	if (mon_opts_->flag_output_uri){
		uri = mon_opts_->uri;
		char* bulk_s = "_bulk?pretty";
    		char* post_s = "_doc?pretty";
    		strcpy (bulk, uri);
    		strcat (bulk, bulk_s); 
    		strcpy (post, uri);
    		strcat (post, post_s);
	}

	/*if (mon_opts_->flag_output_uri) {
		for (int i=0; i<fnames.size(); i++) 
			fout.push_back(new fstream());
			fout[i]->open("output/" + fnames[i] + ".csv", ios::out);
	    	    	*fout[i] << "ExperimentID;Network;Section;Element;@timestamp;" + name << endl;
	}*/
    }

    static size_t write_data(void *ptr, size_t size, size_t nitems, struct url_data *data) {
	    size_t index = data->size;
	    size_t n = (size * nitems); 
	    data->size += (size * nitems);

	    char *temp_str = (char *) malloc(data->size + 1);
	    if(temp_str==NULL) {
		    fprintf(stderr, "Failed to allocate memory.\n");
		    return 0;
	    }
	    unsigned int i=0; 
	    while((data->data[i]!='\0') &&(i < (unsigned int) index)) {
		    temp_str[i]=data->data[i];
		    i++;
	    }
	    temp_str[i]='\0';
	    if(data->data!=NULL) free(data->data); 
	    data->data=NULL;
	    data->data=temp_str;
	    memcpy((data->data + index), ptr, n);
	    data->data[data->size] = '\0';
	    if(n>9){
		    char tempinit[20];
		    memcpy(tempinit, ptr, 9);
		    tempinit[9] = '\0';
		    int rc = strcmp(tempinit, "HTTP/1.1 ");//[0..8] 
		    if(rc == 0){ 
			    if(data->headercode==NULL)
				    data->headercode = (char *) malloc(headercode_char_size); 
			    i=0;
			    while (data->data[i+9]>='0' && data->data[i+9]<='9' && i<headercode_char_size){
				    data->headercode[i]=data->data[i+9];
				    i++;
			    } 
			    data->headercode[i]='\0';
		    }
	    }
	    return size * nitems;
    }

    void free_data_struc(struct url_data *data){
	    if(data->data!=NULL) free(data->data);
	    if(data->headercode!=NULL) free(data->headercode);
	    data->data=NULL;
	    data->headercode=NULL;
    }

    int reserve_data_struc(struct url_data *data){
	    data->size = 0;
	    data->data = (char*) malloc(5192); /* reasonable size initial buffer */
	    if(NULL == data->data) {
		    data->data=NULL;
		    data->headercode=NULL;
		    return FAILED;
	    }
	    data->data[0] = '\0';
	    data->headercode = (char*) malloc(headercode_char_size); /* reasonable size initial buffer */
	    if(NULL == data->headercode) {
		    free_data_struc(data);
		    return FAILED;
	    }
	    data->headercode[0]= '\0';
	    return SUCCESS;
    }
      

    /** Check if the url is set  */
    int check_URL(const char *URL) {
	    if(URL == NULL || *URL == '\0') {
		    const char *error_msg = "URL not set.";
		    printf("publish(char *, Message) %s", error_msg);
		    return FAILED;
	    }
	    return SUCCESS;
    }

    /** Check if the message is set  */
    int check_message(const char *message) {
	    if(message == NULL || *message == '\0') {
		    const char *error_msg = "message not set.";
		    printf("publish(char *, Message) %s", error_msg);
		    return FAILED;
	    }
	    return SUCCESS;
    }

    /* Initialize libcurl; set headers format */
    void init_curl( ) {
	    if(headers != NULL )
		    return;
	    curl_global_init(CURL_GLOBAL_ALL); 
	    headers = curl_slist_append(headers, "Accept: application/json");
	    headers = curl_slist_append(headers, "Content-Type: application/json");
	    headers = curl_slist_append(headers, "charsets: utf-8");
    }

    // DESCRIPTION: This function releases resources acquired by curl_global_init.
    void close_curl(void) {
    // 	printf(" ***************** CLOSE CURL ***************\n");
	    curl_global_cleanup( );
    }


    /** Prepare for using libcurl with message */
    CURL *prepare_publish(const char *URL,const char *message, FILE *send_fp, const char *operation ) {
	    init_curl( );//this defined the headers
	    CURL *curl = curl_easy_init();
	    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	    curl_easy_setopt(curl, CURLOPT_URL, URL);
	    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	    if (message!= NULL){
		    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);
		    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long ) strlen(message));
	    } 
	    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, operation); /* PUT, POST, ... */
	    return curl;
    }


     /** json-formatted data publish using libcurl */
    int publish_json(char *URL, char *message) {

	    struct url_data rescode;
	    if(reserve_data_struc(&rescode)==FAILED)
		    return FAILED;
	    if(check_URL(URL)!=SUCCESS || check_message(message)!=SUCCESS)
		    return FAILED;
	    const char operation[]="POST";
	    CURL *curl = prepare_publish(URL, message, NULL, operation);
	    if(curl == NULL)
		    return FAILED; 
	    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data); 
	    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &rescode); 
	    CURLcode response_code = curl_easy_perform(curl);
     	//printf("\n RESCODE is %s\n", rescode.data);
	    free(rescode.data); rescode.data=NULL;
	    if(response_code != CURLE_OK) {
		    const char *error_msg = curl_easy_strerror(response_code);
		    printf("publish(char *, Message) %s", error_msg);
		    return FAILED;
	    }
	    //curl_slist_free_all(headers);/* free the list again */
	    curl_easy_cleanup(curl);
	    return SUCCESS;
    }



    int mapping(char *URL, char *message) {
	    struct url_data rescode;
	    if(reserve_data_struc(&rescode)==FAILED)
		    return FAILED;
	    if(check_URL(URL)!=SUCCESS || check_message(message)!=SUCCESS)
		    return FAILED;
	    const char operation[]="PUT";
	    CURL *curl = prepare_publish(URL, message, NULL, operation);
	    if(curl == NULL)
		    return FAILED; 
	    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data); 
	    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &rescode); 
	    CURLcode response_code = curl_easy_perform(curl);
	    free(rescode.data); rescode.data=NULL;
	    if(response_code != CURLE_OK) {
		    const char *error_msg = curl_easy_strerror(response_code);
		    printf("publish(char *, Message) %s", error_msg);
		    return FAILED;
	    }
	    curl_easy_cleanup(curl);
	    return SUCCESS;
    }

    int del(char *URL) {
	    struct url_data rescode;
	    if(reserve_data_struc(&rescode)==FAILED)
		    return FAILED;
	    if(check_URL(URL)!=SUCCESS)
		    return FAILED;
	    const char operation[]="DELETE";
	    CURL *curl = prepare_publish(URL, " ", NULL, operation);
	    if(curl == NULL)
		    return FAILED; 
	    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data); 
	    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &rescode); 
	    CURLcode response_code = curl_easy_perform(curl);
	    free(rescode.data); rescode.data=NULL;
	    if(response_code != CURLE_OK) {
		    const char *error_msg = curl_easy_strerror(response_code);
		    printf("publish(char *, Message) %s", error_msg);
		    return FAILED;
	    }
	    curl_easy_cleanup(curl);
	    return SUCCESS;
    }

    template<typename T>
    void add_entry(string net, string sec, string elem, string timestamp, vector<Entry_to_save<T>>& entries)
    {
	for (int i=0; i<entries.size(); i++) {
	    
	    if (mon_opts->flag_output_file){
                    *ss[i] << "\"" << experiment_id << "\";\"" << net << "\";\""<< sec << "\";\""
                       << elem << "\";\"" << timestamp << "\";" << entries[i].value << endl;

            }
            else{
                    *ss[i] << "{\"index\":{\"_index\":\"ms\",\"_type\":\"_doc\"} }" << endl;
                    *ss[i] << "{\"ExperimentID\":\"" << experiment_id << "\",\"Network\":\""
                       << net << "\",\"Section\":\"" << sec << "\",\"Element\":\""
                       << elem << "\",\"@timestamp\":\"" << timestamp << "\",\"" << entries[i].id
                       << "\":" << entries[i].value << "}" << endl;
            }
        }
        
        counter++;

        if (counter == mon_opts->buf_size) {
            if (mon_opts->flag_output_file){
        	for (int i=0; i<entries.size(); i++){
                    *fout[i] << ss[i]->str();
		    fout[i]->flush();
		}
            }
            else{
                    if (mon_opts->flag_output_uri){
			for (int i=0; i<entries.size(); i++){
                            char json_msg[ss[i]->str().length()+1];
		   	    strcpy (json_msg, ss[i]->str().c_str());
			    publish_json(bulk, json_msg);
			}
                    }
            }
    	
    	    for (int i=0; i<entries.size(); i++)
    		ss[i]->str("");
    	    counter = 0;
	}
    }
    
    template<typename T>
    void add_entry(string net, string sec, string elem, float timestamp, vector<Entry_to_save<T>>& entries)
    {
	add_entry<T>(net, sec, elem, to_string(timestamp), entries);
    }

 template<typename T>
 void data_flush(string id_str, vector<Entry_to_save<T>>& entries)
    {
	
	int count_lines=0;
	char c;
	char labels[30][100];
	char values[30][100];
	char j_msg[512];
	int fcounter=0;
	int num_label=0;
	int num_char=0;
	int flush_counter = 0;
	stringstream ssf;


        if ((mon_opts->flag_output_uri) && (!mon_opts->flag_output_file)) {
	    if (counter != 0){
		for (int i=0; i<entries.size(); i++){
		      	char json_msg[ss[i]->str().length()+1];						
		      	strcpy (json_msg, ss[i]->str().c_str());
		      	publish_json(bulk, json_msg);
		}
	    }	
	} 
	
	if (mon_opts->flag_output_file) {
		for (int i=0; i<entries.size(); i++){
		    //cout << "flush ss is:" << ss[i]->str() << endl;
                    *fout[i] << ss[i]->str() << "!";
		    //fout[i]->flush();
		}
		if (mon_opts->flag_output_uri){ 
		    for (int i=0; i<entries.size(); i++){
			fout[i]->clear();
			fout[i]->seekg(0, ios::beg);	    
			int finish = 0;
			fcounter = 0;
			flush_counter = 0;
			num_char=0;

			while (!finish) {	
			    c = fout[i]->get();
			    if (c == EOF) {	
				finish = 1;
			    } else {   
				if(( fcounter ==0))  {
				    if ((c==';' || c=='\n')) {
					num_label++;
		    			num_char=0;
					labels[num_label][num_char]='\0';
				    }
			    
				    if ((c!=';')&& (c!=10)) {
					labels[num_label][num_char]=c;
					labels[num_label][num_char+1]='\0';
				    	num_char=num_char+1;
				    }
				
				    if(c=='\n') {
					fcounter ++;
				    	num_label=0;
				    	num_char=0;
				    	values[num_label][num_char]='\0';
				    }
				} else {
				    if ((c==';' || c=='\n')){
				    	num_label++;
				    	num_char=0;
				    	values[num_label][num_char]='\0';
				    }
				
				    if ((c!=';') && (c!=10)){
					values[num_label][num_char]=c;
				    	values[num_label][num_char+1]='\0';
				    	num_char=num_char+1;
				    }
				    
				    if(c=='\n') {
					sprintf(j_msg,"{");    
					for(int i=0;i<num_label;i++) {
					    if(i>0) sprintf(j_msg,"%s,",j_msg);
					    	sprintf(j_msg,"%s\"%s\":%s",j_msg,labels[i],values[i] );
					}
					sprintf(j_msg,"%s}\n",j_msg);
				     	//cout << "j_msg is:" <<  j_msg << endl;
					ssf << "{\"index\":{\"_index\":\"ms\",\"_type\":\"_doc\"} } \n" 				    << j_msg;
					flush_counter++;	
						    
					if (flush_counter == mon_opts->buf_size){ 
					    flush_counter = 0;
					    if (mon_opts->flag_output_uri){
						char msg[ssf.str().length()+1];
						strcpy (msg, ssf.str().c_str());
						//cout << "flush msg is:" <<  msg << endl;
						publish_json(bulk, msg);
						}
					    ssf.str("");
					} 					   
					num_label=0;
				     }
				     if (c == '!'){
					if (flush_counter !=0){ 
					char msg[ssf.str().length()+1];
					strcpy (msg, ssf.str().c_str());
					//cout << "after !flush msg is:" <<  msg << endl;
					publish_json(bulk, msg);}
				    }
				}
			    }
			} 
		    }
	        }  
	  }
    }

}; // Clas Monitoring

class Time_MS {
public:
    long year;
    long month;
    long day;
    long hours;
    long minutes;
    long seconds;
    long milliseconds;
    char* time_str;


    string time_stamp()
        {
                time_t t = time(0);   // get time now
                tm* now = localtime(&t);

                year = now->tm_year + 1900;
                month = now->tm_mon + 1;
                day = now->tm_mday;

                char buf[40];
                sprintf(buf, "%04ld-%02ld-%02ldT%02ld:%02ld:%02ld.%03ld", year, month, day, hours, minutes, seconds, milliseconds);
                return buf;
        }


    void init_time() {
        // Get current time from the clock, using microseconds resolution
        const boost::posix_time::ptime now =
        boost::posix_time::microsec_clock::local_time();


        // Get the time offset in current day
        const boost::posix_time::time_duration td = now.time_of_day();

        hours        = td.hours();
        minutes      = td.minutes();
        seconds      = td.seconds();
        //milliseconds = td.total_milliseconds() - ((hours * 3600 + minutes * 60 + seconds) * 1000);
	milliseconds = 000;
    }

    void increment_time_ms(float step)
        {
            milliseconds += step*1000;
                 if (milliseconds > 999)
                 {
                        milliseconds = (milliseconds-1000);
                        seconds++;
                        if (seconds > 59)
                                {
                                seconds = (seconds-60);
                                minutes++;
                                if (minutes > 59)
                                        {
                                        minutes = (minutes-60);
                                        hours++;
                                        if (hours > 23)
                                                hours = (hours-24);
                                        }
                                }
                 }
        }
};
        
#endif // MONITORING_H
