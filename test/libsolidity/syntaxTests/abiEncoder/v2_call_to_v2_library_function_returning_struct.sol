==== Source: A ====
pragma abicoder               v2;

library L {
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
        L.get();
    }
}
// ----
