# PRCO304 Embedded Processor Design and Compiler
This repository is part of the final project for University of Plymouth's PRCO304 module. 

Start by reading `doc/final/build/Ben_Lancaster_10424877.pdf`.

---
## prco304_compiler
[![Build Status](https://travis-ci.com/bendl/prco304.svg?token=uCNqxYXpT8GvYAGuH7zS&branch=master)](https://travis-ci.com/bendl/prco304)

### Build the compiler and emulator
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

or to run all tests (recommended):
```bash
cd prco_compiler/test
./travis-ci.sh
```
### Usage
To compile to machine code:
```bash
cli -i tests/binary_ops_1.prco -d -D0xFF -O1
```

To emulate machine code:
```
emu -D0xFF
```

### Example programs
Example programs can be found in `prco_compiler/test/tests/*.prco`.

---
## prco304_core
Instantiate the `Verilog` processor core using the following snippet (fill in your wires):
```verilog
// Instantiate a processor core
prco_core inst_core (
    .i_clk(), 
    .i_en(), 
    .i_reset(),
    
    // Operating mode (HIGH=single-step)
    .i_mode(),
    // Single-step pulse
    .i_step(),
    
    // UART comms
    .i_rx(),
    .q_tx(),
    .q_tx_byte(),
    
    // Debug outputs
    .q_debug_instr_clk(),
    .q_debug()
);
```

---
Made publically available on: `21/05/2018`