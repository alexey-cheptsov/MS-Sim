export OPEN_MPI_PATH=/home/alex/Tools/OpenMPI/openmpi-4.0.1/build-gnu-5.4.0
export LD_LIBRARY_PATH=$OPEN_MPI_PATH/lib:$LD_LIBRARY_PATH
export PATH=$OPEN_MPI_PATH/bin:$PATH

mpic++ -std=c++14 -pthread element_gas.cpp ../../communicator.cpp  ../../mpi_communicator.cpp -o element_gas -lcurl -Wno-write-strings