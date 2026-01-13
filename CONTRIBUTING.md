# Contributing to Trishul Ultra-HFT

Thank you for your interest in contributing! This project aims to be the fastest open-source HFT engine.

## Coding Standards

1.  **C++ Standard:** Use C++20 features where appropriate (Concepts, `std::bit_cast`).
2.  **Zero Allocation:** NEVER use `new`, `malloc`, or standard containers (`std::vector`, `std::map`) in the hot path (MD, Strategy, Exec threads). Use `ObjectPool` or `std::array`.
3.  **Thread Safety:** Use `std::atomic` with explicit memory ordering (`acquire`/`release`) for inter-thread communication. Avoid `std::mutex` in the hot path.
4.  **Formatting:** Follow the existing indentation (4 spaces).

## Workflow

1.  **Fork** the repository.
2.  **Create a branch** for your feature (`feature/new-indicator`).
3.  **Implement** your changes.
4.  **Add Tests:** Ensure you add unit tests in `tests/unit/`.
5.  **Benchmark:** Run `latency_bench` to ensure no regressions.
6.  **Submit PR:** Describe your changes and attach benchmark results.

## Running Tests

```bash
cd build
cmake ..
make -j
./test_ai_strategy
```
