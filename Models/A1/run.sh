export LD_LIBRARY_PATH=$OPEN_MPI_PATH/lib:$LD_LIBRARY_PATH
export PATH=$OPEN_MPI_PATH/bin:$PATH

mkdir output
mpirun -np 22 --oversubscribe ./element_air