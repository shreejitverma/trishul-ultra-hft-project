import os
import subprocess
import time
import glob
import re

RTL_DIR = os.path.join(os.path.dirname(__file__), '../../fpga/rtl')
TB_DIR = os.path.join(os.path.dirname(__file__), '../../fpga/tb')
WORK_DIR = os.path.join(os.path.dirname(__file__), 'work')
WAVE_DIR = os.path.join(os.path.dirname(__file__), 'waveforms')

class FPGASimulator:
    def __init__(self):
        os.makedirs(WORK_DIR, exist_ok=True)
        os.makedirs(WAVE_DIR, exist_ok=True)
        self.rtl_files = self._get_rtl_files()
        self.metrics = []

    def _get_rtl_files(self):
        rtl_files = []
        for root, _, files in os.walk(RTL_DIR):
            for file in files:
                if file.endswith('.v') or file.endswith('.sv'):
                    rtl_files.append(os.path.join(root, file))
        return rtl_files

    def _get_testbenches(self):
        return glob.glob(os.path.join(TB_DIR, 'tb_*.v'))

    def _inject_vcd(self, tb_path, tb_name):
        """Injects $dumpfile and $dumpvars into the testbench for waveform generation."""
        with open(tb_path, 'r') as f:
            content = f.read()

        if '$dumpfile' in content:
            return tb_path

        vcd_path = os.path.join(WAVE_DIR, f"{tb_name}.vcd")
        
        # Safe multi-line string substitution
        injection_lines = [
            "    initial begin",
            f'        $dumpfile("{vcd_path}");',
            f'        $dumpvars(0, {tb_name});',
            "    end",
            "endmodule"
        ]
        injection_text = "\\n".join(injection_lines)
        
        # Replace actual newline character without syntax issues
        new_content = content.replace("endmodule", "\n".join(injection_lines))
        
        injected_path = os.path.join(WORK_DIR, f"{tb_name}_injected.v")
        with open(injected_path, 'w') as f:
            f.write(new_content)
            
        return injected_path

    def run_simulation(self, tb_path):
        tb_name = os.path.basename(tb_path).replace('.v', '')
        print(f"--- Running Simulation for {tb_name} ---")
        
        sim_tb_path = self._inject_vcd(tb_path, tb_name)
        out_vvp = os.path.join(WORK_DIR, f"{tb_name}.vvp")

        # 2. HDL Parsing & Logic Synthesis (Compilation via iverilog)
        compile_cmd = ['iverilog', '-g2012', '-o', out_vvp, sim_tb_path] + self.rtl_files
        
        print(f"[{tb_name}] Compiling (Parsing & Synthesis)...")
        start_compile = time.time()
        compile_res = subprocess.run(compile_cmd, capture_output=True, text=True)
        compile_time = time.time() - start_compile

        if compile_res.returncode != 0:
            print(f"[{tb_name}] Compilation FAILED:\n{compile_res.stderr}")
            self.metrics.append({'tb': tb_name, 'status': 'COMPILE_FAIL', 'compile_time': compile_time, 'exec_time': 0, 'vcd': ''})
            return

        # 3. Testbench Execution & Waveform Generation (vvp)
        exec_cmd = ['vvp', out_vvp]
        print(f"[{tb_name}] Executing Simulation & Generating Waveforms...")
        start_exec = time.time()
        exec_res = subprocess.run(exec_cmd, capture_output=True, text=True)
        exec_time = time.time() - start_exec

        output = exec_res.stdout
        status = 'PASS' if 'PASS' in output else ('FAIL' if 'FAIL' in output else 'UNKNOWN')

        print(output)
        print(f"[{tb_name}] Status: {status}")
        print(f"[{tb_name}] Compile Time: {compile_time*1000:.2f} ms | Exec Time: {exec_time*1000:.2f} ms\n")

        self.metrics.append({
            'tb': tb_name, 
            'status': status, 
            'compile_time': compile_time, 
            'exec_time': exec_time,
            'vcd': os.path.join(WAVE_DIR, f"{tb_name}.vcd")
        })

    def print_performance_metrics(self):
        print("=== FPGA Simulator Performance Metrics ===")
        print(f"{'Testbench':<25} | {'Status':<12} | {'Compile (ms)':<15} | {'Exec (ms)':<15} | {'Waveform'}")
        print("-" * 90)
        for m in self.metrics:
            vcd_info = "Generated" if os.path.exists(m['vcd']) and m['status'] != 'COMPILE_FAIL' else "N/A"
            print(f"{m['tb']:<25} | {m['status']:<12} | {m['compile_time']*1000:<15.2f} | {m['exec_time']*1000:<15.2f} | {vcd_info}")

    def run_all(self):
        testbenches = self._get_testbenches()
        if not testbenches:
            print("No testbenches found in", TB_DIR)
            return
            
        for tb in testbenches:
            self.run_simulation(tb)
            
        self.print_performance_metrics()

if __name__ == "__main__":
    try:
        subprocess.run(['iverilog', '-V'], capture_output=True, check=True)
    except FileNotFoundError:
        print("Error: 'iverilog' not found. Please install Icarus Verilog.")
        print("macOS: brew install icarus-verilog")
        print("Linux: sudo apt-get install iverilog")
        exit(1)
        
    sim = FPGASimulator()
    sim.run_all()
