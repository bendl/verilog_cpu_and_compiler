// PRCO debug defines

`define BINP17 "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
`define BIN17(b) \
        b[16], \
        b[15], \
        b[14], \
        b[13], \
        b[12], \
        b[11], \
        b[10], \
        b[9], \
        b[8], \
        b[7], \
        b[6], \
        b[5], \
        b[4], \
        b[3], \
        b[2], \
        b[1], \
        b[0]
        
`define BINP16 "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
`define BIN16(b) \
        b[15], \
        b[14], \
        b[13], \
        b[12], \
        b[11], \
        b[10], \
        b[9], \
        b[8], \
        b[7], \
        b[6], \
        b[5], \
        b[4], \
        b[3], \
        b[2], \
        b[1], \
        b[0]

`define BINP8 "%d%d%d%d%d%d%d%d"
`define BIN8(b) \
        b[7], \
        b[6], \
        b[5], \
        b[4], \
        b[3], \
        b[2], \
        b[1], \
        b[0]

`define BINP4 "%d%d%d%d"
`define BIN4(b) \
        b[3], \
        b[2], \
        b[1], \
        b[0]