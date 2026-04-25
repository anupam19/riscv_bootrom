# RV32I Base Integer Instruction Set (Version 2.1) — Technical Analysis

**Architect:** Anupam Datta  
**Date:** 2026-04-25  
**Scope:** Deep technical analysis of RV32I ISA architecture, microarchitecture, compiler implications, and design tradeoffs

---

## High-Level Overview

RV32I is the mandatory base integer instruction set of the RISC-V architecture. It defines a **32-bit flat address space** with **32 general-purpose integer registers** (each 32 bits wide), a strict **load-store architecture**, and **fixed-length 32-bit instructions**. The design philosophy prioritizes:

1. **Hardware simplicity** — minimal viable instruction set enables low-area, high-frequency implementations
2. **Compiler-friendly** — regular instruction formats, abundant registers, orthogonal operations
3. **Scalable** — minimal base allows optional extensions (M/A/F/D/C/etc.)
4. **No implementation-defined behaviors** — architectural spec leaves no ambiguity to software

RV32I is the **only** mandatory subset of RISC-V; all other extensions (M multiply/divide, A atomic, F single-precision float, D double-precision float, G general [imafd], C compressed) are optional. A compliant RV32I core may implement *only* the I-base and nothing else.

**Key parameters:**

| Property | Value |
|---|---|
| XLEN | 32 bits |
| Registers | 32 (x0–x31), each 32 bits |
| Instruction length | Fixed 32 bits |
| Endianness | Little-endian (LEON3/Berkeley correct), byte-address invariant |
| Virtual memory | None (no MMU, no MPU in base I) |
| Privilege modes | M (Machine) only in pure I; U/S optional |
| Trap model | Synchronous exceptions & interrupts via MTVAL/MEPC/MCAUSE CSR (part of "I" system registers) |

---

## Programmer’s Model

### Register File (x0–x31, pc)

| Register | ABI Name | Purpose | Call-preserved? |
|---|---|---|---|
| x0 | zero | Hardwired constant 0 | N/A |
| x1 | ra | Return address (JAL, JALR) | Clobbered |
| x2 | sp | Stack pointer | Callee-saved |
| x3 | gp | Global pointer (optional, linker-defined) | Callee-saved |
| x4 | tp | Thread pointer (optional) | Callee-saved |
| x5 | t0 | Temporary / alternate link register | Clobbered |
| x6–7 | t1–t2 | Temporaries | Clobbered |
| x8 | s0/fp | Saved register / frame pointer | Callee-saved |
| x9 | s1 | Saved register | Callee-saved |
| x10–11 | a0–a1 | Function arguments / return values | Clobbered |
| x12–17 | a2–a7 | Function arguments | Clobbered |
| x18–27 | s2–s11 | Saved registers | Callee-saved |
| x28–31 | t3–t6 | Temporaries | Clobbered |
| pc | — | Program counter (not directly accessible) | N/A |

**Design notes:**

- **x0 hardwired zero:** Eliminates need for `mov` instructions (copy via `addi rd, x0, rs1` or `or rd, x0, rs1`). Any write to x0 is discarded; reads always return 0.
- **No dedicated link/stack registers:** RA (x1) and SP (x2) are **architecturally general-purpose** but software-convention assigns them. This keeps ISA minimal; software conventions (ABI) define usage.
- **pc is implicit:** Not a register file entry. JAL/JALR write PC+4 (or PC+imm) to rd, but reading PC requires AUIPC/JALR trick.
- **Calling convention:** 8 argument registers (a0–a7), allowing up to 8 integer args without stack spills in simple C. s0–s11 preserved across calls; t0–t6 are caller-saved.
- **gp (x3) & tp (x4):** Optional linker-managed pointers for position-independent code (PIC) and thread-local storage. Not required in bare-metal.

---

## Instruction Formats (Bit-Level Breakdown)

All instructions are **32 bits** wide. Four primary formats:

### R-type — Register-Register Operations

```
 31    25 24    20 19    15 14    12 11     7 6      0
+--------+--------+--------+--------+--------+--------+
|  funct7 |  rs2   |  rs1   | funct3 |   rd   | opcode |
+--------+--------+--------+--------+--------+--------+
```

- `opcode`: 7 bits (identifies instruction class)
- `rd`: 5 bits (destination register)
- `funct3`: 3 bits (sub-function selector)
- `rs1`: 5 bits (first source register)
- `rs2`: 5 bits (second source register)
- `funct7`: 7 bits (additional sub-function, often sign-extension of funct3 for arithmetic)

**Example:** `ADD x3, x4, x5` → opcode=OP (0x33), rd=3, funct3=0, rs1=4, rs2=5, funct7=0

### I-type — Register-Immediate Operations

```
 31    20 19    15 14    12 11     7 6      0
+------------------+--------+--------+--------+
|     imm[11:0]    |  rs1   | funct3 |   rd   | opcode |
+------------------+--------+--------+--------+
```

- `imm[11:0]`: 12-bit signed immediate (sign-extended to XLEN)
- Stored in bits [31:20] of instruction word

**Used by:** ADDI, SLTI, ANDI, ORI, XORI, SLLI, SRLI, SRAI, LB/LH/LW, JALR

### S-type — Store Operations

```
 31    25 24    20 19    15 14    12 11     7 6      0
+--------+--------+--------+--------+--------+--------+
| imm[11:5] |  rs2   |  rs1   | funct3 | imm[4:0] | opcode |
+--------+--------+--------+--------+--------+--------+
```

- `imm[11:5]` in bits [31:25]
- `imm[4:0]` in bits [11:7] (non-contiguous!)
- Effective immediate = {imm[11:5], imm[4:0]} sign-extended

**Used by:** SB, SH, SW

### B-type — Conditional Branch Operations

```
 31     30 29    25 24    20 19    15 14    12 11     8 7      0
+----+--------+--------+--------+--------+--------+--------+
| 13 | imm[11|  rs2   |  rs1   | funct3 | imm[4:1] | opcode |
+----+--------+--------+--------+--------+--------+--------+
```

- 12-bit immediate, but **non-contiguous** bits
- Encoding: imm[12|10:5|4:1|11] → {imm[12], imm[10:5], imm[4:1], imm[11]}
- Reconstructed as `{imm[12], imm[10:5], imm[4:1], imm[11], 0}` (LSB always 0 → multiples of 2)
- Branch target = PC + sign-extend({imm[12:1], 1'b0})

**Used by:** BEQ, BNE, BLT, BGE, BLTU, BGEU

### U-type — Upper Immediate (LUI, AUIPC)

```
 31             12 11     7 6      0
+-----------------+--------+--------+
|     imm[31:12]  |   rd   | opcode |
+-----------------+--------+--------+
```

- 20-bit immediate placed in bits [31:12]
- Used to construct 32-bit constants: `LUI rd, imm[31:12]` → rd = imm[31:12] << 12
- AUIPC adds current PC: rd = PC + (imm[31:12] << 12)

### J-type — Unconditional Jump (JAL)

```
 31             12 11     7 6      0
+-----------------+--------+--------+
|     imm[31:12]  |   rd   | opcode |
+-----------------+--------+--------+
```

- 20-bit immediate (bits [31:12])
- Encoding: imm[20|10:1|11|19:12] → complex rearrangement
- Target = PC + sign-extend({imm[20], imm[10:1], imm[11], imm[19:12], 1'b0})
- Always odd after encoding (LSB=1 indicates indirect via JALR if needed)

---

## Instruction Categories (with Examples)

### 1. Integer Computational — Register-Immediate

**ADDI** — Add Immediate (sign-extended 12-bit)
```asm
addi x10, x11, 100       # x10 = x11 + 100 (sign-extended)
addi x10, x0, -5         # x10 = -5  (load immediate pattern)
addi x0, x0, 0           # NOP
```

**SLTI** — Set Less Than Immediate (signed comparison)
```asm
slti x10, x11, 0         # x10 = (x11 < 0) ? 1 : 0
```

**ANDI, ORI, XORI** — Bitwise AND/OR/XOR with immediate
```asm
andi x10, x11, 0xFF       # x10 = x11 & 0xFF
ori  x10, x11, 0x800      # x10 = x11 | 0x800
xori x10, x11, 0x123      # x10 = x11 ^ 0x123
```

**SLLI / SRLI / SRAI** — Shifts (shamt in lower 5 bits of immediate)
```asm
slli x10, x11, 5          # x10 = x11 << 5
srli x10, x11, 3          # x10 = x11 >> 3 (logical)
srai x10, x11, 3          # x10 = x11 >> 3 (arithmetic, sign-extend)
```

**Note:** Shift immediate's upper bits [11:6] must be zero (undefined behavior if non-zero; may trap in future).

### 2. Integer Computational — Register-Register

**ADD / SUB** — Addition/Subtraction (funct7 distinguishes)
```asm
add  x10, x11, x12       # x10 = x11 + x12
sub  x10, x11, x12       # x10 = x11 - x12
```

**SLT / SLTU** — Set Less Than (signed/unsigned)
```asm
slt  x10, x11, x12       # x10 = (x11 < x12) ? 1 : 0 (signed)
sltu x10, x11, x12       # x10 = (x11 < x12) ? 1 : 0 (unsigned)
```

**AND / OR / XOR** — Bitwise operations
```asm
and x10, x11, x12
or  x10, x11, x12
xor x10, x11, x12
```

**SLL / SRL / SRA** — Variable shifts
```asm
sll x10, x11, x12        # x10 = x11 << (x12 & 0x1F)
srl x10, x11, x12        # x10 = x11 >> (x12 & 0x1F)
sra x x10, x11, x12      # x10 = x11 >> (x12 & 0x1F), sign-extend
```

### 3. Constant Construction

**LUI** — Load Upper Immediate
```asm
lui x10, 0x12345          # x10 = 0x12345000 (imm[31:12] << 12)
```

**AUIPC** — Add Upper Immediate to PC
```asm
auipc x10, 0x12345        # x10 = PC + (0x12345 << 12)
```

**Why both?** LUI sets absolute 32-bit constants; AUIPC enables PC-relative addressing (position-independent code).

### 4. Control Flow — Jumps & Links

**JAL** — Jump and Link (PC-relative)
```asm
jal x1, label            # x1 = PC+4; PC = PC + sign-extended-imm (×2)
jal x0, label            # pure jump (discard link)
```

**JALR** — Jump and Link Register (indirect)
```asm
jalr x1, x10, 0          # x1 = PC+4; PC = (x10 & ~1)  (LSB forced 0)
jalr x0, x10, 8          # jump to x10+8, no link
```

**Key:** JALR clears LSB of target address → all indirect jumps target **even** addresses (ensures ABI alignment).

### 5. Control Flow — Conditional Branches

**BEQ / BNE** — Equal / Not Equal
```asm
beq x10, x11, label      # if x10==x11: PC += sign-extend(imm<<1)
bne x10, x11, label      # if x10!=x11: take branch
```

**BLT / BGE / BLTU / BGEU** — Signed/Unsigned comparisons
```asm
blt  x10, x11, label     # signed: if x10 < x11, branch
bge  x10, x11, label     # signed: if x10 >= x11, branch
bltu x10, x11, label     # unsigned comparison
bgeu x10, x11, label
```

**Branch offset:** 12-bit signed immediate, LSB forced 0 (branch target always 2-byte aligned minimum; in RV32I code must be 4-byte aligned anyway).

### 6. Memory Access — Loads & Stores

**LB / LH / LW** — Load byte/half/word (sign-extending)
```asm
lb  x10, 0(x11)          # x10 = sign_extend(mem[x11+0])
lh  x10, 4(x11)          # x10 = sign_extend(mem[x11+4])
lw  x10, 8(x11)          # x10 = mem[x11+8]  (zero/sign extend to 32)
```

**LBU / LHU** — Load unsigned (zero-extend)
```asm
lbu x10, 0(x11)          # x10 = zero_extend(mem[x11+0])
lhu x10, 4(x11)          # x10 = zero_extend(mem[x11+4])
```

**SB / SH / SW** — Store byte/half/word
```asm
sb x10, 0(x11)           # mem[x11+0] = x10[7:0]
sh x10, 4(x11)           # mem[x11+4] = x10[15:0]
sw x10, 8(x11)           # mem[x11+8] = x10[31:0]
```

**Alignment rules:**
- Naturally aligned accesses (address % size == 0) are guaranteed atomic.
- Misaligned accesses may trap (implementation-defined; EEI decides). Software must align.

### 7. System Instructions

**ECALL** — Environment Call (trap to supervisor/OS)
```asm
ecall                   # Raise exception, jump via MTVEC
```

**EBREAK** — Breakpoint (trap for debugger)
```asm
ebreak                  # Raise breakpoint exception
```

**Note:** In bare-metal (no S-mode/ supervisor), ECALL typically loops or resets; EBREAK may drop into debugger or hang.

### 8. HINT Instructions

HINTs are standard instructions with `rd = x0` that have no architectural side effects. Used by microarchitecture for performance:

```asm
nop                    # addi x0, x0, 0  (HINT to pipeline)
fence                  # memory ordering (see below)
```

---

## Control Flow Deep Dive

### JAL — Jump and Link

**Encoding:** J-type  
**Target calculation:**
```
imm[20|10:1|11|19:12] → {imm[20], imm[10:1], imm[11], imm[19:12], 1'b0}
target = PC + sign_extend(imm[20:1])
```

**Why this bizarre rearrangement?** Because bits in the instruction word must be arranged for easy decoding:
- imm[20] (MSB) placed in inst[31]
- imm[10:1] placed in contiguous blocks
- LSB always 1 (ensures JAL target within ±1 MiB, aligned on 2-byte boundary)

**Example:**
```asm
auipc x10, 0           # x10 = PC
jal   x1, 8            # jump forward 8 bytes (2 instructions), link
# x1 receives PC+4
```

### JALR — Jump and Link Register

**Encoding:** I-type, funct3=0  
**Semantics:**
```
target = (rs1_val & ~1)  # clear LSB → ensure even address
if (rd != 0)  reg[rd] = PC + 4
PC = target + sign_extend(imm[11:0])
```

**Why clear LSB?** Guarantees jump target is word-aligned (RV32 mandates 4-byte alignment for instructions). Also ABI convention: function pointers always have LSB=0; indirect via JALR cannot jump to odd addresses.

**Typical use:**
```asm
# Function call through pointer in x10
jalr x1, x10, 0     # x1=return, jump to *x10
```

### Branches — PC-Relative with B-type Immediates

Branch displacements are **±4 KiB** range (12-bit signed ×2). Small but sufficient for most branches (if/then/else, loops).

**Encoding trick:** Bits of 13-bit signed immediate (imm[12:0], but LSB=0) rearranged:
```
inst[31]        = imm[12]    (sign)
inst[30:25]     = imm[10:5]
inst[11:8]      = imm[4:1]
inst[7]         = imm[11]
```
This places the sign bit at inst[31] (easy to sign-extend) and splits imm into two fields to avoid overlap with rs1/rs2/funct3.

**Example:**
```asm
    beq x10, x11, label   # if equal, branch ±4 KiB
    bne x10, x11, label   # if not equal
```

**Branch Prediction Philosophy:** No branch delay slots. Hardware may speculatively execute past branches; mispredict penalty typical 2 cycles.

### NOP — No Operation

**Encoding:** `ADDI x0, x0, 0` (I-type, rd=x0, rs1=x0, imm=0)  
**Rationale:** NOP is a HINT when rd=x0; writes to x0 discarded. No dedicated NOP opcode needed → saves encoding space.

---

## Memory System & Access Rules

### Load/Store Model

- **Only** loads and stores access memory.
- All other instructions operate on registers only.
- Memory addresses are **byte addresses**, but accesses must be naturally aligned (unless EEI permits misaligned).
- **Endianness:** Little-endian (least significant byte at lowest address). Byte address invariant: `sw` stores word at aligned address `A`; `lb` from `A`, `A+1`, `A+2`, `A+3` each yield correct byte.

**Example little-endian:**
```
address:   A     A+1   A+2   A+3
data:    [7:0] [15:8] [23:16] [31:24]  in little-endian memory
```

### Load Types

- `LW` — Load Word (32-bit, zero-extended/sign-extended? Actually LW loads full 32 bits and zero-extends the upper bits implicitly since XLEN=32). In RV32I, `lw` loads 32 bits, result is 32-bit value (no extension needed).
- `LH` / `LB` — sign extend to 32 bits.
- `LHU` / `LBU` — zero extend to 32 bits.

### Stores

- `SW` stores all 32 bits.
- `SH` stores lower 16 bits.
- `SB` stores lower 8 bits.
- Upper bytes/ halfwords of the memory location are unaffected on partial stores? Actually: stores write exactly the size specified; other bytes in the containing word are not modified unless the store spans a word boundary (misaligned case). In RV32, naturally aligned stores update only the addressed bytes.

### Atomicity

**Load-Reserved (LR) and Store-Conditional (SC)** are part of the **A (Atomic)** extension, **not** in RV32I base. Base RV32I has no atomic memory instructions.

---

## Memory Ordering — FENCE and RVWMO

RV32I includes the `FENCE` instruction for memory ordering. The **RISC-V Weak Memory Ordering (RVWMO)** model defines default relaxed ordering; `FENCE` provides guarantees.

### FENCE Instruction

Encoding: I-type with imm[11:0] as predecessor/successor masks.

```
fence pred, succ   # pred = accesses before fence that must complete; succ = accesses after fence that must wait
```

**Bits:**
- `imm[3:0]` = predecessor (P) mask: I, O, R, W bits
- `imm[7:4]` = successor (S) mask: I, O, R, W bits
- Bits: I=Instruction, O=Order (barrier ordering), R=Read, W=Write

**Common patterns:**

| Fence | Meaning |
|---|---|
| `fence rw, rw` | Full memory barrier (RW,RW) — all pre reads/writes complete before any post reads/writes start |
| `fence w, w` | Write-drain barrier |
| `fence r, r` | Read-drain barrier |
| `fence.tso` | (Pseudoinstruction) expands to `fence rw, rw` with specific ordering (TSO consistency) |

**RVWMO vs. Sequential Consistency:**
Base model allows:
- Load-load reordering
- Store-store reordering
- Load-store reordering (with respect to other addresses)
- Store-load reordering (allowed unless fence prevents)

`fence rw, rw` enforces sequential consistency for the accesses in the predecessor/successor sets.

**Interaction with Devices:** For memory-mapped I/O (MMIO), use `fence` to ensure ordering of register accesses (MMIO is strongly ordered by platform definition; typically `fence w, w` after a store to a device command register).

---

## System Instructions & Trap Model

### ECALL — Environment Call

Triggers an exception, causing:
1. `mcause` ← exception cause (8 for environment call from M-mode)
2. `mepc` ← PC of ECALL instruction (or next PC depending on implementation; spec says PC of ECALL)
3. `mtval` ← 0 (no additional info)
4. PC ← `mtvec` (Machine trap-vector base-address register)

**Use in BootROM:** In a minimal system with no OS, ECALL may simply loop or jump to a hardcoded handler.

### EBREAK — Breakpoint

Similar to ECALL but:
- `mcause` ← 3 (breakpoint)
- Used by debuggers to stop execution

**Semihosting:** Not part of base RV32I. Semihosting typically uses EBREAK with a special immediate value in a register to indicate a system call to the debug host.

### CSR Instructions (Zicsr)

The **Zicsr** extension (CSR operations) is **implicitly required** for any implementation with traps/interrupts. Base ISA includes:
- `CSRRW` — Atomic read/write
- `CSRRS` — Atomic read/set
- `CSRRC` — Atomic read/clear
- plus immediate variants `CSRRSI`, `CSRRCI` (if Zicsr present)

In minimal BootROM, CSR ops may be omitted if traps disabled; but spec says if CSR access to a register is not implemented, illegal instruction exception.

---

## HINTs and Encoding Philosophy

### HINT Mechanism

Any instruction with `rd = x0` is architecturally a **HINT** (no architectural state change). Implementation may treat as NOP or use for microarchitectural hints:

- `fence` with no memory accesses → HINT for ordering
- `prefetch.i` (if Zifencei extension present) → instruction cache prefetch hint
- Custom HINTs via reserved opcode spaces

**Rationale:** Avoids NOP opcode waste; HINTs exploit unused destination register encoding space.

---

## Microarchitectural Implications

### Decoder Simplicity

- Fixed 32-bit instructions → simple, single-cycle decode (no variable-length complexity)
- All formats place opcode in bits [6:0] → fast primary decode
- Three operand format (rd, rs1, rs2) → three read ports, one write port (register file)
- Immediate sign-extension is straightforward (imm[31] copied to upper bits)

### Pipeline Impact

- **No branch delay slots** → single-path ISA, branch resolution typically at EX or MEM stage → 2-cycle penalty typical
- **Load-use hazard:** Load followed by dependent ALU op → 1-cycle stall (if 5-stage pipeline with load-use interlock)
- **Shift by variable amount:** Requires shift amount routed from register file → sometimes separate issue slot
- **ALU operations:** Single-cycle in simple implementations; multiplier (M-extension) may be multi-cycle

### Critical Paths

- **Register file read:** Two read ports (rs1, rs2) → one write port (rd). Write in first half, read in second (open question: write-first or read-first?). Dual-ported SRAM critical.
- **Immediate generation:** Sign extension immediate (12-bit) → early in decode stage → no delay.
- **Branch target adder:** Needs PC + imm<<1 — separate adder from main ALU may help.

---

## Compiler Implications

### Code Generation Patterns

**Loading large constants:**
```asm
lui  x10, 0x12345          # upper 20 bits
addi x10, x10, 0x678       # lower 12 bits (sign-extend! if >=0x800, need addiw or use ori)
```
But `addi` sign-extends its 12-bit immediate. To set bits [11:0] without affecting upper bits, need `ori` after `lui`:
```asm
lui x10, 0x12345
ori x10, x10, 0x678        # zero-extend ORI with 12-bit imm (zero-extended by ORI semantics)
```
But `ori` zero-extends its immediate? Actually ORI: `x[rd] = x[rs1] | imm[11:0]` (zero-extend). Since immediate is 12-bit zero-extended to XLEN before OR. So `lui`+`ori` correctly sets any 32-bit constant except 0 (use `li x10, 0` = `addi x10, x0, 0`).

**Address loading:**
```asm
auipc x10, %hi(symbol)     # x10 = PC + (symbol >> 12)
addi  x10, x10, %lo(symbol)  # x10 = PC + symbol (if symbol within ±2^11 of PC)
```
For absolute addresses (not PC-relative), use `lui`+`addi`:
```asm
lui x10, %hi(symbol)       # upper bits
addi x10, x10, %lo(symbol) # sign-extend lower bits — must work if %lo fits signed 12-bit
```

**Function calls:**
```asm
jal ra, target             # link return, jump
# ...                      # function prologue: save ra to stack if needed
ret:                       # return
    jalr x0, ra, 0         # jump to ra, discard link (x0)
```

**Switch statements:** Often implemented via jump tables using `jr` (register-indirect jump). Indirect jump via `jalr x0, table_base(reg)`.

### Register Pressure

- 32 registers is very generous vs. x86-64 (16) or ARM (16). Call-used vs. callee-saved split allows efficient leaf functions without stack traffic.
- s0–s11 preserved across calls → good for deeply recursive functions; but in embedded BootROM, most functions are leaf, so t0–t6 suffice.

### Optimization Patterns

**Zero idioms:**
- `addi rd, x0, 0` → zero rd
- `or rd, x0, rs` → move rs to rd (copy)
- `andi rd, x0, 0` → clear rd

**Comparison with zero:**
- `slti rd, rs, 0` → set rd = (rs < 0)
- `sltiu rd, rs, 1` → unsigned comparison rc = (rs == 0)

**Branch likely:** RV32I has no branch-likely; all branches have fall-through path.

---

## Performance Tradeoffs

### Branch Prediction

- No branch hint bits in instruction; all branches predicted by hardware dynamically (if enabled).
- Static prediction: forward not-taken, backward taken (common loop heuristic).
- Return Address Stack (RAS): JAL writes PC+4 to rd if rd≠0; if rd is ra (x1 or x5), hardware may push return address onto RAS. JALR with rd≠0 and rs1=ra triggers RAS pop for branch target → supports fast function return.

### Register Pressure vs. Instruction Density

- More registers (32) reduce spills but increase instruction bits needed to encode 5-bit register fields.
- Tradeoff: Larger insns (5-bit fields) but fewer loads/stores overall. Good for register-rich compilation.

### Immediate Encoding Tradeoffs

- 12-bit immediate in I-format limits offset range for loads/stores: `±4 KiB` from base register. Large structs need multiple loads with adjusted base.
- 20-bit U-imm enables large constant construction but requires two instructions (lui+addi). Compromise: keep constant pool in .rodata and use `auipc`+`load` for PC-relative constant load.

---

## Design Philosophies & Why Alternatives Were Rejected

### 1. No Condition Codes (Zero/Overflow Flags)

**Alternative:** x86/ARM have condition codes (C, Z, N, V).  
**Rejected:** Condition codes introduce implicit state, making out-of-order execution and speculative execution complex (flags become dependencies). RISC-V uses register-based comparisons (SLT, XOR, etc.) → explicit, no hidden state, easier to pipeline.

### 2. No Predicated Execution

**Alternative:** ARM Thumb-2 has `IT` block; IA-64 had predicated instructions.  
**Rejected:** Predication complicates speculation and increases code size for unlikely branches (most predicated instructions execute only one path). RV32I prefers simple branches.

### 3. Fixed 32-bit Instructions (No Variable Length in I-Base)

**Alternative:** ARM Thumb mixes 16/32-bit; x86 variable-length.  
**Rejected:** Variable-length complicates fetch/decode; alignment issues; hard to pipeline. RV32I minimal; optional C extension adds 16-bit compressed instructions for density.

### 4. No Zero-Extend Immediates

**Alternative:** Some ISAs have zero-extend immediates for logicals.  
**Rejected:** Sign-extension is sufficient; zero-extend can be synthesized with `andi` (which zero-extends). Having both would waste bits. Only one immediate type (signed) simplifies decode.

### 5. No 2-Address Instructions

**Alternative:** x86 often 2-address (dest overwrites source).  
**Rejected:** 3-address (rd, rs1, rs2) eliminates false dependencies → better for out-of-order execution, simpler register renaming. More bits needed but RV has 32 registers so acceptable.

### 6. Flat 32-bit Address Space

**Alternative:** segmented (x86 real mode) or banked.  
**Rejected:** Too complex; virtual memory optional (via page table, not in base I). Physical addressing keeps MMU/PMP simple.

---

## Advanced Insights

### AUIPC + JALR = Full 32-bit PC-Relative Call

To call a function at arbitrary 32-bit address from position-independent code:

```asm
auipc x10, %pcrel_hi(func)   # x10 = PC + (func >> 12) & 0xFFFFF000
jalr  x1, x10, %pcrel_lo(1)  # x1 = return addr; PC = x10 + sign-ext(%lo)
# The linker resolves %pcrel_hi/%pcrel_lo to ensure correct sum.
```

This is how PIC works in RISC-V.

### Why Branch Offsets Are Multiples of 2

In IALIGN=32-bit machines, all instructions are 4-byte aligned. However branch target calculation includes LSB=0 for alignment hint; if LSB were 1, target would be halfword-aligned but still valid? Actually: the spec requires that the branch offset's LSB is always 0 (encoded as 0). This is because:
- For RV32, branch offsets are multiples of 2 (2-byte aligned) to allow future support of C extension (16-bit instructions).
- Even though base ISA uses 32-bit instructions, keeping alignment 2 allows adding C extension without breaking binary compatibility.

### Why JALR Clears LSB

Spec: `target = rs1 & ~1`.  
Reason:
- Guarantees target address is **word-aligned** (4-byte for RV32I).  
- Prevents jumping to middle of instruction.
- ABI: function pointers have bit0 = 0. If ABI ever uses LSB for other meaning (Thumb, like ARM), can't clear; RISC-V chooses simple rule.

### NOP Encoding Rationale

`ADDI x0, x0, 0`:
- Simple decode: I-type, opcode=OP-IMM, rd=0, rs1=0, funct3=0, imm=0
- No side effects (writes to x0 discarded)
- Hints may use other `* x0, ...` encodings.

### RV32I Enables Emulation of Other ISAs

Because RV32I is orthogonal and has enough registers, it's easy to emulate smaller ISAs:
- Emulate 16-bit ISA by interpreting with 32-bit RISC-V code.
- Emulate x86's 8-register model using fewer RV32 registers.

### Compressed (C) Extension Impact on ABI

C extension adds 16-bit instructions; the compressed-spec states:
- `x1` (ra) and `x2` (sp) have special roles in C branches (`c.j` expands to `jal x0, offset`).
- Stack pointer must maintain 16-byte alignment at function call sites (ABI requirement).
- C extension reuses encodings of `addi x0, x0, nop` for dense `c.nop`.

---

## Common Pitfalls & Edge Cases

### 1. Sign-Extension Gotcha with LUI+ADDI

When constructing 32-bit constants:
```asm
lui x10, 0x80000     # sets x10[31:12] = 0x80000, bits[11:0] = 0
addi x10, x10, 0xFF  # imm=0xFF sign-extended to 0xFFFFFFFF → x10 = 0x7FFFFF? No!
```
Because `addi` sign-extends its immediate, adding 0xFF (-1) to a positive number may decrease it. Use `ori` for zero-extend.

**Correct:**
```asm
lui x10, 0x80000
ori x10, x10, 0xFF   # zero-extend OR
```

### 2. Branch Offset Range

Only ±4 KiB. If branch target farther, need:
```asm
bne x10, x11, far_label  # fails if out of range → compile-time error
# Compiler emits:
    beq  x0, x0, skip
    j    far_label      # unconditional jump using JAL
skip:
```

### 3. Shift Amount Masking

Shift amount only looks at lower 5 bits (for 32-bit). Spec says high bits are **don't care**, but implementations may ignore or trap. Best practice: ensure shift amount in range 0–31 (`slli x10, x11, 32` is same as `slli x10, x11, 0` on most implementations, but not guaranteed).

### 4. Misaligned Accesses

RV32I does **not** require support for misaligned loads/stores. If enabled, they may:
- Traps (C load/store misaligned exception)
- Behave atomically (if EEI defines)
- Corrupt memory (unknown)

**Never** rely on misaligned accesses in portable software.

### 5. x0 Write Behavior

Writes to x0 are **discarded**, not ignored. Compiler may generate `add x0, x0, x0` to stall pipeline; some HINTs use `x0` destination.

### 6. JALR LSB Clearing

If input address has LSB=1, JALR clears it. This could break function pointers if caller set LSB=1 (Thumb-style). Never set LSB in RV registers.

### 7. AUIPC + Load for PC-Relative Constants

Large constants relative to PC:
```asm
auipc x10, %pcrel_hi(symbol)
lw    x11, %pcrel_lo(symbol)(x10)
```
But must ensure `%pcrel_lo` fits signed 12-bit offset from `auipc` result.

### 8. No Implicit Zero Extension for Loads

`lw` loads 32 bits; no upper bits to extend. But `lh` sign-extends, `lhu` zero-extends. Mistaking `lh` for zero-extend yields bugs.

### 9. Stack Alignment

ABI requires stack pointer (`sp`) **16-byte aligned** at function call boundaries. Violating may break `double` or vector accesses (if present). BootROM may relax, but Linux/SBI require 16-byte alignment.

### 10. Undefined Behavior

- Using x0 as source in arithmetic yields 0 (well-defined: x0 always reads 0).
- Shifts by >= XLEN: in RV32, shift amount is modulo 32 (only lower 5 bits used), but future extensions might trap. Safer: mask shift amount.

---

## Microarchitecture vs. Architecture — HINTs

HINTs are architecturally NOPs but may guide microarchitecture:

| HINT (rd=x0) | Microarchitectural meaning (if implemented) |
|---|---|
| `fence` | Full barrier; no effect if no outstanding accesses |
| `prefetch.i rd, imm(rs1)` (if Zifencei) | Prefetch instruction cache at address rs1+imm |
| `fence.i` (deprecated) | Invalidate I-cache |
| Custom HINTs (vendor) | reserved opcode spaces may convey implementation-specific hints |

Not all implementations support HINTs; they are optional no-ops.

---

## Extensibility & Future-Proofing

### Reserved Opcodes

- Many opcode spaces reserved for future standard extensions (e.g., B, V, J, P, GM) and custom vendor extensions.
- **Custom-3/Zero-reserved:** Bits [1:0] = 3 (for 32-bit custom instructions); bits [1:0] = 0 (for 16-bit C, but base doesn't use 16-bit) allow future expansion.

### Standard vs. Custom Extensions

- Standard extensions identified by single letters (M, A, F, D, C, etc.) encoded in `misa` CSR.
- Custom extensions use non-letter bits in `misa` or separate mechanism (hidden).

### Interrupt Priorities

Base I defines synchronous exceptions (illegal instruction, misaligned address, etc.). Interrupts (timer, external, software) require additional PLIC/CLINT (platform-level) but trap model is defined; CSR addresses are fixed.

---

## Summary: Why RV32I Works

1. **Simplicity** — Only ~50 instructions total (incl. system & hints). Decoder trivial.
2. **Regularity** — All immediates sign-extended; register fields always same position in some formats.
3. **No corner cases** — No condition codes, no predication, no variable-latency integer ops (M-extension may be multi-cycle but architecturally defined).
4. **Compiler-friendly** — 32 registers, 3-operand format, abundant immediates → efficient codegen.
5. **Scalable** — Can scale up to superscalar out-of-order, or down to single-cycle pipeline with same ISA.

---

## Practical BootROM Implications

In a **minimal BootROM** context, only a tiny subset of RV32I is used:

```asm
# start.S — minimal startup
_start:
    lui  sp, %hi(_stack_top)   # Setup stack
    addi sp, sp, %lo(_stack_top)
    # Zero .bss if needed:
    li   t0, __bss_start
    li   t1, __bss_end
1:  sw   x0, 0(t0)
    addi t0, t0, 4
    blt  t0, t1, 1b
    # Call C main
    call boot_main
2:  wfi
    j 2b
```

C `boot_main` may use only:
- `call` / `ret` (JAL/JALR)
- `addi` / `lui` / `auipc`
- `sb/sw/lb/lw` if UART MMIO
- `ebreak` / `ecall` if needed
- No floating point, no atomics, no multiplication (`mul` is in M-extension; may not be present)

**Toolchain flags:** `-march=rv64imac` for 64-bit; `-march=rv32imac` for 32-bit. Use `-mabi=lp64` (64-bit) or `-mabi=ilp32` (32-bit).

---

## References

- **RISC-V Unprivileged Spec** (20191213) — Chapter 2-5 for I-base
- **RISC-V Privileged Spec** — Chapter 3 for CSRs/trap handling
- **RISC-V Assembly Programmer’s Manual**
- **Embedded Systems Handbook** — microcontroller constraints alignment

---

**End of analysis.** This document provides a complete, bit-accurate, microarchitecture-aware breakdown of RV32I designed for firmware engineers, compiler writers, and ISA architects implementing or extending RISC-V systems.
