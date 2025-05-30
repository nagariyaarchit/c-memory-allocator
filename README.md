# C Memory Allocator – `cmalloc` and `cfree` Implementation

This project implements a custom memory allocator in C that provides functionality similar to `malloc()` and `free()` using `cmalloc()` and `cfree()` over a fixed-size contiguous memory block. The allocator is built from scratch without relying on standard memory allocation functions (except during setup via `make_contiguous()`).

## 🧠 What It Does
- Manages dynamic memory within a single pre-allocated block using linked nodes (`struct cnode`).
- Supports allocation and deallocation of memory chunks with internal bookkeeping.
- Includes debugging support to visualize memory layout (`print_debug()`).

## 📂 Project Structure
├── contiguous.c # Allocator implementation
├── contiguous.h # Allocator interface (header file)
├── test-contiguous.c # Test driver with main()
├── myprogram # Compiled executable (built with clang not included in the repository)
├── tests/
│ ├── public.in # Input for testing
│ └── public.expect # Expected output


---

## 🧠 Key Concepts Demonstrated

- **Manual memory management** without relying on `malloc()` or `free()` internally
- Use of **pointer arithmetic** to calculate gaps and align memory
- Implementation of a **doubly linked list** to track allocations
- Debugging tools via `print_debug()` to visualize memory usage
- Construction of a lightweight simulation of how allocators manage user space memory

---

