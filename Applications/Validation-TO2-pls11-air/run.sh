export LD_LIBRARY_PATH=$OPEN_MPI_PATH/lib:$LD_LIBRARY_PATH
export PATH=$OPEN_MPI_PATH/bin:$PATH

mkdir -p output
mpirun -np 92 --oversubscribe ./test