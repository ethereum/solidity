==== Source: A ====
pragma experimental ABIEncoderV2;

contract C {
    struct Item {
        uint x;
    }

    function get() external view returns(Item memory) {}
}
==== Source: B ====
import "A";

contract Test {
    function foo() public view {
        C(0x00).get();
    }
}
// ----
// TypeError 2428: (B:70-83): The type of return parameter 1, struct C.Item, is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature.
