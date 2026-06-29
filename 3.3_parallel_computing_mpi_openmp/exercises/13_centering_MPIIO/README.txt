Centering a matrix from file to file
====================================

Centering of a matrix is important for many analysis activities, for example in bioinformatics.

The procedure for computing it is quite simple;
here is a Python example from scikit-bio:
    row_means = input.mean(axis=1, keepdims=True)
    col_means = input.mean(axis=0, keepdims=True)
    matrix_mean = input.mean()
    output = input - row_means - col_means + matrix_mean

But how do you compute it, if the matrix is too large to fit on a single node?

Splitting the problem over multiple nodes is obviously an option.
It is however not a pleasantly parallel application, as it needs global information in the final step,
and MPI can obviously help here.

Using MPI I/O
-------------

In this exercise we will use MPI I/O to read a large matrix from disk,
we center it in memory,
and then use MPI I/O to write the resulting matrix back on disk.

At no point we need the whole matrix in memory.
(as we assume we don't have enough memory for that)

There are two variants of the same program, one in C++ and one in Fortran:
centering.cpp
centering.f90
Use the one that you are more comfortable with.
(Optionally, look at both of them)

The MPI I/O part is already implemented, so you should just try to understand what it does.

Your task is to implement the
center_matrix
subroutine itself, using the necessary MPI functions.
(Look for TODO in the code)

Note that The code already contains the
center_matrix_full
subroutine that implements the single-process logic.
Use that as a starting point to make the other subroutine MPI-capable.

Optional but recommended:
Add openMP in-process paralllelization, too.

Build and run
-------------

The instructions are similar to the other exercises.
just one additional note:
  We will be using very large files, so you should use the lustre storage.


Once you have done the necessary code changes:
# get an interactive allocation and setup the environment
salloc --partition=<given_partition> --res=<reservation> --account=<given_account> --nodes=2 --mem=256G -n 4 -c 16 -t 00:30:00
module load gcc/10.2.0 openmpi/4.1.3 python/3.8.12 py-numpy/1.20.3

# Build the executable
mpic++ -O3 -fopenmp -o centering centering.cpp
or
mpifort -O3 -fopenmp -o centering centering.f90

# Create the input file (only needed once)
python3 create_in.py in.bin 14400

# Run the centering execitable
srun -n 4 ./centering in.bin out.bin

# Validate the result
# Note: You could also try with a different number of MPI jobs (but check allocation)
srun -n 4 ./centering out.bin out2.bin

Note:
out.bin should be very different than in.bin.
But, out2.bin should be (almost) identical to out.bin (apart from minor rounding errors).

Centering an already centered matrix is a no-op.
The initial input file will be generated using random numbers, so it is unlikely to be already centered.



