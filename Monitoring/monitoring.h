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
    fstream* fout_1 = nullptr;
    fstream* fout_2 = nullptr;

    char command[512];
    char* bulk = "_bulk?pretty";
    char* post = "_doc?pretty";
    char * json_msg;
    string username;
    string password;
    stringstream ss;
    stringstream ssg;

    int counter = 0;
  

    Monitoring(Monitoring_opts* mon_opts_) {
	mon_opts = mon_opts_;
    
	// FOR EXAMPLE: if ((mon_opts_->flag_output_file) && (mon_opts_->flag_output_file)) 
	experiment_id = mon_opts_->experiment_id;
	if (mon_opts_->flag_output_uri)
		uri = mon_opts_->uri;
    }

    /*Monitoring() {
	    uri = "http://localhost:9200/ms/";
	    username = "admin";
	    password = "admin";

	    experiment_id = "2019-01-01T00:00:00.000";
    };

    Monitoring(string experiment_id_, char* uri_) {
	    uri = uri_;

	    experiment_id = experiment_id_;
    };


    Monitoring(string experiment_id_, char* uri_, string username_, string password_) {
	    uri = uri_;
	    username = username_;
	    password = password_;

	    experiment_id = experiment_id_;
    };

    Monitoring(string experiment_id_, fstream *fout_1_)
    {
            uri = nullptr;
	    fout_1 = fout_1_;
	    fout_2 = nullptr;

	    experiment_id = experiment_id_;
    }

    Monitoring(string experiment_id_, fstream *fout_1_, fstream *fout_2_)
    {
	    experiment_id = experiment_id_;
    }

    Monitoring(string experiment_id_, fstream *fout_1_, char* uri_)
    {
	    fout_1 = fout_1_;
	    fout_2 = nullptr;
	    uri = uri_;

	    experiment_id = experiment_id_;
    }

    Monitoring(string experiment_id_, fstream *fout_1_, fstream *fout_2_, char* uri_)
    {
	    fout_1 = fout_1_;
	    fout_2 = fout_2_;
	    uri = uri_;

	    experiment_id = experiment_id_;
    }*/

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

     
    int prepare_save(string id_str)   
    {	
	 if (id_str == "Q_OS_q7"){
	    char* map_msg = " {\"mappings\": { \"properties\": {  \"P0\": {\"type\": \"float\"},          					\"P1\": {\"type\": \"float\"}, \"Pqmt0\": {\"type\": \"float\"},   					\"Pqmt1\": {\"type\": \"float\"}, \"q0\": {\"type\": \"float\"},     					\"q1\": {\"type\": \"float\"},   \"q2\": {\"type\": \"float\"},     					\"q3\": {\"type\": \"float\"},   \"q4\": {\"type\": \"float\"},   					\"q5\": {\"type\": \"float\"},   \"q6\": {\"type\": \"float\"},   					\"q7\": {\"type\": \"float\"},   \"q8\": {\"type\": \"float\"},   					\"q9\": {\"type\": \"float\"},   \"qm0_gas\": {\"type\": \"float\"},   				\"qm1_gas\": {\"type\": \"float\"},   \"qm2_gas\": {\"type\": \"float\"},   				\"qm0_air\": {\"type\": \"float\"}, \"qm1_air\": {\"type\": \"float\"},   				\"qm2_air\": {\"type\": \"float\"}, \"p0\": {\"type\": \"float\"},  					 \"p1\": {\"type\": \"float\"},   \"p2\": {\"type\": \"float\"},  					 \"p3\": {\"type\": \"float\"},   \"p4\": {\"type\": \"float\"},   					 \"p5\": {\"type\": \"float\"},   \"p6\": {\"type\": \"float\"},   					 \"p7\": {\"type\": \"float\"},   \"p8\": {\"type\": \"float\"},   					 \"p9\": {\"type\": \"float\"},  					   				\"qmt0_gas\": {\"type\": \"float\"}, \"qmt1_gas\": {\"type\": \"float\"},   				\"qmt2_gas\": {\"type\": \"float\"}, \"qmt3_gas\": {\"type\": \"float\"}, 		  		\"qmt4_gas\": {\"type\": \"float\"}, \"qmt5_gas\": {\"type\": \"float\"},   				\"qmt6_gas\": {\"type\": \"float\"}, \"qmt7_gas\": {\"type\":\"float\"}, 				\"qmt8_gas\": {\"type\": \"float\"}, \"qmt9_gas\": {\"type\":\"float\"},	 				\"qmt0_air\": {\"type\": \"float\"}, \"qmt1_air\": {\"type\": \"float\"},   				\"qmt2_air\": {\"type\": \"float\"}, \"qmt3_air\": {\"type\": \"float\"}, 		  		\"qmt4_air\": {\"type\": \"float\"}, \"qmt5_air\": {\"type\": \"float\"},   				\"qmt6_air\": {\"type\": \"float\"}, \"qmt7_air\": {\"type\":\"float\"}, 				\"qmt8_air\": {\"type\": \"float\"}, \"qmt9_air\": {\"type\":\"float\"} }}}";
	    //del(uri); // delete all previous records on the db
	    mapping(uri, map_msg );
	    system("read");
	    }
    }

    char * convert_comand(char * com)
    {
	    strcpy (command, uri);
	    strcat (command, com); 
	    return command;
    }

    template<typename T>
    void add_entry(string net, string sec, string elem, string name, string timestamp, vector<Entry_to_save<T>>& entries)
    {
	for (int i=0; i<entries.size(); i++) {
	    //entries[i].id
	    //entries[i].value
	}
	
    }


    void add_entry(string net, string sec, string elem, string name, string timestamp, float value)
    {
	    if (mon_opts->flag_output_file){
		    ss << "\"" << experiment_id << "\";\"" << net << "\";\""<< sec << "\";\"" 
		       << elem << "\";\"" << timestamp << "\";" << value << endl;

	    }
	    else{
		    ss << "{\"index\":{\"_index\":\"ms\",\"_type\":\"_doc\"} }" << endl;
		    ss << "{\"ExperimentID\":\"" << experiment_id << "\",\"Network\":\"" 
		       << net << "\",\"Section\":\"" << sec << "\",\"Element\":\"" 
		       << elem << "\",\"@timestamp\":\"" << timestamp << "\",\"" << name 
		       << "\":" << value << "}" << endl;	
		    
		    string s = ss.str();
		    json_msg = new char [s.length()+1];
		    strcpy (json_msg, s.c_str());
	    }
	    counter++;
	    
	    if (counter == mon_opts->buf_size){				    
	    	if (mon_opts->flag_output_file){
			*fout_1 << ss.str();
		}
		else{
			if (mon_opts->flag_output_uri){
				publish_json(convert_comand(bulk), json_msg);
			}
		}
	    ss.str("");
	    counter = 0;
	    }    
    }
    
    void add_entry(string net, string sec, string elem, string name, float timestamp, float value)
    {
	    if (mon_opts->flag_output_file){
		    ss << "\"" << experiment_id << "\";\"" << net << "\";\""<< sec << "\";\"" 
		       << elem << "\";" << timestamp << ";" << value << endl;

	    }
	    else{
		    ss << "{\"index\":{\"_index\":\"ms\",\"_type\":\"_doc\"} }" << endl;
		    ss << "{\"ExperimentID\":\"" << experiment_id << "\",\"Network\":\"" 
		       << net << "\",\"Section\":\"" << sec << "\",\"Element\":\"" 
		       << elem << "\",\"@timestamp\":" << timestamp << ",\"" << name 
		       << "\":" << value << "}" << endl;	
		    
		    string s = ss.str();
		    json_msg = new char [s.length()+1];
		    strcpy (json_msg, s.c_str());
	    }
	    counter++;
	    
	    if (counter == mon_opts->buf_size){				    
	    	if (mon_opts->flag_output_file){
			*fout_1 << ss.str();
		}
		else{
			if (mon_opts->flag_output_uri){
				publish_json(convert_comand(bulk), json_msg);
			}
		}
	    ss.str("");
	    counter = 0;
	    }    
    }

    void add_entry(string net, string sec, string elem, string name, string timestamp, float value_1, float value_2)
    {
		if (mon_opts->flag_output_file){

		    ss << "\"" << experiment_id << "\";\"" << net << "\";\""<< sec << "\";\"" 
		       << elem << "\";\"" << timestamp << "\";" << value_1 << endl;
		   ssg << "\"" << experiment_id << "\";\"" << net << "\";\""<< sec << "\";\"" 
		       << elem << "\";\"" << timestamp << "\";" << value_2 << endl;
	    }
	    else{

		    ss << "{\"index\":{\"_index\":\"ms\",\"_type\":\"_doc\"} }" << endl;
		    ss << "{\"ExperimentID\":\"" << experiment_id << "\",\"Network\":\"" 
		       << net << "\",\"Section\":\"" << sec << "\",\"Element\":\"" 
		       << elem << "\",\"@timestamp\":\"" << timestamp << "\",\"" << name 
		       << "_air\":" << value_1 << "}" << endl;	

		    
		    ss << "{\"index\":{\"_index\":\"ms\",\"_type\":\"_doc\"} }" << endl;
		    ss << "{\"ExperimentID\":\"" << experiment_id << "\",\"Network\":\"" 
		       << net << "\",\"Section\":\"" << sec << "\",\"Element\":\"" 
		       << elem << "\",\"@timestamp\":\"" << timestamp << "\",\"" << name 
		       << "_gas\":" << value_2 << "}" << endl;	

		    string s = ss.str();
		    json_msg = new char [s.length()+1];
		    strcpy (json_msg, s.c_str());
		    
	    }
	    counter++;
	    if (counter == mon_opts->buf_size){
		if (mon_opts->flag_output_file){			
			*fout_1 << ss.str();
			*fout_2 << ssg.str();
		}
		else{			
		    if (mon_opts->flag_output_uri){
		    publish_json(convert_comand(bulk), json_msg);
		    }
		}
	    	ss.str("");
	    	ssg.str("");		
	    	counter = 0;
	    }
    }

 void data_flush(string id_str)
    {
	
	int count_lines=0;
	char c;
	char labels[30][100];
	char values[30][100];
	char j_msg[512];
	int fcounter=0;
	int num_label=0;
	int num_char=0;

        if ((mon_opts->flag_output_uri) && (!mon_opts->flag_output_file)) {
	    cout << "msg is:" << json_msg << endl;	
	    publish_json(convert_comand(bulk), json_msg);
	} 
	
	if ((mon_opts->flag_output_uri) && (mon_opts->flag_output_file)) {
	    *fout_1 << ss.str() << endl;
	    
	    string full_path = "./output/";
	    full_path.append(id_str);
	    full_path.append(".csv");
	    ifstream infile(full_path);
	    
	    int finish = 0;
	    while (!finish) {	
        	c = infile.get();
        	
        	if (c == EOF) {	
        	    finish = 1;
        	} else {   
		    if(( fcounter ==0))  {
			if (  (c==';' || c=='\n')) {
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
		
			if( (c!=';') && (c!=10)){
		    	    values[num_label][num_char]=c;
		    	    values[num_label][num_char+1]='\0';
		    	    num_char=num_char+1;
			}
		    
			if(c=='\n') {
			    cout << "were here" << endl;
		    	    sprintf(j_msg,"{");
		    
		    	    for(int i=0;i<num_label;i++) {
				if(i>0) sprintf(j_msg,"%s,",j_msg);
				sprintf(j_msg,"%s\"%s\":%s",j_msg,labels[i],values[i] );
		    	    }
		
		    	    sprintf(j_msg,"%s}\n",j_msg);
	     		    //printf(" msg is %s",j_msg);		
					    
			    if (mon_opts->flag_output_uri)
				publish_json(convert_comand(post), j_msg);
				    
			    num_label=0;
			    count_lines++;
			}
		    }
		}
	    } 
	
	    infile.close();
	}
    }

    /*void data_flush(string id_str)
    {
	
	int count_lines=0;
	int i;
	char c;
	char labels[30][100];
	char values[30][100];
	char j_msg[512];
	int fcounter=0;
	int num_label=0;
	int num_char=0;
	string outp = "./output/";
	fstream infile;
	string filename;
	DIR *dir;
	struct dirent *ent;


	if ((mon_opts->flag_output_uri) && (!mon_opts->flag_output_file)) {	
	    publish_json(convert_comand(bulk), json_msg);
	} 
	
	if ((mon_opts->flag_output_uri) && (mon_opts->flag_output_file)){
	    if (fout_2 == nullptr) {
		*fout_1 << ss.str();	
	    } else {
	        *fout_1 << ss.str();
		*fout_2 << ssg.str();
	    }
	 
	    /*if (id_str == "Q_OS_q7"){
		  if ((dir = opendir ("./output/")) != NULL) {
	    	    while ((ent = readdir (dir)) != NULL) {
			filename = ent->d_name;
			if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".csv"){
			    cout << "filename is"<< filename << endl;
			    outp.append(filename);
			    infile.open(outp);
			    if (!infile) cerr << "Can't open input file!";

			    while ((c = infile.get()) != EOF) {  
				outp = "./output/";	
				if(( fcounter ==0))  {
				    if (  (c==';' || c=='\n')) {
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
				
				    if( (c!=';') && (c!=10)){
					values[num_label][num_char]=c;
					values[num_label][num_char+1]='\0';
					num_char=num_char+1;
				    }
				    
				    if(c=='\n') {
					sprintf(j_msg,"{");
				    
					for(i=0;i<num_label;i++) {
					    if(i>0) sprintf(j_msg,"%s,",j_msg);
					    sprintf(j_msg,"%s\"%s\":%s",j_msg,labels[i],values[i] );
					}
				
					sprintf(j_msg,"%s}\n",j_msg);
			     		printf(" msg is %s",j_msg);		
							    
					if (mon_opts->flag_output_uri)
						publish_json(convert_comand(post), j_msg);
						    
					num_label=0;
					count_lines++;
				    }
				 }
					
			    } 
	    		infile.close();
			fcounter = 0;
			}
	    	    }
		    closedir (dir);
	    	}
	    }   			  
	} 
    }*/
		

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
        milliseconds = td.total_milliseconds() - ((hours * 3600 + minutes * 60 + seconds) * 1000);
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
