// SPDX-License-Identifier: GPL-2.0
pragma solidity >=0.0;

contract C {
    uint[] x;

    fallback() external {
        uint y_slot = 2;
        uint y_offset = 3;
        uint[] storage y = x;

        assembly {
            pop(y_slot)
            pop(y_offset)

            let z1, z2, z3 := f()

            function f() -> a, b, c {
                a := 1
                b := 2
                c := 3
            }
        }
        y[0] = 2;
    }
}
