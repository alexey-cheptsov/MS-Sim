# See more at the link:
# https://www.getargon.io/docs/articles/elasticsearch/unique-values.html

curl -XPOST 'localhost:9200/ms_realtime/_search?pretty' -H 'Content-Type: application/json' -d '{ "size" : "0", "aggs" : {"nr_distinct_indexes" : {"terms" : {"field" : "ExperimentID"}}} }'
echo -e "\n"
