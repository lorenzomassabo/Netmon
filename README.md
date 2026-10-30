# Netmon

A lightweight, zero-dependency Linux network connection monitor written in pure C.

`netmon` inspects active TCP sockets by directly parsing the kernel's pseudo-filesystem (`/proc/net/tcp`), decoding connection endpoints, port numbers, and socket states in real time.

---

## Features

- **Direct Kernel Inspection:** Reads connection states natively from `/proc/net/tcp` without relying on external wrappers like `netstat` or `ss`.
- **Low Footprint:** Written in standard C with fixed-size buffers, avoiding unnecessary heap allocations and runtime overhead.
- **Fast Endian-Safe Parsing:** Converts raw little-endian hexadecimal addresses to human-readable IPv4 strings using POSIX socket utilities.
- **Modular Design:** Clear separation between kernel data parsing and presentation logic.

---

## Architecture Overview

1. **Kernel Parsing (`/proc/net/tcp`):** Skips column headers and processes connection rows using positional pattern matching (`sscanf`).
2. **Address & Port Translation:**
   - 32-bit hex IPv4 values are translated into standard dotted-decimal strings (`inet_ntop`).
   - Hexadecimal ports are converted into standard decimal integers.
3. **TCP State Machine Mapping:** Translates numeric hex state codes (e.g., `0x01`, `0x0A`) into RFC-standard labels (`ESTABLISHED`, `LISTEN`, `TIME_WAIT`, etc.).

---

## Requirements

- **Operating System:** Linux (requires `/proc` pseudo-filesystem support)
- **Compiler:** `gcc` or `clang` (POSIX.1-2008 compliant)
- **Build System:** `make`

---

## Building and Running

### Compile

Using `gcc` directly with strict warning flags:

```bash
gcc -Wall -Wextra -pedantic -std=c99 -D_POSIX_C_SOURCE=200809L main.c -o netmon
```

Or using `make` (if a Makefile is configured):

```bash
make
```

### Run

```bash
./netmon
```

---

## Project Structure

```text
.
├── include/
│   └── function.h      # Struct definitions and function prototypes
├── src/
│   ├── main.c          # Entry point and loop orchestration
│   └── parser.c        # /proc parsing and network address conversion
├── Makefile            # Build configuration
└── README.md
```

---

## Roadmap

- [x] Parse IPv4 active TCP sockets (`/proc/net/tcp`)
- [x] Decode network-order IP addresses and port numbers
- [x] Map socket states to human-readable strings
- [ ] Add interactive TUI rendering using `ncurses`
- [ ] Implement asynchronous background fetching with POSIX threads (`pthreads`)
- [ ] Add UDP socket support (`/proc/net/udp`) and IPv6 parsing (`/proc/net/tcp6`)

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
