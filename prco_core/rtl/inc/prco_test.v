// prco_test.v

`define assert(condition) \
    if(!condition) begin \
        $display("Assertion failed!"); $finish(1); \
    end