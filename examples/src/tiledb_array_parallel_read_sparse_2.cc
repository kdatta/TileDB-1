/**
 * @file   tiledb_array_parallel_read_sparse_2.cc
 *
 * @section LICENSE
 *
 * The MIT License
 * 
 * @copyright Copyright (c) 2016 MIT and Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * @section DESCRIPTION
 *
 * It shows how to read from a sparse array in parallel with OpenMP.
 */

#include "tiledb.h"
#include <stdio.h>

#ifdef HAVE_OPENMP
#include <omp.h>


// The function to be computed in parallel
void parallel_read(
    const TileDB_CTX* tiledb_ctx,
    const char* array_name,
    const void* subarray,
    void** buffers,
    size_t* buffer_sizes,
    int* count);

int main() {
  // Initialize context with the default configuration parameters
  TileDB_CTX* tiledb_ctx;
  tiledb_ctx_init(&tiledb_ctx, NULL);

  // Array name
  const char* array_name = "my_workspace/sparse_arrays/my_array_B";

  // Prepare cell buffers
  // --- First read ---
  const int64_t subarray_1[] = { 1, 2, 1, 4 }; 
  int buffer_a1_1[4]; 
  void* buffers_1[] = { buffer_a1_1 };
  size_t buffer_sizes_1[] = { sizeof(buffer_a1_1) };
  // --- Second read ---
  const int64_t subarray_2[] = { 3, 4, 1, 4 }; 
  int buffer_a1_2[4]; 
  void* buffers_2[] = { buffer_a1_2 };
  size_t buffer_sizes_2[] = { sizeof(buffer_a1_2) };

  // Buffer to store the individual thread counts 
  int counts[2];

  // Write in parallel
  #pragma omp parallel for 
  for(int i=0; i<2; ++i) {
    // Populate the thread data 
    void** buffers;
    size_t* buffer_sizes;
    const void* subarray;
    if(i==0) {         // First read
      buffers = buffers_1;
      buffer_sizes = buffer_sizes_1;
      subarray = subarray_1;
    } else if(i==1) {  // Second read
      buffers = buffers_2;
      buffer_sizes = buffer_sizes_2;
      subarray = subarray_2;
    }

    // Parallel read
    parallel_read(
        tiledb_ctx, 
        array_name,
        subarray, 
        buffers, 
        buffer_sizes,
        &counts[i]);
  }

  // Output result
  int total_count = 0;
  for(int i=0; i<2; ++i)
    total_count += counts[i];
  printf("Number of a1 values greater than 5: %d \n", total_count);

  // Finalize context
  tiledb_ctx_finalize(tiledb_ctx);

  return 0;
}

void parallel_read(
    const TileDB_CTX* tiledb_ctx,
    const char* array_name,
    const void* subarray,
    void** buffers,
    size_t* buffer_sizes,
    int* count) {
  // Only attribute "a1" is needed
  const char* attributes[] = { "a1" };

  // Initialize array
  TileDB_Array* tiledb_array;
  tiledb_array_init(
      tiledb_ctx,                                // Context 
      &tiledb_array,                             // Array object
      array_name,                                // Array name
      TILEDB_ARRAY_READ,                         // Mode
      subarray,                                  // Subarray
      attributes,                                // Subset on attributes
      1);                                        // Number of attributes

  // Read from array
  tiledb_array_read(tiledb_array, buffers, buffer_sizes); 

  // Count number of a1 values greater than 10
  *count = 0;
  int* a1 = (int*) buffers[0];
  int num = buffer_sizes[0] / sizeof(int);
  for(int i=0; i<num; ++i)
    if(a1[i] > 5)
      ++(*count);

  // Finalize array
  tiledb_array_finalize(tiledb_array);
}

#else

int main() {
  printf("OpenMP not supported.");

  return 0;
}

#endif
