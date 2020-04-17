KIBANA_PATH="/home/alex/Tools/Elasticsearch/kibana-7.3.1-linux-x86_64/bin"

if [ ! -f kibana.pid ]; then
    echo "Kibana is not running. Nothing to stop"
    exit 2
fi

PID=$(cat kibana.pid)
kill ${PID}
rm -f kibana.pid;

echo "Kibana has been stopped."