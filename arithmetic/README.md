# MOS Phase-I Simulator (OS Lab)

## Abstract
This project implements a **Phase-I Multiprogramming Operating System (MOS) simulator** typically used in Operating Systems coursework. The simulator models a small, word-addressable main memory, a minimal CPU with registers and an instruction counter, and a simple Monitor (MOS) that services basic I/O interrupts.

The implementation is in C++ ([main.cpp](main.cpp)) and executes jobs provided as a card-based input stream (`input.txt`) using standard OS lab control cards (`$AMJ`, `$DTA`, `$END`).

## Learning Objectives
- Understand the separation of responsibilities between a **loader**, **CPU execution loop**, and **monitor services**.
- Practice designing a tiny **instruction set architecture (ISA)** and interpreting fixed-width instructions.
- Model memory and register state transitions and observe how I/O is abstracted via interrupts.

## System Model
### Memory
- Main memory: `M[100][4]` → **100 words**, each **4 characters**.
- Words are addressed by 2-digit operands (`00`–`99`).
- A “block” concept is used for I/O:
  - `GDxx` and `PDxx` operate on **10 consecutive words** starting at address `xx` (i.e., 40 characters).

### CPU State
- `IR[4]`: Instruction Register (current 4-char instruction)
- `R[4]`: General-purpose register (4 characters)
- `IC`: Instruction Counter (word address)
- `C`: Condition flag (boolean)

### Monitor (MOS)
The monitor is invoked through a software interrupt indicator `SI`:
- `SI = 1` → `READ`  (for `GD`)
- `SI = 2` → `WRITE` (for `PD`)
- `SI = 3` → `TERMINATE` (for `H`)

## Input Format (Card Deck)
The simulator reads from a file named `input.txt` in the project directory.

A job is structured as:
- `$AMJ` — Start of job (initializes memory and registers)
- **Program cards** — Instruction stream (each instruction is 4 characters)
- `$DTA` — Start of data cards (data read by `GDxx`)
- **Data cards** — Each data line may be up to 40 characters
- `$END` — End of job

Notes:
- Program cards can be on one long line or split across multiple lines (see `MPC.txt`).
- Each program card line is loaded into memory in chunks of 10 words (40 chars), aligning to the next 10-word boundary afterward.

## Instruction Set (ISA)
Each instruction is 4 characters:
- **Opcode**: 2 characters (`IR[0]IR[1]`)
- **Operand**: 2 digits (`IR[2]IR[3]`) for memory address-based instructions

### Supported Instructions
| Instruction | Meaning | Behavior |
|---|---|---|
| `GDxx` | Get Data | Reads one data card (≤40 chars) into memory `M[xx..xx+9]` |
| `PDxx` | Put Data | Writes 40 chars from `M[xx..xx+9]` to `output.txt` (skipping `*`) |
| `H` | Halt | Terminates the current job |
| `LRxx` | Load Register | `R ← M[xx]` |
| `SRxx` | Store Register | `M[xx] ← R` |
| `CRxx` | Compare Register | Sets `C = (R == M[xx])` |
| `BTxx` | Branch True | If `C` is true, `IC ← xx` |
| `ADxx` | Add | Treats `R` and `M[xx]` as integers and computes `R ← R + M[xx]` |
| `SBxx` | Subtract | `R ← R - M[xx]` |
| `MLxx` | Multiply | `R ← R * M[xx]` |
| `DVxx` | Divide | `R ← R / M[xx]` (runtime error if divisor is 0) |

### Numeric Encoding Notes (Important)
Arithmetic uses helper conversions in [main.cpp](main.cpp):
- `toInt(char[4])` extracts only digit characters and parses them (non-digits are ignored).
- `toCharArray(int)` stores a result as a **4-character, zero-padded decimal string**, truncating to the last 4 characters if needed.

Implications:
- Negative results are **not represented correctly** (e.g., `SB` that would go negative).
- Overflow beyond 4 digits is truncated.

## Output
- Output is appended to `output.txt`.
- `PDxx` prints up to 40 characters (10 words × 4 chars), skipping filler `*`.
- `H` prints a termination banner to the output file.

Tip: delete `output.txt` between runs if you want clean outputs.

## Build & Run
### Requirements
- macOS/Linux with `g++` (C++17 or newer)

### Compile
```bash
g++ -std=c++17 -O2 -Wall -Wextra main.cpp -o mos
```

### Run with a sample job
Pick one of the provided sample decks and copy it to `input.txt`:
```bash
cp ADD.txt input.txt
./mos
```

The program will:
- print an execution trace and a memory dump to the terminal
- write results to `output.txt`

## Included Sample Job Decks
These `.txt` files are ready-to-run job streams. Use `cp <file> input.txt`.

- `ADD.txt` — Adds two 4-digit numbers (`AD`) and prints the result.
- `SUB.txt` — Subtracts two numbers (`SB`) and prints the result.
- `MUL.txt` — Multiplies two numbers (`ML`) and prints the result.
- `DIV.txt` — Divides two numbers (`DV`) and prints the result.
- `DIV_0.txt` — Division by zero test (should raise a runtime error).
- `COM_E.txt` — Compare-equal path (prints one of the operands depending on branch).
- `COM_NE.txt` — Compare-not-equal path.
- `LRT.txt` / `SRT.txt` — Demonstrates register load/store with simple data.
- `SPC.txt` — Single `PD`/`GD` I/O path for non-numeric data.
- `MDC.txt` — Multiple data cards loaded and printed.
- `MJ.txt` — Multiple jobs in a single input stream.
- `MPC.txt` — Program cards split across multiple lines.
- `LIS.txt` — Longer instruction sequence (e.g., chained arithmetic).
- `II.txt` — Invalid instruction test (should raise an opcode error).

## Known Limitations (Phase-I Scope)
- No paging, no page tables, no job control block fields beyond `$AMJ/$DTA/$END`.
- Minimal error handling (e.g., divide-by-zero and invalid opcode terminate execution).
- `output.txt` is opened in append mode (`ios::app`).

## Repository Contents
- [main.cpp](main.cpp) — MOS Phase-I simulator implementation.
- `*.txt` — Sample job decks for testing.

