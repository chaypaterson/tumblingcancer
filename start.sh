#!/bin/sh
# A sim starter

# Run script:

DDIR=data
EXEC=tumblera
KEMT=3.16E-2-speed100

echo "\n"

for i in $(seq 0 4)
do
    seed=$i
    qsub -N kMET-$KEMT-$seed submit.sh $EXEC $seed 0.01 $DDIR/kMET-$KEMT-$seed.csv &
done
