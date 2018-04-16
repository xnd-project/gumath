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


#ifndef GUMATH_H
#define GUMATH_H

#include "ndtypes.h"
#include "xnd.h"


#ifdef _MSC_VER
  #if defined (EXPORT)
    #define GM_API __declspec(dllexport)
  #elif defined(IMPORT)
    #define GM_API __declspec(dllimport)
  #else
    #define GM_API
  #endif
  #ifndef GM_UNUSED
    #define GM_UNUSED
  #endif
#else
  #define GM_API
  #if defined(__GNUC__) && !defined(__INTEL_COMPILER)
    #define GM_UNUSED __attribute__((unused))
  #else
    #define GM_UNUSED
  #endif
#endif


#define GM_MAX_KERNELS 128

typedef float float32_t;
typedef double float64_t;


typedef int (* gm_c_kernel_t)(char **args, int64_t *dimensions, void *data);
typedef int (* gm_fortran_kernel_t)(char **args, int64_t *dimensions, void *data);
typedef int (* gm_strided_kernel_t)(char **args, int64_t *dimensions, int64_t *steps, void *data);
typedef int (* gm_xnd_kernel_t)(xnd_t stack[], ndt_context_t *ctx);

/* Collection of specialized kernels for a single function signature. */
typedef struct {
    ndt_t *sig;
    const ndt_constraint_t *constraint;
    bool vectorize;

    gm_c_kernel_t C;
    gm_fortran_kernel_t Fortran;
    gm_strided_kernel_t Strided;
    gm_xnd_kernel_t Xnd;
} gm_kernel_set_t;

typedef struct {
    const char *name;
    const char *type;
    const ndt_methods_t *meth;
} gm_typedef_init_t;

typedef struct {
    const char *name;
    const char *sig;
    const ndt_constraint_t *constraint;
    bool vectorize;

    gm_c_kernel_t C;
    gm_fortran_kernel_t Fortran;
    gm_strided_kernel_t Strided;
    gm_xnd_kernel_t Xnd;
} gm_kernel_init_t;

/* Actual kernel selected for application. */
typedef struct {
    enum ndt_apply tag;
    const gm_kernel_set_t *set;
} gm_kernel_t;

/* Multimethod with associated kernels. */
typedef struct {
    char *name;
    int nkernels;
    gm_kernel_set_t kernels[GM_MAX_KERNELS];
} gm_func_t;


/******************************************************************************/
/*                                  Functions                                 */
/******************************************************************************/

GM_API gm_func_t *gm_func_new(const char *name, ndt_context_t *ctx);
GM_API void gm_func_del(gm_func_t *f);

GM_API gm_func_t *gm_add_func(const char *name, ndt_context_t *ctx);
GM_API int gm_add_kernel(const gm_kernel_init_t *kernel, ndt_context_t *ctx);
GM_API int gm_apply(const gm_kernel_t *kernel, xnd_t stack[], int outer_dims, ndt_context_t *ctx);
// GM_API int gm_map(const gm_kernel_t *f, xnd_t stack[], int outer_dims, ndt_context_t *ctx);
GM_API gm_kernel_t gm_select(ndt_apply_spec_t *spec, const char *name, const ndt_t *in_types[], int nin,
                             const xnd_t args[], ndt_context_t *ctx);


/******************************************************************************/
/*                                NumPy loops                                 */
/******************************************************************************/

GM_API int gm_np_flatten(char **args, const int nargs,
                         int64_t *dimensions, int64_t *strides, const xnd_t stack[],
                         ndt_context_t *ctx);

GM_API int gm_np_convert_xnd(char **args, const int nargs,
                             int64_t *dimensions, const int dims_size,
                             int64_t *steps, const int steps_size,
                             xnd_t stack[], const int outer_dims,
                             ndt_context_t *ctx);

GM_API int gm_np_map(const gm_strided_kernel_t f,
                     char **args, const int nargs,
                     intptr_t *dimensions,
                     intptr_t *steps,
                     void *data,
                     int outer_dims);


/******************************************************************************/
/*                                  Xnd loops                                 */
/******************************************************************************/

GM_API int gm_xnd_map(const gm_xnd_kernel_t f, xnd_t stack[], const int nargs,
                      const int outer_dims, bool vectorize, ndt_context_t *ctx);


/******************************************************************************/
/*                                Gufunc table                                */
/******************************************************************************/

GM_API int gm_tbl_add(const char *key, gm_func_t *value, ndt_context_t *ctx);
GM_API gm_func_t *gm_tbl_find(const char *key, ndt_context_t *ctx);
GM_API int gm_tbl_map(int (*f)(const gm_func_t *, void *state), void *state);


/******************************************************************************/
/*                       Library initialization and tables                    */
/******************************************************************************/

GM_API int gm_init(ndt_context_t *ctx);
GM_API int gm_init_kernels(ndt_context_t *ctx);
GM_API int gm_init_graph_kernels(ndt_context_t *ctx);
GM_API int gm_init_bfloat16_kernels(ndt_context_t *ctx);
GM_API int gm_init_pdist_kernels(ndt_context_t *ctx);

GM_API void gm_finalize(void);


#endif /* GUMATH_H */