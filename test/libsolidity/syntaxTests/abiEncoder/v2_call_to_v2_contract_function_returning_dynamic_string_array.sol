==== Source: A ====
pragma abicoder               v2;

contract C {
    function f() external view returns (string[] memory) {}
}
==== Source: B ====
pragma abicoder               v2;

import "A";

contract D {
    function g() public view {
        C(address(0x00)).f();
    }
}
// ----
