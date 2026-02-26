import unittest
import os
import shutil
import tempfile
import logging
import json
import time
from tools.rl_trainer.logger import FlowLogger, trace

class TestFlowLogger(unittest.TestCase):
    def setUp(self):
        self.test_dir = tempfile.mkdtemp()
        self.log_file = os.path.join(self.test_dir, "test.log")
        # Reset default instance and depth for each test
        FlowLogger._default_instance = None
        FlowLogger._depth = 0

    def tearDown(self):
        shutil.rmtree(self.test_dir)

    def test_log_level_initialization(self):
        fl = FlowLogger(level="DEBUG")
        self.assertEqual(fl.logger.level, logging.DEBUG)
        
        fl2 = FlowLogger(level="INVALID") # Should default to INFO
        self.assertEqual(fl2.logger.level, logging.INFO)

    def test_file_output(self):
        fl = FlowLogger(log_file=self.log_file)
        fl.info("Test message")
        
        # Shutdown handlers to flush
        for handler in fl.logger.handlers:
            handler.close()
            
        self.assertTrue(os.path.exists(self.log_file))
        with open(self.log_file, "r") as f:
            content = f.read()
            self.assertIn("Test message", content)
            self.assertIn("INFO", content)

    def test_trace_decorator(self):
        fl = FlowLogger(level="INFO")
        
        @trace(fl)
        def nested_func():
            fl.info("Inner")
            return "done"

        @trace(fl)
        def outer_func():
            fl.info("Outer")
            nested_func()

        outer_func()
        
        # We can't easily capture console output here without mocking,
        # but we can verify the depth and that it didn't crash.
        self.assertEqual(FlowLogger._depth, 0)

    def test_json_logging(self):
        fl = FlowLogger(log_file=self.log_file, json_mode=True)
        fl.set_context(request_id="REQ-123", user_id="USER-456")
        fl.info("Test message", extra={"custom": "field"})
        
        for handler in fl.logger.handlers:
            handler.close()
            
        with open(self.log_file, "r") as f:
            log_entry = json.loads(f.read())
            self.assertEqual(log_entry["message"], "Test message")
            self.assertEqual(log_entry["level"], "INFO")
            self.assertEqual(log_entry["request_id"], "REQ-123")
            self.assertEqual(log_entry["user_id"], "USER-456")
            self.assertEqual(log_entry["custom"], "field")

    def test_trace_performance_and_error(self):
        fl = FlowLogger(log_file=self.log_file, json_mode=True)
        
        @trace(fl)
        def failing_func():
            time.sleep(0.01)
            raise ValueError("Something went wrong")

        try:
            failing_func()
        except ValueError:
            pass

        for handler in fl.logger.handlers:
            handler.close()

        with open(self.log_file, "r") as f:
            lines = f.readlines()
            # Entry 1: ENTER
            # Entry 2: EXCEPTION/ERROR
            error_entry = json.loads(lines[1])
            self.assertEqual(error_entry["level"], "ERROR")
            self.assertIn("ValueError", error_entry["message"])
            self.assertEqual(error_entry["error_code"], "INTERNAL_ERROR")
            self.assertTrue(error_entry["duration_ms"] >= 10.0)
            self.assertIn("traceback", error_entry["stack_trace"].lower())

if __name__ == "__main__":
    unittest.main()
