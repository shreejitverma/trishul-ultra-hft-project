import logging
import time
import functools
import os
from datetime import datetime
from typing import Callable, Any, Optional

import logging
import time
import functools
import os
import json
import traceback
import contextvars
from datetime import datetime
from typing import Callable, Any, Optional, Dict, List

# Context variables for global request/user ID tracking
context_request_id = contextvars.ContextVar("request_id", default=None)
context_user_id = contextvars.ContextVar("user_id", default=None)

class MicrosecondFormatter(logging.Formatter):
    """
    Custom formatter that supports microseconds (%f) in the datefmt.
    """
    def formatTime(self, record, datefmt=None):
        ct = datetime.fromtimestamp(record.created)
        if datefmt:
            return ct.strftime(datefmt)
        else:
            return ct.strftime("%Y-%m-%d %H:%M:%S.%f")

class JsonFormatter(logging.Formatter):
    """
    Structured JSON formatter for production-ready logs.
    """
    def format(self, record: logging.LogRecord) -> str:
        log_data = {
            "timestamp": datetime.fromtimestamp(record.created).strftime("%Y-%m-%dT%H:%M:%S.%f"),
            "level": record.levelname,
            "logger": record.name,
            "message": record.getMessage(),
            "request_id": context_request_id.get(),
            "user_id": context_user_id.get(),
        }
        
        # Add additional fields if they exist in record.__dict__
        if hasattr(record, "func_name"): log_data["function"] = record.func_name
        if hasattr(record, "error_code"): log_data["error_code"] = record.error_code
        if hasattr(record, "duration_ms"): log_data["duration_ms"] = record.duration_ms
        if hasattr(record, "stack_trace"): log_data["stack_trace"] = record.stack_trace
        if hasattr(record, "call_stack"): log_data["call_stack"] = record.call_stack
        if hasattr(record, "extra_context"): log_data.update(record.extra_context)
        
        return json.dumps(log_data)

class FlowLogger:
    """
    Best-in-class logging system with execution flow tracing and structured JSON support.
    """
    _depth = 0  
    _call_stack = []
    _default_instance = None

    def __init__(
        self,
        name: str = "HFT-Logger",
        level: str = "INFO",
        log_file: Optional[str] = None,
        json_mode: bool = False,
        timestamp_format: str = "%Y-%m-%d %H:%M:%S.%f"
    ):
        self.name = name
        self.json_mode = json_mode
        self.timestamp_format = timestamp_format
        self.logger = logging.getLogger(name)
        
        numeric_level = getattr(logging, level.upper(), logging.INFO)
        if level.upper() == "WARN": numeric_level = logging.WARNING
        self.logger.setLevel(numeric_level)
        
        if self.logger.hasHandlers():
            self.logger.handlers.clear()

        # Formatter Selection
        if json_mode:
            formatter = JsonFormatter()
        else:
            formatter = MicrosecondFormatter(
                '%(asctime)s | %(levelname)-7s | %(message)s',
                datefmt=self.timestamp_format
            )

        # Console Output
        console_handler = logging.StreamHandler()
        console_handler.setFormatter(formatter)
        self.logger.addHandler(console_handler)

        # File Output
        if log_file:
            try:
                log_dir = os.path.dirname(os.path.abspath(log_file))
                os.makedirs(log_dir, exist_ok=True)
                file_handler = logging.FileHandler(log_file)
                file_handler.setFormatter(formatter)
                self.logger.addHandler(file_handler)
            except Exception as e:
                self.logger.warning(f"Could not open log file '{log_file}': {e}")
        
        if FlowLogger._default_instance is None:
            FlowLogger._default_instance = self

    def log(self, level: str, message: str, func_name: Optional[str] = None, 
            error_code: Optional[str] = None, duration_ms: Optional[float] = None,
            stack_trace: Optional[str] = None, extra: Optional[Dict] = None):
        
        lvl = getattr(logging, level.upper(), logging.INFO)
        if level.upper() == "WARN": lvl = logging.WARNING
        
        # Prepare context for the formatter
        extra_fields = {
            "func_name": func_name,
            "error_code": error_code,
            "duration_ms": duration_ms,
            "stack_trace": stack_trace,
            "call_stack": " -> ".join(FlowLogger._call_stack),
            "extra_context": extra or {}
        }
        
        if not self.json_mode:
            flow_prefix = "  " * FlowLogger._depth
            context_part = f"[{func_name}] " if func_name else ""
            err_part = f"(ERR: {error_code}) " if error_code else ""
            perf_part = f" (took {duration_ms:.2f}ms)" if duration_ms else ""
            stack_part = f" | Path: {' -> '.join(FlowLogger._call_stack)}" if FlowLogger._call_stack else ""
            message = f"{flow_prefix}{context_part}{err_part}{message}{perf_part}{stack_part}"
        
        self.logger.log(lvl, message, extra=extra_fields)

    def set_context(self, request_id: Optional[str] = None, user_id: Optional[str] = None):
        """Sets the global context for current execution flow."""
        if request_id: context_request_id.set(request_id)
        if user_id: context_user_id.set(user_id)

    def debug(self, msg: str, **kwargs): self.log("DEBUG", msg, **kwargs)
    def info(self, msg: str, **kwargs): self.log("INFO", msg, **kwargs)
    def warn(self, msg: str, **kwargs): self.log("WARNING", msg, **kwargs)
    def error(self, msg: str, **kwargs): self.log("ERROR", msg, **kwargs)

def trace(desc: Optional[str] = None, log_params: bool = True, log_result: bool = True):
    """
    Advanced decorator for high-fidelity execution tracing.
    Captures: Purpose, Parameters, Return Values, and Performance.
    """
    def actual_decorator(func: Callable[..., Any]):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            active_logger = FlowLogger._default_instance
            func_name = func.__qualname__
            
            # 1. Format Parameters
            param_str = ""
            if log_params:
                p_list = [repr(a) for a in args]
                p_list += [f"{k}={repr(v)}" for k, v in kwargs.items()]
                param_str = f" | Params: ({', '.join(p_list)})"

            # 2. Log Entry
            purpose = f" [{desc}]" if desc else ""
            if active_logger:
                active_logger.info(f"ENTER ->{purpose}{param_str}", func_name=func_name)
            
            FlowLogger._depth += 1
            FlowLogger._call_stack.append(func_name)
            start_time = time.perf_counter()
            try:
                # 3. Execute
                result = func(*args, **kwargs)
                
                # 4. Log Exit with Result
                elapsed = (time.perf_counter() - start_time) * 1000
                FlowLogger._call_stack.pop()
                FlowLogger._depth -= 1
                
                res_str = f" | Result: {repr(result)}" if log_result else ""
                if active_logger:
                    active_logger.info(f"EXIT  <-{res_str}", func_name=func_name, duration_ms=elapsed)
                return result
            except Exception as e:
                elapsed = (time.perf_counter() - start_time) * 1000
                FlowLogger._call_stack.pop()
                FlowLogger._depth -= 1
                if active_logger:
                    active_logger.error(
                        f"EXCEPTION: {type(e).__name__}: {e}",
                        func_name=func_name,
                        error_code="INTERNAL_ERROR",
                        duration_ms=elapsed,
                        stack_trace=traceback.format_exc()
                    )
                raise
        return wrapper

    # Support @trace, @trace(), or @trace(desc="...")
    if callable(desc):
        f = desc
        desc = None
        return actual_decorator(f)
    return actual_decorator



