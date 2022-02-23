==== Source: A ====
pragma abicoder               v2;

contract C {
    struct Item {
        uint x;
    }

    function get() external view returns(Item memory) {}
}
==== Source: B ====
pragma abicoder               v2;

import "A";

contract Test {
    function foo() public view {
        C(address(0x00)).get();
    }
}
// ----
