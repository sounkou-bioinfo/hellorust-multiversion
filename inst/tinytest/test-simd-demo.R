library(hellorust.multiversion)

x <- as.raw(c(0, 1, 2, 0, 3, 255))
expect_equal(simd_count_nonzero(x), 4L)

simd_set_backend("scalar")
expect_equal(simd_info()$selected, "scalar")
expect_equal(simd_count_nonzero(x), 4L)

simd_set_backend("auto")
expect_equal(simd_count_nonzero(x), 4L)

info <- simd_info()
expect_true(is.list(info))
expected_names <- c(
  "selected", "compiled", "supported",
  "compiled_sse2", "compiled_avx2", "compiled_avx512", "compiled_neon",
  "cpu_sse2", "cpu_avx2", "cpu_avx512", "cpu_neon"
)
expect_true(all(expected_names %in% names(info)))
expect_true(is.character(info$selected) && nzchar(info$selected))
expect_true(is.character(info$compiled) && "scalar" %in% info$compiled)
expect_true(is.character(info$supported) && "scalar" %in% info$supported)
expect_true(info$selected %in% info$compiled)
expect_true(info$selected %in% info$supported)
expect_true(all(vapply(info[expected_names[-(1:3)]], is.logical, logical(1))))

simd_set_backend("auto")
