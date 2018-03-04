#!/bin/bash

rm *.d
rm *.o

function run_test {
    PREFIX=$1
    rm -rf obj_dir
    if verilator -Wno-lint -Wno-style -Wno-fatal -cc ../../../rtl/$PREFIX.v -I../../../rtl/ \
        --prefix $PREFIX --public --exe test_$PREFIX.cpp; then
        pushd obj_dir
        if make -f $PREFIX.mk -Iobj_dir > /dev/null; then
            ./$PREFIX 
        fi
        popd
    fi
}

run_test prco_alu