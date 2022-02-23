==== Source: A ====
pragma abicoder               v2;

library L {
    struct Item {
        uint x;
    }

    function f(uint) external view returns (Item memory) {}
}
==== Source: B ====
pragma abicoder v1;
import "A";

contract D {
    using L for uint;

    function test() public {
        uint(1).f();
    }
}
// ----
// TypeError 2428: (B:106-117): The type of return parameter 1, struct L.Item, is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
