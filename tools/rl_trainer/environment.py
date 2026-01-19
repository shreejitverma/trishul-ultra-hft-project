import gymnasium as gym
import numpy as np
from gymnasium import spaces

class MarketMakingEnv(gym.Env):
    """
    Custom Environment for Market Making RL Agent.
    Simulates a Limit Order Book (LOB) and executes strategy orders.
    """
    
    def __init__(self, data_feed=None):
        super(MarketMakingEnv, self).__init__()
        
        self.data_feed = data_feed
        self.tick_size = 0.01
        self.max_inventory = 100
        
        # Action Space: [Spread_Bid, Spread_Ask]
        # Normalized between 0 and 1, scaled to ticks later
        self.action_space = spaces.Box(low=0, high=1, shape=(2,), dtype=np.float32)
        
        # Observation Space: [MidPrice, Imbalance, Volatility, Inventory, PnL]
        self.observation_space = spaces.Box(
            low=np.array([0, -1, 0, -self.max_inventory, -np.inf]),
            high=np.array([np.inf, 1, np.inf, self.max_inventory, np.inf]),
            dtype=np.float32
        )
        
        self.reset()

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        
        self.inventory = 0
        self.cash = 0.0
        self.pnl = 0.0
        self.mid_price = 100.00
        self.time_step = 0
        
        return self._get_obs(), {}

    def step(self, action):
        # 1. Parse Action
        spread_bid = int(action[0] * 10) * self.tick_size
        spread_ask = int(action[1] * 10) * self.tick_size
        
        my_bid = self.mid_price - spread_bid
        my_ask = self.mid_price + spread_ask
        
        # 2. Simulate Market Dynamics (Mock)
        # In real training, this comes from 'data_feed'
        price_change = np.random.normal(0, 0.05)
        self.mid_price += price_change
        
        # 3. Execution Logic
        # Simplistic fill probability based on distance to mid
        fill_prob_bid = np.exp(-10 * spread_bid)
        fill_prob_ask = np.exp(-10 * spread_ask)
        
        reward = 0
        
        # Check Fills
        if np.random.random() < fill_prob_bid and self.inventory < self.max_inventory:
            self.inventory += 1
            self.cash -= my_bid
            reward += 0.01 # Rebate
            
        if np.random.random() < fill_prob_ask and self.inventory > -self.max_inventory:
            self.inventory -= 1
            self.cash += my_ask
            reward += 0.01 # Rebate
            
        # 4. Calculate PnL & Reward
        mark_to_market = self.cash + (self.inventory * self.mid_price)
        step_pnl = mark_to_market - self.pnl
        self.pnl = mark_to_market
        
        # Penalize inventory risk
        inventory_penalty = 0.1 * (self.inventory ** 2)
        reward += step_pnl - inventory_penalty
        
        self.time_step += 1
        terminated = self.time_step > 1000
        truncated = False
        
        return self._get_obs(), reward, terminated, truncated, {}

    def _get_obs(self):
        # [MidPrice, Imbalance, Volatility, Inventory, PnL]
        return np.array([
            self.mid_price, 
            0.0, # Mock Imbalance
            0.5, # Mock Volatility
            float(self.inventory),
            self.pnl
        ], dtype=np.float32)
