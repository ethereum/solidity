==== Source: A ====
pragma experimental ABIEncoderV2;

library L {
    struct Item {
        uint x;
    }

    function get(Item memory _item) external {}
}
==== Source: B ====
pragma experimental ABIEncoderV2;

import "A";

contract Test {
    function foo() public {
        // NOTE: This test checks a case that is currently not possible (pointer to an external
        // library function) but it might become possible in the future.
        function(L.Item memory) external ptr = L.get;
        ptr(L.Item(5));
    }
}
// ----
// TypeError 9574: (B:269-313): Type function (struct L.Item memory) is not implicitly convertible to expected type function (struct L.Item memory) external.
