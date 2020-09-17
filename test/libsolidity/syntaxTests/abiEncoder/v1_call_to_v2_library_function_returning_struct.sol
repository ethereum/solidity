==== Source: A ====
pragma experimental ABIEncoderV2;

library L {
    struct Item {
        uint x;
    }

    function get() external view returns(Item memory) {}
}
==== Source: B ====
import "A";

contract Test {
    function foo() public view {
        L.get();
    }
}
// ----
// TypeError 2428: (B:70-77): The type of return parameter 1, struct L.Item, is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature.
