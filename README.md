# RISC-V Operating System Kernel

A small educational operating system kernel for the 64-bit RISC-V architecture, written in C++ and RISC-V assembly. The project covers low-level execution, memory management, threads, scheduling, and synchronization, and runs in QEMU.

## Project structure

- `src/` — kernel and supporting implementation files
- `h/` — header files
- `lib/` — supporting libraries used by the build
- `kernel.ld` — linker script
- `Makefile` — cross-compilation and QEMU targets

## Kernel extensions

I used the kernel as a base for a series of operating-systems programming exercises. The changes are organized into separate Git branches so that each feature can be inspected independently:

| Branch | Feature |
| --- | --- |
| [`feature/thread-id`](../../tree/feature/thread-id) | Assigning and retrieving thread identifiers |
| [`feature/max-threads`](../../tree/feature/max-threads) | Limiting the number of concurrent threads |
| [`feature/thread-join`](../../tree/feature/thread-join) | Waiting for a thread to finish |
| [`feature/join-all-children`](../../tree/feature/join-all-children) | Waiting for child threads |

The `main` branch contains the base kernel. The feature branches are separate implementations, not a single integrated release. This distinction matters when browsing the code or testing a particular extension.

The work focuses on thread lifecycle management and synchronization: handling shared state, coordinating waiting threads, and checking behavior when execution order changes.

## Building and running

The Makefile targets a RISC-V 64-bit toolchain and `qemu-system-riscv64`. On a Linux environment with the required tools installed:

```bash
make
make qemu
```

The build attempts to detect a suitable RISC-V cross-compiler. If detection fails, specify its prefix explicitly, for example:

```bash
make TOOLPREFIX=riscv64-linux-gnu-
```

For debugging, the Makefile also provides:

```bash
make qemu-gdb
```

The kernel is launched on QEMU's `virt` machine with 128 MB of memory and one CPU core. Build and execution depend on the toolchain and the supporting libraries included in the repository.

## Development notes

This repository documents coursework and independent kernel extensions. It is intended as a learning project rather than a production operating system. The most useful parts of the work were implementing small kernel mechanisms, tracing incorrect behavior, and revising synchronization logic when initial solutions did not work as expected.
