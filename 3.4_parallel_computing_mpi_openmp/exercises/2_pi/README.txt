Simple PI example
===================

This simple C program computes PI several times.
There is no explicit parallelization in the provided code.

Build and test setup
--------------------

Start an interactive session with
srun --partition=<given_partition> --res=<reservation> --account=<given_account> --pty --nodes=1 --ntasks-per-node=1 --mem=32G -c 16 -t 00:30:00 /bin/bash

Then load the correct compiler
module load gcc/10.2.0

Optional, but recommended,
Check what CPU you landed on and which cores are you allowed to use:
lscpu
taskset -pc $$


Basic compile instructions:
---------------------------
# Build
gcc -o pi_base pi.c -lm
# Run
./pi_base

Use a conservative optimization level:
--------------------------------------
gcc -O2 -o pi_o2 pi.c -lm
./pi_o2


Use a more aggressinve optimization level:
------------------------------------------
gcc -O3 -o pi_o3 pi.c -lm
./pi_o3

Use the maximum possible optimization:
--------------------------------------
gcc -march=native -Ofast -o pi_fast pi.c -lm
./pi_fast


How does the runtime change, if you change precision and iterations
--------------------------------------------------------------------

The default is
1000000 500

./pi_base 1000000 500 1.e-10
./pi_fast 1000000 500 1.e-10

Try
./pi_base 100000 5000 1.e-10
./pi_fast 100000 5000 1.e-10

and finally
./pi_fast 500000000 10 1.e-10

Make it parallel with OpenMP
---------------------------

It is left as an exercise to add the OpenMP pragma to the code.
The correct place to put it is in the
one_pi
function.
(There is a comment in the code, too)


To compile, simply add
-fopenmp


For example
gcc -march=native -Ofast -fopenmp -o pi_fast pi.c -lm

You can then execute the code exactly as you have done before.

Alternate OpenMP parallelization
--------------------------------

We compute PI many times just for benchmarking purposes, 
so the suggested parallelization point outlined above 
is the right thing to do in this context.

However, other problems may benefit from
a different parallelization strategy.

Try parallelizing the for loop in main, instead.
Compare the runtimes.


