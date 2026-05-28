#' @useDynLib hellorust.multiversion, .registration = TRUE
NULL

#' Hello Rust!
#'
#' Minimal examples of calling rust functions in R via C.
#'
#' These functions call out to rust functions defined in the `myrustlib` cargo
#' crate which is embedded in this package. They return values generated in Rust,
#' such as a UTF-8 string or random number. In addition, `runthreads` is an
#' example of a multi-threaded rust function.
#'
#' @export
#' @rdname hellorust
#' @examples hello()
#' @return a value generated in Rust (a string, random number, and NULL respectively).
hello <- function() {
  .Call("hello_wrapper", PACKAGE = "hellorust.multiversion")
}

#' @export
#' @rdname hellorust
#' @examples random()
random <- function(){
  .Call("random_wrapper", PACKAGE = "hellorust.multiversion")
}

#' @export
#' @rdname hellorust
#' @examples runthreads()
runthreads <- function(){
  .Call("threads_wapper", PACKAGE = "hellorust.multiversion")
  invisible()
}

#' Count non-zero bytes with Rust SIMD kernels and C dispatch
#'
#' This is a Rust/C/R multiversioning demo. The SIMD kernels and CPU feature
#' probes live in Rust, while the C layer only caches a function pointer and
#' dispatches to the best Rust backend whose compiled feature and runtime CPU
#' feature are both available. The package still builds one R shared library;
#' it does not create one shared library per SIMD backend.
#'
#' @param x A raw vector.
#' @return The number of non-zero bytes in `x`.
#' @export
#' @rdname simd_demo
#' @examples
#' simd_count_nonzero(as.raw(c(0, 1, 2, 0, 3)))
simd_count_nonzero <- function(x) {
  stopifnot(is.raw(x))
  .Call("simd_count_nonzero_wrapper", x, PACKAGE = "hellorust.multiversion")
}

#' Select the Rust SIMD backend
#'
#' `"auto"` selects the fastest available Rust backend from the intersection of
#' compiled features and runtime CPU features. `"scalar"` is always available and
#' is useful for correctness checks and scalar-vs-SIMD benchmarks.
#'
#' @param backend One of `"auto"`, `"scalar"`, `"sse2"`, `"avx2"`,
#'   `"avx512"`, or `"neon"`.
#' @return The selected backend name, invisibly.
#' @export
#' @rdname simd_demo
#' @examples
#' simd_set_backend("scalar")
#' simd_set_backend("auto")
simd_set_backend <- function(backend = c("auto", "scalar", "sse2", "avx2", "avx512", "neon")) {
  backend <- match.arg(backend)
  selected <- .Call("simd_set_backend_wrapper", backend, PACKAGE = "hellorust.multiversion")
  invisible(selected)
}

#' Report the selected Rust SIMD backend
#'
#' @return A named list with character-vector `compiled` and `supported`
#'   backend sets, the runtime `selected` backend, and logical fields for
#'   Rust-reported compiled and CPU features.
#' @export
#' @rdname simd_demo
#' @examples
#' simd_info()
simd_info <- function() {
  .Call("simd_info_wrapper", PACKAGE = "hellorust.multiversion")
}
