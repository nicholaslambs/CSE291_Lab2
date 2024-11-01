#!/bin/bash

N=100
success_count=0

for i in $(seq 1 $N)
do
    # Navigate to the directory containing the Makefile
    output=$(cd $HOME/lab2; make clean; make run_spectre)

    # Uncomment the next line if you want to debug the output
    echo "$output"

    if echo "$output" | grep -q "Load time for secret_array\[47\]:"
    then
        success_count=$((success_count+1))
    fi
done

echo "The 'Load time' line appeared $success_count times out of $N runs"
