/*******************************************************
 * Copyright (c) 2022, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 ********************************************************/

#include <Array.hpp>
#include <err_oneapi.hpp>
#include <kernel/where.hpp>
#include <where.hpp>
#include <af/dim4.hpp>
#include <complex>

namespace arrayfire {
namespace oneapi {

template<typename T>
Array<uint> where(const Array<T> &in) {
    Param<uint> Out;
    Param<T> In = in;
    kernel::where<T>(Out, In);
    return createParamArray<uint>(Out, true);
}

#define INSTANTIATE(T) template Array<uint> where<T>(const Array<T> &in);

INSTANTIATE(float)
INSTANTIATE(cfloat)
INSTANTIATE(double)
INSTANTIATE(cdouble)
INSTANTIATE(char)
INSTANTIATE(int)
INSTANTIATE(uint)
INSTANTIATE(intl)
INSTANTIATE(uintl)
INSTANTIATE(schar)
INSTANTIATE(uchar)
INSTANTIATE(short)
INSTANTIATE(ushort)

}  // namespace oneapi
}  // namespace arrayfire
