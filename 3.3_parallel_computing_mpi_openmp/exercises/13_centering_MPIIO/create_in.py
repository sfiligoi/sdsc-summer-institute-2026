# usage:
#  python3 create_in.py <fname> rows

import sys
import numpy as np

N = int(sys.argv[2])

rng = np.random.default_rng()

# create a new file
with open(sys.argv[1], "wb") as fd:
    # write dimensions
    dims = np.array([N,N], dtype=np.uint32)
    dims.tofile(fd)
    # randomly generate and write one row at a time
    for i in range(N):
        col_data = rng.random(N)
        col_data.tofile(fd)

