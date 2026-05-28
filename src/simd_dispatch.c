#include "simd_dispatch.h"

#include "myrustlib/api.h"

#include <string.h>

typedef int32_t (*hellorust_count_fn)(const uint8_t *x, size_t n);

static hellorust_count_fn count_impl = hellorust_simd_count_nonzero_scalar;
static const char *selected_backend = "scalar";
static char compiled_backends[128] = "scalar";
static char supported_backends[128] = "scalar";
static int initialized = 0;

static void append_backend(char *buffer, size_t capacity, const char *name) {
  size_t used = strlen(buffer);
  if (used + 1 >= capacity) {
    return;
  }
  if (used > 0) {
    buffer[used++] = ',';
    buffer[used] = '\0';
  }
  strncat(buffer, name, capacity - used - 1);
}

static int available_sse2(void) {
  return hellorust_simd_compiled_has_sse2() && hellorust_simd_cpu_has_sse2();
}

static int available_avx2(void) {
  return hellorust_simd_compiled_has_avx2() && hellorust_simd_cpu_has_avx2();
}

static int available_avx512(void) {
  return hellorust_simd_compiled_has_avx512() && hellorust_simd_cpu_has_avx512();
}

static int available_neon(void) {
  return hellorust_simd_compiled_has_neon() && hellorust_simd_cpu_has_neon();
}

static void build_backend_strings(void) {
  strcpy(compiled_backends, "scalar");
  strcpy(supported_backends, "scalar");

  if (hellorust_simd_compiled_has_sse2()) {
    append_backend(compiled_backends, sizeof(compiled_backends), "sse2");
  }
  if (hellorust_simd_compiled_has_avx2()) {
    append_backend(compiled_backends, sizeof(compiled_backends), "avx2");
  }
  if (hellorust_simd_compiled_has_avx512()) {
    append_backend(compiled_backends, sizeof(compiled_backends), "avx512");
  }
  if (hellorust_simd_compiled_has_neon()) {
    append_backend(compiled_backends, sizeof(compiled_backends), "neon");
  }

  if (available_sse2()) {
    append_backend(supported_backends, sizeof(supported_backends), "sse2");
  }
  if (available_avx2()) {
    append_backend(supported_backends, sizeof(supported_backends), "avx2");
  }
  if (available_avx512()) {
    append_backend(supported_backends, sizeof(supported_backends), "avx512");
  }
  if (available_neon()) {
    append_backend(supported_backends, sizeof(supported_backends), "neon");
  }
}

static void select_scalar(void) {
  count_impl = hellorust_simd_count_nonzero_scalar;
  selected_backend = "scalar";
}

static void select_auto(void) {
  if (available_avx512()) {
    count_impl = hellorust_simd_count_nonzero_avx512;
    selected_backend = "avx512";
  } else if (available_avx2()) {
    count_impl = hellorust_simd_count_nonzero_avx2;
    selected_backend = "avx2";
  } else if (available_sse2()) {
    count_impl = hellorust_simd_count_nonzero_sse2;
    selected_backend = "sse2";
  } else if (available_neon()) {
    count_impl = hellorust_simd_count_nonzero_neon;
    selected_backend = "neon";
  } else {
    select_scalar();
  }
}

void hellorust_simd_init(void) {
  if (initialized) {
    return;
  }

  build_backend_strings();
  select_auto();
  initialized = 1;
}

int hellorust_simd_set_backend(const char *backend) {
  hellorust_simd_init();

  if (strcmp(backend, "auto") == 0) {
    select_auto();
    return 1;
  }
  if (strcmp(backend, "scalar") == 0) {
    select_scalar();
    return 1;
  }
  if (strcmp(backend, "avx512") == 0 && available_avx512()) {
    count_impl = hellorust_simd_count_nonzero_avx512;
    selected_backend = "avx512";
    return 1;
  }
  if (strcmp(backend, "avx2") == 0 && available_avx2()) {
    count_impl = hellorust_simd_count_nonzero_avx2;
    selected_backend = "avx2";
    return 1;
  }
  if (strcmp(backend, "sse2") == 0 && available_sse2()) {
    count_impl = hellorust_simd_count_nonzero_sse2;
    selected_backend = "sse2";
    return 1;
  }
  if (strcmp(backend, "neon") == 0 && available_neon()) {
    count_impl = hellorust_simd_count_nonzero_neon;
    selected_backend = "neon";
    return 1;
  }

  return 0;
}

const char *hellorust_simd_selected_backend(void) {
  hellorust_simd_init();
  return selected_backend;
}

const char *hellorust_simd_compiled_backends(void) {
  hellorust_simd_init();
  return compiled_backends;
}

const char *hellorust_simd_supported_backends(void) {
  hellorust_simd_init();
  return supported_backends;
}

int32_t hellorust_simd_count_nonzero(const uint8_t *x, size_t n) {
  hellorust_simd_init();
  return count_impl(x, n);
}
