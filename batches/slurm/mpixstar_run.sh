#!/bin/bash
#SBATCH -p general            # Partition to submit to
#SBATCH -J mpixstar           # Jon name
#SBATCH -n 8                  # Number of cores
#SBATCH -N 1                  # 1 if under 32 cores; >= 2 machines otherwise
#SBATCH -t 0-24:00            # Runtime in minutes
#SBATCH --contiguous          # Ensure that all of the cores are on the same Infiniband network
#SBATCH --mem-per-cpu=4000    # Memory per cpu in MB (see also --mem)
#SBATCH -o mpixstar_%j.out    # File to which STDOUT will be written
#SBATCH -e mpixstar_%j.err    # File to which STDERR will be written
#SBATCH --mail-type=BEGIN,END,FAIL  # Type of email notification- BEGIN,END,FAIL,ALL
#SBATCH --mail-user=youremail@mit.edu # Email to which notifications will be sent
#
module load gcc/4.8.2-fasrc01 openmpi/1.8.1-fasrc01
# 
mpirun --mca mpi_warn_on_fork 0 -np $SLURM_NTASKS ./mpixstar cfrac=0.4 \
temperature=100. lcpres=0 pressure=0.03 spectrum='pow' spectun=0 trad=-1. \
density=1.0e+12 densitytyp=0 columnsof=1.0e+21 columntyp=2 \
column=1.e+23 columnnst=5 columnint=1 rlrad38=1.e+6 rlogxityp=2 \
rlogxisof=0 rlogxi=4 rlogxinst=5 rlogxiint=0 \
habund=1 heabund=1 liabund=0 beabund=0 babund=0 cabund=1 \
nabund=1 oabund=1 fabund=0 neabund=1 naabund=0 mgabund=1 \
alabund=1 siabund=1 pabund=0 sabund=1 clabund=0 arabund=1 \
kabund=0 caabund=1 scabund=0 tiabund=0 vabund=0 crabund=1 \
mnabund=0 feabund=1 coabund=0 niabund=1 cuabund=0 znabund=0 \
modelname='xstar_pg1211' nsteps=10 niter=99 \
lwrite=0 lprint=1 lstep=0 emult=0.5 taumax=5. radexp=0. \
xeemin=0.1 critf=1.e-6 vturbi=100. npass=1 ncn2=9999
#
echo = `date` job $JOB_NAME done.
echo "Program mpixstar finished with exit code $? at: `date`" >> mpixstar.info
