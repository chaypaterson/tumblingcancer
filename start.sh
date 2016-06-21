#!/bin/bash
# A sim starter

DDIR=multi
EXEC=tumblera

for i in $(seq 1 8)
do
    nohup ./$EXEC $i > $DDIR/manyballistic_$i.dat &
done
