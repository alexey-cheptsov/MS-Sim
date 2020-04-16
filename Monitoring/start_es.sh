ES_PATH="/home/alex/Tools/Elasticsearch/elasticsearch-7.6.1/bin"

if [ -f es.pid ]; then
    echo "Cannot run ES. Please ensure it is not running or delete es.pid"
    exit 2
fi

${ES_PATH}/elasticsearch > /dev/null 2>&1 &
pid=$!;
echo "pid of the ES is ${pid}";
touch es.pid
echo ${pid} > es.pid;
