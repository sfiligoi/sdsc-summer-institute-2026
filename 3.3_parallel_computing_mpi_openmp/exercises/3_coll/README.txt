Sample Fusion Plasma Collisional step example
=============================================

The sample program computes the collisional step of a Fusion Plasma simulation severl times.
Both a Fortran and C++ code is provided.
Feel free to try either version, or both.

There is no explicit parallelization in the provided code.

Build and test setup
--------------------

Start an interactive session with
srun --partition=<given_partition> --res=<reservation> --account=<given_account> --pty --nodes=1 --ntasks-per-node=1 --mem=32G -c 24 -t 00:30:00 /bin/bash

(You should now be on one of the worker nodes)


Check what CPU you landed on and which cores are you allowed to use:
lscpu
taskset -pc $$

Then limit yourself to one NUMA domain within that range.

For examples, if your you got
current affinity list: 8-31
You can start a new bash session with
taskset -c 16-31 /bin/bash

Check that it actually worked:
taskset -pc $$

At this point you are all set, and can load the correct compiler
module load gcc/10.2.0

Note that the above module provides both a C++ and a Fotran compiler.


C++ compile instructions:
-------------------------
g++ -march=native -Ofast -o collisions_c collisions.cpp
# Note: The last parameter is the number of repetitions
#       The others determins the problem size
./collisions_c 56 4 576 2 8 5


Fortran compile instructions:
----------------------------
gfortran -march=native -Ofast -o collisions_f collisions.F03
# Note: The last parameter is the number of repetitions
#       The others determins the problem size
./collisions_f 56 4 576 2 8 5


Note: The C++ and Fortran results will be different,
      since we use speudo-random numbers for initialization,
      and the random generator used is different for the two implementations.

Make it parallel with OpenMP
---------------------------

It is left as an exercise to add the OpenMP pragma to the code.
The correct place to put it is in the
calc_collision
function.
(There is a comment in the code, too)


To compile, simply add
-fopenmp


For example
gfortran -march=native -Ofast -fopenmp -o collisions_f collisions.F03

You can then execute the code exactly as you have done before.

The result should not change between the serial and parallel binaries.

Limit parallelism
-----------------

Limit number of cores in use, e.g.
export OMP_NUM_THREADS=3

and re-run the code again.
Compare the run times.

Evaluate impact of crossing NUMA domains
---------------------------------------

Get out of the limited environment you are in, e.g. by typing
exit
(you will still be on the worker node)

Intentionally request cores spanning NUMA domains.

For examples, if your you got
current affinity list: 8-31
You can start a new bash session with
taskset -c 10-25 /bin/bash

Check that it actually worked:
taskset -pc $$

Load again the correct compiler
module load gcc/10.2.0

Run the OpenMP code as above, and compare the runtime, e.g.
./collisions_f 56 4 576 2 8 5

Now use only the subset of the cores on a single NUMA domain, e.g.
taskset -c 16-25 ./collisions_f 56 4 576 2 8 5

