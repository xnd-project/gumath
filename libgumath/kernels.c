/*
* BSD 3-Clause License
*
* Copyright (c) 2017-2018, plures
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <inttypes.h>
#include "ndtypes.h"
#include "xnd.h"
#include "gumath.h"


gm_kernel_set_t empty_kernel_set =
 { .sig = NULL,
   .C = NULL,
   .Fortran = NULL,
   .Strided = NULL,
   .Xnd = NULL };


/****************************************************************************/
/*                               Xnd kernels                                */
/****************************************************************************/

/*
 * Count valid/missing values in a 1D array of records and return the result
 * as a record.
 *
 * Signature:
 *    "... * N * {index: int64, name: string, value: ?int64} -> ... * {valid: int64, missing: int64}"
 */
static int
count_valid_missing(xnd_t stack[], ndt_context_t *ctx)
{
    const xnd_t *array = &stack[0];
    int64_t N = array->type->FixedDim.shape; /* N in the above signature */
    xnd_t *out = &stack[1];
    int64_t ok = 0;
    int64_t na = 0;

    for (int64_t i = 0; i < N; i++) {
        const xnd_t record = xnd_fixed_dim_next(array, i);
        const xnd_t value = xnd_record_next(&record, 2, ctx);
        if (value.ptr == NULL) {
            return -1;
        }

        if (xnd_is_na(&value)) {
            na++;
        }
        else {
            ok++;
        }
    }

    xnd_t valid = xnd_record_next(out, 0, ctx);
    *(int64_t *)(valid.ptr) = ok;

    xnd_t missing = xnd_record_next(out, 1, ctx);
    *(int64_t *)(missing.ptr) = na;

    return 0;
}

int
gm_0D_add_scalar(xnd_t stack[], ndt_context_t *ctx)
{
    const xnd_t *x = &stack[0];
    const xnd_t *y = &stack[1];
    xnd_t *z = &stack[2];
    int64_t N = xnd_fixed_shape(x);
    int64_t yy = *(int64_t *)y->ptr;
    (void)ctx;

    for (int64_t i = 0; i < N; i++) {
        const xnd_t xx = xnd_fixed_dim_next(x, i);
        const xnd_t zz = xnd_fixed_dim_next(z, i);
        *(int64_t *)zz.ptr = *(int64_t *)xx.ptr + yy;
    }

    return 0;
}


/****************************************************************************/
/*                           Generated Xnd kernels                          */
/****************************************************************************/

#define XSTRINGIZE(v) #v
#define STRINGIZE(v) XSTRINGIZE(v)


#define XND_ELEMWISE(func, srctype, desttype) \
static int                                                                     \
gm_fixed_##func##_0D_##srctype##_##desttype(xnd_t stack[], ndt_context_t *ctx) \
{                                                                              \
    const xnd_t *in = &stack[0];                                               \
    xnd_t *out = &stack[1];                                                    \
    (void)ctx;                                                                 \
                                                                               \
    *(desttype##_t *)out->ptr = func(*(const srctype##_t *)in->ptr);           \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static int                                                                     \
gm_fixed_##func##_1D_##srctype##_##desttype(xnd_t stack[], ndt_context_t *ctx) \
{                                                                              \
    const xnd_t *in = &stack[0];                                               \
    xnd_t *out = &stack[1];                                                    \
    int64_t N = xnd_fixed_shape(in);                                           \
    (void)ctx;                                                                 \
                                                                               \
    for (int64_t i = 0; i < N; i++) {                                          \
        const xnd_t v = xnd_fixed_dim_next(in, i);                             \
        const xnd_t u = xnd_fixed_dim_next(out, i);                            \
        *(desttype##_t *)u.ptr = func(*(const srctype##_t *)v.ptr);            \
    }                                                                          \
                                                                               \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static int                                                                     \
gm_var_##func##_0D_##srctype##_##desttype(xnd_t stack[], ndt_context_t *ctx)   \
{                                                                              \
    const xnd_t *in = &stack[0];                                               \
    xnd_t *out = &stack[1];                                                    \
    (void)ctx;                                                                 \
                                                                               \
    *(desttype##_t *)out->ptr = func(*(const srctype##_t *)in->ptr);           \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static int                                                                     \
gm_var_##func##_1D_##srctype##_##desttype(xnd_t stack[], ndt_context_t *ctx)   \
{                                                                              \
    int64_t start[2], step[2];                                                 \
    int64_t shape, n;                                                          \
                                                                               \
    shape = ndt_var_indices(&start[0], &step[0], stack[0].type,                \
                            stack[0].index, ctx);                              \
    if (shape < 0) {                                                           \
        return -1;                                                             \
    }                                                                          \
                                                                               \
    n = ndt_var_indices(&start[1], &step[1], stack[1].type, stack[1].index,    \
                        ctx);                                                  \
    if (n < 0) {                                                               \
        return -1;                                                             \
    }                                                                          \
    if (n != shape) {                                                          \
        ndt_err_format(ctx, NDT_ValueError,                                    \
            "shape mismatch in xnd_var_sin()");                                \
        return -1;                                                             \
    }                                                                          \
                                                                               \
    for (int64_t i = 0; i < shape; i++) {                                      \
        const xnd_t in = xnd_var_dim_next(&stack[0], start[0], step[0], i);    \
        xnd_t out = xnd_var_dim_next(&stack[1], start[1], step[1], i);         \
        *(desttype##_t *)out.ptr = func(*(const srctype##_t *)in.ptr);         \
    }                                                                          \
                                                                               \
    return 0;                                                                  \
}

#define XND_ELEMWISE_INIT(funcname, func, srctype, desttype) \
  { .name = STRINGIZE(funcname),                                                          \
    .sig = "... * N * " STRINGIZE(srctype) "-> ... * N * " STRINGIZE(desttype),           \
    .Xnd = gm_fixed_##func##_1D_##srctype##_##desttype },                                 \
                                                                                          \
  { .name = STRINGIZE(funcname),                                                          \
    .sig = "... * " STRINGIZE(srctype) "-> ... * " STRINGIZE(desttype),                   \
    .Xnd = gm_fixed_##func##_0D_##srctype##_##desttype },                                 \
                                                                                          \
  { .name = STRINGIZE(funcname),                                                          \
    .sig = "var... * var * " STRINGIZE(srctype) "-> var... * var * " STRINGIZE(desttype), \
    .Xnd = gm_var_##func##_1D_##srctype##_##desttype },                                   \
                                                                                          \
  { .name = STRINGIZE(funcname),                                                          \
    .sig = "var... * " STRINGIZE(srctype) "-> var... * " STRINGIZE(desttype),             \
    .Xnd = gm_var_##func##_0D_##srctype##_##desttype }



XND_ELEMWISE(sinf, float32, float32)
XND_ELEMWISE(sinf, int8, float32)
XND_ELEMWISE(sinf, int16, float32)
XND_ELEMWISE(sinf, uint8, float32)
XND_ELEMWISE(sinf, uint16, float32)

XND_ELEMWISE(sin, float64, float64)
XND_ELEMWISE(sin, int32, float64)
XND_ELEMWISE(sin, uint32, float64)

#define copy(x) x
XND_ELEMWISE(copy, int8, int8)
XND_ELEMWISE(copy, int16, int16)
XND_ELEMWISE(copy, int32, int32)
XND_ELEMWISE(copy, int64, int64)
XND_ELEMWISE(copy, uint8, uint8)
XND_ELEMWISE(copy, uint16, uint16)
XND_ELEMWISE(copy, uint32, uint32)
XND_ELEMWISE(copy, uint64, uint64)
XND_ELEMWISE(copy, float32, float32)
XND_ELEMWISE(copy, float64, float64)


static const gm_kernel_init_t kernels[] = {
  /* COPY */
  XND_ELEMWISE_INIT(copy, copy, int8, int8),
  XND_ELEMWISE_INIT(copy, copy, int16, int16),
  XND_ELEMWISE_INIT(copy, copy, int32, int32),
  XND_ELEMWISE_INIT(copy, copy, int64, int64),
  XND_ELEMWISE_INIT(copy, copy, uint8, uint8),
  XND_ELEMWISE_INIT(copy, copy, uint16, uint16),
  XND_ELEMWISE_INIT(copy, copy, uint32, uint32),
  XND_ELEMWISE_INIT(copy, copy, uint64, uint64),
  XND_ELEMWISE_INIT(copy, copy, float32, float32),
  XND_ELEMWISE_INIT(copy, copy, float64, float64),

  /* SIN */
  XND_ELEMWISE_INIT(sin, sinf, float32, float32),
  XND_ELEMWISE_INIT(sin, sinf, uint8, float32),
  XND_ELEMWISE_INIT(sin, sinf, uint16, float32),
  XND_ELEMWISE_INIT(sin, sinf, int8, float32),
  XND_ELEMWISE_INIT(sin, sinf, int16, float32),

  XND_ELEMWISE_INIT(sin, sin, float64, float64),
  XND_ELEMWISE_INIT(sin, sin, uint32, float64),
  XND_ELEMWISE_INIT(sin, sin, int32, float64),


  { .name = "add_scalar", .sig = "... * N * int64, ... * int64 -> ... * N * int64", .Xnd = gm_0D_add_scalar },

  /* example for handling structs with missing values */
  { .name = "count_valid_missing",
    .sig = "... * N * {index: int64, name: string, value: ?int64} -> ... * {valid: int64, missing: int64}",
    .Xnd = count_valid_missing },

  { .name = NULL, .sig = NULL }
};


/****************************************************************************/
/*                       Initialize kernel table                            */
/****************************************************************************/

int
gm_init_kernels(gm_tbl_t *tbl, ndt_context_t *ctx)
{
    const gm_kernel_init_t *k;

    for (k = kernels; k->name != NULL; k++) {
        if (gm_add_kernel(tbl, k, ctx) < 0) {
            return -1;
        }
    }

    return 0;
}
