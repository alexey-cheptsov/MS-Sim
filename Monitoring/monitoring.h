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
    
    // indicates that the timestamp shall be 
    // in real-time format: yyyy-mm-ddThh:mm:ss.mms
    bool flag_is_realtime		= 0;
    
    // indicates the preferable option of 
    // data saving
    bool flag_output_display		= 0;
    bool flag_output_csv 		= 0;
    bool flag_output_es 		= 0;
    bool flag_output_es_via_files 	= 0;
    
    // size of internal data buffer, in entries
    int buf_size			= 0;
    
    // URI of elastic search
    char* es_uri			= "http://localhost:9200/";    
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
    
    // enables (1) or deprecates (0) debug output from curl
    #define CURL_DEBUG_ON 0

    #define headercode_char_size 11

    char* dynamic_mapping = "{ \"mappings\" : {\"dynamic_templates\": [ { \"floats\" : { \"match_mapping_type\" : \"long\", 				       \"mapping\" : { \"type\" : \"float\"}}}]}}";
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

    char* localhost = nullptr;
	
    vector<fstream*> fout;

    int filenr = 0;

    char bulk[512];
    char post[512];
    char search[512];
    char index[512];
    char uri[512];
    bool flag_index_not_found = 0;

    vector<stringstream*> ss;
    vector<stringstream*> ssu;

    int counter = 0;
    int nflush = 0;

    Monitoring(Monitoring_opts* mon_opts_) {
	mon_opts = mon_opts_;
    
	// FOR EXAMPLE: if ((mon_opts_->flag_output_file) && (mon_opts_->flag_output_file)) 
	if (((mon_opts_->flag_output_es)&&(mon_opts_->flag_output_es_via_files))||   					  	     ((mon_opts_->flag_output_csv)&&(mon_opts_->flag_output_es_via_files)))
		mon_opts_->flag_output_es_via_files = 0;

	if ((mon_opts_->flag_output_csv)&&(mon_opts_->flag_output_es_via_files)&&(mon_opts_->flag_output_es))
		mon_opts_->flag_output_es_via_files = 0;


	experiment_id = mon_opts_->experiment_id;
	if ((mon_opts_->flag_output_es)||(mon_opts_->flag_output_es_via_files)){

		char* index_s = "ms";
		char* bulk_s = "/_bulk?pretty";
    		char* post_s = "/_doc?pretty";
		char* search_s = "/_search?pretty";

		strcpy (uri, mon_opts_->es_uri);
    		strcat (uri, index_s); 
    		strcpy (bulk, uri);
    		strcat (bulk, bulk_s); 
    		strcpy (post, uri);
    		strcat (post, post_s);
		strcpy (index, index_s);

		if (mon_opts_->flag_is_realtime){
			char* index_s = "ms_realtime";
			strcpy (uri, mon_opts_->es_uri);
    			strcat (uri, index_s); 
			strcpy (post, uri);
			strcat (post, post_s);
			strcpy (bulk, uri);
    			strcat (bulk, bulk_s); 
			strcpy (index, index_s);	
		}	
	}
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
    
        static size_t write_data_suppress_output(void *buffer, size_t size, size_t nmemb, void *userp)
    {
	return size * nmemb;
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
	    
	    if (! CURL_DEBUG_ON)
    		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_suppress_output);
	    
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

    int map(char *URL, char *message) {
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

    int get(char *URL) {
	    struct url_data rescode;
	    if(reserve_data_struc(&rescode)==FAILED)
		    return FAILED;
	    if(check_URL(URL)!=SUCCESS)
		    return FAILED;
	    const char operation[]="GET";
	    CURL *curl = prepare_publish(URL, "", NULL, operation);
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

    void mapping() //	  
    {	
	if ((mon_opts->flag_output_es)||(mon_opts->flag_output_es_via_files)) {
		map(uri, dynamic_mapping);    

	}
    }


     template<typename T>
    void add_entry(string net, string sec, string elem, string timestamp, vector<Entry_to_save<T>>& entries)
    {
	filenr = entries.size();
	for (int i=0; i<entries.size(); i++) {
	    
	    if (mon_opts->flag_output_csv){
                    *ss[i] << "\"" << experiment_id << "\";\"" << net << "\";\""<< sec << "\";\""
                       << elem << "\";\"" << entries[i].id << "\";\"" << timestamp << "\";"  << entries[i].value << endl;

            }
            if (mon_opts->flag_output_es){
                    *ssu[i] << "{\"index\":{\"_index\":\"" << index << "\",\"_type\":\"_doc\"} }" << endl;
                    *ssu[i] << "{\"ExperimentID\":\"" << experiment_id << "\",\"Network\":\""
                       << net << "\",\"Section\":\"" << sec << "\",\"Element\":\""
                       << elem << "\",\"Approximation\":\"" << entries[i].id
                       << "\",\"@timestamp\":\"" << timestamp << "\",\"value\":" << entries[i].value << "}" << endl;
            }
	    if (mon_opts->flag_output_es_via_files){
		     *ss[i] << "\"" << experiment_id << "\";\"" << net << "\";\""<< sec << "\";\""
                       << elem << "\";\"" << entries[i].id << "\";\"" << timestamp << "\";"  << entries[i].value << endl;

                    *ssu[i] << "{\"index\":{\"_index\":\"" << index << "\",\"_type\":\"_doc\"} }" << endl;
                    *ssu[i] << "{\"ExperimentID\":\"" << experiment_id << "\",\"Network\":\""
                       << net << "\",\"Section\":\"" << sec << "\",\"Element\":\""
                       << elem << "\",\"Approximation\":\"" << entries[i].id
                       << "\",\"@timestamp\":\"" << timestamp << "\",\"value\":" << entries[i].value << "}" << endl;
            }
        }
        
        counter++;
        if (counter == mon_opts->buf_size) {
		for (int i=0; i<entries.size(); i++){
		      if (mon_opts->flag_output_csv){
			    //cout << "add ss is:" << ss[i]->str() << endl;
		            *fout[i] << ss[i]->str();
			    fout[i]->flush();
		     }
		      if (mon_opts->flag_output_es){      
			    //cout << "add ss is:" << ss[i]->str() << endl;
		            char json_msg[ssu[i]->str().length()+1];
			    strcpy (json_msg, ssu[i]->str().c_str());
			    publish_json(bulk, json_msg);
		     }
		      if (mon_opts->flag_output_es_via_files){
			    //cout << "add ss is:" << ssu[i]->str() << endl;
		            *fout[i] << ss[i]->str();
			    fout[i]->flush();
			    char json_msg[ssu[i]->str().length()+1];
			    strcpy (json_msg, ssu[i]->str().c_str());
			    publish_json(bulk, json_msg);
		     }
	    	     ss[i]->str("");
		     ssu[i]->str("");
	    	     counter = 0;
		}
	}	
    }
    
    template<typename T>
    void add_entry(string net, string sec, string elem, float timestamp, vector<Entry_to_save<T>>& entries)
    {
	add_entry<T>(net, sec, elem, to_string(timestamp), entries);
    }

 void data_flush()
    {
	for (int i=0; i<filenr; i++){
		
		 if (mon_opts->flag_output_es){
		    if (counter != 0){
			      	char json_msg[ssu[i]->str().length()+1];						
			      	strcpy (json_msg, ssu[i]->str().c_str());
			      	publish_json(bulk, json_msg);
		    }	
		} 		
		 if (mon_opts->flag_output_csv){
			    //cout << "flush ss is:" << ss[i]->str() << endl;
		            *fout[i] << ss[i]->str() << "";
			    fout[i]->flush();
		} 
		 if (mon_opts->flag_output_es_via_files){
		            *fout[i] << ss[i]->str();
			    fout[i]->flush();
			    if (counter != 0){
			    	//cout << "flush ss is:" << ssu[i]->str() << endl;
			    	char json_msg[ssu[i]->str().length()+1];						
			    	strcpy (json_msg, ssu[i]->str().c_str());
			    	publish_json(bulk, json_msg);
		    	    }	
		} 
	ss[i]->str("");
	ssu[i]->str("");
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
						day++;
                                        }
                                }
                 }
        }
};
     
#endif // MONITORING_H
