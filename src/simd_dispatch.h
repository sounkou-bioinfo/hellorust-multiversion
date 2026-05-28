#ifndef HELLORUST_SIMD_DISPATCH_H
#define HELLORUST_SIMD_DISPATCH_H

#include <stddef.h>
#include <stdint.h>

void hellorust_simd_init(void);
const char *hellorust_simd_selected_backend(void);
const char *hellorust_simd_compiled_backends(void);
const char *hellorust_simd_supported_backends(void);
int hellorust_simd_set_backend(const char *backend);
int32_t hellorust_simd_count_nonzero(const uint8_t *x, size_t n);

#endif
