==== Source: A ====
pragma abicoder               v2;

contract C {
    struct Item {
        uint x;
    }

    constructor(Item memory _item) {}
}
==== Source: B ====
pragma abicoder v1;
import "A";

contract Test {
    function foo() public {
        new C(C.Item(5));
    }
}
// ----
// TypeError 2443: (B:91-100): The type of this parameter, struct C.Item, is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
