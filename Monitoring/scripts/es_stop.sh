ES_PATH="/home/alex/Tools/Elasticsearch/elasticsearch-7.6.1/bin"

if [ ! -f es.pid ]; then
    echo "ES is not running. Nothing to stop"
    exit 2
fi

PID=$(cat es.pid)
kill ${PID}
rm -f es.pid;

echo "ES has been stopped."