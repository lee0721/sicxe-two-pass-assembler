# SIC / SICXE Two-Pass Assembler

Single-file C++17 assembler (`code.cpp`) for both SIC and SIC/XE. It performs a classic two-pass flow:
1. **Pass 1**: tokenizes each line, resolves labels, builds the symbol table, and assigns location counters.
2. **Pass 2**: generates object code (handles formats 1–4, addressing flags nixbpe), expands directives, and writes a listing that matches the reference column layout.

Everything lives in `code.cpp` so you can compile and submit a single source file for coursework, while external tables stay as data files.

## Project Layout
- `code.cpp` – full assembler (tokenizer, symbol management, pass 1/2, listing/object output).
- `SICTABLE/` – opcode/directive tables (formerly `Table*.table`).
- `Sic_Instruction_Set.table`, `Sicxe_Instruction_Set.table` – instruction set definitions.
- `test_input&output/` – sample inputs and their reference outputs.
- `sic_lab_testcases/` – additional lab inputs/expected outputs.
- `doc/` – assignment handouts and flowchart (original project docs).

## Build
```bash
g++ -std=c++17 code.cpp -o assembler
```

## Run
```bash
./assembler
```
Menu steps:
1. Enter `1` for SIC or `2` for SICXE.
2. Input path (e.g., `test_input&output/SIC_input.txt`).
3. Output path (e.g., `test_input&output/SIC_output_generated.txt`).

## Quick Verification
- SIC: `diff -u test_input&output/SIC_correct_output.txt test_input&output/SIC_output_generated.txt`
- SICXE: `diff -u test_input&output/SICXE_correct_output.txt test_input&output/SICXE_output_generated.txt`

## Notes
- Keep the tables in place (`SICTABLE/`, `Sic_Instruction_Set.table`, `Sicxe_Instruction_Set.table`).
- Listing formatting matches the provided reference outputs column-for-column.

## License
MIT License (see `LICENSE`).
