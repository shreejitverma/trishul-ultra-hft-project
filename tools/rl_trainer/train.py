import gymnasium as gym
from stable_baselines3 import PPO
from stable_baselines3.common.env_checker import check_env
from environment import MarketMakingEnv
import os

def train():
    # 1. Create Environment
    env = MarketMakingEnv()
    
    # 2. Check Environment
    print("Checking environment compatibility...")
    check_env(env)
    print("Environment is valid.")
    
    # 3. Define Agent (PPO)
    model = PPO(
        "MlpPolicy", 
        env, 
        verbose=1,
        learning_rate=0.0003,
        n_steps=2048,
        batch_size=64,
        gamma=0.99
    )
    
    # 4. Train
    print("Starting training...")
    model.learn(total_timesteps=10000)
    print("Training complete.")
    
    # 5. Save Model
    save_path = "models/ppo_market_maker"
    os.makedirs(os.path.dirname(save_path), exist_ok=True)
    model.save(save_path)
    print(f"Model saved to {save_path}")

if __name__ == "__main__":
    train()
