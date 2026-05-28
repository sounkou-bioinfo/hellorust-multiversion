#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

char * string_from_rust(void);
void free_string_from_rust(char*);
int32_t random_number(void);
void run_threads(void);

int32_t hellorust_simd_count_nonzero_scalar(const uint8_t *x, size_t n);
int32_t hellorust_simd_count_nonzero_sse2(const uint8_t *x, size_t n);
int32_t hellorust_simd_count_nonzero_avx2(const uint8_t *x, size_t n);
int32_t hellorust_simd_count_nonzero_avx512(const uint8_t *x, size_t n);
int32_t hellorust_simd_count_nonzero_neon(const uint8_t *x, size_t n);

int32_t hellorust_simd_compiled_has_sse2(void);
int32_t hellorust_simd_compiled_has_avx2(void);
int32_t hellorust_simd_compiled_has_avx512(void);
int32_t hellorust_simd_compiled_has_neon(void);

int32_t hellorust_simd_cpu_has_sse2(void);
int32_t hellorust_simd_cpu_has_avx2(void);
int32_t hellorust_simd_cpu_has_avx512(void);
int32_t hellorust_simd_cpu_has_neon(void);

#ifdef __cplusplus
}
#endif
