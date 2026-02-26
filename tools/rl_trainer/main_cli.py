import argparse
import sys
import os

# Add current directory to path so we can import our logger
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from logger import FlowLogger, trace

@trace
def function_b():
    # Use the default singleton instance via FlowLogger._default_instance if needed,
    # but usually we just use a helper or the trace already uses it.
    FlowLogger._default_instance.info("Executing logic in Function B...")
    return 42

@trace
def function_a():
    FlowLogger._default_instance.info("Function A calling Function B...")
    result = function_b()
    FlowLogger._default_instance.info(f"Received result: {result}")

def main():
    parser = argparse.ArgumentParser(description="HFT Project Logging Demo")
    parser.add_argument("--log-level", type=str, default="INFO", choices=["DEBUG", "INFO", "WARN", "ERROR"], help="Set the log level")
    parser.add_argument("--output-file", type=str, help="Path to the log output file")
    parser.add_argument("--timestamp-format", type=str, default="%Y-%m-%d %H:%M:%S.%f", help="strftime format for timestamps")
    
    args = parser.parse_args()

    # Initialize the global FlowLogger
    # The trace decorator will pick this up automatically via FlowLogger._default_instance
    fl = FlowLogger(
        level=args.log_level,
        log_file=args.output_file,
        timestamp_format=args.timestamp_format
    )

    fl.info("CLI Demo Started.")
    
    @trace
    def demo_nested():
        fl.info("Starting nested demo...")
        function_a()
        fl.info("Nested demo complete.")

    demo_nested()
    fl.info("CLI Demo Finished.")

if __name__ == "__main__":
    main()
