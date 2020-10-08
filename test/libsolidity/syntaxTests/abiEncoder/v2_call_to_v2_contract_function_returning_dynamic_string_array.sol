==== Source: A ====
pragma experimental ABIEncoderV2;

contract C {
    function f() external view returns (string[] memory) {}
}
==== Source: B ====
pragma experimental ABIEncoderV2;

import "A";

contract D {
    function g() public view {
        C(0x00).f();
    }
}
// ----
