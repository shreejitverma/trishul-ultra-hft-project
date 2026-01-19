# Contributing to Trishul Ultra-HFT

Thank you for your interest in contributing to the Trishul Ultra-HFT platform! This project aims to bridge the gap between academic research in AI/FPGA and production-grade HFT systems.

## Getting Started

1.  **Fork the repository** on GitHub.
2.  **Clone your fork** locally:
    ```bash
    git clone https://github.com/YOUR_USERNAME/trishul-ultra-hft-project.git
    cd trishul-ultra-hft-project
    ```
3.  **Build the environment** using Docker (highly recommended):
    ```bash
    docker build -t ultra-hft-env .
    docker run -it --rm -v "$(pwd):/home/builder/project" ultra-hft-env
    ```

## Development Guidelines

### C++ Code Style
*   We use **C++20**.
*   Follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
*   Use `clang-format` (config provided in `.clang-format`) before committing.
*   **Performance is paramount.** Avoid:
    *   Virtual functions in the hot path.
    *   Heap allocations (use `ObjectPool` or `HugePageAllocator`).
    *   `std::shared_ptr` (use unique ownership or raw pointers with care).
    *   Exceptions in the hot path (use error codes or `std::optional`).

### FPGA / Verilog
*   Use **SystemVerilog** (`.sv`) or **Verilog 2001** (`.v`).
*   Modules must have a synchronized reset (`rst`).
*   Follow **AXI-Stream** conventions for data flow (`tdata`, `tvalid`, `tready`, `tlast`).
*   Verify modules with a self-checking testbench in `fpga/tb/`.

### Python / AI
*   Use **Python 3.10+**.
*   Follow PEP 8 style guidelines.
*   Ensure Gymnasium environments pass the `check_env` utility.

## Workflow

1.  Create a new branch for your feature or fix: `git checkout -b feature/my-new-feature`.
2.  Implement your changes.
3.  Add tests (C++ Unit Tests in `tests/`, Verilog Testbenches in `fpga/tb/`).
4.  Verify everything builds and passes:
    ```bash
    cd build && make -j4 && ctest
    ```
5.  Commit your changes with clear messages.
6.  Push to your fork and submit a **Pull Request**.

## Architecture Review
Major architectural changes (e.g., changing the threading model, memory layout, or FPGA interface) should be discussed in an Issue before implementation.

## License
By contributing, you agree that your contributions will be licensed under the MIT License.
