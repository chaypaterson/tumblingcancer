#!/bin/sh

#$ -cwd
#$ -l h_rt=24:00:00,h_vmem=32G

#cd $PBS_O_WORKDIR

#./etc/profile.d/modules.sh

#ulimit -c unlimited

./$@

