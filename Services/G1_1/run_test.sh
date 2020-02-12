export LD_LIBRARY_PATH=$OPEN_MPI_PATH/lib:$LD_LIBRARY_PATH
export PATH=$OPEN_MPI_PATH/bin:$PATH

mkdir -p output
rm test
mpic++ -std=c++11 -pthread test.cpp ../../communicator.cpp  ../../mpi_communicator.cpp -o test -lcurl -Wno-write-strings
mpirun -np 1 --oversubscribe ./test
