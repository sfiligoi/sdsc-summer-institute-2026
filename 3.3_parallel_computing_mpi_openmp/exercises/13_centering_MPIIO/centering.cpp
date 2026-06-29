#include <iostream>
#include <vector>
#include <cstdint>
#include <cstdint>
#include <algorithm>
#include <mpi.h>

// center a whole matrix
void center_matrix_full(uint32_t rows, uint32_t cols, const double* input, double* output) {
    // Note: rows==cols in the full version, but will not be the case in the partitioned version
    std::vector<double> row_sums(rows, 0.0);
    std::vector<double> col_sums(cols, 0.0);
    double total_sum = 0.0;

    // Step 1: Accumulate sums for rows, columns, and the entire matrix
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            // Since C++ does not have native matrix type, explicitly compute index
            double val = input[i*cols+j];
            row_sums[i] += val;
            col_sums[j] += val;
            total_sum += val;
        }
    }

    // Step 2: Calculate the means
    // Hint1; In the partitioned version, first collect the sums, before dividing
    // Hint2: All the processes need the resulting information, consider collectives

    std::vector<double> row_means(rows);
    for (size_t i = 0; i < rows; ++i) {
        row_means[i] = row_sums[i] / cols;
    }

    std::vector<double> col_means(cols);
    for (size_t j = 0; j < cols; ++j) {
        col_means[j] = col_sums[j] / rows;
    }

    // since rows and cols may be large, convert to double
    double matrix_mean = total_sum / (double(rows) * double(cols));
    
    // Step 3: Broadcast the subtraction and addition into the final matrix
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            // Since C++ does not have native matrix type, explicitly compute index
            output[i*cols+j] = input[i*cols+j] - row_means[i] - col_means[j] + matrix_mean;
        }
    }
}

// center a subset of the matrix
// MPI used for gathering necessary info from other processes
void center_matrix(uint32_t local_rows, uint32_t cols, const double* input, double* output) {
    // TODO: Implement this function, based on the single-process example above
    // Hint: MPI_COMM_WORLD is global, like all the MPI functions

}

int main(int argc, char* argv[]) {
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Ensure proper command-line arguments
    if (argc < 3) {
        if (rank == 0) {
            std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
        }
        MPI_Finalize();
        return 1;
    }

    const char* input_filename = argv[1];
    const char* output_filename = argv[2];

    // Array to store matrix dimensions: [0] = rows, [1] = cols
    uint32_t dims[2] = {0, 0}; 
    
    // Header offset: 2 * sizeof(uint32_t) = 8 bytes
    const MPI_Offset header_size = 2 * sizeof(uint32_t);

    // ==========================================
    // 1. READ MATRIX VIA MPI-IO
    // ==========================================

    // Assume the file contains the 2 dimentsions as uin32_t, followed by matrix of type double

    MPI_File in_file;
    int err = MPI_File_open(MPI_COMM_WORLD, input_filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &in_file);
    if (err != MPI_SUCCESS) {
        if (rank == 0) std::cerr << "Error: Could not open input file " << input_filename << "\n";
        MPI_Finalize();
        return 1;
    }

    // Rank 0 reads the dimensions and broadcasts them to all processes
    // Note: All processes could read from the file,
    //       But given how small the information is, this is likely faster
    if (rank == 0) {
        MPI_Status status;
        MPI_File_read_at(in_file, 0, dims, 2, MPI_UINT32_T, &status);
    }
    err = MPI_Bcast(dims, 2, MPI_UINT32_T, 0, MPI_COMM_WORLD);
    if (err != MPI_SUCCESS) {
        MPI_Finalize();
        return 1;
    }

    uint32_t total_rows = dims[0];
    uint32_t cols = dims[1];

    if (total_rows != cols) {
        if (rank == 0) std::cerr << "Error: Matrix not square " << total_rows << " , " << cols << "\n";
        MPI_Finalize();
        return 1;
    }

    // Row-wise problem splitting
    const uint32_t local_rows = total_rows / size;
    const uint32_t start_row = rank * local_rows;
    {
      uint32_t remainder = total_rows % size;
      if (remainder!=0) {
        if (rank == 0) std::cerr << "Error: Could not split" << total_rows << " " << size << " ways!\n";
        MPI_Finalize();
        return 2;
      }
    }
    // Allocate memory for the local chunk of the input matrix
    const size_t local_elements = static_cast<size_t>(local_rows) * cols;
    std::vector<double> input_matrix(local_elements);

    // Calculate file offset for this process's data chunk
    MPI_Offset read_offset = header_size + static_cast<MPI_Offset>(start_row) * cols * sizeof(double);

    // Collective read operation
    MPI_Status read_status;
    err = MPI_File_read_at_all(in_file, read_offset, input_matrix.data(), local_elements, MPI_DOUBLE, &read_status);
    if (err != MPI_SUCCESS) {
        MPI_Finalize();
        return 1;
    }
    MPI_File_close(&in_file);

    // ==========================================
    // 2. We are now ready to center the matrix
    // ==========================================
    // Allocate memory for the output matrix chunk
    std::vector<double> output_matrix(local_elements);

    // Call the actual centering logic
    center_matrix(local_rows, cols, input_matrix.data(), output_matrix.data());

    // ==========================================
    // 3. WRITE MATRIX VIA MPI-IO
    // ==========================================
    MPI_File out_file;
    err = MPI_File_open(MPI_COMM_WORLD, output_filename, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &out_file);
    if (err != MPI_SUCCESS) {
        if (rank == 0) std::cerr << "Error: Could not open output file " << output_filename << "\n";
        MPI_Finalize();
        return 1;
    }

    // Rank 0 writes the 8-byte metadata header
    if (rank == 0) {
        MPI_Status status;
        MPI_File_write_at(out_file, 0, dims, 2, MPI_UINT32_T, &status);
    }

    // Calculate file offset for writing this process's data chunk
    MPI_Offset write_offset = header_size + static_cast<MPI_Offset>(start_row) * cols * sizeof(double);

    // Collective write operation
    MPI_Status write_status;
    MPI_File_write_at_all(out_file, write_offset, output_matrix.data(), local_elements, MPI_DOUBLE, &write_status);
    
    // Sync and close the file
    MPI_File_close(&out_file);

    MPI_Finalize();
    return 0;
}

// Acknowlegment: Basic skeleton generated with Gemini. Refined by hand.

