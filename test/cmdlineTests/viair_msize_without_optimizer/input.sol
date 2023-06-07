// SPDX-License-Identifier: GPL-3.0
pragma solidity *;

contract C {
    function f() pure public {
        assembly ("memory-safe") {
            function f() -> x {
                x := mload(0)
            }

            // Presence of msize disables all Yul optimizations, including the minimal steps or
            // stack optimization that would normally be performed even with the optimizer nominally disabled.
            // This block should remain untouched when passed through the optimizer.
            pop(msize())

            let x := 0
            let y := x
            mstore(0, f())
        }
    }
}
