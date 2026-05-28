
# hellorust.multiversion

[![R build
status](https://github.com/sounkou-bioinfo/hellorust-multiversion/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/sounkou-bioinfo/hellorust-multiversion/actions/workflows/R-CMD-check.yaml)

> Minimal examples of Rust integration and Rust SIMD dispatch from R

Rust is a modern alternative to C and compiled rust code is ABI
compatible with C. Many Rust libraries include C API headers so that the
compiled rust code can be called from R/C/C++ as if it were C code. This
package shows how to do this. The [r-rust](https://github.com/r-rust)
organization contains several more simple R packages interfacing with
cargo crates.

To learn more about using Rust code in R packages, also have a look at
the [r-rust FAQ](https://github.com/r-rust/faq) and the
[slides](https://jeroen.github.io/erum2018/) about this project
presented at eRum2018!

## Package Structure

Bundle your rust code in the embedded cargo package (see the
`Cargo.toml` file). `configure` generates
[src/Makevars](src/Makevars.in) from `src/Makevars.in` so R
automatically builds the Rust static library and the small C/R shim when
the R package is installed.

    hellorust-multiversion
    ├─ configure            ← checks if 'cargo' is installed
    ├─ src
    │  ├─ myrustlib            ← bundled cargo package with your code
    │  |  ├─ Cargo.toml          ← cargo dependencies and metadata
    │  |  ├─ src                 ← rust source code
    │  |  └─ api.h               ← C headers for exported rust API
    |  |
    │  ├─ Makevars.in       ← Template generated to Makevars by configure
    │  ├─ simd_dispatch.c   ← C function-pointer dispatch over Rust kernels
    │  └─ wrapper.c         ← C/R package entry points
    ├─ DESCRIPTION
    └─ R                    ← Standard R+C stuff

## Rust SIMD kernels with C runtime dispatch

This fork also includes a Rust SIMD multiversioning demo inspired by the
same pattern used in larger R/Rust packages: keep one R shared library,
link one Rust static library into it, and dispatch at runtime without
creating one `.so`/`.dll` per SIMD backend.

The exported R function `simd_count_nonzero()` enters through the C/R
shim, but the SIMD implementation and feature detection live in Rust
using `#[target_feature]`, `std::arch`, and Rust’s runtime
feature-detection macros:

- `src/myrustlib/src/simd.rs`: scalar, SSE2, AVX2, AVX-512, and NEON
  Rust kernels, plus Rust-reported compiled and runtime CPU feature
  checks.
- `src/simd_dispatch.c`: a tiny C dispatcher that selects a Rust
  function pointer from `compiled && cpu` feature checks.
- `src/wrapper.c`: the regular `.Call` wrappers for R.

The C code does not contain SIMD intrinsics and does not perform CPU
probing itself. It only asks Rust which kernels were compiled and which
CPU features are available, then caches the best Rust function pointer
during package initialization. This demonstrates that Rust/C/R can use
dynamic SIMD dispatch while still producing a single installed R shared
library.

``` r
library(hellorust.multiversion)
simd_info()
#> $selected
#> [1] "avx2"
#> 
#> $compiled
#> [1] "scalar" "sse2"   "avx2"   "avx512"
#> 
#> $supported
#> [1] "scalar" "sse2"   "avx2"  
#> 
#> $compiled_sse2
#> [1] TRUE
#> 
#> $compiled_avx2
#> [1] TRUE
#> 
#> $compiled_avx512
#> [1] TRUE
#> 
#> $compiled_neon
#> [1] FALSE
#> 
#> $cpu_sse2
#> [1] TRUE
#> 
#> $cpu_avx2
#> [1] TRUE
#> 
#> $cpu_avx512
#> [1] FALSE
#> 
#> $cpu_neon
#> [1] FALSE
simd_count_nonzero(as.raw(c(0, 1, 2, 0, 3)))
#> [1] 3
```

The backend can also be pinned explicitly. This is useful for
correctness checks and for scalar-vs-SIMD benchmarks. The benchmark
below uses `bench` and includes the setter in each expression so the
selected backend is unambiguous.

``` r
set.seed(1)
x <- as.raw(sample(0:255, 4 * 1024 * 1024, replace = TRUE))

simd_set_backend("scalar")
scalar_count <- simd_count_nonzero(x)

simd_set_backend("auto")
auto_backend <- simd_info()$selected
auto_count <- simd_count_nonzero(x)

list(auto_backend = auto_backend, same_count = identical(scalar_count, auto_count))
#> $auto_backend
#> [1] "avx2"
#> 
#> $same_count
#> [1] TRUE

bench::mark(
  scalar = {
    simd_set_backend("scalar")
    simd_count_nonzero(x)
  },
  auto_simd = {
    simd_set_backend("auto")
    simd_count_nonzero(x)
  },
  iterations = 30,
  check = FALSE
)
#> # A tibble: 2 × 6
#>   expression      min   median `itr/sec` mem_alloc `gc/sec`
#>   <bch:expr> <bch:tm> <bch:tm>     <dbl> <bch:byt>    <dbl>
#> 1 scalar        455µs    466µs     2149.        0B        0
#> 2 auto_simd     145µs    146µs     6495.        0B        0

simd_set_backend("auto")
```

## Rust crate vendoring

This fork keeps the upstream `hellorust` CRAN-friendly Rust vendoring
workflow. When a maintainer runs `R CMD build`, the package `cleanup`
script calls:

- `src/myrustlib/vendor-update.sh`, which creates
  `src/myrustlib/vendor.tar.xz` with the Cargo dependency sources.
- `src/myrustlib/vendor-authors.R`, which creates `inst/AUTHORS` for
  dependency attribution.

The resulting R source tarball can then be installed without downloading
Rust crates during package installation. Direct developer installs from
a git checkout (`R CMD INSTALL .`) may use Cargo’s normal registry/cache
path unless a vendored bundle is already present.

## Installing this package

If Rust is available, clone this repository and run the regular
`R CMD INSTALL` command:

    R CMD INSTALL hellorust-multiversion

Alternatively, to download and install from within R itself:

``` r
# install.packages("remotes")
remotes::install_github("sounkou-bioinfo/hellorust-multiversion")
```

## What is Cargo

The standard rust toolchain includes a great package manager `cargo`
with a corresponding registry [crates.io](https://crates.io/). Cargo
makes it very easy to build a rust package including all dependencies
into a static library that can easily be linked into an R package.

This is perfect for R because we can compile and link all rust code at
build-time without any system dependencies. Rust itself has no
substantial runtime so the resulting R package is entirely self
contained. Indeed, rust has been designed specifically to serve well as
an embedded language.

## Installing Rust on Linux / MacOS

Note that `cargo` is only needed at **build-time**. Rust has **no
runtime dependencies**. The easiest way to install the latest version of
Rust (including cargo) is from:
<https://www.rust-lang.org/tools/install>

Alternatively, you may install cargo from your OS package manager:

- Debian/Ubuntu: `sudo apt-get install cargo`
- Fedora/CentOS\*: `sudo yum install cargo`
- MacOS: `brew install rustc`

\*Note that on CentOS you first need to enable EPEL via
`sudo yum install epel-release`.

## Installing Rust for R on Windows

In order for rust to work with R you need to install the toolchain using
`rustup` and then add the `x86_64-pc-windows-gnu` target. First download
[rustup-init.exe](https://win.rustup.rs/) and then install the default
toolchain:

    rustup-init.exe -y --default-host x86_64-pc-windows-gnu

Or if rust is already installed (for example on GitHub actions), you can
simply add the target:

    rustup target add x86_64-pc-windows-gnu

To compile 32bit packages also add the `i686-pc-windows-gnu` target, but
32-bit is no longer supported as of R 4.2.

## GitHub Actions

**Update 2023:** This step is no longer needed because GitHub action
runners now have the required Rust targets preinstalled by default.

To use GitHub actions, you can use the [standard r
workflow](https://github.com/r-lib/actions/blob/HEAD/.github/workflows/check-standard.yaml)
script in combination with this extra step:

    - name: Add Rtools targets to Rust
      if: runner.os == 'Windows'
      run: |
        rustup target add i686-pc-windows-gnu
        rustup target add x86_64-pc-windows-gnu

## In the real world

The [gifski](https://cran.r-project.org/web/packages/gifski/index.html)
package has been on CRAN since 2018, and uses this same structure.

## More Resources

- [r-rust FAQ](https://github.com/r-rust/faq)
- Erum2018 [slides](https://jeroen.github.io/erum2018/) about this
  project presented by Jeroen
- [Rust Inside Other
  Languages](https://doc.rust-lang.org/1.6.0/book/rust-inside-other-languages.html)
  chapter from official rust documentation
- [extendr](https://github.com/extendr): a more advanced R extension
  interface using Rust
- Duncan’s proof of concept:
  [RCallRust](https://github.com/duncantl/RCallRust)
