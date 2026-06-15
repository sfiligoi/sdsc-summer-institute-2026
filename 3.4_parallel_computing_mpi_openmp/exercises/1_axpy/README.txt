Simple AXPY example
===================

This simple C program computes AXPY of a large buffer several times.
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
gcc -o axpy_base axpy.c -lm
# Run
./axpy_base

Use a conservative optimization level:
--------------------------------------
gcc -O2 -o axpy_o2 axpy.c -lm
./axpy_o2


Use a more aggressinve optimization level:
------------------------------------------
gcc -O3 -o axpy_o3 axpy.c -lm
./axpy_o3

Use the maximum possible optimization:
--------------------------------------
gcc -march=native -Ofast -o axpy_fast axpy.c -lm
./axpy_fast


How does the runtime change, if you change buffer size vs iterations
--------------------------------------------------------------------

The default is
1000000 500

./axpy_base 1000000 500
./axpy_fast 1000000 500

Try
./axpy_base 100000 5000
./axpy_fast 100000 5000

and
./axpy_base 50000000 10
./axpy_fast 50000000 10

Make it parallel with OpenMP
---------------------------

It is left as an exercise to add the OpenMP pragma to the code.
The correct place to put it is in the
one_axpy
function.
(There is a comment in the code, too)


To compile, simply add
-fopenmp


For example
gcc -O3 -fopenmp -o axpy_o3 axpy.c -lm

You can then execute the code exactly as you have done before.

