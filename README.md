# mini_top

## mini_top
A small, educational `top`-like process monitor written in portable C (Linux & macOS).
It demonstrates POSIX programming patterns: reading process information, a background CPU-sampling thread (pthreads), and a simple `ncurses` text UI.
This project is intended as a learning / portfolio project — compact, well-documented, and easy to extend.

## Features
- Cross-platform process listing:
  - Linux: reads `/proc` to obtain PID, name, memory.
  - macOS: uses `libproc` APIs to obtain PID, name, memory.
- Background CPU sampler implemented with POSIX threads (`pthreads`) that computes per-process CPU usage over time.
- Simple `ncurses` UI with non-blocking input and a q key to quit.
- Clean, modular code:
    - `proc_parser.{c,h}` — platform-specific process reading
    - `cpu_sampler.{c,h}` — CPU sampling thread and bookkeeping
    - `main.c` — ncurses UI and program entry
- Makefile to build on Linux & macOS (with common tweaks for Homebrew ncurses on macOS).
- Educational: minimal third-party dependencies; easy to read and extend.

## Repository layout
```text
mini_top/<br>
├── src/<br>
│   ├── main.c<br>
│   ├── proc_parser.c<br>
│   ├── proc_parser.h<br>
│   ├── cpu_sampler.c<br>
│   └── cpu_sampler.h<br>
├── Makefile<br>
└── README.md<br>
```

## Dependencies
- POSIX environment (Linux / macOS)
- `gcc` or `clang`
- `make`
- `ncurses` development headers & library
- `pthread` (linked via `-pthread`)

## Installing on Linux (Debian/Ubuntu)
```
sudo apt update
sudo apt install build-essential libncurses5-dev libncursesw5-dev
```

## Installing on macOS (Homebrew)
```
brew install ncurses
# You may need to adjust CFLAGS/LDFLAGS to match Homebrew's location:
# e.g. -I/usr/local/opt/ncurses/include -L/usr/local/opt/ncurses/lib
```

## Build
A simple Makefile is provided.

## Default (Linux)
```
make
```

## macOS (Homebrew ncurses)
If make fails to find ncurses headers/libs, build with explicit flags:
```
# Example if Homebrew installs to /usr/local (adjust if your brew prefix differs)
export NCURSES_INC=$(brew --prefix ncurses)/include
export NCURSES_LIB=$(brew --prefix ncurses)/lib

make CFLAGS="-I${NCURSES_INC}" LDFLAGS="-L${NCURSES_LIB} -lncurses"
```
Or edit the Makefile to add the include/lib paths for ncurses on macOS.

## Run
```
./mini_top
```
## Controls:
`q` — quit and cleanly stop background threads.<br>
`Ctrl+C` — sends SIGINT; the program attempts to exit cleanly, but prefer q to ensure graceful thread join and ncurses cleanup.

## Behavior & Implementation notes
- CPU calculation
  The CPU sampler thread periodically (default 1 second) samples per-process CPU time (platform-specific source). It computes a delta and converts it to `% CPU` for the sampling interval. The reported percent is per-process percentage of a single core by default (not normalized across number of cores) — you can divide by the core count to get total-CPU-normalized values.
- Threading and shutdown
  A global `volatile int running` flag is used to signal the sampler thread to stop. `main()` joins the sampler thread before exiting, and `ncurses` is cleaned up via `endwin()`.
- Platform differences
  - Linux: reads `/proc` directories and `/proc/<pid>/stat` / `/proc/<pid>/statm`.
  - macOS: uses `proc_listpids` and `proc_pidinfo` (requires `libproc`).
  - Because macOS has no `/proc`, some fields may be less precise or require different system calls.
- Permissions
  Some process info may be restricted (e.g., processes owned by other users). The program ignores processes it cannot read; run as a privileged user if you want to see all processes.

## Troubleshooting
-  Linker errors for ncurses on macOS: install ncurses with Homebrew and pass the include/lib path to `make` as described above, or update the Makefile.
-  Undefined symbols for pthreads: ensure `-pthread` is in `CFLAGS` (Makefile includes it).
-  Huge CPU numbers: if you see very large CPU values, the sampler might be using raw CPU-time units instead of deltas — ensure the sampler computes delta between samples and divides by the interval (and optionally by `1e6` if converting from microseconds).
-  Program leaves the terminal garbled after crash: press `reset` or `stty sane`, but to avoid this, quit with `q`, and the program will call `endwin()` to restore terminal state.

## Development & extension ideas
-  Normalize CPU% across number of cores (divide by `sysconf(_SC_NPROCESSORS_ONLN)`).
-  Add sorting (by CPU, memory, PID, name) and toggles (like real `top`).
-  Add process filtering (by user, name) and live searching.
-  Replace `napms()` with high-resolution timers for smoother refresh (e.g., `nanosleep` or `poll`).
-  Add a thread-safe process table (mutex) so the sampler and UI coordinate updates more robustly.
-  Display additional columns: command line, elapsed time, thread count, I/O metrics.
-  Add a "follow PID" mode or a per-process detailed view.
-  Add packaging: a `deb`/`rpm` or a Dockerfile that runs the program inside an Ubuntu container.

## Debugging & profiling tips
-  Use `gdb` or `lldb` to set breakpoints and step through the sampler and parser code.
-  Use `valgrind --tool=memcheck ./mini_top` (Linux) to catch memory leaks. Note: Valgrind may not be available or fully supported on recent macOS/Apple Silicon.
-  Use `strace` (Linux) or `dtruss`/`dtrace` (macOS) to see system calls from the program.
-  Add `-DDEBUG` and debug printf statements under `#ifdef DEBUG`.

## License
This project is licensed under the MIT License.  
See the [LICENSE](./LICENSE) file for details.

## Contributing
Contributions, bug reports, and suggested improvements are welcome.
Suggested workflow:
1. Fork the repo
2. Create a feature branch
3. Implement & test on both Linux and macOS when possible
4. Submit a pull request with a clear description and rationale
