==== Source: A ====
pragma experimental ABIEncoderV2;

library L {
    struct Item {
        uint x;
    }
    event E(Item _value);
}
==== Source: B ====
pragma experimental ABIEncoderV2;

import "A";

contract Test {
    function foo() public {
        emit L.E(L.Item(42));
    }
}
// ----
