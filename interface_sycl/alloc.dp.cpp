/*
    -- MAGMA (version 2.0) --
       Univ. of Tennessee, Knoxville
       Univ. of California, Berkeley
       Univ. of Colorado, Denver
       @date

       @author Mark Gates
*/

#include <CL/sycl.hpp>
#include <dpct/dpct.hpp>
#include <stdlib.h>
#include <stdio.h>

#ifdef DEBUG_MEMORY
#include <map>
#include <mutex>  // requires C++11
#endif

#include "magma_v2.h"
#include "magma_internal.h"
#include "error.h"


#ifdef DEBUG_MEMORY
std::mutex                g_pointers_mutex;  // requires C++11
std::map< void*, size_t > g_pointers_dev;
std::map< void*, size_t > g_pointers_cpu;
std::map< void*, size_t > g_pointers_pin;
#endif


/***************************************************************************//**
    Allocates memory on the GPU. CUDA imposes a synchronization.
    Use magma_free() to free this memory.

    @param[out]
    ptrPtr  On output, set to the pointer that was allocated.
            NULL on failure.

    @param[in]
    size    Size in bytes to allocate. If size = 0, allocates some minimal size.

    @return MAGMA_SUCCESS
    @return MAGMA_ERR_DEVICE_ALLOC on failure

    Type-safe versions avoid the need for a (void**) cast and explicit sizeof.
    @see magma_smalloc
    @see magma_dmalloc
    @see magma_cmalloc
    @see magma_zmalloc
    @see magma_imalloc
    @see magma_index_malloc

    @ingroup magma_malloc
*******************************************************************************/
extern "C" magma_int_t magma_malloc(magma_ptr *ptrPtr, size_t size) try {
    // malloc and free sometimes don't work for size=0, so allocate some minimal size
    if ( size == 0 )
        size = sizeof(magmaDoubleComplex);
    /*
    DPCT1003:1: Migrated API does not return error code. (*, 0) is inserted. You
    may need to rewrite this code.
    */
    if (0 != (*ptrPtr = (magma_ptr)sycl::malloc_device(
                  size, dpct::get_default_queue()),
              0)) {
        return MAGMA_ERR_DEVICE_ALLOC;
    }

    #ifdef DEBUG_MEMORY
    g_pointers_mutex.lock();
    g_pointers_dev[ *ptrPtr ] = size;
    g_pointers_mutex.unlock();
    #endif

    return MAGMA_SUCCESS;
}
catch (sycl::exception const &exc) {
  std::cerr << exc.what() << "Exception caught at file:" << __FILE__
            << ", line:" << __LINE__ << std::endl;
  std::exit(1);
}

/***************************************************************************//**
    @fn magma_free( ptr )

    Frees GPU memory previously allocated by magma_malloc().

    @param[in]
    ptr     Pointer to free.

    @return MAGMA_SUCCESS
    @return MAGMA_ERR_INVALID_PTR on failure

    @ingroup magma_malloc
*******************************************************************************/
extern "C" magma_int_t magma_free_internal(magma_ptr ptr, const char *func,
                                           const char *file, int line) try {
#ifdef DEBUG_MEMORY
    g_pointers_mutex.lock();
    if ( ptr != NULL && g_pointers_dev.count( ptr ) == 0 ) {
        fprintf( stderr, "magma_free( %p ) that wasn't allocated with magma_malloc.\n", ptr );
    }
    else {
        g_pointers_dev.erase( ptr );
    }
    g_pointers_mutex.unlock();
    #endif

    /*
    DPCT1003:2: Migrated API does not return error code. (*, 0) is inserted. You
    may need to rewrite this code.
    */
    int err = (sycl::free(ptr, dpct::get_default_queue()), 0);
    check_xerror( err, func, file, line );

    return MAGMA_SUCCESS;
}
catch (sycl::exception const &exc) {
  std::cerr << exc.what() << "Exception caught at file:" << __FILE__
            << ", line:" << __LINE__ << std::endl;
  std::exit(1);
}

/***************************************************************************//**
    Allocate size bytes on CPU.
    The purpose of using this instead of malloc is to properly align arrays
    for vector (SSE, AVX) instructions. The default implementation uses
    posix_memalign (on Linux, MacOS, etc.) or _aligned_malloc (on Windows)
    to align memory to a 64 byte boundary (typical cache line size).
    Use magma_free_cpu() to free this memory.

    @param[out]
    ptrPtr  On output, set to the pointer that was allocated.
            NULL on failure.

    @param[in]
    size    Size in bytes to allocate. If size = 0, allocates some minimal size.

    @return MAGMA_SUCCESS
    @return MAGMA_ERR_HOST_ALLOC on failure

    Type-safe versions avoid the need for a (void**) cast and explicit sizeof.
    @see magma_smalloc_cpu
    @see magma_dmalloc_cpu
    @see magma_cmalloc_cpu
    @see magma_zmalloc_cpu
    @see magma_imalloc_cpu
    @see magma_index_malloc_cpu

    @ingroup magma_malloc_cpu
*******************************************************************************/
extern "C" magma_int_t
magma_malloc_cpu( void** ptrPtr, size_t size )
{
    // malloc and free sometimes don't work for size=0, so allocate some minimal size
    if ( size == 0 )
        size = sizeof(magmaDoubleComplex);
#if 1
#if defined( _WIN32 ) || defined( _WIN64 )
    *ptrPtr = _aligned_malloc( size, 64 );
    if ( *ptrPtr == NULL ) {
        return MAGMA_ERR_HOST_ALLOC;
    }
#else
    int err = posix_memalign( ptrPtr, 64, size );
    if ( err != 0 ) {
        *ptrPtr = NULL;
        return MAGMA_ERR_HOST_ALLOC;
    }
#endif
#else
    *ptrPtr = malloc( size );
    if ( *ptrPtr == NULL ) {
        return MAGMA_ERR_HOST_ALLOC;
    }
#endif

    #ifdef DEBUG_MEMORY
    g_pointers_mutex.lock();
    g_pointers_cpu[ *ptrPtr ] = size;
    g_pointers_mutex.unlock();
    #endif

    return MAGMA_SUCCESS;
}


/***************************************************************************//**
    Frees CPU memory previously allocated by magma_malloc_cpu().
    The default implementation uses free(),
    which works for both malloc and posix_memalign.
    For Windows, _aligned_free() is used.

    @param[in]
    ptr     Pointer to free.

    @return MAGMA_SUCCESS
    @return MAGMA_ERR_INVALID_PTR on failure

    @ingroup magma_malloc_cpu
*******************************************************************************/
extern "C" magma_int_t
magma_free_cpu( void* ptr )
{
    #ifdef DEBUG_MEMORY
    g_pointers_mutex.lock();
    if ( ptr != NULL && g_pointers_cpu.count( ptr ) == 0 ) {
        fprintf( stderr, "magma_free_cpu( %p ) that wasn't allocated with magma_malloc_cpu.\n", ptr );
    }
    else {
        g_pointers_cpu.erase( ptr );
    }
    g_pointers_mutex.unlock();
    #endif

#if defined( _WIN32 ) || defined( _WIN64 )
    _aligned_free( ptr );
#else
    free( ptr );
#endif
    return MAGMA_SUCCESS;
}


/***************************************************************************//**
    Allocates memory on the CPU in pinned memory.
    Use magma_free_pinned() to free this memory.

    @param[out]
    ptrPtr  On output, set to the pointer that was allocated.
            NULL on failure.

    @param[in]
    size    Size in bytes to allocate. If size = 0, allocates some minimal size.

    @return MAGMA_SUCCESS
    @return MAGMA_ERR_HOST_ALLOC on failure

    Type-safe versions avoid the need for a (void**) cast and explicit sizeof.
    @see magma_smalloc_pinned
    @see magma_dmalloc_pinned
    @see magma_cmalloc_pinned
    @see magma_zmalloc_pinned
    @see magma_imalloc_pinned
    @see magma_index_malloc_pinned

    @ingroup magma_malloc_pinned
*******************************************************************************/
extern "C" magma_int_t magma_malloc_pinned(void **ptrPtr, size_t size) try {
    // malloc and free sometimes don't work for size=0, so allocate some minimal size
    // (for pinned memory, the error is detected in free)
    if ( size == 0 )
        size = sizeof(magmaDoubleComplex);
    /*
    DPCT1003:3: Migrated API does not return error code. (*, 0) is inserted. You
    may need to rewrite this code.
    */
    /*
    DPCT1048:0: The original value cudaHostAllocPortable is not meaningful in
    the migrated code and was removed or replaced with 0. You may need to check
    the migrated code.
    */
    if (0 !=
        (*ptrPtr = (void *)sycl::malloc_host(size, dpct::get_default_queue()),
         0)) {
        return MAGMA_ERR_HOST_ALLOC;
    }

    #ifdef DEBUG_MEMORY
    g_pointers_mutex.lock();
    g_pointers_pin[ *ptrPtr ] = size;
    g_pointers_mutex.unlock();
    #endif

    return MAGMA_SUCCESS;
}
catch (sycl::exception const &exc) {
  std::cerr << exc.what() << "Exception caught at file:" << __FILE__
            << ", line:" << __LINE__ << std::endl;
  std::exit(1);
}

/***************************************************************************//**
    @fn magma_free_pinned( ptr )

    Frees CPU pinned memory previously allocated by magma_malloc_pinned().

    @param[in]
    ptr     Pointer to free.

    @return MAGMA_SUCCESS
    @return MAGMA_ERR_INVALID_PTR on failure

    @ingroup magma_malloc_pinned
*******************************************************************************/
extern "C" magma_int_t magma_free_pinned_internal(void *ptr, const char *func,
                                                  const char *file,
                                                  int line) try {
#ifdef DEBUG_MEMORY
    g_pointers_mutex.lock();
    if ( ptr != NULL && g_pointers_pin.count( ptr ) == 0 ) {
        fprintf( stderr, "magma_free_pinned( %p ) that wasn't allocated with magma_malloc_pinned.\n", ptr );
    }
    else {
        g_pointers_pin.erase( ptr );
    }
    g_pointers_mutex.unlock();
    #endif

    /*
    DPCT1003:4: Migrated API does not return error code. (*, 0) is inserted. You
    may need to rewrite this code.
    */
    int err = (sycl::free(ptr, dpct::get_default_queue()), 0);
    check_xerror( err, func, file, line );

    return MAGMA_SUCCESS;
}
catch (sycl::exception const &exc) {
  std::cerr << exc.what() << "Exception caught at file:" << __FILE__
            << ", line:" << __LINE__ << std::endl;
  std::exit(1);
}

/***************************************************************************//**
    @fn magma_mem_info( free, total )

    Sets the parameters 'free' and 'total' to the free and total memory in the
    system (in bytes).

    @param[in]
    free    Address of the result for 'free' bytes on the system
    total   Address of the result for 'total' bytes on the system
    
    @return MAGMA_SUCCESS
    @return MAGMA_ERR_INVALID_PTR on failure

*******************************************************************************/
extern "C" magma_int_t
magma_mem_info(size_t * freeMem, size_t * totalMem) {
    /*
    DPCT1072:5: DPC++ currently does not support getting the available memory on
    the current device. You may need to adjust the code.
    */
	// NNB: see https://github.com/intel/llvm/issues/5713
	// this is currently in an Intel-specific extension to SYCL, not portable!
	// TODO: remove use of dpct get_current_device entirely
    sycl::device d = dpct::get_current_device();
	// TODO: sycl::ext::intel::info not availble in oneapi/eng-compiler/2022.10.15.004
	//   even though the SYCL_EXT_INTEL_DEVICE_INFO is 3, just as in
	//   the 2022.10.15.006 module (where it is found)
	//   But all latest versions don't have version 4, which is required for the
	//   free memory calculation to work.
	//   We should change to the below lines when it is available?
    //    *totalMem = d.get_info<sycl::info::device::global_mem_size>();
    //    *freeMem = d.get_info<sycl::ext::intel::info::device::free_memory>();
    *totalMem =
        dpct::get_current_device().get_device_info().get_global_mem_size();
    *freeMem = (*totalMem); // FIX this when extension available!
    return MAGMA_SUCCESS;
}

extern "C" magma_int_t magma_memset(void *ptr, int value, size_t count) try {
    /*
    DPCT1003:6: Migrated API does not return error code. (*, 0) is inserted. You
    may need to rewrite this code.
    */
    return (dpct::get_default_queue().memset(ptr, value, count).wait(), 0);
}
catch (sycl::exception const &exc) {
  std::cerr << exc.what() << "Exception caught at file:" << __FILE__
            << ", line:" << __LINE__ << std::endl;
  std::exit(1);
}

extern "C" magma_int_t magma_memset_async(void *ptr, int value, size_t count,
                                          magma_queue_t queue) try {
#ifdef MAGMA_HAVE_SYCL
//    return cudaMemsetAsync(ptr, value, count, queue);
    /*
    DPCT1003:7: Migrated API does not return error code. (*, 0) is inserted. You
    may need to rewrite this code.
    */
    return (queue->sycl_stream()->memset(ptr, value, count), 0);
#endif
}
catch (sycl::exception const &exc) {
  std::cerr << exc.what() << "Exception caught at file:" << __FILE__
            << ", line:" << __LINE__ << std::endl;
  std::exit(1);
}

//#endif // MAGMA_HAVE_CUDA