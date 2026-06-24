! Benchmark example of Fusion plasma collisional step
! All real work happens in the subroutine

program bench_collision
    use, intrinsic :: iso_fortran_env, only: int64
    implicit none

    ! --- Parameters and Declarations ---
    integer :: nv_loc, nt, nc_loc, n_sim, nv, vcount, n_iter
    integer :: i, j, k, l, m
    integer(int64) :: t1, t2, clock_rate
    character(len=20) :: arg
    complex, allocatable :: cap_h_v(:,:,:,:)
    real, allocatable :: cmat(:,:,:,:)
    complex, allocatable :: fsendf(:,:,:,:)
    ! random handling
    integer :: n, seed_value
    integer, allocatable :: seed_array(:)


    ! --- Input Parsing ---
    ! Expecting: ./bench nv_loc nt nc_loc n_sim vcount n_iter
    if (command_argument_count() < 6) then
        print *, "Usage: ./bench <nv_loc> <nt> <nc_loc> <n_sim> <vcount> <n_iter>"
        stop
    end if

    call get_command_argument(1, arg); read(arg, *) nv_loc
    call get_command_argument(2, arg); read(arg, *) nt
    call get_command_argument(3, arg); read(arg, *) nc_loc
    call get_command_argument(4, arg); read(arg, *) n_sim
    call get_command_argument(5, arg); read(arg, *) vcount
    call get_command_argument(6, arg); read(arg, *) n_iter

    nv = nv_loc * vcount

    allocate(cap_h_v(nc_loc, nt, nv, n_sim))
    allocate(cmat(nv, nv, nc_loc, nt))
    allocate(fsendf(nv_loc, nt, nc_loc, vcount * n_sim))

    ! --- Initialization ---
    call random_seed(size = n)
    allocate(seed_array(n))
    seed_value = 123456789 
    seed_array = seed_value
    call random_seed(put = seed_array)

    call random_number(cap_h_v%re)
    call random_number(cap_h_v%im)
    call random_number(cmat)
    fsendf(1,1,1,1) = 0.0

    ! --- Timing Loop ---
    call system_clock(count_rate=clock_rate)
    call system_clock(count=t1)

    do i = 1, n_iter
       ! minor perturbation to avoid over-optimization
       cmat(2,3,1,1) = cmat(2,3,1,1) + real(fsendf(1,1,1,1))
       call calc_collision(nv_loc, nt, nc_loc, n_sim, nv, vcount, &
                           cap_h_v, cmat, fsendf)
    end do

    call system_clock(count=t2)

    ! --- Results ---
    print *, "Total time for ", n_iter, " iterations: ", &
                 real(t2 - t1, 8) / real(clock_rate, 8), " seconds"
    print *, "Result ", fsendf(1,1,1,1)

!
! Acknowledgement: The benchmark harness was created by Gemini,
!                  and improved by hand.
!

contains

! Fusion plasma collision step, using pre-computed Sugama matrix
!
! Student TODO: Add OpenMP parallelization in it
! Hint: Remember that Fortran does not have loop scoping, so use OpenMP for that
subroutine calc_collision( &
                nv_loc,nt,nc_loc,n_sim,nv,vcount,&
                cap_h_v,cmat, &
                fsendf)

  ! --------------------------------------------------
  implicit none
  !
  integer, intent(in) :: nv_loc,nt,nc_loc,n_sim,nv,vcount
  complex, dimension(nc_loc,nt,nv,n_sim), intent(in) :: cap_h_v
  real, dimension(nv,nv,nc_loc,nt), intent(in) :: cmat
  complex, dimension(nv_loc,nt,nc_loc,vcount*n_sim), intent(out) :: fsendf

  !
  integer :: ic_loc,iv,ivp,j,k,itor,ism
  complex, dimension(:), allocatable :: bvec,cvec
  real :: cvec_re,cvec_im
  real :: cval
  ! --------------------------------------------------

  allocate(bvec(nv))
  allocate(cvec(nv))

  do itor=1,nt
   do ic_loc=1,nc_loc
    do ism=1,n_sim
     ! local transpose
     do iv=1,nv
        cvec(iv) = cap_h_v(ic_loc,itor,iv,ism)
     enddo
     ! compute local values
     bvec(:) = (0.0,0.0)
     do ivp=1,nv
        cvec_re = real(cvec(ivp))
        cvec_im = aimag(cvec(ivp))
        do iv=1,nv
           cval = cmat(iv,ivp,ic_loc,itor)
           bvec(iv) = bvec(iv)+ cmplx(cval*cvec_re, cval*cvec_im)
        enddo
     enddo
     ! transpose and save results
     do k=1,vcount
       do j=1,nv_loc ! == nv_loc
          fsendf(j,itor,ic_loc,k+(ism-1)*vcount) = bvec(j+(k-1)*nv_loc)
       enddo
     enddo
    enddo ! ism
   enddo ! ic_loc
  enddo ! itor

 deallocate(bvec)
 deallocate(cvec)
end subroutine

end program
