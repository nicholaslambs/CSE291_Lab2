#!/bin/bash

N=100

success_count=0
for i in $(seq 1 $N)
do
    # Run the commands and capture the output
    output=$(make clean; make run)

    # echo "$output"

    if echo "$output" | grep -q "Load time for secret_array\[47\]:"
    then
        success_count=$((success_count+1))
    fi
done

echo "The 'Load time' line appeared $success_count times out of $N runs"
