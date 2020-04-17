KIBANA_PATH="/home/alex/Tools/Elasticsearch/kibana-7.3.1-linux-x86_64/bin"

if [ -f kibana.pid ]; then
    echo "Cannot run Kibana. Please ensure it is not running or delete kibana.pid"
    exit 2
fi

${KIBANA_PATH}/kibana > /dev/null 2>&1 &
pid=$!;
echo "pid of the Kibana is ${pid}";
touch kibana.pid
echo ${pid} > kibana.pid;

echo -e "You may now switch to http://localhost:5601"