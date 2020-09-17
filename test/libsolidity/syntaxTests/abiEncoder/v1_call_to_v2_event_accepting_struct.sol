==== Source: A ====
pragma experimental ABIEncoderV2;

library L {
    struct Item {
        uint x;
    }
    event E(Item _value);
}
==== Source: B ====
import "A";

contract Test {
    function foo() public {
        emit L.E(L.Item(42));
    }
}
// ----
// TypeError 2443: (B:74-84): The type of this parameter, struct L.Item, is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature.
