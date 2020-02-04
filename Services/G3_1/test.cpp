/*
 * Copyright (c) 2019 	   Alexey Cheptsov. HLRS.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/*
 * This is a model of a simple ventilation element with unsteady
 * air distribution with neither leaks nor methan transport
 */

#include "mpi.h"

#include "G3_1.h"
#include "../../mpi_communicator.h"
#include "../../mpi_deployment.h"


using namespace std;
using namespace G3_1;


int main (int argc, char* argv[]) {
    int mpi_rank, mpi_size, provided;
    
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    //
    // parameters of Q-elements
    //
	// OS
    float OS_S = 7.0;   // square cut
    float OS_R = 0.24;  // resistance
    float OS_L = 500;   // length
    	// Streb
    float Streb_S = 2.3;   // square cut
    float Streb_R = 6.61;  // resistance
    float Streb_L = 150;   // length
    	// AM
    float AM_S = 2.3;   // square cut
    float AM_R = 81  ;  // resistance
    float AM_L = 130;   // length
    float AM_A = 1460.0;
    float AM_BRf = 1.71;
    float AM_Qm0 = 0.0175;
	// VS
    float VS_S = 5.5;   // square cut
    float VS_R = 0.44;  // resistance
    float VS_L = 500;   // length
    
    float dX = 50;
    
    // parameters of P-elements
    float H_start = 0;
    float H_end = 242.6;
            
    // numeric parameters
    Solver_Params solv_params = { 0.45        /*time step in s.*/,
                                  0.0001      /*precision*/,
                                  1           /*nr. of numeric steps in sim block*/  };
    
    // Deployment options
    MpiProcessMap mpi_map;
    
    mpi_map.add({"QQmt0",		0});
    
    	// OS
    mpi_map.add({"Q_OS",		1});
    mpi_map.add({"Q_OS_q0",		2});
    mpi_map.add({"Q_OS_q1",		3});
    mpi_map.add({"Q_OS_q2",		4});
    mpi_map.add({"Q_OS_q3",		5});
    mpi_map.add({"Q_OS_q4",		6});    
    mpi_map.add({"Q_OS_q5",		7});    
    mpi_map.add({"Q_OS_q6",		8});    
    mpi_map.add({"Q_OS_q7",		9});    
    mpi_map.add({"Q_OS_q8",		10});    
    mpi_map.add({"Q_OS_q9",		11});    
    mpi_map.add({"Q_OS_p0",		12});
    mpi_map.add({"Q_OS_p1",		13});
    mpi_map.add({"Q_OS_p2",		14});
    mpi_map.add({"Q_OS_p3",		15});
    mpi_map.add({"Q_OS_p4",		16});    
    mpi_map.add({"Q_OS_p5",		17});    
    mpi_map.add({"Q_OS_p6",		18});    
    mpi_map.add({"Q_OS_p7",		19});    
    mpi_map.add({"Q_OS_p8",		20});    
    
	// Streb
    mpi_map.add({"Q_Streb",		21});
    mpi_map.add({"Q_Streb_q0",   	22});
    mpi_map.add({"Q_Streb_q1",		23});
    mpi_map.add({"Q_Streb_q2",		24});
    mpi_map.add({"Q_Streb_p0",		25});
    mpi_map.add({"Q_Streb_p1",		26});

	// AM
    mpi_map.add({"Q_AM",		27});
    mpi_map.add({"Q_AM_qm0",	   	28});
    mpi_map.add({"Q_AM_q0",	   	28});
    mpi_map.add({"Q_AM_qm1",		29});
    mpi_map.add({"Q_AM_q1",		29});
    mpi_map.add({"Q_AM_qm2",		30});
    mpi_map.add({"Q_AM_q2",		30});
    mpi_map.add({"Q_AM_p0",		31});
    mpi_map.add({"Q_AM_p1",		32});

    	// VS
    mpi_map.add({"Q_VS",		33});
    mpi_map.add({"Q_VS_qmt0",		34});
    mpi_map.add({"Q_VS_q0",		34});
    mpi_map.add({"Q_VS_qmt1",		35});
    mpi_map.add({"Q_VS_q1",		35});
    mpi_map.add({"Q_VS_qmt2",		36});
    mpi_map.add({"Q_VS_q2",		36});
    mpi_map.add({"Q_VS_qmt3",		37});
    mpi_map.add({"Q_VS_q3",		37});
    mpi_map.add({"Q_VS_qmt4",		38});    
    mpi_map.add({"Q_VS_q4",		38});    
    mpi_map.add({"Q_VS_qmt5",		39});    
    mpi_map.add({"Q_VS_q5",		39});    
    mpi_map.add({"Q_VS_qmt6",		40});    
    mpi_map.add({"Q_VS_q6",		40});    
    mpi_map.add({"Q_VS_qmt7",		41});
    mpi_map.add({"Q_VS_q7",		41});
    mpi_map.add({"Q_VS_qmt8",		42});    
    mpi_map.add({"Q_VS_q8",		42});    
    mpi_map.add({"Q_VS_qmt9",		43});    
    mpi_map.add({"Q_VS_q9",		43});    
    mpi_map.add({"Q_VS_p0",		44});
    mpi_map.add({"Q_VS_p1",		45});
    mpi_map.add({"Q_VS_p2",		46});
    mpi_map.add({"Q_VS_p3",		47});
    mpi_map.add({"Q_VS_p4",		48});    
    mpi_map.add({"Q_VS_p5",		49});    
    mpi_map.add({"Q_VS_p6",		50});    
    mpi_map.add({"Q_VS_p7",		51});    
    mpi_map.add({"Q_VS_p8",		52});
    
    mpi_map.add({"Pqmt0",		53});
    mpi_map.add({"Pqmt1",		54});


    //mpi_map.add({"G3_1",      		0});
    //mpi_map.add({"P0",			1});
    //mpi_map.add({"P1",			2});

    
    MpiCommunicator* communicator = new MpiCommunicator(mpi_map);
    
    char experiment_id_char[23];
    if (mpi_rank == 0) {
        Time_MS now;
        now.init_time();
        string experiment_id = now.time_stamp();
        cout << "Experiment_id: " << experiment_id << endl;
        strcpy(experiment_id_char, experiment_id.c_str());
    }
    MPI_Bcast(experiment_id_char, 23, MPI_CHAR, 0, MPI_COMM_WORLD);
            
    string experiment_id(experiment_id_char);

    
    Monitoring_opts* mon_opts   = new Monitoring_opts();
    mon_opts->experiment_id     = experiment_id;
    mon_opts->flag_output_file  = 1;
    mon_opts->buf_size          = 10;


    QQmt* QQmt0 = new QQmt(0, "QQmt0", communicator,
	    "Section1" /*section*/, "Network1" /*network*/,
	    mon_opts,
/*OS*/      OS_S, OS_R, OS_L,
/*Streb*/   Streb_S, Streb_R, Streb_L,
/*AM*/      AM_S, AM_R, AM_L, AM_A, AM_BRf,
/*VS*/      VS_S, VS_R, VS_L,
            dX,
            solv_params);

    QQmt0->add_buffer(new LocalIntBuffer(QQmt0->id /*ms_id*/, QQmt0->id_str, 0 /*port*/, QQmt0->communicator));
    QQmt0->add_buffer_value(0, Commands_QQmt::id);
    QQmt0->add_buffer_value(0, Commands_QQmt::stop);
    
    MpiDeploymentPool deployment_pool(mpi_map);
    deployment_pool.add_ms(QQmt0);
    deployment_pool.deploy_all();
    deployment_pool.join_all();
    
    MPI_Finalize();
}