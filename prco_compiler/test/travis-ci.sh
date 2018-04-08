#!/bin/bash

num_tests=0
passed_tests=0
ret=0

fnc_make() {
    pushd ../

    mkdir lbuild
    cd lbuild
    cmake ..
    make

    popd
}

fnc_run_cli() {
    ../lbuild/cli/cli -i $1 -d -D0x0002
}

fnc_run_emu() {
    ../lbuild/emu/emu -D0x0000
}

fnc_run_test() {
    echo -n -e "Running test \t$1..."
    num_tests=$(($num_tests + 1))

    fnc_run_cli $1
    fnc_run_emu $1 $2
    code=$?

    if [ $code -eq $2 ]; then
        echo -e "\t\tPASSED"
        passed_tests=$(($passed_tests + 1))
    else
        echo -e "\t\tFAILED\t\tExpected $2, got $code"
    fi
}

fnc_make
fnc_run_test ./tests/binary_ops_1.prco 6
fnc_run_test ./tests/binary_ops_2.prco 11
#fnc_run_test ./tests/binary_ops_3.prco 31


fnc_run_test ./tests/control_for_1.prco 5
fnc_run_test ./tests/control_for_2.prco 1
fnc_run_test ./tests/control_for_3.prco 3

fnc_run_test ./tests/control_if_1.prco 10
fnc_run_test ./tests/control_if_2.prco 2
fnc_run_test ./tests/control_if_3.prco 32

fnc_run_test ./tests/control_while_1.prco 5
fnc_run_test ./tests/control_while_2.prco 5
fnc_run_test ./tests/control_while_3.prco 7
fnc_run_test ./tests/control_while_4.prco 8

fnc_run_test ./tests/foo.prco 1

fnc_run_test ./tests/funcs_1.prco 33
fnc_run_test ./tests/funcs_2.prco 16

fnc_run_test ./tests/ports_uart_1.prco 32

fnc_run_test ./tests/strings_1.prco 2
fnc_run_test ./tests/strings_2.prco 0
fnc_run_test ./tests/strings_3.prco 111

fnc_run_test ./tests/vars_1.prco 0
fnc_run_test ./tests/vars_2.prco 3

echo -e "\n$passed_tests/$num_tests passed."

if [ $passed_tests -eq $num_tests ]; then
    ret=0
else
    ret=$(( $num_tests - $passed_tests ))
fi

echo "Exiting with exit code $ret"
exit $ret