#!/bin/bash
# A sim starter

DDIR=map/dt0.1
EXEC=tumblera

for i in $(seq 0 10)
do
    alpha=$(echo "scale=2; 0.1*$i" | bc)
    for k in $(seq 0 4)
    do
        for j in $(seq 0 4)
        do
            seed=$(echo "$k*4+$j" | bc)
            nohup ./$EXEC $seed $alpha $DDIR/alpha$alpha-$j.dat &
        done
        wait
    done
done
