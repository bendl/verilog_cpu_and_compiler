// prco_constants.v

// Register widths
`define REG_WIDTH       15

// Register names
`define REG_AX          0
`define REG_BX          1
`define REG_CX          2
`define REG_X0          3
`define REG_X1          4
`define REG_SR          5
`define REG_BP          6
`define REG_SP          7

`define STATE_RESET     8'h1
`define STATE_FETCH     8'h2
`define STATE_DECODE    8'h4
`define STATE_READ      8'h8
`define STATE_EXEC      8'h10
`define STATE_RAM       8'h20
`define STATE_WRITE     8'h40
`define STATE_HALT      8'h80