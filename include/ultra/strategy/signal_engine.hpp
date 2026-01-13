#pragma once
#include "../core/simd_utils.hpp"
#include <vector>
#include <algorithm>

namespace ultra::strategy {

/**
 * Vectorized Signal Engine
 * Computes indicators on batches of data using AVX2.
 */
class SignalEngine {
public:
    // Simple Moving Average
    static std::vector<double> sma(const std::vector<double>& prices, int window) {
        std::vector<double> result(prices.size());
        SIMDUtils::compute_rolling_mean(prices.data(), prices.size(), window, result.data());
        return result;
    }

    // Relative Strength Index (Vectorized)
    static std::vector<double> rsi(const std::vector<double>& prices, int period = 14) {
        size_t n = prices.size();
        if (n <= static_cast<size_t>(period)) return std::vector<double>(n, 50.0);

        std::vector<double> rsi_vec(n);
        std::vector<double> gains(n, 0.0);
        std::vector<double> losses(n, 0.0);

        // 1. Calculate Deltas
        for (size_t i = 1; i < n; ++i) {
            double delta = prices[i] - prices[i - 1];
            if (delta > 0) gains[i] = delta;
            else losses[i] = -delta;
        }

        // 2. Rolling Average of Gains/Losses (Smoothed is standard RSI, here simple for speed/example)
        // Using SMA for simplicity in this vector example, though Wilders is standard.
        // To use SIMDUtils::compute_rolling_mean, we treat it as SMA-RSI
        std::vector<double> avg_gain(n), avg_loss(n);
        SIMDUtils::compute_rolling_mean(gains.data(), n, period, avg_gain.data());
        SIMDUtils::compute_rolling_mean(losses.data(), n, period, avg_loss.data());

        // 3. Compute RS and RSI
        for (size_t i = period; i < n; ++i) {
            double rs = (avg_loss[i] == 0) ? 100.0 : (avg_gain[i] / avg_loss[i]);
            rsi_vec[i] = 100.0 - (100.0 / (1.0 + rs));
        }

        return rsi_vec;
    }
    
    // Generate signals based on MA Crossover (Vectorized)
    // Returns 1 (Buy), -1 (Sell), 0 (Hold)
    static std::vector<int> ma_crossover_signal(const std::vector<double>& fast_ma, const std::vector<double>& slow_ma) {
        size_t n = std::min(fast_ma.size(), slow_ma.size());
        std::vector<int> signals(n, 0);
        
        // This loop can be auto-vectorized by modern compilers (-O3 -mavx2)
        // Explicit intrinsics for int comparison/blending is verbose, trusting compiler here.
        for (size_t i = 1; i < n; ++i) {
            if (fast_ma[i] > slow_ma[i] && fast_ma[i-1] <= slow_ma[i-1]) {
                signals[i] = 1; // Golden Cross
            } else if (fast_ma[i] < slow_ma[i] && fast_ma[i-1] >= slow_ma[i-1]) {
                signals[i] = -1; // Death Cross
            }
        }
        return signals;
    }
};

} // namespace ultra::strategy
