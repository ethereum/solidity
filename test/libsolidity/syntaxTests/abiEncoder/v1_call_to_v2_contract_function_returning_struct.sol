==== Source: A ====
pragma abicoder               v2;

contract C {
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
        C(address(0x00)).get();
    }
}
// ----
// TypeError 2428: (B:90-112): The type of return parameter 1, struct C.Item memory, is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
