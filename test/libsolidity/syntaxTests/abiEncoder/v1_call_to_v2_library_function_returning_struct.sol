==== Source: A ====
pragma abicoder               v2;

library L {
    struct Item {
        uint x;
    }

    function get() external view returns(Item memory) {}
}
==== Source: B ====
pragma abicoder v1;
import "A";

contract Test {
    function foo() public view {
        L.get();
    }
}
// ----
// TypeError 2428: (B:90-97): The type of return parameter 1, struct L.Item, is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
