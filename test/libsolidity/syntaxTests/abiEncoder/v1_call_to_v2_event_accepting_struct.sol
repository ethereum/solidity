==== Source: A ====
pragma abicoder               v2;

library L {
    struct Item {
        uint x;
    }
    event E(Item _value);
}
==== Source: B ====
pragma abicoder v1;
import "A";

contract Test {
    function foo() public {
        emit L.E(L.Item(42));
    }
}
// ----
// TypeError 2443: (B:94-104): The type of this parameter, struct L.Item, is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
