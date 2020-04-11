set -x

#
# 0 Obtain mapping
#
#curl -XGET "localhost:9200/ms/_mapping?pretty"

#
# 1 Simple query
#

#curl -XGET "localhost:9200/ms/_search?pretty" -H "Content-Type: application/json" \
#    -d '{ "size": 100, "from": 0, "query": { "match": {"@timestamp" : "0.150000"} } }'

#http://localhost:9200/[your_index_name]/_search
#{
#  "size": [your value] //default 10
#  "from": [your start index] //default 0
#  "query":
#   {
#    "match_all": {}
#   }
#} 

#
# 2 Query with multiple fields match
#

#curl -XGET "localhost:9200/ms/_search?pretty" -H "Content-Type: application/json" \
#   -d '{ "from": 0, "size": 100, "query": { "bool": { "filter": [ {"match": {"@timestamp" : "0.150000"}} , {"match": {"ExperimentID" : "2020-04-11T11:15:23.000"}}, {"match": {"Element" : "Streb"}} ] } } }'


#
# 3 Query with multiple fields match and values range
#

#curl -XGET "localhost:9200/ms/_search?pretty" -H "Content-Type: application/json" \
#   -d '{ "from": 0, "size": 100, "query": { "bool": { "filter": [ {"match": {"ExperimentID" : "2020-04-10T15:07:37.000"}}, {"match": {"Element" : "Streb"}}, {"range": {"@timestamp" : {"from" : "0.150000", "to" : "0.300000"} }} ] } } }'

#
# 4 Query with multiple fields match and values range and specific field
#

#curl -XGET "localhost:9200/ms/_search?pretty" -H "Content-Type: application/json" \
#   -d '{ "from": 0, "size": 10000, "query": { "bool": { "filter": [ {"match": {"ExperimentID" : "2020-04-10T15:07:37.000"}}, {"match": {"Element" : "Streb"}}, {"range": {"@timestamp" : {"from" : "0.150000", "to" : "0.300000"} }}, {"exists" : {"field" : "q0"}} ] } } }'

# and for real-time

curl -XGET "localhost:9200/ms_realtime/_search?pretty" -H "Content-Type: application/json" \
   -d '{ "from": 0, "size": 10, "query": { "bool": { "filter": [ {"match": {"ExperimentID" : "2020-04-11T19:44:01.000"}}, {"match": {"Element" : "Streb"}}, {"match": {"Approximation" : "q0"}}, {"range": {"@timestamp" : {"gte" : "2020-04-11T20:06:08.350", "lte" : "2020-04-11T20:06:08.500"} }} ] } } }'

#
# 4 Query with multiple fields match and values range and specific field
#

#GET my_inedx/my_type/_search
#{
#    "query" : {
#       "bool": {             //bool indicates we are using boolean operator
#            "must" : [       //must is for **AND**
#                 {
#                   "match" : {
#                         "description" : "some text"  
#                     }
#                 },
#                 {
#                    "match" :{
#                          "type" : "some Type"
#                     }
#                 },
#                 {
#                    "bool" : {          //here its a nested boolean query
#                          "should" : [  //should is for **OR**
#                                 {
#                                   "match" : {
#                                       //ur query
#                                  }
#                                 },
#                                 { 
#                                    "match" : {} 
#                                 }     
#                               ]
#                          }
#                 }
#             ]
#        }
#    }
#}

