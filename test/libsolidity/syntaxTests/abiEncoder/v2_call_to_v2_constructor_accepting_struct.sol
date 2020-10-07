==== Source: A ====
pragma experimental ABIEncoderV2;

contract C {
    struct Item {
        uint x;
    }

    constructor(Item memory _item) {}
}
==== Source: B ====
pragma experimental ABIEncoderV2;

import "A";

contract Test {
    function foo() public {
        new C(C.Item(5));
    }
}
// ----
