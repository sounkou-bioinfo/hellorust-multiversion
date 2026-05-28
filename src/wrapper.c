#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include <limits.h>
#include <stdint.h>

// Import C headers for rust API
#include "myrustlib/api.h"
#include "simd_dispatch.h"

// Actual Wrappers
SEXP hello_wrapper(void){
  char* hello_rust = string_from_rust();
  SEXP hello_world_string = PROTECT(Rf_mkCharCE(hello_rust, CE_UTF8));
  free_string_from_rust(hello_rust);
  UNPROTECT(1);
  return Rf_ScalarString(hello_world_string);
}

SEXP random_wrapper(void){
  return Rf_ScalarInteger(random_number());
}

SEXP threads_wapper(void){
  run_threads();
  return R_NilValue;
}

SEXP simd_count_nonzero_wrapper(SEXP x){
  if(TYPEOF(x) != RAWSXP){
    Rf_error("x must be a raw vector");
  }
  R_xlen_t n = XLENGTH(x);
  if(n > (R_xlen_t) INT_MAX){
    Rf_error("x is too large for an integer count");
  }
  int32_t count = hellorust_simd_count_nonzero((const uint8_t*) RAW(x), (size_t) n);
  return Rf_ScalarInteger((int) count);
}

SEXP simd_set_backend_wrapper(SEXP backend){
  if(TYPEOF(backend) != STRSXP || XLENGTH(backend) != 1){
    Rf_error("backend must be a single string");
  }
  if(STRING_ELT(backend, 0) == NA_STRING){
    Rf_error("backend must not be NA");
  }

  const char *name = CHAR(STRING_ELT(backend, 0));
  if(!hellorust_simd_set_backend(name)){
    Rf_error("backend '%s' is not available; supported backends are: %s",
             name, hellorust_simd_supported_backends());
  }

  return Rf_mkString(hellorust_simd_selected_backend());
}

static void set_backend_vector(SEXP out, int index, int supported){
  int has_sse2 = supported ?
    (hellorust_simd_compiled_has_sse2() && hellorust_simd_cpu_has_sse2()) :
    hellorust_simd_compiled_has_sse2();
  int has_avx2 = supported ?
    (hellorust_simd_compiled_has_avx2() && hellorust_simd_cpu_has_avx2()) :
    hellorust_simd_compiled_has_avx2();
  int has_avx512 = supported ?
    (hellorust_simd_compiled_has_avx512() && hellorust_simd_cpu_has_avx512()) :
    hellorust_simd_compiled_has_avx512();
  int has_neon = supported ?
    (hellorust_simd_compiled_has_neon() && hellorust_simd_cpu_has_neon()) :
    hellorust_simd_compiled_has_neon();

  int n = 1 + has_sse2 + has_avx2 + has_avx512 + has_neon;
  SEXP backends = PROTECT(Rf_allocVector(STRSXP, n));
  int i = 0;
  SET_STRING_ELT(backends, i++, Rf_mkChar("scalar"));
  if(has_sse2) SET_STRING_ELT(backends, i++, Rf_mkChar("sse2"));
  if(has_avx2) SET_STRING_ELT(backends, i++, Rf_mkChar("avx2"));
  if(has_avx512) SET_STRING_ELT(backends, i++, Rf_mkChar("avx512"));
  if(has_neon) SET_STRING_ELT(backends, i++, Rf_mkChar("neon"));
  SET_VECTOR_ELT(out, index, backends);
  UNPROTECT(1);
}

SEXP simd_info_wrapper(void){
  hellorust_simd_init();

  SEXP out = PROTECT(Rf_allocVector(VECSXP, 11));
  SEXP names = PROTECT(Rf_allocVector(STRSXP, 11));

  SET_STRING_ELT(names, 0, Rf_mkChar("selected"));
  SET_STRING_ELT(names, 1, Rf_mkChar("compiled"));
  SET_STRING_ELT(names, 2, Rf_mkChar("supported"));
  SET_STRING_ELT(names, 3, Rf_mkChar("compiled_sse2"));
  SET_STRING_ELT(names, 4, Rf_mkChar("compiled_avx2"));
  SET_STRING_ELT(names, 5, Rf_mkChar("compiled_avx512"));
  SET_STRING_ELT(names, 6, Rf_mkChar("compiled_neon"));
  SET_STRING_ELT(names, 7, Rf_mkChar("cpu_sse2"));
  SET_STRING_ELT(names, 8, Rf_mkChar("cpu_avx2"));
  SET_STRING_ELT(names, 9, Rf_mkChar("cpu_avx512"));
  SET_STRING_ELT(names, 10, Rf_mkChar("cpu_neon"));

  SET_VECTOR_ELT(out, 0, Rf_mkString(hellorust_simd_selected_backend()));
  set_backend_vector(out, 1, 0);
  set_backend_vector(out, 2, 1);
  SET_VECTOR_ELT(out, 3, Rf_ScalarLogical(hellorust_simd_compiled_has_sse2() != 0));
  SET_VECTOR_ELT(out, 4, Rf_ScalarLogical(hellorust_simd_compiled_has_avx2() != 0));
  SET_VECTOR_ELT(out, 5, Rf_ScalarLogical(hellorust_simd_compiled_has_avx512() != 0));
  SET_VECTOR_ELT(out, 6, Rf_ScalarLogical(hellorust_simd_compiled_has_neon() != 0));
  SET_VECTOR_ELT(out, 7, Rf_ScalarLogical(hellorust_simd_cpu_has_sse2() != 0));
  SET_VECTOR_ELT(out, 8, Rf_ScalarLogical(hellorust_simd_cpu_has_avx2() != 0));
  SET_VECTOR_ELT(out, 9, Rf_ScalarLogical(hellorust_simd_cpu_has_avx512() != 0));
  SET_VECTOR_ELT(out, 10, Rf_ScalarLogical(hellorust_simd_cpu_has_neon() != 0));
  Rf_setAttrib(out, R_NamesSymbol, names);

  UNPROTECT(2);
  return out;
}

// Standard R package stuff
static const R_CallMethodDef CallEntries[] = {
  {"hello_wrapper", (DL_FUNC) &hello_wrapper, 0},
  {"random_wrapper", (DL_FUNC) &random_wrapper, 0},
  {"threads_wapper", (DL_FUNC) &threads_wapper, 0},
  {"simd_count_nonzero_wrapper", (DL_FUNC) &simd_count_nonzero_wrapper, 1},
  {"simd_set_backend_wrapper", (DL_FUNC) &simd_set_backend_wrapper, 1},
  {"simd_info_wrapper", (DL_FUNC) &simd_info_wrapper, 0},
  {NULL, NULL, 0}
};

void R_init_hellorust_multiversion(DllInfo *dll) {
  hellorust_simd_init();
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
}
