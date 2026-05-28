use std::os::raw::c_int;
use std::slice;

#[cfg(target_arch = "aarch64")]
use std::arch::aarch64::*;
#[cfg(target_arch = "x86")]
use std::arch::x86::*;
#[cfg(target_arch = "x86_64")]
use std::arch::x86_64::*;

fn count_nonzero_scalar(bytes: &[u8]) -> usize {
    bytes.iter().filter(|&&x| x != 0).count()
}

unsafe fn bytes_from_raw<'a>(ptr: *const u8, len: usize) -> &'a [u8] {
    if len == 0 {
        &[]
    } else {
        slice::from_raw_parts(ptr, len)
    }
}

#[no_mangle]
pub unsafe extern "C" fn hellorust_simd_count_nonzero_scalar(ptr: *const u8, len: usize) -> c_int {
    count_nonzero_scalar(bytes_from_raw(ptr, len)) as c_int
}

#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
#[target_feature(enable = "sse2")]
unsafe fn count_nonzero_sse2(bytes: &[u8]) -> usize {
    let mut count = 0usize;
    let mut i = 0usize;
    let zero = _mm_setzero_si128();

    while i + 16 <= bytes.len() {
        let chunk = _mm_loadu_si128(bytes.as_ptr().add(i) as *const __m128i);
        let eq = _mm_cmpeq_epi8(chunk, zero);
        let zero_mask = _mm_movemask_epi8(eq) as u32;
        count += 16 - zero_mask.count_ones() as usize;
        i += 16;
    }

    count + count_nonzero_scalar(&bytes[i..])
}

#[no_mangle]
pub unsafe extern "C" fn hellorust_simd_count_nonzero_sse2(ptr: *const u8, len: usize) -> c_int {
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    {
        return count_nonzero_sse2(bytes_from_raw(ptr, len)) as c_int;
    }

    #[cfg(not(any(target_arch = "x86", target_arch = "x86_64")))]
    {
        hellorust_simd_count_nonzero_scalar(ptr, len)
    }
}

#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
#[target_feature(enable = "avx2")]
unsafe fn count_nonzero_avx2(bytes: &[u8]) -> usize {
    let mut count = 0usize;
    let mut i = 0usize;
    let zero = _mm256_setzero_si256();

    while i + 32 <= bytes.len() {
        let chunk = _mm256_loadu_si256(bytes.as_ptr().add(i) as *const __m256i);
        let eq = _mm256_cmpeq_epi8(chunk, zero);
        let zero_mask = _mm256_movemask_epi8(eq) as u32;
        count += 32 - zero_mask.count_ones() as usize;
        i += 32;
    }

    count + count_nonzero_scalar(&bytes[i..])
}

#[no_mangle]
pub unsafe extern "C" fn hellorust_simd_count_nonzero_avx2(ptr: *const u8, len: usize) -> c_int {
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    {
        return count_nonzero_avx2(bytes_from_raw(ptr, len)) as c_int;
    }

    #[cfg(not(any(target_arch = "x86", target_arch = "x86_64")))]
    {
        hellorust_simd_count_nonzero_scalar(ptr, len)
    }
}

#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
#[target_feature(enable = "avx512f,avx512bw,avx512vl")]
unsafe fn count_nonzero_avx512(bytes: &[u8]) -> usize {
    let mut count = 0usize;
    let mut i = 0usize;
    let zero = _mm512_setzero_si512();

    while i + 64 <= bytes.len() {
        let chunk = _mm512_loadu_si512(bytes.as_ptr().add(i) as *const __m512i);
        let zero_mask = _mm512_cmpeq_epi8_mask(chunk, zero) as u64;
        count += 64 - zero_mask.count_ones() as usize;
        i += 64;
    }

    count + count_nonzero_scalar(&bytes[i..])
}

#[no_mangle]
pub unsafe extern "C" fn hellorust_simd_count_nonzero_avx512(ptr: *const u8, len: usize) -> c_int {
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    {
        return count_nonzero_avx512(bytes_from_raw(ptr, len)) as c_int;
    }

    #[cfg(not(any(target_arch = "x86", target_arch = "x86_64")))]
    {
        hellorust_simd_count_nonzero_scalar(ptr, len)
    }
}

#[cfg(target_arch = "aarch64")]
unsafe fn count_nonzero_neon(bytes: &[u8]) -> usize {
    let mut count = 0usize;
    let mut i = 0usize;
    let zero = vdupq_n_u8(0);

    while i + 16 <= bytes.len() {
        let chunk = vld1q_u8(bytes.as_ptr().add(i));
        let eq = vceqq_u8(chunk, zero);
        let zero_bits = vaddvq_u8(vcntq_u8(eq)) as usize;
        count += 16 - (zero_bits / 8);
        i += 16;
    }

    count + count_nonzero_scalar(&bytes[i..])
}

#[no_mangle]
pub unsafe extern "C" fn hellorust_simd_count_nonzero_neon(ptr: *const u8, len: usize) -> c_int {
    #[cfg(target_arch = "aarch64")]
    {
        return count_nonzero_neon(bytes_from_raw(ptr, len)) as c_int;
    }

    #[cfg(not(target_arch = "aarch64"))]
    {
        hellorust_simd_count_nonzero_scalar(ptr, len)
    }
}

#[no_mangle]
pub extern "C" fn hellorust_simd_compiled_has_sse2() -> c_int {
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    {
        return 1;
    }
    #[cfg(not(any(target_arch = "x86", target_arch = "x86_64")))]
    {
        0
    }
}

#[no_mangle]
pub extern "C" fn hellorust_simd_compiled_has_avx2() -> c_int {
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    {
        return 1;
    }
    #[cfg(not(any(target_arch = "x86", target_arch = "x86_64")))]
    {
        0
    }
}

#[no_mangle]
pub extern "C" fn hellorust_simd_compiled_has_avx512() -> c_int {
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    {
        return 1;
    }
    #[cfg(not(any(target_arch = "x86", target_arch = "x86_64")))]
    {
        0
    }
}

#[no_mangle]
pub extern "C" fn hellorust_simd_compiled_has_neon() -> c_int {
    #[cfg(target_arch = "aarch64")]
    {
        return 1;
    }
    #[cfg(not(target_arch = "aarch64"))]
    {
        0
    }
}

#[no_mangle]
pub extern "C" fn hellorust_simd_cpu_has_sse2() -> c_int {
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    {
        return if std::is_x86_feature_detected!("sse2") { 1 } else { 0 };
    }
    #[cfg(not(any(target_arch = "x86", target_arch = "x86_64")))]
    {
        0
    }
}

#[no_mangle]
pub extern "C" fn hellorust_simd_cpu_has_avx2() -> c_int {
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    {
        return if std::is_x86_feature_detected!("avx2") { 1 } else { 0 };
    }
    #[cfg(not(any(target_arch = "x86", target_arch = "x86_64")))]
    {
        0
    }
}

#[no_mangle]
pub extern "C" fn hellorust_simd_cpu_has_avx512() -> c_int {
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    {
        return if std::is_x86_feature_detected!("avx512f")
            && std::is_x86_feature_detected!("avx512bw")
            && std::is_x86_feature_detected!("avx512vl")
        {
            1
        } else {
            0
        };
    }
    #[cfg(not(any(target_arch = "x86", target_arch = "x86_64")))]
    {
        0
    }
}

#[no_mangle]
pub extern "C" fn hellorust_simd_cpu_has_neon() -> c_int {
    #[cfg(target_arch = "aarch64")]
    {
        return 1;
    }
    #[cfg(not(target_arch = "aarch64"))]
    {
        0
    }
}
