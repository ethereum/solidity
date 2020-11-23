==== Source: A ====
pragma abicoder               v2;

contract C {
    struct Item {
        uint x;
    }

    function get(Item memory _item) external {}
}
==== Source: B ====
pragma abicoder               v2;

import "A";

contract Test {
    function foo() public {
        C c = new C();
        function(C.Item memory) external ptr = c.get;
        ptr(C.Item(5));
    }
}
// ----
