Simple AXPY example
===================

This simple C program computes AXPY of a large buffer several times.
There is no explicit parallelization in the provided code.

Build and test setup
--------------------

Get an MPI allocation:
salloc --partition=<given_partition> --res=<reservation> --account=<given_account> --nodes=1 --mem=32G -n 8 -c 4 -t 00:30:00

(Note that you are still no the login node)

You can check which node the MPI executable will run with
srun -n 1 hostname

Then load the correct compiler and MPI
module load gcc/10.2.0 openmpi/4.1.3

Compile instructions:
---------------------------
# Build
mpic++ -O3 -o axpy axpy.c -lm
# Run
srun -n 1 ./axpy

(Note that the process will run on a remote node)

Feel free to experiment with other optimization options, if you want.


Make it parallel with MPI
---------------------------

It is left as an exercise to modify the code to use MPI parallelism.

The correct place to put it is in main.
There are comments in the code that will guide you on what needs to be done.

No changes to how you compile, since we already used the mpi compiler wrapper above.

To use more than one process, change the n argument, e.g.
srun -n 8 ./axpy

The result should be the same as the single-process version.

Use multiple nodes
------------------

Above you were still running on a single node, not unlike OpenMP.
But that is not necessary.

(After relinquishing any existing allocations, e.g. by typing
exit
)
Request an allocation spanning 2 nodes:
salloc --partition=<given_partition> --res=<reservation> --account=<given_account> --nodes=1 --mem=32G -n 8 -c 4 -t 00:30:00

You can check which nodes the MPI executable will run with
srun -n 8 hostname

Then run the MPI-enabled axpy again:
srun -n 8 ./axpy

How does the runtime compare?


Optional exercise
-----------------

Use both OpenMP and MPI in the code.


