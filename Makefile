#----------------------------------------------------------------
# mpi_xstar: A parallel execution of multiple XSTAR runs using 
# Message Passing Interface (MPI). XSTAR is part of the HEASARC's 
# standard HEADAS package, and is computer program used to for
# calculating the physical conditions and emission spectra of 
# photoionized gases (Kallman & Bautista 2001).
# 
# The master program (rank=0) runs xstinitable from the HEADAS to 
# create a list of XSTAR commands for given physical parameters, 
# so called joblist, as well as a xstinitable.fits file. which are 
# necessary for producing xout_ain.fits, xout_aout.fits, and 
# xout_mtable.fits. The joblist is used to create directories in 
# ascending order, where each individual xstar is spawned on each 
# processor and outputs are saved. When each processor spawns the 
# xstar, the main thread is waited until the xstar execution is 
# completed.
# 
# The master program (rank=0) then invokes xstar2table from the 
# HEADAS upon the contents of each directory in order to produce 
# table modelfiles, namely xout_ain.fits, xout_aout.fits, and 
# xout_mtable.fits, suitable for spectroscopy analysis tools such 
# as ISIS or XSPEC.
#
# How to compile and link:
#     > make
#
# How to clean:
#     > make  clean
#
# Pre-compile Step:
#
# Set OpenMPI Path (bash):
#    export PATH="/../openmpi/bin:$PATH"
#    export LD_LIBRARY_PATH=/../openmpi/lib:$LD_LIBRARY_PATH
#
# Set OpenMPI Path (cshrc):
#    setenv PATH "/../openmpi/bin:$PATH"
#    setenv LD_LIBRARY_PATH /../openmpi/lib:$LD_LIBRARY_PATH
# 
#----------------------------------------------------------------

SOURCE=source/mpixstar.cpp
MYPROGRAM=mpixstar

CC=mpic++

all: $(MYPROGRAM)

$(MYPROGRAM): $(SOURCE)
	$(CC) $(SOURCE) -o$(MYPROGRAM) 
#   	$(CC) -I$(MYINCLUDES) $(SOURCE) -o $(MYPROGRAM) # -l$(MYLIBRARIES)

clean:
	rm -f $(MYPROGRAM)
