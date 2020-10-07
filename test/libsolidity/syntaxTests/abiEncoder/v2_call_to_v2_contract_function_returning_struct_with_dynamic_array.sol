==== Source: A ====
pragma experimental ABIEncoderV2;

contract C {
    struct Item {
        uint[] y;
    }

    function get() external view returns(Item memory) {}
}
==== Source: B ====
pragma experimental ABIEncoderV2;

import "A";

contract Test {
    function foo() public view {
        C(0x00).get();
    }
}
// ----
