program centering
    use mpi
    use iso_c_binding
    implicit none

    integer :: rank, size, ierr, num_args
    character(len=256) :: input_filename, output_filename
    
    ! 4-byte integers for dimensions to match uint32_t
    integer(kind=c_int32_t) :: dims(2) 
    integer(kind=c_int32_t) :: total_rows, cols, local_rows, remainder, start_row
    
    integer(kind=MPI_OFFSET_KIND) :: header_size, read_offset, write_offset
    integer :: in_file, out_file
    integer :: status(MPI_STATUS_SIZE)
    
    ! Dynamic 2D arrays for the matrix chunks
    real(kind=c_double), allocatable :: input_matrix(:,:)
    real(kind=c_double), allocatable :: output_matrix(:,:)
    integer(kind=c_long) :: local_elements

    ! Initialize MPI environment
    call MPI_Init(ierr)
    call MPI_Comm_rank(MPI_COMM_WORLD, rank, ierr)
    call MPI_Comm_size(MPI_COMM_WORLD, size, ierr)

    ! Check command line arguments
    num_args = command_argument_count()
    if (num_args < 2) then
        if (rank == 0) then
            print *, "Usage: matrix_processor <input_file> <output_file>"
        end if
        call MPI_Finalize(ierr)
        stop
    end if

    call get_command_argument(1, input_filename)
    call get_command_argument(2, output_filename)

    header_size = 2 * 4 ! 2 * sizeof(uint32_t) = 8 bytes

    ! ==========================================
    ! 1. READ MATRIX VIA MPI-IO

    ! Assume the file contains the 2 dimentsions as uin32_t, followed by matrix of type double

    call MPI_File_open(MPI_COMM_WORLD, trim(input_filename), MPI_MODE_RDONLY, &
                       MPI_INFO_NULL, in_file, ierr)
    if (ierr /= MPI_SUCCESS) then
        if (rank == 0) print *, "Error: Could not open input file ", trim(input_filename)
        call MPI_Finalize(ierr)
        stop
    end if

    ! Rank 0 reads dimensions, then broadcasts to all
    !  Note: All processes could read from the file,
    !        But given how small the information is, this is likely faster
    if (rank == 0) then
        call MPI_File_read_at(in_file, 0_MPI_OFFSET_KIND, dims, 2, MPI_INTEGER4, status, ierr)
    end if
    call MPI_Bcast(dims, 2, MPI_INTEGER4, 0, MPI_COMM_WORLD, ierr)
    if (ierr /= MPI_SUCCESS) then
        call MPI_Finalize(ierr)
        stop
    end if

    total_rows = dims(1)
    cols = dims(2)

    if (total_rows /= cols) then
        if (rank == 0) print *, "Error: Matrix not square ", total_rows, cols
        call MPI_Finalize(ierr)
        stop
    end if

    ! Row-wise problem splitting
    local_rows = total_rows / size
    start_row = rank * local_rows
    remainder = mod(total_rows, size)
    if (remainder /= 0) then
        if (rank == 0) print *, "Error: Would not evenly split rows ", total_rows
        call MPI_Finalize(ierr)
        stop
    end if
    

    ! Allocate matrices using (cols, rows) layout to natively map row-major files
    allocate(input_matrix(cols, local_rows))
    allocate(output_matrix(cols, local_rows))
    local_elements = int(cols, c_long) * int(local_rows, c_long)

    ! Calculate reading file offset
    read_offset = header_size + int(start_row, MPI_OFFSET_KIND) * int(cols, MPI_OFFSET_KIND) * 8_MPI_OFFSET_KIND

    ! Collective read operation
    call MPI_File_read_at_all(in_file, read_offset, input_matrix, int(local_elements), &
                             MPI_DOUBLE_PRECISION, status, ierr)
    if (ierr /= MPI_SUCCESS) then
        call MPI_Finalize(ierr)
        stop
    end if
    call MPI_File_close(in_file, ierr)

    ! ==========================================
    ! 2. EXTERNAL COMPUTATION
    call center_matrix(local_rows, cols, input_matrix, output_matrix)

    ! ==========================================
    ! 3. WRITE MATRIX VIA MPI-IO
    ! ior acts as a bitwise OR for file opening flags
    call MPI_File_open(MPI_COMM_WORLD, trim(output_filename), ior(MPI_MODE_CREATE, MPI_MODE_WRONLY), &
                       MPI_INFO_NULL, out_file, ierr)
    if (ierr /= MPI_SUCCESS) then
        if (rank == 0) print *, "Error: Could not open output file ", trim(output_filename)
        call MPI_Finalize(ierr)
        stop
    end if

    ! Rank 0 writes the 8-byte metadata header
    if (rank == 0) then
        call MPI_File_write_at(out_file, 0_MPI_OFFSET_KIND, dims, 2, MPI_INTEGER4, status, ierr)
    end if

    ! Calculate writing file offset (matches reading geometry)
    write_offset = header_size + int(start_row, MPI_OFFSET_KIND) * int(cols, MPI_OFFSET_KIND) * 8_MPI_OFFSET_KIND

    ! Collective write operation
    call MPI_File_write_at_all(out_file, write_offset, output_matrix, int(local_elements), &
                              MPI_DOUBLE_PRECISION, status, ierr)
    call MPI_File_close(out_file, ierr)

    ! Clean up memory and environment
    deallocate(input_matrix, output_matrix)
    call MPI_Finalize(ierr)

contains

! center a subset of the matrix
! MPI used for gathering necessary info from other processes
subroutine center_matrix(local_rows, cols, input, output)
    use mpi
    use iso_c_binding
    implicit none
    
    integer(c_int32_t), value :: local_rows, cols
    real(c_double), intent(in) :: input(cols, local_rows)
    real(c_double), intent(out) :: output(cols, local_rows)

    ! TODO: Implement this function, based on the single-process example below
    ! Hint: MPI_COMM_WORLD is avaialble, like all the MPI functions

end subroutine center_matrix

! center a whole matrix
subroutine center_matrix_full(rows, cols, input, output)
    use iso_c_binding
    implicit none
    
    ! Note: rows==cols in the full version, but will not be the case in the partitioned version
    integer(c_int32_t), value :: rows, cols
    real(c_double), intent(in) :: input(cols, rows)
    real(c_double), intent(out) :: output(cols, rows)

    ! Local variables
    real(c_double), allocatable :: row_sums(:)
    real(c_double), allocatable :: col_sums(:)
    real(c_double) :: total_sum
    real(c_double), allocatable :: row_means(:)
    real(c_double), allocatable :: col_means(:)
    real(c_double) :: matrix_mean
    real(c_double) :: val
    integer :: i, j

    ! Allocate and initialize sums
    allocate(row_sums(rows))
    allocate(col_sums(cols))
    row_sums = 0.0
    col_sums = 0.0
    total_sum = 0.0

    ! Step 1: Accumulate sums for rows, columns, and the entire matrix
    do i = 1, rows
        do j = 1, cols
            val = input(j, i)
            row_sums(i) = row_sums(i) + val
            col_sums(j) = col_sums(j) + val
            total_sum = total_sum + val
        end do
    end do

    ! Step 2: Calculate the means
    ! Hint1; In the partitioned version, first collect the sums, before dividing
    ! Hint2: All the processes need the resulting information, consider collectives

    allocate(row_means(rows))
    allocate(col_means(cols))
    do i = 1, rows
        row_means(i) = row_sums(i) / cols
    end do

    do j = 1, cols
        col_means(j) = col_sums(j) / rows
    end do

    ! since rows and cols may be large, convert to double
    matrix_mean = total_sum / (real(rows, c_double) * real(cols, c_double))
    
    ! Step 3: Broadcast the subtraction and addition into the final matrix
    do i = 1, rows
        do j = 1, cols
            output(j, i) = input(j, i) - row_means(i) - col_means(j) + matrix_mean
        end do
    end do

    ! Clean up allocated temporary memory
    deallocate(row_sums)
    deallocate(col_sums)
    deallocate(row_means)
    deallocate(col_means)

end subroutine center_matrix_full

! Acknowlegment: Basic skeleton generated with Gemini. Refined by hand.

end program centering
