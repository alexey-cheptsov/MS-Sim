export LD_LIBRARY_PATH=$OPEN_MPI_PATH/lib:$LD_LIBRARY_PATH
export PATH=$OPEN_MPI_PATH/bin:$PATH

mpic++ -std=c++11 -pthread element_air.cpp ../../communicator.cpp  ../../mpi_communicator.cpp -o element_air -lcurl -Wno-write-strings