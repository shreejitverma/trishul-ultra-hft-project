import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

def generate_market_data(n_steps=10000):
    dt = 1/252/6.5/3600 
    s0 = 100.0
    prices = [s0]
    
    current_regime = 0
    vol_map = {0: 0.1, 1: 0.25, 2: 0.8}
    
    for _ in range(n_steps):
        # Markov regime switching
        if current_regime == 0:
            current_regime = np.random.choice([0, 1], p=[0.999, 0.001])
        elif current_regime == 1:
            current_regime = np.random.choice([0, 1, 2], p=[0.002, 0.996, 0.002])
        else:
            current_regime = np.random.choice([1, 2], p=[0.01, 0.99])
            
        sigma = vol_map[current_regime]
        dw = np.random.normal(0, np.sqrt(dt))
        
        # Mean reversion + Jumps
        ds = 0.1 * (100.0 - prices[-1]) * dt + prices[-1] * sigma * dw
        if current_regime == 2 and np.random.random() < 0.01:
            ds += prices[-1] * np.random.normal(0, 0.005)
            
        prices.append(prices[-1] + ds)
        
    return np.array(prices)

def simulate_hft(prices, latency_steps, adaptive):
    cash = 100000.0
    inventory = 0
    pnl = [0.0]
    
    for i in range(1, len(prices)):
        # 1. Perception: We see price from 'latency_steps' ago
        obs_idx = max(0, i - 1 - latency_steps)
        obs_price = prices[obs_idx]
        
        # 2. Strategy
        if adaptive:
            # Estimate vol locally
            window = prices[max(0, obs_idx-100):obs_idx+1]
            vol = np.std(np.diff(window)) * 100 if len(window) > 1 else 0.1
            spread = obs_price * (vol * 0.001) + 0.002
        else:
            spread = 0.005
            
        my_bid = obs_price - spread
        my_ask = obs_price + spread
        
        # 3. Execution (The "Low Latency" advantage)
        # If latency is high, by the time our order is LIVE, the price has moved.
        # We only fill if the CURRENT price 'i' is within our stale quotes.
        
        # Toxic Fill: Market moves THROUGH our quote before we can cancel
        if prices[i] < my_bid: # Price dropped below our old bid
            inventory += 10
            cash -= my_bid
        elif prices[i] > my_ask: # Price rose above our old ask
            inventory -= 10
            cash += my_ask
            
        # Inventory Management
        if abs(inventory) > 100:
            # Liquidate at CURRENT price (Slippage)
            if inventory > 0:
                cash += prices[i] * 0.9999 * inventory
            else:
                cash -= abs(prices[i] * 1.0001 * inventory)
            inventory = 0
            
        mtm = cash + inventory * prices[i] - 100000.0
        pnl.append(mtm)
        
    return np.array(pnl)

def run():
    np.random.seed(123)
    prices = generate_market_data(50000)
    
    # Project: Latency 1 (100us)
    p1 = simulate_hft(prices, latency_steps=1, adaptive=True)
    
    # Traditional: Latency 100 (10ms)
    p2 = simulate_hft(prices, latency_steps=100, adaptive=True)
    
    # Static: Latency 1, No Adaptive
    p3 = simulate_hft(prices, latency_steps=1, adaptive=False)
    
    print(f"Project: {p1[-1]:.2f}")
    print(f"Traditional: {p2[-1]:.2f}")
    print(f"Static: {p3[-1]:.2f}")
    
    # Calculate Drawdown
    dd1 = np.maximum.accumulate(p1) - p1
    dd2 = np.maximum.accumulate(p2) - p2
    dd3 = np.maximum.accumulate(p3) - p3
    
    df = pd.DataFrame({'Tick': range(len(p1)), 'Project': p1, 'Traditional': p2, 'Static': p3})
    df.iloc[::200, :].to_csv('pnl_comparison.csv', index=False)
    
    df_dd = pd.DataFrame({'Tick': range(len(p1)), 'Project': dd1, 'Traditional': dd2, 'Static': dd3})
    df_dd.iloc[::200, :].to_csv('drawdown_comparison.csv', index=False)

if __name__ == "__main__":
    run()
