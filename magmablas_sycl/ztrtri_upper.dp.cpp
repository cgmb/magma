/*
    -- MAGMA (version 2.0) --
       Univ. of Tennessee, Knoxville
       Univ. of California, Berkeley
       Univ. of Colorado, Denver
       @date

       @precisions normal z -> c d s

       @author Peng Du
       @author Tingxing Dong
       @author Mark Gates
       @author Azzam Haidar
       @author Ahmad Abdelfattah
       
       This file implements upper case, and is called by ztrtri_kernel.cu.
       It's convenient to have separate files for lower & upper, to diff the sources.
*/

#include <CL/sycl.hpp>
#include <dpct/dpct.hpp>
#include "magma_internal.h"

#define TRTRI_NONBATCHED
#include "ztrtri.dp.hpp"
#include "ztrtri_upper_device.dp.hpp"

/******************************************************************************/
void
ztrtri_diag_upper_kernel(
    magma_diag_t diag, int n, const magmaDoubleComplex *A, int lda, magmaDoubleComplex *d_dinvA,
    sycl::nd_item<3> item_ct1, magmaDoubleComplex *sB)
{
    ztrtri_diag_upper_device(diag, n, A, lda, d_dinvA, item_ct1, sB);
}


/******************************************************************************/
void
triple_zgemm16_part1_upper_kernel(
    int n, const magmaDoubleComplex *Ain, int lda, magmaDoubleComplex *d_dinvA, int jb, int npages,
    sycl::nd_item<3> item_ct1,
    sycl::local_accessor<magmaDoubleComplex, 2> sB)
{
    triple_zgemm16_part1_upper_device(n, Ain, lda, d_dinvA, jb, npages,
                                      item_ct1, sB);
}


/******************************************************************************/
void
triple_zgemm16_part2_upper_kernel(
    int n, const magmaDoubleComplex *Ain, int lda, magmaDoubleComplex *d_dinvA, int jb, int npages,
    sycl::nd_item<3> item_ct1,
    sycl::local_accessor<magmaDoubleComplex, 2> sB)
{
    triple_zgemm16_part2_upper_device(n, Ain, lda, d_dinvA, jb, npages,
                                      item_ct1, sB);
}


/******************************************************************************/
void
triple_zgemm32_part1_upper_kernel(
    int n, const magmaDoubleComplex *Ain, int lda, magmaDoubleComplex *d_dinvA, int jb, int npages,
    sycl::nd_item<3> item_ct1,
    sycl::local_accessor<magmaDoubleComplex, 2> sB)
{
    triple_zgemm32_part1_upper_device(n, Ain, lda, d_dinvA, jb, npages,
                                      item_ct1, sB);
}


/******************************************************************************/
void
triple_zgemm32_part2_upper_kernel(
    int n, const magmaDoubleComplex *Ain, int lda, magmaDoubleComplex *d_dinvA, int jb, int npages,
    sycl::nd_item<3> item_ct1,
    sycl::local_accessor<magmaDoubleComplex, 2> sB)
{
    triple_zgemm32_part2_upper_device(n, Ain, lda, d_dinvA, jb, npages,
                                      item_ct1, sB);
}


/******************************************************************************/
void
triple_zgemm64_part1_upper_kernel(
    int n, const magmaDoubleComplex *Ain, int lda, magmaDoubleComplex *d_dinvA, int jb, int npages,
    sycl::nd_item<3> item_ct1,
    sycl::local_accessor<magmaDoubleComplex, 2> sB)
{
    triple_zgemm64_part1_upper_device(n, Ain, lda, d_dinvA, jb, npages,
                                      item_ct1, sB);
}


/******************************************************************************/
void
triple_zgemm64_part2_upper_kernel(
    int n, const magmaDoubleComplex *Ain, int lda, magmaDoubleComplex *d_dinvA, int jb, int npages,
    sycl::nd_item<3> item_ct1,
    sycl::local_accessor<magmaDoubleComplex, 2> sB)
{
    triple_zgemm64_part2_upper_device(n, Ain, lda, d_dinvA, jb, npages,
                                      item_ct1, sB);
}


/******************************************************************************/
void
triple_zgemm_above64_part1_upper_kernel(
    int n, const magmaDoubleComplex *Ain, int lda, magmaDoubleComplex *d_dinvA, int jb, int npages,
    sycl::nd_item<3> item_ct1,
    sycl::local_accessor<magmaDoubleComplex, 2> sB)
{
    triple_zgemm_above64_part1_upper_device(n, Ain, lda, d_dinvA, jb, npages,
                                            item_ct1, sB);
}


/******************************************************************************/
void
triple_zgemm_above64_part2_upper_kernel(
    int n, const magmaDoubleComplex *Ain, int lda, magmaDoubleComplex *d_dinvA, int jb, int npages,
    sycl::nd_item<3> item_ct1,
    sycl::local_accessor<magmaDoubleComplex, 2> sB)
{
    triple_zgemm_above64_part2_upper_device(n, Ain, lda, d_dinvA, jb, npages,
                                            item_ct1, sB);
}


/******************************************************************************/
void
triple_zgemm_above64_part3_upper_kernel(
    int n, const magmaDoubleComplex *Ain, int lda, magmaDoubleComplex *d_dinvA, int jb, int npages,
    sycl::nd_item<3> item_ct1)
{
    triple_zgemm_above64_part3_upper_device(n, Ain, lda, d_dinvA, jb, npages,
                                            item_ct1);
}
