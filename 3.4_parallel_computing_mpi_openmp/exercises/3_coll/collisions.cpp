#include <complex>
#include <vector>
#include <chrono>
#include <random>
#include <iostream>

// Helper class, to mimick a Fortran-stype 4D Array
template<typename TReal>
class Tensor4DBuffer {
private:
  // Fortran order...dim[0] contiguous (row)
  TReal * const data;  // buffer owned by this object
  uint64_t dim3D;
  uint64_t dim2D;
  uint32_t dims[4];

  static constexpr uint64_t Tensor2D_els(uint32_t dim0, uint32_t dim1) {return uint64_t(dim0)*dim1;}
  static constexpr uint64_t Tensor3D_els(uint32_t dim0, uint32_t dim1, uint32_t dim2) {return Tensor2D_els(dim0,dim1)*dim2;}
  static constexpr uint64_t Tensor4D_els(uint32_t dim0, uint32_t dim1, uint32_t dim2, uint32_t dim3) {return Tensor3D_els(dim0,dim1,dim2)*dim3;}

public:
  Tensor4DBuffer() = delete;

  Tensor4DBuffer(uint32_t _dim0, uint32_t _dim1, uint32_t _dim2, uint32_t _dim3)
    : data(new TReal[Tensor4D_els(_dim0,_dim1,_dim2,_dim3)]),
      dim3D(Tensor3D_els(_dim0,_dim1,_dim2)), dim2D(Tensor2D_els(_dim0,_dim1)),
      dims{_dim0,_dim1,_dim2,_dim3} {}

  ~Tensor4DBuffer() { delete[] data; }

  // We do not allow copy and move constructors and assignments, to avoid accidental copies
  Tensor4DBuffer(const Tensor4DBuffer &) = delete;
  Tensor4DBuffer &operator=(const Tensor4DBuffer &) = delete;
  Tensor4DBuffer(Tensor4DBuffer &&other) = delete;
  Tensor4DBuffer &operator=(Tensor4DBuffer &&other) = delete;

  constexpr uint64_t els() const {return dim3D*dims[3];}

  // indexes are 0-based
  constexpr uint64_t get_3Dbuf_idx(uint32_t idx3) const { return dim3D*idx3;}
  constexpr uint64_t get_2Dbuf_idx(uint32_t idx2, uint32_t idx3) const { return get_3Dbuf_idx(idx3)+dim2D*idx2;}
  constexpr uint64_t get_row_idx(uint32_t idx1, uint32_t idx2, uint32_t idx3) const { return get_2Dbuf_idx(idx2, idx3)+uint64_t(dims[0])*idx1;}
  constexpr uint64_t get_el_idx(uint32_t idx0, uint32_t idx1, uint32_t idx2, uint32_t idx3) const { return get_row_idx(idx1,idx2,idx3)+idx0;}

  constexpr TReal       &operator()(uint32_t idx0, uint32_t idx1, uint32_t idx2, uint32_t idx3)       { return data[get_el_idx(idx0,idx1,idx2,idx3)]; }
  constexpr TReal const &operator()(uint32_t idx0, uint32_t idx1, uint32_t idx2, uint32_t idx3) const { return data[get_el_idx(idx0,idx1,idx2,idx3)]; }

  void fill(const TReal val) { uint64_t len=els(); for (uint64_t i=0; i<len; i++) data[i] = val; }
};

// Fusion plasma collision step, using pre-computed Sugama matrix
//
// Student TODO: Add OpenMP parallelization in it
// Hint: Note the work buffers allocated outside the loop
void calc_collision(
    int nv_loc, int nt, int nc_loc, int n_sim, int nv, int vcount,
    const Tensor4DBuffer< std::complex<double> >& cap_h_v,
    const Tensor4DBuffer< double >& cmat,
    Tensor4DBuffer< std::complex<double> >& fsendf) 
{
    // Use std::vector for local buffer allocation, instead of manual allocation
    std::vector< std::complex<double> > bvec_raw(nv);
    std::vector< std::complex<double> > cvec_raw(nv);

    std::complex<double> * const bvec = bvec_raw.data();
    std::complex<double> * const cvec = cvec_raw.data();

    for (int itor = 0; itor < nt; ++itor) {
        for (int ic_loc = 0; ic_loc < nc_loc; ++ic_loc) {
            for (int ism = 0; ism < n_sim; ++ism) {
                
                // 1. Local transpose: Extract from cap_h_v
                for (int iv = 0; iv < nv; ++iv) {
                    cvec[iv] = cap_h_v(ic_loc, itor, iv, ism);
                }

                // 2. Compute local values
                for (int iv = 0; iv < nv; ++iv) bvec[iv] = std::complex<double>(0.0, 0.0);
                for (int ivp = 0; ivp < nv; ++ivp) {
                    double cvec_re = cvec[ivp].real();
                    double cvec_im = cvec[ivp].imag();
                    
                    for (int iv = 0; iv < nv; ++iv) {
                        double cval = cmat(iv, ivp, ic_loc, itor);
                        bvec[iv] += std::complex<double>(cval * cvec_re, cval * cvec_im);
                    }
                }

                // 3. Transpose and save results
                for (int k = 0; k < vcount; ++k) {
                    for (int j = 0; j < nv_loc; ++j) {
                        int target_ism_idx = k + (ism * vcount);
                        fsendf(j, itor, ic_loc, target_ism_idx) = bvec[j + (k * nv_loc)];
                    }
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 7) {
        std::cerr << "Usage: " << argv[0]
                  << " <nv_loc> <nt> <nc_loc> <n_sim> <vcount> <iterations>" << std::endl;
        return 1;
    }

    // 1. Parse command line arguments
    const int nv_loc = std::stoi(argv[1]);
    const int nt = std::stoi(argv[2]);
    const int nc_loc = std::stoi(argv[3]);
    const int n_sim = std::stoi(argv[4]);
    const int vcount = std::stoi(argv[5]);
    const int iterations = std::stoi(argv[6]);
    const int nv = nv_loc * vcount;

    // 2. Initialize Buffers
    Tensor4DBuffer< std::complex<double> > cap_h_v(nc_loc, nt, nv, n_sim);
    Tensor4DBuffer< double > cmat(nv, nv, nc_loc, nt);
    Tensor4DBuffer< std::complex<double> > fsendf(nv_loc, nt, nc_loc, vcount * n_sim);

    // 3. Populate with pseudo-random values
    std::mt19937 gen(42); // Fixed seed for reproducibility
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    for(int l=0; l<n_sim; ++l)
     for(int k=0; k<nv; ++k)
      for(int j=0; j<nt; ++j)
       for(int i=0; i<nc_loc; ++i)
        cap_h_v(i,j,k,l) = {dist(gen), dist(gen)};

    for(int l=0; l<nt; ++l)
     for(int k=0; k<nc_loc; ++k)
      for(int j=0; j<nv; ++j)
       for(int i=0; i<nv; ++i)
        cmat(i,j,k,l) = dist(gen);
    fsendf(0,0,0,0) = {0.0,0.0};

    // 4. Benchmarking loop
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        // minor perturbation to avoid over-optimization
        cmat(1,2,0,0) = cmat(1,2,0,0) + fsendf(0,0,0,0).real();
        calc_collision(nv_loc, nt, nc_loc, n_sim, nv, vcount, cap_h_v, cmat, fsendf);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "Total time for " << iterations << " iterations: " << diff.count() << " seconds" << std::endl;
    std::cout << "Result " << fsendf(0,0,0,0) << std::endl;

    return 0;
}

