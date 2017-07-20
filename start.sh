#!/bin/sh
# A sim starter

# Run script:

DDIR=data/Veffect_kEMT-1e-4_kMET-1e-2
EXEC=tumblera
KEMT=30

echo "\n"

for i in $(seq 0 9)
do
    seed=$i
    qsub -N V-$KEMT-$seed submit.sh $EXEC $seed 0.01 $DDIR/V-$KEMT-$seed.csv &
done
