import gymnasium as gym
from stable_baselines3 import PPO
from stable_baselines3.common.env_checker import check_env
from environment import MarketMakingEnv
import os
import argparse
from logger import FlowLogger, trace

@trace("Main training loop")
def train(args):
    # 0. Initialize Logger
    log_file = args.output_file if args.output_file else "/Users/shreejitverma/Documents/GitHub/ultra-hft-project/logs/trainer.json"
    logger = FlowLogger(
        level=args.log_level,
        log_file=log_file,
        json_mode=True
    )
    logger.info("Initializing Training Pipeline")

    # 1. Create Environment
    env = MarketMakingEnv()
    
    # 2. Check Environment
    logger.info("Checking environment compatibility...")
    check_env(env)
    logger.info("Environment is valid.")
    
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
    logger.info("Starting training...")
    model.learn(total_timesteps=args.timesteps)
    logger.info("Training complete.")
    
    # 5. Save Model
    save_path = "models/ppo_market_maker"
    os.makedirs(os.path.dirname(save_path), exist_ok=True)
    model.save(save_path)
    logger.info(f"Model saved to {save_path}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="RL Market Making Trainer")
    parser.add_argument("--timesteps", type=int, default=10000, help="Total training timesteps")
    parser.add_argument("--log-level", type=str, default="INFO", help="Logging level")
    parser.add_argument("--output-file", type=str, default=None, help="File to write logs to")
    
    args = parser.parse_args()
    train(args)
