/*******************************************************
 * Copyright (c) 2014, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 ********************************************************/

#include <assign.hpp>
#include <kernel/assign.hpp>

#include <Array.hpp>
#include <common/half.hpp>
#include <err_opencl.hpp>
#include <handle.hpp>
#include <memory.hpp>
#include <af/dim4.hpp>

using af::dim4;
using arrayfire::common::half;

namespace arrayfire {
namespace opencl {

static std::mutex mtx;
static std::map<std::pair<const cl::Context*, const int /* deviceId */>,
                cl::Buffer*>
    cachedEmptyBuffers;

template<typename T>
void assign(Array<T>& out, const af_index_t idxrs[], const Array<T>& rhs) {
    kernel::AssignKernelParam_t p;
    std::vector<af_seq> seqs(4, af_span);
    // create seq vector to retrieve output
    // dimensions, offsets & offsets
    for (dim_t x = 0; x < 4; ++x) {
        if (idxrs[x].isSeq) { seqs[x] = idxrs[x].idx.seq; }
    }

    // retrieve dimensions, strides and offsets
    const dim4& dDims = out.dims();
    // retrieve dimensions & strides for array
    // to which rhs is being copied to
    dim4 dstOffs  = toOffset(seqs, dDims);
    dim4 dstStrds = toStride(seqs, dDims);

    for (dim_t i = 0; i < 4; ++i) {
        p.isSeq[i] = idxrs[i].isSeq;
        p.offs[i]  = dstOffs[i];
        p.strds[i] = dstStrds[i];
    }

    cl::Buffer* bPtrs[4];

    std::vector<Array<uint>> idxArrs(4, createEmptyArray<uint>(dim4()));

    // Prepare commonBuffer for empty indexes
    // Buffer is dependent on the context.
    // To avoid copying between devices, we add also deviceId as a dependency
    cl::Buffer* emptyBuffer;
    {
        std::lock_guard<std::mutex> lck(mtx);
        const auto dependent = std::make_pair<const cl::Context*, const int>(
            &getContext(), getActiveDeviceId());
        auto it = cachedEmptyBuffers.find(dependent);
        if (it == cachedEmptyBuffers.end()) {
            emptyBuffer = new cl::Buffer(
                getContext(),
                CL_MEM_READ_ONLY,  // NOLINT(hicpp-signed-bitwise)
                sizeof(uint));
            cachedEmptyBuffers[dependent] = emptyBuffer;
        } else {
            emptyBuffer = it->second;
        }
    }

    // look through indexs to read af_array indexs
    for (dim_t x = 0; x < 4; ++x) {
        // set index pointers were applicable
        if (!p.isSeq[x]) {
            idxArrs[x] = castArray<uint>(idxrs[x].idx.arr);
            bPtrs[x]   = idxArrs[x].get();
        } else {
            // alloc an 1-element buffer to avoid OpenCL from failing using
            // direct buffer allocation as opposed to mem manager to avoid
            // reference count desprepancies between different backends
            bPtrs[x] = emptyBuffer;
        }
    }

    kernel::assign<T>(out, rhs, p, bPtrs);
}

#define INSTANTIATE(T)                                                \
    template void assign<T>(Array<T> & out, const af_index_t idxrs[], \
                            const Array<T>& rhs);

INSTANTIATE(cdouble)
INSTANTIATE(double)
INSTANTIATE(cfloat)
INSTANTIATE(float)
INSTANTIATE(int)
INSTANTIATE(uint)
INSTANTIATE(intl)
INSTANTIATE(uintl)
INSTANTIATE(schar)
INSTANTIATE(uchar)
INSTANTIATE(char)
INSTANTIATE(short)
INSTANTIATE(ushort)
INSTANTIATE(half)

}  // namespace opencl
}  // namespace arrayfire
