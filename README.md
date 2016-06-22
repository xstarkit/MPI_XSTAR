# MPI_XSTAR
MPI-based parallelization of XSTAR

Website: http://hea-www.cfa.harvard.edu/~adanehka/mpi_xstar/

MPI_XSTAR: A parallel execution of multiple XSTAR runs using 
Message Passing Interface (MPI). XSTAR is part of the HEASARC's 
standard HEADAS package, and is computer program used to for
calculating the physical conditions and emission spectra of 
photoionized gases (Kallman & Bautista 2001).
 
The master program (rank=0) runs xstinitable from the HEADAS to 
create a list of XSTAR commands for given physical parameters, 
so called joblist, as well as a xstinitable.fits file, which is 
necessary for producing xout_ain.fits, xout_aout.fits, and 
xout_mtable.fits. The joblist is used to create directories in 
ascending order, where each individual xstar is spawned on each 
processor and outputs are saved. When each processor spawns the 
xstar, the main thread is waited until the xstar execution is 
completed.
 
The master program (rank=0) then invokes xstar2table from the 
HEADAS upon the contents of each directory in order to produce 
table model files, namely xout_ain.fits, xout_aout.fits, and 
xout_mtable.fits, suitable for spectroscopy analysis tools such 
as ISIS or XSPEC.

1. Pre-compile Step:

 Set OpenMPI Path (bash):
    export PATH="/../openmpi/bin:$PATH"
    export LD_LIBRARY_PATH=/../openmpi/lib:$LD_LIBRARY_PATH

 Set OpenMPI Path (cshrc):
    setenv PATH "/../openmpi/bin:$PATH"
    setenv LD_LIBRARY_PATH /../openmpi/lib:$LD_LIBRARY_PATH

2. How to compile and link:
     > make

 How to clean:
     > make  clean

3. Set HEADAS Paths:
 Initialize the HEADAS and AtomDB (bash):
    export HEADAS=/../heasoft/x86_64-unknown-linux-gnu-libc2.12
    source $HEADAS/headas-init.sh
    export ATOMDB=/../atomdb/atomdb_v3.0.2

 Initialize the HEADAS and AtomDB (cshrc):
    setenv HEADAS /../heasoft/x86_64-unknown-linux-gnu-libc2.12
    source $HEADAS/headas-init.sh
    setenv ATOMDB /../atomdb/atomdb_v3.0.2

4. How to use mpirun for running mpixstar:
    mpirun -np number_of_processor ... e.g.

    mpirun --mca mpi_warn_on_fork 0 -np 8 ./mpixstar cfrac=0.4 \

    temperature=100. lcpres=0 pressure=0.03 spectrum='pow' \

    spectun=0 trad=-1. density=1.0e+12 densitytyp=0 \

    columnsof=1.0e+20 columntyp=2 column=1.e+24 columnnst=9 columnint=1 \

    rlrad38=1.e+6 rlogxityp=2 rlogxisof=0 rlogxi=5 rlogxinst=6 rlogxiint=0 \

    habund=1 heabund=1 liabund=0 beabund=0 babund=0 cabund=1 \

    nabund=1 oabund=1 fabund=0 neabund=1 naabund=0 mgabund=1 \

    alabund=1 siabund=1 pabund=0 sabund=1 clabund=0 arabund=1 \

    kabund=0 caabund=1 scabund=0 tiabund=0 vabund=0 crabund=1 \

    mnabund=0 feabund=1 coabund=0 niabund=1 cuabund=0 znabund=0 \

    modelname='xstar_pg1211' nsteps=10 niter=99 \

    lwrite=0 lprint=1 lstep=0 emult=0.5 taumax=5. radexp=0. \

    xeemin=0.1 critf=1.e-6 vturbi=100. npass=1 ncn2=9999 > mpixstar.log &

How to run mpirun on clusters and supercomputers:
In the batches folder, a sample of running batch files are included 
for different job scheduling systems. 

To submit a batch file:

+ Portable Batch System (PBS)
     > qsub mpixstar_run.job

+ Sun Grid Engine (SGE)
     > qsub mpixstar_run.job

+ Simple Linux Utility for Resource Management (SLURM)
     > sbatch mpixstar_run.sh
