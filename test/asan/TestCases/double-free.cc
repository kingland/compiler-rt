// RUN: %clangxx_asan -O0 %s -o %t 2>&1
// RUN: not %run %t 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=MALLOC-CTX

// Also works if no malloc context is available.
// RUN: env ASAN_OPTIONS=malloc_context_size=0:fast_unwind_on_malloc=0 not %run %t 2>&1 | FileCheck %s
// RUN: env ASAN_OPTIONS=malloc_context_size=0:fast_unwind_on_malloc=1 not %run %t 2>&1 | FileCheck %s
// XFAIL: arm-linux-gnueabi

#include <stdlib.h>
#include <string.h>
int main(int argc, char **argv) {
  char *x = (char*)malloc(10 * sizeof(char));
  memset(x, 0, 10);
  int res = x[argc];
  free(x);
  free(x + argc - 1);  // BOOM
  // CHECK: AddressSanitizer: attempting double-free{{.*}}in thread T0
  // CHECK: #0 0x{{.*}} in {{.*}}free
  // CHECK: #1 0x{{.*}} in main {{.*}}double-free.cc:[[@LINE-3]]
  // CHECK: freed by thread T0 here:
  // MALLOC-CTX: #0 0x{{.*}} in {{.*}}free
  // MALLOC-CTX: #1 0x{{.*}} in main {{.*}}double-free.cc:[[@LINE-7]]
  // CHECK: allocated by thread T0 here:
  // MALLOC-CTX: double-free.cc:[[@LINE-12]]
  return res;
}
