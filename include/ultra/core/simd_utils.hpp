#pragma once
#include "compiler.hpp"
#include <cstdint>
#include <vector>
#include <cmath>

#if defined(__x86_64__) || defined(_M_X64)
    #include <immintrin.h>
    #define ULTRA_HAS_AVX2 1
#else
    #define ULTRA_HAS_AVX2 0
#endif

namespace ultra {

class SIMDUtils {
public:
    // Rolling Sum (Sliding Window)
    static void compute_rolling_mean(const double* input, size_t n, int window, double* output) {
        if (n < static_cast<size_t>(window)) return;

        double current_sum = 0.0;
        // Initial window
        for (int i = 0; i < window; ++i) current_sum += input[i];
        output[window - 1] = current_sum / window;

        // Sliding
        for (size_t i = window; i < n; ++i) {
            current_sum += input[i] - input[i - window];
            output[i] = current_sum / window;
        }
    }
    
    // Fast array search
    static int find_index_avx2(const int32_t* data, size_t size, int32_t value) {
#if ULTRA_HAS_AVX2
        __m256i target = _mm256_set1_epi32(value);
        size_t i = 0;
        for (; i + 8 <= size; i += 8) {
            __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&data[i]));
            __m256i cmp = _mm256_cmpeq_epi32(chunk, target);
            int mask = _mm256_movemask_ps(_mm256_castsi256_ps(cmp));
            if (mask != 0) {
                return static_cast<int>(i + __builtin_ctz(mask));
            }
        }
        for (; i < size; ++i) {
            if (data[i] == value) return static_cast<int>(i);
        }
        return -1;
#else
        // ARM64 / Generic Fallback
        // Compiler auto-vectorization is usually sufficient here
        for (size_t i = 0; i < size; ++i) {
            if (data[i] == value) return static_cast<int>(i);
        }
        return -1;
#endif
    }
    
    // Calculate Standard Deviation
    static double calculate_std_dev(const std::vector<double>& returns) {
        size_t n = returns.size();
        if (n < 2) return 0.0;

        double sum = 0.0;
        for (double r : returns) sum += r;
        double mean = sum / n;

#if ULTRA_HAS_AVX2
        __m256d v_mean = _mm256_set1_pd(mean);
        __m256d v_ssd = _mm256_setzero_pd();
        
        size_t i = 0;
        for (; i + 4 <= n; i += 4) {
            __m256d v_val = _mm256_loadu_pd(&returns[i]);
            __m256d diff = _mm256_sub_pd(v_val, v_mean);
            v_ssd = _mm256_add_pd(v_ssd, _mm256_mul_pd(diff, diff));
        }
        
        double ssd_arr[4];
        _mm256_storeu_pd(ssd_arr, v_ssd);
        double ssd = ssd_arr[0] + ssd_arr[1] + ssd_arr[2] + ssd_arr[3];

        for (; i < n; ++i) {
            double diff = returns[i] - mean;
            ssd += diff * diff;
        }
#else
        double ssd = 0.0;
        // Logic for auto-vectorization
        for (size_t i = 0; i < n; ++i) {
            double diff = returns[i] - mean;
            ssd += diff * diff;
        }
#endif
        return std::sqrt(ssd / (n - 1));
    }

    /**
     * Compares two arrays and returns a mask where a[i] > b[i]
     */
    static void batch_compare_gt(const double* a, const double* b, size_t n, uint8_t* result_mask) {
#if ULTRA_HAS_AVX2
        size_t i = 0;
        for (; i + 4 <= n; i += 4) {
            __m256d va = _mm256_loadu_pd(&a[i]);
            __m256d vb = _mm256_loadu_pd(&b[i]);
            __m256d cmp = _mm256_cmp_pd(va, vb, _CMP_GT_OQ);
            
            // Extract mask to result_mask[i...i+3]
            // cmp is all 1s (true) or 0s (false)
            double tmp[4];
            _mm256_storeu_pd(tmp, cmp);
            for (int j = 0; j < 4; ++j) {
                result_mask[i + j] = (tmp[j] != 0);
            }
        }
        for (; i < n; ++i) {
            result_mask[i] = (a[i] > b[i]);
        }
#else
        for (size_t i = 0; i < n; ++i) {
            result_mask[i] = (a[i] > b[i]);
        }
#endif
    }
};

} // namespace ultra