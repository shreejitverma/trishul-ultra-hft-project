#pragma once
#include "../core/simd_utils.hpp"
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace ultra::strategy {

struct PerformanceReport {
    double total_return;
    double cagr;
    double sharpe_ratio;
    double sortino_ratio;
    double max_drawdown;
    double win_rate;
    size_t total_trades;
    
    void print() const {
        std::cout << "\n=== PERFORMANCE REPORT ===" << std::endl;
        std::cout << "Total Return  : " << std::fixed << std::setprecision(2) << total_return * 100 << "%" << std::endl;
        std::cout << "CAGR          : " << cagr * 100 << "%" << std::endl;
        std::cout << "Sharpe Ratio  : " << std::setprecision(4) << sharpe_ratio << std::endl;
        std::cout << "Sortino Ratio : " << sortino_ratio << std::endl;
        std::cout << "Max Drawdown  : " << std::setprecision(2) << max_drawdown * 100 << "%" << std::endl;
        std::cout << "Win Rate      : " << win_rate * 100 << "%" << std::endl;
        std::cout << "Total Trades  : " << total_trades << std::endl;
        std::cout << "==========================\n" << std::endl;
    }
};

class PerformanceAnalyst {
public:
    static PerformanceReport analyze(const std::vector<double>& equity_curve, double risk_free_rate = 0.0) {
        PerformanceReport report{};
        size_t n = equity_curve.size();
        if (n < 2) return report;

        double initial_equity = equity_curve.front();
        double final_equity = equity_curve.back();
        
        // 1. Returns
        report.total_return = (final_equity - initial_equity) / initial_equity;
        
        // CAGR (Assuming each point is a trade/tick, simpler to annualized if we know frequency)
        // Let's assume the series covers 'T' years. Without time data, we just return total return roughly.
        // We'll leave CAGR as simplified annualized return assuming n points = 1 year (usually 252 days)
        report.cagr = report.total_return; // Placeholder if time unknown

        // 2. Returns Series
        std::vector<double> returns;
        returns.reserve(n-1);
        for (size_t i = 1; i < n; ++i) {
            returns.push_back((equity_curve[i] - equity_curve[i-1]) / equity_curve[i-1]);
        }

        // 3. Sharpe Ratio
        // Mean / StdDev
        double mean_ret = 0;
        for (double r : returns) mean_ret += r;
        mean_ret /= returns.size();
        
        double std_dev = SIMDUtils::calculate_std_dev(returns);
        if (std_dev > 0) {
            // Annualize (assuming daily data 252) -> sqrt(252) * mean/std
            // For HFT (tick data), annualization factor is massive. We report raw Sharpe per period.
            report.sharpe_ratio = (mean_ret - risk_free_rate) / std_dev; 
        }

        // 4. Sortino Ratio (Downside Deviation)
        std::vector<double> neg_returns;
        for (double r : returns) {
            if (r < 0) neg_returns.push_back(r);
        }
        double downside_std = 0;
        if (!neg_returns.empty()) {
            // Simple RMS of negative returns
            double sum_sq = 0;
            for(double r : neg_returns) sum_sq += r*r;
            downside_std = std::sqrt(sum_sq / neg_returns.size());
        }
        
        if (downside_std > 0) {
            report.sortino_ratio = (mean_ret - risk_free_rate) / downside_std;
        }

        // 5. Max Drawdown
        double peak = -1e9;
        double max_dd = 0;
        for (double val : equity_curve) {
            if (val > peak) peak = val;
            double dd = (peak - val) / peak;
            if (dd > max_dd) max_dd = dd;
        }
        report.max_drawdown = max_dd;
        
        // 6. Win Rate (Derived from trades, approximated here by positive return periods)
        size_t wins = 0;
        for (double r : returns) if (r > 0) wins++;
        report.win_rate = static_cast<double>(wins) / returns.size();
        report.total_trades = returns.size(); // Approximation

        return report;
    }
};

} // namespace ultra::strategy
