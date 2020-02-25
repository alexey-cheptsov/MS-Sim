#!/bin/bash

# checking the parameters
if [$# -ne 1]; 
    then echo "Usage: ./visualize_Q_P.sh P.csv Q.csv"
fi

export P_INPUT=$1
export Q_INPUT=$2

gnuplot visualize_Q_P.gp -p