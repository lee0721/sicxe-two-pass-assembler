# SIC / SICXE Two-Pass Assembler

A C++17 implementation of a two-pass assembler that targets both SIC and SIC/XE. It reads assembly source, resolves symbols, emits listings, and produces object code that matches the reference outputs used in the original course project.

## Project Layout
- `code.cpp` – single-file implementation of the two-pass assembler (SIC + SICXE).
- `SICTABLE/` – opcode and directive tables for SIC/SICXE (formerly `Table*.table`).
- `Sic_Instruction_Set.table`, `Sicxe_Instruction_Set.table` – instruction set definitions.
- `test_input&output/` – sample inputs/expected outputs for quick checks.
- `sic_lab_testcases/` – additional lab test cases.
- `doc/` – assignment handouts and flowchart for reference.

## Build
```bash
g++ -std=c++17 code.cpp -o assembler
```

## Run
```bash
./assembler
```
Follow the menu prompts:
1. Choose `1` for SIC or `2` for SICXE.
2. Provide input file path (e.g., `test_input&output/SIC_input.txt`).
3. Provide output file path (e.g., `test_input&output/SIC_output_generated.txt`).

The program writes a formatted listing with locations and object code to the specified output file.

## Quick Verification
- SIC: compare `test_input&output/SIC_output_generated.txt` with `test_input&output/SIC_correct_output.txt`.
- SICXE: compare `test_input&output/SICXE_output_generated.txt` with `test_input&output/SICXE_correct_output.txt`.

```bash
diff -u test_input&output/SIC_correct_output.txt test_input&output/SIC_output_generated.txt
diff -u test_input&output/SICXE_correct_output.txt test_input&output/SICXE_output_generated.txt
```

## Notes
- The assembler assumes all supporting tables remain in the repo root (`SICTABLE/`, `Sic_Instruction_Set.table`, `Sicxe_Instruction_Set.table`).
- Object code formatting now matches the provided reference listings column-for-column.
