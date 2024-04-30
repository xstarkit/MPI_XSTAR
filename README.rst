=========
MPI_XSTAR
=========

.. image:: https://app.travis-ci.com/xstarkit/MPI_XSTAR.svg?branch=master
    :target: https://app.travis-ci.com/github/xstarkit/MPI_XSTAR
    :alt: Build Status
    
.. image:: http://mybinder.org/badge.svg
    :target: http://mybinder.org/repo/xstarkit/mpi_xstar
    :alt: Binder
    
.. image:: https://img.shields.io/badge/license-GPL-blue.svg
    :target: https://github.com/xstarkit/MPI_XSTAR/blob/master/LICENSE
    :alt: GitHub license
    
.. image:: https://img.shields.io/badge/DOI-10.5281/zenodo.1890561-blue.svg
    :target: https://doi.org/10.5281/zenodo.1890561
    :alt: Zenodo
    
.. image:: https://img.shields.io/badge/ASCL-1712.002-green.svg
    :target: https://ui.adsabs.harvard.edu/abs/2017ascl.soft12002D/abstract
    :alt: ASCL
    
.. image:: https://img.shields.io/badge/DOI-10.1088%2F1538--3873%2Faa9dff-blue.svg
    :target: https://doi.org/10.1088/1538-3873/aa9dff
    :alt: DOI


**MPI-based parallelization of XSTAR photoionization program**

Description
===========

MPI_XSTAR: A parallel execution of multiple `XSTAR <https://heasarc.gsfc.nasa.gov/xstar/xstar.html>`_ runs using `Message Passing Interface <http://www.mpi-forum.org/docs/docs.html>`_ (MPI). XSTAR is part of the `HEASARC's standard HEADAS package <http://heasarc.nasa.gov/lheasoft/>`_, and is a computer program used for calculating the physical conditions and emission spectra of photoionized gases (`Kallman & Bautista 2001 <https://ui.adsabs.harvard.edu/abs/2001ApJS..133..221K/abstract>`_).
 
The master program (rank=0) runs xstinitable from the HEADAS to create a list of XSTAR commands for given physical parameters, so called joblist, as well as a xstinitable.fits file, which is necessary for producing xout_ain.fits, xout_aout.fits, and xout_mtable.fits. The joblist is used to create directories in ascending order, where each individual xstar is spawned on each processor and outputs are saved. When each processor spawns the xstar, the main thread is waited until the xstar execution is completed.
 
The master program (rank=0) then invokes xstar2table from the HEADAS upon the contents of each directory in order to produce table model files, namely xout_ain.fits, xout_aout.fits, and xout_mtable.fits, suitable for spectroscopy analysis tools such as `ISIS <http://space.mit.edu/asc/isis/>`_ or `XSPEC <https://heasarc.gsfc.nasa.gov/xanadu/xspec/>`_.

Installation
============

1: Pre-compile Step:

Set OpenMPI Path (bash):

.. code-block::
 
        export PATH="/../openmpi/bin:$PATH"
        export LD_LIBRARY_PATH=/../openmpi/lib:$LD_LIBRARY_PATH

Set OpenMPI Path (cshrc):

.. code-block::

        setenv PATH "/../openmpi/bin:$PATH"
        setenv LD_LIBRARY_PATH /../openmpi/lib:$LD_LIBRARY_PATH

2: How to compile and link:

.. code-block::

        make

How to clean:

.. code-block::

        make  clean

3: Set HEADAS Paths:
Initialize the HEADAS and AtomDB (bash):

.. code-block::

        export HEADAS=/../heasoft/x86_64-unknown-linux-gnu-libc2.12
        source $HEADAS/headas-init.sh
        export ATOMDB=/../atomdb/atomdb_v3.0.2

Initialize the HEADAS and AtomDB (cshrc):

.. code-block::

        setenv HEADAS /../heasoft/x86_64-unknown-linux-gnu-libc2.12
        source $HEADAS/headas-init.sh
        setenv ATOMDB /../atomdb/atomdb_v3.0.2

*Note:* The XSTAR program does not need AtomDB, and uses its own atomic data. However, AtomDB data are required for working with some plasma models in the `Interactive Spectral Interpretation System (ISIS) <http://space.mit.edu/cxc/isis/>`_.

4: How to use mpirun for running mpixstar:

.. code-block::

        mpirun -np number_of_processor ...

for example:

.. code-block::

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
        xeemin=0.1 critf=1.e-6 vturbi=100. npass=1 ncn2=9999 

5: How to run mpirun on clusters and supercomputers:

In the batches folder, a sample of running batch files are included 
for different job scheduling systems. To submit a batch file:

Portable Batch System (PBS):

.. code-block::

        qsub mpixstar_run.job

Sun Grid Engine (SGE):

.. code-block::

        qsub mpixstar_run.job

Simple Linux Utility for Resource Management (SLURM):

.. code-block::

        sbatch mpixstar_run.sh

Acknowledgement
===============

If you employ **MPI_XSTAR** in your scholarly work, please acknowledge the usage by citing the following reference:
	
* Danehkar, A., Nowak, M. A., Lee, J. C. and Smith, R. K. (2018). MPI_XSTAR: MPI-based Parallelization of the XSTAR Photoionization Program. *Publ. Astron. Soc. Pac.*, **130**, 024501. doi: `10.1088/1538-3873/aa9dff <https://doi.org/10.1088/1538-3873/aa9dff>`_ ads: ` 2018PASP..130b4501D <https://ui.adsabs.harvard.edu/abs/2018PASP..130b4501D/abstract>`_.

.. code-block:: bibtex

   @article{Danehkar2018,
     author = {{Danehkar}, Ashkbiz and {Nowak}, Michael A. and {Lee}, Julia C. and
         {Smith}, Randall K.},
     title = {MPI\_XSTAR: MPI-based Parallelization of the XSTAR Photoionization Program},
     journal = {PASP},
     volume = {130},
     number = {984},
     pages = {024501},
     year = {2018},
     doi = {10.1088/1538-3873/aa9dff}
   }

Learn More
==========

==================  =============================================
**Documentation**   https://xstarkit.github.io/MPI_XSTAR/
**Repository**      https://github.com/xstarkit/MPI_XSTAR
**Issues & Ideas**  https://github.com/xstarkit/MPI_XSTAR/issues
**DOI**             `10.1088/1538-3873/aa9dff <https://doi.org/10.1088/1538-3873/aa9dff>`_
**Archive**         `10.5281/zenodo.1890561 <https://doi.org/10.5281/zenodo.1890561>`_
==================  =============================================
