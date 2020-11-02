==== Source: A ====
pragma experimental ABIEncoderV2;

library L {
    struct Item {
        uint x;
    }

    function f(uint) external view returns (Item memory) {}
}
==== Source: B ====
import "A";

contract D {
    using L for uint;

    function test() public {
        uint(1).f();
    }
}
// ----
// TypeError 2428: (B:86-97): The type of return parameter 1, struct L.Item, is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature.
